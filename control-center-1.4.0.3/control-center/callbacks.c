/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#include <config.h>
#include "callbacks.h"
#include "tree.h"
#include "corba-glue.h"
#include "capplet-manager.h"
extern GtkWidget *main_window;
extern GtkWidget *exit_dialog;
extern GtkWidget *multiview;
extern GtkWidget *create_exit_dialog();
extern GList *capplet_list;
static GtkWidget *about = NULL;
extern gint destroying;
extern CORBA_Environment ev;

/* this is meant to be called with a foreach to make a list of all modified nodes. */
void
create_templist (node_data *data, GList **list)
{
        if (data->modified == TRUE)
                *list = (g_list_prepend (*list, data));
}
int
exit_callback(GtkWidget *widget, gpointer data) 
{
        GList* templist = NULL;
        
        if (exit_dialog != NULL) {
		destroying = TRUE;
		gnome_dialog_close (GNOME_DIALOG (exit_dialog));
	}

        g_list_foreach (capplet_list, (GFunc)create_templist, &templist);

        if (!templist) {
                gtk_widget_destroy (main_window);
                control_center_corba_gtk_main_quit();
                return FALSE;
        }                
        exit_dialog = create_exit_dialog(templist);
        gtk_widget_show(GTK_WIDGET(exit_dialog));
				return TRUE;
}
void
exit_dialog_ok_callback(GtkWidget *widget, gpointer data)
{
        revert_all ();
        destroying = TRUE;
        gtk_widget_destroy (multiview);
        control_center_corba_gtk_main_quit();
}
void
exit_dialog_cancel_callback(GtkWidget *widget, gpointer data)
{
        gnome_dialog_close (GNOME_DIALOG (exit_dialog));
        exit_dialog = NULL;
}
void
exit_dialog_close_callback(GtkWidget *widget, gpointer data)
{
        exit_dialog = NULL;
}
void
help_callback (GtkWidget *widget, gpointer data)
{
        GnomeHelpMenuEntry entry;

        entry.name = "control-center";
        entry.path = "index.html";
        gnome_help_display(0, &entry);
}

void
item_help_callback (GtkWidget *widget, gpointer data)
{
        node_data *nd = (node_data *) data;

        if (data == NULL)
                return;

        GNOME_capplet_help (nd->capplet,nd->id, &ev);

}
static void
delete_callback (GtkWidget *widget, gpointer data)
{
        about = NULL;
}
void
about_callback (GtkWidget *widget, gpointer data)
{
	gchar *authors[] = {
                "Jonathan Blandford <jrb@redhat.com>",
		NULL
	};

        if (about == NULL) {
                about = gnome_about_new(_("GNOME Control Center"), VERSION,
                                        "Copyright (C) 1999 Red Hat Software Inc.",
                                        (const gchar **) authors,
                                        _("Desktop Properties manager."),
                                        NULL);
                gtk_signal_connect (GTK_OBJECT (about), "destroy", delete_callback, NULL);
        }
        gtk_widget_show(about);
}
void
exit_row_callback(GtkWidget *widget, gint row, gint column, GdkEventButton * event, gpointer nulldata)
{
        node_data *data;

        
        if (event && event->type == GDK_2BUTTON_PRESS) {
                data = (node_data *) gtk_clist_get_row_data (GTK_CLIST (widget),row);
                gnome_dialog_close (GNOME_DIALOG (exit_dialog));
                exit_dialog = NULL;
                launch_capplet (data, TRUE);
        }
}
