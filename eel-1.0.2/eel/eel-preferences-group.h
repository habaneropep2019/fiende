/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-preferences-group.h - A preferences group is a widget that manages
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

#ifndef EEL_PREFERENCES_GROUP_H
#define EEL_PREFERENCES_GROUP_H

#include <gtk/gtkframe.h>

#include <eel/eel-preferences.h>
#include <eel/eel-preferences-item.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_PREFERENCES_GROUP            (eel_preferences_group_get_type ())
#define EEL_PREFERENCES_GROUP(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_PREFERENCES_GROUP, EelPreferencesGroup))
#define EEL_PREFERENCES_GROUP_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_PREFERENCES_GROUP, EelPreferencesGroupClass))
#define EEL_IS_PREFERENCES_GROUP(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_PREFERENCES_GROUP))
#define EEL_IS_PREFERENCES_GROUP_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_PREFERENCES_GROUP))

typedef struct EelPreferencesGroup	      EelPreferencesGroup;
typedef struct EelPreferencesGroupClass	      EelPreferencesGroupClass;
typedef struct EelPreferencesGroupDetails     EelPreferencesGroupDetails;

struct EelPreferencesGroup
{
	/* Super Class */
	GtkFrame frame;
	
	/* Private stuff */
	EelPreferencesGroupDetails *details;
};

struct EelPreferencesGroupClass
{
	GtkFrameClass parent_class;
};

/* A callback for eel_preferences_group_for_each_group() */
typedef void (* EelPreferencesGroupForEachCallback) (EelPreferencesItem *item,
						     gpointer callback_data);

GtkType    eel_preferences_group_get_type              (void);
GtkWidget* eel_preferences_group_new                   (const gchar                        *title);
GtkWidget* eel_preferences_group_add_item              (EelPreferencesGroup                *group,
							const char                         *preference_name,
							EelPreferencesItemType              item_type,
							int                                 column);
GtkWidget* eel_preferences_group_add_custom_item       (EelPreferencesGroup                *group,
							const char                         *preference_name,
							GtkWidget                          *child,
							const char                         *signal_name,
							int                                 column);
void       eel_preferences_group_update                (EelPreferencesGroup                *group);
guint      eel_preferences_group_get_num_visible_items (const EelPreferencesGroup          *group);
char *     eel_preferences_group_get_title_label       (const EelPreferencesGroup          *group);
void       eel_preferences_group_set_title_label       (EelPreferencesGroup                *group,
							const char                         *title_label);
int        eel_preferences_group_get_max_caption_width (const EelPreferencesGroup          *group,
							int                                 column);
void       eel_preferences_group_align_captions        (EelPreferencesGroup                *group,
							int                                 max_caption_width,
							int                                 column);
void       eel_preferences_group_for_each_item         (const EelPreferencesGroup          *preferences_group,
							EelPreferencesGroupForEachCallback  callback,
							gpointer                            callback_data);

END_GNOME_DECLS

#endif /* EEL_PREFERENCES_GROUP_H */


