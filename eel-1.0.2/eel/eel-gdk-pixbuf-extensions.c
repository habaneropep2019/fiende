/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-gdk-pixbuf-extensions.c: Routines to augment what's in gdk-pixbuf.

   Copyright (C) 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Darin Adler <darin@eazel.com>
            Ramiro Estrugo <ramiro@eazel.com>
*/

#include <config.h>
#include "eel-gdk-pixbuf-extensions.h"

#include "eel-gdk-extensions.h"
#include "eel-glib-extensions.h"
#include "eel-string.h"
#include "eel-art-gtk-extensions.h"
#include "eel-lib-self-check-functions.h"
#include "eel-debug-drawing.h"
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include <libgnomevfs/gnome-vfs-async-ops.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <math.h>
#include <png.h>

#define LOAD_BUFFER_SIZE 65536

const ArtIRect eel_gdk_pixbuf_whole_pixbuf = { -1, -1, -1, -1 };

struct EelPixbufLoadHandle {
	GnomeVFSAsyncHandle *vfs_handle;
	EelPixbufLoadCallback callback;
	gpointer callback_data;
	GdkPixbufLoader *loader;
	char buffer[LOAD_BUFFER_SIZE];
};

static void file_opened_callback (GnomeVFSAsyncHandle *vfs_handle,
				  GnomeVFSResult       result,
				  gpointer             callback_data);
static void file_read_callback   (GnomeVFSAsyncHandle *vfs_handle,
				  GnomeVFSResult       result,
				  gpointer             buffer,
				  GnomeVFSFileSize     bytes_requested,
				  GnomeVFSFileSize     bytes_read,
				  gpointer             callback_data);
static void file_closed_callback (GnomeVFSAsyncHandle *handle,
				  GnomeVFSResult       result,
				  gpointer             callback_data);
static void load_done            (EelPixbufLoadHandle *handle,
				  GnomeVFSResult       result,
				  gboolean             get_pixbuf);

/**
 * eel_gdk_pixbuf_list_ref
 * @pixbuf_list: A list of GdkPixbuf objects.
 *
 * Refs all the pixbufs.
 **/
void
eel_gdk_pixbuf_list_ref (GList *pixbuf_list)
{
	g_list_foreach (pixbuf_list, (GFunc) gdk_pixbuf_ref, NULL);
}

/**
 * eel_gdk_pixbuf_list_free
 * @pixbuf_list: A list of GdkPixbuf objects.
 *
 * Unrefs all the pixbufs, then frees the list.
 **/
void
eel_gdk_pixbuf_list_free (GList *pixbuf_list)
{
	eel_g_list_free_deep_custom (pixbuf_list, (GFunc) gdk_pixbuf_unref, NULL);
}

GdkPixbuf *
eel_gdk_pixbuf_load (const char *uri)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;
	char buffer[LOAD_BUFFER_SIZE];
	GnomeVFSFileSize bytes_read;
	GdkPixbufLoader *loader;
	GdkPixbuf *pixbuf;	

	g_return_val_if_fail (uri != NULL, NULL);

	result = gnome_vfs_open (&handle,
				 uri,
				 GNOME_VFS_OPEN_READ);
	if (result != GNOME_VFS_OK) {
		return NULL;
	}

	loader = gdk_pixbuf_loader_new ();
	while (1) {
		result = gnome_vfs_read (handle,
					 buffer,
					 sizeof (buffer),
					 &bytes_read);
		if (result != GNOME_VFS_OK) {
			break;
		}
		if (bytes_read == 0) {
			break;
		}
		if (!gdk_pixbuf_loader_write (loader,
					      buffer,
					      bytes_read)) {
			result = GNOME_VFS_ERROR_WRONG_FORMAT;
			break;
		}
	}

	if (result != GNOME_VFS_OK && result != GNOME_VFS_ERROR_EOF) {
		gtk_object_unref (GTK_OBJECT (loader));
		gnome_vfs_close (handle);
		return NULL;
	}

	gnome_vfs_close (handle);
	gdk_pixbuf_loader_close (loader);

	pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
	if (pixbuf != NULL) {
		gdk_pixbuf_ref (pixbuf);
	}
	gtk_object_unref (GTK_OBJECT (loader));

	return pixbuf;
}

EelPixbufLoadHandle *
eel_gdk_pixbuf_load_async (const char *uri,
			   EelPixbufLoadCallback callback,
			   gpointer callback_data)
{
	EelPixbufLoadHandle *handle;

	handle = g_new0 (EelPixbufLoadHandle, 1);
	handle->callback = callback;
	handle->callback_data = callback_data;

	gnome_vfs_async_open (&handle->vfs_handle,
			      uri,
			      GNOME_VFS_OPEN_READ,
			      file_opened_callback,
			      handle);

	return handle;
}

static void
file_opened_callback (GnomeVFSAsyncHandle *vfs_handle,
		      GnomeVFSResult result,
		      gpointer callback_data)
{
	EelPixbufLoadHandle *handle;

	handle = callback_data;
	g_assert (handle->vfs_handle == vfs_handle);

	if (result != GNOME_VFS_OK) {
		handle->vfs_handle = NULL;
		load_done (handle, result, FALSE);
		return;
	}

	handle->loader = gdk_pixbuf_loader_new ();

	gnome_vfs_async_read (handle->vfs_handle,
			      handle->buffer,
			      sizeof (handle->buffer),
			      file_read_callback,
			      handle);
}

static void
file_read_callback (GnomeVFSAsyncHandle *vfs_handle,
		    GnomeVFSResult result,
		    gpointer buffer,
		    GnomeVFSFileSize bytes_requested,
		    GnomeVFSFileSize bytes_read,
		    gpointer callback_data)
{
	EelPixbufLoadHandle *handle;

	handle = callback_data;
	g_assert (handle->vfs_handle == vfs_handle);
	g_assert (handle->buffer == buffer);

	if (result == GNOME_VFS_OK && bytes_read != 0) {
		if (!gdk_pixbuf_loader_write (handle->loader,
					      buffer,
					      bytes_read)) {
			result = GNOME_VFS_ERROR_WRONG_FORMAT;
		}
		gnome_vfs_async_read (handle->vfs_handle,
				      handle->buffer,
				      sizeof (handle->buffer),
				      file_read_callback,
				      handle);
		return;
	}

	load_done (handle, result, result == GNOME_VFS_OK || result == GNOME_VFS_ERROR_EOF);
}

static void
file_closed_callback (GnomeVFSAsyncHandle *handle,
		      GnomeVFSResult result,
		      gpointer callback_data)
{
	g_assert (callback_data == NULL);
}

static void
free_pixbuf_load_handle (EelPixbufLoadHandle *handle)
{
	if (handle->loader != NULL) {
		gtk_object_unref (GTK_OBJECT (handle->loader));
	}
	g_free (handle);
}

static void
load_done (EelPixbufLoadHandle *handle, GnomeVFSResult result, gboolean get_pixbuf)
{
	GdkPixbuf *pixbuf;

	if (handle->loader != NULL) {
		gdk_pixbuf_loader_close (handle->loader);
	}

	pixbuf = get_pixbuf ? gdk_pixbuf_loader_get_pixbuf (handle->loader) : NULL;
	
	if (handle->vfs_handle != NULL) {
		gnome_vfs_async_close (handle->vfs_handle, file_closed_callback, NULL);
	}

	handle->callback (result, pixbuf, handle->callback_data);

	free_pixbuf_load_handle (handle);
}

void
eel_cancel_gdk_pixbuf_load (EelPixbufLoadHandle *handle)
{
	if (handle == NULL) {
		return;
	}
	if (handle->vfs_handle != NULL) {
		gnome_vfs_async_cancel (handle->vfs_handle);
	}
	free_pixbuf_load_handle (handle);
}

/* return the average value of each component */
guint32
eel_gdk_pixbuf_average_value_argb (GdkPixbuf *pixbuf)
{
	guint64 a_total, r_total, g_total, b_total;
	int row, column;
	gboolean has_alpha;
	int width, height;
	int row_stride;
	const guchar *pixels, *p;
	int r, g, b, a;
	int dividend;

	has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	row_stride = gdk_pixbuf_get_rowstride (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);
	
	/* iterate through the pixbuf, counting up each component */
	a_total = 0;
	r_total = 0;
	g_total = 0;
	b_total = 0;
	for (row = 0; row < height; row++) {
		p = pixels + (row * row_stride);
		for (column = 0; column < width; column++) {
			r = *p++;
			g = *p++;
			b = *p++;
			a = has_alpha ? *p++ : 0xFF;

			a_total += a;
			r_total += r * a;
			g_total += g * a;
			b_total += b * a;
		}
	}

	a_total *= 0xFF;
	dividend = height * width * 0xFF;
	return ((a_total + dividend / 2) / dividend) << 24
		| ((r_total + dividend / 2) / dividend) << 16
		| ((g_total + dividend / 2) / dividend) << 8
		| ((b_total + dividend / 2) / dividend);
}

void
eel_gdk_pixbuf_average_value (GdkPixbuf *pixbuf, GdkColor *average_color)
{
	guint32 argb;

	argb = eel_gdk_pixbuf_average_value_argb (pixbuf);
	average_color->pixel = 0;
	average_color->red = ((argb >> 16) & 0xFF) * 0x101;
	average_color->green = ((argb >> 8) & 0xFF) * 0x101;
	average_color->blue = (argb & 0xFF) * 0x101;
}

double
eel_gdk_scale_to_fit_factor (int width, int height,
			     int max_width, int max_height,
			     int *scaled_width, int *scaled_height)
{
	double scale_factor;
	
	scale_factor = MIN (max_width  / (double) width, max_height / (double) height);

	*scaled_width  = floor (width * scale_factor + .5);
	*scaled_height = floor (height * scale_factor + .5);

	return scale_factor;
}

/* Returns a scaled copy of pixbuf, preserving aspect ratio. The copy will
 * be scaled as large as possible without exceeding the specified width and height.
 */
GdkPixbuf *
eel_gdk_pixbuf_scale_to_fit (GdkPixbuf *pixbuf, int max_width, int max_height)
{
	int scaled_width;
	int scaled_height;

	eel_gdk_scale_to_fit_factor (gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf),
				     max_width, max_height,
				     &scaled_width, &scaled_height);

	return gdk_pixbuf_scale_simple (pixbuf, scaled_width, scaled_height, GDK_INTERP_BILINEAR);	
}

/* Returns a copy of pixbuf scaled down, preserving aspect ratio, to fit
 * within the specified width and height. If it already fits, a copy of
 * the original, without scaling, is returned.
 */
GdkPixbuf *
eel_gdk_pixbuf_scale_down_to_fit (GdkPixbuf *pixbuf, int max_width, int max_height)
{
	int scaled_width;
	int scaled_height;
	
	double scale_factor;

	scale_factor = eel_gdk_scale_to_fit_factor (gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf),
						    max_width, max_height,
						    &scaled_width, &scaled_height);

	if (scale_factor >= 1.0) {
		return gdk_pixbuf_copy (pixbuf);
	} else {				
		return gdk_pixbuf_scale_simple (pixbuf, scaled_width, scaled_height, GDK_INTERP_BILINEAR);	
	}
}

/**
 * eel_gdk_pixbuf_is_valid:
 * @pixbuf: A GdkPixbuf
 *
 * Return value: A boolean indicating whether the given pixbuf is valid.
 *
 * A pixbuf is valid if:
 * 
 *   1. It is non NULL
 *   2. It is has non NULL pixel data.
 *   3. It has width and height greater than 0.
 */
gboolean
eel_gdk_pixbuf_is_valid (const GdkPixbuf *pixbuf)
{
	return ((pixbuf != NULL)
		&& (gdk_pixbuf_get_pixels (pixbuf) != NULL)
		&& (gdk_pixbuf_get_width (pixbuf) > 0)
		&& (gdk_pixbuf_get_height (pixbuf) > 0));
}

/**
 * eel_gdk_pixbuf_get_dimensions:
 * @pixbuf: A GdkPixbuf
 *
 * Return value: The dimensions of the pixbuf as a EelDimensions.
 *
 * This function is useful in code that uses libart rect 
 * intersection routines.
 */
EelDimensions
eel_gdk_pixbuf_get_dimensions (const GdkPixbuf *pixbuf)
{
	EelDimensions dimensions;

	g_return_val_if_fail (eel_gdk_pixbuf_is_valid (pixbuf), eel_dimensions_empty);

	dimensions.width = gdk_pixbuf_get_width (pixbuf);
	dimensions.height = gdk_pixbuf_get_height (pixbuf);

	return dimensions;
}

/**
 * eel_gdk_pixbuf_fill_rectangle:
 * @pixbuf: Target pixbuf to fill into.
 * @area: Rectangle to fill.
 * @color: The color to use.
 *
 * Fill the rectangle with the the given color.
 * Use the given rectangle if not NULL.
 * If rectangle is NULL, fill the whole pixbuf.
 */
void
eel_gdk_pixbuf_fill_rectangle_with_color (GdkPixbuf *pixbuf,
					  ArtIRect area,
					  guint32 color)
{
	ArtIRect target;
	guchar red;
	guchar green;
	guchar blue;
	guchar alpha;
	guchar *pixels;
	gboolean has_alpha;
	guint pixel_offset;
	guint rowstride;
	guchar *row_offset;
	int x;
	int y;

	g_return_if_fail (eel_gdk_pixbuf_is_valid (pixbuf));
	
	target = eel_gdk_pixbuf_intersect (pixbuf, 0, 0, area);
	if (art_irect_empty (&target)) {
		return;
	}

	pixels = gdk_pixbuf_get_pixels (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
	pixel_offset = has_alpha ? 4 : 3;
	red = EEL_RGBA_COLOR_GET_R (color);
	green = EEL_RGBA_COLOR_GET_G (color);
	blue = EEL_RGBA_COLOR_GET_B (color);
	alpha = EEL_RGBA_COLOR_GET_A (color);

	row_offset = pixels + target.y0 * rowstride;

	for (y = target.y0; y < target.y1; y++) {
		guchar *offset = row_offset + (target.x0 * pixel_offset);
		
		for (x = target.x0; x < target.x1; x++) {
			*(offset++) = red;
			*(offset++) = green;
			*(offset++) = blue;
			
			if (has_alpha) {
				*(offset++) = alpha;
			}
			
		}

		row_offset += rowstride;
	}
}

/* utility routine for saving a pixbuf to a png file.
 * This was adapted from Iain Holmes' code in gnome-iconedit, and probably
 * should be in a utility library, possibly in gdk-pixbuf itself.
 */
gboolean
eel_gdk_pixbuf_save_to_file (const GdkPixbuf *pixbuf,
			     const char *file_name)
{
	FILE *handle;
  	char *buffer;
	gboolean has_alpha;
	int width, height, depth, rowstride;
  	guchar *pixels;
  	png_structp png_ptr;
  	png_infop info_ptr;
  	png_text text[2];
  	int i;

	g_return_val_if_fail (pixbuf != NULL, FALSE);
	g_return_val_if_fail (file_name != NULL, FALSE);
	g_return_val_if_fail (file_name[0] != '\0', FALSE);

        handle = fopen (file_name, "wb");
        if (handle == NULL) {
        	return FALSE;
	}

	png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose (handle);
		return FALSE;
	}

	info_ptr = png_create_info_struct (png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct (&png_ptr, (png_infopp)NULL);
		fclose (handle);
	    	return FALSE;
	}

	if (setjmp (png_ptr->jmpbuf)) {
		png_destroy_write_struct (&png_ptr, &info_ptr);
		fclose (handle);
		return FALSE;
	}

	png_init_io (png_ptr, handle);

        has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	depth = gdk_pixbuf_get_bits_per_sample (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);

	png_set_IHDR (png_ptr, info_ptr, width, height,
		      depth, PNG_COLOR_TYPE_RGB_ALPHA,
		      PNG_INTERLACE_NONE,
		      PNG_COMPRESSION_TYPE_DEFAULT,
		      PNG_FILTER_TYPE_DEFAULT);

	/* Some text to go with the png image */
	text[0].key = "Title";
	text[0].text = (char *) file_name;
	text[0].compression = PNG_TEXT_COMPRESSION_NONE;
	text[1].key = "Software";
	text[1].text = "Eel Thumbnail";
	text[1].compression = PNG_TEXT_COMPRESSION_NONE;
	png_set_text (png_ptr, info_ptr, text, 2);

	/* Write header data */
	png_write_info (png_ptr, info_ptr);

	/* if there is no alpha in the data, allocate buffer to expand into */
	if (has_alpha) {
		buffer = NULL;
	} else {
		buffer = g_malloc(4 * width);
	}
	
	/* pump the raster data into libpng, one scan line at a time */	
	for (i = 0; i < height; i++) {
		if (has_alpha) {
			png_bytep row_pointer = pixels;
			png_write_row (png_ptr, row_pointer);
		} else {
			/* expand RGB to RGBA using an opaque alpha value */
			int x;
			char *buffer_ptr = buffer;
			char *source_ptr = pixels;
			for (x = 0; x < width; x++) {
				*buffer_ptr++ = *source_ptr++;
				*buffer_ptr++ = *source_ptr++;
				*buffer_ptr++ = *source_ptr++;
				*buffer_ptr++ = 255;
			}
			png_write_row (png_ptr, (png_bytep) buffer);		
		}
		pixels += rowstride;
	}
	
	png_write_end (png_ptr, info_ptr);
	png_destroy_write_struct (&png_ptr, &info_ptr);
	
	g_free (buffer);
		
	fclose (handle);
	return TRUE;
}

void
eel_gdk_pixbuf_ref_if_not_null (GdkPixbuf *pixbuf_or_null)
{
	if (pixbuf_or_null != NULL) {
		gdk_pixbuf_ref (pixbuf_or_null);
	}
}

void
eel_gdk_pixbuf_unref_if_not_null (GdkPixbuf *pixbuf_or_null)
{
	if (pixbuf_or_null != NULL) {
		gdk_pixbuf_unref (pixbuf_or_null);
	}
}

void
eel_gdk_pixbuf_draw_to_drawable (const GdkPixbuf *pixbuf,
				 GdkDrawable *drawable,
				 GdkGC *gc,
				 int source_x,
				 int source_y,
				 ArtIRect destination_area,
				 GdkRgbDither dither,
				 GdkPixbufAlphaMode alpha_compositing_mode,
				 int alpha_threshold)
{
	EelDimensions dimensions;
	ArtIRect target;
	ArtIRect source;
	int target_width;
	int target_height;
	int source_width;
	int source_height;

	g_return_if_fail (eel_gdk_pixbuf_is_valid (pixbuf));
	g_return_if_fail (drawable != NULL);
	g_return_if_fail (gc != NULL);
	g_return_if_fail (!art_irect_empty (&destination_area));
 	g_return_if_fail (alpha_threshold > EEL_OPACITY_FULLY_TRANSPARENT);
 	g_return_if_fail (alpha_threshold <= EEL_OPACITY_FULLY_OPAQUE);
 	g_return_if_fail (alpha_compositing_mode >= GDK_PIXBUF_ALPHA_BILEVEL);
 	g_return_if_fail (alpha_compositing_mode <= GDK_PIXBUF_ALPHA_FULL);

	dimensions = eel_gdk_pixbuf_get_dimensions (pixbuf);
	
	g_return_if_fail (source_x >= 0);
	g_return_if_fail (source_y >= 0);
	g_return_if_fail (source_x < dimensions.width);
	g_return_if_fail (source_y < dimensions.height);

	/* Clip the destination area to the pixbuf dimensions; bail if no work */
	target = eel_gdk_pixbuf_intersect (pixbuf,
					   destination_area.x0,
					   destination_area.y0,
					   destination_area);
	if (art_irect_empty (&target)) {
		return;
	}

	/* Assign the source area */
	source = eel_art_irect_assign (source_x,
				       source_y,
				       dimensions.width - source_x,
				       dimensions.height - source_y);

	/* Adjust the target width if the source area is smaller than the
	 * source pixbuf dimensions */
	target_width = target.x1 - target.x0;
	target_height = target.y1 - target.y0;
	source_width = source.x1 - source.x0;
	source_height = source.y1 - source.y0;

	target.x1 = target.x0 + MIN (target_width, source_width);
	target.y1 = target.y0 + MIN (target_height, source_height);

	if (gdk_pixbuf_get_has_alpha (pixbuf)) {
		gdk_pixbuf_render_to_drawable_alpha ((GdkPixbuf *) pixbuf,
						     drawable,
						     source.x0,
						     source.y0,
						     target.x0,
						     target.y0,
						     target.x1 - target.x0,
						     target.y1 - target.y0,
						     alpha_compositing_mode,
						     alpha_threshold,
						     dither,
						     0,
						     0);
	} else {
		gdk_pixbuf_render_to_drawable ((GdkPixbuf *) pixbuf,
					       drawable,
					       gc,
					       source.x0,
					       source.y0,
					       target.x0,
					       target.y0,
					       target.x1 - target.x0,
					       target.y1 - target.y0,
					       dither,
					       0,
					       0);
	}
}

/**
 * eel_gdk_pixbuf_draw_to_pixbuf:
 * @pixbuf: The source pixbuf to draw.
 * @destination_pixbuf: The destination pixbuf.
 * @source_x: The source pixbuf x coordiate to composite from.
 * @source_y: The source pixbuf y coordiate to composite from.
 * @destination_area: The destination area within the destination pixbuf.
 *                    This area will be clipped if invalid in any way.
 *
 * Copy one pixbuf onto another another..  This function has some advantages
 * over plain gdk_pixbuf_copy_area():
 *
 *   Composition paramters (source coordinate, destination area) are
 *   given in a way that is consistent with the rest of the extensions
 *   in this file.  That is, it matches the declaration of
 *   eel_gdk_pixbuf_draw_to_pixbuf_alpha() and 
 *   eel_gdk_pixbuf_draw_to_drawable() very closely.
 *
 *   All values are clipped to make sure they are valid.
 *
 */
void
eel_gdk_pixbuf_draw_to_pixbuf (const GdkPixbuf *pixbuf,
			       GdkPixbuf *destination_pixbuf,
			       int source_x,
			       int source_y,
			       ArtIRect destination_area)
{
	EelDimensions dimensions;
	ArtIRect target;
	ArtIRect source;
	int target_width;
	int target_height;
	int source_width;
	int source_height;
	
	g_return_if_fail (eel_gdk_pixbuf_is_valid (pixbuf));
	g_return_if_fail (eel_gdk_pixbuf_is_valid (destination_pixbuf));
	g_return_if_fail (!art_irect_empty (&destination_area));

	dimensions = eel_gdk_pixbuf_get_dimensions (pixbuf);

	g_return_if_fail (source_x >= 0);
	g_return_if_fail (source_y >= 0);
	g_return_if_fail (source_x < dimensions.width);
	g_return_if_fail (source_y < dimensions.height);

	/* Clip the destination area to the pixbuf dimensions; bail if no work */
	target = eel_gdk_pixbuf_intersect (destination_pixbuf, 0, 0, destination_area);
	if (art_irect_empty (&target)) {
 		return;
 	}

	/* Assign the source area */
	source = eel_art_irect_assign (source_x,
				       source_y,
				       dimensions.width - source_x,
				       dimensions.height - source_y);

	/* Adjust the target width if the source area is smaller than the
	 * source pixbuf dimensions */
	target_width = target.x1 - target.x0;
	target_height = target.y1 - target.y0;
	source_width = source.x1 - source.x0;
	source_height = source.y1 - source.y0;

	target.x1 = target.x0 + MIN (target_width, source_width);
	target.y1 = target.y0 + MIN (target_height, source_height);

	gdk_pixbuf_copy_area (pixbuf,
			      source.x0,
			      source.y0,
			      target.x1 - target.x0,
			      target.y1 - target.y0,
			      destination_pixbuf,
			      target.x0,
			      target.y0);
}

/**
 * eel_gdk_pixbuf_draw_to_pixbuf_alpha:
 * @pixbuf: The source pixbuf to draw.
 * @destination_pixbuf: The destination pixbuf.
 * @source_x: The source pixbuf x coordiate to composite from.
 * @source_y: The source pixbuf y coordiate to composite from.
 * @destination_area: The destination area within the destination pixbuf.
 *                    This area will be clipped if invalid in any way.
 * @opacity: The opacity of the drawn tiles where 0 <= opacity <= 255.
 * @interpolation_mode: The interpolation mode.  See <gdk-pixbuf.h>
 *
 * Composite one pixbuf over another.  This function has some advantages
 * over plain gdk_pixbuf_composite():
 *
 *   Composition paramters (source coordinate, destination area) are
 *   given in a way that is consistent with the rest of the extensions
 *   in this file.  That is, it matches the declaration of
 *   eel_gdk_pixbuf_draw_to_pixbuf() and 
 *   eel_gdk_pixbuf_draw_to_drawable() very closely.
 *
 *   All values are clipped to make sure they are valid.
 *
 *   Workaround a limitation in gdk_pixbuf_composite() that does not allow
 *   the source (x,y) to be greater than (0,0)
 * 
 */
void
eel_gdk_pixbuf_draw_to_pixbuf_alpha (const GdkPixbuf *pixbuf,
				     GdkPixbuf *destination_pixbuf,
				     int source_x,
				     int source_y,
				     ArtIRect destination_area,
				     int opacity,
				     GdkInterpType interpolation_mode)
{
	EelDimensions dimensions;
	ArtIRect target;
	ArtIRect source;
	int target_width;
	int target_height;
	int source_width;
	int source_height;

	g_return_if_fail (eel_gdk_pixbuf_is_valid (pixbuf));
	g_return_if_fail (eel_gdk_pixbuf_is_valid (destination_pixbuf));
	g_return_if_fail (!art_irect_empty (&destination_area));
	g_return_if_fail (opacity >= EEL_OPACITY_FULLY_TRANSPARENT);
	g_return_if_fail (opacity <= EEL_OPACITY_FULLY_OPAQUE);
	g_return_if_fail (interpolation_mode >= GDK_INTERP_NEAREST);
	g_return_if_fail (interpolation_mode <= GDK_INTERP_HYPER);
	
	dimensions = eel_gdk_pixbuf_get_dimensions (pixbuf);

	g_return_if_fail (source_x >= 0);
	g_return_if_fail (source_y >= 0);
	g_return_if_fail (source_x < dimensions.width);
	g_return_if_fail (source_y < dimensions.height);

	/* Clip the destination area to the pixbuf dimensions; bail if no work */
	target = eel_gdk_pixbuf_intersect (destination_pixbuf, 0, 0, destination_area);
	if (art_irect_empty (&target)) {
 		return;
 	}

	/* Assign the source area */
	source = eel_art_irect_assign (source_x,
				       source_y,
				       dimensions.width - source_x,
				       dimensions.height - source_y);
	
	/* Adjust the target width if the source area is smaller than the
	 * source pixbuf dimensions */
	target_width = target.x1 - target.x0;
	target_height = target.y1 - target.y0;
	source_width = source.x1 - source.x0;
	source_height = source.y1 - source.y0;

	target.x1 = target.x0 + MIN (target_width, source_width);
	target.y1 = target.y0 + MIN (target_height, source_height);
	
	/* If the source point is not (0,0), then we need to create a sub pixbuf
	 * with only the source area.  This is needed to work around a limitation
	 * in gdk_pixbuf_composite() that requires the source area to be (0,0). */
	if (source.x0 != 0 || source.y0 != 0) {
		ArtIRect area;
		int width;
		int height;

		width = dimensions.width - source.x0;
		height = dimensions.height - source.y0;
		
		area.x0 = source.x0;
		area.y0 = source.y0;
		area.x1 = area.x0 + width;
		area.y1 = area.y0 + height;
		
		pixbuf = eel_gdk_pixbuf_new_from_pixbuf_sub_area ((GdkPixbuf *) pixbuf, area);
	} else {
		gdk_pixbuf_ref ((GdkPixbuf *) pixbuf);
	}
	
	gdk_pixbuf_composite (pixbuf,
			      destination_pixbuf,
			      target.x0,
			      target.y0,
			      target.x1 - target.x0,
			      target.y1 - target.y0,
			      target.x0,
			      target.y0,
			      1.0,
			      1.0,
			      interpolation_mode,
			      opacity);

	gdk_pixbuf_unref ((GdkPixbuf *) pixbuf);
}

static void
pixbuf_destroy_callback (guchar *pixels,
			 gpointer callback_data)
{
	g_return_if_fail (pixels != NULL);
	g_return_if_fail (callback_data != NULL);

	gdk_pixbuf_unref ((GdkPixbuf *) callback_data);
}

/**
 * eel_gdk_pixbuf_new_from_pixbuf_sub_area:
 * @pixbuf: The source pixbuf.
 * @area: The area within the source pixbuf to use for the sub pixbuf.
 *        This area needs to be contained within the bounds of the 
 *        source pixbuf, otherwise it will be clipped to that. 
 *
 * Return value: A newly allocated pixbuf that shares the pixel data
 *               of the source pixbuf in order to represent a sub area.
 *
 * Create a pixbuf from a sub area of another pixbuf.  The resulting pixbuf
 * will share the pixel data of the source pixbuf.  Memory bookeeping is
 * all taken care for the caller.  All you need to do is gdk_pixbuf_unref()
 * the resulting pixbuf to properly free resources.
 */
GdkPixbuf *
eel_gdk_pixbuf_new_from_pixbuf_sub_area (GdkPixbuf *pixbuf,
					 ArtIRect area)
{
	GdkPixbuf *sub_pixbuf;
	ArtIRect target;
	guchar *pixels;
	
	g_return_val_if_fail (eel_gdk_pixbuf_is_valid (pixbuf), NULL);
	g_return_val_if_fail (!art_irect_empty (&area), NULL);

	/* Clip the pixbuf by the given area; bail if no work */
	target = eel_gdk_pixbuf_intersect (pixbuf, 0, 0, area);
	if (art_irect_empty (&target)) {
 		return NULL;
 	}

	/* Since we are going to be sharing the given pixbuf's data, we need 
	 * to ref it.  It will be unreffed in the destroy function above */
	gdk_pixbuf_ref (pixbuf);

	/* Compute the offset into the pixel data */
	pixels = 
		gdk_pixbuf_get_pixels (pixbuf)
		+ (target.y0 * gdk_pixbuf_get_rowstride (pixbuf))
		+ (target.x0 * (gdk_pixbuf_get_has_alpha (pixbuf) ? 4 : 3));
	
	/* Make a pixbuf pretending its real estate is the sub area */
	sub_pixbuf = gdk_pixbuf_new_from_data (pixels,
					       GDK_COLORSPACE_RGB,
					       gdk_pixbuf_get_has_alpha (pixbuf),
					       8,
					       eel_art_irect_get_width (target),
					       eel_art_irect_get_height (target),
					       gdk_pixbuf_get_rowstride (pixbuf),
					       pixbuf_destroy_callback,
					       pixbuf);

	return sub_pixbuf;
}

/**
 * eel_gdk_pixbuf_new_from_existing_buffer:
 * @buffer: The existing buffer.
 * @buffer_rowstride: The existing buffer's rowstride.
 * @buffer_has_alpha: A boolean value indicating whether the buffer has alpha.
 * @area: The area within the existing buffer to use for the pixbuf.
 *        This area needs to be contained within the bounds of the 
 *        buffer, otherwise memory will be trashed.
 *
 * Return value: A newly allocated pixbuf that uses the existing buffer
 *               for its pixel data.
 *
 * Create a pixbuf from an existing buffer.
 *
 * The resulting pixbuf is only valid for as long as &buffer is valid.  It is
 * up to the caller to make sure they both exist in the same scope.
 * Also, it is up to the caller to make sure that the given area is fully 
 * contained in the buffer, otherwise memory trashing will happen.
 */
GdkPixbuf *
eel_gdk_pixbuf_new_from_existing_buffer (guchar *buffer,
					 int buffer_rowstride,
					 gboolean buffer_has_alpha,
					 ArtIRect area)
{
	GdkPixbuf *pixbuf;
	guchar *pixels;
	
	g_return_val_if_fail (buffer != NULL, NULL);
	g_return_val_if_fail (buffer_rowstride > 0, NULL);
	g_return_val_if_fail (!art_irect_empty (&area), NULL);
	
	/* Compute the offset into the buffer */
	pixels = 
		buffer
		+ (area.y0 * buffer_rowstride)
		+ (area.x0 * (buffer_has_alpha ? 4 : 3));
	
	pixbuf = gdk_pixbuf_new_from_data (pixels,
					   GDK_COLORSPACE_RGB,
					   buffer_has_alpha,
					   8,
					   eel_art_irect_get_width (area),
					   eel_art_irect_get_height (area),
					   buffer_rowstride,
					   NULL,
					   NULL);

	return pixbuf;
}

/* The tile algorithm is identical whether the destination is 
 * a pixbuf or a drawable.  So, we use a simple callback
 * mechanism to share it regardless of the destination.
 */
typedef struct {
	GdkPixbuf *destination_pixbuf;
	int opacity;
	GdkInterpType interpolation_mode;
} PixbufTileData;

typedef struct {
	GdkDrawable *drawable;
	GdkGC *gc;
	GdkRgbDither dither;
	GdkPixbufAlphaMode alpha_compositing_mode;
	int alpha_threshold;
} DrawableTileData;

typedef void (* DrawPixbufTileCallback) (const GdkPixbuf *pixbuf,
					 int x,
					 int y,
					 ArtIRect destination_area,
					 gpointer callback_data);

/* The shared tiliing implementation */
static void
pixbuf_draw_tiled (const GdkPixbuf *pixbuf,
		   EelDimensions destination_dimensions,
		   ArtIRect destination_area,
		   int tile_width,
		   int tile_height,
		   int tile_origin_x,
		   int tile_origin_y,
		   DrawPixbufTileCallback callback,
		   gpointer callback_data)
{
	ArtIRect target;
	int x;
	int y;
	EelArtIPoint min_point;
	EelArtIPoint max_point;
	int num_left;
	int num_above;
	ArtIRect clipped_destination_area;

	g_return_if_fail (pixbuf != NULL);
	g_return_if_fail (destination_dimensions.width > 0);
	g_return_if_fail (destination_dimensions.height > 0);
	g_return_if_fail (tile_width > 0);
	g_return_if_fail (tile_height > 0);
	g_return_if_fail (tile_width <= gdk_pixbuf_get_width (pixbuf));
	g_return_if_fail (tile_height <= gdk_pixbuf_get_height (pixbuf));
	g_return_if_fail (callback != NULL);
	g_return_if_fail (!art_irect_empty (&destination_area));

	/* FIXME: This is confusing.  Instead of passing in the destination_dimensions
	 *        I should just pass in the destination pixbuf, so that we can use
	 *        eel_gdk_pixbuf_intersect directly on that.
	 */
	clipped_destination_area = eel_art_irect_assign_dimensions (0, 0, destination_dimensions);
	art_irect_intersect (&target, &destination_area, &clipped_destination_area);
	if (art_irect_empty (&target)) {
		return;
	}

	/* The number of tiles left and above the target area */
	num_left = (target.x0 - tile_origin_x) / tile_width;
	num_above = (target.y0 - tile_origin_y) / tile_height;
	
	min_point.x = tile_origin_x - tile_width + (num_left * tile_width);
	min_point.y = tile_origin_y - tile_height + (num_above * tile_height);
	
	max_point.x = (target.x1 + 2 * tile_width);
	max_point.y = (target.y1 + 2 * tile_height);
	
	for (y = min_point.y; y <= max_point.y; y += tile_height) {
		for (x = min_point.x; x <= max_point.x; x += tile_width) {
			ArtIRect current;
			ArtIRect area;

			current = eel_art_irect_assign (x, y, tile_width, tile_height);

			/* FIXME: A potential speed improvement here would be to clip only the
			 * first and last rectangles, not the ones in between.  
			 */
			art_irect_intersect (&area, &target, &current);

			if (!art_irect_empty (&area)) {
				g_assert (area.x0 >= x);
				g_assert (area.y0 >= y);

				(* callback) (pixbuf,
					      area.x0 - x,
					      area.y0 - y,
					      area,
					      callback_data);
			}
		}
	}
}

static void
draw_tile_to_pixbuf_callback (const GdkPixbuf *pixbuf,
			      int x,
			      int y,
			      ArtIRect area,
			      gpointer callback_data)
{
	PixbufTileData *pixbuf_tile_data;

	g_return_if_fail (pixbuf != NULL);
	g_return_if_fail (callback_data != NULL);
	g_return_if_fail (!art_irect_empty (&area));

	pixbuf_tile_data = (PixbufTileData *) callback_data;

	if (pixbuf_tile_data->opacity == EEL_OPACITY_FULLY_TRANSPARENT) {
		eel_gdk_pixbuf_draw_to_pixbuf (pixbuf,
					       pixbuf_tile_data->destination_pixbuf,
					       x,
					       y,
					       area);
	} else {
		eel_gdk_pixbuf_draw_to_pixbuf_alpha (pixbuf,
						     pixbuf_tile_data->destination_pixbuf,
						     x,
						     y,
						     area,
						     pixbuf_tile_data->opacity,
						     pixbuf_tile_data->interpolation_mode);
	}
}

static void
draw_tile_to_drawable_callback (const GdkPixbuf *pixbuf,
				int x,
				int y,
				ArtIRect area,
				gpointer callback_data)
{
	DrawableTileData *drawable_tile_data;
	
	g_return_if_fail (pixbuf != NULL);
	g_return_if_fail (callback_data != NULL);
	g_return_if_fail (!art_irect_empty (&area));

	drawable_tile_data = (DrawableTileData *) callback_data;

	eel_gdk_pixbuf_draw_to_drawable (pixbuf,
					 drawable_tile_data->drawable,
					 drawable_tile_data->gc,
					 x,
					 y,
					 area,
					 drawable_tile_data->dither,
					 drawable_tile_data->alpha_compositing_mode,
					 drawable_tile_data->alpha_threshold);
}

/**
 * eel_gdk_pixbuf_draw_to_pixbuf_tiled:
 * @pixbuf: Source tile pixbuf.
 * @destination_pixbuf: Destination pixbuf.
 * @destination_area: Area of the destination pixbuf to tile.
 * @tile_width: Width of the tile.  This can be less than width of the
 *              tile pixbuf, but not greater.
 * @tile_height: Height of the tile.  This can be less than width of the
 *               tile pixbuf, but not greater.
 * @tile_origin_x: The x coordinate of the tile origin.  Can be negative.
 * @tile_origin_y: The y coordinate of the tile origin.  Can be negative.
 * @opacity: The opacity of the drawn tiles where 0 <= opacity <= 255.
 * @interpolation_mode: The interpolation mode.  See <gdk-pixbuf.h>
 *
 * Fill an area of a GdkPixbuf with a tile.
 */
void
eel_gdk_pixbuf_draw_to_pixbuf_tiled (const GdkPixbuf *pixbuf,
				     GdkPixbuf *destination_pixbuf,
				     ArtIRect destination_area,
				     int tile_width,
				     int tile_height,
				     int tile_origin_x,
				     int tile_origin_y,
				     int opacity,
				     GdkInterpType interpolation_mode)
{
	PixbufTileData pixbuf_tile_data;
	EelDimensions destination_dimensions;

	g_return_if_fail (eel_gdk_pixbuf_is_valid (destination_pixbuf));
	g_return_if_fail (eel_gdk_pixbuf_is_valid (pixbuf));
	g_return_if_fail (tile_width > 0);
	g_return_if_fail (tile_height > 0);
	g_return_if_fail (tile_width <= gdk_pixbuf_get_width (pixbuf));
	g_return_if_fail (tile_height <= gdk_pixbuf_get_height (pixbuf));
	g_return_if_fail (opacity >= EEL_OPACITY_FULLY_TRANSPARENT);
	g_return_if_fail (opacity <= EEL_OPACITY_FULLY_OPAQUE);
	g_return_if_fail (interpolation_mode >= GDK_INTERP_NEAREST);
	g_return_if_fail (interpolation_mode <= GDK_INTERP_HYPER);

	destination_dimensions = eel_gdk_pixbuf_get_dimensions (destination_pixbuf);

	pixbuf_tile_data.destination_pixbuf = destination_pixbuf;
	pixbuf_tile_data.opacity = opacity;
	pixbuf_tile_data.interpolation_mode = interpolation_mode;

	pixbuf_draw_tiled (pixbuf,
			   destination_dimensions,
			   destination_area,
			   tile_width,
			   tile_height,
			   tile_origin_x,
			   tile_origin_y,
			   draw_tile_to_pixbuf_callback,
			   &pixbuf_tile_data);
}

/**
 * eel_gdk_pixbuf_draw_to_drawable_tiled:
 * @pixbuf: Source tile pixbuf.
 * @gc: GC for copy area operation.
 * @drawable: Destination drawable.
 * @destination_area: Area of the destination pixbuf to tile.
 * @tile_width: Width of the tile.  This can be less than width of the
 *              tile pixbuf, but not greater.
 * @tile_height: Height of the tile.  This can be less than width of the
 *               tile pixbuf, but not greater.
 * @tile_origin_x: The x coordinate of the tile origin.  Can be negative.
 * @tile_origin_y: The y coordinate of the tile origin.  Can be negative.
 * @dither: Dither type to use (see <gdkrgb.h>)
 * @dither: Dither type to use (see <gdkrgb.h>)
 * @alpha_compositing_mode: The alpha compositing mode.  See <gdk-pixbuf.h>
 *
 * Fill an area of a GdkDrawable with a tile.
 */
void
eel_gdk_pixbuf_draw_to_drawable_tiled (const GdkPixbuf *pixbuf,
				       GdkDrawable *drawable,
				       GdkGC *gc,
				       ArtIRect destination_area,
				       int tile_width,
				       int tile_height,
				       int tile_origin_x,
				       int tile_origin_y,
				       GdkRgbDither dither,
				       GdkPixbufAlphaMode alpha_compositing_mode,
				       int alpha_threshold)
{
	DrawableTileData drawable_tile_data;
	EelDimensions destination_dimensions;

	g_return_if_fail (eel_gdk_pixbuf_is_valid (pixbuf));
	g_return_if_fail (drawable != NULL);
	g_return_if_fail (tile_width > 0);
	g_return_if_fail (tile_height > 0);
	g_return_if_fail (tile_width <= gdk_pixbuf_get_width (pixbuf));
	g_return_if_fail (tile_height <= gdk_pixbuf_get_height (pixbuf));
 	g_return_if_fail (alpha_threshold > EEL_OPACITY_FULLY_TRANSPARENT);
 	g_return_if_fail (alpha_threshold <= EEL_OPACITY_FULLY_OPAQUE);
 	g_return_if_fail (alpha_compositing_mode >= GDK_PIXBUF_ALPHA_BILEVEL);
 	g_return_if_fail (alpha_compositing_mode <= GDK_PIXBUF_ALPHA_FULL);

	destination_dimensions = eel_gdk_window_get_dimensions (drawable);
	
	drawable_tile_data.drawable = drawable;
	drawable_tile_data.gc = gc;
	drawable_tile_data.dither = dither;
	drawable_tile_data.alpha_compositing_mode = alpha_compositing_mode;
	drawable_tile_data.alpha_threshold = alpha_threshold;
	
	pixbuf_draw_tiled (pixbuf,
			   destination_dimensions,
			   destination_area,
			   tile_width,
			   tile_height,
			   tile_origin_x,
			   tile_origin_y,
			   draw_tile_to_drawable_callback,
			   &drawable_tile_data);
}

/**
 * eel_gdk_pixbuf_get_global_buffer:
 * @minimum_width: The minimum width for the requested buffer.
 * @minimum_height: The minimum height for the requested buffer.
 *
 * Return value: A GdkPixbuf that is at least as big as the passed in 
 *               dimensions.
 *
 * Access a global buffer for temporary GdkPixbuf operations.  
 * The returned buffer will be at least as big as the passed in 
 * dimensions.  The contents are not guaranteed to be anything at
 * anytime.  Also, it is not thread safe at all. */

static GdkPixbuf *global_buffer = NULL;

static void
destroy_global_buffer (void)
{
	if (global_buffer != NULL) {
		gdk_pixbuf_unref (global_buffer);
		global_buffer = NULL;
	}
}

GdkPixbuf *
eel_gdk_pixbuf_get_global_buffer (int minimum_width,
				  int minimum_height)
{
	static gboolean at_exit_deallocator_installed = FALSE;

	g_return_val_if_fail (minimum_width > 0, NULL);
	g_return_val_if_fail (minimum_height > 0, NULL);

	if (global_buffer != NULL) {
		if (gdk_pixbuf_get_width (global_buffer) >= minimum_width
		    && gdk_pixbuf_get_height (global_buffer) >= minimum_height) {
			return global_buffer;
		}

		destroy_global_buffer ();		
	}

	g_assert (global_buffer == NULL);

	global_buffer = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 
					minimum_width, minimum_height);


	if (at_exit_deallocator_installed == FALSE) {
		at_exit_deallocator_installed = TRUE;
		g_atexit (destroy_global_buffer);
	}

	return global_buffer;
}

/* Same as gdk_pixbuf_get_from_drawable() except it deals with 
 * race conditions and other evil things that can happen */
GdkPixbuf *
eel_gdk_pixbuf_get_from_window_safe (GdkWindow *window,
				     int x,
				     int y,
				     int width,
				     int height)
{
	GdkWindowPrivate *window_private;
	GdkPixbuf *pixbuf;
	int error_code;
	GdkWindowType save_window_type;
	GdkColormap *colormap;

	g_return_val_if_fail (window != NULL, NULL);


	/* Push an error handler so that we can catch
	 * the very rare (but possible) case where 
	 * the GetImage() request fails.  See HACK2 below
	 * for a more complete excuse.
	 */
	gdk_error_trap_push ();

	/* Save the window type and colormap.  Otherwise
	 * GdkPixbuf will try to fetch them from a hacked
	 * window.  See HACK1 below for reason why we
	 * hack the private window contents.
	 */
	save_window_type = gdk_window_get_type (window);
	colormap = gdk_window_get_colormap (window);

	/* HACK1:
	 *
	 * This horrible thing we do here is needed to
	 * prevent GdkPixbuf from doing geometry sanity
	 * checks on the window.  By pretending it
	 * is a Pixmap, we fool GdkPixbuf into not doing
	 * these checks.  
	 *
	 * Why ? The sanity checks that GdkPixbuf does
	 * are good most of the time, but not 100% of 
	 * the time.  This is because there is no guarantee
	 * that when the x server gets the GetImage request,
	 * the geometry of that window is what GdkPixbuf
	 * thought it was.
	 *
	 * For example, the window manager could have triggered
	 * a sequence of events causing the window to resize.
	 */
	window_private = (GdkWindowPrivate*) window;
	window_private->window_type = GDK_WINDOW_PIXMAP;

	pixbuf = gdk_pixbuf_get_from_drawable (NULL,
					       window,
					       colormap,
					       x,
					       y,
					       0,
					       0,
					       width,
					       height);

	/* Restore the window's guts */
	window_private->window_type = save_window_type;
		
	/* HACK2:
	 *
	 * Now we pop the error handler and see whether an error
	 * occured.
	 * 
	 * It is very rare that an error might occur.  The conditions
	 * under which it might are:
	 *
	 * 1) Race condition as described above in HACK1
	 *
	 * 2) Bogus coordinates and/or dimensions given to
	 *    the GetImage() request - which GdkPixbuf cant
	 *    safely sanity check against.
	 *
	 * So, if we get an error, we simply drop this request on
	 * the floor and return NULL.  The caller needs to deal with
	 * the fact that their request couldnt be executed.  Most of
	 * the time, all that is needed is to simply ignore it.
	 */
	error_code = gdk_error_trap_pop ();
	
	if (error_code != 0) {
		/* HACK3:
		 *
		 * The magical number "8" is the minor x error request
		 * code.  That is the only error we are expecting.
		 *
		 * Otherwise we still return NULL, but the caller
		 * gets a critical.
		 */
		g_return_val_if_fail (error_code == 8, NULL);

		return NULL;
	}

	return pixbuf;
}

/**
 * eel_gdk_pixbuf_intersect:
 * @pixbuf: A GdkPixbuf.
 * @pixbuf_x: X coordinate of pixbuf.
 * @pixbuf_y: Y coordinate of pixbuf.
 * @rectangle: An ArtIRect or eel_gdk_pixbuf_whole_pixbuf.
 *
 * Return value: The intersection of the pixbuf and the given rectangle.
 *
 * If &rectangle is eel_gdk_pixbuf_whole_pixbuf, then the resulting
 * rectangle is a rectangle at the given orign with the pixbuf's dimensions.
 *
 */
ArtIRect
eel_gdk_pixbuf_intersect (const GdkPixbuf *pixbuf,
			  int pixbuf_x,
			  int pixbuf_y,
			  ArtIRect rectangle)
{
	ArtIRect intersection;
	ArtIRect bounds;
	EelDimensions dimensions;

	g_return_val_if_fail (eel_gdk_pixbuf_is_valid (pixbuf), eel_art_irect_empty);

	dimensions = eel_gdk_pixbuf_get_dimensions (pixbuf);
	bounds = eel_art_irect_assign_dimensions (pixbuf_x, pixbuf_y, dimensions);

	if (eel_art_irect_equal (rectangle, eel_gdk_pixbuf_whole_pixbuf)) {
		return bounds;
	}

	art_irect_intersect (&intersection, &rectangle, &bounds);

	/* In theory, this is not needed because a rectangle is empty
	 * regardless of how MUCH negative the dimensions are.  
	 * However, to make debugging and self checks simpler, we
	 * consistenly return a standard empty rectangle.
	 */
	if (art_irect_empty (&intersection)) {
		return eel_art_irect_empty;
	}

	return intersection;
}

#if !defined (EEL_OMIT_SELF_CHECK)

static char *
check_average_value (int width, int height, const char* fill)
{
	char c;
	guint r, g, b, a;
	gboolean alpha, gray;
	int gray_tweak;
	GdkPixbuf *pixbuf;
	int x, y, rowstride, n_channels;
	guchar *pixels;
	guint32 average;
	guchar v;

	r = g = b = a = 0;
	alpha = FALSE;
	gray = FALSE;
	gray_tweak = 0;
	if (sscanf (fill, " %x,%x,%x,%x %c", &r, &g, &b, &a, &c) == 4) {
		alpha = TRUE;
	} else if (sscanf (fill, " %x,%x,%x %c", &r, &g, &b, &c) == 3) {
	} else if (sscanf (fill, " gray%d %c", &gray_tweak, &c) == 1) {
		gray = TRUE;
	} else {
		return g_strdup ("bad fill string format");
	}

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, alpha, 8, width, height);

	pixels = gdk_pixbuf_get_pixels (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	n_channels = gdk_pixbuf_get_n_channels (pixbuf);

	if (!gray) {
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				pixels [y * rowstride + x * n_channels + 0] = r;
				pixels [y * rowstride + x * n_channels + 1] = g;
				pixels [y * rowstride + x * n_channels + 2] = b;
				if (alpha) {
					pixels [y * rowstride + x * n_channels + 3] = a;
				}
			}
		}
	} else {
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				v = ((x + y) & 1) ? 0x80 : 0x7F;
				if (((x + y) & 0xFF) == 0)
					v += gray_tweak;
				pixels [y * rowstride + x * n_channels + 0] = v;
				pixels [y * rowstride + x * n_channels + 1] = v;
				pixels [y * rowstride + x * n_channels + 2] = v;
			}
		}
		pixels [0] += gray_tweak;
		pixels [1] += gray_tweak;
		pixels [2] += gray_tweak;
	}

	average = eel_gdk_pixbuf_average_value_argb (pixbuf);
	gdk_pixbuf_unref (pixbuf);

	return g_strdup_printf ("%02X,%02X,%02X,%02X",
				(average >> 16) & 0xFF,
				(average >> 8) & 0xFF,
				average & 0xFF,
				average >> 24);
}

void
eel_self_check_gdk_pixbuf_extensions (void)
{
	GdkPixbuf *pixbuf;
	ArtIRect clip_area;

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 100, 100);

	EEL_CHECK_BOOLEAN_RESULT (eel_gdk_pixbuf_is_valid (pixbuf), TRUE);
	EEL_CHECK_BOOLEAN_RESULT (eel_gdk_pixbuf_is_valid (NULL), FALSE);

	EEL_CHECK_DIMENSIONS_RESULT (eel_gdk_pixbuf_get_dimensions (pixbuf), 100, 100);

	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, eel_gdk_pixbuf_whole_pixbuf), 0, 0, 100, 100);

	clip_area = eel_art_irect_assign (0, 0, 0, 0);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 0, 0, 0, 0);

	clip_area = eel_art_irect_assign (0, 0, 0, 0);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 0, 0, 0, 0);

	clip_area = eel_art_irect_assign (0, 0, 100, 100);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 0, 0, 100, 100);

	clip_area = eel_art_irect_assign (-10, -10, 100, 100);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 0, 0, 90, 90);

	clip_area = eel_art_irect_assign (-10, -10, 110, 110);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 0, 0, 100, 100);

	clip_area = eel_art_irect_assign (0, 0, 99, 99);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 0, 0, 99, 99);

	clip_area = eel_art_irect_assign (0, 0, 1, 1);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 0, 0, 1, 1);

	clip_area = eel_art_irect_assign (-1, -1, 1, 1);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 0, 0, 0, 0);

	clip_area = eel_art_irect_assign (-1, -1, 2, 2);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 0, 0, 1, 1);

	clip_area = eel_art_irect_assign (100, 100, 1, 1);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 0, 0, 0, 0);

	clip_area = eel_art_irect_assign (101, 101, 1, 1);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 0, 0, 0, 0);

	clip_area = eel_art_irect_assign (80, 0, 100, 100);
	EEL_CHECK_RECTANGLE_RESULT (eel_gdk_pixbuf_intersect (pixbuf, 0, 0, clip_area), 80, 0, 100, 100);

	gdk_pixbuf_unref (pixbuf);

	/* No checks for empty pixbufs because GdkPixbuf doesn't seem to allow them. */
	EEL_CHECK_STRING_RESULT (check_average_value (1, 1, "00,00,00"), "00,00,00,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1, 1, "00,00,00,00"), "00,00,00,00");
	EEL_CHECK_STRING_RESULT (check_average_value (1, 1, "00,00,00,FF"), "00,00,00,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1, 1, "01,01,01"), "01,01,01,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1, 1, "FE,FE,FE"), "FE,FE,FE,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1, 1, "FF,FF,FF"), "FF,FF,FF,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1, 1, "FF,FF,FF,00"), "00,00,00,00");
	EEL_CHECK_STRING_RESULT (check_average_value (1, 1, "11,22,33"), "11,22,33,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1000, 1000, "00,00,00"), "00,00,00,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1000, 1000, "00,00,00,00"), "00,00,00,00");
	EEL_CHECK_STRING_RESULT (check_average_value (1000, 1000, "00,00,00,FF"), "00,00,00,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1000, 1000, "01,01,01"), "01,01,01,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1000, 1000, "FE,FE,FE"), "FE,FE,FE,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1000, 1000, "FF,FF,FF"), "FF,FF,FF,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1000, 1000, "FF,FF,FF,00"), "00,00,00,00");
	EEL_CHECK_STRING_RESULT (check_average_value (1000, 1000, "11,22,33"), "11,22,33,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1000, 1000, "gray -1"), "7F,7F,7F,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1000, 1000, "gray 0"), "80,80,80,FF");
	EEL_CHECK_STRING_RESULT (check_average_value (1000, 1000, "gray 1"), "80,80,80,FF");
}

#endif /* !EEL_OMIT_SELF_CHECK */
