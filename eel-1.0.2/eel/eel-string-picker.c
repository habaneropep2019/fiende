/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-string-picker.c - A widget to pick a string from a list.

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
#include "eel-string-picker.h"

#include "eel-glib-extensions.h"
#include "eel-gtk-macros.h"
#include "eel-string.h"
#include <gtk/gtklabel.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtksignal.h>

#define STRING_PICKER_SPACING 10
#define STRING_PICKER_SEPARATOR "----------"

/* Signals */
typedef enum
{
	CHANGED,
	LAST_SIGNAL
} EelStringPickerSignals;

struct EelStringPickerDetail
{
	GtkWidget *option_menu;
	GtkWidget *menu;
	EelStringList *string_list;
	EelStringList *insensitive_list;
};

/* EelStringPickerClass methods */
static void eel_string_picker_initialize_class (EelStringPickerClass *string_picker_class);
static void eel_string_picker_initialize       (EelStringPicker      *string_picker);


/* GtkObjectClass methods */
static void eel_string_picker_destroy          (GtkObject            *object);


/* Option menu item callbacks */
static void option_menu_activate_callback      (GtkWidget            *menu_item,
						gpointer              callback_data);

EEL_DEFINE_CLASS_BOILERPLATE (EelStringPicker, eel_string_picker, EEL_TYPE_CAPTION)

static guint string_picker_signals[LAST_SIGNAL] = { 0 };

/*
 * EelStringPickerClass methods
 */
static void
eel_string_picker_initialize_class (EelStringPickerClass *string_picker_class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	
	object_class = GTK_OBJECT_CLASS (string_picker_class);
	widget_class = GTK_WIDGET_CLASS (string_picker_class);

	/* GtkObjectClass */
	object_class->destroy = eel_string_picker_destroy;
	
	/* Signals */
	string_picker_signals[CHANGED] = gtk_signal_new ("changed",
							 GTK_RUN_LAST,
							 object_class->type,
							 0,
							 gtk_marshal_NONE__NONE,
							 GTK_TYPE_NONE, 
							 0);

	gtk_object_class_add_signals (object_class, string_picker_signals, LAST_SIGNAL);
}

static void
eel_string_picker_initialize (EelStringPicker *string_picker)
{
	string_picker->detail = g_new0 (EelStringPickerDetail, 1);

	gtk_box_set_homogeneous (GTK_BOX (string_picker), FALSE);
	gtk_box_set_spacing (GTK_BOX (string_picker), STRING_PICKER_SPACING);
	
	string_picker->detail->string_list = eel_string_list_new (TRUE);
	string_picker->detail->insensitive_list = eel_string_list_new (TRUE);
	string_picker->detail->option_menu = gtk_option_menu_new ();

	eel_caption_set_child (EEL_CAPTION (string_picker),
			       string_picker->detail->option_menu,
			       FALSE,
			       FALSE);

	gtk_widget_show (string_picker->detail->option_menu);
}

/*
 * GtkObjectClass methods
 */
static void
eel_string_picker_destroy (GtkObject *object)
{
	EelStringPicker * string_picker;
	
	g_return_if_fail (EEL_IS_STRING_PICKER (object));
	
	string_picker = EEL_STRING_PICKER (object);

	eel_string_list_free (string_picker->detail->string_list);
	eel_string_list_free (string_picker->detail->insensitive_list);
	g_free (string_picker->detail);

	/* Chain */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/* Option menu item callbacks */
static void
option_menu_activate_callback (GtkWidget *menu_item,
			       gpointer callback_data)
{
	EelStringPicker *string_picker;

	g_return_if_fail (GTK_IS_MENU_ITEM (menu_item));
	g_return_if_fail (EEL_IS_STRING_PICKER (callback_data));

	string_picker = EEL_STRING_PICKER (callback_data);

	gtk_signal_emit (GTK_OBJECT (string_picker), string_picker_signals[CHANGED]);
}

static void
menu_item_set_sensitivity_callback (const char *string,
				    gpointer callback_data)
{
	GtkWidget *menu_item;
	GtkWidget *child;

	g_return_if_fail (string != NULL);
	g_return_if_fail (GTK_IS_MENU_ITEM (callback_data));

	menu_item = GTK_WIDGET (callback_data);
	child = GTK_BIN (menu_item)->child;
	
	g_return_if_fail (GTK_IS_LABEL (child));
	
	if (eel_str_is_equal (string, GTK_LABEL (child)->label)) {
		gtk_widget_set_sensitive (GTK_WIDGET (callback_data), FALSE);
	}
}

static void
menu_item_update_sensitivity (GtkMenuItem *menu_item,
			      const EelStringList *insensitive_list)
{
	GtkWidget *child;

	g_return_if_fail (GTK_IS_MENU_ITEM (menu_item));
	g_return_if_fail (insensitive_list != NULL);

	child = GTK_BIN (menu_item)->child;
	
	if (child != NULL) {
		gtk_widget_set_sensitive (GTK_WIDGET (menu_item), TRUE);
		
		eel_string_list_for_each (insensitive_list,
					  menu_item_set_sensitivity_callback,
					  menu_item);
	}
}

static void
string_picker_update_menu_sensitivities (EelStringPicker *string_picker)
{
	GList *node;
	GtkMenuShell *menu_shell;

	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));

	if (string_picker->detail->menu == NULL) {
		return;
	}
	
	menu_shell = GTK_MENU_SHELL (string_picker->detail->menu);
	for (node = menu_shell->children; node != NULL; node = node->next) {
		g_assert (GTK_IS_MENU_ITEM (node->data));
		menu_item_update_sensitivity (GTK_MENU_ITEM (node->data),
					      string_picker->detail->insensitive_list);
	}
}

/*
 * EelStringPicker public methods
 */
GtkWidget *
eel_string_picker_new (void)
{
	return gtk_widget_new (eel_string_picker_get_type (), NULL);
}

/**
 * eel_string_picker_set_string_list:
 * @string_picker: A EelStringPicker
 * @string_list: A list of strings
 *
 * Returns: nope
 */
void
eel_string_picker_set_string_list (EelStringPicker *string_picker,
				   const EelStringList *string_list)
{
	guint i;
	GtkWidget *menu_item;
	char *item_label;

	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));

	/* Make sure the string list is different */
	if (eel_string_list_equals (string_list, string_picker->detail->string_list)) {
		return;
	}

	eel_string_list_assign_from_string_list (string_picker->detail->string_list, string_list);
	
	/* Kill the old menu if alive */
	if (string_picker->detail->menu != NULL) {
		gtk_option_menu_remove_menu (GTK_OPTION_MENU (string_picker->detail->option_menu));

		/* The widget gets unrefed in the above call */
		string_picker->detail->menu = NULL;
	}

	/* Make a new menu */
	string_picker->detail->menu = gtk_menu_new ();
	
	if (eel_string_list_get_length (string_picker->detail->string_list) > 0) {
		for (i = 0; i < eel_string_list_get_length (string_picker->detail->string_list); i++) {
			item_label = eel_string_list_nth (string_picker->detail->string_list, i);
			g_assert (item_label != NULL);
			
			if (eel_str_is_equal (item_label, STRING_PICKER_SEPARATOR)) {
				menu_item = gtk_menu_item_new ();
				gtk_widget_set_sensitive (menu_item, FALSE);
			} else {
				menu_item = gtk_menu_item_new_with_label (item_label);

				gtk_signal_connect (GTK_OBJECT (menu_item),
						    "activate",
						    GTK_SIGNAL_FUNC (option_menu_activate_callback),
						    string_picker);
			}
			
			/* Save the index so we can later use it to retrieve the nth label from the list */
			gtk_object_set_data (GTK_OBJECT (menu_item), "index", GINT_TO_POINTER (i));
			
			gtk_widget_show (menu_item);
			
			gtk_menu_append (GTK_MENU (string_picker->detail->menu), menu_item);
		}
	}

	/* Attatch the menu to the option button */
	gtk_option_menu_set_menu (GTK_OPTION_MENU (string_picker->detail->option_menu), string_picker->detail->menu);

	string_picker_update_menu_sensitivities (string_picker);
}

/**
 * eel_string_picker_get_string_list:
 * @string_picker: A EelStringPicker
 *
 * Returns: A copy of the list of strings for the string picker.  Need to free it.
 */
EelStringList*
eel_string_picker_get_string_list (const EelStringPicker *string_picker)
{

	g_return_val_if_fail (EEL_IS_STRING_PICKER (string_picker), NULL);

	return eel_string_list_copy (string_picker->detail->string_list);
}

/* FIXME bugzilla.eazel.com 1556: 
 * Rename confusing string picker get/set functions
 */

/**
 * eel_string_picker_get_selected_string
 * @string_picker: A EelStringPicker
 *
 * Returns: A copy of the currently selected text.  Need to g_free() it.
 */
char *
eel_string_picker_get_selected_string (EelStringPicker *string_picker)
{
	int item_index;
	GtkWidget *option_menu;
	GtkWidget *menu_item;

	g_return_val_if_fail (EEL_IS_STRING_PICKER (string_picker), NULL);

	option_menu = string_picker->detail->option_menu;
	menu_item = GTK_OPTION_MENU (option_menu)->menu_item;
	item_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (menu_item), "index"));

	return (item_index != -1) ? eel_string_list_nth (string_picker->detail->string_list, item_index) : NULL;
}

/**
 * eel_string_picker_set_selected_string
 * @string_picker: A EelStringPicker
 *
 * Set the active item corresponding to the given text.
 */
void
eel_string_picker_set_selected_string (EelStringPicker *string_picker,
				       const char *text)
{
	int item_index;

	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));
	g_return_if_fail (eel_string_list_contains (string_picker->detail->string_list, text));

	item_index = eel_string_list_get_index_for_string (string_picker->detail->string_list, text);
	g_assert (item_index != EEL_STRING_LIST_NOT_FOUND);

	gtk_option_menu_set_history (GTK_OPTION_MENU (string_picker->detail->option_menu), item_index);
}

/**
 * eel_string_picker_set_selected_string_index
 * @string_picker: A EelStringPicker
 * @index: Index of selected string.
 *
 * Set the selected entry corresponding to the given index.
 */
void
eel_string_picker_set_selected_string_index (EelStringPicker *string_picker,
					     guint index)
{
	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));
	g_return_if_fail (index < eel_string_list_get_length (string_picker->detail->string_list));
	
	gtk_option_menu_set_history (GTK_OPTION_MENU (string_picker->detail->option_menu), index);
}

/**
 * eel_string_picker_insert_string
 * @string_picker: A EelStringPicker
 * @string: The string to insert.
 *
 * Insert a new string into the string picker.
 */
void
eel_string_picker_insert_string (EelStringPicker *string_picker,
				 const char *string)
{
	EelStringList *new_string_list;

	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));

	new_string_list = eel_string_list_copy (string_picker->detail->string_list);
	eel_string_list_insert (new_string_list, string);
	eel_string_picker_set_string_list (string_picker, new_string_list);
	eel_string_list_free (new_string_list);

	string_picker_update_menu_sensitivities (string_picker);
}

/**
 * eel_string_picker_insert_separator
 * @string_picker: A EelStringPicker
 *
 * Insert a separator into the string picker.
 */
void
eel_string_picker_insert_separator (EelStringPicker *string_picker)
{
	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));

	eel_string_picker_insert_string (string_picker, STRING_PICKER_SEPARATOR);
}

/**
 * eel_string_picker_insert_string
 * @string_picker: A EelStringPicker
 * @string: The string to insert.
 *
 * Insert a new string into the string picker.
 */
gboolean
eel_string_picker_contains (const EelStringPicker *string_picker,
			    const char *string)
{
	g_return_val_if_fail (EEL_IS_STRING_PICKER (string_picker), FALSE);

	return eel_string_list_contains (string_picker->detail->string_list, string);
}

/**
 * eel_string_picker_get_index_for_string
 * @string_picker: A EelStringPicker
 * @string: String to find.
 *
 * Return the index for the given string.  
 * Return EEL_STRING_LIST_NOT_FOUND if the string is not found.
 */
int 
eel_string_picker_get_index_for_string (const EelStringPicker *string_picker,
					const char *string)
{
	g_return_val_if_fail (EEL_IS_STRING_PICKER (string_picker), EEL_STRING_LIST_NOT_FOUND);

	return eel_string_list_get_index_for_string (string_picker->detail->string_list, string);
}

/**
 * eel_string_picker_clear:

 * @string_picker: A EelStringPicker
 *
 * Remove all entries from the string picker.
 */
void
eel_string_picker_clear (EelStringPicker *string_picker)
{
	EelStringList *empty_list;

	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));

	/* Already empty */
	if (eel_string_list_get_length (string_picker->detail->string_list) == 0) {
		return;
	}

	empty_list = eel_string_list_new (TRUE);
	eel_string_picker_set_string_list (string_picker, empty_list);
	eel_string_list_free (empty_list);
}

/* Set the list of insensitive strings */
void
eel_string_picker_set_insensitive_list (EelStringPicker *string_picker,
					const EelStringList *insensitive_list)
{
	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));

	/* Make sure the string list is different */
	if (eel_string_list_equals (insensitive_list, string_picker->detail->insensitive_list)) {
		return;
	}

	eel_string_list_assign_from_string_list (string_picker->detail->insensitive_list, insensitive_list);
	string_picker_update_menu_sensitivities (string_picker);
}
