/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-text-caption.c - A text caption widget.

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

#ifndef EEL_TEXT_CAPTION_H
#define EEL_TEXT_CAPTION_H

#include <eel/eel-caption.h>

/*
 * EelTextCaption is made up of 2 widgets. 
 *
 * [title label] [string combo box]
 *
 * The user can select a string from the list.
 */
BEGIN_GNOME_DECLS

#define EEL_TYPE_TEXT_CAPTION            (eel_text_caption_get_type ())
#define EEL_TEXT_CAPTION(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_TEXT_CAPTION, EelTextCaption))
#define EEL_TEXT_CAPTION_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_TEXT_CAPTION, EelTextCaptionClass))
#define EEL_IS_TEXT_CAPTION(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_TEXT_CAPTION))

typedef struct EelTextCaption		EelTextCaption;
typedef struct EelTextCaptionClass      EelTextCaptionClass;
typedef struct EelTextCaptionDetail     EelTextCaptionDetail;

struct EelTextCaption
{
	/* Super Class */
	EelCaption caption;
	
	/* Private stuff */
	EelTextCaptionDetail *detail;
};

struct EelTextCaptionClass
{
	EelCaptionClass parent_class;
};

GtkType    eel_text_caption_get_type         (void);
GtkWidget* eel_text_caption_new              (void);

/* Entry text accesor. */
char *     eel_text_caption_get_text         (const EelTextCaption *text_caption);

/* Entry text mutator. */
void       eel_text_caption_set_text         (EelTextCaption       *text_caption,
					      const char           *text);
void       eel_text_caption_set_editable     (EelTextCaption       *text_caption,
					      gboolean              editable);
void       eel_text_caption_set_expand_tilde (EelTextCaption       *text_caption,
					      gboolean              expand_tilde);

END_GNOME_DECLS

#endif /* EEL_TEXT_CAPTION_H */


