/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include "config.h"
#include <gnome.h>
#include <gtk/gtk.h>
#include "gtkscrollpane.h"
#include "gtkgs.h"
#include "prefs.h"
#include "ggvwindow.h"
#include "gsmessage.h"


/* clears the text in the tbox */
static void del_gs_status_text (ggv_window *ggv)
{
	gtk_text_freeze (GTK_TEXT (ggv->gs_text));
	gtk_text_set_point (GTK_TEXT (ggv->gs_text), 0);
	gtk_text_forward_delete (GTK_TEXT (ggv->gs_text),
				 gtk_text_get_length (GTK_TEXT (ggv->gs_text)));
	gtk_text_thaw (GTK_TEXT (ggv->gs_text));
}

static gint
delete_statustext_event (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	/* ggv->status is created and deleted when ggv is created/deleted
         * In the meantime we only show or hide it */
        gtk_widget_hide (((ggv_window *)data)->status);
	return TRUE;
}

static void close_clicked (GtkButton *button, gpointer data)
{
        gtk_widget_hide (((ggv_window *)data)->status);
}


static void help_clicked (GtkButton *button, gpointer data)
{
	gchar *helpfile;

	helpfile = gnome_help_file_find_file("ggv", "probs.html");
	if (helpfile) {
                gchar *url;
		url = g_strconcat ("ghelp:", helpfile, NULL);
                gnome_help_goto (NULL, url);
		g_free (url);
		g_free(helpfile);
	} else {
		gnome_error_dialog (_("Couldn't find the GGv manual!"));
	}
}


/* FIXME: this should be configurable */
#define STATUS_WIDTH 520
#define STATUS_HEIGHT 320

static void set_up_gs_status(ggv_window *ggv)
{
	GtkWidget *vbox, *hbox, *vscrollbar, *buttonbox, *button;
        GtkAccelGroup *accel_group;

        accel_group = gtk_accel_group_new ();

        ggv->status = gtk_window_new (GTK_WINDOW_DIALOG);
	gtk_window_set_default_size (GTK_WINDOW (ggv->status),
				     STATUS_WIDTH, STATUS_HEIGHT);
	gtk_window_set_policy (GTK_WINDOW (ggv->status), TRUE, TRUE, FALSE);

	gtk_signal_connect(GTK_OBJECT(ggv->status), "delete_event",
                           GTK_SIGNAL_FUNC(delete_statustext_event), ggv);
	gtk_signal_connect (GTK_OBJECT (ggv->status), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed), &(ggv->status));

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (ggv->status), vbox);

        /* set up gtk_text */
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

	ggv->gs_text = gtk_text_new (NULL, NULL);
	gtk_widget_show (ggv->gs_text);
	gtk_box_pack_start (GTK_BOX (hbox), ggv->gs_text, TRUE, TRUE, 0);

	vscrollbar = gtk_vscrollbar_new (GTK_TEXT (ggv->gs_text)->vadj);
	gtk_box_pack_start (GTK_BOX (hbox), vscrollbar, FALSE, FALSE, 0);
	gtk_widget_show (vscrollbar);

	/* set up buttons */
        buttonbox = gtk_hbutton_box_new ();
	gtk_button_box_set_layout (GTK_BUTTON_BOX (buttonbox),
				   GTK_BUTTONBOX_SPREAD);
	gtk_widget_show (buttonbox);
	gtk_box_pack_start (GTK_BOX (vbox), buttonbox, FALSE, FALSE, 4);

        button = gnome_stock_button (GNOME_STOCK_BUTTON_CLOSE);
        gtk_widget_show (button);
        gtk_container_add (GTK_CONTAINER (buttonbox), button);
        GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
        gtk_widget_add_accelerator (button, "clicked", accel_group,
                                    GDK_Return, 0, GTK_ACCEL_VISIBLE);
        gtk_widget_add_accelerator (button, "clicked", accel_group,
                                    GDK_Escape, 0, GTK_ACCEL_VISIBLE);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
                            GTK_SIGNAL_FUNC (close_clicked), ggv);
        gtk_widget_grab_default (button);

        button = gnome_stock_button (GNOME_STOCK_BUTTON_HELP);
        gtk_widget_show (button);
	gtk_container_add (GTK_CONTAINER (buttonbox), button);
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                            GTK_SIGNAL_FUNC (help_clicked), ggv);

        gtk_window_add_accel_group (GTK_WINDOW (ggv->status), accel_group);
}


/* set up status window, clear text field, but do not show window */
void init_gs_status (ggv_window *ggv, gchar *title)
{
	g_return_if_fail (ggv);

	if (!ggv->status)
		set_up_gs_status (ggv);
	else
		del_gs_status_text (ggv);

	if (title)
                gtk_window_set_title (GTK_WINDOW (ggv->status), title);
}


/* shows the status window */
void show_gs_status (ggv_window *ggv)
{
	g_return_if_fail (ggv);

	if (!ggv->status)
		init_gs_status (ggv, NULL);

	if (!GTK_WIDGET_VISIBLE (ggv->status))
                gtk_widget_show (ggv->status);
}


/* adds new_text to the text box */
void add_gs_status_text (ggv_window *ggv, gchar *text, gint show)
{
	g_return_if_fail (ggv);
	g_return_if_fail (text);

        if (!ggv->status)
		init_gs_status (ggv, NULL);

	if (show)
		show_gs_status (ggv);

	gtk_text_insert(GTK_TEXT(ggv->gs_text),
			NULL, NULL, NULL, text, -1);
}
