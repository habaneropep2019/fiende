#include "container-filesel.h"

static void
cancel_cb (GtkWidget *caller, GtkWidget *fs)
{
	gtk_widget_destroy (fs);
}

void
container_request_file (SampleApp    *app,
			gboolean      save,
			GtkSignalFunc cb,
			gpointer      user_data)
{
	GtkWidget *fs;

	app->fileselection = fs =
	    gtk_file_selection_new (_("Select file"));

	if (save)
		gtk_file_selection_show_fileop_buttons (GTK_FILE_SELECTION (fs));
	else
		gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (fs));

	gtk_signal_connect (GTK_OBJECT
			    (GTK_FILE_SELECTION (fs)->ok_button),
			    "clicked", cb, user_data);

	gtk_signal_connect (GTK_OBJECT
			    (GTK_FILE_SELECTION (fs)->cancel_button),
			    "clicked", cancel_cb, fs);

	gtk_window_set_modal (GTK_WINDOW (fs), TRUE);

	gtk_widget_show (fs);
}
