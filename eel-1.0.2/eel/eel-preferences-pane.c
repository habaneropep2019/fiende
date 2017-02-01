/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-preferences-pane.h - A preferences pane is a widget that manages
                            a vertical arrangement of preference groups.

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
#include "eel-preferences-pane.h"
#include "eel-gtk-macros.h"
#include "eel-gtk-extensions.h"
#include "eel-string.h"
#include "eel-string-list.h"

#include <gtk/gtkhbox.h>

static const guint GROUPS_BOX_TOP_OFFSET = 0;
static const guint IN_BETWEEN_OFFSET = 4;

struct EelPreferencesPaneDetails
{
	GtkWidget *groups_box;
	GList *groups;
	EelStringList *control_preference_list;
};

/* EelPreferencesPaneClass methods */
static void eel_preferences_pane_initialize_class (EelPreferencesPaneClass *preferences_pane_class);
static void eel_preferences_pane_initialize       (EelPreferencesPane      *preferences_pane);

/* GtkObjectClass methods */
static void eel_preferences_pane_destroy          (GtkObject               *object);

EEL_DEFINE_CLASS_BOILERPLATE (EelPreferencesPane, eel_preferences_pane, GTK_TYPE_VBOX)

/*
 * EelPreferencesPaneClass methods
 */
static void
eel_preferences_pane_initialize_class (EelPreferencesPaneClass *preferences_pane_class)
{
	GtkObjectClass *object_class;
	
	object_class = GTK_OBJECT_CLASS (preferences_pane_class);
	
	/* GtkObjectClass */
	object_class->destroy = eel_preferences_pane_destroy;
}

static void
preferences_pane_update_and_resize_callback (gpointer callback_data)
{
	g_return_if_fail (EEL_IS_PREFERENCES_PANE (callback_data));

	eel_preferences_pane_update (EEL_PREFERENCES_PANE (callback_data));

	gtk_widget_queue_resize (GTK_WIDGET (callback_data));
}

static void
eel_preferences_pane_initialize (EelPreferencesPane *pane)
{
	pane->details = g_new0 (EelPreferencesPaneDetails, 1);
	
 	eel_preferences_add_callback_while_alive ("user_level",
						  preferences_pane_update_and_resize_callback,
						  pane,
						  GTK_OBJECT (pane));
}

/* GtkObjectClass methods */
static void
eel_preferences_pane_destroy (GtkObject* object)
{
	EelPreferencesPane *pane;
	
	g_return_if_fail (EEL_IS_PREFERENCES_PANE (object));
	
	pane = EEL_PREFERENCES_PANE (object);

	g_list_free (pane->details->groups);
	eel_string_list_free (pane->details->control_preference_list);
	g_free (pane->details);

	/* Chain destroy */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/*
 * EelPreferencesPane public methods
 */
GtkWidget *
eel_preferences_pane_new (void)
{
	EelPreferencesPane *pane;

	pane = EEL_PREFERENCES_PANE
		(gtk_widget_new (eel_preferences_pane_get_type (), NULL));

	/* Groups box */
	pane->details->groups_box = gtk_vbox_new (FALSE, 0);

	/* Add groups box to ourselves */
	gtk_box_pack_start (GTK_BOX (pane),
			    pane->details->groups_box,
			    FALSE,
			    FALSE,
			    GROUPS_BOX_TOP_OFFSET);

	gtk_widget_show (pane->details->groups_box);
	gtk_widget_show (GTK_WIDGET (pane));

	return GTK_WIDGET (pane);
}

GtkWidget *
eel_preferences_pane_add_group (EelPreferencesPane *pane,
				const char *group_title)
{
	GtkWidget *group;
	
	g_return_val_if_fail (EEL_IS_PREFERENCES_PANE (pane), NULL);
	g_return_val_if_fail (group_title != NULL, NULL);

	group = eel_preferences_group_new (group_title);

	pane->details->groups = g_list_append (pane->details->groups,
							   group);

	gtk_box_pack_start (GTK_BOX (pane->details->groups_box),
			    group,
			    TRUE,
			    TRUE,
			    IN_BETWEEN_OFFSET);

	gtk_widget_show (group);

	return group;
}

static int
preferences_pane_get_max_caption_width (const EelPreferencesPane *pane,
					int column)
{
	EelPreferencesGroup *group;
	GList *node;
	int max_caption_width = 0;

	g_return_val_if_fail (EEL_IS_PREFERENCES_PANE (pane), 0);
	g_return_val_if_fail (column >= 0, 0);
	g_return_val_if_fail (column <= 1, 0);

	for (node = pane->details->groups; node != NULL; node = node->next) {
		g_assert (EEL_IS_PREFERENCES_GROUP (node->data));
		group = EEL_PREFERENCES_GROUP (node->data);

		if (GTK_WIDGET_VISIBLE (group)) {
			max_caption_width = MAX (max_caption_width,
						 eel_preferences_group_get_max_caption_width (group, column));
		}
	}

	return max_caption_width;
}

void
eel_preferences_pane_update (EelPreferencesPane *pane)
{
	GList *node;
	int max_caption_widths[2];
	EelPreferencesGroup *group;

	g_return_if_fail (EEL_IS_PREFERENCES_PANE (pane));

	for (node = pane->details->groups; node != NULL; node = node->next) {
		g_assert (EEL_IS_PREFERENCES_GROUP (node->data));
		group = EEL_PREFERENCES_GROUP (node->data);
		eel_preferences_group_update (group);
		eel_gtk_widget_set_shown (GTK_WIDGET (group),
					  eel_preferences_group_get_num_visible_items (group) > 0);
	}

	max_caption_widths[0] = preferences_pane_get_max_caption_width (pane, 0);
	max_caption_widths[1] = preferences_pane_get_max_caption_width (pane, 1);

	for (node = pane->details->groups; node != NULL; node = node->next) {
		g_assert (EEL_IS_PREFERENCES_GROUP (node->data));
		group = EEL_PREFERENCES_GROUP (node->data);

		if (GTK_WIDGET_VISIBLE (group)) {
			if (max_caption_widths[0] > 0) {
				eel_preferences_group_align_captions (group,
								      max_caption_widths[0],
								      0);
			}

			if (max_caption_widths[1] > 0) {
				eel_preferences_group_align_captions (group,
								      max_caption_widths[1],
								      1);
			}
		}
	}
}

guint
eel_preferences_pane_get_num_visible_groups (const EelPreferencesPane *pane)
{
	guint n = 0;
	GList *node;

	g_return_val_if_fail (EEL_IS_PREFERENCES_PANE (pane), 0);

	for (node = pane->details->groups; node != NULL; node = node->next) {
		EelPreferencesGroup *group = EEL_PREFERENCES_GROUP (node->data);

		if (GTK_WIDGET_VISIBLE (group)) {
			n++;
		}
	}

	return n;
}

guint
eel_preferences_pane_get_num_groups (const EelPreferencesPane *pane)
{
	g_return_val_if_fail (EEL_IS_PREFERENCES_PANE (pane), 0);

	return g_list_length (pane->details->groups);
}

GtkWidget *
eel_preferences_pane_find_group (const EelPreferencesPane *pane,
				 const char *group_title)
{
	GList *node;
	char *title;

	g_return_val_if_fail (EEL_IS_PREFERENCES_PANE (pane), 0);

	for (node = pane->details->groups; node != NULL; node = node->next) {
		g_assert (EEL_IS_PREFERENCES_GROUP (node->data));

		title = eel_preferences_group_get_title_label (EEL_PREFERENCES_GROUP (node->data));
		if (eel_str_is_equal (title, group_title)) {
			g_free (title);
			return node->data;
		}

		g_free (title);
	}
	
	return NULL;
}

void
eel_preferences_pane_add_control_preference (EelPreferencesPane *pane,
					     const char *control_preference_name)
{
	g_return_if_fail (EEL_IS_PREFERENCES_PANE (pane));
	g_return_if_fail (control_preference_name != NULL);

	if (eel_string_list_contains (pane->details->control_preference_list,
				      control_preference_name)) {
		return;
	}

	if (pane->details->control_preference_list == NULL) {
		pane->details->control_preference_list = eel_string_list_new (TRUE);
	}

	eel_string_list_insert (pane->details->control_preference_list,
				control_preference_name);

 	eel_preferences_add_callback_while_alive (control_preference_name,
						  preferences_pane_update_and_resize_callback,
						  pane,
						  GTK_OBJECT (pane));
}

void
eel_preferences_pane_for_each_group (const EelPreferencesPane *pane,
				     EelPreferencesPaneForEachCallback callback,
				     gpointer callback_data)
{
	GList *node;

	g_return_if_fail (EEL_IS_PREFERENCES_PANE (pane));
	g_return_if_fail (callback != NULL);

	for (node = pane->details->groups; node != NULL; node = node->next) {
		g_assert (EEL_IS_PREFERENCES_GROUP (node->data));
		(* callback) (EEL_PREFERENCES_GROUP (node->data), callback_data);
	}
}
