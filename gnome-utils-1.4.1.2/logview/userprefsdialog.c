/*  ----------------------------------------------------------------------

    Copyright (C) 1998  Cesar Miquel  (miquel@df.uba.ar)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    ---------------------------------------------------------------------- */


#include <config.h>
#include <gnome.h>

#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "logview.h"

#define USERPREFSDIALOG_WIDTH            500
#define USERPREFSDIALOG_HEIGHT           300

/*
 *  --------------------------
 *  Local and global variables
 *  --------------------------
 */

extern ConfigData *cfg;
extern UserPrefsStruct *user_prefs;
extern GtkWidget *app;

/* Struct to hold function data */
typedef struct 
{
   GtkWidget *userprefs_property;
   GtkWidget *frame;
   GtkWidget *table;
   GtkWidget *columns_tab_label;
   GtkWidget *hostname_width_label;
   GtkWidget *hostname_width_spinner;
   GtkAdjustment *hostname_width_spinner_adjust;
} UserPrefsDialogStruct;

UserPrefsDialogStruct upds;

void save_property_changes(GnomePropertyBox *prop, gint page, gpointer p);
void UserPrefsDialog (GtkWidget * widget, gpointer user_data);

void UserPrefsDialog (GtkWidget * widget, gpointer user_data)
{
   memset((void *)&upds, 0, sizeof(upds));

   upds.table = gtk_table_new(3, 2, FALSE);
   upds.hostname_width_label = gtk_label_new(_("hostname column width ( 0 to hide this column ): ")); 
   upds.columns_tab_label = gtk_label_new(_("Columns")); 
   upds.hostname_width_spinner_adjust = (GtkAdjustment *)gtk_adjustment_new(0, 0.0, 100.0, 1.0, 5.0, 5.0); 
   upds.hostname_width_spinner = gtk_spin_button_new(upds.hostname_width_spinner_adjust, 1.0, 0); 
   upds.userprefs_property  = gnome_property_box_new();

   gtk_table_attach_defaults(GTK_TABLE(upds.table), upds.hostname_width_label, 0, 1, 0, 1);
   gtk_table_attach(GTK_TABLE(upds.table), upds.hostname_width_spinner, 1, 2, 0, 1, GTK_EXPAND, GTK_EXPAND, 0, 0);

   gtk_container_set_border_width (GTK_CONTAINER (upds.table), 2);

   /* Create frame for main view */
   upds.frame = gtk_frame_new (NULL);
   gtk_container_set_border_width (GTK_CONTAINER (upds.frame), 3);
   gtk_widget_set_style (upds.frame, cfg->main_style);
   gtk_frame_set_shadow_type (GTK_FRAME (upds.frame), GTK_SHADOW_ETCHED_IN);
   gtk_container_add(GTK_CONTAINER(upds.frame), upds.table);

   gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(upds.hostname_width_spinner), TRUE);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(upds.hostname_width_spinner), (gfloat)user_prefs->hostname_column_width);
   gnome_property_box_append_page((GnomePropertyBox *)upds.userprefs_property, upds.frame, upds.columns_tab_label);

   gtk_signal_connect (GTK_OBJECT (upds.userprefs_property), "apply",
		       (GtkSignalFunc) save_property_changes,
		       &upds);

   gtk_signal_connect_object(GTK_OBJECT(upds.hostname_width_spinner), "changed",
		       (GtkSignalFunc) gnome_property_box_changed,
		       GTK_OBJECT(upds.userprefs_property));

   gtk_widget_show(upds.hostname_width_label);
   gtk_widget_show(upds.hostname_width_spinner);
   gtk_widget_show(upds.table);
   gtk_widget_show(upds.frame);
   gtk_widget_show(upds.userprefs_property);
}

void save_property_changes(GnomePropertyBox *prop, gint page, gpointer p)
{
	UserPrefsDialogStruct *u = (UserPrefsDialogStruct *)p;

	switch(page)
	{
	case 0:
		user_prefs->hostname_column_width =
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(u->hostname_width_spinner));
		gtk_widget_queue_draw (app);
		break;
	default: ;
	}
}
