/* bonobo-ui-state-cache.h: Image state cache for the Bonobo UI engine
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

#include "bonobo-ui-state-cache.h"
#include "bonobo-ui-icon-modify.h"



/* Structure to hold image modification parameters for the different widget states */
typedef struct {
	double saturation;
	gboolean pixelate;
	double pixelate_dark_factor;
} Modifiers;

/* A provided pixbuf and its modifiers; used as keys in the cache hash table */
typedef struct {
	GdkPixbuf *pixbuf;
	Modifiers mods;
} PixbufModifiers;

/* Defaults for icon desaturation/pixelation */
const static Modifiers defaults[] = {
	{ 1.0, FALSE, 1.0 },	/* GTK_STATE_NORMAL */
	{ 1.0, FALSE, 1.0 },	/* GTK_STATE_ACTIVE */
	{ 1.0, FALSE, 1.0 },	/* GTK_STATE_PRELIGHT */
	{ 1.0, FALSE, 1.0 },	/* GTK_STATE_SELECTED */
	{ 0.8, TRUE, 0.7 },	/* GTK_STATE_INSENSITIVE */
};



/* Number of GdkStateType values */
#define NUM_STATES 5

/* Mapping of PixbufModfier structures to modified pixbufs */
static GHashTable *pixbuf_modifier_hash = NULL;



/* Hash function for a PixbufModifiers key. */
static guint
pixbuf_modifier_hash_fn (gconstpointer key)
{
	const PixbufModifiers *pm;
	int saturation, pixelate_dark_factor;
	guint hash;

	pm = key;

	/* Convert the doubles to reasonable ints for hashing */
	saturation = G_MAXINT * pm->mods.saturation;
	pixelate_dark_factor = G_MAXINT * pm->mods.pixelate_dark_factor;

	hash = GPOINTER_TO_UINT (pm->pixbuf) ^ saturation ^ pixelate_dark_factor;

	if (pm->mods.pixelate)
		hash = ~hash;

	return hash;
}

/* Comparison function for the PixbufModifiers hash table */
static gint
pixbuf_modifier_equal_fn (gconstpointer a, gconstpointer b)
{
	const PixbufModifiers *pma, *pmb;

	pma = a;
	pmb = b;

	return (pma->pixbuf == pmb->pixbuf
		&& pma->mods.saturation == pmb->mods.saturation
		&& pma->mods.pixelate == pmb->mods.pixelate
		&& pma->mods.pixelate_dark_factor == pmb->mods.pixelate_dark_factor);
}

/* Ensures that the pixbuf->set hash table has been created */
static void
ensure_hash_table (void)
{
	if (pixbuf_modifier_hash)
		return;

	pixbuf_modifier_hash = g_hash_table_new (pixbuf_modifier_hash_fn, pixbuf_modifier_equal_fn);
}

/* Generates a pixbuf out of the specified one by modifying it as appropriate
 * for a widget state.
 */
static GdkPixbuf *
make_pixbuf_for_state (GdkPixbuf *source, GtkStateType state)
{
	return bonobo_ui_icon_modify (source,
				      defaults[state].saturation,
				      defaults[state].pixelate,
				      defaults[state].pixelate_dark_factor);
}

/**
 * bonobo_ui_state_cache_get:
 * @source: Source pixbuf.
 * @state: Widget state for the result.
 *
 * Takes a @source pixbuf and modifies it as appropriate for display in the
 * specified widget @state.  For example, an image could be grayed out for
 * insensitive widgets.  The returned pixbuf comes from a cache; you should
 * unref it with gdk_pixbuf_unref() when you are done with it.
 *
 * Return value: A pixbuf, or NULL if the modified image could not be created.
 **/
GdkPixbuf *
bonobo_ui_state_cache_get (GdkPixbuf *source, GtkStateType state)
{
	PixbufModifiers pm;
	GdkPixbuf *modified;

	g_return_val_if_fail (source != NULL, NULL);
	g_return_val_if_fail (state >= GTK_STATE_NORMAL && state <= GTK_STATE_INSENSITIVE, NULL);

	ensure_hash_table ();

	pm.pixbuf = source;
	pm.mods = defaults[state];

	modified = g_hash_table_lookup (pixbuf_modifier_hash, &pm);

	if (!modified) {
		PixbufModifiers *pm_copy;

		modified = make_pixbuf_for_state (source, state);
		if (!modified)
			return NULL;

		pm_copy = g_new (PixbufModifiers, 1);
		*pm_copy = pm;
		gdk_pixbuf_ref (source);

		g_hash_table_insert (pixbuf_modifier_hash, pm_copy, modified);
	}

	g_assert (modified != NULL);

	gdk_pixbuf_ref (modified);

	return modified;
}
