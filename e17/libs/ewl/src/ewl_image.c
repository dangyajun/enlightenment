
#include <Ewl.h>

void            __ewl_image_realize(Ewl_Widget * w, void *ev_data,
				    void *user_data);
void            __ewl_image_configure(Ewl_Widget * w, void *ev_data,
				      void *user_data);
void            __ewl_image_mouse_down(Ewl_Widget * w, void *ev_data,
				       void *user_data);
void            __ewl_image_mouse_up(Ewl_Widget * w, void *ev_data,
				     void *user_data);
void            __ewl_image_mouse_move(Ewl_Widget * w, void *ev_data,
				       void *user_data);

Ewl_Image_Type  __ewl_image_get_type(const char *i);

/**
 * ewl_image_set_file - load an image widget with specified image contents
 * @i: the path to the image to be displayed by the image widget
 *
 * Returns a pointer to the newly allocated image widget on success, NULL on
 * failure.
 */
Ewl_Widget     *ewl_image_new(char *i)
{
	Ewl_Image      *image;

	DENTER_FUNCTION(DLEVEL_STABLE);

	image = NEW(Ewl_Image, 1);

	ZERO(image, Ewl_Image, 1);
	ewl_image_init(image, i);

	DRETURN_PTR(EWL_WIDGET(image), DLEVEL_STABLE);
}

/**
 * ewl_image_init - initialize an image widget to default values and callbacks
 * @i: the image widget to initialize
 *
 * Returns no value. Sets the fields and callbacks of @i to their default
 * values.
 */
void ewl_image_init(Ewl_Image * i, char *path)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("i", i);

	w = EWL_WIDGET(i);

	ewl_widget_init(w, "image");

	/*
	 * Append necessary callbacks.
	 */
	ewl_callback_append(w, EWL_CALLBACK_REALIZE, __ewl_image_realize, NULL);
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
			    __ewl_image_configure, NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_DOWN,
			    __ewl_image_mouse_down, NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_UP, __ewl_image_mouse_up,
			    NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_MOVE, __ewl_image_mouse_move,
			    NULL);

	i->sw = 1.0;
	i->sh = 1.0;

	ewl_image_set_file(i, path);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_image_set_file - change the image file displayed by an image widget
 * @i: the image widget to change the displayed image
 * @im: the path to the new image to be displayed by @i
 *
 * Returns no value. Set the image displayed by @i to the one found at the
 * path @im.
 */
void ewl_image_set_file(Ewl_Image * i, char *im)
{
	int             old_type;
	Ewl_Widget     *w;
	Ewl_Embed      *emb;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("i", i);

	w = EWL_WIDGET(i);

	emb = ewl_embed_find_by_widget(w);

	IF_FREE(i->path);

	/*
	 * Determine the type of image to be loaded.
	 */
	old_type = i->type;
	if (im) {
		i->type = __ewl_image_get_type(im);
		i->path = strdup(im);
	}
	else
		i->type = EWL_IMAGE_TYPE_NORMAL;

	/*
	 * Load the new image if widget has been realized
	 */
	if (REALIZED(w)) {
		/*
		 * Free the image if it had been loaded.
		 */
		if (i->image) {

			/*
			 * Type is important for using the correct free calls.
			 */
			evas_object_hide(i->image);
			evas_object_clip_unset(i->image);
			evas_object_del(i->image);

			i->image = NULL;
		}

		/*
		 * Now draw the new image
		 */
		__ewl_image_realize(w, NULL, NULL);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_image_set_proportional - set boolean to determine how to scale
 * @i: the image to change proportional setting
 * @p: the boolean indicator of proportionality
 *
 * Returns no value. Changes the flag indicating if the image is scaled
 * proportionally.
 */
void
ewl_image_set_proportional(Ewl_Image *i, char p)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	DCHECK_PARAM_PTR("i", i);

	i->proportional = p;
	ewl_widget_configure(EWL_WIDGET(i));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_image_scale - scale image dimensions by a percentage
 * @i: the image to scale
 * @wp: the percentage to scale width
 * @hp: the percentage to scale height
 *
 * Returns no value. Scales the given image to @wp percent of preferred width
 * by @hp percent of preferred height. If @i->proportional is set to TRUE, the
 * lesser of @wp and @hp is applied for both directions.
 */
void
ewl_image_scale(Ewl_Image *i, double wp, double hp)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("i", i);

	if (i->proportional) {
		if (wp < hp)
			hp = wp;
		else
			wp = hp;
	}

	i->sw = wp;
	i->sh = hp;

	ewl_object_set_preferred_w(EWL_OBJECT(i), wp * i->ow);
	ewl_object_set_preferred_h(EWL_OBJECT(i), hp * i->oh);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_image_scale_to - scale image dimensions to a specific size
 * @i: the image to scale
 * @w: the size to scale width
 * @h: the size to scale height
 *
 * Returns no value. Scales the given image to @w by @hp. If @i->proportional
 * is set to TRUE, the image is scaled proportional to the lesser scale
 * percentage of preferred size.
 */
void
ewl_image_scale_to(Ewl_Image *i, int w, int h)
{

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("i", i);

	/*
	 * Scale the image to be proportional inside the available space.
	 */
	if (i->ow && i->oh && i->proportional) {
		double wp, hp;

		wp = (double)w / (double)i->ow;
		hp = (double)h / (double)i->oh;

		if (wp < hp)
			hp = wp;
		else
			wp = hp;

		w = wp * i->ow;
		h = hp * i->oh;
	}

	i->sw = 1.0;
	i->sh = 1.0;
	ewl_object_set_preferred_size(EWL_OBJECT(i), w, h);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void __ewl_image_realize(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Image      *i;
	Ewl_Embed      *emb;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	i = EWL_IMAGE(w);

	emb = ewl_embed_find_by_widget(w);

	/*
	 * Load the image based on the type.
	 */
	if (i->type == EWL_IMAGE_TYPE_EDJE) {
		i->image = edje_object_add(emb->evas);
		if (!i->image)
			return;

		if (i->path)
			edje_object_file_set(i->image, i->path, "EWL");

	} else {
		i->image = evas_object_image_add(emb->evas);
		if (!i->image)
			return;

		if (i->path)
			evas_object_image_file_set(i->image, i->path, NULL);

	}

	evas_object_layer_set(i->image, LAYER(w));
	evas_object_clip_set(i->image, w->fx_clip_box);
	evas_object_image_size_get(i->image, &i->ow, &i->oh);
	evas_object_show(i->image);

	if (!i->ow)
		i->ow = 256;
	else
		ewl_object_set_preferred_w(EWL_OBJECT(i), i->ow);
	if (!i->oh)
		i->oh = 256;
	else
		ewl_object_set_preferred_h(EWL_OBJECT(i), i->oh);

	if (ewl_object_get_preferred_w(EWL_OBJECT(i)))
		ewl_image_scale_to(i, ewl_object_get_preferred_w(EWL_OBJECT(i)),
				ewl_object_get_preferred_h(EWL_OBJECT(i)));
	else
		ewl_image_scale(i, i->sw, i->sh);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void __ewl_image_reparent(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Image      *i;

	i = EWL_IMAGE(w);
	if (!i->image)
		return;

	evas_object_layer_set(i->image, LAYER(w));
}

void __ewl_image_configure(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Image      *i;
	Ewl_Embed      *emb;
	int 		ww, hh;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	i = EWL_IMAGE(w);
	if (!i->image)
		return;

	emb = ewl_embed_find_by_widget(w);

	ww = CURRENT_W(w) - (INSET_LEFT(w) + INSET_RIGHT(w));
	hh = CURRENT_H(w) - (INSET_TOP(w) + INSET_BOTTOM(w));

	/*
	 * Move the image into place based on type.
	 */
	if (i->type != EWL_IMAGE_TYPE_EDJE)
		evas_object_image_fill_set(i->image, 0, 0, i->sw * ww,
				i->sh * hh);

	evas_object_move(i->image, CURRENT_X(w), CURRENT_Y(w));
	evas_object_resize(i->image, ww, hh);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/*
 * Determine the type of the file based on the filename.
 */
Ewl_Image_Type __ewl_image_get_type(const char *i)
{
	int             l;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("i", i, -1);

	l = strlen(i);

	if (l >= 8 && !(strncasecmp((char *) i + l - 8, ".bits.db", 8)))
		return EWL_IMAGE_TYPE_EDJE;

	return EWL_IMAGE_TYPE_NORMAL;
}


void __ewl_image_mouse_down(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Image      *i;
	Ewl_Embed      *emb;
	Ecore_X_Event_Mouse_Button_Down *ev;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	i = EWL_IMAGE(w);
	emb = ewl_embed_find_by_widget(w);
	ev = ev_data;

	if (i->type == EWL_IMAGE_TYPE_EDJE)
		evas_event_feed_mouse_down(emb->evas, ev->button);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void __ewl_image_mouse_up(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Image      *i;
	Ewl_Embed      *emb;
	Ecore_X_Event_Mouse_Button_Up *ev;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	i = EWL_IMAGE(w);
	emb = ewl_embed_find_by_widget(w);
	ev = ev_data;

	if (i->type == EWL_IMAGE_TYPE_EDJE)
		evas_event_feed_mouse_up(emb->evas, ev->button);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void __ewl_image_mouse_move(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Image      *i;
	Ewl_Embed      *emb;
	Ecore_X_Event_Mouse_Move *ev;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	i = EWL_IMAGE(w);
	emb = ewl_embed_find_by_widget(w);
	ev = ev_data;

	if (i->type == EWL_IMAGE_TYPE_EDJE)
		evas_event_feed_mouse_move(emb->evas, ev->x, ev->y);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
