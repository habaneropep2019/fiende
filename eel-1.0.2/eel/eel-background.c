/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   eel-background.c: Object for the background of a widget.
 
   Copyright (C) 2000 Eazel, Inc.
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Author: Darin Adler <darin@eazel.com>
*/

#include <config.h>
#include <ctype.h>

#include "eel-background.h"

#include <math.h>

#include <stdio.h>
#include "eel-glib-extensions.h"

#include <gtk/gtksignal.h>
#include "eel-gdk-extensions.h"
#include "eel-gdk-pixbuf-extensions.h"
#include "eel-gnome-extensions.h"
#include "eel-background-canvas-group.h"
#include "eel-lib-self-check-functions.h"
#include "eel-gtk-macros.h"
#include "eel-string.h"

#include <libgnomeui/gnome-canvas.h>
#include <libgnomeui/gnome-canvas-util.h>

#include <libart_lgpl/art_rgb.h>

static void     eel_background_initialize_class          (gpointer       klass);
static void     eel_background_initialize                (gpointer       object,
							  gpointer       klass);
static void     eel_background_destroy                   (GtkObject     *object);
static void     eel_background_draw_flat_box             (GtkStyle      *style,
							  GdkWindow     *window,
							  GtkStateType   state_type,
							  GtkShadowType  shadow_type,
							  GdkRectangle  *area,
							  GtkWidget     *widget,
							  char          *detail,
							  int            x,
							  int            y,
							  int            width,
							  int            height);
static void     eel_background_start_loading_image       (EelBackground *background,
							  gboolean       emit_appearance_change);
static gboolean eel_background_is_image_load_in_progress (EelBackground *background);

EEL_DEFINE_CLASS_BOILERPLATE (EelBackground, eel_background, GTK_TYPE_OBJECT)

enum {
	APPEARANCE_CHANGED,
	SETTINGS_CHANGED,
	IMAGE_LOADING_DONE,
	RESET,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

struct EelBackgroundDetails {
	char *color;
	
	int gradient_num_pixels;
	guchar *gradient_buffer;
	gboolean gradient_is_horizontal;

	gboolean is_solid_color;
	GdkColor solid_color;
	
	char *image_uri;
	GdkPixbuf *image;
	int image_width_unscaled;
	int image_height_unscaled;
	EelPixbufLoadHandle *load_image_handle;
	gboolean emit_after_load;
	EelBackgroundImagePlacement image_placement;

	/* The image_rect is the area (canvas relative) the image will cover.
	 * Note: image_rect_width/height are not always the same as the image's
	 * width and height - e.g. if the image is tiled the image rect covers
	 * the whole background.
	 */
	int image_rect_x;
	int image_rect_y;
	int image_rect_width;
	int image_rect_height;
};

static void
eel_background_initialize_class (gpointer klass)
{
	GtkObjectClass *object_class;
	EelBackgroundClass *background_class;

	object_class = GTK_OBJECT_CLASS (klass);
	background_class = EEL_BACKGROUND_CLASS (klass);

	signals[APPEARANCE_CHANGED] =
		gtk_signal_new ("appearance_changed",
				GTK_RUN_LAST | GTK_RUN_NO_RECURSE,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelBackgroundClass,
						   appearance_changed),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE,
				0);
	signals[SETTINGS_CHANGED] =
		gtk_signal_new ("settings_changed",
				GTK_RUN_LAST | GTK_RUN_NO_RECURSE,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelBackgroundClass,
						   settings_changed),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE,
				0);
	signals[IMAGE_LOADING_DONE] =
		gtk_signal_new ("image_loading_done",
				GTK_RUN_LAST | GTK_RUN_NO_RECURSE,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelBackgroundClass,
						   image_loading_done),
				gtk_marshal_NONE__BOOL,
				GTK_TYPE_NONE,
				1,
				GTK_TYPE_BOOL);
	signals[RESET] =
		gtk_signal_new ("reset",
				GTK_RUN_LAST | GTK_RUN_NO_RECURSE,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelBackgroundClass,
						   reset),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE,
				0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	object_class->destroy = eel_background_destroy;
}

static void
eel_background_initialize (gpointer object, gpointer klass)
{
	EelBackground *background;

	background = EEL_BACKGROUND(object);

	background->details = g_new0 (EelBackgroundDetails, 1);
}

/* The safe way to clear an image from a background is:
 * 		eel_background_set_image_uri (NULL);
 * This fn is a private utility - it does NOT clear
 * the details->image_uri setting.
 */
static void
eel_background_remove_current_image (EelBackground *background)
{
	if (background->details->image != NULL) {
		gdk_pixbuf_unref (background->details->image);
		background->details->image = NULL;
	}
}

static void
eel_background_destroy (GtkObject *object)
{
	EelBackground *background;

	background = EEL_BACKGROUND (object);

	eel_cancel_gdk_pixbuf_load (background->details->load_image_handle);
	background->details->load_image_handle = NULL;

	g_free (background->details->color);
	g_free (background->details->gradient_buffer);
	g_free (background->details->image_uri);
	eel_background_remove_current_image (background);
	g_free (background->details);

	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

EelBackgroundImagePlacement
eel_background_get_image_placement (EelBackground *background)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), EEL_BACKGROUND_TILED);

	return background->details->image_placement;
}

static gboolean
eel_background_set_image_placement_no_emit (EelBackground *background,
						 EelBackgroundImagePlacement new_placement)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);

	if (new_placement != background->details->image_placement) {

		if (eel_background_is_image_load_in_progress (background)) {
			/* We try to be smart and keep using the current image for updates
			 * while a new image is loading. However, changing the placement
			 * foils these plans.
			 */
			eel_background_remove_current_image (background);
		}

		background->details->image_placement = new_placement;
		return TRUE;
	} else {
		return FALSE;
	}
}

void
eel_background_set_image_placement (EelBackground *background,
					 EelBackgroundImagePlacement new_placement)
{
	if (eel_background_set_image_placement_no_emit (background, new_placement)) {
		gtk_signal_emit (GTK_OBJECT (background),
			 signals[SETTINGS_CHANGED]);
		gtk_signal_emit (GTK_OBJECT (background),
			 signals[APPEARANCE_CHANGED]);
	}
}

EelBackground *
eel_background_new (void)
{
	return EEL_BACKGROUND (gtk_object_new (EEL_TYPE_BACKGROUND, NULL));
}
 
static void
reset_cached_color_info (EelBackground *background)
{
	background->details->gradient_num_pixels = 0;
	
	background->details->is_solid_color = !eel_gradient_is_gradient (background->details->color);
	
	if (background->details->is_solid_color) {
		g_free (background->details->gradient_buffer);
		background->details->gradient_buffer = NULL;
		eel_gdk_color_parse_with_white_default (background->details->color, &background->details->solid_color);
	} else {
		/* If color is still a gradient, don't g_free the buffer, eel_background_ensure_gradient_buffered
		 * uses g_realloc to try to reuse it.
		 */
		background->details->gradient_is_horizontal = eel_gradient_is_horizontal (background->details->color);
	}
}

static void
eel_background_ensure_gradient_buffered (EelBackground *background, int dest_width, int dest_height)
{
	int num_pixels;

	guchar *buff_ptr;
	guchar *buff_limit;

	GdkColor cur_color;

	char* color_spec;
	const char* spec_ptr;

	if (background->details->is_solid_color) {
		return;
	}

	num_pixels = background->details->gradient_is_horizontal ? dest_width : dest_height;

	if (background->details->gradient_num_pixels == num_pixels) {
		return;
	}

	background->details->gradient_num_pixels = num_pixels;
	background->details->gradient_buffer = g_realloc (background->details->gradient_buffer, num_pixels * 3);
	
	buff_ptr   = background->details->gradient_buffer;
	buff_limit = background->details->gradient_buffer + num_pixels * 3;

	spec_ptr = background->details->color;
	
	color_spec = eel_gradient_parse_one_color_spec (spec_ptr, NULL, &spec_ptr);
	eel_gdk_color_parse_with_white_default (color_spec, &cur_color);
	g_free (color_spec);

	while (spec_ptr != NULL && buff_ptr < buff_limit) {
		int percent;
		int fill_pos;
		int fill_width;
		int dr, dg, db;
		guchar *fill_limit;
		GdkColor new_color;
	
		color_spec = eel_gradient_parse_one_color_spec (spec_ptr, &percent, &spec_ptr);
		eel_gdk_color_parse_with_white_default (color_spec, &new_color);
		g_free (color_spec);

		dr = new_color.red   - cur_color.red;
		dg = new_color.green - cur_color.green;
		db = new_color.blue  - cur_color.blue;

		fill_pos   = 1;
		fill_limit = MIN (background->details->gradient_buffer + 3 * ((num_pixels * percent) / 100), buff_limit);
		fill_width = (fill_limit - buff_ptr) / 3;
		
		while (buff_ptr < fill_limit) {
			*buff_ptr++ = (cur_color.red   + (dr * fill_pos) / fill_width) >> 8;
			*buff_ptr++ = (cur_color.green + (dg * fill_pos) / fill_width) >> 8;
			*buff_ptr++ = (cur_color.blue  + (db * fill_pos) / fill_width) >> 8;
			++fill_pos;
		}
		cur_color = new_color;
	}

	/* fill in the remainder */
	art_rgb_fill_run (buff_ptr, cur_color.red, cur_color.green, cur_color.blue, (buff_limit - buff_ptr) / 3);	
}

static void
canvas_gradient_helper_v (const GnomeCanvasBuf *buf, const art_u8 *gradient_buff)
{
	int width  = buf->rect.x1 - buf->rect.x0;
	int height = buf->rect.y1 - buf->rect.y0;

	art_u8 *dst       = buf->buf;
	art_u8 *dst_limit = buf->buf + height * buf->buf_rowstride;

	gradient_buff += buf->rect.y0 * 3;
	
	while (dst < dst_limit) {
		art_u8 r = *gradient_buff++;
		art_u8 g = *gradient_buff++;
		art_u8 b = *gradient_buff++;
 		art_rgb_fill_run (dst, r, g, b, width);
		dst += buf->buf_rowstride;
	}
}

static void
canvas_gradient_helper_h (const GnomeCanvasBuf *buf, const art_u8 *gradient_buff)
{
	int width  = buf->rect.x1 - buf->rect.x0;
	int height = buf->rect.y1 - buf->rect.y0;

	art_u8 *dst       = buf->buf;
	art_u8 *dst_limit = buf->buf + height * buf->buf_rowstride;

	int copy_bytes_per_row = width * 3;

	gradient_buff += buf->rect.x0 * 3;
	
	while (dst < dst_limit) {
 		memcpy (dst, gradient_buff, copy_bytes_per_row);
		dst += buf->buf_rowstride;
	}
}

static void
fill_canvas_from_gradient_buffer (const GnomeCanvasBuf *buf, const EelBackground *background)
{
	g_return_if_fail (background->details->gradient_buffer != NULL);

	/* FIXME bugzilla.eazel.com 4876: This hack is needed till we fix background
	 * scolling.
	 *
	 * I.e. currently you can scroll off the end of the gradient - and we
	 * handle this by pegging it the the last rgb value.
	 *
	 * It might be needed permanently after depending on how this is fixed.
	 * If we tie gradients to the boundry of icon placement (as opposed to
	 * window size) then when dragging an icon you might scroll off the
	 * end of the gradient - which will get recaluated after the drop.
	 */
	if (background->details->gradient_is_horizontal) {
		if (buf->rect.x1 > background->details->gradient_num_pixels) {
			art_u8 *rgb888 = background->details->gradient_buffer + (background->details->gradient_num_pixels - 1) * 3;
			GnomeCanvasBuf gradient = *buf;
			GnomeCanvasBuf overflow = *buf;
			gradient.rect.x1 =  gradient.rect.x0 < background->details->gradient_num_pixels ? background->details->gradient_num_pixels : gradient.rect.x0;
			overflow.buf += (gradient.rect.x1 - gradient.rect.x0) * 3;
			overflow.rect.x0 = gradient.rect.x1;
			eel_gnome_canvas_fill_rgb (&overflow, rgb888[0], rgb888[1], rgb888[2]);
			canvas_gradient_helper_h (&gradient, background->details->gradient_buffer);
			return;
		}
	} else {
		if (buf->rect.y1 > background->details->gradient_num_pixels) {
			art_u8 *rgb888 = background->details->gradient_buffer + (background->details->gradient_num_pixels - 1) * 3;
			GnomeCanvasBuf gradient = *buf;
			GnomeCanvasBuf overflow = *buf;
			gradient.rect.y1 = gradient.rect.y0 < background->details->gradient_num_pixels ? background->details->gradient_num_pixels : gradient.rect.y0;
			overflow.buf += (gradient.rect.y1 - gradient.rect.y0) * gradient.buf_rowstride;
			overflow.rect.y0 = gradient.rect.y1;
			eel_gnome_canvas_fill_rgb (&overflow, rgb888[0], rgb888[1], rgb888[2]);
			canvas_gradient_helper_v (&gradient, background->details->gradient_buffer);
			return;
		}
	}

	(background->details->gradient_is_horizontal ? canvas_gradient_helper_h : canvas_gradient_helper_v) (buf, background->details->gradient_buffer);
}

/* Initializes a pseudo-canvas buf so canvas drawing routines can be used to draw into a pixbuf.
 */
static void
canvas_buf_from_pixbuf (GnomeCanvasBuf* buf, GdkPixbuf *pixbuf, int x, int y, int width, int height)
{
	buf->buf =  gdk_pixbuf_get_pixels (pixbuf);
	buf->buf_rowstride =  gdk_pixbuf_get_rowstride (pixbuf);
	buf->rect.x0 = x;
	buf->rect.y0 = y;
	buf->rect.x1 = x + width;
	buf->rect.y1 = y + height;
	buf->bg_color = 0xFFFFFFFF;
	buf->is_bg = TRUE;
	buf->is_buf = FALSE;
}

static gboolean
eel_background_image_totally_obscures (EelBackground *background)
{
	if (background->details->image == NULL || gdk_pixbuf_get_has_alpha (background->details->image)) {
		return FALSE;
	}

	switch (background->details->image_placement) {
	case EEL_BACKGROUND_TILED:
	case EEL_BACKGROUND_SCALED:
		return TRUE;
	default:
		g_assert_not_reached ();
		/* fall through */
	case EEL_BACKGROUND_CENTERED:
	case EEL_BACKGROUND_SCALED_ASPECT:
		/* It's possible that the image totally obscures in this case, but we don't
		 * have enough info to know. So we guess conservatively.
		 */
		return FALSE;
	}
}

static void
eel_background_ensure_image_scaled (EelBackground *background, int dest_width, int dest_height)
{
	if (background->details->image == NULL) {
		background->details->image_rect_x = 0;
		background->details->image_rect_y = 0;
		background->details->image_rect_width = 0;
		background->details->image_rect_height = 0;
	} else if (!eel_background_is_image_load_in_progress (background)){
		int image_width;
		int image_height;
		int fit_width;
		int fit_height;
		gboolean cur_scaled;
		gboolean reload_image;
		GdkPixbuf *scaled_pixbuf;

		image_width = gdk_pixbuf_get_width (background->details->image);
		image_height = gdk_pixbuf_get_height (background->details->image);
		cur_scaled = image_width != background->details->image_width_unscaled ||
			     image_height != background->details->image_height_unscaled;
		reload_image = FALSE;
		scaled_pixbuf = NULL;
	
		switch (background->details->image_placement) {
		case EEL_BACKGROUND_TILED:
		case EEL_BACKGROUND_CENTERED:
			reload_image = cur_scaled;
			break;
		case EEL_BACKGROUND_SCALED:
			if (image_width != dest_width || image_height != dest_height) {
				if (cur_scaled) {
					reload_image = TRUE;
				} else {
					scaled_pixbuf = gdk_pixbuf_scale_simple (background->details->image, dest_width, dest_height, GDK_INTERP_BILINEAR);
					gdk_pixbuf_unref (background->details->image);
					background->details->image = scaled_pixbuf;
					image_width = gdk_pixbuf_get_width (scaled_pixbuf);
					image_height = gdk_pixbuf_get_height (scaled_pixbuf);
				}
			}
			break;
		case EEL_BACKGROUND_SCALED_ASPECT:
			eel_gdk_scale_to_fit_factor (background->details->image_width_unscaled,
							  background->details->image_height_unscaled,
							  dest_width, dest_height,
							  &fit_width, &fit_height);
			if (image_width != fit_width || image_height != fit_height) {
				if (cur_scaled) {
					reload_image = TRUE;
				} else {
					scaled_pixbuf = eel_gdk_pixbuf_scale_to_fit (background->details->image, dest_width, dest_height);
					gdk_pixbuf_unref (background->details->image);
					background->details->image = scaled_pixbuf;
					image_width = gdk_pixbuf_get_width (scaled_pixbuf);
					image_height = gdk_pixbuf_get_height (scaled_pixbuf);
				}
			}
			break;
		}

		if (reload_image) {
			gdk_pixbuf_unref (background->details->image);
			background->details->image = NULL;
			eel_background_start_loading_image (background, TRUE);
			background->details->image_rect_x = 0;
			background->details->image_rect_y = 0;
			background->details->image_rect_width = 0;
			background->details->image_rect_height = 0;
		} else if (background->details->image_placement == EEL_BACKGROUND_TILED) {
			background->details->image_rect_x = 0;
			background->details->image_rect_y = 0;
			background->details->image_rect_width = dest_width;
			background->details->image_rect_height = dest_height;
		} else {
			background->details->image_rect_x = (dest_width - image_width) / 2;
			background->details->image_rect_y = (dest_height - image_height) / 2;
			background->details->image_rect_width = image_width;
			background->details->image_rect_height = image_height;
		}
	}
}

void
eel_background_pre_draw (EelBackground *background, int entire_width, int entire_height)
{
	eel_background_ensure_image_scaled (background, entire_width, entire_height);
	eel_background_ensure_gradient_buffered (background, entire_width, entire_height);
}

void
eel_background_draw (EelBackground *background,
		     GdkDrawable *drawable, GdkGC *gc,
		     int src_x, int src_y,
		     int dest_x, int dest_y,
		     int dest_width, int dest_height)
{
	int x, y;
	int x_canvas, y_canvas;
	int width, height;
	
	GnomeCanvasBuf buffer;
	GdkPixbuf *pixbuf;

	/* Non-aa background drawing is done by faking up a GnomeCanvasBuf
	 * and passing it to the aa code.
	 *
	 * The width and height were chosen to match those used by gnome-canvas
	 * (IMAGE_WIDTH_AA & IMAGE_HEIGHT_AA  in gnome-libs/libgnomeui/gnome-canvas.c)
	 * They're not required to match - so this could be changed if necessary.
	 */
	static const int PIXBUF_WIDTH = 256;
	static const int PIXBUF_HEIGHT = 64;

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, PIXBUF_WIDTH, PIXBUF_HEIGHT);

	/* x & y are relative to the drawable
	 */
	for (y = 0; y < dest_height; y += PIXBUF_HEIGHT) {
		for (x = 0; x < dest_width; x += PIXBUF_WIDTH) {

			width = MIN (dest_width - x, PIXBUF_WIDTH);
			height = MIN (dest_height - y, PIXBUF_HEIGHT);

			x_canvas = src_x + x;
			y_canvas = src_y + y;

			canvas_buf_from_pixbuf (&buffer, pixbuf, x_canvas, y_canvas, width, height);
			eel_background_draw_aa (background, &buffer);
			gdk_pixbuf_render_to_drawable (pixbuf, drawable, gc,
						       0, 0,
						       dest_x + x, dest_y + y,
						       width, height,
						       GDK_RGB_DITHER_MAX, dest_x + x, dest_y + y);
		}
	}
	
	gdk_pixbuf_unref (pixbuf);
}

void
eel_background_draw_to_drawable (EelBackground *background,
				      GdkDrawable *drawable, GdkGC *gc,
				      int drawable_x, int drawable_y,
				      int drawable_width, int drawable_height,
				      int entire_width, int entire_height)
{
	eel_background_pre_draw (background, entire_width, entire_height);
	eel_background_draw (background, drawable, gc,
			     drawable_x, drawable_y,
			     drawable_x, drawable_y,
			     drawable_width, drawable_height);
}

void
eel_background_draw_to_pixbuf (EelBackground *background,
				    GdkPixbuf *pixbuf,
				    int pixbuf_x,
				    int pixbuf_y,
				    int pixbuf_width,
				    int pixbuf_height,
				    int entire_width,
				    int entire_height)
{
	GnomeCanvasBuf fake_buffer;

	g_return_if_fail (background != NULL);
	g_return_if_fail (pixbuf != NULL);

	canvas_buf_from_pixbuf (&fake_buffer, pixbuf, pixbuf_x, pixbuf_y, pixbuf_width, pixbuf_height);

	eel_background_draw_to_canvas (background,
					    &fake_buffer,
					    entire_width,
					    entire_height);
}

/* fill the canvas buffer with a tiled pixbuf */
static void
draw_pixbuf_tiled_aa (GdkPixbuf *pixbuf, GnomeCanvasBuf *buffer)
{
	int x, y;
	int start_x, start_y;
	int tile_width, tile_height;
	
	tile_width = gdk_pixbuf_get_width (pixbuf);
	tile_height = gdk_pixbuf_get_height (pixbuf);
	
	start_x = buffer->rect.x0 - (buffer->rect.x0 % tile_width);
	start_y = buffer->rect.y0 - (buffer->rect.y0 % tile_height);

	for (y = start_y; y < buffer->rect.y1; y += tile_height) {
		for (x = start_x; x < buffer->rect.x1; x += tile_width) {
			eel_gnome_canvas_draw_pixbuf (buffer, pixbuf, x, y);
		}
	}
}

/* draw the background on the anti-aliased canvas */
void
eel_background_draw_aa (EelBackground *background, GnomeCanvasBuf *buffer)
{	
	g_return_if_fail (EEL_IS_BACKGROUND (background));

	/* If the image has alpha - we always draw the gradient behind it.
	 * In principle, we could do better by having already drawn gradient behind
	 * the scaled image. However, this would add a significant amount of
	 * complexity to our image scaling/caching logic. I.e. it would tie the
	 * scaled image to a location on the screen because it holds a gradient.
	 * This is especially problematic for tiled images with alpha.
	 */
	if (!background->details->image ||
	     gdk_pixbuf_get_has_alpha (background->details->image) ||
	     buffer->rect.x0  < background->details->image_rect_x ||
	     buffer->rect.y0  < background->details->image_rect_y ||
	     buffer->rect.x1  > (background->details->image_rect_x + background->details->image_rect_width) ||
	     buffer->rect.y1  > (background->details->image_rect_y + background->details->image_rect_height)) {
		if (background->details->is_solid_color) {
			eel_gnome_canvas_fill_rgb (buffer,
							background->details->solid_color.red >> 8,
							background->details->solid_color.green >> 8,
							background->details->solid_color.blue >> 8);
		} else {
			fill_canvas_from_gradient_buffer (buffer, background);
		}
	}

	if (background->details->image != NULL) {
		switch (background->details->image_placement) {
		case EEL_BACKGROUND_TILED:
			draw_pixbuf_tiled_aa (background->details->image, buffer);
			break;
		default:
			g_assert_not_reached ();
			/* fall through */
		case EEL_BACKGROUND_CENTERED:
		case EEL_BACKGROUND_SCALED:
		case EEL_BACKGROUND_SCALED_ASPECT:
			/* Since the image has already been scaled, all these cases
			 * can be treated identically.
			 */
			eel_gnome_canvas_draw_pixbuf (buffer,
							   background->details->image,
							   background->details->image_rect_x,
							   background->details->image_rect_y);
			break;
		}
	}
					
	buffer->is_bg  = FALSE;
	buffer->is_buf = TRUE;
}

void
eel_background_draw_to_canvas (EelBackground *background,
				    GnomeCanvasBuf *buffer,
				    int entire_width,
				    int entire_height)
{
	eel_background_pre_draw (background, entire_width, entire_height);
	eel_background_draw_aa (background, buffer);
}

char *
eel_background_get_color (EelBackground *background)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), NULL);

	return g_strdup (background->details->color);
}

char *
eel_background_get_image_uri (EelBackground *background)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), NULL);

	return g_strdup (background->details->image_uri);
}

static gboolean
eel_background_set_color_no_emit (EelBackground *background,
			       const char *color)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);

	if (eel_strcmp (background->details->color, color) == 0) {
		return FALSE;
	}
	g_free (background->details->color);
	background->details->color = g_strdup (color);
	reset_cached_color_info (background);

	return TRUE;
}

void
eel_background_set_color (EelBackground *background,
			       const char *color)
{
	if (eel_background_set_color_no_emit (background, color)) {
		gtk_signal_emit (GTK_OBJECT (background), signals[SETTINGS_CHANGED]);
		if (!eel_background_image_totally_obscures (background)) {
			gtk_signal_emit (GTK_OBJECT (background), signals[APPEARANCE_CHANGED]);
		}
	}
}

static void
eel_background_load_image_callback (GnomeVFSResult error,
					 GdkPixbuf *pixbuf,
					 gpointer callback_data)
{
	EelBackground *background;

	background = EEL_BACKGROUND (callback_data);

	background->details->load_image_handle = NULL;

	eel_background_remove_current_image (background);

	/* Just ignore errors. */
	if (pixbuf != NULL) {
		gdk_pixbuf_ref (pixbuf);
		background->details->image = pixbuf;
		background->details->image_width_unscaled = gdk_pixbuf_get_width (pixbuf);
		background->details->image_height_unscaled = gdk_pixbuf_get_height (pixbuf);
	}

	gtk_signal_emit (GTK_OBJECT (background), signals[IMAGE_LOADING_DONE], pixbuf != NULL || background->details->image_uri == NULL);

	if (background->details->emit_after_load) {
		gtk_signal_emit (GTK_OBJECT (background), signals[APPEARANCE_CHANGED]);
	}
}

static gboolean
eel_background_is_image_load_in_progress (EelBackground *background)
{
	return background->details->load_image_handle != NULL;
}

static void
eel_background_cancel_loading_image (EelBackground *background)
{
	if (eel_background_is_image_load_in_progress (background)) {
		eel_cancel_gdk_pixbuf_load (background->details->load_image_handle);
		background->details->load_image_handle = NULL;
		gtk_signal_emit (GTK_OBJECT (background), signals[IMAGE_LOADING_DONE], FALSE);
	}
}

static void
eel_background_start_loading_image (EelBackground *background, gboolean emit_appearance_change)
{
	background->details->emit_after_load = emit_appearance_change;

	if (background->details->image_uri != NULL) {
		background->details->load_image_handle = eel_gdk_pixbuf_load_async (background->details->image_uri,
											 eel_background_load_image_callback,
											 background);
	} else {
		eel_background_load_image_callback (0, NULL, background);
	}
}

static gboolean
eel_background_set_image_uri_helper (EelBackground *background,
					const char *image_uri,
					gboolean emit_setting_change,
					gboolean emit_appearance_change)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);

	if (eel_strcmp (background->details->image_uri, image_uri) == 0) {
		return FALSE;
	}

	eel_background_cancel_loading_image (background);
	
	g_free (background->details->image_uri);
	background->details->image_uri = g_strdup (image_uri);

	/* We do not get rid of the current image here. This gets done after the new
	 * image loads - in eel_background_load_image_callback. This way the
	 * current image can be used if an update is needed before the load completes.
	 */
	
	eel_background_start_loading_image (background, emit_appearance_change);

	if (emit_setting_change) {
		gtk_signal_emit (GTK_OBJECT (background), signals[SETTINGS_CHANGED]);
	}

	return TRUE;
}

void
eel_background_set_image_uri (EelBackground *background, const char *image_uri)
{
	eel_background_set_image_uri_helper (background, image_uri, TRUE, TRUE);
}

static void
set_image_and_color_image_loading_done_callback (EelBackground *background, gboolean successful_load, char *color)
{
	gtk_signal_disconnect_by_func (GTK_OBJECT (background), GTK_SIGNAL_FUNC (set_image_and_color_image_loading_done_callback), (gpointer) color);

	eel_background_set_color_no_emit (background, color);

	g_free (color);
	
	/* We always emit , even if the color didn't change, because the image change
	 * relies on us doing it here.
	 */
	gtk_signal_emit (GTK_OBJECT (background), signals[SETTINGS_CHANGED]);
	gtk_signal_emit (GTK_OBJECT (background), signals[APPEARANCE_CHANGED]);
}

/* Use this fn to set both the image and color and avoid flash. The color isn't
 * changed till after the image is done loading, that way if an update occurs
 * before then, it will use the old color and image.
 */
static void
eel_background_set_image_uri_and_color (EelBackground *background, const char *image_uri, const char *color)
{
	char *color_copy;

	if (eel_strcmp (background->details->color, color) == 0 &&
	    eel_strcmp (background->details->image_uri, image_uri) == 0) {
		return;
	}

	color_copy = g_strdup (color);

	gtk_signal_connect (GTK_OBJECT (background),
			    "image_loading_done",
			    GTK_SIGNAL_FUNC (set_image_and_color_image_loading_done_callback),
			    (gpointer) color_copy);
			    
	/* set_image_and_color_image_loading_done_callback must always be called
	 * because we rely on it to:
	 *  - disconnect the image_loading_done signal handler
	 *  - emit SETTINGS_CHANGED & APPEARANCE_CHANGED
	 *  - free color_copy
	 *  - prevent the common cold
	 */
	     
	/* We use eel_background_set_image_uri_helper because its
	 * return value (if false) tells us whether or not we need to
	 * call set_image_and_color_image_loading_done_callback ourselves.
	 */
	if (!eel_background_set_image_uri_helper (background, image_uri, FALSE, FALSE)) {
		set_image_and_color_image_loading_done_callback (background, TRUE, color_copy);
	}
}

void
eel_background_receive_dropped_background_image (EelBackground *background,
						      const char *image_uri)
{
	/* Currently, we only support tiled images. So we set the placement.
	 * We rely on eel_background_set_image_uri_and_color to emit
	 * the SETTINGS_CHANGED & APPEARANCE_CHANGE signals.
	 */
	eel_background_set_image_placement_no_emit (background, EEL_BACKGROUND_TILED);

	eel_background_set_image_uri_and_color (background, image_uri, NULL);
}

static GtkStyleClass *
eel_gtk_style_get_default_class (void)
{
	static GtkStyleClass *default_class;
	GtkStyle *style;
	
	if (default_class == NULL) {
		style = gtk_style_new ();
		default_class = style->klass;
		gtk_style_unref (style);
	}

	return default_class;
}

static void
eel_gdk_window_update_sizes (GdkWindow *window, int *width, int *height)
{
	g_return_if_fail (window != NULL);
	g_return_if_fail (width != NULL);
	g_return_if_fail (height != NULL);

	if (*width == -1 && *height == -1) {
		gdk_window_get_size (window, width, height);
	} else if (*width == -1) {
		gdk_window_get_size (window, width, NULL);
	} else if (*height == -1) {
		gdk_window_get_size (window, NULL, height);
	}
}

static void
eel_background_draw_flat_box (GtkStyle *style,
				   GdkWindow *window,
				   GtkStateType state_type,
				   GtkShadowType shadow_type,
				   GdkRectangle *area,
				   GtkWidget *widget,
				   char *detail,
				   int x,
				   int y,
				   int width,
				   int height)
{
	gboolean call_parent;
	EelBackground *background;
	GdkGC *gc;

	call_parent = TRUE;

	background = NULL;
	if (state_type == GTK_STATE_NORMAL) {
		background = eel_get_widget_background (widget);
		if (background != NULL) {
			call_parent = FALSE;
		}
	}

	if (call_parent) {
		(* eel_gtk_style_get_default_class()->draw_flat_box)
			(style, window, state_type, shadow_type, area, widget,
			 detail, x, y, width, height);
		return;
	}

    	gc = style->bg_gc[state_type];
	if (area)
		gdk_gc_set_clip_rectangle (gc, area);

	eel_gdk_window_update_sizes (window, &width, &height);	

	if (area)
		eel_background_draw_to_drawable (background, window, gc,
						 area->x, area->y,
						 area->width, area->height,
						 widget->allocation.width, widget->allocation.height);
	else
		eel_background_draw_to_drawable (background, window, gc,
						 0, 0,
						 width, height,
						 widget->allocation.width, widget->allocation.height);
	if (area)
		gdk_gc_set_clip_rectangle (gc, NULL);
}

static GtkStyleClass *
eel_background_get_gtk_style_class (void)
{
	static GtkStyleClass *klass;

	if (klass == NULL) {
		static GtkStyleClass klass_storage;

		klass = &klass_storage;
		*klass = *eel_gtk_style_get_default_class ();

		klass->draw_flat_box = eel_background_draw_flat_box;
	}

	return klass;
}

static void
eel_background_set_widget_style (EelBackground *background,
				      GtkWidget *widget)
{
	GtkStyle *style;
	char *start_color_spec;
	GdkColor color;
	
	g_return_if_fail (EEL_IS_BACKGROUND (background));
	g_return_if_fail (GTK_IS_WIDGET (widget));
	
	style = gtk_widget_get_style (widget);
	
	/* Make a copy of the style. */
	style = gtk_style_copy (style);
	
	/* Give it the special class that allows it to draw gradients. */
	style->klass = eel_background_get_gtk_style_class ();

	/* Set up the colors in the style. */
	start_color_spec = eel_gradient_get_start_color_spec (background->details->color);
	eel_gdk_color_parse_with_white_default (start_color_spec, &color);
	g_free (start_color_spec);
	
	style->bg[GTK_STATE_NORMAL] = color;
	style->base[GTK_STATE_NORMAL] = color;
	style->bg[GTK_STATE_ACTIVE] = color;
	style->base[GTK_STATE_ACTIVE] = color;
	
	/* Put the style in the widget. */
	gtk_widget_set_style (widget, style);
	gtk_style_unref (style);
}

/**
 * eel_background_is_set:
 * 
 * Check whether the background's color or image has been set.
 */
gboolean
eel_background_is_set (EelBackground *background)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);

	return background->details->color != NULL
		|| background->details->image_uri != NULL;
}

/* Returns false if the image is still loading, true
 * if it's done loading or there is no image.
 */
gboolean
eel_background_is_loaded (EelBackground *background)
{
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);
	
	return background->details->image_uri == NULL ||
		   (!eel_background_is_image_load_in_progress (background) && background->details->image != NULL);
}

/**
 * eel_background_reset:
 *
 * Emit the reset signal to forget any color or image that has been
 * set previously.
 */
void
eel_background_reset (EelBackground *background)
{
	g_return_if_fail (EEL_IS_BACKGROUND (background));

	gtk_signal_emit (GTK_OBJECT (background), signals[RESET]);
}

static void
eel_background_set_up_canvas (GtkWidget *widget)
{
	if (GNOME_IS_CANVAS (widget)) {
		eel_background_canvas_group_supplant_root_class (GNOME_CANVAS (widget));
	}
}

static void
eel_widget_background_changed (GtkWidget *widget, EelBackground *background)
{
	eel_background_set_widget_style (background, widget);
	eel_background_set_up_canvas (widget);

	gtk_widget_queue_clear (widget);
}

/* Gets the background attached to a widget.

   If the widget doesn't already have a EelBackground object,
   this will create one. To change the widget's background, you can
   just call eel_background methods on the widget.

   Later, we might want a call to find out if we already have a background,
   or a way to share the same background among multiple widgets; both would
   be straightforward.
*/
EelBackground *
eel_get_widget_background (GtkWidget *widget)
{
	gpointer data;
	EelBackground *background;

	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	/* Check for an existing background. */
	data = gtk_object_get_data (GTK_OBJECT (widget), "eel_background");
	if (data != NULL) {
		g_assert (EEL_IS_BACKGROUND (data));
		return data;
	}

	/* Store the background in the widget's data. */
	background = eel_background_new ();
	gtk_object_ref (GTK_OBJECT (background));
	gtk_object_sink (GTK_OBJECT (background));
	gtk_object_set_data_full (GTK_OBJECT (widget), "eel_background",
				  background, (GtkDestroyNotify) gtk_object_unref);

	/* Arrange to get the signal whenever the background changes. */
	gtk_signal_connect_object_while_alive (GTK_OBJECT (background),
					       "appearance_changed",
					       eel_widget_background_changed,
					       GTK_OBJECT (widget));
	eel_widget_background_changed (widget, background);

	return background;
}

gboolean
eel_widget_has_attached_background (GtkWidget *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

	return gtk_object_get_data (GTK_OBJECT (widget), "eel_background") != NULL;
}

GtkWidget *
eel_gtk_widget_find_background_ancestor (GtkWidget *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	while (widget != NULL) {
		if (eel_widget_has_attached_background (widget)) {
			return widget;
		}

		widget = widget->parent;
	}

	return NULL;
}

gboolean
eel_background_is_too_complex_for_gtk_style (EelBackground *background)
{
	if (background == NULL) {
		return FALSE;
	}
	
	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);
	
	if (background->details->image_uri != NULL) {
		return TRUE;
	}
	if (!background->details->is_solid_color) {
		return TRUE;
	}
	
	return FALSE;
}

/* determine if a background is darker or lighter than average, to help clients know what
   colors to draw on top with */
gboolean
eel_background_is_dark (EelBackground *background)
{
	GdkColor color, end_color;
	int intensity;
	char *start_color_spec, *end_color_spec;
	guint32 argb;
	guchar a;

	g_return_val_if_fail (EEL_IS_BACKGROUND (background), FALSE);
	
	if (background->details->is_solid_color) {
		eel_gdk_color_parse_with_white_default (background->details->color, &color);	
	} else {
		start_color_spec = eel_gradient_get_start_color_spec (background->details->color);
		eel_gdk_color_parse_with_white_default (start_color_spec, &color);	
		g_free (start_color_spec);

		end_color_spec = eel_gradient_get_end_color_spec (background->details->color);
		eel_gdk_color_parse_with_white_default (end_color_spec, &end_color);
		g_free (end_color_spec);
		
		color.red = (color.red + end_color.red) / 2;
		color.green = (color.green + end_color.green) / 2;
		color.blue = (color.blue + end_color.blue) / 2;
	}
	if (background->details->image != NULL) {
		argb = eel_gdk_pixbuf_average_value_argb (background->details->image);
		
		a = argb >> 24;

		color.red = (color.red * (0xFF - a) + ((argb >> 16) & 0xFF) * 0x101 * a) / 0xFF;
		color.green = (color.green * (0xFF - a) + ((argb >> 8) & 0xFF) * 0x101 * a) / 0xFF;
		color.blue = (color.blue * (0xFF - a) + (argb & 0xFF) * 0x101 * a) / 0xFF;
	}
	
	intensity = (color.red * 77 + color.green * 150 + color.blue * 28) >> 16;		 
	return intensity < 160; /* biased slightly to be dark */
}
   
/* handle dropped colors */
void
eel_background_receive_dropped_color (EelBackground *background,
					   GtkWidget *widget,
					   int drop_location_x,
					   int drop_location_y,
					   const GtkSelectionData *selection_data)
{
	guint16 *channels;
	char *color_spec;
	char *new_gradient_spec;
	int left_border, right_border, top_border, bottom_border;

	g_return_if_fail (EEL_IS_BACKGROUND (background));
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (selection_data != NULL);

	/* Convert the selection data into a color spec. */
	if (selection_data->length != 8 || selection_data->format != 16) {
		g_warning ("received invalid color data");
		return;
	}
	channels = (guint16 *) selection_data->data;
	color_spec = g_strdup_printf ("rgb:%04hX/%04hX/%04hX", channels[0], channels[1], channels[2]);

	/* Figure out if the color was dropped close enough to an edge to create a gradient.
	   For the moment, this is hard-wired, but later the widget will have to have some
	   say in where the borders are.
	*/
	left_border = 32;
	right_border = widget->allocation.width - 32;
	top_border = 32;
	bottom_border = widget->allocation.height - 32;
	if (drop_location_x < left_border && drop_location_x <= right_border) {
		new_gradient_spec = eel_gradient_set_left_color_spec (background->details->color, color_spec);
	} else if (drop_location_x >= left_border && drop_location_x > right_border) {
		new_gradient_spec = eel_gradient_set_right_color_spec (background->details->color, color_spec);
	} else if (drop_location_y < top_border && drop_location_y <= bottom_border) {
		new_gradient_spec = eel_gradient_set_top_color_spec (background->details->color, color_spec);
	} else if (drop_location_y >= top_border && drop_location_y > bottom_border) {
		new_gradient_spec = eel_gradient_set_bottom_color_spec (background->details->color, color_spec);
	} else {
		new_gradient_spec = g_strdup (color_spec);
	}
	
	g_free (color_spec);

	eel_background_set_image_uri_and_color (background, NULL, new_gradient_spec);

	g_free (new_gradient_spec);
}

/* self check code */

#if !defined (EEL_OMIT_SELF_CHECK)

void
eel_self_check_background (void)
{
	EelBackground *background;

	background = eel_background_new ();

	eel_background_set_color (background, NULL);
	eel_background_set_color (background, "");
	eel_background_set_color (background, "red");
	eel_background_set_color (background, "red-blue");
	eel_background_set_color (background, "red-blue:h");

	gtk_object_unref (GTK_OBJECT (background));
}

#endif /* !EEL_OMIT_SELF_CHECK */
