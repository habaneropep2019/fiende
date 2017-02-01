/* progress.c
 *
 * Copyright (C) 1999 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */
#include "config.h"
#include <sys/wait.h>
#include <signal.h>
#include <gnome.h>

#include "progress.h"


#define BUFSIZE 256
static char *error_label=NULL;
static char *error_type=GNOME_MESSAGE_BOX_INFO;
static GtkWidget *progress_dialog=NULL;
static GtkWidget *msg_label=NULL;
static GtkWidget *progressbar=NULL;
static gboolean program_ended;
static gboolean show_error_dialog;

static void
read_stdinput (gpointer data,
	       gint source,
	       GdkInputCondition condition)
{
	int size=0;
	static char buf[BUFSIZE];
	static int pos=0;
	char curbuf[BUFSIZE];
	long int val;
	GFloppy *floppy = (GFloppy *)data;

	if (condition != GDK_INPUT_READ)
		return;

	while ((size = read(source,curbuf,MIN (BUFSIZE-1, BUFSIZE-pos-1)))>0) {
		int  i;

		/* append onto existing buffer */
		memcpy (buf+pos, curbuf, size);
		pos += size;

		while (1) {
 
			/* scan to see if we have a new message */
			for (i=0; i<pos && buf[i]; i++);
		    
			if ( i == pos )
				break;

			/* found a '\000' so parse message */
			switch (buf[0]) {
			case 'E':
				show_error_dialog = TRUE;
				error_label = g_strdup (buf+1);
				error_type = GNOME_MESSAGE_BOX_ERROR;
				break;
			case 'M':
				if (msg_label == NULL)
					break;
				gtk_label_set_text (GTK_LABEL (msg_label), buf + 1);
				fflush(stdout);
				break;
			case 'P':
				if (msg_label == NULL)
					break;
				if (strlen (buf) <4)
					break;
				val = strtol (buf + 1, NULL, 10);
				gtk_progress_bar_update (GTK_PROGRESS_BAR (progressbar),
							 ((gfloat) val)/100.0);
				break;
			default:
				if (msg_label == NULL)
					break;
				fflush(stdout);
				gtk_label_set_text (GTK_LABEL (msg_label), buf);
				break;
			}

			/* move remaining data in buffer to start */
			memmove (buf, buf+i+1, pos-i-1);
			pos -= i+1;
		}
	}

	/* zero length read means child exitted */
	if (size == 0) {
		int status;
		int rc;
		GtkWidget *error_dialog;
		GtkWidget *w;
		
		wait4 (floppy->pid, &status, WNOHANG, NULL);
		rc = WEXITSTATUS (status);


		/* destroy input handler */
		gdk_input_remove (floppy->handler_id);
		if (rc == 0 && !WIFSIGNALED (status)) {
			error_label = g_strdup (_("Floppy formatted successfully."));
			show_error_dialog = TRUE;
		}
		if (show_error_dialog == FALSE)
			error_label = g_strdup (_("Floppy formatting cancelled."));
					    

		error_dialog = gnome_message_box_new (error_label, error_type,
						      GNOME_STOCK_BUTTON_CLOSE, NULL);

		/* reset the dialog properties */
		error_type = GNOME_MESSAGE_BOX_INFO;
		g_free (error_label);
		error_label = NULL;

		if (progress_dialog)
			gnome_dialog_set_parent (GNOME_DIALOG (error_dialog), GTK_WINDOW (progress_dialog));
		w = GNOME_DIALOG (error_dialog)->vbox;
		w = ((GtkBoxChild *) GTK_BOX (w)->children->data)->widget;
	        w = ((GtkBoxChild *) GTK_BOX (w)->children->next->data)->widget;
		gtk_label_set_line_wrap (GTK_LABEL (w), TRUE);

		gnome_dialog_run (GNOME_DIALOG (error_dialog));
		if (progress_dialog) {
			/* We didn't hit "Cancel", so we kill the progress dialog */
			gtk_widget_destroy (progress_dialog);
			program_ended = TRUE;
		}
		gtk_main_quit ();
	}
}

static void
progress_destroy_callback (GtkWidget *progress, gpointer data)
{
	GFloppy *floppy = (GFloppy *) data;

	/* This should only occur when the user hits the [X] in the corner. */
	kill (floppy->pid, SIGTERM);
	progress_dialog = NULL;
}

static void
progress_cancel_callback (GtkWidget *progress, gpointer data)
{
	GFloppy *floppy = (GFloppy *) data;

	kill (floppy->pid, SIGTERM);
	gtk_widget_destroy (progress_dialog);
	progress_dialog = NULL;

}

void
setup_progress_and_run (GFloppy *floppy, GtkWidget *parent)
{

	program_ended = FALSE;
	show_error_dialog = FALSE;
	progress_dialog = gnome_dialog_new (_("Format progress."), GNOME_STOCK_BUTTON_CANCEL, NULL);
	gnome_dialog_set_parent (GNOME_DIALOG (progress_dialog), GTK_WINDOW (parent));
	gtk_widget_set_usize (progress_dialog, 300, -1);
	msg_label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (msg_label), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (progress_dialog)->vbox), msg_label, FALSE, FALSE, 0);
	gtk_widget_show (msg_label);
	progressbar = gtk_progress_bar_new ();
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (progress_dialog)->vbox), progressbar, FALSE, FALSE, 0);
	floppy->handler_id = gdk_input_add (floppy->message[0], GDK_INPUT_READ, read_stdinput, floppy);

	gtk_signal_connect (GTK_OBJECT (progress_dialog), "destroy", progress_destroy_callback, floppy);
	gnome_dialog_button_connect (GNOME_DIALOG (progress_dialog), 0, progress_cancel_callback, floppy);
	gtk_widget_show_all (progress_dialog);
	gtk_main ();
}
