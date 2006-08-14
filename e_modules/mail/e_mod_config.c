#include <e.h>
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
  int show_label;
  double check_time;
  Evas_List *boxes;

  /* Evas Object References */
  Evas_Object *il;
  Evas_Object *del;
  Evas_Object *configure;
};

static void *_create_data (E_Config_Dialog * cfd);
static void _free_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata);
static Evas_Object *_basic_create_widgets (E_Config_Dialog * cfd, Evas * evas,
					   E_Config_Dialog_Data * cfdata);
static int _basic_apply_data (E_Config_Dialog * cfd,
			      E_Config_Dialog_Data * cfdata);

static void _load_boxes (E_Config_Dialog * cfd);
static int _ilist_header_exists (Evas_Object * il, const char *name);
static void _ilist_cb_selected (void *data);
static void _cb_add_box (void *data, void *data2);
static void _cb_edit_box (void *data, void *data2);
static void _cb_del_box (void *data, void *data2);

void
_config_mail_module (Config_Item * ci)
{
  E_Config_Dialog *cfd;
  E_Config_Dialog_View *v;
  E_Container *con;
  char buf[4096];

  v = E_NEW (E_Config_Dialog_View, 1);
  if (!v)
    return;

  v->create_cfdata = _create_data;
  v->free_cfdata = _free_data;
  v->basic.apply_cfdata = _basic_apply_data;
  v->basic.create_widgets = _basic_create_widgets;

  snprintf (buf, sizeof (buf), "%s/module.eap",
	    e_module_dir_get (mail_config->module));
  con = e_container_current_get (e_manager_current_get ());
  cfd = e_config_dialog_new (con, _("Mail Configuration"), "Mail", "_e_modules_mail_config_dialog", buf, 0, v, ci);
  mail_config->config_dialog = cfd;
}

static void
_fill_data (Config_Item * ci, E_Config_Dialog_Data * cfdata)
{
  cfdata->show_label = ci->show_label;
  cfdata->check_time = ci->check_time;
  cfdata->boxes = ci->boxes;
}

static void *
_create_data (E_Config_Dialog * cfd)
{
  E_Config_Dialog_Data *cfdata;
  Config_Item *ci;

  ci = cfd->data;
  cfdata = E_NEW (E_Config_Dialog_Data, 1);
  _fill_data (ci, cfdata);
  return cfdata;
}

static void
_free_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata)
{
  if (!mail_config)
    return;
  mail_config->config_dialog = NULL;
  free (cfdata);
  cfdata = NULL;
}

static Evas_Object *
_basic_create_widgets (E_Config_Dialog * cfd, Evas * evas,
		       E_Config_Dialog_Data * cfdata)
{
  Evas_Object *o, *of, *ob, *ot;

  o = e_widget_list_add (evas, 0, 0);
  of = e_widget_framelist_add (evas, _("General Settings"), 0);
  ob =
    e_widget_check_add (evas, _("Always Show Labels"), &(cfdata->show_label));
  e_widget_framelist_object_append (of, ob);
  ob = e_widget_label_add (evas, _("Check Interval"));
  e_widget_framelist_object_append (of, ob);
  ob =
    e_widget_slider_add (evas, 1, 0, _("%2.0f minutes"), 1.0, 60.0, 1.0, 0,
			 &(cfdata->check_time), NULL, 100);
  e_widget_framelist_object_append (of, ob);
  e_widget_list_object_append (o, of, 1, 1, 0.5);

  of = e_widget_frametable_add (evas, _("Mail Boxes"), 0);
  ob = e_widget_ilist_add (evas, 32, 32, NULL);
  e_widget_ilist_selector_set (ob, 1);
  cfdata->il = ob;
  _load_boxes (cfd);
  e_widget_min_size_set (ob, 160, 140);
  e_widget_frametable_object_append (of, ob, 0, 0, 1, 4, 1, 1, 1, 1);

  ot = e_widget_table_add (evas, 0);
  ob =
    e_widget_button_add (evas, _("Add"), "widget/add", _cb_add_box, cfd,
			 NULL);
  e_widget_table_object_append (ot, ob, 0, 0, 1, 1, 1, 1, 1, 0);
  ob =
    e_widget_button_add (evas, _("Delete"), "widget/del", _cb_del_box, cfd,
			 NULL);
  e_widget_disabled_set (ob, 1);
  cfdata->del = ob;
  e_widget_table_object_append (ot, ob, 0, 1, 1, 1, 1, 1, 1, 0);
  ob =
    e_widget_button_add (evas, _("Configure..."), "widget/config",
			 _cb_edit_box, cfd, NULL);
  e_widget_disabled_set (ob, 1);
  cfdata->configure = ob;
  e_widget_table_object_append (ot, ob, 0, 2, 1, 1, 1, 1, 1, 0);
  e_widget_frametable_object_append (of, ot, 1, 0, 1, 1, 1, 0, 1, 0);

  e_widget_list_object_append (o, of, 1, 1, 0.5);

  return o;
}

static int
_basic_apply_data (E_Config_Dialog * cfd, E_Config_Dialog_Data * cfdata)
{
  Config_Item *ci;

  ci = cfd->data;
  ci->show_label = cfdata->show_label;
  ci->check_time = cfdata->check_time;
  e_config_save_queue ();
  _mail_config_updated (ci->id);
  return 1;
}

static void
_load_boxes (E_Config_Dialog * cfd)
{
  E_Config_Dialog_Data *cfdata;
  Evas_Object *il;
  Config_Item *ci;
  int i;
  char buf[4096];

  cfdata = cfd->cfdata;
  il = cfdata->il;
  if (!il)
    return;

  ci = cfd->data;
  if (!ci)
    return;
  if (!ci->boxes)
    return;
  if (evas_list_count (ci->boxes) <= 0)
    return;

  snprintf (buf, sizeof (buf), "%s/module.eap",
	    e_module_dir_get (mail_config->module));

  e_widget_ilist_clear (il);

  for (i = 0; i <= 3; i++)
    {
      Evas_List *box;
      int found = -1;
      char *name;

      switch (i)
	{
	case MAIL_TYPE_POP:
	  name = "Pop3";
	  break;
	case MAIL_TYPE_IMAP:
	  name = "Imap";
	  break;
	case MAIL_TYPE_MDIR:
	  name = "Maildir";
	  break;
	case MAIL_TYPE_MBOX:
	  name = "Mailbox";
	  break;
	}
      found = _ilist_header_exists (il, name);
      if (found <= 0)
	e_widget_ilist_header_append (il, NULL, name);

      for (box = ci->boxes; box; box = box->next)
	{
	  Config_Box *cb;
	  Evas_Object *ic;

	  cb = box->data;
	  if (cb->type != i)
	    continue;
	  if (!cb->name)
	    continue;
	  ic = edje_object_add (cfd->dia->win->evas);
	  edje_object_file_set (ic, buf, "icon");
	  e_widget_ilist_append (il, ic, cb->name, _ilist_cb_selected, cfd,
				 NULL);
	}
    }
  e_widget_ilist_go (il);
}

static int
_ilist_header_exists (Evas_Object * il, const char *name)
{
  int c, i, found = 0;

  if (!il)
    return -1;

  c = e_widget_ilist_count (il);
  if (c <= 0)
    return -1;

  for (i = 0; i < c; i++)
    {
      const char *n;

      n = e_widget_ilist_nth_label_get (il, i);
      if (!n)
	continue;
      if (!strcmp (n, name))
	{
	  found = 1;
	  break;
	}
    }
  if (found)
    return i;
  return -1;
}

static void
_ilist_cb_selected (void *data)
{
  E_Config_Dialog *cfd;
  E_Config_Dialog_Data *cfdata;

  cfd = data;
  cfdata = cfd->cfdata;

  e_widget_disabled_set (cfdata->del, 0);
  e_widget_disabled_set (cfdata->configure, 0);
}

static void
_cb_add_box (void *data, void *data2)
{
  E_Config_Dialog *cfd;
  E_Config_Dialog_Data *cfdata;
  Config_Item *ci;

  cfd = data;
  if (!cfd)
    return;

  ci = cfd->data;
  cfdata = cfd->cfdata;

  _config_box (ci, NULL, cfd);
}

static void
_cb_edit_box (void *data, void *data2)
{
  E_Config_Dialog *cfd;
  E_Config_Dialog_Data *cfdata;
  Evas_Object *il;
  Evas_List *l;
  Config_Item *ci;
  const char *s;

  cfd = data;
  if (!cfd)
    return;

  ci = cfd->data;
  cfdata = cfd->cfdata;
  il = cfdata->il;
  s = e_widget_ilist_selected_label_get (il);

  if (!s)
    return;

  for (l = ci->boxes; l; l = l->next)
    {
      Config_Box *cb;

      cb = l->data;
      if (!cb->name)
	continue;
      if (!strcmp (s, cb->name))
	{
	  _config_box (ci, cb, cfd);
	  break;
	}
    }
}

static void
_cb_del_box (void *data, void *data2)
{
  E_Config_Dialog *cfd;
  E_Config_Dialog_Data *cfdata;
  Evas_Object *il;
  Evas_List *l;
  Config_Item *ci;
  const char *s;

  cfd = data;
  if (!cfd)
    return;

  ci = cfd->data;
  cfdata = cfd->cfdata;
  il = cfdata->il;
  s = e_widget_ilist_selected_label_get (il);

  for (l = ci->boxes; l; l = l->next)
    {
      Config_Box *cb;

      cb = l->data;
      if (!cb->name)
	continue;
      if (!strcmp (s, cb->name))
	{
	  _mail_box_deleted (ci->id, cb->name);
	  break;
	}
    }
  _load_boxes (cfd);
}

void
_mail_box_config_updated (E_Config_Dialog * cfd)
{
  _load_boxes (cfd);
  return;
}
