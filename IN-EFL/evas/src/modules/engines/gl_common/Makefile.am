
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_srcdir)/src/lib \
-I$(top_srcdir)/src/lib/include \
@FREETYPE_CFLAGS@ \
@PIXMAN_CFLAGS@ \
@GL_EET_CFLAGS@ \
@EVAS_GENERAL_CFLAGS@ \
@FRIBIDI_CFLAGS@

if BUILD_ENGINE_GL_COMMON

noinst_LTLIBRARIES = libevas_engine_gl_common.la
libevas_engine_gl_common_la_SOURCES  = \
evas_gl_private.h \
evas_gl_common.h \
evas_gl_context.c \
evas_gl_shader.c \
evas_gl_rectangle.c \
evas_gl_texture.c \
evas_gl_image.c \
evas_gl_font.c \
evas_gl_polygon.c \
evas_gl_line.c \
evas_gl_filter.c \
evas_gl_core.c \
evas_gl_api_ext.h \
evas_gl_api_ext_def.h \
evas_gl_core.h \
evas_gl_core_private.h \
evas_gl_api.c \
evas_gl_api_ext.c \
shader/rect_frag.h \
shader/rect_vert.h \
shader/font_frag.h \
shader/font_vert.h \
shader/img_frag.h \
shader/img_vert.h \
shader/img_nomul_frag.h \
shader/img_nomul_vert.h \
shader/img_bgra_frag.h \
shader/img_bgra_vert.h \
shader/img_bgra_nomul_frag.h \
shader/img_bgra_nomul_vert.h \
shader/img_mask_frag.h \
shader/img_mask_vert.h \
shader/nv12_nomul_vert.h \
shader/nv12_vert.h \
shader/nv12_nomul_frag.h \
shader/nv12_frag.h \
shader/yuv_frag.h \
shader/yuv_vert.h \
shader/yuv_nomul_frag.h \
shader/yuv_nomul_vert.h \
shader/tex_frag.h \
shader/tex_vert.h \
shader/tex_nomul_frag.h \
shader/tex_nomul_vert.h \
shader/filter_invert.h \
shader/filter_invert_nomul.h \
shader/filter_invert_bgra.h \
shader/filter_invert_bgra_nomul.h \
shader/filter_sepia.h \
shader/filter_sepia_nomul.h \
shader/filter_sepia_bgra.h \
shader/filter_sepia_bgra_nomul.h \
shader/filter_greyscale.h \
shader/filter_greyscale_nomul.h \
shader/filter_greyscale_bgra.h \
shader/filter_greyscale_bgra_nomul.h \
shader/filter_blur.h \
shader/filter_blur_nomul.h \
shader/filter_blur_bgra.h \
shader/filter_blur_bgra_nomul.h \
shader/yuy2_frag.h \
shader/yuy2_vert.h \
shader/yuy2_nomul_frag.h \
shader/yuy2_nomul_vert.h

libevas_engine_gl_common_la_LIBADD = @EVAS_GENERAL_LIBS@ @GL_EET_LIBS@ @evas_engine_gl_common_libs@ @dlopen_libs@
endif

EXTRA_DIST = \
shader/compile.sh \
shader/make-c-str.sh \
shader/rect_frag.shd \
shader/rect_vert.shd \
shader/font_frag.shd \
shader/font_vert.shd \
shader/img_frag.shd \
shader/img_vert.shd \
shader/img_nomul_frag.shd \
shader/img_nomul_vert.shd \
shader/img_bgra_vert.shd \
shader/img_bgra_frag.shd \
shader/img_bgra_vert.shd \
shader/img_bgra_nomul_frag.shd \
shader/img_bgra_nomul_vert.shd \
shader/img_mask_frag.h \
shader/img_mask_vert.h \
shader/yuv_frag.shd \
shader/yuv_vert.shd \
shader/yuv_nomul_frag.shd \
shader/yuv_nomul_vert.shd \
shader/tex_frag.shd \
shader/tex_vert.shd \
shader/tex_nomul_frag.shd \
shader/tex_nomul_vert.shd \
shader/filter_invert.shd \
shader/filter_invert_nomul.shd \
shader/filter_invert_bgra.shd \
shader/filter_invert_bgra_nomul.shd \
shader/filter_greyscale.shd \
shader/filter_greyscale_nomul.shd \
shader/filter_greyscale_bgra.shd \
shader/filter_greyscale_bgra_nomul.shd \
shader/filter_sepia.shd \
shader/filter_sepia_nomul.shd \
shader/filter_sepia_bgra.shd \
shader/filter_sepia_bgra_nomul.shd \
shader/filter_blur.shd \
shader/filter_blur_nomul.shd \
shader/filter_blur_bgra.shd \
shader/filter_blur_bgra_nomul.shd \
shader/nv12_frag.h \
shader/nv12_frag.shd \
shader/nv12_nomul_frag.h \
shader/nv12_nomul_frag.shd \
shader/nv12_nomul_vert.h \
shader/nv12_nomul_vert.shd \
shader/nv12_vert.h \
shader/nv12_vert.shd \
shader/yuy2_frag.h \
shader/yuy2_frag.shd \
shader/yuy2_nomul_frag.h \
shader/yuy2_nomul_frag.shd \
shader/yuy2_nomul_vert.h \
shader/yuy2_nomul_vert.shd \
shader/yuy2_vert.h \
shader/yuy2_vert.shd


