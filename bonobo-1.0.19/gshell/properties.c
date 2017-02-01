/*
 * sample-control-container.c
 * 
 * Authors:
 *   Nat Friedman  (nat@helixcode.com)
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */

#include <config.h>
#include <gnome.h>

#if USING_OAF
#include <liboaf/liboaf.h>
#else
#include <libgnorba/gnorba.h>
#endif

#include <bonobo.h>

#include "properties.h"

static gboolean
populate_property_list (BonoboControlFrame *cf, GtkCList *clist)
{
	GList *property_list, *l;
	Bonobo_PropertyBag pb;

	pb = bonobo_control_frame_get_control_property_bag (cf, NULL);

	/* Get the list of properties. */
	if (pb == CORBA_OBJECT_NIL) 
		return FALSE;

	property_list = bonobo_property_bag_client_get_property_names (pb, NULL);
	for (l = property_list; l != NULL; l = l->next) {
		char *row_array[2];
		CORBA_TypeCode tc;
		gchar *name = l->data;

		row_array [0] = name;

		tc = bonobo_property_bag_client_get_property_type (pb, name, NULL);
		switch (tc->kind) {

		case CORBA_tk_boolean:
			row_array [1] = g_strdup (
				bonobo_property_bag_client_get_value_gboolean (pb, name, NULL) ? "TRUE" : "FALSE");
			break;

		case CORBA_tk_string:
			row_array [1] = g_strdup (bonobo_property_bag_client_get_value_string (pb, name, NULL));
			break;

		case CORBA_tk_long:
			row_array [1] = g_strdup_printf ("%ld", bonobo_property_bag_client_get_value_glong (pb, name, NULL));
			break;

		case CORBA_tk_float:
			row_array [1] = g_strdup_printf ("%f", bonobo_property_bag_client_get_value_gfloat (pb, name, NULL));
			break;

		case CORBA_tk_double:
			row_array [1] = g_strdup_printf ("%g", bonobo_property_bag_client_get_value_gdouble (pb, name, NULL));
			break;

		default:
			row_array [1] = g_strdup ("Unhandled Property Type");
			break;
		}

		gtk_clist_append (clist, row_array);
	}
	g_list_free (property_list);

	return TRUE;
}

static void
edit_property (GtkCList *clist, GdkEventButton *event, BonoboControlFrame *cf)
{
	gchar *prop;
	gint row, col;
	GList *l;
	CORBA_TypeCode tc;
	Bonobo_PropertyBag pb;

	pb = bonobo_control_frame_get_control_property_bag (cf, NULL);
	g_return_if_fail (pb != CORBA_OBJECT_NIL);

	if (event->button == 3) {
		gtk_clist_get_selection_info (clist, event->x, event->y,
		                              &row, &col);
		if (row < 0) return;
		l = bonobo_property_bag_client_get_property_names (pb, NULL);
		if (row > g_list_length (l) - 1) return;

		/* Get the value of the property they clicked on. */
		prop = g_list_nth_data (l, row);
		/* Change it appropriately. */
		tc = bonobo_property_bag_client_get_property_type (pb, prop, NULL);

		switch (tc->kind) {

		case CORBA_tk_boolean:
			bonobo_property_bag_client_set_value_gboolean (
				pb, prop, !bonobo_property_bag_client_get_value_gboolean (pb, prop, NULL), NULL);
			break;

		default:
			g_warning ("Cannot set_value this type of property yet, sorry.");
			break;
			
		}

		g_list_free (l);

		/* Redraw the property list. */
		gtk_clist_clear (clist);
		populate_property_list (cf, clist);
	}

}

static GtkWidget *
create_proplist (BonoboControlFrame *cf)
{
	gchar *clist_titles[] = {"Property Name", "Value"};
	GtkWidget *clist;

	/* Put the property CList on the bottom. */
	clist = gtk_clist_new_with_titles (2, clist_titles);
	gtk_signal_connect (GTK_OBJECT (clist), "button_press_event",
			    GTK_SIGNAL_FUNC (edit_property), cf);
 
	populate_property_list (cf, GTK_CLIST (clist));

	return clist;
}

void
show_property_dialog (BonoboControlFrame *cf)
{
    GtkWidget *dialog;
    GtkWidget *clist;

    clist = create_proplist (cf);

    if (!clist || !cf) {
	    dialog = gnome_message_box_new (
		    _("Component has no editable properties"),
		    GNOME_MESSAGE_BOX_INFO, GNOME_STOCK_BUTTON_OK, NULL);
    } else {
	    dialog = gnome_dialog_new ("Properties", GNOME_STOCK_BUTTON_OK, NULL);
	    
	    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox),
				clist, TRUE, TRUE, 0);
	    
	    gtk_widget_show (clist);
    }

    gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
}

	
