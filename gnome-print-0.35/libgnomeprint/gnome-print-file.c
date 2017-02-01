/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-print-file.c: A file selector to use in print to file 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors :
 *   Chema Celorio (chema@celorio.com)
 *
 */



#include "config.h"
#include <gnome.h>

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-print-file.h>
#include <libgnomeprint/gnome-printer-private.h>

static void 
gnome_print_file_destroy_cb (gpointer dummy, gpointer widget)
{
	gtk_widget_destroy (GTK_WIDGET (widget));
	gtk_main_quit ();
}

static void
gnome_print_file_ok_selected (gpointer a, GtkFileSelection *f)
{
	GnomePrinter *printer;

	printer = gtk_object_get_data (GTK_OBJECT (f), "Printer");

	g_return_if_fail (GNOME_IS_PRINTER (printer));

	if (printer->filename)
		g_free (printer->filename);
	
	printer->filename = g_strdup (gtk_file_selection_get_filename (f));

	if (printer->filename[0] == 0) {
		g_free (printer->filename);
		printer->filename = NULL;
		return;
	}
	
	if (g_file_test (printer->filename, G_FILE_TEST_ISDIR)) {
		g_free (printer->filename);
		printer->filename = NULL;
		return;
	}

        if (g_file_exists (printer->filename))
	{
		guchar * msg;
		GtkWidget *msgbox;
		gint ret;
		msg = g_strdup_printf (_("'%s' is about to be overwritten.\n\n"
					 "Do you want to continue?"), printer->filename);
		msgbox = gnome_message_box_new (msg,
						GNOME_MESSAGE_BOX_QUESTION,
						GNOME_STOCK_BUTTON_YES,
						GNOME_STOCK_BUTTON_NO,
						GNOME_STOCK_BUTTON_CANCEL,
						NULL);
		gnome_dialog_set_default (GNOME_DIALOG (msgbox), 2);
		ret = gnome_dialog_run_and_close (GNOME_DIALOG (msgbox));
		g_free (msg);
		switch (ret)
		{
		/* Yes selected */
		case 0:
			break;
		case 1:
			return;
		default:
			g_free (printer->filename);
			printer->filename = NULL;
			gnome_print_file_destroy_cb (NULL, GTK_WIDGET (f));
			return;
		}
	}

	gnome_print_file_destroy_cb (NULL, f);
}

static gint
gnome_print_file_dialog_key (GtkFileSelection *fsel, GdkEventKey *event)
{
	if (event->keyval == GDK_Escape) {
		gtk_button_clicked (GTK_BUTTON (fsel->cancel_button));
		return 1;
	}

	return 0;
}


static void 
gnome_print_file_delete_event_cb (gpointer dummy, gpointer dummy2, gpointer widget)
{
	gnome_print_file_destroy_cb (NULL, widget);
}

static GtkWidget *
gnome_print_file_create (GnomePrinter *printer)
{
	GtkWidget *file_selector;

	file_selector = gtk_file_selection_new (NULL);

	gtk_object_set_data (GTK_OBJECT (file_selector), "Printer", printer);
	
	gtk_signal_connect (GTK_OBJECT (file_selector),
			    "delete_event",
			    GTK_SIGNAL_FUNC (gnome_print_file_delete_event_cb),
			    file_selector);
	gtk_signal_connect (GTK_OBJECT (file_selector),
			    "key_press_event",
			    GTK_SIGNAL_FUNC (gnome_print_file_dialog_key),
			    NULL);
	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(file_selector)->cancel_button),
			    "clicked",
			    GTK_SIGNAL_FUNC (gnome_print_file_destroy_cb),
			    file_selector);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION (file_selector)->ok_button),
			    "clicked",
			    GTK_SIGNAL_FUNC (gnome_print_file_ok_selected),
			    file_selector);

	return file_selector;
}


/**
 * gnome_print_file_dialog:
 * @printer: 
 * 
 * 
 * 
 * Return Value: -1 on user canceled or erorr, 0 otherwise
 **/
gint
gnome_print_file_dialog (GnomePrinter *printer)
{
	GtkWidget *file_selector;

	if (printer->filename)
		g_free (printer->filename);
	printer->filename = NULL;
	
	file_selector = gnome_print_file_create (printer);
	
	gtk_window_set_modal (GTK_WINDOW (file_selector), TRUE);
	gtk_window_set_title (GTK_WINDOW (file_selector), _("Select output file"));

	gtk_widget_show (file_selector);

	gtk_main ();

	return printer->filename ? 0 : -1;
}
