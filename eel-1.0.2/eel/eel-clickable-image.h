/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-clickable-image.h - A clickable image widget.

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

/* EelClickableImage is a EelLabeledImage sublclass with signals
 * usful for detecting user click and prelight events.
 *
 * The following signals are provided by EelClickableImage:
 *
 * "clicked" - Widget was clicked
 * "enter"   - Pointer entered widget.
 * "leave"   - Pointer left widget.
 *
 * EelClickableImage is a NO_WINDOW widget.  It does its event detection
 * by monitoring events on the first windowed ancestor in its widget hierarchy.
 *
 * Being a NO_WINDOW widget, it will work nicely with tile_pixbufs.
 */

#ifndef EEL_CLICKABLE_IMAGE_H
#define EEL_CLICKABLE_IMAGE_H

#include <eel/eel-labeled-image.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_CLICKABLE_IMAGE            (eel_clickable_image_get_type ())
#define EEL_CLICKABLE_IMAGE(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_CLICKABLE_IMAGE, EelClickableImage))
#define EEL_CLICKABLE_IMAGE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_CLICKABLE_IMAGE, EelClickableImageClass))
#define EEL_IS_CLICKABLE_IMAGE(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_CLICKABLE_IMAGE))
#define EEL_IS_CLICKABLE_IMAGE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_CLICKABLE_IMAGE))

typedef struct EelClickableImage	   EelClickableImage;
typedef struct EelClickableImageClass	   EelClickableImageClass;
typedef struct EelClickableImageDetails    EelClickableImageDetails;

struct EelClickableImage
{
	/* Superclass */
	EelLabeledImage labeled_image;

	/* Private things */
	EelClickableImageDetails *details;
};

struct EelClickableImageClass
{
	EelLabeledImageClass parent_class;
	
	void (*clicked) (EelClickableImage *image);
	void (*enter) (EelClickableImage *image);
	void (*leave) (EelClickableImage *image);
};

GtkType    eel_clickable_image_get_type           (void);
GtkWidget *eel_clickable_image_new                (const char        *text,
						   GdkPixbuf         *pixbuf);
GtkWidget *eel_clickable_image_new_from_file_name (const char        *text,
						   const char        *pixbuf_file_name);
GtkWidget *eel_clickable_image_new_solid          (const char        *text,
						   GdkPixbuf         *pixbuf,
						   guint              drop_shadow_offset,
						   guint32            drop_shadow_color,
						   guint32            text_color,
						   float              x_alignment,
						   float              y_alignment,
						   int                x_padding,
						   int                y_padding,
						   guint32            background_color,
						   GdkPixbuf         *tile_pixbuf);
void       eel_clickable_image_set_prelight       (EelClickableImage *clickable_image,
						   gboolean           prelight);

END_GNOME_DECLS

#endif /* EEL_CLICKABLE_IMAGE_H */


