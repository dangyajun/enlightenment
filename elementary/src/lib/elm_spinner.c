#include <Elementary.h>
//#include <ctype.h>
#include "elm_priv.h"
#include "elm_widget_spinner.h"

#include "Eo.h"

EAPI Eo_Op ELM_OBJ_SPINNER_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_SPINNER_CLASS

#define MY_CLASS_NAME "elm_spinner"

static const char SIG_CHANGED[] = "changed";
static const char SIG_DELAY_CHANGED[] = "delay,changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_DELAY_CHANGED, ""},
   {NULL, NULL}
};

static void
_entry_show(Elm_Spinner_Smart_Data *sd)
{
   char buf[32], fmt[32] = "%0.f";

   /* try to construct just the format from given label
    * completely ignoring pre/post words
    */
   if (sd->label)
     {
        const char *start = strchr(sd->label, '%');
        while (start)
          {
             /* handle %% */
             if (start[1] != '%')
               break;
             else
               start = strchr(start + 2, '%');
          }

        if (start)
          {
             const char *itr, *end = NULL;
             for (itr = start + 1; *itr != '\0'; itr++)
               {
                  /* allowing '%d' is quite dangerous, remove it? */
                  if ((*itr == 'd') || (*itr == 'f'))
                    {
                       end = itr + 1;
                       break;
                    }
               }

             if ((end) && ((size_t)(end - start + 1) < sizeof(fmt)))
               {
                  memcpy(fmt, start, end - start);
                  fmt[end - start] = '\0';
               }
          }
     }
   snprintf(buf, sizeof(buf), fmt, sd->val);
   elm_object_text_set(sd->ent, buf);
}

static void
_label_write(Evas_Object *obj)
{
   Eina_List *l;
   char buf[1024];
   Elm_Spinner_Special_Value *sv;

   ELM_SPINNER_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->special_values, l, sv)
     {
        if (sv->value == sd->val)
          {
             snprintf(buf, sizeof(buf), "%s", sv->label);
             goto apply;
          }
     }
   if (sd->label)
     snprintf(buf, sizeof(buf), sd->label, sd->val);
   else
     snprintf(buf, sizeof(buf), "%.0f", sd->val);

apply:
   elm_layout_text_set(obj, "elm.text", buf);
   if (sd->entry_visible) _entry_show(sd);
}

static Eina_Bool
_delay_change(void *data)
{
   ELM_SPINNER_DATA_GET(data, sd);

   sd->delay = NULL;
   evas_object_smart_callback_call(data, SIG_DELAY_CHANGED, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_value_set(Evas_Object *obj,
           double new_val)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   if (sd->round > 0)
     new_val = sd->val_base +
       (double)((((int)(new_val - sd->val_base)) / sd->round) * sd->round);

   if (sd->wrap)
     {
        if (new_val < sd->val_min)
          new_val = sd->val_max;
        else if (new_val > sd->val_max)
          new_val = sd->val_min;
     }
   else
     {
        if (new_val < sd->val_min)
          new_val = sd->val_min;
        else if (new_val > sd->val_max)
          new_val = sd->val_max;
     }

   if (new_val == sd->val) return EINA_FALSE;
   sd->val = new_val;

   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
   if (sd->delay) ecore_timer_del(sd->delay);
   sd->delay = ecore_timer_add(0.2, _delay_change, obj);

   return EINA_TRUE;
}

static void
_val_set(Evas_Object *obj)
{
   double pos = 0.0;

   ELM_SPINNER_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (sd->val_max > sd->val_min)
     pos = ((sd->val - sd->val_min) / (sd->val_max - sd->val_min));
   if (pos < 0.0) pos = 0.0;
   else if (pos > 1.0)
     pos = 1.0;
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.slider", pos, pos);
}

static void
_drag_cb(void *data,
         Evas_Object *_obj __UNUSED__,
         const char *emission __UNUSED__,
         const char *source __UNUSED__)
{
   double pos = 0.0, offset, delta;
   Evas_Object *obj = data;

   ELM_SPINNER_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (sd->entry_visible) return;
   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.dragable.slider", &pos, NULL);

   offset = sd->step * _elm_config->scale;
   delta = (pos - sd->drag_start_pos) * offset;
   /* If we are on rtl mode, change the delta to be negative on such changes */
   if (elm_widget_mirrored_get(obj)) delta *= -1;
   if (_value_set(data, sd->drag_start_pos + delta)) _label_write(data);
   sd->dragging = 1;
}

static void
_drag_start_cb(void *data,
               Evas_Object *obj __UNUSED__,
               const char *emission __UNUSED__,
               const char *source __UNUSED__)
{
   double pos;

   ELM_SPINNER_DATA_GET(data, sd);
   Elm_Widget_Smart_Data *wd = eo_data_get(data, ELM_OBJ_WIDGET_CLASS);

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.dragable.slider", &pos, NULL);
   sd->drag_start_pos = pos;
}

static void
_drag_stop_cb(void *data,
              Evas_Object *obj __UNUSED__,
              const char *emission __UNUSED__,
              const char *source __UNUSED__)
{
   ELM_SPINNER_DATA_GET(data, sd);
   Elm_Widget_Smart_Data *wd = eo_data_get(data, ELM_OBJ_WIDGET_CLASS);

   sd->drag_start_pos = 0;
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.slider", 0.0, 0.0);
}

static void
_entry_hide(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   elm_layout_signal_emit(obj, "elm,state,inactive", "elm");
   sd->entry_visible = EINA_FALSE;
}

static void
_reset_value(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   _entry_hide(obj);
   elm_spinner_value_set(obj, sd->orig_val);
}

static void
_entry_value_apply(Evas_Object *obj)
{
   const char *str;
   double val;
   char *end;

   ELM_SPINNER_DATA_GET(obj, sd);

   if (!sd->entry_visible) return;

   _entry_hide(obj);
   str = elm_object_text_get(sd->ent);
   if (!str) return;
   val = strtod(str, &end);
   if ((*end != '\0') && (!isspace(*end))) return;
   elm_spinner_value_set(obj, val);
}

static void
_entry_toggle_cb(void *data,
                 Evas_Object *obj __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   ELM_SPINNER_DATA_GET(data, sd);

   if (sd->dragging)
     {
        sd->dragging = 0;
        return;
     }
   if (elm_widget_disabled_get(data)) return;
   if (!sd->editable) return;
   if (sd->entry_visible) _entry_value_apply(data);
   else
     {
        sd->orig_val = sd->val;
        elm_layout_signal_emit(data, "elm,state,active", "elm");
        _entry_show(sd);
        elm_entry_select_all(sd->ent);
        elm_widget_focus_set(sd->ent, 1);
        sd->entry_visible = EINA_TRUE;
     }
}

static Eina_Bool
_spin_value(void *data)
{
   ELM_SPINNER_DATA_GET(data, sd);
   double real_speed = sd->spin_speed;

   /* Sanity check: our step size should be at least as large as our rounding value */
   if ((sd->spin_speed != 0.0) && (abs(sd->spin_speed) < sd->round))
     {
        WRN("The spinning step is smaller than the rounding value, please check your code");
        real_speed = sd->spin_speed > 0 ? sd->round : -sd->round;
     }

   if (_value_set(data, sd->val + real_speed)) _label_write(data);
   sd->interval = sd->interval / 1.05;
   ecore_timer_interval_set(sd->spin, sd->interval);

   return ECORE_CALLBACK_RENEW;
}

static void
_val_inc_start(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = sd->step;
   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = ecore_timer_add(sd->interval, _spin_value, obj);
   _spin_value(obj);
}

static void
_val_inc_stop(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = 0;
   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = NULL;
}

static void
_val_dec_start(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = -sd->step;
   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = ecore_timer_add(sd->interval, _spin_value, obj);
   _spin_value(obj);
}

static void
_val_dec_stop(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = 0;
   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = NULL;
}

static void
_button_inc_start_cb(void *data,
                     Evas_Object *obj __UNUSED__,
                     const char *emission __UNUSED__,
                     const char *source __UNUSED__)
{
   ELM_SPINNER_DATA_GET(data, sd);

   if (sd->entry_visible)
     {
        _reset_value(data);
        return;
     }
   _val_inc_start(data);
}

static void
_button_inc_stop_cb(void *data,
                    Evas_Object *obj __UNUSED__,
                    const char *emission __UNUSED__,
                    const char *source __UNUSED__)
{
   _val_inc_stop(data);
}

static void
_button_dec_start_cb(void *data,
                     Evas_Object *obj __UNUSED__,
                     const char *emission __UNUSED__,
                     const char *source __UNUSED__)
{
   ELM_SPINNER_DATA_GET(data, sd);

   if (sd->entry_visible)
     {
        _reset_value(data);
        return;
     }
   _val_dec_start(data);
}

static void
_button_dec_stop_cb(void *data,
                    Evas_Object *obj __UNUSED__,
                    const char *emission __UNUSED__,
                    const char *source __UNUSED__)
{
   _val_dec_stop(data);
}

static void
_entry_activated_cb(void *data,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   ELM_SPINNER_DATA_GET(data, sd);

   _entry_value_apply(data);
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
   if (sd->delay) ecore_timer_del(sd->delay);
   sd->delay = ecore_timer_add(0.2, _delay_change, data);
}

static void
_elm_spinner_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;

   Elm_Widget_Smart_Data *wd = eo_data_get(obj, ELM_OBJ_WIDGET_CLASS);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_elm_spinner_smart_event(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{

   Evas_Object *src = va_arg(*list, Evas_Object *);
   (void) src;
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   void *event_info = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   if (elm_widget_disabled_get(obj)) return;
   if (type == EVAS_CALLBACK_KEY_DOWN)
     {
        Evas_Event_Key_Down *ev = event_info;

        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
        else if (!strcmp(ev->keyname, "Left") ||
                 ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)) ||
                 !strcmp(ev->keyname, "Down") ||
                 ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
          {
             _val_dec_start(obj);
             elm_layout_signal_emit(obj, "elm,left,anim,activate", "elm");
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             if (ret) *ret = EINA_TRUE;
             return;
          }
        else if (!strcmp(ev->keyname, "Right") ||
                 ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)) ||
                 !strcmp(ev->keyname, "Up") ||
                 ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
          {
             _val_inc_start(obj);
             elm_layout_signal_emit(obj, "elm,right,anim,activate", "elm");
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             if (ret) *ret = EINA_TRUE;
             return;
          }
     }
   else if (type == EVAS_CALLBACK_KEY_UP)
     {
        Evas_Event_Key_Down *ev = event_info;

        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
        if (!strcmp(ev->keyname, "Right") ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)) ||
            !strcmp(ev->keyname, "Up") ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
          _val_inc_stop(obj);
        else if (!strcmp(ev->keyname, "Left") ||
                 ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)) ||
                 !strcmp(ev->keyname, "Down") ||
                 ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
          _val_dec_stop(obj);
        else return;

        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        if (ret) *ret = EINA_TRUE;
        return;
     }
}

static void
_elm_spinner_smart_on_focus(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   eo_do_super(obj, elm_wdg_on_focus(&int_ret));
   if (!int_ret) return;

   if (!elm_widget_focus_get(obj))
     _entry_value_apply(obj);

   if (ret) *ret = EINA_TRUE;
}

static char *
_access_info_cb(void *data,
                Evas_Object *obj,
                Elm_Widget_Item *item __UNUSED__)
{
   Evas_Object *spinner;
   const char *txt = elm_widget_access_info_get(obj);

   spinner = data;
   if (!txt) txt = elm_layout_text_get(spinner, "elm.text");
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data,
                 Evas_Object *obj __UNUSED__,
                 Elm_Widget_Item *item __UNUSED__)
{
   if (elm_widget_disabled_get(data))
     return strdup(E_("State: Disabled"));

   return NULL;
}

static void
_access_spinner_register(Evas_Object *obj)
{
   Elm_Access_Info *ai;
   const char* increment_part;
   const char* decrement_part;

   ELM_SPINNER_DATA_GET(obj, sd);

   if (!strcmp(elm_widget_style_get(obj), "vertical"))
     {
        increment_part = "up_bt";
        decrement_part = "down_bt";
     }
   else
     {
        increment_part = "right_bt";
        decrement_part = "left_bt";
     }

   // increment button
   sd->increment_btn_access =
     _elm_access_edje_object_part_object_register
       (obj, elm_layout_edje_get(obj), increment_part);

   ai = _elm_access_object_get(sd->increment_btn_access);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("spinner increment button"));

   // decrement button
   sd->decrement_btn_access =
     _elm_access_edje_object_part_object_register
       (obj, elm_layout_edje_get(obj), decrement_part);

   ai = _elm_access_object_get(sd->decrement_btn_access);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("spinner decrement button"));

   // spinner label
   sd->access_obj = _elm_access_edje_object_part_object_register
                       (obj, elm_layout_edje_get(obj), "access_text");

   ai = _elm_access_object_get(sd->access_obj);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("spinner"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, obj);
   _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, obj);
}

static void
_elm_spinner_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Spinner_Smart_Data *priv = _pd;
   Elm_Widget_Smart_Data *wd = eo_data_get(obj, ELM_OBJ_WIDGET_CLASS);

   eo_do_super(obj, evas_obj_smart_add());

   priv->val = 0.0;
   priv->val_min = 0.0;
   priv->val_max = 100.0;
   priv->wrap = 0;
   priv->step = 1.0;
   priv->first_interval = 0.85;
   priv->entry_visible = EINA_FALSE;
   priv->editable = EINA_TRUE;

   elm_layout_theme_set(obj, "spinner", "base", elm_widget_style_get(obj));
   elm_layout_signal_callback_add(obj, "drag", "*", _drag_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,start", "*", _drag_start_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,stop", "*", _drag_stop_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,step", "*", _drag_stop_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,page", "*", _drag_stop_cb, obj);

   elm_layout_signal_callback_add
     (obj, "elm,action,increment,start", "*", _button_inc_start_cb, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,increment,stop", "*", _button_inc_stop_cb, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,decrement,start", "*", _button_dec_start_cb, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,decrement,stop", "*", _button_dec_stop_cb, obj);

   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.slider", 0.0, 0.0);

   priv->ent = elm_entry_add(obj);
   elm_entry_single_line_set(priv->ent, EINA_TRUE);
   evas_object_smart_callback_add
     (priv->ent, "activated", _entry_activated_cb, obj);

   elm_layout_content_set(obj, "elm.swallow.entry", priv->ent);
   elm_layout_signal_callback_add
     (obj, "elm,action,entry,toggle", "*", _entry_toggle_cb, obj);

   _label_write(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_spinner_register(obj);
}

static void
_elm_spinner_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Spinner_Special_Value *sv;

   Elm_Spinner_Smart_Data *sd = _pd;

   if (sd->label) eina_stringshare_del(sd->label);
   if (sd->delay) ecore_timer_del(sd->delay);
   if (sd->spin) ecore_timer_del(sd->spin);
   if (sd->special_values)
     {
        EINA_LIST_FREE (sd->special_values, sv)
          {
             eina_stringshare_del(sv->label);
             free(sv);
          }
     }

   eo_do_super(obj, evas_obj_smart_del());
}

static void
_elm_spinner_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret;

   int_ret = elm_layout_theme_set(obj, "spinner", "base",
                              elm_widget_style_get(obj));
   if (ret) *ret = int_ret;

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_spinner_register(obj);
}

static Eina_Bool _elm_spinner_smart_focus_next_enable = EINA_FALSE;

static void
_elm_spinner_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = _elm_spinner_smart_focus_next_enable;
}

static void
_elm_spinner_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_spinner_smart_focus_next(Eo *obj, void *_pd, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   Eina_List *items = NULL;

   ELM_SPINNER_CHECK(obj);
   Elm_Spinner_Smart_Data *sd = _pd;

   items = eina_list_append(items, sd->access_obj);
   items = eina_list_append(items, sd->decrement_btn_access);
   items = eina_list_append(items, sd->increment_btn_access);

   int_ret = elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);
   if (ret) *ret = int_ret;
}

static void
_elm_spinner_smart_access(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   ELM_SPINNER_CHECK(obj);

   _elm_spinner_smart_focus_next_enable = va_arg(*list, int);
   if (_elm_spinner_smart_focus_next_enable)
     {
        _access_spinner_register(obj);
     }
   else
     {
        //TODO: unregister edje part object
     }
}

EAPI Evas_Object *
elm_spinner_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));

   Evas_Object *parent = eo_parent_get(obj);
   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);
}

EAPI void
elm_spinner_label_format_set(Evas_Object *obj,
                             const char *fmt)
{
   ELM_SPINNER_CHECK(obj);
   eo_do(obj, elm_obj_spinner_label_format_set(fmt));
}

static void
_elm_spinner_label_format_set(Eo *obj, void *_pd, va_list *list)
{
   const char *fmt = va_arg(*list, const char *);
   Elm_Spinner_Smart_Data *sd = _pd;

   eina_stringshare_replace(&sd->label, fmt);
   _label_write(obj);
   elm_layout_sizing_eval(obj);
}

EAPI const char *
elm_spinner_label_format_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) NULL;
   const char *ret;
   eo_do((Eo *) obj, elm_obj_spinner_label_format_get(&ret));
   return ret;
}

static void
_elm_spinner_label_format_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Spinner_Smart_Data *sd = _pd;
   *ret = sd->label;
}

EAPI void
elm_spinner_min_max_set(Evas_Object *obj,
                        double min,
                        double max)
{
   ELM_SPINNER_CHECK(obj);
   eo_do(obj, elm_obj_spinner_min_max_set(min, max));
}

static void
_elm_spinner_min_max_set(Eo *obj, void *_pd, va_list *list)
{
   double min = va_arg(*list, double);
   double max = va_arg(*list, double);
   Elm_Spinner_Smart_Data *sd = _pd;

   if ((sd->val_min == min) && (sd->val_max == max)) return;
   sd->val_min = min;
   sd->val_max = max;
   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;
   _val_set(obj);
   _label_write(obj);
}

EAPI void
elm_spinner_min_max_get(const Evas_Object *obj,
                        double *min,
                        double *max)
{
   if (min) *min = 0.0;
   if (max) *max = 0.0;

   ELM_SPINNER_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_spinner_min_max_get(min, max));
}

static void
_elm_spinner_min_max_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *min = va_arg(*list, double *);
   double *max = va_arg(*list, double *);

   Elm_Spinner_Smart_Data *sd = _pd;

   if (min) *min = sd->val_min;
   if (max) *max = sd->val_max;
}

EAPI void
elm_spinner_step_set(Evas_Object *obj,
                     double step)
{
   ELM_SPINNER_CHECK(obj);
   eo_do(obj, elm_obj_spinner_step_set(step));
}

static void
_elm_spinner_step_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double step = va_arg(*list, double);
   Elm_Spinner_Smart_Data *sd = _pd;
   sd->step = step;
}

EAPI double
elm_spinner_step_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) 0.0;
   double ret;
   eo_do((Eo *) obj, elm_obj_spinner_step_get(&ret));
   return ret;
}

static void
_elm_spinner_step_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Spinner_Smart_Data *sd = _pd;
   *ret = sd->step;
}

EAPI void
elm_spinner_value_set(Evas_Object *obj,
                      double val)
{
   ELM_SPINNER_CHECK(obj);
   eo_do(obj, elm_obj_spinner_value_set(val));
}

static void
_elm_spinner_value_set(Eo *obj, void *_pd, va_list *list)
{
   double val = va_arg(*list, double);
   Elm_Spinner_Smart_Data *sd = _pd;

   if (sd->val == val) return;
   sd->val = val;
   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;
   _val_set(obj);
   _label_write(obj);
}

EAPI double
elm_spinner_value_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) 0.0;
   double ret;
   eo_do((Eo *) obj, elm_obj_spinner_value_get(&ret));
   return ret;
}

static void
_elm_spinner_value_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Spinner_Smart_Data *sd = _pd;
   *ret = sd->val;
}

EAPI void
elm_spinner_wrap_set(Evas_Object *obj,
                     Eina_Bool wrap)
{
   ELM_SPINNER_CHECK(obj);
   eo_do(obj, elm_obj_spinner_wrap_set(wrap));
}

static void
_elm_spinner_wrap_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool wrap = va_arg(*list, int);
   Elm_Spinner_Smart_Data *sd = _pd;
   sd->wrap = wrap;
}

EAPI Eina_Bool
elm_spinner_wrap_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) EINA_FALSE;
   Eina_Bool ret;
   eo_do((Eo *) obj, elm_obj_spinner_wrap_get(&ret));
   return ret;
}

static void
_elm_spinner_wrap_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Spinner_Smart_Data *sd = _pd;
   *ret = sd->wrap;
}

EAPI void
elm_spinner_special_value_add(Evas_Object *obj,
                              double value,
                              const char *label)
{
   ELM_SPINNER_CHECK(obj);
   eo_do(obj, elm_obj_spinner_special_value_add(value, label));
}

static void
_elm_spinner_special_value_add(Eo *obj, void *_pd, va_list *list)
{
   double value = va_arg(*list, double);
   const char *label = va_arg(*list, const char *);
   Elm_Spinner_Special_Value *sv;
   Eina_List *l;
   Elm_Spinner_Smart_Data *sd = _pd;

   EINA_LIST_FOREACH(sd->special_values, l, sv)
     {
        if (sv->value != value)
          continue;

        eina_stringshare_replace(&sv->label, label);
        _label_write(obj);
        return;
     }

   sv = calloc(1, sizeof(*sv));
   if (!sv) return;
   sv->value = value;
   sv->label = eina_stringshare_add(label);

   sd->special_values = eina_list_append(sd->special_values, sv);
   _label_write(obj);
}

EAPI void
elm_spinner_special_value_del(Evas_Object *obj,
                              double value)
{
   Elm_Spinner_Special_Value *sv;
   Eina_List *l;

   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->special_values, l, sv)
     {
        if (sv->value != value)
          continue;

        sd->special_values = eina_list_remove_list(sd->special_values, l);
        eina_stringshare_del(sv->label);
        free(sv);
        _label_write(obj);
        return;
     }
}

EAPI const char *
elm_spinner_special_value_get(Evas_Object *obj,
                              double value)
{
   Elm_Spinner_Special_Value *sv;
   Eina_List *l;

   ELM_SPINNER_CHECK(obj) NULL;
   ELM_SPINNER_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->special_values, l, sv)
     {
        if (sv->value == value)
          return sv->label;
     }

   return NULL;
}

EAPI void
elm_spinner_editable_set(Evas_Object *obj,
                         Eina_Bool editable)
{
   ELM_SPINNER_CHECK(obj);
   eo_do(obj, elm_obj_spinner_editable_set(editable));
}

static void
_elm_spinner_editable_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool editable = va_arg(*list, int);
   Elm_Spinner_Smart_Data *sd = _pd;
   sd->editable = editable;
}

EAPI Eina_Bool
elm_spinner_editable_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) EINA_FALSE;
   Eina_Bool ret;
   eo_do((Eo *) obj, elm_obj_spinner_editable_get(&ret));
   return ret;
}

static void
_elm_spinner_editable_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Spinner_Smart_Data *sd = _pd;
   *ret = sd->editable;
}

EAPI void
elm_spinner_interval_set(Evas_Object *obj,
                         double interval)
{
   ELM_SPINNER_CHECK(obj);
   eo_do(obj, elm_obj_spinner_interval_set(interval));
}

static void
_elm_spinner_interval_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double interval = va_arg(*list, double);
   Elm_Spinner_Smart_Data *sd = _pd;
   sd->first_interval = interval;
}

EAPI double
elm_spinner_interval_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) 0.0;
   double ret;
   eo_do((Eo *) obj, elm_obj_spinner_interval_get(&ret));
   return ret;
}

static void
_elm_spinner_interval_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Spinner_Smart_Data *sd = _pd;
   *ret = sd->first_interval;
}

EAPI void
elm_spinner_base_set(Evas_Object *obj,
                     double base)
{
   ELM_SPINNER_CHECK(obj);
   eo_do(obj, elm_obj_spinner_base_set(base));
}

static void
_elm_spinner_base_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double base = va_arg(*list, double);
   Elm_Spinner_Smart_Data *sd = _pd;
   sd->val_base = base;
}

EAPI double
elm_spinner_base_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) 0.0;
   double ret;
   eo_do((Eo *) obj, elm_obj_spinner_base_get(&ret));
   return ret;
}

static void
_elm_spinner_base_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Spinner_Smart_Data *sd = _pd;
   *ret = sd->val_base;
}

EAPI void
elm_spinner_round_set(Evas_Object *obj,
                      int rnd)
{
   ELM_SPINNER_CHECK(obj);
   eo_do(obj, elm_obj_spinner_round_set(rnd));
}

static void
_elm_spinner_round_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int rnd = va_arg(*list, int);
   Elm_Spinner_Smart_Data *sd = _pd;
   sd->round = rnd;
}

EAPI int
elm_spinner_round_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) 0;
   int ret;
   eo_do((Eo *) obj, elm_obj_spinner_round_get(&ret));
   return ret;
}

static void
_elm_spinner_round_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Spinner_Smart_Data *sd = _pd;
   *ret = sd->round;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_spinner_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_spinner_smart_del),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME), _elm_spinner_smart_theme),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ON_FOCUS), _elm_spinner_smart_on_focus),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT), _elm_spinner_smart_event),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_spinner_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT), _elm_spinner_smart_focus_next),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_spinner_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACCESS), _elm_spinner_smart_access),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_spinner_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_LABEL_FORMAT_SET), _elm_spinner_label_format_set),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_LABEL_FORMAT_GET), _elm_spinner_label_format_get),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_MIN_MAX_SET), _elm_spinner_min_max_set),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_MIN_MAX_GET), _elm_spinner_min_max_get),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_STEP_SET), _elm_spinner_step_set),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_STEP_GET), _elm_spinner_step_get),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_VALUE_SET), _elm_spinner_value_set),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_VALUE_GET), _elm_spinner_value_get),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_WRAP_SET), _elm_spinner_wrap_set),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_WRAP_GET), _elm_spinner_wrap_get),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_SPECIAL_VALUE_ADD), _elm_spinner_special_value_add),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_EDITABLE_SET), _elm_spinner_editable_set),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_EDITABLE_GET), _elm_spinner_editable_get),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_INTERVAL_SET), _elm_spinner_interval_set),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_INTERVAL_GET), _elm_spinner_interval_get),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_BASE_SET), _elm_spinner_base_set),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_BASE_GET), _elm_spinner_base_get),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_ROUND_SET), _elm_spinner_round_set),
        EO_OP_FUNC(ELM_OBJ_SPINNER_ID(ELM_OBJ_SPINNER_SUB_ID_ROUND_GET), _elm_spinner_round_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
      _elm_spinner_smart_focus_next_enable = EINA_TRUE;
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_LABEL_FORMAT_SET, "Set the format string of the displayed label."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_LABEL_FORMAT_GET, "Get the label format of the spinner."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_MIN_MAX_SET, "Set the minimum and maximum values for the spinner."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_MIN_MAX_GET, "Get the minimum and maximum values of the spinner."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_STEP_SET, "Set the step used to increment or decrement the spinner value."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_STEP_GET, "Get the step used to increment or decrement the spinner value."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_VALUE_SET, "Set the value the spinner displays."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_VALUE_GET, "Get the value displayed by the spinner."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_WRAP_SET, "Set whether the spinner should wrap when it reaches its minimum or maximum value."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_WRAP_GET, "Get whether the spinner should wrap when it reaches its minimum or maximum value."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_SPECIAL_VALUE_ADD, "Set a special string to display in the place of the numerical value."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_EDITABLE_SET, "Set whether the spinner can be directly edited by the user or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_EDITABLE_GET, "Get whether the spinner can be directly edited by the user or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_INTERVAL_SET, "Set the interval on time updates for an user mouse button hold on spinner widgets' arrows."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_INTERVAL_GET, "Get the interval on time updates for an user mouse button hold on spinner widgets' arrows."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_BASE_SET, "Set the base for rounding."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_BASE_GET, "Get the base for rounding."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_ROUND_SET, "Set the round value for rounding."),
     EO_OP_DESCRIPTION(ELM_OBJ_SPINNER_SUB_ID_ROUND_GET, "Get the round value for rounding."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_SPINNER_BASE_ID, op_desc, ELM_OBJ_SPINNER_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Spinner_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_spinner_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
