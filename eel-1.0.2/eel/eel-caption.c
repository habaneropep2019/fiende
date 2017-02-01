/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-caption.c - A captioned widget.

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

#include "eel-caption.h"

#include "eel-gtk-macros.h"
#include "eel-glib-extensions.h"
#include "eel-art-gtk-extensions.h"

#include <gtk/gtklabel.h>
#include <gtk/gtkentry.h>

#define CAPTION_SPACING 10

struct EelCaptionDetail
{
	GtkWidget *title_label;
	GtkWidget *child;
	gboolean show_title;
};

/* EelCaptionClass methods */
static void eel_caption_initialize_class (EelCaptionClass *klass);
static void eel_caption_initialize       (EelCaption      *caption);

/* GtkObjectClass methods */
static void eel_caption_destroy          (GtkObject       *object);
static void caption_show_all     (GtkWidget       *widget);
static void update_title                 (EelCaption      *caption);

EEL_DEFINE_CLASS_BOILERPLATE (EelCaption, eel_caption, GTK_TYPE_HBOX)

/*
 * EelCaptionClass methods
 */
static void
eel_caption_initialize_class (EelCaptionClass *caption_class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	
	object_class = GTK_OBJECT_CLASS (caption_class);
	widget_class = GTK_WIDGET_CLASS (caption_class);

	/* GtkObjectClass */
	object_class->destroy = eel_caption_destroy;

	/* GtkWidgetClass */
	widget_class->show_all = caption_show_all;
}

static void
eel_caption_initialize (EelCaption *caption)
{
	caption->detail = g_new0 (EelCaptionDetail, 1);

	gtk_box_set_homogeneous (GTK_BOX (caption), FALSE);
	gtk_box_set_spacing (GTK_BOX (caption), CAPTION_SPACING);

	caption->detail->show_title = TRUE;
	caption->detail->title_label = gtk_label_new ("Title Label:");
	caption->detail->child = NULL;

	gtk_box_pack_start (GTK_BOX (caption),
			    caption->detail->title_label,
			    FALSE,	/* expand */
			    TRUE,	/* fill */
			    0);		/* padding */

	gtk_widget_show (caption->detail->title_label);
}

/*
 * GtkObjectClass methods
 */
static void
eel_caption_destroy (GtkObject *object)
{
	EelCaption * caption;
	
	g_return_if_fail (EEL_IS_CAPTION (object));
	
	caption = EEL_CAPTION (object);

	g_free (caption->detail);

	/* Chain */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/* GtkObjectClass methods */
static void
caption_show_all (GtkWidget *widget)
{
	g_return_if_fail (EEL_IS_CAPTION (widget));

	EEL_CALL_PARENT (GTK_WIDGET_CLASS, show_all, (widget));

	/* Now update the title visibility */
	update_title (EEL_CAPTION (widget));
}

static void
update_title (EelCaption	*caption)
{
	g_return_if_fail (EEL_IS_CAPTION (caption));

	if (caption->detail->show_title) {
		gtk_widget_show (caption->detail->title_label);
	}
	else {
		gtk_widget_hide (caption->detail->title_label);
	}
}

/*
 * EelCaption public methods
 */
GtkWidget *
eel_caption_new (void)
{
	return gtk_widget_new (eel_caption_get_type (), NULL);
}

/**
 * eel_caption_set_title_label:
 * @caption: A EelCaption
 * @title_label: The title label
 *
 */
void
eel_caption_set_title_label (EelCaption		*caption,
				  const char			*title_label)
{
	g_return_if_fail (EEL_IS_CAPTION (caption));
	g_return_if_fail (title_label != NULL);

	gtk_label_set_text (GTK_LABEL (caption->detail->title_label), title_label);
}

/**
 * eel_caption_set_show_title:
 * @caption: A EelCaption
 * @show_title: Whether to show the title or not
 *
 */
void
eel_caption_set_show_title (EelCaption *caption,
			    gboolean show_title)
{
	g_return_if_fail (EEL_IS_CAPTION (caption));

	if (caption->detail->show_title == show_title) {
		return;
	}

	caption->detail->show_title = show_title;

	update_title (caption);
}

/**
 * eel_caption_get_title_label:
 * @caption: A EelCaption
 *
 * Returns: A newly allocated copy of the title label.
 */
char *
eel_caption_get_title_label (const EelCaption *caption)
{
	gchar *str;

	g_return_val_if_fail (EEL_IS_CAPTION (caption), NULL);

	/* DANGER! DANGER!
	 * 
	 * gtk_label_get () does not strdup the result.
	 */
	gtk_label_get (GTK_LABEL (caption->detail->title_label), &str);

	return str ? g_strdup (str) : NULL;
}

/**
 * eel_caption_get_title_label_width:
 * @caption: A EelCaption
 *
 * Returns: A width of the title label.
 */
int
eel_caption_get_title_label_width (const EelCaption *caption)
{
	EelDimensions title_dimensions;
	
	g_return_val_if_fail (EEL_IS_CAPTION (caption), 0);
	
	title_dimensions = eel_gtk_widget_get_preferred_dimensions (caption->detail->title_label);
	
	return title_dimensions.width;
}

/**
 * eel_caption_set_child
 * @caption: A EelCaption
 * @child: A GtkWidget to become the caption's one and only child.
 * @expand: Same as GtkBox.
 * @fill: Same as GtkBox.
 *
 * Install a widget as the one and only child of the caption.
 */
void
eel_caption_set_child (EelCaption *caption,
			    GtkWidget *child,
			    gboolean expand,
			    gboolean fill)
{
	g_return_if_fail (EEL_IS_CAPTION (caption));
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (caption->detail->child == NULL);

	caption->detail->child = child;
	
	gtk_box_pack_start (GTK_BOX (caption),
			    caption->detail->child,
			    expand,	/* expand */
			    fill,	/* fill */
			    0);	        /* padding */
	
	gtk_widget_show (caption->detail->child);
}

/**
 * eel_caption_set_extra_spacing
 * @caption: A EelCaption
 * @spacing: Extra spacing in pixels between the title and the child,
 * beyond the nominal amount
 *
 * Set the amount of extra spacing between the title label and the 
 * caption's one and only child.
 */
void
eel_caption_set_extra_spacing (EelCaption *caption,
			      	    int extra_spacing)
{
	g_return_if_fail (EEL_IS_CAPTION (caption));
	g_return_if_fail (extra_spacing >= 0);

	gtk_box_set_spacing (GTK_BOX (caption), 
			     CAPTION_SPACING + extra_spacing);
}
