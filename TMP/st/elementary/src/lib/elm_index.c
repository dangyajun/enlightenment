#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Index Index
 *
 * XXX
 *
 * Signals that you can add callbacks for are:
 *
 * xxx - XXX.
 * 
 * xxx - XXX.
 */

typedef struct _Widget_Data Widget_Data;
typedef struct _Item Item;

struct _Widget_Data
{
   Evas_Object *base;
   Evas_Object *event;
   Evas_Object *bx[2]; // 2 - for now all that's supported
   Eina_List *items; // 1 list. yes N levels, but only 2 for now and # of items will be small
   int level;
   Eina_Bool horizontal : 1;
   Eina_Bool active : 1;
   Eina_Bool down : 1;
};

struct _Item
{
   Evas_Object *obj;
   const char *letter;
   const void *data;
   int level;
   Evas_Object *base;
   Eina_Bool selected : 1;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _index_eval(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
//   if (wd->label) eina_stringshare_del(wd->label);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->horizontal)
     _elm_theme_set(wd->base, "index", "base/horizontal", elm_widget_style_get(obj));
   else
     _elm_theme_set(wd->base, "index", "base/vertical", elm_widget_style_get(obj));
   edje_object_part_swallow(wd->base, "elm.swallow.event", wd->event);
   edje_object_part_swallow(wd->base, "elm.swallow.content", wd->bx[0]);
   if (edje_object_part_exists(wd->base, "elm.swallow.content.sub"))
     {
        if (!wd->bx[1])
          {
             wd->bx[1] = _els_smart_box_add(evas_object_evas_get(wd->base));
             _els_smart_box_orientation_set(wd->bx[1], 0);
             _els_smart_box_homogenous_set(wd->bx[1], 1);
             elm_widget_sub_object_add(obj, wd->bx[1]);
          }
        edje_object_part_swallow(wd->base, "elm.swallow.content.sub", wd->bx[1]);
        evas_object_show(wd->bx[1]);
     }
   else if (wd->bx[1])
     {
        evas_object_del(wd->bx[1]);
        wd->bx[1] = NULL;
     }
   edje_object_message_signal_process(wd->base);
   edje_object_scale_set(wd->base, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
   if (wd->active) _index_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->base, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static Item *
_item_new(Evas_Object *obj, const char *letter, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   it = calloc(1, sizeof(Item));
   if (!it) return NULL;
   it->obj = obj;
   it->letter = eina_stringshare_add(letter);
   it->data = item;
   it->level = wd->level;
   return it;
}

static Item *
_item_find(Evas_Object *obj, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Item *it;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (it->data == item) return it;
     }
   return NULL;
}

static void
_item_free(Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   wd->items = eina_list_remove(wd->items, it);
   if (it->base) evas_object_del(it->base);
   eina_stringshare_del(it->letter);
   free(it);
}

static void
_index_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
}

static void 
_wheel(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Wheel *ev = event_info;
   Evas_Object *obj = o;
}

static void 
_mouse_down(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Object *obj = o;
   if (ev->button != 1) return;
   wd->down = 1;
   printf("down!\n");
}

static void 
_mouse_up(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Object *obj = o;
   if (ev->button != 1) return;
   wd->down = 0;
   printf("up!\n");
}

static void 
_mouse_move(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Object *obj = o;
   Evas_Coord x, y, w, h;

   if (!wd->down) return;
   evas_object_geometry_get(o, &x, &y, &w, &h);
   if (wd->horizontal)
     {
     }
   else
     {
        if (ev->cur.canvas.x < x)
          {
             printf("%i\n", wd->level);
             if (wd->level == 0)
               {
                  printf("level up\n");
                  wd->level = 1;
               }
          }
        else
          {
             if (wd->level == 1)
               {
                  printf("level down\n");
                  wd->level = 0;
               }
          }
     }
}

/**
 * Add a new index to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Index
 */
EAPI Evas_Object *
elm_index_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas_Object *o;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord minw, minh;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "index");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->horizontal = EINA_FALSE;

   wd->base = edje_object_add(e);
   _elm_theme_set(wd->base, "index", "base/vertical", "default");
   elm_widget_resize_object_set(obj, wd->base);
   
   o = evas_object_rectangle_add(e);
   wd->event = o;
   evas_object_color_set(o, 0, 0, 0, 0);
   minw = minh = 0;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(o, minw, minh);
   edje_object_part_swallow(wd->base, "elm.swallow.event", o);
   elm_widget_sub_object_add(obj, o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_WHEEL, _wheel, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _mouse_up, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, obj);
   evas_object_show(o);
   
   wd->bx[0] = _els_smart_box_add(e);
   _els_smart_box_orientation_set(wd->bx[0], 0);
   _els_smart_box_homogenous_set(wd->bx[0], 1);
   elm_widget_sub_object_add(obj, wd->bx[0]);
   edje_object_part_swallow(wd->base, "elm.swallow.content", wd->bx[0]);
   evas_object_show(wd->bx[0]);

   if (edje_object_part_exists(wd->base, "elm.swallow.content.sub"))
     {
        wd->bx[1] = _els_smart_box_add(e);
        _els_smart_box_orientation_set(wd->bx[1], 0);
        _els_smart_box_homogenous_set(wd->bx[1], 1);
        elm_widget_sub_object_add(obj, wd->bx[1]);
        edje_object_part_swallow(wd->base, "elm.swallow.content.sub", wd->bx[1]);
        evas_object_show(wd->bx[1]);
     }

   _sizing_eval(obj);
   return obj;
}

/**
 * Set the active state of the index programatically
 *
 * @param obj The index object
 * @param active The active starte
 *
 * @ingroup Index
 */
EAPI void
elm_index_active_set(Evas_Object *obj, Eina_Bool active)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->active == active) return;
   wd->active = active;
   _index_eval(obj);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_level_set(Evas_Object *obj, int level)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->level == level) return;
   wd->level = level;
   _index_eval(obj);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI int
elm_index_item_level_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->level;
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI const void *
elm_index_item_selected_get(Evas_Object *obj, int level)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Item *it;
   if (!wd) return NULL;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if ((it->selected) && (it->level == level)) return it->data;
     }
   return NULL;
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_append(Evas_Object *obj, const char *letter, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   if (!wd) return;
   it = _item_new(obj, letter, item);
   if (!it) return;
   wd->items = eina_list_append(wd->items, it);
   if (wd->active) _index_eval(obj);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_prepend(Evas_Object *obj, const char *letter, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   if (!wd) return;
   it = _item_new(obj, letter, item);
   if (!it) return;
   wd->items = eina_list_prepend(wd->items, it);
   if (wd->active) _index_eval(obj);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_append_relative(Evas_Object *obj, const char *letter, const void *item, const void *relative)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it, *it_rel;
   if (!wd) return;
   if (!relative)
     {
        elm_index_item_append(obj, letter, item);
        return;
     }
   it = _item_new(obj, letter, item);
   it_rel = _item_find(obj, relative);
   if (!it_rel)
     {
        elm_index_item_append(obj, letter, item);
        return;
     }
   if (!it) return;
   wd->items = eina_list_append_relative(wd->items, it, it_rel);
   if (wd->active) _index_eval(obj);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_prepend_relative(Evas_Object *obj, const char *letter, const void *item, const void *relative)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it, *it_rel;
   if (!wd) return;
   if (!relative)
     {
        elm_index_item_prepend(obj, letter, item);
        return;
     }
   it = _item_new(obj, letter, item);
   it_rel = _item_find(obj, relative);
   if (!it_rel)
     {
        elm_index_item_append(obj, letter, item);
        return;
     }
   if (!it) return;
   wd->items = eina_list_prepend_relative(wd->items, it, it_rel);
   if (wd->active) _index_eval(obj);
}

/**
 * XXX
 *
 * @param obj The index object
 *
 * @ingroup Index
 */
EAPI void
elm_index_item_del(Evas_Object *obj, const void *item)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item *it;
   if (!wd) return;
   it = _item_find(obj, item);
   if (!it) return;
   _item_free(it);
   if (wd->active) _index_eval(obj);
}
