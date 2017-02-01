/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-preferences-group.c - A preferences group is a widget that manages
                             an vertical arrangement of related preference
                             item widgets.   These can be arranged in 1 or
                             2 columns.

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
#include "eel-preferences-group.h"

#include "eel-gtk-extensions.h"
#include "eel-gtk-macros.h"
#include <gnome.h>

struct EelPreferencesGroupDetails
{
	GtkWidget *main_box;
	GtkWidget *columns[2];
	GList *items[2];
};

/* EelPreferencesGroupClass methods */
static void eel_preferences_group_initialize_class (EelPreferencesGroupClass *klass);
static void eel_preferences_group_initialize       (EelPreferencesGroup      *preferences_group);

/* GtkObjectClass methods */
static void eel_preferences_group_destroy          (GtkObject                     *object);

EEL_DEFINE_CLASS_BOILERPLATE (EelPreferencesGroup,
			      eel_preferences_group,
			      GTK_TYPE_FRAME);

/*
 * EelPreferencesGroupClass methods
 */
static void
eel_preferences_group_initialize_class (EelPreferencesGroupClass *preferences_group_class)
{
	GtkObjectClass *object_class;
	
	object_class = GTK_OBJECT_CLASS (preferences_group_class);

	/* GtkObjectClass */
	object_class->destroy = eel_preferences_group_destroy;
}

static void
eel_preferences_group_initialize (EelPreferencesGroup *group)
{
	group->details = g_new0 (EelPreferencesGroupDetails, 1);
}

/*
 * GtkObjectClass methods
 */
static void
eel_preferences_group_destroy (GtkObject *object)
{
	EelPreferencesGroup *group;
	
	g_return_if_fail (EEL_IS_PREFERENCES_GROUP (object));
	
	group = EEL_PREFERENCES_GROUP (object);

	g_list_free (group->details->items[0]);
	g_list_free (group->details->items[1]);
	g_free (group->details);

	/* Chain destroy */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/*
 * EelPreferencesGroup public methods
 */
GtkWidget *
eel_preferences_group_new (const gchar *title)
{
	EelPreferencesGroup *group;
	
	g_return_val_if_fail (title != NULL, NULL);

	group = EEL_PREFERENCES_GROUP
		(gtk_widget_new (eel_preferences_group_get_type (), NULL));

	/* Ourselves */
	gtk_frame_set_shadow_type (GTK_FRAME (group), GTK_SHADOW_ETCHED_IN);

	gtk_frame_set_label (GTK_FRAME (group), title);

	/* Main box */
	group->details->main_box = gtk_hbox_new (FALSE, 20);
	gtk_container_add (GTK_CONTAINER (group), group->details->main_box);

	/* Column 1 */
	group->details->columns[0] = gtk_vbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (group->details->main_box),
			    group->details->columns[0],
			    TRUE,
			    TRUE,
			    0);

	/* Column 2 */
	group->details->columns[1] = gtk_vbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (group->details->main_box),
			    group->details->columns[1],
			    TRUE,
			    TRUE,
			    0);

	gtk_container_set_border_width (GTK_CONTAINER (group->details->columns[0]), 6);

	gtk_widget_show (group->details->columns[0]);
	gtk_widget_show (group->details->columns[1]);
	gtk_widget_show (group->details->main_box);
	
	return GTK_WIDGET (group);
}

GtkWidget *
eel_preferences_group_add_item (EelPreferencesGroup *group,
				const char *preference_name,
				EelPreferencesItemType item_type,
				int column)
{
	GtkWidget *item;

	g_return_val_if_fail (EEL_IS_PREFERENCES_GROUP (group), NULL);
	g_return_val_if_fail (preference_name != NULL, NULL);
	g_return_val_if_fail (column >= 0, NULL);
	g_return_val_if_fail (column <= 1, NULL);

	item = eel_preferences_item_new (preference_name, item_type);

	group->details->items[column] = g_list_append (group->details->items[column],
						       item);

	gtk_box_pack_start (GTK_BOX (group->details->columns[column]),
			    item,
 			    FALSE,
 			    FALSE,
			    0);

	gtk_widget_show (item);

	return item;
}

GtkWidget *
eel_preferences_group_add_custom_item (EelPreferencesGroup *group,
				       const char *preference_name,
				       GtkWidget *child,
				       const char *signal_name,
				       int column)
{
	GtkWidget *item;

	g_return_val_if_fail (EEL_IS_PREFERENCES_GROUP (group), NULL);
	g_return_val_if_fail (preference_name != NULL, NULL);
	g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
	g_return_val_if_fail (signal_name != NULL, NULL);
	g_return_val_if_fail (column >= 0, NULL);
	g_return_val_if_fail (column <= 1, NULL);
	
	item = eel_preferences_item_new_custom (preference_name,
						child,
						signal_name);

	group->details->items[column] = g_list_append (group->details->items[column],
						       item);

	gtk_box_pack_start (GTK_BOX (group->details->columns[column]),
			    item,
 			    FALSE,
 			    FALSE,
			    0);

	gtk_widget_show (item);

	return item;
}

void
eel_preferences_group_update (EelPreferencesGroup *group)
{
	GList *node;
	
	g_return_if_fail (EEL_IS_PREFERENCES_GROUP (group));

	for (node = group->details->items[0]; node != NULL; node = node->next) {
		g_assert (EEL_IS_PREFERENCES_ITEM (node->data));
		eel_preferences_item_update_showing (EEL_PREFERENCES_ITEM (node->data));
	}

	for (node = group->details->items[1]; node != NULL; node = node->next) {
		g_assert (EEL_IS_PREFERENCES_ITEM (node->data));
		eel_preferences_item_update_showing (EEL_PREFERENCES_ITEM (node->data));
	}
}

guint
eel_preferences_group_get_num_visible_items (const EelPreferencesGroup *group)
{
	guint n = 0;
	GList *node;

	g_return_val_if_fail (EEL_IS_PREFERENCES_GROUP (group), 0);

	for (node = group->details->items[0]; node != NULL; node = node->next) {
		if (eel_preferences_item_is_showing (EEL_PREFERENCES_ITEM (node->data))) {
			n++;
		}
	}

	for (node = group->details->items[1]; node != NULL; node = node->next) {
		if (eel_preferences_item_is_showing (EEL_PREFERENCES_ITEM (node->data))) {
			n++;
		}
	}
	
	return n;
}

char *
eel_preferences_group_get_title_label (const EelPreferencesGroup *group)
{
	g_return_val_if_fail (EEL_IS_PREFERENCES_GROUP (group), NULL);

	return g_strdup (GTK_FRAME (group)->label);
}

void
eel_preferences_group_set_title_label (EelPreferencesGroup *group,
				       const char *title_label)
{
	g_return_if_fail (EEL_IS_PREFERENCES_GROUP (group));
	g_return_if_fail (title_label != NULL);

	gtk_frame_set_label (GTK_FRAME (group), title_label);
}

int
eel_preferences_group_get_max_caption_width (const EelPreferencesGroup *group,
					     int column)
{
	GList *node;
	EelPreferencesItem *item;
	int max_caption_width = 0;
	
	g_return_val_if_fail (EEL_IS_PREFERENCES_GROUP (group), 0);
	g_return_val_if_fail (column >= 0, 0);
	g_return_val_if_fail (column <= 1, 0);

	for (node = group->details->items[column]; node != NULL; node = node->next) {
		g_assert (EEL_IS_PREFERENCES_ITEM (node->data));
		item = EEL_PREFERENCES_ITEM (node->data);
		
		if (eel_preferences_item_is_showing (item)
		    && eel_preferences_item_child_is_caption (item)) {
			max_caption_width = MAX (max_caption_width,
						 eel_preferences_item_get_child_width (item));
		}
	}

	return max_caption_width;
}

void
eel_preferences_group_align_captions (EelPreferencesGroup *group,
				      int max_caption_width,
				      int column)
{
	GList *node;
	EelPreferencesItem *item;
	int width;

	g_return_if_fail (EEL_IS_PREFERENCES_GROUP (group));
	g_return_if_fail (max_caption_width > 0);
	g_return_if_fail (column >= 0);
	g_return_if_fail (column <= 1);

	/* Set the spacing on all the captions */
	for (node = group->details->items[column]; node != NULL; node = node->next) {
		g_assert (EEL_IS_PREFERENCES_ITEM (node->data));
		item = EEL_PREFERENCES_ITEM (node->data);

		if (eel_preferences_item_is_showing (item)		
		    && eel_preferences_item_child_is_caption (item)) {
			width = eel_preferences_item_get_child_width (item);
			g_assert (width <= max_caption_width);
			eel_preferences_item_set_caption_extra_spacing (item, max_caption_width - width);
		}
	}
}

void
eel_preferences_group_for_each_item (const EelPreferencesGroup *group,
				     EelPreferencesGroupForEachCallback callback,
				     gpointer callback_data)
{
	GList *node;

	g_return_if_fail (EEL_IS_PREFERENCES_GROUP (group));
	g_return_if_fail (callback != NULL);

	for (node = group->details->items[0]; node != NULL; node = node->next) {
		g_assert (EEL_IS_PREFERENCES_ITEM (node->data));
		(* callback) (EEL_PREFERENCES_ITEM (node->data), callback_data);
	}

	for (node = group->details->items[1]; node != NULL; node = node->next) {
		g_assert (EEL_IS_PREFERENCES_ITEM (node->data));
		(* callback) (EEL_PREFERENCES_ITEM (node->data), callback_data);
	}
}
