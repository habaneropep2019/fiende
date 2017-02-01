/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-radio-button-group.c - A radio button group container.

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
#include "eel-radio-button-group.h"
#include "eel-image.h"

#include <gtk/gtkradiobutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtksignal.h>

#include "eel-gtk-macros.h"
#include "eel-glib-extensions.h"

/* Signals */
typedef enum
{
	CHANGED,
	LAST_SIGNAL
} RadioGroupSignals;

struct EelRadioButtonGroupDetails
{
	GList *rows;
	GSList *group;
	guint num_items;
	gboolean horizontal;
};

typedef struct
{
	GtkWidget *button;
	GtkWidget *image;
	GtkWidget *description;
} TableRow;

/* EelRadioButtonGroupClass methods */
static void eel_radio_button_group_initialize_class (EelRadioButtonGroupClass *klass);
static void eel_radio_button_group_initialize       (EelRadioButtonGroup      *button_group);

/* GtkObjectClass methods */
static void eel_radio_button_group_destroy          (GtkObject                *object);

/* Radio button callbacks */
static void button_toggled                          (GtkWidget                *button,
						     gpointer                  user_data);

EEL_DEFINE_CLASS_BOILERPLATE (EelRadioButtonGroup,
			      eel_radio_button_group,
			      GTK_TYPE_TABLE)

static guint radio_group_signals[LAST_SIGNAL] = { 0 };

/*
 * EelRadioButtonGroupClass methods
 */
static void
eel_radio_button_group_initialize_class (EelRadioButtonGroupClass *radio_button_group_class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	
	object_class = GTK_OBJECT_CLASS (radio_button_group_class);
	widget_class = GTK_WIDGET_CLASS (radio_button_group_class);

 	parent_class = gtk_type_class (gtk_table_get_type ());

	/* GtkObjectClass */
	object_class->destroy = eel_radio_button_group_destroy;

	/* Signals */
	radio_group_signals[CHANGED] = gtk_signal_new ("changed",
						       GTK_RUN_LAST,
						       object_class->type,
						       0,
						       gtk_marshal_NONE__NONE,
						       GTK_TYPE_NONE, 
						       0);

	gtk_object_class_add_signals (object_class, radio_group_signals, LAST_SIGNAL);
}

static void
eel_radio_button_group_initialize (EelRadioButtonGroup *button_group)
{
	button_group->details = g_new0 (EelRadioButtonGroupDetails, 1);
}

/*
 * GtkObjectClass methods
 */
static void
eel_radio_button_group_destroy (GtkObject *object)
{
	EelRadioButtonGroup * button_group;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (EEL_IS_RADIO_BUTTON_GROUP (object));
	
	button_group = EEL_RADIO_BUTTON_GROUP (object);

	eel_radio_button_group_clear (button_group);
	g_free (button_group->details);
	
	/* Chain */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/*
 * Radio button callbacks
 */
static void
button_toggled (GtkWidget *button, gpointer user_data)
{
	EelRadioButtonGroup *button_group = (EelRadioButtonGroup *) user_data;
	
	g_assert (button_group != NULL);
	g_assert (button_group->details != NULL);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button))) {
		gtk_signal_emit (GTK_OBJECT (button_group), radio_group_signals[CHANGED]);
	}
}

/*
 * EelRadioButtonGroup public methods
 */
GtkWidget*
eel_radio_button_group_new (gboolean is_horizontal)
{
	EelRadioButtonGroup *button_group;

	button_group = EEL_RADIO_BUTTON_GROUP
		(gtk_widget_new (eel_radio_button_group_get_type (), NULL));
	button_group->details->horizontal = is_horizontal;
	
	return GTK_WIDGET (button_group);
}
 
/**
 * eel_radio_button_group_insert:
 * @button_group: The button group
 * @label: Label to use for the new button
 *
 * Create and insert a new radio button to the collection.
 *
 * Returns: The index of the new button.
 */
guint
eel_radio_button_group_insert (EelRadioButtonGroup	*button_group,
				    const gchar			*label)
{
	GtkTable	*table;
	TableRow	*row;

	g_return_val_if_fail (button_group != NULL, 0);
	g_return_val_if_fail (EEL_IS_RADIO_BUTTON_GROUP (button_group), 0);
	g_return_val_if_fail (label != NULL, 0);

	table = GTK_TABLE (button_group);

	row = g_new0 (TableRow, 1);

	row->button = gtk_radio_button_new_with_label (button_group->details->group, label);

	/*
	 * For some crazy reason I dont grok, the group has to be fetched each
	 * time from the previous button
	 */
	button_group->details->group = gtk_radio_button_group (GTK_RADIO_BUTTON (row->button));

	gtk_signal_connect (GTK_OBJECT (row->button),
			    "toggled",
			    GTK_SIGNAL_FUNC (button_toggled),
			    (gpointer) button_group);

	button_group->details->num_items++;

	if (button_group->details->horizontal) {
		/* Resize the table to fit all items in one row. */
		gtk_table_resize (table, 1, button_group->details->num_items);
		/* Place the radio button in the last (so far) column of the only row */
		gtk_table_attach (table, 
				  row->button,				/* child */
				  button_group->details->num_items - 1, /* left_attach */
				  button_group->details->num_items,	/* right_attach */
				  0,					/* top_attach */
				  1,					/* bottom_attach */
				  (GTK_FILL|GTK_EXPAND),		/* xoptions */
				  (GTK_FILL|GTK_EXPAND),		/* yoptions */
				  0,					/* xpadding */
				  0);					/* ypadding */
	} else {
		/* Resize the table to put each item on separate row. */
		gtk_table_resize (table, button_group->details->num_items, 3);
		/* Place the radio button in column 2 of the last (so far) row */
		gtk_table_attach (table, 
				  row->button,				/* child */
				  1,					/* left_attach */
				  2,					/* right_attach */
				  button_group->details->num_items - 1,	/* top_attach */
				  button_group->details->num_items,	/* bottom_attach */
				  (GTK_FILL|GTK_EXPAND),		/* xoptions */
				  (GTK_FILL|GTK_EXPAND),		/* yoptions */
				  0,					/* xpadding */
				  0);					/* ypadding */
	}


	gtk_widget_show (row->button);
	
	button_group->details->rows = g_list_append (button_group->details->rows, row);
	
	return g_list_length (button_group->details->rows) - 1;
}

/**
 * eel_radio_button_group_get_active_index:
 * @button_group: The button group
 *
 * Returns: The index of the active button.  There is always one active by law.
 */
int
eel_radio_button_group_get_active_index (EelRadioButtonGroup *button_group)
{
	GList	*button_iterator;
	int	i = 0;

 	g_return_val_if_fail (button_group != NULL, 0);
	g_return_val_if_fail (EEL_IS_RADIO_BUTTON_GROUP (button_group), 0);

	if (button_group->details->rows == NULL) {
		return -1;
	}
	
	g_assert (button_group != NULL);

	button_iterator = button_group->details->rows;

	while (button_iterator) {
		TableRow *row;

		row = button_iterator->data;
		g_assert (row != NULL);
		g_assert (row->button != NULL);
		g_assert (GTK_TOGGLE_BUTTON (row->button));

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (row->button))) {
			return i;
		}
		
		button_iterator = button_iterator->next;

		i++;
	}

	g_assert_not_reached ();

	return 0;
}

void
eel_radio_button_group_set_active_index (EelRadioButtonGroup *button_group,
					      guint active_index)
{
	TableRow *row;

 	g_return_if_fail (button_group != NULL);
	g_return_if_fail (EEL_IS_RADIO_BUTTON_GROUP (button_group));

	if (button_group->details->rows == NULL) {
		return;
	}

	row = g_list_nth_data (button_group->details->rows, active_index);
	g_assert (row != NULL);
	g_assert (row->button != NULL);
	g_assert (GTK_TOGGLE_BUTTON (row->button));

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (row->button), TRUE);
}

/* Set an item's pixbuf. */
void
eel_radio_button_group_set_entry_pixbuf (EelRadioButtonGroup *button_group,
					      guint                     entry_index,
					      GdkPixbuf                *pixbuf)
{
	GtkTable	*table;
	TableRow	*row;

 	g_return_if_fail (button_group != NULL);
	g_return_if_fail (EEL_IS_RADIO_BUTTON_GROUP (button_group));
	g_return_if_fail (entry_index < g_list_length (button_group->details->rows));
	g_return_if_fail (button_group->details->horizontal == FALSE);

	table = GTK_TABLE (button_group);

	row = g_list_nth_data (button_group->details->rows, entry_index);
	g_assert (row != NULL);

	if (row->image == NULL) {
		row->image = eel_image_new (NULL);
		
		gtk_table_attach (table,
				  row->image,			/* child */
				  0,				/* left_attach */
				  1,				/* right_attach */
				  entry_index,			/* top_attach */
				  entry_index + 1,		/* bottom_attach */
				  GTK_FILL,			/* xoptions */
				  (GTK_FILL|GTK_EXPAND),	/* yoptions */
				  0,				/* xpadding */
				  0);				/* ypadding */
		
		gtk_widget_show (row->image);
	}

	g_assert (row->image != NULL);
	
	eel_image_set_pixbuf (EEL_IMAGE (row->image), pixbuf);
}

/* Set an item's description. */
void
eel_radio_button_group_set_entry_description_text (EelRadioButtonGroup *button_group,
							guint                     entry_index,
							const char               *description_text)
{
	GtkTable	*table;
	TableRow	*row;

 	g_return_if_fail (button_group != NULL);
	g_return_if_fail (EEL_IS_RADIO_BUTTON_GROUP (button_group));
	g_return_if_fail (entry_index < g_list_length (button_group->details->rows));
	g_return_if_fail (button_group->details->horizontal == FALSE);

	table = GTK_TABLE (button_group);

	row = g_list_nth_data (button_group->details->rows, entry_index);
	g_assert (row != NULL);
	
	if (row->description == NULL) {
		row->description = gtk_label_new (description_text);

		gtk_misc_set_alignment (GTK_MISC (row->description), 0, 0.5);
		
		gtk_table_attach (table,
				  row->description,		/* child */
				  2,				/* left_attach */
				  3,				/* right_attach */
				  entry_index,			/* top_attach */
				  entry_index + 1,		/* bottom_attach */
				  (GTK_FILL|GTK_EXPAND),	/* xoptions */
				  (GTK_FILL|GTK_EXPAND),	/* yoptions */
				  0,				/* xpadding */
				  0);				/* ypadding */
		
		gtk_widget_show (row->description);
	}
	else {
		gtk_label_set_text (GTK_LABEL (row->description), description_text);
	}
}

void
eel_radio_button_group_clear (EelRadioButtonGroup *button_group)
{
	GList *node;
	TableRow *row;

	g_return_if_fail (EEL_IS_RADIO_BUTTON_GROUP (button_group));
	
	g_assert (button_group != NULL);

	node = button_group->details->rows;

	for (node = button_group->details->rows; node != NULL; node = node->next) {
		g_assert (node->data != NULL);
		row = node->data;

		if (row->button != NULL) {
			gtk_widget_destroy (row->button);
		}
		if (row->image != NULL) {
			gtk_widget_destroy (row->image);
		}
		if (row->description != NULL) {
			gtk_widget_destroy (row->description);
		}

		g_free (row);
	}
	g_list_free (button_group->details->rows);
	button_group->details->rows = NULL;
	button_group->details->group = NULL;
	button_group->details->num_items = 0;
}


