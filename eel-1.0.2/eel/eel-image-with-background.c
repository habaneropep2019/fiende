/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-image-with-background.c - A EelImage that uses EelBackground.

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

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#include <config.h>

#include <gtk/gtksignal.h>

#include "eel-image-with-background.h"
#include "eel-background.h"
#include "eel-gdk-pixbuf-extensions.h"

static void
draw_background_callback (GtkWidget *widget,
			  GdkPixbuf *buffer,
			  const ArtIRect *area,
			  gpointer callback_data)
{
	GtkWidget *background_ancestor;
	EelBackground *background;

	g_return_if_fail (eel_gdk_pixbuf_is_valid (buffer));
	g_return_if_fail (area != NULL);

	background_ancestor = eel_gtk_widget_find_background_ancestor (widget);
	g_return_if_fail (GTK_IS_WIDGET (background_ancestor));

	background = eel_get_widget_background (background_ancestor);
	g_return_if_fail (EEL_IS_BACKGROUND (background));
	
	eel_background_draw_to_pixbuf (background,
				       buffer,
				       area->x0,
				       area->y0,
				       area->x1 - area->x0,
				       area->y1 - area->y0,
				       background_ancestor->allocation.width,
				       background_ancestor->allocation.height);
}

GtkWidget *
eel_image_new_with_background (const char *file_name)
{
	GtkWidget *image;
	
	image = eel_image_new (file_name);

	eel_image_set_background_mode (EEL_IMAGE (image),
					    EEL_SMOOTH_BACKGROUND_CALLBACK);
	
	gtk_signal_connect_while_alive (GTK_OBJECT (image),
					"draw_background",
					GTK_SIGNAL_FUNC (draw_background_callback),
					NULL,
					GTK_OBJECT (image));

	return image;
}
