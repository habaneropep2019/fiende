#include <config.h>
#include <math.h>
#include "testgnome.h"


void
create_canvas (void)
{
	GtkWidget *app;
	GtkWidget *notebook;

/* 	gtk_debug_flags = GTK_DEBUG_OBJECTS; */

	app = create_newwin (TRUE, "testGNOME", "Canvas");
	gtk_window_set_policy (GTK_WINDOW (app), TRUE, TRUE, FALSE);

	notebook = gtk_notebook_new ();
	gnome_app_set_contents (GNOME_APP (app), notebook);
	gtk_widget_show (notebook);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), create_canvas_primitives (0), gtk_label_new ("Primitives"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), create_canvas_primitives (1), gtk_label_new ("Antialias"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), create_canvas_arrowhead (), gtk_label_new ("Arrowhead"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), create_canvas_fifteen (), gtk_label_new ("Fifteen"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), create_canvas_features (), gtk_label_new ("Features"));

	gtk_widget_show (app);
}
