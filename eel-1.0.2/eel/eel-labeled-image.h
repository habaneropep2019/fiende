/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-labeled-image.h - A labeled image.

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

/* EelLabeledImage is a container widget.  It can only contain internal 
 * widgets.  These internal widgets are can be a EelLabel and/or a 
 * EelImage.  These internal widgets are created as needed.  That means
 * that EelLabeledImage can always be used for "free" instead of a 
 * EelLabel or EelImage.  The only overhead is that of the GtkObject
 * machinery.
 *
 * The position of the label with respect to the image is controlled by the
 * 'label_positon' attribute.
 *
 * By default the internal image and label widgets are sized to their natural
 * preferred geometry.  You can use the 'fill' attribute of LabeledImage
 * to have the internal widgets fill as much of the LabeledImage allocation
 * as is available.  This is useful if you install a tile_pixbuf and want it
 * to cover the whole widget, and not just the areas occupied by the internal
 * widgets.
 *
 * LabeledImage also has x_padding/y_padding and x_alignment/y_alignment 
 * attributes that behave exaclty as those in the GtkMisc class.
 *
 * Note that the alignments are ignored if the fill attribute is TRUE.
 */

#ifndef EEL_LABELED_IMAGE_H
#define EEL_LABELED_IMAGE_H

#include <gtk/gtkcontainer.h>
#include <eel/eel-label.h>
#include <eel/eel-image.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_LABELED_IMAGE            (eel_labeled_image_get_type ())
#define EEL_LABELED_IMAGE(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_LABELED_IMAGE, EelLabeledImage))
#define EEL_LABELED_IMAGE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_LABELED_IMAGE, EelLabeledImageClass))
#define EEL_IS_LABELED_IMAGE(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_LABELED_IMAGE))
#define EEL_IS_LABELED_IMAGE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_LABELED_IMAGE))

typedef struct EelLabeledImage		  EelLabeledImage;
typedef struct EelLabeledImageClass	  EelLabeledImageClass;
typedef struct EelLabeledImageDetails	  EelLabeledImageDetails;

struct EelLabeledImage
{
	/* Superclass */
	GtkContainer container;

	/* Private things */
	EelLabeledImageDetails *details;
};

struct EelLabeledImageClass
{
	GtkContainerClass parent_class;
};

/* Public GtkLabeledImage methods */
GtkType         eel_labeled_image_get_type                         (void);
GtkWidget *     eel_labeled_image_new                              (const char              *text,
								    GdkPixbuf               *pixbuf);
GtkWidget *     eel_labeled_image_new_from_file_name               (const char              *text,
								    const char              *pixbuf_file_name);
void            eel_labeled_image_set_label_position               (EelLabeledImage         *labeled_image,
								    GtkPositionType          label_position);
GtkPositionType eel_labeled_image_get_label_position               (const EelLabeledImage   *labeled_image);
void            eel_labeled_image_set_show_label                   (EelLabeledImage         *labeled_image,
								    gboolean                 show_label);
gboolean        eel_labeled_image_get_show_label                   (const EelLabeledImage   *labeled_image);
void            eel_labeled_image_set_show_image                   (EelLabeledImage         *labeled_image,
								    gboolean                 show_image);
gboolean        eel_labeled_image_get_show_image                   (const EelLabeledImage   *labeled_image);
void            eel_labeled_image_set_spacing                      (EelLabeledImage         *labeled_image,
								    guint                    spacing);
guint           eel_labeled_image_get_spacing                      (const EelLabeledImage   *labeled_image);
int             eel_labeled_image_get_x_padding                    (const EelLabeledImage   *labeled_image);
void            eel_labeled_image_set_x_padding                    (EelLabeledImage         *labeled_image,
								    int                      x_padding);
int             eel_labeled_image_get_y_padding                    (const EelLabeledImage   *labeled_image);
void            eel_labeled_image_set_y_padding                    (EelLabeledImage         *labeled_image,
								    int                      y_padding);
float           eel_labeled_image_get_x_alignment                  (const EelLabeledImage   *labeled_image);
void            eel_labeled_image_set_x_alignment                  (EelLabeledImage         *labeled_image,
								    float                    x_alignment);
float           eel_labeled_image_get_y_alignment                  (const EelLabeledImage   *labeled_image);
void            eel_labeled_image_set_y_alignment                  (EelLabeledImage         *labeled_image,
								    float                    y_alignment);
void            eel_labeled_image_set_fill                         (EelLabeledImage         *labeled_image,
								    gboolean                 fill);
gboolean        eel_labeled_image_get_fill                         (const EelLabeledImage   *labeled_image);
void            eel_labeled_image_set_fixed_image_height           (EelLabeledImage         *labeled_image,
								    int                      fixed_image_height);
									 
/* Functions for creating stock GtkButtons with a labeled image child */
GtkWidget *     eel_labeled_image_button_new                       (const char              *text,
								    GdkPixbuf               *pixbuf);
GtkWidget *     eel_labeled_image_button_new_from_file_name        (const char              *text,
								    const char              *pixbuf_file_name);
GtkWidget *     eel_labeled_image_toggle_button_new                (const char              *text,
								    GdkPixbuf               *pixbuf);
GtkWidget *     eel_labeled_image_toggle_button_new_from_file_name (const char              *text,
								    const char              *pixbuf_file_name);
GtkWidget *     eel_labeled_image_check_button_new                 (const char              *text,
								    GdkPixbuf               *pixbuf);
GtkWidget *     eel_labeled_image_check_button_new_from_file_name  (const char              *text,
								    const char              *pixbuf_file_name);

/* These are proxies for methods in EelImage and EelLabel */
void            eel_labeled_image_set_pixbuf                       (EelLabeledImage         *labeled_image,
								    GdkPixbuf               *pixbuf);
void            eel_labeled_image_set_pixbuf_from_file_name        (EelLabeledImage         *labeled_image,
								    const char              *pixbuf_file_name);
GdkPixbuf*      eel_labeled_image_get_pixbuf                       (const EelLabeledImage   *labeled_image);
void            eel_labeled_image_set_text                         (EelLabeledImage         *labeled_image,
								    const char              *text);
char*           eel_labeled_image_get_text                         (const EelLabeledImage   *labeled_image);
void            eel_labeled_image_set_tile_pixbuf                  (EelLabeledImage         *image,
								    GdkPixbuf               *pixbuf);
void            eel_labeled_image_set_tile_pixbuf_from_file_name   (EelLabeledImage         *image,
								    const char              *tile_file_name);
void            eel_labeled_image_make_bold                        (EelLabeledImage         *labeled_image);
void            eel_labeled_image_make_larger                      (EelLabeledImage         *labeled_image,
								    guint                    num_sizes);
void            eel_labeled_image_make_smaller                     (EelLabeledImage         *labeled_image,
								    guint                    num_sizes);
void            eel_labeled_image_set_tile_width                   (EelLabeledImage         *labeled_image,
								    int                      tile_width);
void            eel_labeled_image_set_tile_height                  (EelLabeledImage         *labeled_image,
								    int                      tile_height);
void            eel_labeled_image_set_background_mode              (EelLabeledImage         *labeled_image,
								    EelSmoothBackgroundMode  background_mode);
void            eel_labeled_image_set_solid_background_color       (EelLabeledImage         *labeled_image,
								    guint32                  solid_background_color);
void            eel_labeled_image_set_smooth_drop_shadow_offset    (EelLabeledImage         *labeled_image,
								    guint                    drop_shadow_offset);
void            eel_labeled_image_set_smooth_drop_shadow_color     (EelLabeledImage         *labeled_image,
								    guint32                  drop_shadow_color);
void            eel_labeled_image_set_text_color                   (EelLabeledImage         *labeled_image,
								    guint32                  text_color);
void            eel_labeled_image_set_label_never_smooth           (EelLabeledImage         *labeled_image,
								    gboolean                 never_smooth);
ArtIRect        eel_labeled_image_get_image_bounds                 (const EelLabeledImage   *labeled_image);
ArtIRect        eel_labeled_image_get_label_bounds                 (const EelLabeledImage   *labeled_image);

END_GNOME_DECLS

#endif /* EEL_LABELED_IMAGE_H */


