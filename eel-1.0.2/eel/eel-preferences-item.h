/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-preferences-item.h - A preferences item is a widget that represents
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

#ifndef EEL_PREFERENCES_ITEM_H
#define EEL_PREFERENCES_ITEM_H

#include <gtk/gtkvbox.h>
#include <eel/eel-preferences.h>
#include <eel/eel-enumeration.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_PREFERENCES_ITEM            (eel_preferences_item_get_type ())
#define EEL_PREFERENCES_ITEM(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_PREFERENCES_ITEM, EelPreferencesItem))
#define EEL_PREFERENCES_ITEM_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_PREFERENCES_ITEM, EelPreferencesItemClass))
#define EEL_IS_PREFERENCES_ITEM(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_PREFERENCES_ITEM))
#define EEL_IS_PREFERENCES_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_PREFERENCES_ITEM))

typedef struct EelPreferencesItem	     EelPreferencesItem;
typedef struct EelPreferencesItemClass	     EelPreferencesItemClass;
typedef struct EelPreferencesItemDetails     EelPreferencesItemDetails;

struct EelPreferencesItem
{
	/* Super Class */
	GtkVBox vbox;

	/* Private stuff */
	EelPreferencesItemDetails *details;
};

struct EelPreferencesItemClass
{
	GtkVBoxClass vbox_class;
};

/*
 * EelPreferencesItemType:
 *
 * The types of supported preferences that also have a corresponding ui in the 
 * preferences dialog.  Note that this is different than EelPreferencesType
 * because it is possible to have a prefernce that is not exposed in the ui.
 */
typedef enum
{
	EEL_PREFERENCE_ITEM_BOOLEAN,
	EEL_PREFERENCE_ITEM_CUSTOM,
	EEL_PREFERENCE_ITEM_EDITABLE_INTEGER,
	EEL_PREFERENCE_ITEM_EDITABLE_STRING,
	EEL_PREFERENCE_ITEM_ENUMERATION_HORIZONTAL_RADIO,
	EEL_PREFERENCE_ITEM_ENUMERATION_LIST_HORIZONTAL,
	EEL_PREFERENCE_ITEM_ENUMERATION_LIST_VERTICAL,
	EEL_PREFERENCE_ITEM_ENUMERATION_MENU,
	EEL_PREFERENCE_ITEM_ENUMERATION_VERTICAL_RADIO,
	EEL_PREFERENCE_ITEM_FONT,
	EEL_PREFERENCE_ITEM_PADDING,
	EEL_PREFERENCE_ITEM_SMOOTH_FONT
} EelPreferencesItemType;

typedef enum
{
	EEL_PREFERENCE_ITEM_SHOW,
	EEL_PREFERENCE_ITEM_HIDE
} EelPreferencesItemControlAction;

GtkType    eel_preferences_item_get_type                               (void);
GtkWidget* eel_preferences_item_new                                    (const char                      *preference_name,
									EelPreferencesItemType           item_type);
GtkWidget* eel_preferences_item_new_custom                             (const char                      *preference_name,
									GtkWidget                       *child,
									const char                      *signal_name);
char *     eel_preferences_item_get_name                               (const EelPreferencesItem        *preferences_item);
void       eel_preferences_item_set_control_preference                 (EelPreferencesItem              *preferences_item,
									const char                      *control_preference_name);
void       eel_preferences_item_set_control_action                     (EelPreferencesItem              *preferences_item,
									EelPreferencesItemControlAction  control_action);
gboolean   eel_preferences_item_child_is_caption                       (const EelPreferencesItem        *preferences_item);
int        eel_preferences_item_get_child_width                        (const EelPreferencesItem        *item);
void       eel_preferences_item_set_caption_extra_spacing              (EelPreferencesItem              *item,
									int                              extra_spacing);
void       eel_preferences_item_update_showing                         (EelPreferencesItem              *item);
gboolean   eel_preferences_item_is_showing                             (const EelPreferencesItem        *item);
void       eel_preferences_item_enumeration_list_set_unique_exceptions (EelPreferencesItem              *item,
									const char                      *exceptions,
									const char                      *exceptions_delimeter);
void       eel_preferences_item_set_description                        (EelPreferencesItem              *preferences_item,
									const char                      *description);

END_GNOME_DECLS

#endif /* EEL_PREFERENCES_ITEM_H */


