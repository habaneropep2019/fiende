/* bonobo-ui-icon-modify.h: Icon modifier for the Bonobo UI engine
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Author: Federico Mena-Quintero <federico@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bonobo-ui-icon-modify.h"



/**
 * bonobo_ui_icon_modify:
 * @source: A pixbuf.
 * @saturation: Saturation value for the new image.  1.0 means full saturation and
 * leaves the original colors untouched.  0.0 means full desaturation and effectively
 * converts the image to grayscale.
 * @pixelate: Whether to apply a 50% dark stipple pattern to the destination image.
 * @pixelation_dark_factor: If @pixelate is TRUE, then the stipple pattern will
 * be created using the colors of the original image darkened by this factor.
 * 0.0 is completely black and 1.0 leaves the original colors untouched.
 * 
 * Modifies a @source pixbuf for use as a menu/toolbar icon by optionally
 * desaturating and applying a dark stipple pattern to it.
 * 
 * Return value: The generated, modified image or NULL if it could not be
 * created.
 **/
GdkPixbuf *
bonobo_ui_icon_modify (GdkPixbuf *source, double saturation,
		       gboolean pixelate, double pixelation_dark_factor)
{
	GdkPixbuf *dest;
	int width, height, source_rowstride, n_channels;
	int dest_rowstride;
	gboolean has_alpha;
	int x, y;
	guchar *src_row, *dest_row;

	g_return_val_if_fail (source != NULL, NULL);
	g_return_val_if_fail (gdk_pixbuf_get_colorspace (source) == GDK_COLORSPACE_RGB, NULL);

	n_channels = gdk_pixbuf_get_n_channels (source);
	has_alpha = gdk_pixbuf_get_has_alpha (source);

	g_assert ((!has_alpha && n_channels == 3) || (has_alpha && n_channels == 4));

	width = gdk_pixbuf_get_width (source);
	height = gdk_pixbuf_get_height (source);
	source_rowstride = gdk_pixbuf_get_rowstride (source);

	dest = gdk_pixbuf_new (GDK_COLORSPACE_RGB, has_alpha, 8, width, height);
	if (!dest)
		return NULL;

	dest_rowstride = gdk_pixbuf_get_rowstride (dest);

	src_row = gdk_pixbuf_get_pixels (source);
	dest_row = gdk_pixbuf_get_pixels (dest);

	for (y = 0; y < height; y++) {
		guchar *s, *d;

		s = src_row;
		d = dest_row;

		for (x = 0; x < width; x++) {
			int r, g, b;

			r = *s++;
			g = *s++;
			b = *s++;

			/* Desaturate */

			if (saturation != 1.0) {
				int intensity;

				intensity = r * 0.30 + g * 0.59 + b * 0.11 + 0.5;

				r = intensity + (r - intensity) * saturation + 0.5;
				g = intensity + (g - intensity) * saturation + 0.5;
				b = intensity + (b - intensity) * saturation + 0.5;
			}

			/* Pixelate if needed */

			if (pixelate && ((x + y) % 2) == 0) {
				r = r * pixelation_dark_factor + 0.5;
				g = g * pixelation_dark_factor + 0.5;
				b = b * pixelation_dark_factor + 0.5;
			}

			/* Set destination pixel */
			
			*d++ = CLAMP (r, 0, 255);
			*d++ = CLAMP (g, 0, 255);
			*d++ = CLAMP (b, 0, 255);

			if (has_alpha)
				*d++ = *s++;
		}

		src_row += source_rowstride;
		dest_row += dest_rowstride;
	}

	return dest;
}
