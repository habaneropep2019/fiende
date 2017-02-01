/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   eel-background.h: Object for the background of a widget.

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
  
   Authors: Darin Adler <darin@eazel.com>
*/

#ifndef EEL_BACKGROUND_H
#define EEL_BACKGROUND_H

/* Windows for Eel can contain backgrounds that are either tiled
   with an image, a solid color, or a color gradient. This class manages
   the process of loading the image if necessary and parsing the string
   that specifies either a color or color gradient.

   The color or gradient is always present, even if there's a tiled image
   on top of it. This is used when the tiled image can't be loaded for
   some reason (or just has not been loaded yet).

   The EelBackground object is easier to modify than a GtkStyle.
   You can just call eel_get_window_background and modify the
   returned background directly, unlike a style, which must be copied,
   modified and then set.
*/

#include <gdk/gdktypes.h>
#include <gtk/gtkwidget.h>

#include <libgnomeui/gnome-canvas.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct EelBackground EelBackground;
typedef struct EelBackgroundClass EelBackgroundClass;

#define EEL_TYPE_BACKGROUND \
	(eel_background_get_type ())
#define EEL_BACKGROUND(obj) \
	(GTK_CHECK_CAST ((obj), EEL_TYPE_BACKGROUND, EelBackground))
#define EEL_BACKGROUND_CLASS(klass) \
	(GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_BACKGROUND, EelBackgroundClass))
#define EEL_IS_BACKGROUND(obj) \
	(GTK_CHECK_TYPE ((obj), EEL_TYPE_BACKGROUND))
#define EEL_IS_BACKGROUND_CLASS(klass) \
	(GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_BACKGROUND))

typedef enum {
	EEL_BACKGROUND_TILED = 0, /* zero makes this the default placement */
	EEL_BACKGROUND_CENTERED,
	EEL_BACKGROUND_SCALED,
	EEL_BACKGROUND_SCALED_ASPECT
} EelBackgroundImagePlacement;

GtkType                     eel_background_get_type                         (void);
EelBackground *             eel_background_new                              (void);


/* Calls to change a background. */
void                        eel_background_set_color                        (EelBackground               *background,
									     const char                  *color_or_gradient);
void                        eel_background_set_image_uri                    (EelBackground               *background,
									     const char                  *image_uri);
void                        eel_background_reset                            (EelBackground               *background);
void                        eel_background_set_image_placement              (EelBackground               *background,
									     EelBackgroundImagePlacement  placement);
/* Calls to interrogate the current state of a background. */
char *                      eel_background_get_color                        (EelBackground               *background);
char *                      eel_background_get_image_uri                    (EelBackground               *background);
EelBackgroundImagePlacement eel_background_get_image_placement              (EelBackground               *background);
gboolean                    eel_background_is_dark                          (EelBackground               *background);
gboolean                    eel_background_is_set                           (EelBackground               *background);
gboolean                    eel_background_is_loaded                        (EelBackground               *background);

/* For preping the background to be used in one of the two calls
 * below. Only intended to be called by eel_background_canvas_group_update.
 */
void                        eel_background_pre_draw                         (EelBackground               *background,
									     int                          entire_width,
									     int                          entire_height);
/* For updating the canvas, non-aa case. Note: eel_background_pre_draw
 * must have been previously called. Only intended to be called by
 * eel_background_canvas_group_draw.
 */
void                        eel_background_draw                             (EelBackground               *background,
									     GdkDrawable                 *drawable,
									     GdkGC                       *gc,
									     int                          src_x,
									     int                          src_y,
									     int                          dest_x,
									     int                          dest_y,
									     int                          dest_width,
									     int                          dest_height);
/* For updating the canvas, aa case. Note: eel_background_pre_draw
 * must have been previously called. Only intended to be called by
 * eel_background_canvas_group_render.
 */
void                        eel_background_draw_aa                          (EelBackground               *background,
									     GnomeCanvasBuf              *buffer);
/* Used to fill a drawable with a background.
 *  - entire_width/height describe the total area the background covers
 *  - drawable_x/y/width/height describe the portion of that area the drawable covers
 */
void                        eel_background_draw_to_drawable                 (EelBackground               *background,
									     GdkDrawable                 *drawable,
									     GdkGC                       *gc,
									     int                          drawable_x,
									     int                          drawable_y,
									     int                          drawable_width,
									     int                          drawable_height,
									     int                          entire_width,
									     int                          entire_height);

/* Used to fill a drawable with a background.
 *  - entire_width/height describe the total area the background covers
 *  - buffer is a portion of that area
 */
void                        eel_background_draw_to_canvas                   (EelBackground               *background,
									     GnomeCanvasBuf              *buffer,
									     int                          entire_width,
									     int                          entire_height);
/* Used to fill a pixbuf with a background.
 *  - entire_width/height describe the total area the background covers
 *  - drawable_x/y/width/height describe the portion of that area the pixbuf covers
 */
void                        eel_background_draw_to_pixbuf                   (EelBackground               *background,
									     GdkPixbuf                   *pixbuf,
									     int                          pixbuf_x,
									     int                          pixbuf_y,
									     int                          pixbuf_width,
									     int                          pixbuf_height,
									     int                          entire_width,
									     int                          entire_height);
							       
/* Handles a dragged color being dropped on a widget to change the background color. */
void                        eel_background_receive_dropped_color            (EelBackground               *background,
									     GtkWidget                   *widget,
									     int                          drop_location_x,
									     int                          drop_location_y,
									     const GtkSelectionData      *dropped_color);

/* Handles a special-case image name that means "reset to default background" too. */
void                        eel_background_receive_dropped_background_image (EelBackground               *background,
									     const char                  *image_uri);

/* Gets or creates a background so that it's attached to a widget. */
EelBackground *             eel_get_widget_background                       (GtkWidget                   *widget);

/* Return whether a background has beed attatched to the given widget. */
gboolean                    eel_widget_has_attached_background              (GtkWidget                   *widget);

/* Find the background ancestor for the widget. */
GtkWidget *                 eel_gtk_widget_find_background_ancestor         (GtkWidget                   *widget);

/* Find out if a eel background is too complex for GtkStyle, so that we have to draw it ourselves */
gboolean                    eel_background_is_too_complex_for_gtk_style     (EelBackground               *background);


typedef struct EelBackgroundDetails EelBackgroundDetails;

struct EelBackground
{
	GtkObject object;
	EelBackgroundDetails *details;
};

struct EelBackgroundClass
{
	GtkObjectClass parent_class;

	/* This signal is emitted whenever the background settings are
	 * changed.
	 */
	void (* settings_changed) (EelBackground *);

	/* This signal is emitted whenever the appearance of the
	 * background has changed, like when the background settings are
	 * altered or when an image is loaded.
	 */
	void (* appearance_changed) (EelBackground *);

	/* This signal is emitted when image loading is over - whether it
	 * was successfully loaded or not.
	 */
	void (* image_loading_done) (EelBackground *background, gboolean successful_load);

	/* This signal is emitted when the background is reset by receiving
	   the reset property from a drag
	 */
	void (* reset) (EelBackground *);

};

#endif /* EEL_BACKGROUND_H */
