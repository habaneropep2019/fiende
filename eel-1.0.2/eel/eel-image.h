/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-image.h - A widget to smoothly display images.

   Copyright (C) 1999, 2000 Eazel, Inc.

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

#ifndef EEL_IMAGE_H
#define EEL_IMAGE_H

#include <gtk/gtkmisc.h>
#include <libgnome/gnome-defs.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/art_rect.h>
#include <eel/eel-smooth-widget.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_IMAGE            (eel_image_get_type ())
#define EEL_IMAGE(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_IMAGE, EelImage))
#define EEL_IMAGE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_IMAGE, EelImageClass))
#define EEL_IS_IMAGE(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_IMAGE))
#define EEL_IS_IMAGE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_IMAGE))

typedef struct EelImage		  EelImage;
typedef struct EelImageClass	  EelImageClass;
typedef struct EelImageDetails	  EelImageDetails;

struct EelImage
{
	/* Superclass */
	GtkMisc misc;

	/* Private things */
	EelImageDetails *details;
};

struct EelImageClass
{
	GtkMiscClass parent_class;

	EelSmoothWidgetDrawBackground draw_background;
	EelSmoothWidgetSetIsSmooth set_is_smooth;
};

GtkType                 eel_image_get_type                       (void);
GtkWidget *             eel_image_new                            (const char              *file_name);
GtkWidget *             eel_image_new_solid                      (GdkPixbuf               *pixbuf,
								  float                    xalign,
								  float                    yalign,
								  int                      xpadding,
								  int                      ypadding,
								  guint32                  background_color,
								  GdkPixbuf               *tile_pixbuf);
void                    eel_image_set_is_smooth                  (EelImage                *image,
								  gboolean                 is_smooth);
gboolean                eel_image_get_is_smooth                  (const EelImage          *image);
void                    eel_image_set_tile_pixbuf                (EelImage                *image,
								  GdkPixbuf               *pixbuf);
void                    eel_image_set_tile_width                 (EelImage                *image,
								  int                      tile_width);
int                     eel_image_get_tile_width                 (const EelImage          *image);
void                    eel_image_set_tile_height                (EelImage                *image,
								  int                      tile_height);
int                     eel_image_get_tile_height                (const EelImage          *image);
void                    eel_image_set_tile_pixbuf_from_file_name (EelImage                *image,
								  const char              *tile_file_name);
GdkPixbuf*              eel_image_get_tile_pixbuf                (const EelImage          *image);
void                    eel_image_set_tile_opacity               (EelImage                *image,
								  int                      tile_opacity);
int                     eel_image_get_tile_opacity               (const EelImage          *image);
void                    eel_image_set_tile_mode_vertical         (EelImage                *image,
								  EelSmoothTileMode        horizontal_tile_mode);
EelSmoothTileMode       eel_image_get_tile_mode_vertical         (const EelImage          *image);
void                    eel_image_set_tile_mode_horizontal       (EelImage                *image,
								  EelSmoothTileMode        horizontal_tile_mode);
EelSmoothTileMode       eel_image_get_tile_mode_horizontal       (const EelImage          *image);
void                    eel_image_set_pixbuf                     (EelImage                *image,
								  GdkPixbuf               *pixbuf);
void                    eel_image_set_pixbuf_from_file_name      (EelImage                *image,
								  const char              *file_name);
GdkPixbuf*              eel_image_get_pixbuf                     (const EelImage          *image);
void                    eel_image_set_pixbuf_opacity             (EelImage                *image,
								  int                      pixbuf_opacity);
int                     eel_image_get_pixbuf_opacity             (const EelImage          *image);
void                    eel_image_set_pixbuf_insensitive_opacity (EelImage                *image,
								  int                      pixbuf_insensitive_opacity);
int                     eel_image_get_pixbuf_insensitive_opacity (const EelImage          *image);
void                    eel_image_set_background_mode            (EelImage                *image,
								  EelSmoothBackgroundMode  background_mode);
EelSmoothBackgroundMode eel_image_get_background_mode            (const EelImage          *image);
void                    eel_image_set_solid_background_color     (EelImage                *image,
								  guint32                  solid_background_color);
guint32                 eel_image_get_solid_background_color     (const EelImage          *image);
void                    eel_image_set_never_smooth               (EelImage                *image,
								  gboolean                 never_smooth);
END_GNOME_DECLS

#endif /* EEL_IMAGE_H */


