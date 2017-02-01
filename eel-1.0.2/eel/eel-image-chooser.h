/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-image-chooser.h - A widget to choose an image from a list.

   Copyright (C) 2001 Eazel, Inc.

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

#ifndef EEL_IMAGE_CHOOSER_H
#define EEL_IMAGE_CHOOSER_H

#include <gtk/gtkbin.h>
#include <libgnome/gnome-defs.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <eel/eel-art-extensions.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_IMAGE_CHOOSER            (eel_image_chooser_get_type ())
#define EEL_IMAGE_CHOOSER(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_IMAGE_CHOOSER, EelImageChooser))
#define EEL_IMAGE_CHOOSER_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_IMAGE_CHOOSER, EelImageChooserClass))
#define EEL_IS_IMAGE_CHOOSER(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_IMAGE_CHOOSER))

typedef struct EelImageChooser		 EelImageChooser;
typedef struct EelImageChooserClass	 EelImageChooserClass;
typedef struct EelImageChooserDetails	 EelImageChooserDetails;

struct EelImageChooser
{
	/* Super Class */
	GtkBin bin;
	
	/* Private stuff */
	EelImageChooserDetails *details;
};

struct EelImageChooserClass
{
	GtkBinClass parent_class;
};

GtkType        eel_image_chooser_get_type                      (void);
GtkWidget*     eel_image_chooser_new                           (void);
void           eel_image_chooser_insert_row                    (EelImageChooser        *image_chooser,
								GdkPixbuf              *pixbuf,
								const char             *title,
								const char             *description,
								gpointer                row_data,
								GFreeFunc               row_data_free_func);
void           eel_image_chooser_clear                         (EelImageChooser        *image_chooser);
int            eel_image_chooser_get_selected_row              (const EelImageChooser  *image_chooser);
void           eel_image_chooser_set_selected_row              (EelImageChooser        *image_chooser,
								int                     icon_position);
gpointer       eel_image_chooser_get_row_data                  (const EelImageChooser  *image_chooser,
								guint                   row_index);
guint          eel_image_chooser_get_num_rows                  (const EelImageChooser  *image_chooser);
void           eel_image_chooser_synthetic_motion              (EelImageChooser        *image_chooser,
								int                     x,
								int                     y);
GtkWidget     *eel_scrolled_image_chooser_new                  (GtkWidget             **image_chooser_out);
void           eel_scrolled_image_chooser_set_num_visible_rows (EelImageChooser        *image_chooser,
								GtkWidget              *scrolled_window,
								guint                   num_visible_rows);
void           eel_scrolled_image_chooser_show_selected_row    (EelImageChooser        *image_chooser,
								GtkWidget              *scrolled_window);

END_GNOME_DECLS

#endif /* EEL_IMAGE_CHOOSER_H */


