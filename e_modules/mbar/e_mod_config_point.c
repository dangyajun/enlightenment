#include <e.h>
#include <libgen.h>
#include "e_mod_config.h"

typedef struct _App_Edit App_Edit;
typedef struct _App_Edit_CFData App_Edit_CFData;

struct _App_Edit
{      
   E_App *eap;
   E_App *apps;
   Evas_Object *il;
   Evas_Object *img;
   Evas_Object *img_widget;
   int          img_set;
   
   App_Edit_CFData *cfdata;
};

struct _App_Edit_CFData
{
   /*- BASIC -*/
   char *name;
   char *exe;
   char *generic;
   char *comment;
   char *path;
   int   startup_notify;
   int   wait_exit;
   char *image;
   App_Edit *editor;
   int is_new;
};

/* local subsystem functions */

static void           _e_eap_edit_fill_data(App_Edit_CFData *cdfata);
static void          *_e_eap_edit_create_data(E_Config_Dialog *cfd);
static void           _e_eap_edit_free_data(E_Config_Dialog *cfd, void *data);
static int            _e_eap_edit_basic_apply_data(E_Config_Dialog *cfd, void *data);
static int            _e_eap_edit_advanced_apply_data(E_Config_Dialog *cfd, void *data);
static Evas_Object   *_e_eap_edit_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, void *data);
static Evas_Object   *_e_eap_edit_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, void *data);
static void           _e_eap_edit_select_cb(Evas_Object *obj, char *file, void *data);
static void           _e_eap_edit_hilite_cb(Evas_Object *obj, char *file, void *data);

#define IFDUP(src, dst) if (src) dst = strdup(src); else dst = NULL

/* externally accessible functions */

void
point_edit_show(E_Container *con, E_App *a, E_App *apps, Evas_Object *il)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   App_Edit *editor;

   if (!con) return;
   
   editor = E_NEW(App_Edit, 1);
   if (!editor) return;
   
   editor->eap = a;
   editor->img = NULL;
   editor->il = il;
   editor->apps = apps;
   
   v = E_NEW(E_Config_Dialog_View, 1);
   if (v)
      {
         /* methods */
         v->create_cfdata           = _e_eap_edit_create_data;
         v->free_cfdata             = _e_eap_edit_free_data;
         v->basic.apply_cfdata      = _e_eap_edit_basic_apply_data;
         v->basic.create_widgets    = _e_eap_edit_basic_create_widgets;
         v->override_auto_apply = 1;
         /* create config diaolg for NULL object/data */
         cfd = e_config_dialog_new(con, _("Mount Point Editor"), NULL, 0, v, editor);
      }
}

/* local subsystem functions */

static void 
_e_eap_edit_fill_data(App_Edit_CFData *cfdata)
{
   /*- BASIC -*/
   IFDUP(cfdata->editor->eap->name, cfdata->name);
   IFDUP(cfdata->editor->eap->exe, cfdata->exe);
   IFDUP(cfdata->editor->eap->generic, cfdata->generic);
   IFDUP(cfdata->editor->eap->comment, cfdata->comment);
   //IFDUP(cfdata->editor->eap->icon_class, cfdata->iclass);   
   IFDUP(cfdata->editor->eap->path, cfdata->path);
   cfdata->startup_notify = cfdata->editor->eap->startup_notify;
   cfdata->wait_exit = cfdata->editor->eap->wait_exit;
   cfdata->is_new = 0;
   if (!cfdata->editor->eap->path)
     cfdata->is_new = 1;
}

static void *
_e_eap_edit_create_data(E_Config_Dialog *cfd)
{
   App_Edit_CFData *cfdata;
   
   cfdata = E_NEW(App_Edit_CFData, 1);
   if (!cfdata) return NULL;
   cfdata->editor = cfd->data;
   cfdata->image = NULL;
   _e_eap_edit_fill_data(cfdata);
   return cfdata;
}

static void
_e_eap_edit_free_data(E_Config_Dialog *cfd, void *data)
{
   App_Edit_CFData *cfdata;
   
   cfdata = data;
   E_FREE(cfdata->name);
   E_FREE(cfdata->exe);
   E_FREE(cfdata->generic);
   E_FREE(cfdata->comment);
   free(data);
}

static int
_e_eap_edit_basic_apply_data(E_Config_Dialog *cfd, void *data)
{
   App_Edit *editor;
   E_App *eap;
   App_Edit_CFData *cfdata;
   char *path;
   char buf[4096];
   
   cfdata = data;
   editor = cfdata->editor;
   eap = editor->eap;
      
   if (eap->name) evas_stringshare_del(eap->name);
   if (eap->exe) evas_stringshare_del(eap->exe);
   if (eap->image) evas_stringshare_del(eap->image);
   if (eap->generic) evas_stringshare_del(eap->generic);
   if (eap->comment) evas_stringshare_del(eap->comment);
   
   if (cfdata->is_new) 
     {
	if (cfdata->name)
	  path = basename(cfdata->name);
	snprintf(buf, sizeof(buf), "%s/.e/e/applications/all/%s.eap", getenv("HOME"), path);
	eap->path = buf;
     }
   
   if (cfdata->name) eap->name = evas_stringshare_add(cfdata->name);
   if (cfdata->exe) eap->exe = evas_stringshare_add(cfdata->exe);
   if (cfdata->image) eap->image = evas_stringshare_add(cfdata->image);
   if (cfdata->generic) eap->generic = evas_stringshare_add(cfdata->generic);
   if (cfdata->comment) eap->comment = evas_stringshare_add(cfdata->comment);
      
   /* FIXME: hardcoded until the eap editor provides fields to change it */
   eap->width = 128;
   eap->height = 128;   
   
   e_app_fields_save(eap);

   if (cfdata->is_new) 
     {
	e_app_append(eap, editor->apps);
	_mbar_point_new(editor->il, eap);
     }
   
   return 1;
}

static Evas_Object *
_e_eap_edit_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, void *data)
{
   App_Edit_CFData *cfdata;
   App_Edit *editor;
   E_App *eap;
   Evas_Object *ol, *o;
   Evas_Object *entry;   
   
   cfdata = data;
   editor = cfdata->editor;
   eap = editor->eap;
   
   //ol = e_widget_list_add(evas, 0, 1);
   ol = e_widget_table_add(evas, 0);
   
   o = e_widget_frametable_add(evas, _("Icon"), 0);
   
   if(!editor->img || editor->img_set != 1)
     {
	editor->img = e_icon_add(evas);   
	if(eap->path)
	  {      
	     e_icon_file_key_set(editor->img, eap->path, "images/0");
	     e_icon_fill_inside_set(editor->img, 1);	
	  }
     }
   else if(editor->img_set)
     {
        editor->img = e_icon_add(evas);
	e_icon_file_set(editor->img, cfdata->image);
	e_icon_fill_inside_set(editor->img, 1);	
     }
   
   editor->img_widget = e_widget_iconsel_add(evas, editor->img, 48, 48, 
					     &(cfdata->image));
   e_widget_iconsel_select_callback_add(editor->img_widget, _e_eap_edit_select_cb, editor);
   e_widget_iconsel_hilite_callback_add(editor->img_widget, _e_eap_edit_hilite_cb, editor);
   e_widget_frametable_object_append(o, editor->img_widget,
				0, 0, 1, 1,
				1, 1, 1, 1);
   e_widget_table_object_append(ol, o, 0, 0, 1, 1, 1 ,1, 1, 1);
      
   o = e_widget_frametable_add(evas, _("General"), 0);
   e_widget_frametable_object_append(o, e_widget_label_add(evas, _("Device")),
				     0, 0, 1, 1,
				     1, 1, 1, 1);    
   entry = e_widget_entry_add(evas, &(cfdata->name));
   e_widget_min_size_set(entry, 100, 1);
   e_widget_frametable_object_append(o, entry,
				     1, 0, 1, 1,
				     1, 1, 1, 1);
   e_widget_frametable_object_append(o, e_widget_label_add(evas, _("Mount Point")),
				0, 1, 1, 1,
				1, 1, 1, 1);
   entry = e_widget_entry_add(evas, &(cfdata->generic));
   e_widget_min_size_set(entry, 100, 1);   
   e_widget_frametable_object_append(o, entry,
				1, 1, 1, 1,
				1, 1, 1, 1);   
   e_widget_frametable_object_append(o, e_widget_label_add(evas, _("Label")),
				0, 2, 1, 1,
				1, 1, 1, 1);
   e_widget_frametable_object_append(o, e_widget_entry_add(evas, &(cfdata->comment)),
				1, 2, 1, 1,
				1, 1, 1, 1);
   e_widget_frametable_object_append(o, e_widget_label_add(evas, _("Execute")),
				     0, 3, 1, 1,
				     1, 1, 1, 1);
   e_widget_frametable_object_append(o, e_widget_entry_add(evas, &(cfdata->exe)),
				     1, 3, 1, 1,
				     1, 1, 1, 1);   
   e_widget_table_object_append(ol, o, 1, 0, 1, 1, 1 ,1, 1, 1);
   
   return ol;
}

void
_e_eap_edit_select_cb(Evas_Object *obj, char *file, void *data)
{
   App_Edit *editor;
   
   editor = data;
   editor->img_set = 1;
   printf("selected: %s\n", file);
}

void
_e_eap_edit_hilite_cb(Evas_Object *obj, char *file, void *data)
{
   App_Edit *editor;
   
   editor = data;
   editor->img_set = 1;   
   printf("hilited: %s\n", file);
}
