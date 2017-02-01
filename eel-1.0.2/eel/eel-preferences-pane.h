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

#ifndef EEL_PREFERENCES_PANE_H
#define EEL_PREFERENCES_PANE_H

#include <libgnomeui/gnome-dialog.h>
#include <gtk/gtkvbox.h>
#include <eel/eel-preferences-group.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_PREFERENCES_PANE            (eel_preferences_pane_get_type ())
#define EEL_PREFERENCES_PANE(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_PREFERENCES_PANE, EelPreferencesPane))
#define EEL_PREFERENCES_PANE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_PREFERENCES_PANE, EelPreferencesPaneClass))
#define EEL_IS_PREFERENCES_PANE(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_PREFERENCES_PANE))
#define EEL_IS_PREFERENCES_PANE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_PREFERENCES_PANE))

typedef struct EelPreferencesPane	     EelPreferencesPane;
typedef struct EelPreferencesPaneClass	     EelPreferencesPaneClass;
typedef struct EelPreferencesPaneDetails     EelPreferencesPaneDetails;

struct EelPreferencesPane
{
	/* Super Class */
	GtkVBox vbox;

	/* Private stuff */
	EelPreferencesPaneDetails *details;
};

struct EelPreferencesPaneClass
{
	GtkVBoxClass parent_class;
};

/* A callback for eel_preferences_pane_for_each_pane() */
typedef void (* EelPreferencesPaneForEachCallback) (EelPreferencesGroup *group,
						    gpointer callback_data);

GtkType    eel_preferences_pane_get_type               (void);
GtkWidget* eel_preferences_pane_new                    (void);
GtkWidget *eel_preferences_pane_add_group              (EelPreferencesPane                *preferences_pane,
							const char                        *group_title);
void       eel_preferences_pane_update                 (EelPreferencesPane                *preferences_pane);
guint      eel_preferences_pane_get_num_groups         (const EelPreferencesPane          *pane);
guint      eel_preferences_pane_get_num_visible_groups (const EelPreferencesPane          *pane);
GtkWidget* eel_preferences_pane_find_group             (const EelPreferencesPane          *preferences_pane,
							const char                        *group_title);
void       eel_preferences_pane_add_control_preference (EelPreferencesPane                *preferences_pane,
							const char                        *control_preference_name);
void       eel_preferences_pane_for_each_group         (const EelPreferencesPane          *preferences_pane,
							EelPreferencesPaneForEachCallback  callback,
							gpointer                           callback_data);

END_GNOME_DECLS

#endif /* EEL_PREFERENCES_PANE_H */
