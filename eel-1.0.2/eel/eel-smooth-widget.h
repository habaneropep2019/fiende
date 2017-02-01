/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-smooth-widget.h - Functions and types shared by smooth widgets.

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

#ifndef EEL_SMOOTH_WIDGET_H
#define EEL_SMOOTH_WIDGET_H

#include <libgnome/gnome-defs.h>
#include <gtk/gtkwidget.h>
#include <eel/eel-art-extensions.h>
#include <eel/eel-gdk-pixbuf-extensions.h>

BEGIN_GNOME_DECLS

/* See eel_smooth_widget_get_tile_bounds() */
#define EEL_SMOOTH_TILE_EXTENT_FULL	-1
#define EEL_SMOOTH_TILE_EXTENT_ONE_STEP	-2

/* See eel_smooth_widget_paint() */
typedef enum
{
	EEL_SMOOTH_BACKGROUND_GTK,
	EEL_SMOOTH_BACKGROUND_CALLBACK,
	EEL_SMOOTH_BACKGROUND_SOLID_COLOR
} EelSmoothBackgroundMode;

/* See eel_smooth_widget_paint() */
typedef enum
{
	EEL_SMOOTH_TILE_SELF,
	EEL_SMOOTH_TILE_ANCESTOR
} EelSmoothTileMode;

/* Prototypes for shared methods.  We declare these here so that
 * the signatures in the smooth widget's class struture dont
 * get out of whack.
 */
typedef void (* EelSmoothWidgetDrawBackground) (GtkWidget *widget,
						GdkPixbuf *buffer,
						ArtIRect area);
typedef void (* EelSmoothWidgetSetIsSmooth) (GtkWidget *widget,
					     gboolean is_smooth);


/* Callback for eel_smooth_widget_paint() used to render the
 * smooth widget's content when not in smooth mode.
 */
typedef void (* EelSmoothPaintOpaqueCallback) (GtkWidget *widget,
					       GdkDrawable *destination_drawable,
					       GdkGC *gc,
					       int source_x,
					       int source_y,
					       ArtIRect area,
					       gpointer callback_data);

/* Callback for eel_smooth_widget_paint() used to composite the
 * smooth widget's content when in smooth mode.
 */
typedef void (* EelSmoothCompositeCallback) (GtkWidget *widget,
					     GdkPixbuf *destination_pixbuf,
					     int source_x,
					     int source_y,
					     ArtIRect area,
					     int opacity,
					     gpointer callback_data);

void          eel_smooth_widget_register                 (GtkWidget                    *widget);
void          eel_smooth_widget_paint                    (GtkWidget                    *widget,
							  GdkGC                        *gc,
							  gboolean                      is_smooth,
							  EelSmoothBackgroundMode       background_mode,
							  guint32                       solid_background_color,
							  const GdkPixbuf              *tile_pixbuf,
							  ArtIRect                      tile_bounds,
							  int                           tile_opacity,
							  EelSmoothTileMode             tile_mode_vertical,
							  EelSmoothTileMode             tile_mode_horizontal,
							  ArtIRect                      content_bounds,
							  int                           content_opacity,
							  ArtIRect                      dirty_area,
							  EelSmoothPaintOpaqueCallback  paint_callback,
							  EelSmoothCompositeCallback    composite_callback,
							  gpointer                      callback_data);
ArtIRect      eel_smooth_widget_get_tile_bounds          (const GtkWidget              *widget,
							  const GdkPixbuf              *tile_pixbuf,
							  int                           tile_width,
							  int                           tile_height);
EelDimensions eel_smooth_widget_get_preferred_dimensions (const GtkWidget              *widget,
							  EelDimensions                 content_dimensions,
							  EelDimensions                 tile_dimensions,
							  int                           tile_width,
							  int                           tile_height);
void          eel_smooth_widget_register_type            (GtkType                       type);
void          eel_smooth_widget_global_set_is_smooth     (gboolean                      is_smooth);

END_GNOME_DECLS

#endif /* EEL_SMOOTH_WIDGET_H */


