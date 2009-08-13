#include <Elementary.h>

void
test_button(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *ic, *bt;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "buttons", ELM_WIN_BASIC);
   elm_win_title_set(win, "Buttons");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Icon sized to button");
   elm_button_icon_set(bt, ic);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Icon no scale");
   elm_button_icon_set(bt, ic);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Label Only");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   bt = elm_button_add(win);
   elm_button_icon_set(bt, ic);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   evas_object_show(win);
}
