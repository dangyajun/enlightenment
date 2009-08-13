#include <Elementary.h>

typedef struct Progressbar
{
   Evas_Object *pb1;
   Evas_Object *pb2;
   Evas_Object *pb3;
   Evas_Object *pb4;
   Evas_Object *pb5;
   Evas_Object *pb6;
   Eina_Bool run;
   Ecore_Timer *timer;
} Progressbar;

static Progressbar _test_progressbar;

static int
_my_bt_39_progressbar_value_set (void *data)
{
   double progress;

   progress = elm_progressbar_value_get (_test_progressbar.pb1);
   if (progress < 1.0) progress += 0.0123;
   else progress = 0.0;
   elm_progressbar_value_set(_test_progressbar.pb1, progress);
   elm_progressbar_value_set(_test_progressbar.pb4, progress);
   elm_progressbar_value_set(_test_progressbar.pb3, progress);
   elm_progressbar_value_set(_test_progressbar.pb6, progress);
   if (progress < 1.0) return ECORE_CALLBACK_RENEW;
   _test_progressbar.run = 0;
   return ECORE_CALLBACK_CANCEL;
}

static void
my_bt_39_test_start(void *data, Evas_Object *obj, void *event_info)
{
   elm_progressbar_pulse(_test_progressbar.pb2, EINA_TRUE);
   elm_progressbar_pulse(_test_progressbar.pb5, EINA_TRUE);
   if (!_test_progressbar.run)
     {
        _test_progressbar.timer = ecore_timer_add(0.1, _my_bt_39_progressbar_value_set, NULL);
        _test_progressbar.run = EINA_TRUE;
     }
}

static void
my_bt_39_test_stop(void *data, Evas_Object *obj, void *event_info)
{
   elm_progressbar_pulse(_test_progressbar.pb2, EINA_FALSE);
   elm_progressbar_pulse(_test_progressbar.pb5, EINA_FALSE);
   if (_test_progressbar.run)
     {
        ecore_timer_del(_test_progressbar.timer);
        _test_progressbar.run = EINA_FALSE;
     }
}

static void
my_bt_39_destroy(void *data, Evas_Object *obj, void *event_info)
{
   my_bt_39_test_stop(NULL, NULL, NULL);
   evas_object_del(obj);
}

void
test_progressbar(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *pb, *bx, *pbx, *hbx, *bt, *bt_bx, *ic1, *ic2;
   Progressbar *test;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "progressbar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Progressbar");
   evas_object_smart_callback_add(win, "delete-request", my_bt_39_destroy, test);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_show(bx);

   pb = elm_progressbar_add(win);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   evas_object_size_hint_align_set(pb, -1.0, 0.5);
   elm_box_pack_end(bx, pb);
//   elm_progressbar_horizontal_set(pb, EINA_TRUE);
//   elm_progressbar_label_set(pb, "Progression %");
//   elm_progressbar_unit_format_set(pb, NULL);
   evas_object_show(pb);
   _test_progressbar.pb1 = pb;

   pb = elm_progressbar_add(win);
   evas_object_size_hint_align_set(pb, -1.0, 0.5);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   elm_progressbar_label_set(pb, "Infinite bounce");
   elm_progressbar_pulse_set(pb, EINA_TRUE);
   elm_box_pack_end(bx, pb);
   evas_object_show(pb);
   _test_progressbar.pb2 = pb;

   ic1 = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic1, buf, NULL);
   evas_object_size_hint_aspect_set(ic1, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   pb = elm_progressbar_add(win);
   elm_progressbar_label_set(pb, "Label");
   elm_progressbar_icon_set(pb, ic1);
   elm_progressbar_inverted_set(pb, 1);
   elm_progressbar_unit_format_set(pb, "%1.1f units");
   elm_progressbar_span_size_set(pb, 200);
   evas_object_size_hint_align_set(pb, -1.0, 0.5);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   elm_box_pack_end(bx, pb);
   evas_object_show(ic1);
   evas_object_show(pb);
   _test_progressbar.pb3 = pb;

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, 1.0, 1.0);
   evas_object_size_hint_align_set(hbx, -1.0, -1.0);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   pb = elm_progressbar_add(win);
   elm_progressbar_horizontal_set(pb, EINA_FALSE);
   evas_object_size_hint_align_set(pb, -1.0, -1.0);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   elm_box_pack_end(hbx, pb);
   elm_progressbar_span_size_set(pb, 60);
   elm_progressbar_label_set(pb, "percent");
   evas_object_show(pb);
   _test_progressbar.pb4 = pb;

   pb = elm_progressbar_add(win);
   elm_progressbar_horizontal_set(pb, EINA_FALSE);
   evas_object_size_hint_align_set(pb, -1.0, 0.5);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   elm_progressbar_span_size_set(pb, 80);
   elm_progressbar_pulse_set(pb, EINA_TRUE);
   elm_progressbar_unit_format_set(pb, NULL);
   elm_progressbar_label_set(pb, "Infinite bounce");
   elm_box_pack_end(hbx, pb);
   evas_object_show(pb);
   _test_progressbar.pb5 = pb;

   ic2 = elm_icon_add(win);
   elm_icon_file_set(ic2, buf, NULL);
   evas_object_size_hint_aspect_set(ic2, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);
   pb = elm_progressbar_add(win);
   elm_progressbar_horizontal_set(pb, EINA_FALSE);
   elm_progressbar_label_set(pb, "Label");
   elm_progressbar_icon_set(pb, ic2);
   elm_progressbar_inverted_set(pb, 1);
   elm_progressbar_unit_format_set(pb, "%1.2f%%");
   elm_progressbar_span_size_set(pb, 200);
   evas_object_size_hint_align_set(pb, -1.0, 0.5);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   elm_box_pack_end(hbx, pb);
   evas_object_show(ic2);
   evas_object_show(pb);
   _test_progressbar.pb6 = pb;

   bt_bx = elm_box_add(win);
   elm_box_horizontal_set(bt_bx, 1);
   evas_object_size_hint_weight_set(bt_bx, 1.0, 1.0);
   elm_box_pack_end(bx, bt_bx);
   evas_object_show(bt_bx);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Start");
   evas_object_smart_callback_add(bt, "clicked", my_bt_39_test_start, NULL);
   elm_box_pack_end(bt_bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Stop");
   evas_object_smart_callback_add(bt, "clicked", my_bt_39_test_stop, NULL);
   elm_box_pack_end(bt_bx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}
