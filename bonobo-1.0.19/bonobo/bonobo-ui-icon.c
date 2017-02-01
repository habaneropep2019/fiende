/* bonobo-ui-icon.h: Icon widget for the Bonobo UI engine
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

#include "bonobo-ui-icon.h"
#include "bonobo-ui-pixmap-cache.h"
#include "bonobo-ui-state-cache.h"



/* Number of GdkStateType values */
#define NUM_STATES 5

/* Private part of the BonoboUIIcon structure */
struct BonoboUIIconPrivate {
	/* Images for each of the widget states */
	GdkPixbuf *images[NUM_STATES];

	/* Size requisition; it is the maximum of the sizes of the images */
	int req_width, req_height;
};



static void bonobo_ui_icon_class_init (BonoboUIIconClass *class);
static void bonobo_ui_icon_init (BonoboUIIcon *icon);
static void bonobo_ui_icon_destroy (GtkObject *object);

static void bonobo_ui_icon_size_request (GtkWidget *widget, GtkRequisition *requisition);
static gint bonobo_ui_icon_expose_event (GtkWidget *widget, GdkEventExpose *event); 

static GtkWidgetClass *parent_class;



/**
 * bonobo_ui_icon_get_type:
 * 
 * Registers the #BonoboUIIcon class if necessary, and returns the type ID
 * associated to it.
 * 
 * Return value: The type ID of the #BonoboUIIcon class.
 **/
GtkType
bonobo_ui_icon_get_type (void)
{
	static GtkType bonobo_ui_icon_type;

	if (!bonobo_ui_icon_type) {
		static const GtkTypeInfo bonobo_ui_icon_info = {
			"BonoboUIIcon",
			sizeof (BonoboUIIcon),
			sizeof (BonoboUIIconClass),
			(GtkClassInitFunc) bonobo_ui_icon_class_init,
			(GtkObjectInitFunc) bonobo_ui_icon_init,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};

		bonobo_ui_icon_type = gtk_type_unique (GTK_TYPE_WIDGET, &bonobo_ui_icon_info);
	}

	return bonobo_ui_icon_type;
}

/* Class initialization function for the BonoboUIIcon */
static void
bonobo_ui_icon_class_init (BonoboUIIconClass *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) class;
	widget_class = (GtkWidgetClass *) class;

	parent_class = gtk_type_class (GTK_TYPE_WIDGET);

	object_class->destroy = bonobo_ui_icon_destroy;

	widget_class->size_request = bonobo_ui_icon_size_request;
	widget_class->expose_event = bonobo_ui_icon_expose_event;
}

/* Object initialization function for the BonoboUIIcon */
static void
bonobo_ui_icon_init (BonoboUIIcon *icon)
{
	BonoboUIIconPrivate *priv;
	int i;

	priv = g_new (BonoboUIIconPrivate, 1);
	icon->priv = priv;

	GTK_WIDGET_SET_FLAGS (icon, GTK_NO_WINDOW);

	for (i = 0; i < NUM_STATES; i++)
		priv->images[i] = NULL;

	priv->req_width = 0;
	priv->req_height = 0;
}

/* Gets rid of all the image pixbufs in the icon.  Also sets its requisition
 * fields to zero, but does NOT queue a resize.
 */
static void
destroy_images (BonoboUIIcon *icon)
{
	BonoboUIIconPrivate *priv;
	int i;

	priv = icon->priv;

	for (i = 0; i < NUM_STATES; i++)
		if (priv->images[i]) {
			gdk_pixbuf_unref (priv->images[i]);
			priv->images[i] = NULL;
		}

	priv->req_width = 0;
	priv->req_height = 0;
}

/* Destroy handler for the BonoboUIIcon */
static void
bonobo_ui_icon_destroy (GtkObject *object)
{
	BonoboUIIcon *icon;
	BonoboUIIconPrivate *priv;

	g_return_if_fail (object != NULL);
	g_return_if_fail (BONOBO_IS_UI_ICON (object));

	icon = BONOBO_UI_ICON (object);
	priv = icon->priv;

	destroy_images (icon);

	g_free (priv);
	icon->priv = NULL;

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}



/* Size_request method for the BonoboUIIcon */
static void
bonobo_ui_icon_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	BonoboUIIcon *icon;
	BonoboUIIconPrivate *priv;

	icon = BONOBO_UI_ICON (widget);
	priv = icon->priv;

	requisition->width = priv->req_width;
	requisition->height = priv->req_height;
}

/* Expose_event method for the BonoboUIIcon */
static gint
bonobo_ui_icon_expose_event (GtkWidget *widget, GdkEventExpose *event)
{
	BonoboUIIcon *icon;
	BonoboUIIconPrivate *priv;
	GdkPixbuf *image;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	gint width, height;
	int xofs, yofs;
	GdkRectangle pix_area, dest;

	icon = BONOBO_UI_ICON (widget);
	priv = icon->priv;

	if (!GTK_WIDGET_DRAWABLE (widget))
		return TRUE;

	image = priv->images[GTK_WIDGET_STATE (widget)];

	bonobo_ui_pixmap_cache_get (image, &pixmap, &mask);

	if (!pixmap)
		goto out;

	gdk_window_get_size (pixmap, &width, &height);

	/* Compute the offsets for the image centered on our allocation */
	xofs = widget->allocation.x + (widget->allocation.width - width) / 2;
	yofs = widget->allocation.y + (widget->allocation.height - height) / 2;

	pix_area.x = xofs;
	pix_area.y = yofs;
	pix_area.width = width;
	pix_area.height = height;

	if (!gdk_rectangle_intersect (&event->area, &pix_area, &dest))
		goto out;

	if (mask) {
		gdk_gc_set_clip_mask (widget->style->black_gc, mask);
		gdk_gc_set_clip_origin (widget->style->black_gc, xofs, yofs);
	}

	gdk_draw_pixmap (widget->window, widget->style->black_gc,
			 pixmap,
			 dest.x - xofs, dest.y - yofs,
			 dest.x, dest.y,
			 dest.width, dest.height);

	if (mask)
		gdk_gc_set_clip_mask (widget->style->black_gc, NULL);

 out:
	if (pixmap)
		gdk_pixmap_unref (pixmap);

	if (mask)
		gdk_bitmap_unref (mask);

	return TRUE;
}



/**
 * bonobo_ui_icon_new:
 * 
 * Creates a new, empty #BonoboUIIcon widget.  You should set its contents with
 * bonobo_ui_icon_set_images().
 *
 * #BonoboUIIcon is a windowless widget, so it draws on its parent's window.
 * You should ensure that the parent has the GdkRGB visual and colormap set for
 * it, otherwise you will get BadMatch errors from the X server.
 * 
 * Return value: A newly-created #BonoboUIIcon.
 **/
GtkWidget *
bonobo_ui_icon_new (void)
{
	return GTK_WIDGET (gtk_type_new (BONOBO_UI_ICON_TYPE));
}

/**
 * bonobo_ui_icon_set_images:
 * @icon: A #BonoboUIIcon.
 * @images: Set of images to use as a 5-element array, or NULL if no the icon is
 * to be displayed as empty.
 * 
 * Sets the images that a #BonoboUIIcon will use to display itself.  The @images
 * array must have 5 elements, one for each of the GTK+ widget states.  If any
 * of the images is omitted, then that state will use the image for the
 * #GTK_STATE_NORMAL state; this implies that the first image in the array can
 * never be omitted, i.e. the one for the normal state.  This function will keep
 * references to the pixbufs in the @images array, so you can get rid of the
 * pixbufs on your own if you no longer need them after calling this function.
 **/
void
bonobo_ui_icon_set_images (BonoboUIIcon *icon, GdkPixbuf **images)
{
	BonoboUIIconPrivate *priv;
	int width, height;

	g_return_if_fail (icon != NULL);
	g_return_if_fail (BONOBO_IS_UI_ICON (icon));

	priv = icon->priv;

	width = height = 0;

	if (images) {
		int i;

		g_return_if_fail (images[0] != NULL); /* images[GTK_STATE_NORMAL] */

		for (i = 0; i < NUM_STATES; i++) {
			if (images[i]) {
				int w, h;

				gdk_pixbuf_ref (images[i]);

				w = gdk_pixbuf_get_width (images[i]);
				h = gdk_pixbuf_get_height (images[i]);

				if (width < w)
					width = w;

				if (height < h)
					height = h;
			}

			if (priv->images[i])
				gdk_pixbuf_unref (priv->images[i]);

			priv->images[i] = images[i];
		}
	} else
		destroy_images (icon);

	if (width == priv->req_width && height == priv->req_height)
		gtk_widget_queue_draw (GTK_WIDGET (icon));
	else {
		priv->req_width = width;
		priv->req_height = height;
		gtk_widget_queue_resize (GTK_WIDGET (icon));
	}
}

/**
 * bonobo_ui_icon_set_from_pixbuf:
 * @icon: A #BonoboUIIcon
 * @base: Base pixbuf for setting the icon.
 * 
 * This is a convenience function that takes a @base pixbuf and uses
 * bonobo_ui_state_cache_get() to generate all the images that correspond to the
 * different widget states.  It then sets these images on the @icon widget by
 * calling bonobo_ui_icon_set_images().
 * 
 * Return value: TRUE on success.  Will return FALSE if it cannot obtain
 * the image for GTK_STATE_NORMAL; in this case, the icon will be cleared
 * of any images it may already contain.
 **/
gboolean
bonobo_ui_icon_set_from_pixbuf (BonoboUIIcon *icon, GdkPixbuf *base)
{
	GdkPixbuf *images[NUM_STATES];
	int i;

	g_return_val_if_fail (icon != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_UI_ICON (icon), FALSE);
	g_return_val_if_fail (base != NULL, FALSE);

	for (i = 0; i < NUM_STATES; i++) {
		images[i] = bonobo_ui_state_cache_get (base, i);

		if (i == 0 && images[i] == NULL) {
			/* If we can't get the GTK_STATE_NORMAL image, clear the
			 * icon and bail out.
			 */
			bonobo_ui_icon_set_images (icon, NULL);
			return FALSE;
		}
	}

	bonobo_ui_icon_set_images (icon, images);

	for (i = 0; i < NUM_STATES; i++)
		if (images[i])
			gdk_pixbuf_unref (images[i]);

	return TRUE;
}
