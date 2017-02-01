/* sample for help subsystem, stolen from : */

/* gnome-hello-menus.c -- Example for the "Adding menus" section
   of the Gnome Developers' Tutorial (that's is included in the
   Gnome Developers' Documentation in devel-progs/)
*/
/* Includes: Basic stuff
   Menus
*/

/* Copyright (C) 1998 Mark Galassi, Horacio J. Peña, all rights reserved */

/* including gnome.h gives you all you need to use the gtk toolkit as
   well as the GNOME libraries; it also handles internationalization
   via GNU gettext. Including config.h before gnome.h is very important
   (else gnome-i18n can't find ENABLE_NLS), of course i'm assuming
   that we're in the gnome tree. */
/*#include <config.h> */
#include <gnome.h>

#define VERSION "1.0"

void hello_cb (GtkWidget *widget, void *data);
void about_cb (GtkWidget *widget, void *data);
void quit_cb (GtkWidget *widget, void *data);

void prepare_app();
GtkWidget *app;

/* The menu definitions: File/Exit and Help/About are mandatory */
GnomeUIInfo filemenu[] = {
	{GNOME_APP_UI_ITEM, "Exit", "Exit program", quit_cb, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	{GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL}
};

GnomeUIInfo helpmenu[] = {
	{GNOME_APP_UI_ITEM, "About...", "Info about this program", about_cb,
	 NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	{GNOME_APP_UI_SEPARATOR, NULL, NULL, NULL, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	{GNOME_APP_UI_HELP, NULL, NULL, "sample-help", NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	{GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL}
};
 
GnomeUIInfo mainmenu[] = {
	{GNOME_APP_UI_SUBTREE, "File", NULL, filemenu, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	{GNOME_APP_UI_SUBTREE, "Help", NULL,  helpmenu, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	{GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL}
};

int
main(int argc, char *argv[])
{
	/* gnome_init() is always called at the beginning of a program.  it
	   takes care of initializing both Gtk and GNOME */
	gnome_init ("sample", NULL, argc, argv, 0, NULL);
	
	/* prepare_app() makes all the gtk calls necessary to set up a
	   minimal Gnome application; It's based on the hello world example
	   from the Gtk+ tutorial */
	prepare_app ();
	
	gtk_main ();
	
	return 0;
}

void
prepare_app()
{
	GtkWidget *button;

	/* Make the main window and binds the delete event so you can close
	   the program from your WM */
	app = gnome_app_new ("hello", "Hello World Gnomified");
	gtk_widget_realize (app);
	gtk_signal_connect (GTK_OBJECT (app), "delete_event",
			    GTK_SIGNAL_FUNC (quit_cb),
			    NULL);

	/* Now that we've the main window we'll make the menues */
	/* I'm using GtkMenuFactory, i've asked to the gnome-list if i should
	   use gnome_app_create_menu instead and i'm waiting the answer */
	gnome_app_create_menus(GNOME_APP(app), mainmenu);

	/* We make a button, bind the 'clicked' signal to hello and setting it
	   to be the content of the main window */
	button = gtk_button_new_with_label ("Hello GNOME");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (hello_cb), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (button), 60);
	gnome_app_set_contents ( GNOME_APP (app), button);
	
	/* We now show the widgets, the order doesn't matter, but i suggests 
	   showing the main window last so the whole window will popup at
	   once rather than seeing the window pop up, and then the button form
	   inside of it. Although with such simple example, you'd never 
	   notice. */
	gtk_widget_show (button);
	gtk_widget_show (app);
}

/* Callbacks functions */

void
hello_cb (GtkWidget *widget, void *data)
{
	g_print ("Hello GNOME\n");
	gtk_main_quit ();
	return;
}

void
quit_cb (GtkWidget *widget, void *data)
{
	gtk_main_quit ();
	return;
}

void
about_cb (GtkWidget *widget, void *data)
{
	GtkWidget *about;
	gchar *authors[] = {
/* Here should be your names */
		"Mark Galassi",
		"Horacio J. Peña",
		NULL
	};

	about = gnome_about_new ( "The Hello World Gnomified", VERSION,
				  /* copyright notice */
				  "(C) 1998 the Free Software Foundation",
				  authors,
				  /* another comments */
				  "GNOME is a civilized software system "
				  "so we've a \"hello world\" program",
				  NULL);
	gtk_widget_show (about);
	
	return;
}
