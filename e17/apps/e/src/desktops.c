#include "e.h"

static Evas_List desktops = NULL;
static Window    e_base_win = 0;
static int       screen_w, screen_h;
static int       current_desk = 0;

static void e_idle(void *data);
static void e_key_down(Eevent * ev);
static void e_key_up(Eevent * ev);
static void e_mouse_down(Eevent * ev);
static void e_mouse_up(Eevent * ev);
static void e_mouse_in(Eevent * ev);
static void e_mouse_out(Eevent * ev);
static void e_window_expose(Eevent * ev);

static void
e_idle(void *data)
{
   Evas_List l;
   
   for (l = desktops; l; l = l->next)
     {
	E_Desktop *desk;
	
	desk = l->data;
	e_desktops_update(desk);
     }
   e_db_runtime_flush();
   return;
   UN(data);
}

/* handling key down events */
static void
e_key_down(Eevent * ev)
{
   Ev_Key_Down          *e;

   e = ev->event;
     {
	Evas_List l;
	
	for (l = desktops; l; l = l->next)
	  {
	     E_Desktop *desk;
	     
	     desk = l->data;
	     
	     if (desk->win.desk == e->win)
	       {
		  if (!strcmp(e->key, "Up"))
		    {
		    }
		  else if (!strcmp(e->key, "Down"))
		    {
		    }
		  else if (!strcmp(e->key, "Left"))
		    {
		    }
		  else if (!strcmp(e->key, "Right"))
		    {
		    }
		  else if (!strcmp(e->key, "Escape"))
		    {
		    }
		  else
		    {
		       /* infact we should pass this onto the view handling */
		       /* this desktop here */
		       char *type;
		       
		       type = e_key_press_translate_into_typeable(e);
		       if (type)
			 {
			 }
		    }
	       }
	  }
     }
}

/* handling key up events */
static void
e_key_up(Eevent * ev)
{
   Ev_Key_Up          *e;

   e = ev->event;
     {
	Evas_List l;
	
	for (l = desktops; l; l = l->next)
	  {
	     E_Desktop *desk;
	     
	     desk = l->data;
	     
	     if (desk->win.desk == e->win)
	       {
	       }
	  }
     }
}

/* handling mouse down events */
static void 
e_mouse_down(Eevent * ev)
{
   Ev_Mouse_Down      *e;
   
   e = ev->event;
     {
	Evas_List l;
	
	for (l = desktops; l; l = l->next)
	  {
	     E_Desktop *desk;
	     
	     desk = l->data;
	     
	     if (desk->win.desk == e->win)
	       {
		  Evas evas;
		  int x, y;
		  
		  evas = desk->evas.desk;
		  e_window_get_root_relative_location(evas_get_window(evas),
						      &x, &y);
		  x = e->rx - x;
		  y = e->ry - y;
		  evas_event_button_down(evas, x, y, e->button);
		  if (e->button == 1)
		    {
		       int count;
		       static E_Menu *menu = NULL;
		       char buf[4096];
		       
		       if (!menu)
			 {
			    menu = e_menu_new();
			    menu->pad.icon = 2;
			    menu->pad.state = 2;
			    for (count = 1; count <= 16; count++)
			      {
				 int count2;
				 E_Menu *menu2;
				 E_Menu_Item *menuitem;
				 char *icons[] = 
				   {
				      "cd.png",
				      "drawer_closed.png",
				      "drawer_cube_closed.png",
				      "drawer_cube_open.png",
				      "drawer_cube_open_socks.png",
				      "drawer_image_closed.png",
				      "drawer_image_open.png",
				      "drawer_image_open_socks.png",
				      "drawer_light_closed.png",
				      "drawer_light_open.png",
				      "drawer_light_open_socks.png",
				      "drawer_open.png",
				      "drawer_open_socks.png",
				      "drawer_palette_closed.png",
				      "drawer_palette_open.png",
				      "drawer_palette_open_socks.png",
				      "drawer_style_closed.png",
				      "drawer_style_open.png",
				      "drawer_style_open_socks.png",
				      "drawer_text_closed.png",
				      "drawer_text_open.png",
				      "drawer_text_open_socks.png",
				      "palette.png",
				      "quake3.png",
				      "trash_closed.png",
				      "trash_full_closed.png",
				      "trash_full_open.png",
				      "trash_open.png",
				      "watch.png"
				   };
				 
				 sprintf(buf, "Menu item %i", count);
				 menuitem = e_menu_item_new(buf);
				 sprintf(buf, "/home/raster/icons/%s", icons[rand() % 29]);
				 menuitem->icon = strdup(buf);
				 menuitem->scale_icon = 1;
				 menuitem->radio = rand() & 0x1;
				 menuitem->check = rand() & 0x1;
				 menuitem->on = rand() & 0x1;
				 if (count < 10)
				   {
				      menu2 = e_menu_new();
				      menu2->pad.icon = 2;
				      menu2->pad.state = 2;
				      menuitem->submenu = menu2;
				   }
				 e_menu_add_item(menu, menuitem);
				 if (count < 10)
				   {
				      for (count2 = 1; count2 <= 14; count2++)
					{
					   E_Menu_Item *menuitem2;
					   E_Menu *menu3;
					   int count3;
					   
					   sprintf(buf, "Submenu item %i", count2);
					   if (!(rand()%3)) menuitem2 = e_menu_item_new(buf);
					   else menuitem2 = e_menu_item_new("");
					   menu3 = e_menu_new();
					   menu3->pad.icon = 2;
					   menu3->pad.state = 2;
					   menuitem2->submenu = menu3;
					   sprintf(buf, "/home/raster/icons/%s", icons[rand() % 29]);
					   if (!(rand()%3)) menuitem2->icon = strdup(buf);
					   menuitem2->scale_icon = 1;
					   menuitem2->radio = rand() & 0x1;
					   menuitem2->check = rand() & 0x1;
					   menuitem2->on = rand() & 0x1;
					   if (!(rand()%3)) menuitem2->separator = 1;
					   e_menu_add_item(menu2, menuitem2);
					   for (count3 = 1; count3 <= 12; count3++)
					     {
						E_Menu_Item *menuitem3;
						
						sprintf(buf, "Submenu item %i", count3);
						menuitem3 = e_menu_item_new(NULL);
						sprintf(buf, "/home/raster/icons/%s", icons[rand() % 29]);
						menuitem3->icon = strdup(buf);
						menuitem3->scale_icon = 1;
						menuitem3->radio = rand() & 0x1;
						menuitem3->check = rand() & 0x1;
						menuitem3->on = rand() & 0x1;
						if (!(rand()%3)) menuitem3->separator = 1;
						e_menu_add_item(menu3, menuitem3);
					     }
					}
				   }
			      }
			 }
		       e_menu_show_at_mouse(menu, e->rx, e->ry, e->time);
		    }
		  if (e->button == 3)
		    e_exec_restart();
		  return;
	       }
	  }
     }
}

/* handling mouse up events */
static void
e_mouse_up(Eevent * ev)
{
   Ev_Mouse_Up      *e;
   
   e = ev->event;
     {
	Evas_List l;
	
	for (l = desktops; l; l = l->next)
	  {
	     E_Desktop *desk;
	     
	     desk = l->data;
	     
	     if (desk->win.desk == e->win)
	       {
		  Evas evas;
		  int x, y;
		  
		  evas = desk->evas.desk;
		  e_window_get_root_relative_location(evas_get_window(evas),
						      &x, &y);
		  x = e->rx - x;
		  y = e->ry - y;
		  evas_event_button_up(evas, x, y, e->button);
		  return;
	       }
	  }
     }
}

/* handling mouse move events */
static void
e_mouse_move(Eevent * ev)
{
   Ev_Mouse_Move      *e;
   
   e = ev->event;
     {
	Evas_List l;
	
	for (l = desktops; l; l = l->next)
	  {
	     E_Desktop *desk;
	     
	     desk = l->data;
	     
	     if (desk->win.desk == e->win)
	       {
		  Evas evas;
		  int x, y;
		  
		  evas = desk->evas.desk;
		  e_window_get_root_relative_location(evas_get_window(evas),
						      &x, &y);
		  x = e->rx - x;
		  y = e->ry - y;
		  evas_event_move(evas, x, y);
		  return;
	       }
	  }
     }
}

/* handling mouse enter events */
static void
e_mouse_in(Eevent * ev)
{
   Ev_Window_Enter      *e;
   
   e = ev->event;
     {
	Evas_List l;
	
	for (l = desktops; l; l = l->next)
	  {
	     E_Desktop *desk;
	     
	     desk = l->data;
	     
	     if (desk->win.desk == e->win)
	       {
		  Evas evas;
		  int x, y;
		  
		  /* focus handling for desktop */
		  e_focus_to_window(e->win);
		  evas = desk->evas.desk;
		  e_window_get_root_relative_location(evas_get_window(evas),
						      &x, &y);
		  x = e->rx - x;
		  y = e->ry - y;
		  evas_event_move(evas, x, y);
		  evas_event_enter(evas);
		  return;
	       }
	  }
     }
}

/* handling mouse leave events */
static void
e_mouse_out(Eevent * ev)
{
   Ev_Window_Leave      *e;
   
   e = ev->event;
     {
	Evas_List l;
	
	for (l = desktops; l; l = l->next)
	  {
	     E_Desktop *desk;
	     
	     desk = l->data;
	     
	     if (desk->win.desk == e->win)
	       {
		  Evas evas;
		  int x, y;
		  
		  evas = desk->evas.desk;
		  e_window_get_root_relative_location(evas_get_window(evas),
						      &x, &y);
		  x = e->rx - x;
		  y = e->ry - y;
		  evas_event_move(evas, x, y);
		  evas_event_leave(evas);
		  return;
	       }
	  }
     }
}

/* handling expose events */
static void
e_window_expose(Eevent * ev)
{
   Ev_Window_Expose      *e;
   
   e = ev->event;
     {
	Evas_List l;
	
	for (l = desktops; l; l = l->next)
	  {
	     E_Desktop *desk;
	     
	     desk = l->data;
	     if (!desk->evas.pmap)
	       {
		  if (evas_get_window(desk->evas.desk) == e->win)
		    evas_update_rect(desk->evas.desk, e->x, e->y, e->w, e->h);
	       }
	  }
     }
}

void
e_desktops_init(void)
{
   E_Desktop *desk;
   
   e_window_get_geometry(0, NULL, NULL, &screen_w, &screen_h);
   e_base_win = e_window_override_new(0, 0, 0, screen_w, screen_h);
   e_window_show(e_base_win);
   desk = e_desktops_new();
   e_desktops_show(desk);
   e_event_filter_handler_add(EV_MOUSE_DOWN,               e_mouse_down);
   e_event_filter_handler_add(EV_MOUSE_UP,                 e_mouse_up);
   e_event_filter_handler_add(EV_MOUSE_MOVE,               e_mouse_move);
   e_event_filter_handler_add(EV_MOUSE_IN,                 e_mouse_in);
   e_event_filter_handler_add(EV_MOUSE_OUT,                e_mouse_out);
   e_event_filter_handler_add(EV_WINDOW_EXPOSE,            e_window_expose);
   e_event_filter_handler_add(EV_KEY_DOWN,                 e_key_down);
   e_event_filter_handler_add(EV_KEY_UP,                   e_key_up);
   e_event_filter_idle_handler_add(e_idle, NULL);
 
   e_icccm_advertise_e_compat();
   e_icccm_advertise_mwm_compat();
   e_icccm_advertise_gnome_compat();
   e_icccm_advertise_kde_compat();
   e_icccm_advertise_net_compat();
   
   e_icccm_set_desk_area_size(0, 1, 1);
   e_icccm_set_desk_area(0, 0, 0);
   e_icccm_set_desk(0, 0);
}

void
e_desktops_scroll(E_Desktop *desk, int dx, int dy)
{
   Evas_List l;
   int xd, yd, wd, hd;
   int grav, grav_stick;   
   
   /* set grav */
   if ((dx ==0) && (dy == 0)) return;
   desk->x -= dx;
   desk->y -= dy;
   xd = yd = wd = hd = 0;
   grav = NorthWestGravity;
   grav_stick= SouthEastGravity;
   if ((dx <= 0) && (dy <= 0))
     {
	grav = NorthWestGravity;
	grav_stick = SouthEastGravity;
	xd = dx;
	yd = dy;
	wd = -dx;
	hd = -dy;
     }
   else if ((dx >= 0) && (dy <= 0))
     {
	grav = NorthEastGravity;
	grav_stick = SouthWestGravity;
	xd = 0;
	yd = dy;
	wd = dx;
	hd = -dy;
     }
   else if ((dx >= 0) && (dy >= 0))
     {
	grav = SouthEastGravity;
	grav_stick = NorthWestGravity;
	xd = 0;
	yd = 0;
	wd = dx;
	hd = dy;
     }
   else if ((dx <= 0) && (dy >= 0))
     {
	grav = SouthWestGravity;
	grav_stick = NorthEastGravity;
	xd = dx;
	yd = 0;
	wd = -dx;
	hd = dy;
     }
   for (l = desk->windows; l; l = l->next)
     {
	E_Border *b;
	
	b = l->data;
	/* if sticky */
	if (b->client.sticky)
	  e_window_gravity_set(b->win.main, StaticGravity);
	e_window_gravity_set(b->win.main, grav);	
     }
   grav_stick = StaticGravity;
   e_window_gravity_set(desk->win.desk, grav_stick);
   /* scroll */
   e_window_move_resize(desk->win.container, 
			xd, yd, 
			screen_w + wd, screen_h + hd);
   /* reset */
   for (l = desk->windows; l; l = l->next)
     {
	E_Border *b;
	
	b = l->data;
	e_window_gravity_set(b->win.main, grav_stick);
     }   
   e_window_move_resize(desk->win.container, 0, 0, screen_w, screen_h);
   e_window_gravity_reset(desk->win.desk);
   for (l = desk->windows; l; l = l->next)
     {
	E_Border *b;
	
	b = l->data;
	e_window_gravity_reset(b->win.main);
	b->current.requested.x += dx;
	b->current.requested.y += dy;
	b->current.x = b->current.requested.x;
	b->current.y = b->current.requested.y;
	b->previous.requested.x = b->current.requested.x;
	b->previous.requested.y = b->current.requested.y;
	b->previous.x = b->current.x;
	b->previous.y = b->current.y;
	b->changed = 1;
     }
   e_sync();
}

void
e_desktops_free(E_Desktop *desk)
{
   while (desk->windows)
     {
	E_Border *b;
	
	b = desk->windows->data;
	e_action_stop_by_object(b, NULL, 0, 0, 0, 0);
	OBJ_UNREF(b);
	OBJ_IF_FREE(b)
	  {
	     e_window_reparent(b->win.client, 0, 0, 0);
	     e_icccm_release(b->win.client);
	     OBJ_FREE(b);
	  }
     }
   e_window_destroy(desk->win.main);
   if (desk->evas.pmap) e_pixmap_free(desk->evas.pmap);
   IF_FREE(desk->name);
   IF_FREE(desk->dir);
   FREE(desk);
}

void
e_desktops_init_file_display(E_Desktop *desk)
{
   int max_colors = 216;
   int font_cache = 1024 * 1024;
   int image_cache = 8192 * 1024;
   char *font_dir;
   
   font_dir = e_config_get("fonts");
   /* software */
   desk->evas.desk = evas_new_all(e_display_get(),
				  desk->win.container,
				  0, 0, screen_w, screen_h,
				  RENDER_METHOD_ALPHA_SOFTWARE,
				  max_colors,
				  font_cache,
				  image_cache,
				  font_dir);
   desk->win.desk = evas_get_window(desk->evas.desk);
   e_add_child(desk->win.container, desk->win.desk);
   /* pixmap backing for desktop */
   desk->evas.pmap = e_pixmap_new(desk->win.desk, screen_w, screen_h, 0);
   evas_set_output(desk->evas.desk, e_display_get(), desk->evas.pmap, 
		   evas_get_visual(desk->evas.desk), evas_get_colormap(desk->evas.desk));
   e_window_set_background_pixmap(desk->win.desk, desk->evas.pmap);
   /* normal stuff */
   e_window_set_events(desk->win.desk, XEV_EXPOSE | XEV_MOUSE_MOVE | XEV_BUTTON | XEV_IN_OUT | XEV_KEY); 
   e_window_show(desk->win.desk);
     {
	Evas_Object o;
	Evas e;
	char buf[4096];
	
	e = desk->evas.desk;
	sprintf(buf, "%sbg.png", e_config_get("images"));
	o = evas_add_image_from_file(e, buf);
	evas_move(e, o, 0, 0);
	evas_resize(e, o, screen_w, screen_h);
	evas_show(e, o);
	sprintf(buf, "%se_logo.png", e_config_get("images"));
	o = evas_add_image_from_file(e, buf);
	evas_move(e, o, 0, 0);
	evas_show(e, o);
     }   
}

E_Desktop *
e_desktops_new(void)
{
   E_Desktop *desk;
   
   desk = NEW(E_Desktop, 1);
   ZERO(desk, E_Desktop, 1);
   
   OBJ_INIT(desk, e_desktops_free);
   
   desk->win.main = e_window_override_new(e_base_win, 0, 0, screen_w, screen_h);
   desk->win.container = e_window_override_new(desk->win.main, 0, 0, screen_w, screen_h);
   
   e_window_show(desk->win.container);

   desk->x = 0;
   desk->y = 0;
   desk->real.w = screen_w;
   desk->real.h = screen_h;
   desk->virt.w = screen_w;
   desk->virt.h = screen_h;
   
   desktops = evas_list_append(desktops, desk);
   
   e_desktops_init_file_display(desk);
   
   return desk;
}

void
e_desktops_add_border(E_Desktop *d, E_Border *b)
{
   if ((!d) || (!b)) return;
   b->desk = d;
   e_border_raise(b);
}

void
e_desktops_del_border(E_Desktop *d, E_Border *b)
{
   if ((!d) || (!b)) return;
   d->windows = evas_list_remove(d->windows, b);
   b->desk = NULL;
}

void
e_desktops_delete(E_Desktop *d)
{
   OBJ_DO_FREE(d);
}

void
e_desktops_show(E_Desktop *d)
{
   e_desktops_update(d);
   e_window_show(d->win.main);
}

void
e_desktops_hide(E_Desktop *d)
{
   e_window_hide(d->win.main);
}

int
e_desktops_get_num(void)
{
   Evas_List l;
   int i;
   
   for (i = 0, l = desktops; l; l = l->next, i++);
   return i;
}

E_Desktop *
e_desktops_get(int d)
{
   Evas_List l;
   int i;
   
   for (i = 0, l = desktops; l; l = l->next, i++)
     {
	if (i == d) return (E_Desktop *)l->data;
     }
   return NULL;
}

int
e_desktops_get_current(void)
{
   return current_desk;
}

void
e_desktops_update(E_Desktop *desk)
{
   Imlib_Updates up;
   
   up = evas_render_updates(desk->evas.desk);
   if (up)
     {
	Imlib_Updates u;
	
	for (u = up; u; u = imlib_updates_get_next(u))
	  {
	     int x, y, w, h;
	     
	     imlib_updates_get_coordinates(u, &x, &y, &w, &h);
	     e_window_clear_area(desk->win.desk, x, y, w, h);
	  }
	imlib_updates_free(up);
     }
   if (desk->changed)
     {
	desk->changed = 0;
     }
}
