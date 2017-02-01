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

#include <config.h>

#include "eel-text-caption.h"
#include "eel-gtk-macros.h"
#include "eel-glib-extensions.h"

#include <gtk/gtklabel.h>
#include <gtk/gtkentry.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>

#include <libgnomevfs/gnome-vfs-utils.h>

#include <string.h>
 
static const gint TEXT_CAPTION_INVALID = -1;
static const gint TEXT_CAPTION_SPACING = 10;

/* Signals */
typedef enum
{
	CHANGED,
	LAST_SIGNAL
} EelTextCaptionSignals;

struct EelTextCaptionDetail
{
	GtkWidget *text;
	gboolean expand_tilde;
};

/* EelTextCaptionClass methods */
static void eel_text_caption_initialize_class (EelTextCaptionClass *klass);
static void eel_text_caption_initialize       (EelTextCaption      *text_caption);

/* GtkObjectClass methods */
static void eel_text_caption_destroy          (GtkObject           *object);

/* Editable (entry) callbacks */
static void entry_changed_callback            (GtkWidget           *entry,
					       gpointer             user_data);
static void entry_key_press_callback          (GtkWidget           *widget,
					       GdkEventKey         *event,
					       gpointer             user_data);

EEL_DEFINE_CLASS_BOILERPLATE (EelTextCaption, eel_text_caption, EEL_TYPE_CAPTION)

static guint text_caption_signals[LAST_SIGNAL] = { 0 };

/*
 * EelTextCaptionClass methods
 */
static void
eel_text_caption_initialize_class (EelTextCaptionClass *text_caption_class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	
	object_class = GTK_OBJECT_CLASS (text_caption_class);
	widget_class = GTK_WIDGET_CLASS (text_caption_class);

	/* GtkObjectClass */
	object_class->destroy = eel_text_caption_destroy;
	
	/* Signals */
	text_caption_signals[CHANGED] =
		gtk_signal_new ("changed",
				GTK_RUN_LAST,
				object_class->type,
				0,
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 
				0);

	gtk_object_class_add_signals (object_class, text_caption_signals, LAST_SIGNAL);
}

static void
eel_text_caption_initialize (EelTextCaption *text_caption)
{
	text_caption->detail = g_new (EelTextCaptionDetail, 1);

	gtk_box_set_homogeneous (GTK_BOX (text_caption), FALSE);
	gtk_box_set_spacing (GTK_BOX (text_caption), TEXT_CAPTION_SPACING);

	text_caption->detail->text = gtk_entry_new ();
	
	gtk_entry_set_editable (GTK_ENTRY (text_caption->detail->text), TRUE);
	
	eel_caption_set_child (EEL_CAPTION (text_caption),
				    text_caption->detail->text,
				    TRUE,
				    TRUE);

	gtk_signal_connect (GTK_OBJECT (text_caption->detail->text),
			    "changed",
			    GTK_SIGNAL_FUNC (entry_changed_callback),
			    (gpointer) text_caption);
	gtk_signal_connect_after (GTK_OBJECT (text_caption->detail->text),
				  "key_press_event",
				  GTK_SIGNAL_FUNC (entry_key_press_callback),
				  (gpointer) text_caption);

	gtk_widget_show (text_caption->detail->text);
}

/*
 * GtkObjectClass methods
 */
static void
eel_text_caption_destroy (GtkObject* object)
{
	EelTextCaption * text_caption;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (EEL_IS_TEXT_CAPTION (object));
	
	text_caption = EEL_TEXT_CAPTION (object);

	g_free (text_caption->detail);

	/* Chain */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/*
 * Editable (entry) callbacks
 */
static void
entry_changed_callback (GtkWidget *entry, gpointer user_data)
{
	EelTextCaption *text_caption;

	g_assert (user_data != NULL);
	g_assert (EEL_IS_TEXT_CAPTION (user_data));

	text_caption = EEL_TEXT_CAPTION (user_data);

	gtk_signal_emit (GTK_OBJECT (text_caption), text_caption_signals[CHANGED]);
}

static void
entry_key_press_callback (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	EelTextCaption *text_caption;
	char *expanded_text;
	
	text_caption = EEL_TEXT_CAPTION (user_data);
	
	if (event->keyval == GDK_asciitilde) {
		if (text_caption->detail->expand_tilde) {
			expanded_text = gnome_vfs_expand_initial_tilde (gtk_entry_get_text (GTK_ENTRY (widget)));
			
			gtk_entry_set_text (GTK_ENTRY (widget), expanded_text);
			g_free (expanded_text);
		}
	}
}

/*
 * EelTextCaption public methods
 */
GtkWidget *
eel_text_caption_new (void)
{
	return gtk_widget_new (eel_text_caption_get_type (), NULL);
}

/**
 * eel_text_caption_get_text
 * @text_caption: A EelTextCaption
 *
 * Returns: A copy of the currently selected text.  Need to g_free() it.
 */
char *
eel_text_caption_get_text (const EelTextCaption *text_caption)
{
	const char *entry_text;

 	g_return_val_if_fail (text_caption != NULL, NULL);
	g_return_val_if_fail (EEL_IS_TEXT_CAPTION (text_caption), NULL);

	/* WATCHOUT: 
	 * As of gtk1.2, gtk_entry_get_text() returns a non const reference to
	 * the internal string.
	 */
	entry_text = (const char *) gtk_entry_get_text (GTK_ENTRY (text_caption->detail->text));

	return g_strdup (entry_text);
}

void
eel_text_caption_set_text (EelTextCaption	*text_caption,
				 const char		*text)
{
 	g_return_if_fail (text_caption != NULL);
	g_return_if_fail (EEL_IS_TEXT_CAPTION (text_caption));

	gtk_entry_set_text (GTK_ENTRY (text_caption->detail->text), text);
}

void
eel_text_caption_set_editable (EelTextCaption *text_caption,
				    gboolean editable)
{
	g_return_if_fail (EEL_IS_TEXT_CAPTION (text_caption));

	gtk_entry_set_editable (GTK_ENTRY (text_caption->detail->text), editable);
}

void
eel_text_caption_set_expand_tilde (EelTextCaption *text_caption,
					gboolean expand_tilde)
{
	g_return_if_fail (EEL_IS_TEXT_CAPTION (text_caption));

	text_caption->detail->expand_tilde = expand_tilde;
}
