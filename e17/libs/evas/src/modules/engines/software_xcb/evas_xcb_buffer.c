#include "evas_common.h"
#include "evas_engine.h"

static int _xcb_err = 0;

void
evas_software_xcb_x_write_mask_line(Outbuf            *buf,
                                    Xcb_Output_Buffer *xcbob,
				    DATA32            *src,
				    int                w,
				    int                y)
{
   int     x;
   DATA32 *src_ptr;
   DATA8  *dst_ptr;
   int     bpl = 0;

   src_ptr = src;
   dst_ptr = evas_software_xcb_x_output_buffer_data(xcbob, &bpl);
   dst_ptr = dst_ptr + (bpl * y);
   if (buf->priv.x.bit_swap)
     {
	for (x = 0; x < w; x += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[0])) >> 7) << 7) |
	       ((A_VAL(&(src_ptr[1])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[2])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[3])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[4])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[5])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[6])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[7])) >> 7) << 0);
	     src_ptr += 8;
	     dst_ptr++;
	  }
     }
   else
     {
	for (x = 0; x < w; x += 8)
	  {
	     *dst_ptr =
	       ((A_VAL(&(src_ptr[0])) >> 7) << 0) |
	       ((A_VAL(&(src_ptr[1])) >> 7) << 1) |
	       ((A_VAL(&(src_ptr[2])) >> 7) << 2) |
	       ((A_VAL(&(src_ptr[3])) >> 7) << 3) |
	       ((A_VAL(&(src_ptr[4])) >> 7) << 4) |
	       ((A_VAL(&(src_ptr[5])) >> 7) << 5) |
	       ((A_VAL(&(src_ptr[6])) >> 7) << 6) |
	       ((A_VAL(&(src_ptr[7])) >> 7) << 7);
	     src_ptr += 8;
	     dst_ptr++;
	  }
     }
   for (; x < w; x ++)
      {
	 xcb_image_put_pixel(xcbob->image, x, y, A_VAL(src_ptr) >> 7);
	 src_ptr++;
      }
}

int
evas_software_xcb_x_can_do_shm(xcb_connection_t *c)
{
   static xcb_connection_t  *cached_c = NULL;
   static int                cached_result = 0;
   xcb_get_geometry_reply_t *geom;
   xcb_drawable_t            drawable;
   int                       depth;

   if (c == cached_c) return cached_result;
   cached_c = c;
   /* FIXME: get the corect screen */
   drawable = xcb_setup_roots_iterator (xcb_get_setup(c)).data->root;
   geom = xcb_get_geometry_reply (c, xcb_get_geometry_unchecked(c, drawable), 0);
   if(!geom)
     return 0;

   depth = geom->depth;
   free (geom);

   if (xcb_get_extension_data(c, &xcb_shm_id))
     {
	Xcb_Output_Buffer *xcbob;

	xcbob = evas_software_xcb_x_output_buffer_new(c,
						      depth,
						      16,
						      16,
						      2,
						      NULL);
	if (!xcbob)
	  {
	     cached_result = 0;
	     return 0;
	  }
	evas_software_xcb_x_output_buffer_free(xcbob, 1);
	cached_result = 1;
	return 1;
     }
   cached_result = 0;
   return 0;
}

/*
 * FIXME: no error mechanism
 */

/* static void */
/* x_output_tmp_xcb_err(xcb_connection_t *c, XErrorEvent * ev) */
/* { */
/*    _xcb_err = 1; */
/*    return; */
/* } */

Xcb_Output_Buffer *
evas_software_xcb_x_output_buffer_new(xcb_connection_t *c,
				      int               depth,
				      int               w,
				      int               h,
				      int               try_shm,
				      void             *data)
{
   Xcb_Output_Buffer *xcbob;

   xcbob = calloc(1, sizeof(Xcb_Output_Buffer));
   if (!xcbob) return NULL;

   xcbob->connection = c;
   xcbob->image      = NULL;
   xcbob->shm_info   = NULL;

   if (try_shm > 0)
     {
        xcbob->shm_info = malloc(sizeof(xcb_shm_segment_info_t));
        if (xcbob->shm_info)
          {
             xcbob->shm_info->shmseg = xcb_generate_id(c);
             xcbob->image = xcb_image_shm_create(c, depth, XCB_IMAGE_FORMAT_Z_PIXMAP, NULL, w, h);
             if (xcbob->image)
               {
                  xcbob->shm_info->shmid = shmget(IPC_PRIVATE,
                                                  xcbob->image->bytes_per_line *
                                                  xcbob->image->height,
                                                  IPC_CREAT | 0777);
                  if (xcbob->shm_info->shmid >= 0)
                    {
                       xcbob->shm_info->shmaddr = xcbob->image->data =
                         shmat(xcbob->shm_info->shmid, 0, 0);
                       if (xcbob->shm_info->shmaddr != NULL)
                         {
                            /*
                             * FIXME: no error mechanism
                             */

                           /* XErrorHandler ph; */
                           /* EventHandlers eh; */

/*                            xcb_sync(c, 0); */
                           _xcb_err = 0;
                           /* ph = XSetErrorHandler((XErrorHandler) */
                           /* x_output_tmp_x_err); */
                           xcb_shm_attach(c,
                                        xcbob->shm_info->shmseg,
                                        xcbob->shm_info->shmid, 0);
/*                            xcb_sync(c, 0); */
                           /* XSetErrorHandler((XErrorHandler)ph); */
                           if (!_xcb_err)
                             {
                                return xcbob;
                             }
                         }
                       shmdt(xcbob->shm_info->shmaddr);
                       shmctl(xcbob->shm_info->shmid, IPC_RMID, 0);
                    }
                  if (xcbob->image) xcb_image_shm_destroy(xcbob->image);
                  xcbob->image = NULL;
               }
             if (xcbob->shm_info) free(xcbob->shm_info);
             xcbob->shm_info = NULL;
          }
     }

   if (try_shm > 1) return NULL;

   xcbob->image = xcb_image_create(c, depth, XCB_IMAGE_FORMAT_Z_PIXMAP, 0, data, w, h, 32, 0);
   if (!xcbob->image)
     {
	free(xcbob);
	return NULL;
     }

   xcbob->data = data;

   if (!xcbob->image->data)
     {
	xcbob->image->data = malloc(xcbob->image->bytes_per_line * xcbob->image->height);
	if (!xcbob->image->data)
	  {
	     xcb_image_destroy(xcbob->image);
	     free(xcbob);
	     return NULL;
	  }
     }
   return xcbob;
}

void
evas_software_xcb_x_output_buffer_free(Xcb_Output_Buffer *xcbob,
				       int                sync)
{
   if (xcbob->shm_info)
     {
	if (sync)
          {
             xcb_get_input_focus_reply_t *reply;

             reply = xcb_get_input_focus_reply(xcbob->connection,
                                               xcb_get_input_focus_unchecked(xcbob->connection),
                                               NULL);
             free(reply);
          }
	xcb_shm_detach(xcbob->connection, xcbob->shm_info->shmseg);
	xcb_image_shm_destroy(xcbob->image);
	shmdt(xcbob->shm_info->shmaddr);
	shmctl(xcbob->shm_info->shmid, IPC_RMID, 0);
	free(xcbob->shm_info);
     }
   else
     {
	if (xcbob->data) xcbob->image->data = NULL;
	xcb_image_destroy(xcbob->image);
     }
   free(xcbob);
}

void
evas_software_xcb_x_output_buffer_paste(Xcb_Output_Buffer    *xcbob,
					xcb_drawable_t        d,
					xcb_gcontext_t        gc,
					int                   x,
					int                   y,
					int                   sync)
{
   if (xcbob->shm_info)
     {
	xcb_image_shm_put(xcbob->connection, d, gc,
                          xcbob->image, *xcbob->shm_info,
                          0, 0,
                          x, y,
                          xcbob->image->width, xcbob->image->height,
                          0);
	if (sync)
          {
             xcb_get_input_focus_reply_t *reply;

             reply = xcb_get_input_focus_reply(xcbob->connection,
                                               xcb_get_input_focus_unchecked(xcbob->connection),
                                               NULL);
             free(reply);
          }
     }
   else
      xcb_image_put(xcbob->connection,
                    d,
                    gc,
                    xcbob->image,
                    0, 0,
                    x, y,
                    xcbob->image->width, xcbob->image->height);
}

DATA8 *
evas_software_xcb_x_output_buffer_data(Xcb_Output_Buffer *xcbob,
				       int               *bytes_per_line_ret)
{
   if (bytes_per_line_ret) *bytes_per_line_ret = xcbob->image->bytes_per_line;
   return xcbob->image->data;
}

int
evas_software_xcb_x_output_buffer_depth(Xcb_Output_Buffer *xcbob)
{
   return xcbob->image->bits_per_pixel;
}

int
evas_software_xcb_x_output_buffer_byte_order(Xcb_Output_Buffer *xcbob)
{
   return xcbob->image->image_byte_order;
}

int
evas_software_xcb_x_output_buffer_bit_order(Xcb_Output_Buffer *xcbob)
{
   return xcbob->image->bitmap_format_bit_order;
}
