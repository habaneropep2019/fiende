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
#include "eel-background-canvas-group.h"

#include <libgnomeui/gnome-canvas.h>
#include <libgnomeui/gnome-canvas-util.h>
#include <libart_lgpl/art_rgb_affine.h>
#include <libart_lgpl/art_rgb_rgba_affine.h>

#include "eel-background.h"
#include "eel-gdk-extensions.h"
#include "eel-gtk-macros.h"

#include <stdio.h>

static void eel_background_canvas_group_initialize_class (gpointer         klass);
static void eel_background_canvas_group_initialize       (gpointer         object,
							  gpointer         klass);
static void eel_background_canvas_group_draw             (GnomeCanvasItem *item,
							  GdkDrawable     *drawable,
							  int              x,
							  int              y,
							  int              width,
							  int              height);
static void eel_background_canvas_group_render           (GnomeCanvasItem *item,
							  GnomeCanvasBuf  *buffer);

typedef GnomeCanvasGroup EelBackgroundCanvasGroup;
typedef GnomeCanvasGroupClass EelBackgroundCanvasGroupClass;

#define EEL_TYPE_BACKGROUND_CANVAS_GROUP \
	(eel_background_canvas_group_get_type ())
#define EEL_BACKGROUND_CANVAS_GROUP(obj) \
	(GTK_CHECK_CAST ((obj), EEL_TYPE_BACKGROUND_CANVAS_GROUP, EelBackgroundCanvasGroup))
#define EEL_BACKGROUND_CANVAS_GROUP_CLASS(klass) \
	(GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_BACKGROUND_CANVAS_GROUP, EelBackgroundCanvasGroupClass))
#define EEL_IS_BACKGROUND_CANVAS_GROUP(obj) \
	(GTK_CHECK_TYPE ((obj), EEL_TYPE_BACKGROUND_CANVAS_GROUP))
#define EEL_IS_BACKGROUND_CANVAS_GROUP_CLASS(klass) \
	(GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_BACKGROUND_CANVAS_GROUP))

GtkType eel_background_canvas_group_get_type (void);

EEL_DEFINE_CLASS_BOILERPLATE (EelBackgroundCanvasGroup, eel_background_canvas_group, GNOME_TYPE_CANVAS_GROUP)

static void
eel_background_canvas_group_initialize_class (gpointer klass)
{
	GNOME_CANVAS_ITEM_CLASS (klass)->draw   = eel_background_canvas_group_draw;
	GNOME_CANVAS_ITEM_CLASS (klass)->render = eel_background_canvas_group_render;
}

/* This function is for initialization code that's needed both when we're allocating
 * a new EelBackgroundCanvasGroup object as well as when we're taking over an existing
 * GnomeCanvasGroup item (replacing its klass).
 */
static void
eel_background_canvas_group_initialize_common (EelBackgroundCanvasGroup *canvas_group)
{
#ifdef HAVE_GNOME_CANVAS_SET_DITHER
	gnome_canvas_set_dither (GNOME_CANVAS_ITEM (canvas_group)->canvas, GDK_RGB_DITHER_MAX);
#endif
}

static void
eel_background_canvas_group_initialize (gpointer object, gpointer klass)
{
	/* The way we currently use eel_background_canvas_group, assigning
	 * it to the klass of a root canvas object, circumvents this initialze fn.
	 */
	eel_background_canvas_group_initialize_common (EEL_BACKGROUND_CANVAS_GROUP (object));
}


void
eel_background_canvas_group_supplant_root_class (GnomeCanvas *canvas)
{
	/* Attach ourselves to a canvas in a way that will work.
	   Changing the style is not sufficient.

	   Since there's no signal to override in GnomeCanvas to control
	   drawing the background, we change the class of the canvas root.
	   This gives us a chance to draw the background before any of the
	   objects draw themselves, and has no effect on the bounds or
	   anything related to scrolling.

	   We settled on this after less-than-thrilling results using a
	   canvas item as the background. The canvas item contributed to
	   the bounds of the canvas and had to constantly be resized.
	*/
	
	g_assert (GNOME_IS_CANVAS (canvas));
	
	if (GTK_OBJECT (canvas->root)->klass != gtk_type_class (EEL_TYPE_BACKGROUND_CANVAS_GROUP)) {
	
		g_assert (GTK_OBJECT (canvas->root)->klass == gtk_type_class (GNOME_TYPE_CANVAS_GROUP));
	
		GTK_OBJECT (canvas->root)->klass =
			gtk_type_class (EEL_TYPE_BACKGROUND_CANVAS_GROUP);

		eel_background_canvas_group_initialize_common (EEL_BACKGROUND_CANVAS_GROUP (canvas->root));
	}
}

static void
eel_background_canvas_group_draw (GnomeCanvasItem *item,
				       GdkDrawable *drawable,
				       int x,
				       int y,
				       int width,
				       int height)
{
	EelBackground *background;
	GdkGC *gc;

	/* Draw the background. */
	background = eel_get_widget_background (GTK_WIDGET (item->canvas));

	/* If GtkStyle handled it, then we don't want to bother doing
	 * any additional work. It would be way slow to draw again.
	 */
	if (eel_background_is_too_complex_for_gtk_style (background)) {
		/* Create a new gc each time.
		 * If this is a speed problem, we can create one and keep it around,
		 * but it's a bit more complicated to ensure that it's always compatible
		 * with whatever drawable is passed in.
		 */
		gc = gdk_gc_new (drawable);

		eel_background_pre_draw (background,
					      GTK_WIDGET (item->canvas)->allocation.width,
					      GTK_WIDGET (item->canvas)->allocation.height);

		eel_background_draw (background, drawable, gc, x, y, 0, 0, width, height);

		gdk_gc_unref (gc);
	}

	/* Call through to the GnomeCanvasGroup implementation, which
	 * will draw all the canvas items.
	 */
	EEL_CALL_PARENT (GNOME_CANVAS_ITEM_CLASS, draw, (item, drawable, x, y, width, height));				     
}


/* draw the background for the anti-aliased canvas case */
static void
eel_background_canvas_group_render (GnomeCanvasItem *item, GnomeCanvasBuf *buffer)
{
	EelBackground *background;
			
	background = eel_get_widget_background (GTK_WIDGET (item->canvas));
	if (background != NULL) {
		eel_background_pre_draw (background,
					      GTK_WIDGET (item->canvas)->allocation.width,
					      GTK_WIDGET (item->canvas)->allocation.height);

		eel_background_draw_aa (background, buffer);
	}
	
	/* Call through to the GnomeCanvasGroup implementation, which will draw all
	 * the canvas items.
	 */
	EEL_CALL_PARENT (GNOME_CANVAS_ITEM_CLASS, render, (item, buffer));
}
