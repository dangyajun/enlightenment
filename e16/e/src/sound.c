/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2007 Kim Woelders
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "E.h"
#include "dialog.h"
#include "e16-ecore_list.h"
#include "emodule.h"
#include "settings.h"

#ifdef HAVE_LIBESD
#include <esd.h>
#include <audiofile.h>

#ifdef WORDS_BIGENDIAN
#define SWAP_SHORT( x ) x = ( ( x & 0x00ff ) << 8 ) | ( ( x >> 8 ) & 0x00ff )
#define SWAP_LONG( x ) x = ( ( ( x & 0x000000ff ) << 24 ) |\
      ( ( x & 0x0000ff00 ) << 8 ) |\
      ( ( x & 0x00ff0000 ) >> 8 ) |\
      ( ( x & 0xff000000 ) >> 24 ) )
#endif
#endif

typedef struct
{
   char               *file;
   int                 rate;
   int                 format;
   int                 samples;
   unsigned char      *data;
   int                 id;
} Sample;

typedef struct
{
   char               *name;
   char               *file;
   Sample             *sample;
   unsigned int        ref_count;
} SoundClass;

static struct
{
   char                enable;
   char               *theme;
} Conf_sound;

static int          sound_fd = -1;

static Ecore_List  *sound_list = NULL;

#ifdef HAVE_LIBESD
static Sample      *
LoadWav(const char *file)
{
   AFfilehandle        in_file;
   char               *find = NULL;
   Sample             *s;
   int                 in_format, in_width, in_channels, frame_count;
   int                 bytes_per_frame, frames_read;
   double              in_rate;

   find = FindFile(file, Mode.theme.path, 0);
   if (!find)
     {
	DialogOK(_("Error finding sound file"),
		 _("Warning!  Enlightenment was unable to load the\n"
		   "following sound file:\n%s\n"
		   "Enlightenment will continue to operate, but you\n"
		   "may wish to check your configuration settings.\n"), file);
	return NULL;
     }

   in_file = afOpenFile(find, "r", NULL);
   if (!in_file)
     {
	Efree(find);
	return NULL;
     }

   s = EMALLOC(Sample, 1);
   if (!s)
     {
	Efree(find);
	afCloseFile(in_file);
	return NULL;
     }

   frame_count = afGetFrameCount(in_file, AF_DEFAULT_TRACK);
   in_channels = afGetChannels(in_file, AF_DEFAULT_TRACK);
   in_rate = afGetRate(in_file, AF_DEFAULT_TRACK);
   afGetSampleFormat(in_file, AF_DEFAULT_TRACK, &in_format, &in_width);
#ifdef WORDS_BIGENDIAN
   afSetVirtualByteOrder(in_file, AF_DEFAULT_TRACK, AF_BYTEORDER_BIGENDIAN);
#else
   afSetVirtualByteOrder(in_file, AF_DEFAULT_TRACK, AF_BYTEORDER_LITTLEENDIAN);
#endif
   s->file = Estrdup(find);
   s->rate = 44100;
   s->format = ESD_STREAM | ESD_PLAY;
   s->samples = 0;
   s->data = NULL;
   s->id = 0;

   if (in_width == 8)
      s->format |= ESD_BITS8;
   else if (in_width == 16)
      s->format |= ESD_BITS16;
   bytes_per_frame = (in_width * in_channels) / 8;
   if (in_channels == 1)
      s->format |= ESD_MONO;
   else if (in_channels == 2)
      s->format |= ESD_STEREO;
   s->rate = (int)in_rate;

   s->samples = frame_count * bytes_per_frame;
   s->data = EMALLOC(unsigned char, frame_count * bytes_per_frame);

   frames_read = afReadFrames(in_file, AF_DEFAULT_TRACK, s->data, frame_count);
   afCloseFile(in_file);
   Efree(find);

   return s;
}

static void
SamplePlay(Sample * s)
{
   int                 size, confirm = 0;

   if ((sound_fd < 0) || (!Conf_sound.enable) || (!s))
      return;

   if (!s->id && s->data)
     {
	size = s->samples;
	s->id = esd_sample_getid(sound_fd, s->file);
	if (s->id < 0)
	  {
	     s->id =
		esd_sample_cache(sound_fd, s->format, s->rate, size, s->file);
	     write(sound_fd, s->data, size);
	     confirm = esd_confirm_sample_cache(sound_fd);
	     if (confirm != s->id)
		s->id = 0;
	  }
	Efree(s->data);
	s->data = NULL;
     }
   if (s->id > 0)
      esd_sample_play(sound_fd, s->id);
}
#endif /* HAVE_LIBESD */

static void
SampleDestroy(Sample * s)
{
#ifdef HAVE_LIBESD
   if ((s->id) && (sound_fd >= 0))
     {
/*      Why the hell is this symbol not in esd? */
/*      it's in esd.h - evil evil evil */
/*      esd_sample_kill(sound_fd,s->id); */
	esd_sample_free(sound_fd, s->id);
     }
#endif
   if (s->data)
      Efree(s->data);
   if (s->file)
      Efree(s->file);
   if (s)
      Efree(s);
}

static void
_SclassSampleDestroy(void *data, void *user_data __UNUSED__)
{
   SoundClass         *sclass = (SoundClass *) data;

   if (!sclass || !sclass->sample)
      return;

   SampleDestroy(sclass->sample);
   sclass->sample = NULL;
}

static SoundClass  *
SclassCreate(const char *name, const char *file)
{
   SoundClass         *sclass;

   sclass = EMALLOC(SoundClass, 1);
   if (!sclass)
      return NULL;

   if (!sound_list)
      sound_list = ecore_list_new();
   ecore_list_prepend(sound_list, sclass);

   sclass->name = Estrdup(name);
   sclass->file = Estrdup(file);
   sclass->sample = NULL;

   return sclass;
}

static void
SclassDestroy(SoundClass * sclass)
{
   if (!sclass)
      return;
   ecore_list_remove_node(sound_list, sclass);
   if (sclass->name)
      Efree(sclass->name);
   if (sclass->file)
      Efree(sclass->file);
   if (sclass->sample)
      SampleDestroy(sclass->sample);
   Efree(sclass);
}

static void
SclassApply(SoundClass * sclass)
{
   if (!sclass || !Conf_sound.enable)
      return;
#ifdef HAVE_LIBESD
   if (!sclass->sample)
      sclass->sample = LoadWav(sclass->file);
   if (sclass->sample)
      SamplePlay(sclass->sample);
   else
      SclassDestroy(sclass);
#endif
}

static int
_SclassMatchName(const void *data, const void *match)
{
   return strcmp(((const SoundClass *)data)->name, (const char *)match);
}

static SoundClass  *
SclassFind(const char *name)
{
   return (SoundClass *) ecore_list_find(sound_list, _SclassMatchName, name);
}

void
SoundPlay(const char *name)
{
   SoundClass         *sclass;

   if (!Conf_sound.enable)
      return;

   if (!name || !*name)
      return;

   sclass = SclassFind(name);
   SclassApply(sclass);
}

static int
SoundFree(const char *name)
{
   SoundClass         *sclass;

   sclass = SclassFind(name);
   SclassDestroy(sclass);

   return sclass != NULL;
}

static void
SoundInit(void)
{
#ifdef HAVE_LIBESD
   int                 fd;
#endif

#ifdef HAVE_LIBESD
   if (!Conf_sound.enable)
      return;

   if (sound_fd != -1)
      return;

   fd = esd_open_sound(NULL);
   if (fd >= 0)
      sound_fd = fd;
   else
     {
	AlertX(_("Error initialising sound"), _("OK"), NULL, NULL,
	       _("Audio was enabled for Enlightenment but there was an error\n"
		 "communicating with the audio server (Esound). Audio will\n"
		 "now be disabled.\n"));
	Conf_sound.enable = 0;
     }
#else
   Conf_sound.enable = 0;
#endif
}

static void
SoundExit(void)
{
   if (sound_fd < 0)
      return;

   ecore_list_for_each(sound_list, _SclassSampleDestroy, NULL);

   close(sound_fd);
   sound_fd = -1;
}

/*
 * Configuration load/save
 */

static int
SoundConfigLoad(FILE * fs)
{
   int                 err = 0;
   SoundClass         *sc;
   char                s[FILEPATH_LEN_MAX];
   char                s1[FILEPATH_LEN_MAX];
   char                s2[FILEPATH_LEN_MAX];
   int                 i1, fields;

   while (GetLine(s, sizeof(s), fs))
     {
	i1 = -1;
	fields = sscanf(s, "%d", &i1);
	if (fields == 1)	/* Just skip the numeric config stuff */
	   continue;

	s1[0] = s2[0] = '\0';
	fields = sscanf(s, "%4000s %4000s", s1, s2);
	if (fields != 2)
	  {
	     Eprintf("Ignoring line: %s\n", s);
	     continue;
	  }
	sc = SclassCreate(s1, s2);
     }
   if (err)
      ConfigAlertLoad("Sound");

   return err;
}

/*
 * Sound module
 */

static void
SoundSighan(int sig, void *prm __UNUSED__)
{
   switch (sig)
     {
     case ESIGNAL_INIT:
	SoundInit();
	break;
     case ESIGNAL_CONFIGURE:
	ConfigFileLoad("sound.cfg", Mode.theme.path, SoundConfigLoad, 1);
	break;
     case ESIGNAL_START:
	if (!Conf_sound.enable)
	   break;
	SoundPlay("SOUND_STARTUP");
	SoundFree("SOUND_STARTUP");
	break;
     case ESIGNAL_EXIT:
/*      if (Mode.wm.master) */
	SoundExit();
	break;
     }
}

/*
 * Configuration dialog
 */

static char         tmp_audio;

static void
CB_ConfigureAudio(Dialog * d __UNUSED__, int val, void *data __UNUSED__)
{
   if (val < 2)
     {
	Conf_sound.enable = tmp_audio;
	if (Conf_sound.enable)
	   SoundInit();
	else
	   SoundExit();
     }
   autosave();
}

static void
_DlgFillSound(Dialog * d __UNUSED__, DItem * table, void *data __UNUSED__)
{
   DItem              *di;

   tmp_audio = Conf_sound.enable;

   DialogItemTableSetOptions(table, 2, 0, 0, 0);

#ifdef HAVE_LIBESD
   di = DialogAddItem(table, DITEM_CHECKBUTTON);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di, _("Enable sounds"));
   DialogItemCheckButtonSetPtr(di, &tmp_audio);
#else
   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 2);
   DialogItemSetText(di,
		     _("Audio not available since EsounD was not\n"
		       "present at the time of compilation."));
#endif
}

const DialogDef     DlgSound = {
   "CONFIGURE_AUDIO",
   N_("Sound"),
   N_("Audio Settings"),
   "SOUND_SETTINGS_AUDIO",
   "pix/sound.png",
   N_("Enlightenment Audio\n" "Settings Dialog\n"),
   _DlgFillSound,
   DLG_OAC, CB_ConfigureAudio,
};

/*
 * IPC functions
 */

static void
SoundIpc(const char *params, Client * c __UNUSED__)
{
   const char         *p;
   char                cmd[128], prm[4096];
   int                 len;
   SoundClass         *sc;

   cmd[0] = prm[0] = '\0';
   p = params;
   if (p)
     {
	len = 0;
	sscanf(p, "%100s %4000s %n", cmd, prm, &len);
	p += len;
     }

   if (!strncmp(cmd, "cfg", 3))
     {
	DialogShowSimple(&DlgSound, NULL);
     }
   else if (!strncmp(cmd, "del", 3))
     {
	SoundFree(prm);
     }
   else if (!strncmp(cmd, "list", 2))
     {
	ECORE_LIST_FOR_EACH(sound_list, sc) IpcPrintf("%s\n", sc->name);
     }
   else if (!strncmp(cmd, "new", 3))
     {
	SclassCreate(prm, p);
     }
   else if (!strncmp(cmd, "off", 2))
     {
	Conf_sound.enable = 0;
	SoundExit();
     }
   else if (!strncmp(cmd, "on", 2))
     {
	Conf_sound.enable = 1;
	SoundInit();
     }
   else if (!strncmp(cmd, "play", 2))
     {
	SoundPlay(prm);
     }
}

static const IpcItem SoundIpcArray[] = {
   {
    SoundIpc,
    "sound", "snd",
    "Sound functions",
    "  sound add <classname> <filename> Create soundclass\n"
    "  sound del <classname>            Delete soundclass\n"
    "  sound list                       Show all sounds\n"
    "  sound off                        Disable sounds\n"
    "  sound on                         Enable sounds\n"
    "  sound play <classname>           Play sounds\n"}
};
#define N_IPC_FUNCS (sizeof(SoundIpcArray)/sizeof(IpcItem))

static const CfgItem SoundCfgItems[] = {
   CFG_ITEM_BOOL(Conf_sound, enable, 0),
   CFG_ITEM_STR(Conf_sound, theme),
};
#define N_CFG_ITEMS (sizeof(SoundCfgItems)/sizeof(CfgItem))

/*
 * Module descriptor
 */
extern const EModule ModSound;
const EModule       ModSound = {
   "sound", "audio",
   SoundSighan,
   {N_IPC_FUNCS, SoundIpcArray},
   {N_CFG_ITEMS, SoundCfgItems}
};
