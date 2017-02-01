/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-caption.h - A captioned widget.

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

#ifndef EEL_CAPTION_H
#define EEL_CAPTION_H

#include <gtk/gtkhbox.h>
#include <libgnome/gnome-defs.h>

/*
 * EelCaption is made up of 2 widgets. 
 *
 * [title label] [something]
 *
 */
BEGIN_GNOME_DECLS

#define EEL_TYPE_CAPTION            (eel_caption_get_type ())
#define EEL_CAPTION(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_CAPTION, EelCaption))
#define EEL_CAPTION_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_CAPTION, EelCaptionClass))
#define EEL_IS_CAPTION(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_CAPTION))

typedef struct EelCaption	   EelCaption;
typedef struct EelCaptionClass	   EelCaptionClass;
typedef struct EelCaptionDetail    EelCaptionDetail;

struct EelCaption
{
	/* Super Class */
	GtkHBox hbox;
	
	/* Private stuff */
	EelCaptionDetail *detail;
};

struct EelCaptionClass
{
	GtkHBoxClass		parent_class;
};

GtkType    eel_caption_get_type              (void);
GtkWidget* eel_caption_new                   (void);


/* Title label mutator. */
void       eel_caption_set_title_label       (EelCaption       *caption,
					      const char       *title_label);
void       eel_caption_set_show_title        (EelCaption       *caption,
					      gboolean          show_title);


/* Title label accessor. */
char *     eel_caption_get_title_label       (const EelCaption *caption);


/* Set the child. */
void       eel_caption_set_child             (EelCaption       *caption,
					      GtkWidget        *child,
					      gboolean          expand,
					      gboolean          fill);
void       eel_caption_set_extra_spacing     (EelCaption       *caption,
					      int               extra_spacing);
int        eel_caption_get_title_label_width (const EelCaption *caption);

END_GNOME_DECLS

#endif /* EEL_CAPTION_H */


