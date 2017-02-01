/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-preferences-item.c - A preferences item is a widget that represents
                            a single eel preference key.

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
#include "eel-preferences-item.h"

#include "eel-preferences.h"
#include "eel-art-gtk-extensions.h"
#include "eel-enumeration.h"
#include "eel-font-picker.h"
#include "eel-glib-extensions.h"
#include "eel-gtk-extensions.h"
#include "eel-gtk-macros.h"
#include "eel-radio-button-group.h"
#include "eel-string-picker.h"
#include "eel-string.h"
#include "eel-text-caption.h"
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

#define PREFERENCES_ITEM_UNDEFINED_ITEM -1U

static gboolean text_idle_handler = FALSE;
static gboolean integer_idle_handler = FALSE;

typedef struct
{
	GtkWidget *widget;
	guint signal_id;
} PreferencesItemConnection;

struct EelPreferencesItemDetails
{
	char *preference_name;
	EelPreferencesItemType item_type;
	GtkWidget *main_child;
	GSList *change_signal_connections;
	char *control_preference_name;
	EelPreferencesItemControlAction control_action;
	EelStringList *enumeration_list_unique_exceptions;
};

/* Signals */
typedef enum
{
	CUSTOM_UPDATE_DISPLAYED_VALUE,
	CUSTOM_DESCRIPTION_CHANGED,
	LAST_SIGNAL
} EelStringListSignals;

static guint preferences_item_signals[LAST_SIGNAL];

/* GtkObjectClass methods */
static void eel_preferences_item_initialize_class                     (EelPreferencesItemClass *preferences_item_class);
static void eel_preferences_item_initialize                           (EelPreferencesItem      *preferences_item);
static void preferences_item_destroy                                  (GtkObject               *object);

/* Private stuff */
static void preferences_item_create_boolean                           (EelPreferencesItem      *item);
static void preferences_item_create_editable_integer                  (EelPreferencesItem      *item);
static void preferences_item_create_editable_string                   (EelPreferencesItem      *item);
static void preferences_item_create_enumeration_menu                  (EelPreferencesItem      *item);
static void preferences_item_create_enumeration_radio                 (EelPreferencesItem      *item,
								       gboolean                 horizontal);
static void preferences_item_create_enumeration_list                  (EelPreferencesItem      *item,
								       gboolean                 horizontal);
static void preferences_item_create_font                              (EelPreferencesItem      *item);
static void preferences_item_create_padding                           (EelPreferencesItem      *item);
static void preferences_item_create_smooth_font                       (EelPreferencesItem      *item);
static void preferences_item_update_displayed_value                   (EelPreferencesItem      *preferences_item);
static void preferences_item_update_editable_integer_settings_at_idle (EelPreferencesItem      *preferences_item);
static void preferences_item_update_text_settings_at_idle             (EelPreferencesItem      *preferences_item);
static void preferences_item_update_description                       (EelPreferencesItem      *preferences_item);

/* User triggered item changed callbacks */
static void enumeration_radio_changed_callback                        (EelRadioButtonGroup     *radio_button_group,
								       gpointer                 callback_data);
static void boolean_button_toggled_callback                           (GtkWidget               *button_group,
								       gpointer                 user_data);
static void editable_string_changed_callback                          (GtkWidget               *caption,
								       gpointer                 user_data);
static void editable_integer_changed_callback                         (GtkWidget               *caption,
								       gpointer                 user_data);
static void enumeration_menu_changed_callback                         (EelStringPicker         *string_picker,
								       EelPreferencesItem      *item);
static void font_changed_callback                                     (GtkWidget               *caption,
								       gpointer                 user_data);
static void enumeration_list_changed_callback                         (EelStringPicker         *string_picker,
								       EelPreferencesItem      *item);
static void smooth_font_changed_callback                              (EelFontPicker           *font_picker,
								       gpointer                 callback_data);
static void custom_changed_callback                                   (GtkWidget               *custom,
								       gpointer                 user_data);

EEL_DEFINE_CLASS_BOILERPLATE (EelPreferencesItem, eel_preferences_item, GTK_TYPE_VBOX)

/* EelPreferencesItemClass methods */
static void
eel_preferences_item_initialize_class (EelPreferencesItemClass *preferences_item_class)
{
	GtkObjectClass *object_class;

	object_class = GTK_OBJECT_CLASS (preferences_item_class);

	/* GtkObjectClass */
	object_class->destroy = preferences_item_destroy;

	/* Signals */
	preferences_item_signals[CUSTOM_UPDATE_DISPLAYED_VALUE] = 
		gtk_signal_new ("custom_update_displayed_value",
				GTK_RUN_LAST,
				object_class->type,
				0,
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 
				0);
	preferences_item_signals[CUSTOM_DESCRIPTION_CHANGED] = 
		gtk_signal_new ("custom_description_changed",
				GTK_RUN_LAST,
				object_class->type,
				0,
				gtk_marshal_NONE__POINTER_POINTER,
				GTK_TYPE_NONE, 
				2,
				GTK_TYPE_OBJECT,
				GTK_TYPE_STRING);
	gtk_object_class_add_signals (object_class, preferences_item_signals, LAST_SIGNAL);
}

static void
eel_preferences_item_initialize (EelPreferencesItem *item)
{
	item->details = g_new0 (EelPreferencesItemDetails, 1);
	item->details->item_type = PREFERENCES_ITEM_UNDEFINED_ITEM;
	item->details->enumeration_list_unique_exceptions = eel_string_list_new (TRUE);
}

/* GtkObjectClass methods */
static void
preferences_item_destroy (GtkObject *object)
{
	EelPreferencesItem * item;
	
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (object));
	
	item = EEL_PREFERENCES_ITEM (object);

	g_free (item->details->preference_name);
	g_free (item->details->control_preference_name);
	eel_g_slist_free_deep (item->details->change_signal_connections);
	eel_string_list_free (item->details->enumeration_list_unique_exceptions);
	g_free (item->details);

	/* Chain destroy */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/*
 * Private stuff
 */
static void
preferences_item_update_enumeration_radio (EelPreferencesItem *item)
{
	int value;
	char *enumeration_id;
	guint i;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (item->details->item_type == EEL_PREFERENCE_ITEM_ENUMERATION_VERTICAL_RADIO
			  || item->details->item_type == EEL_PREFERENCE_ITEM_ENUMERATION_HORIZONTAL_RADIO);

 	value = eel_preferences_get_integer (item->details->preference_name);

	enumeration_id = eel_preferences_get_enumeration_id (item->details->preference_name);
	g_return_if_fail (eel_strlen (enumeration_id) > 0);
	g_return_if_fail (eel_enumeration_id_get_length (enumeration_id) > 0);
	
	/* Set the active button */
	for (i = 0; i < eel_enumeration_id_get_length (enumeration_id); i++) {
		if (value == eel_enumeration_id_get_nth_value (enumeration_id, i)) {
			eel_radio_button_group_set_active_index (EEL_RADIO_BUTTON_GROUP (item->details->main_child), i);
		}
	}

 	g_free (enumeration_id);
}

/* Make sure the string pickers are wired such that duplicate choices cannot be
 * made by the user.  We do this by making items that would result in duplicates
 * insensitive.  Its possible to bypass this rule for some items.  We use the
 * use the enumeration_list_unique_exceptions for that.
 */
static void
preferences_item_update_enumeration_list_uniqueness (EelPreferencesItem *item)
{
	const GSList *node;
	guint i;
	guint j;
	const PreferencesItemConnection *connection;
	guint num_pickers;
	EelStringList **insensitive_lists;
	char *selected_string;

	g_return_if_fail (item->details->item_type == EEL_PREFERENCE_ITEM_ENUMERATION_LIST_VERTICAL
			  || item->details->item_type == EEL_PREFERENCE_ITEM_ENUMERATION_LIST_HORIZONTAL);
	
	num_pickers = g_slist_length (item->details->change_signal_connections);

	g_return_if_fail (num_pickers > 0);
	
	/* Allocate as many insensitive lists as we have string pickers */
	insensitive_lists = g_new (EelStringList *, num_pickers);
	for (j = 0; j < num_pickers; j++) {
		insensitive_lists[j] = eel_string_list_new (TRUE);
	}

	/* Populate the insensitive lists with the selected strings of all the
	 * other lists.
	 */
	for (node = item->details->change_signal_connections, i = 0; node != NULL; node = node->next, i++) {
		g_assert (node->data != NULL);

		connection = node->data;
		g_assert (EEL_IS_STRING_PICKER (connection->widget));
		
		selected_string = eel_string_picker_get_selected_string (EEL_STRING_PICKER (connection->widget));

		for (j = 0; j < num_pickers; j++) {
			if (j != i && !eel_string_list_contains (item->details->enumeration_list_unique_exceptions,
								 selected_string)) {
				eel_string_list_insert (insensitive_lists[j],
							selected_string);
			}
		}
		g_free (selected_string);
	}
	
	/* Install the insensitive lists on the string pickers */
	for (node = item->details->change_signal_connections, i = 0; node != NULL; node = node->next, i++) {
		g_assert (node->data != NULL);

		connection = node->data;
		g_assert (EEL_IS_STRING_PICKER (connection->widget));

		eel_string_picker_set_insensitive_list (EEL_STRING_PICKER (connection->widget),
							insensitive_lists[i]);
	}
	
	/* Free the insensitive lists */
	for (j = 0; j < num_pickers; j++) {
		eel_string_list_free (insensitive_lists[j]);
	}
	g_free (insensitive_lists);
}

static void
preferences_item_update_enumeration_list (EelPreferencesItem *item)
{
	char *enumeration_id;
	const GSList *node;
	EelStringList *value;
	char *nth_value_name;
	char *nth_value_description;
	guint i;
	int position;
	const PreferencesItemConnection *connection;

	g_return_if_fail (item->details->item_type == EEL_PREFERENCE_ITEM_ENUMERATION_LIST_VERTICAL
			  || item->details->item_type == EEL_PREFERENCE_ITEM_ENUMERATION_LIST_HORIZONTAL);
	
	enumeration_id = eel_preferences_get_enumeration_id (item->details->preference_name);
	g_return_if_fail (eel_strlen (enumeration_id) > 0);
	g_return_if_fail (eel_enumeration_id_get_length (enumeration_id) > 0);

 	value = eel_preferences_get_string_list (item->details->preference_name);

	g_return_if_fail (eel_string_list_get_length (value)
			  == g_slist_length (item->details->change_signal_connections));

	for (node = item->details->change_signal_connections, i = 0; node != NULL; node = node->next, i++) {
		g_assert (node->data != NULL);

		connection = node->data;
		g_assert (EEL_IS_STRING_PICKER (connection->widget));
		
		nth_value_name = eel_string_list_nth (value, i);
		
		position = eel_enumeration_id_get_name_position (enumeration_id,
								 nth_value_name);
		
		nth_value_description = eel_enumeration_id_get_nth_description_translated (enumeration_id,
											   position);
		
		eel_string_picker_set_selected_string (EEL_STRING_PICKER (connection->widget),
						       nth_value_description);
		
		g_free (nth_value_name);
		g_free (nth_value_description);
	}

	eel_string_list_free (value);
 	g_free (enumeration_id);

	preferences_item_update_enumeration_list_uniqueness (item);
}

static void
preferences_item_update_custom (EelPreferencesItem *item)
{
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));

	gtk_signal_emit (GTK_OBJECT (item),
			 preferences_item_signals[CUSTOM_UPDATE_DISPLAYED_VALUE]);
}

/* This callback is called whenever the preference value changes, so that we can
 * update the item widgets accordingly.
 */
static void
preferences_item_value_changed_callback (gpointer callback_data)
{
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (callback_data));
	
	preferences_item_update_displayed_value (EEL_PREFERENCES_ITEM (callback_data));
}

static void
preferences_item_set_main_child (EelPreferencesItem *item,
				 GtkWidget *child)
{
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (eel_strlen (item->details->preference_name) > 0);
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (item->details->main_child == NULL);

	if (item->details->item_type != EEL_PREFERENCE_ITEM_PADDING) {
		eel_preferences_add_callback_while_alive (item->details->preference_name,
							  preferences_item_value_changed_callback,
							  item,
							  GTK_OBJECT (item));
	}

	gtk_box_pack_start (GTK_BOX (item),
			    child,
			    FALSE,
			    FALSE,
			    0);
	
	gtk_widget_show (child);

	item->details->main_child = child;

	preferences_item_update_description (item);
}

static void
preferences_item_add_connection_child (EelPreferencesItem *item,
				       GtkWidget *child,
				       const char *signal_name,
				       GtkSignalFunc signal)
{
	PreferencesItemConnection *connection;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (eel_strlen (signal_name) > 0);
	g_return_if_fail (signal != NULL);

	connection = g_new0 (PreferencesItemConnection, 1);
	connection->widget = child;
	connection->signal_id = gtk_signal_connect (GTK_OBJECT (child),
						    signal_name,
						    signal,
						    item);
	
	item->details->change_signal_connections = g_slist_append (
		item->details->change_signal_connections, connection);
}

static void
preferences_item_create_enumeration_radio (EelPreferencesItem *item,
					   gboolean horizontal)
{
 	guint i;
	char *enumeration_id;
	char *description;
	GtkWidget *child;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (eel_strlen (item->details->preference_name) > 0);

	enumeration_id = eel_preferences_get_enumeration_id (item->details->preference_name);
	g_return_if_fail (eel_strlen (enumeration_id) > 0);
	g_return_if_fail (eel_enumeration_id_get_length (enumeration_id) > 0);
	
	child = eel_radio_button_group_new (horizontal);

	/* Populate the radio group */
	for (i = 0; i < eel_enumeration_id_get_length (enumeration_id); i++) {
		description = eel_enumeration_id_get_nth_description_translated (enumeration_id, i);
		g_assert (description != NULL);
		
		eel_radio_button_group_insert (EEL_RADIO_BUTTON_GROUP (child),
					       description);
		g_free (description);
	}
 	g_free (enumeration_id);
	
	preferences_item_add_connection_child (item,
					       child,
					       "changed",
					       GTK_SIGNAL_FUNC (enumeration_radio_changed_callback));

	preferences_item_set_main_child (item, child);
}

static void
preferences_item_create_enumeration_list (EelPreferencesItem *item,
					  gboolean horizontal)
{
 	guint i;
 	guint j;
	char *enumeration_id;
	char *enum_description;
	EelStringList *default_value;
	guint num_pickers;
	GtkWidget *string_picker;
	GtkWidget *picker_box;
	GtkWidget *title;
	GtkWidget *child;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (eel_strlen (item->details->preference_name) > 0);

	enumeration_id = eel_preferences_get_enumeration_id (item->details->preference_name);
	g_return_if_fail (eel_strlen (enumeration_id) > 0);
	g_return_if_fail (eel_enumeration_id_get_length (enumeration_id) > 0);

	/* FIXME: Hard coded user level */
	default_value = eel_preferences_default_get_string_list (item->details->preference_name, 0);

	num_pickers = eel_string_list_get_length (default_value);
	g_return_if_fail (num_pickers > 0);
	
	child = gtk_vbox_new (FALSE, 4);
	picker_box = horizontal ? gtk_hbox_new (FALSE, 4): gtk_vbox_new (FALSE, 4);

	title = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (title), 0.0, 0.5);
	gtk_label_set_justify (GTK_LABEL (title), GTK_JUSTIFY_LEFT);

	gtk_box_pack_start (GTK_BOX (child),
			    title,
			    FALSE,
			    FALSE,
			    0);

	gtk_box_pack_start (GTK_BOX (child),
			    picker_box,
			    TRUE,
			    TRUE,
			    0);

	gtk_widget_show (title);
	gtk_widget_show (picker_box);
	
	/* Populate the string pickers */
	for (j = 0; j < num_pickers; j++) {
		string_picker = eel_string_picker_new ();
		eel_caption_set_show_title (EEL_CAPTION (string_picker), FALSE);
		
		for (i = 0; i < eel_enumeration_id_get_length (enumeration_id); i++) {
			enum_description = eel_enumeration_id_get_nth_description_translated (enumeration_id, i);
			g_assert (enum_description != NULL);

			if (enum_description[0] == '-') {
				eel_string_picker_insert_separator (EEL_STRING_PICKER (string_picker));
			} else {
				eel_string_picker_insert_string (EEL_STRING_PICKER (string_picker), enum_description);
			}
			g_free (enum_description);
		}

		gtk_box_pack_start (GTK_BOX (picker_box),
				    string_picker,
				    FALSE,
				    FALSE,
				    0);
		
		gtk_widget_show (string_picker);

		preferences_item_add_connection_child (item,
						       string_picker,
						       "changed",
						       GTK_SIGNAL_FUNC (enumeration_list_changed_callback));
	}
 	g_free (enumeration_id);

	preferences_item_set_main_child (item, child);
}

static void
preferences_item_update_boolean (EelPreferencesItem *item)
{
	gboolean value;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (item->details->item_type == EEL_PREFERENCE_ITEM_BOOLEAN);

	value = eel_preferences_get_boolean (item->details->preference_name);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (item->details->main_child), value);
}

static void
preferences_item_create_boolean (EelPreferencesItem *item)
{
	GtkWidget *child;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (eel_strlen (item->details->preference_name) > 0);

	child = gtk_check_button_new_with_label ("");
	gtk_label_set_justify (GTK_LABEL (GTK_BIN (child)->child), GTK_JUSTIFY_LEFT);
	
	preferences_item_add_connection_child (item,
					       child,
					       "toggled",
					       GTK_SIGNAL_FUNC (boolean_button_toggled_callback));
				      
	preferences_item_set_main_child (item, child);
}

static void
preferences_item_update_editable_string (EelPreferencesItem *item)
{
	char *current_value;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (item->details->item_type == EEL_PREFERENCE_ITEM_EDITABLE_STRING);

	current_value = eel_preferences_get (item->details->preference_name);

	g_assert (current_value != NULL);
	if (strcmp (eel_text_caption_get_text (EEL_TEXT_CAPTION (item->details->main_child)),
		    current_value) != 0) {
		eel_text_caption_set_text (EEL_TEXT_CAPTION (item->details->main_child), 
					   current_value);
	}

	g_free (current_value);
}

static void
preferences_item_create_editable_string (EelPreferencesItem *item)
{
	GtkWidget *child;
	
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (eel_strlen (item->details->preference_name) > 0);

	child = eel_text_caption_new ();

 	/* FIXME:
	 * This is a Nautilus specific special case for the home uri preference,
	 * in the future this should be generalized.
	 */
 	if (g_strcasecmp (item->details->preference_name, "preferences/home_uri") == 0) {
 		eel_text_caption_set_expand_tilde (EEL_TEXT_CAPTION (child), TRUE);
 	}
	
	eel_caption_set_title_label (EEL_CAPTION (child), "");
	
	preferences_item_add_connection_child (item,
					       child,
					       "changed",
					       GTK_SIGNAL_FUNC (editable_string_changed_callback));

	preferences_item_set_main_child (item, child);
}

static void
preferences_item_update_editable_integer (EelPreferencesItem *item)
{
	char *current_value;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (item->details->item_type == EEL_PREFERENCE_ITEM_EDITABLE_INTEGER);

	current_value = g_strdup_printf ("%d", eel_preferences_get_integer (item->details->preference_name));

	g_assert (current_value != NULL);

	if (strcmp (eel_text_caption_get_text (EEL_TEXT_CAPTION (item->details->main_child)),
		    current_value) != 0) {
		eel_text_caption_set_text (EEL_TEXT_CAPTION (item->details->main_child), 
					   current_value);
	}

	g_free (current_value);
}

static void
preferences_item_create_editable_integer (EelPreferencesItem *item)
{
	GtkWidget *child;
	
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (eel_strlen (item->details->preference_name) > 0);

	child = eel_text_caption_new ();

	eel_caption_set_title_label (EEL_CAPTION (child), "");

	preferences_item_add_connection_child (item,
					       child,
					       "changed",
					       GTK_SIGNAL_FUNC (editable_integer_changed_callback));

	preferences_item_set_main_child (item, child);
}

static void
preferences_item_update_enumeration_menu (EelPreferencesItem *item)
{
	char *current_label;
	int current_value;
	int position;
	char *enumeration_id;
	
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (item->details->item_type == EEL_PREFERENCE_ITEM_ENUMERATION_MENU);

	enumeration_id = eel_preferences_get_enumeration_id (item->details->preference_name);

	g_return_if_fail (eel_strlen (enumeration_id) > 0);
	g_return_if_fail (eel_enumeration_id_get_length (enumeration_id) > 0);

	current_value = eel_preferences_get_integer (item->details->preference_name);

	position = eel_enumeration_id_get_value_position (enumeration_id,
							  current_value);
	g_return_if_fail (position != EEL_STRING_LIST_NOT_FOUND);
	
	current_label = eel_enumeration_id_get_nth_description_translated (enumeration_id,
									   position);
	
	if (eel_string_picker_contains (EEL_STRING_PICKER (item->details->main_child), current_label)) {
		eel_string_picker_set_selected_string (EEL_STRING_PICKER (item->details->main_child),
						       current_label);
	} else {
		g_warning ("Value string for %s is %s, which isn't in the expected set of values",
			   item->details->preference_name,
			   current_label);
	}
	
 	g_free (enumeration_id);
	g_free (current_label);
}

static void
preferences_item_create_enumeration_menu (EelPreferencesItem *item)
{
 	guint i;
	char *enumeration_id;
	GtkWidget *child;
	char *enum_description;
	
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (eel_strlen (item->details->preference_name) > 0);
	
	child = eel_string_picker_new ();
	eel_caption_set_title_label (EEL_CAPTION (child), "");

	enumeration_id = eel_preferences_get_enumeration_id (item->details->preference_name);
	g_return_if_fail (eel_strlen (enumeration_id) > 0);
	g_return_if_fail (eel_enumeration_id_get_length (enumeration_id) > 0);
	
	/* Populate the string picker */
	for (i = 0; i < eel_enumeration_id_get_length (enumeration_id); i++) {
		enum_description = eel_enumeration_id_get_nth_description_translated (enumeration_id, i);
		g_assert (enum_description != NULL);
		
		if (enum_description[0] == '-') {
			eel_string_picker_insert_separator (EEL_STRING_PICKER (child));
		} else {
			eel_string_picker_insert_string (EEL_STRING_PICKER (child), enum_description);
		}
		
		g_free (enum_description);
	}
 	g_free (enumeration_id);

	preferences_item_add_connection_child (item,
					       child,
					       "changed",
					       GTK_SIGNAL_FUNC (enumeration_menu_changed_callback));

	preferences_item_set_main_child (item, child);
}

static void
preferences_item_update_font (EelPreferencesItem *item)
{
	char *current_value;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (item->details->item_type == EEL_PREFERENCE_ITEM_FONT);

	current_value = eel_preferences_get (item->details->preference_name);
	g_assert (current_value != NULL);

	/* The value of the gconf preference can be anything.  In theory garbage could
	 * be used for the preference using a third party tool.  So we make sure that
	 * it is one of the choice before trying to select it, otherwise we would get
	 * assertions.
	 */
	if (eel_string_picker_contains (EEL_STRING_PICKER (item->details->main_child), current_value)) {
		eel_string_picker_set_selected_string (EEL_STRING_PICKER (item->details->main_child),
						       current_value);
	}

	g_free (current_value);
}

static void
preferences_item_create_font (EelPreferencesItem *item)
{
	EelStringList *font_list;
	GtkWidget *child;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (eel_strlen (item->details->preference_name) > 0);

	child = eel_string_picker_new ();
	eel_caption_set_title_label (EEL_CAPTION (child), "");
	
	/* FIXME bugzilla.eazel.com 1274: Need to query system for available fonts */
	font_list = eel_string_list_new (TRUE);

	/* Once upon a time we had a bug in Eel that caused crashes with the "fixed" 
	 * font.  That bug (2256) was fixed by removing the "fixed" choice from this menu
	 * below.  Subsequently we fixed many font bugs in eel (such hard coded font sizes)
	 * that would cause both crashes and ugliness. Bug 2256 seems to have been fixed by
	 * these changes as well.
	 *
	 * Anyhow, the "fixed" font choice is not very interesting because the other fonts 
	 * look much better.  However, in multi byte locales, the fixed font is usually the
	 * only one that is available at the right encoding.
	 */

	/* FIXME bugzilla.eazel.com 7907: 
	 * The "GTK System Font" string is hard coded in many places.
	 */
	eel_string_list_insert (font_list, "GTK System Font");
	eel_string_list_insert (font_list, "fixed");
	eel_string_list_insert (font_list, "helvetica");
	eel_string_list_insert (font_list, "times");
	eel_string_list_insert (font_list, "courier");
	eel_string_list_insert (font_list, "lucida");

	eel_string_picker_set_string_list (EEL_STRING_PICKER (child), font_list);
	eel_string_list_free (font_list);
	
	preferences_item_add_connection_child (item,
					       child,
					       "changed",
					       GTK_SIGNAL_FUNC (font_changed_callback));
	
	preferences_item_set_main_child (item, child);
}

static void
preferences_item_create_padding (EelPreferencesItem *item)
{
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));

	preferences_item_set_main_child (item, gtk_label_new (""));
}

static void
preferences_item_update_smooth_font (EelPreferencesItem *item)
{
 	char *current_value;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (item->details->item_type == EEL_PREFERENCE_ITEM_SMOOTH_FONT);

 	current_value = eel_preferences_get (item->details->preference_name);
 	g_assert (current_value != NULL);

 	eel_font_picker_set_selected_font (EEL_FONT_PICKER (item->details->main_child),
					   current_value);
 	g_free (current_value);
}

static void
smooth_font_changed_callback (EelFontPicker *font_picker,
			      gpointer callback_data)
{
	EelPreferencesItem *item;
 	char *new_value;

	g_return_if_fail (EEL_IS_FONT_PICKER (font_picker));
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (callback_data));

	item = EEL_PREFERENCES_ITEM (callback_data);
 	new_value = eel_font_picker_get_selected_font (EEL_FONT_PICKER (item->details->main_child));
 	g_assert (new_value != NULL);
	eel_preferences_set (item->details->preference_name, new_value);
 	g_free (new_value);
}

static void
preferences_item_create_smooth_font (EelPreferencesItem *item)
{
	GtkWidget *child;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (eel_strlen (item->details->preference_name) > 0);

	child = eel_font_picker_new ();
	eel_caption_set_title_label (EEL_CAPTION (child), "");

	preferences_item_add_connection_child (item,
					       child,
					       "changed",
					       GTK_SIGNAL_FUNC (smooth_font_changed_callback));

	preferences_item_set_main_child (item, child);
}

static void
custom_changed_callback (GtkWidget *widget,
			 gpointer callback_data)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
}

/* EelPreferencesItem public methods */
GtkWidget *
eel_preferences_item_new (const char *preference_name,
			  EelPreferencesItemType item_type)
{
	EelPreferencesItem *item;

	g_return_val_if_fail (eel_strlen (preference_name) > 0, NULL);
	g_return_val_if_fail (item_type >= EEL_PREFERENCE_ITEM_BOOLEAN, FALSE);
	g_return_val_if_fail (item_type <= EEL_PREFERENCE_ITEM_SMOOTH_FONT, FALSE);

	item = EEL_PREFERENCES_ITEM
		(gtk_widget_new (eel_preferences_item_get_type (), NULL));

	item->details->preference_name = g_strdup (preference_name);
	item->details->item_type = item_type;

	/* Create the child widget according to the item type */
	switch (item_type)
	{
	case EEL_PREFERENCE_ITEM_BOOLEAN:
		preferences_item_create_boolean (item);
		break;
		
	case EEL_PREFERENCE_ITEM_ENUMERATION_VERTICAL_RADIO:
		preferences_item_create_enumeration_radio (item, FALSE);
		break;

	case EEL_PREFERENCE_ITEM_ENUMERATION_HORIZONTAL_RADIO:
		preferences_item_create_enumeration_radio (item, TRUE);
		break;

	case EEL_PREFERENCE_ITEM_ENUMERATION_LIST_VERTICAL:
		preferences_item_create_enumeration_list (item, FALSE);
		break;

	case EEL_PREFERENCE_ITEM_ENUMERATION_LIST_HORIZONTAL:
		preferences_item_create_enumeration_list (item, TRUE);
		break;

	case EEL_PREFERENCE_ITEM_FONT:
		preferences_item_create_font (item);
		break;
	
	case EEL_PREFERENCE_ITEM_SMOOTH_FONT:
		preferences_item_create_smooth_font (item);
		break;
	
	case EEL_PREFERENCE_ITEM_EDITABLE_STRING:
		preferences_item_create_editable_string (item);
		break;	

	case EEL_PREFERENCE_ITEM_EDITABLE_INTEGER:
		preferences_item_create_editable_integer (item);
		break;	

	case EEL_PREFERENCE_ITEM_ENUMERATION_MENU:
		preferences_item_create_enumeration_menu (item);
		break;	

	case EEL_PREFERENCE_ITEM_PADDING:
		preferences_item_create_padding (item);
		break;	

	case EEL_PREFERENCE_ITEM_CUSTOM:
		g_assert_not_reached ();
		break;	
	}

	g_return_val_if_fail (GTK_IS_WIDGET (item->details->main_child), NULL);

	preferences_item_update_displayed_value (item);

	return GTK_WIDGET (item);
}

GtkWidget*
eel_preferences_item_new_custom (const char *preference_name,
				 GtkWidget *child,
				 const char *signal_name)
{
	EelPreferencesItem *item;

	g_return_val_if_fail (eel_strlen (preference_name) > 0, NULL);
	g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
	g_return_val_if_fail (eel_strlen (signal_name) > 0, NULL);
	
	item = EEL_PREFERENCES_ITEM
		(gtk_widget_new (eel_preferences_item_get_type (), NULL));

	item->details->preference_name = g_strdup (preference_name);
	item->details->item_type = EEL_PREFERENCE_ITEM_CUSTOM;

	preferences_item_add_connection_child (item,
					       child,
					       signal_name,
					       GTK_SIGNAL_FUNC (custom_changed_callback));

	preferences_item_set_main_child (item, child);

	return GTK_WIDGET (item);
}

static void
enumeration_radio_changed_callback (EelRadioButtonGroup *radio_button_group,
				    gpointer callback_data)
{
	EelPreferencesItem *item;
	int i;
	char *enumeration_id;

	g_return_if_fail (EEL_IS_RADIO_BUTTON_GROUP (radio_button_group));
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (callback_data));
	
	item = EEL_PREFERENCES_ITEM (callback_data);

	g_assert (item->details->preference_name != NULL);

	i = eel_radio_button_group_get_active_index (radio_button_group);

	enumeration_id = eel_preferences_get_enumeration_id (item->details->preference_name);
	g_return_if_fail (eel_strlen (enumeration_id) > 0);
	g_return_if_fail ((guint)i < eel_enumeration_id_get_length (enumeration_id));
	
	eel_preferences_set_integer (item->details->preference_name,
				     eel_enumeration_id_get_nth_value (enumeration_id, i));
	g_free (enumeration_id);
}

static void
boolean_button_toggled_callback (GtkWidget *button, gpointer user_data)
{
	EelPreferencesItem *item;
	gboolean		active_state;

	g_assert (user_data != NULL);
	g_assert (EEL_IS_PREFERENCES_ITEM (user_data));
	
	item = EEL_PREFERENCES_ITEM (user_data);

	active_state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));

	eel_preferences_set_boolean (item->details->preference_name, active_state);
}

static void
font_changed_callback (GtkWidget *string_picker, gpointer user_data)
{
	EelPreferencesItem *item;
	char *selected_string;

	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (user_data));

	item = EEL_PREFERENCES_ITEM (user_data);

	g_return_if_fail (item->details->preference_name != NULL);

	selected_string = eel_string_picker_get_selected_string (EEL_STRING_PICKER (string_picker));
	g_return_if_fail (selected_string != NULL);

	eel_preferences_set (item->details->preference_name, selected_string);

	g_free (selected_string);
}

static void
editable_string_changed_callback (GtkWidget *button, gpointer user_data)
{
	EelPreferencesItem *item;
	
	g_assert (user_data != NULL);
	g_assert (EEL_IS_PREFERENCES_ITEM (user_data));

	item = EEL_PREFERENCES_ITEM (user_data);

	g_assert (item->details->main_child != NULL);
	g_assert (EEL_IS_TEXT_CAPTION (item->details->main_child));

	preferences_item_update_text_settings_at_idle (item);
}

static void
editable_integer_changed_callback (GtkWidget *button, gpointer user_data)
{
	EelPreferencesItem *item;
	
	g_assert (user_data != NULL);
	g_assert (EEL_IS_PREFERENCES_ITEM (user_data));

	item = EEL_PREFERENCES_ITEM (user_data);

	g_assert (item->details->main_child != NULL);
	g_assert (EEL_IS_TEXT_CAPTION (item->details->main_child));

	preferences_item_update_editable_integer_settings_at_idle (item);
}

static void
enumeration_menu_changed_callback (EelStringPicker *string_picker,
				   EelPreferencesItem *item)
{
 	char *selected_label;
	int position;
	int new_value;
	char *enumeration_id;

	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));

	enumeration_id = eel_preferences_get_enumeration_id (item->details->preference_name);
	g_return_if_fail (eel_strlen (enumeration_id) > 0);
	g_return_if_fail (eel_enumeration_id_get_length (enumeration_id) > 0);

 	selected_label = eel_string_picker_get_selected_string (string_picker);
	g_return_if_fail (selected_label != NULL);

	position = eel_enumeration_id_get_description_position (enumeration_id,
								selected_label);
	g_free (selected_label);
	g_return_if_fail (position != EEL_STRING_LIST_NOT_FOUND);
	
	new_value = eel_enumeration_id_get_nth_value (enumeration_id,
						      position);
	
	eel_preferences_set_integer (item->details->preference_name, new_value);

 	g_free (enumeration_id);
}

static void
enumeration_list_changed_callback (EelStringPicker *string_picker,
				   EelPreferencesItem *item)
{
	const GSList *node;
	const PreferencesItemConnection *connection;
	char *selected_label;
	char *enumeration_id;
	int position;
	char *new_value_nth_name;
	EelStringList *new_value;

	g_return_if_fail (EEL_IS_STRING_PICKER (string_picker));
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));

	enumeration_id = eel_preferences_get_enumeration_id (item->details->preference_name);
	g_return_if_fail (eel_strlen (enumeration_id) > 0);
	g_return_if_fail (eel_enumeration_id_get_length (enumeration_id) > 0);

	new_value = eel_string_list_new (TRUE);

	for (node = item->details->change_signal_connections; node != NULL; node = node->next) {
		g_assert (node->data != NULL);
		connection = node->data;
		g_assert (EEL_IS_STRING_PICKER (connection->widget));

		selected_label = eel_string_picker_get_selected_string (EEL_STRING_PICKER (connection->widget));
		g_return_if_fail (selected_label != NULL);

		position = eel_enumeration_id_get_description_position (enumeration_id,
									selected_label);
		g_free (selected_label);
		g_return_if_fail (position != EEL_STRING_LIST_NOT_FOUND);
		
		new_value_nth_name = eel_enumeration_id_get_nth_name (enumeration_id,
								      position);

		eel_string_list_insert (new_value, new_value_nth_name);

		g_free (new_value_nth_name);
	}

	g_return_if_fail (eel_string_list_get_length (new_value)
			  == g_slist_length (item->details->change_signal_connections));

	eel_preferences_set_string_list (item->details->preference_name, new_value);

	eel_string_list_free (new_value);

 	g_free (enumeration_id);

	preferences_item_update_enumeration_list_uniqueness (item);
}

char *
eel_preferences_item_get_name (const EelPreferencesItem *item)
{
	g_return_val_if_fail (EEL_IS_PREFERENCES_ITEM (item), NULL);

	return g_strdup (item->details->preference_name);
}

static void
preferences_item_update_displayed_value (EelPreferencesItem *item)
{
	EelPreferencesItemType item_type;
	const GSList *node;
	const PreferencesItemConnection *connection;
	
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));

	item_type = item->details->item_type;

	g_return_if_fail (item->details->item_type != PREFERENCES_ITEM_UNDEFINED_ITEM);

	/* Block the change signals while we update the widget to match the preference */
	for (node = item->details->change_signal_connections; node != NULL; node = node->next) {
		g_assert (node->data != NULL);
		connection = node->data;
		g_assert (GTK_IS_WIDGET (connection->widget));
		
		gtk_signal_handler_block (GTK_OBJECT (connection->widget),
					  connection->signal_id);
	}

	/* Update the child widget according to the item type */
	switch (item_type)
	{
	case EEL_PREFERENCE_ITEM_BOOLEAN:
		preferences_item_update_boolean (item);
		break;
		
	case EEL_PREFERENCE_ITEM_ENUMERATION_VERTICAL_RADIO:
	case EEL_PREFERENCE_ITEM_ENUMERATION_HORIZONTAL_RADIO:
		preferences_item_update_enumeration_radio (item);
		break;

	case EEL_PREFERENCE_ITEM_ENUMERATION_LIST_VERTICAL:
	case EEL_PREFERENCE_ITEM_ENUMERATION_LIST_HORIZONTAL:
		preferences_item_update_enumeration_list (item);
		break;

	case EEL_PREFERENCE_ITEM_FONT:
		preferences_item_update_font (item);
		break;
	
	case EEL_PREFERENCE_ITEM_SMOOTH_FONT:
		preferences_item_update_smooth_font (item);
		break;
	
	case EEL_PREFERENCE_ITEM_EDITABLE_STRING:
		preferences_item_update_editable_string (item);
		break;	

	case EEL_PREFERENCE_ITEM_EDITABLE_INTEGER:
		preferences_item_update_editable_integer (item);
		break;

	case EEL_PREFERENCE_ITEM_ENUMERATION_MENU:
		preferences_item_update_enumeration_menu (item);
		break;

	case EEL_PREFERENCE_ITEM_PADDING:
		break;

	case EEL_PREFERENCE_ITEM_CUSTOM:
		preferences_item_update_custom (item);
		break;
	default:
		g_assert_not_reached ();
	}

	for (node = item->details->change_signal_connections; node != NULL; node = node->next) {
		g_assert (node->data != NULL);
		connection = node->data;
		g_assert (GTK_IS_WIDGET (connection->widget));
		
		gtk_signal_handler_unblock (GTK_OBJECT (connection->widget),
					    connection->signal_id);
	}
}

static gboolean
update_text_settings_at_idle (EelPreferencesItem *item)
{
	char *text;

	text = eel_text_caption_get_text (EEL_TEXT_CAPTION (item->details->main_child));

	if (text != NULL) {
		eel_preferences_set (item->details->preference_name, text);
		g_free (text);
	}
	
	text_idle_handler = FALSE;

	return FALSE;
}

static void
preferences_item_update_text_settings_at_idle (EelPreferencesItem *item)
{
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));

	if (text_idle_handler == FALSE) {
		gtk_idle_add ((GtkFunction) update_text_settings_at_idle, item);
		text_idle_handler = TRUE;
	}
}

static gboolean
update_integer_settings_at_idle (EelPreferencesItem *item)
{
	int value = 0;
	char *text;

	text = eel_text_caption_get_text (EEL_TEXT_CAPTION (item->details->main_child));

	if (text != NULL) {
		eel_eat_str_to_int (text, &value);
	}
	
	eel_preferences_set_integer (item->details->preference_name, value);

	integer_idle_handler = FALSE;

	return FALSE;
}

static void
preferences_item_update_editable_integer_settings_at_idle (EelPreferencesItem *item)
{
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));

	if (integer_idle_handler == FALSE) {
		gtk_idle_add ((GtkFunction) update_integer_settings_at_idle, item);
		integer_idle_handler = TRUE;
	}
}

static void
preferences_item_update_description (EelPreferencesItem *item)
{
	char *description;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (eel_strlen (item->details->preference_name) > 0);
	
	description = eel_preferences_get_description (item->details->preference_name);
	g_return_if_fail (description != NULL);
	eel_preferences_item_set_description (item, description);
	g_free (description);
}

void
eel_preferences_item_set_control_preference (EelPreferencesItem *item,
					     const char *control_preference_name)
{
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));

	if (eel_str_is_equal (item->details->control_preference_name,
			      control_preference_name)) {
		return;
	}

	g_free (item->details->control_preference_name);
	item->details->control_preference_name = g_strdup (control_preference_name);
}

void
eel_preferences_item_set_control_action (EelPreferencesItem *item,
					 EelPreferencesItemControlAction control_action)
{
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (control_action >= EEL_PREFERENCE_ITEM_SHOW);
	g_return_if_fail (control_action <= EEL_PREFERENCE_ITEM_HIDE);

	if (item->details->control_action == control_action) {
		return;
	}

	item->details->control_action = control_action;
}

static gboolean
preferences_item_get_control_showing (const EelPreferencesItem *item)
{
	gboolean value;

	g_return_val_if_fail (EEL_IS_PREFERENCES_ITEM (item), FALSE);

	if (item->details->control_preference_name == NULL) {
		return TRUE;
	}

	value = eel_preferences_get_boolean (item->details->control_preference_name);

	if (item->details->control_action == EEL_PREFERENCE_ITEM_SHOW) {
		return value;
	}

	if (item->details->control_action == EEL_PREFERENCE_ITEM_HIDE) {
		return !value;
	}

	return !value;
}

gboolean
eel_preferences_item_child_is_caption (const EelPreferencesItem *item)
{
	g_return_val_if_fail (EEL_IS_PREFERENCES_ITEM (item), FALSE);

	return EEL_IS_CAPTION (item->details->main_child);
}

int
eel_preferences_item_get_child_width (const EelPreferencesItem *item)
{
	EelDimensions child_dimensions;

	g_return_val_if_fail (EEL_IS_PREFERENCES_ITEM (item), 0);

	if (item->details->main_child == NULL) {
		return 0;
	}

	child_dimensions = eel_gtk_widget_get_preferred_dimensions (item->details->main_child);
	
	return child_dimensions.width;
}

void
eel_preferences_item_set_caption_extra_spacing (EelPreferencesItem *item,
						int extra_spacing)
{
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (extra_spacing >= 0);

	if (!eel_preferences_item_child_is_caption (item)) {
		return;
	}

	eel_caption_set_extra_spacing (EEL_CAPTION (item->details->main_child), 
				       extra_spacing);
}

gboolean
eel_preferences_item_is_showing (const EelPreferencesItem *item)
{
	g_return_val_if_fail (EEL_IS_PREFERENCES_ITEM (item), FALSE);
	
	if (item->details->item_type == EEL_PREFERENCE_ITEM_PADDING) {
		return TRUE;
	} else if (eel_preferences_is_visible (item->details->preference_name)) {
		return preferences_item_get_control_showing (item);
	}
	
	return FALSE;
}

void
eel_preferences_item_update_showing (EelPreferencesItem *item)
{
	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));

	eel_gtk_widget_set_shown (GTK_WIDGET (item),
				  eel_preferences_item_is_showing (item));
}

void
eel_preferences_item_enumeration_list_set_unique_exceptions (EelPreferencesItem *item,
							     const char *exceptions,
							     const char *exceptions_delimeter)
{
	EelStringList *new_exceptions;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (item->details->item_type == EEL_PREFERENCE_ITEM_ENUMERATION_LIST_VERTICAL
			  || item->details->item_type == EEL_PREFERENCE_ITEM_ENUMERATION_LIST_HORIZONTAL);
	g_return_if_fail (eel_strlen (exceptions_delimeter) > 0);

	new_exceptions = eel_string_list_new_from_tokens (exceptions,
							  exceptions_delimeter,
							  TRUE);

	if (eel_string_list_equals (new_exceptions, item->details->enumeration_list_unique_exceptions)) {
		eel_string_list_free (new_exceptions);
		return;
	}

	eel_string_list_free (item->details->enumeration_list_unique_exceptions);
	item->details->enumeration_list_unique_exceptions = new_exceptions;

	preferences_item_update_enumeration_list_uniqueness (item);
}

void
eel_preferences_item_set_description (EelPreferencesItem *item,
				      const char *description)
{
	GList *node;

	g_return_if_fail (EEL_IS_PREFERENCES_ITEM (item));
	g_return_if_fail (description != NULL);

	switch (item->details->item_type) {
	case EEL_PREFERENCE_ITEM_BOOLEAN:
		g_assert (GTK_IS_CHECK_BUTTON (item->details->main_child));
		g_assert (GTK_IS_LABEL (GTK_BIN (item->details->main_child)->child));
		gtk_label_set_text (GTK_LABEL (GTK_BIN (item->details->main_child)->child),
				    description);
		break;
	case EEL_PREFERENCE_ITEM_ENUMERATION_VERTICAL_RADIO:
	case EEL_PREFERENCE_ITEM_ENUMERATION_HORIZONTAL_RADIO:
		/* These 2 dont have a visible description.  */
		break;
	case EEL_PREFERENCE_ITEM_ENUMERATION_LIST_VERTICAL:
	case EEL_PREFERENCE_ITEM_ENUMERATION_LIST_HORIZONTAL:
		g_assert (GTK_IS_BOX (item->details->main_child));
		node = g_list_first (GTK_BOX (item->details->main_child)->children);
		g_assert (node != NULL);
		g_assert (node->data != NULL);
		g_assert (GTK_IS_LABEL (((GtkBoxChild *) node->data)->widget));
		gtk_label_set_text (GTK_LABEL (((GtkBoxChild *) node->data)->widget),
				    description);
		break;
	case EEL_PREFERENCE_ITEM_FONT:
	case EEL_PREFERENCE_ITEM_SMOOTH_FONT:
	case EEL_PREFERENCE_ITEM_EDITABLE_STRING:
	case EEL_PREFERENCE_ITEM_EDITABLE_INTEGER:
	case EEL_PREFERENCE_ITEM_ENUMERATION_MENU:
		g_assert (EEL_IS_CAPTION (item->details->main_child));
		eel_caption_set_title_label (EEL_CAPTION (item->details->main_child),
					     description);
		break;	
	case EEL_PREFERENCE_ITEM_PADDING:
		/* Padding doesnt have a description */
		break;
	case EEL_PREFERENCE_ITEM_CUSTOM:
		gtk_signal_emit (GTK_OBJECT (item),
				 preferences_item_signals[CUSTOM_DESCRIPTION_CHANGED],
				 item->details->main_child,
				 description);
		break;
	default:
		g_assert_not_reached ();
	}
}
