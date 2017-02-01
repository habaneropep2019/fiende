/* bonobo-ui-pixmap-cache.h: Pixmap cache for the Bonobo UI engine
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

#include "bonobo-ui-pixmap-cache.h"



/* Pair of pixmap/mask from a rendered pixbuf */
typedef struct {
	GdkPixmap *pixmap;
	GdkBitmap *mask;
} PixmapMask;

/* Mapping of pixbuf pointers to PixmapMask pairs */
static GHashTable *pixbuf_pixmap_hash = NULL;



/* Ensures that the pixbuf->pixmap hash table has been created */
static void
ensure_hash_table (void)
{
	if (pixbuf_pixmap_hash)
		return;

	pixbuf_pixmap_hash = g_hash_table_new (g_direct_hash, g_direct_equal);
}

/**
 * bonobo_ui_pixmap_cache_get:
 * @source: Source pixbuf.
 * @pixmap_ret: Return value for the rendered pixmap.
 * @mask_ret: Return value for the opacity mask.
 * 
 * Gets a rendered pixmap/mask pair from a pixbuf.  The pixmap and mask are
 * cached based on the @source pixbuf pointer so that no duplicated rendering is
 * performed.  You should unref the returned pixmap and mask with
 * gdk_pixmap_unref() and gdk_bitmap_unref(), respectively, when you are done
 * with them.
 **/
void
bonobo_ui_pixmap_cache_get (GdkPixbuf *source, GdkPixmap **pixmap_ret, GdkBitmap **mask_ret)
{
	PixmapMask *pm;

	g_return_if_fail (source != NULL);
	g_return_if_fail (pixmap_ret != NULL);
	g_return_if_fail (mask_ret != NULL);

	ensure_hash_table ();

	pm = g_hash_table_lookup (pixbuf_pixmap_hash, source);

	if (!pm) {
		pm = g_new (PixmapMask, 1);
		gdk_pixbuf_render_pixmap_and_mask (source, &pm->pixmap, &pm->mask, 128);
		g_hash_table_insert (pixbuf_pixmap_hash, source, pm);
	}

	if (pm->pixmap)
		gdk_pixmap_ref (pm->pixmap);

	if (pm->mask)
		gdk_bitmap_ref (pm->mask);

	*pixmap_ret = pm->pixmap;
	*mask_ret = pm->mask;
}
