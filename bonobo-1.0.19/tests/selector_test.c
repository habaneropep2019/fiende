/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <config.h>
#include <gnome.h>
#include <gtk/gtk.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-selector.h>

#if USING_OAF
#include <liboaf/liboaf.h>
#else
#include <libgnorba/gnorba.h>
#endif

CORBA_Environment ev;
CORBA_ORB orb;

static void
noact_callback (GtkWidget *widget, gpointer data)
{
	gchar *text;

	text = bonobo_selector_select_id (_("Select an object"), NULL);
	g_print ("%s\n", text);

	g_free (text);
}

static void
quit_callback (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();
}

static void
panel_callback (GtkWidget *widget, gpointer data)
{
/* it filters! */
#if USING_OAF
	g_warning ("You can't get an id of a panel applet since the panel"
		   "is using GOAD at the moment");
#else
	{
		const gchar *ints [] = { "IDL:Bonobo/Applet:1.0", NULL };
		gchar *text;

		text = gnome_bonobo_select_goad_id (_("Select an object"), ints);
		
		g_print ("%s\n", text);
		g_free (text);
	}
#endif
}

int
main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *button;

	CORBA_exception_init (&ev);

#if USING_OAF
        gnome_init_with_popt_table("BonoboSel Test", "1.0",
				   argc, argv,
				   oaf_popt_options, 0, NULL); 
	orb = oaf_init (argc, argv);
#else
	gnome_CORBA_init ("BonoboSel Test", "1.0", &argc, argv, 0, &ev);
	orb = gnome_CORBA_ORB ();
#endif

	window = gnome_app_new ("selector_test", "Bonobo Selection Test");
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", 
		GTK_SIGNAL_FUNC (quit_callback), NULL);
	
	vbox = gtk_vbox_new (TRUE, 0);
	gnome_app_set_contents (GNOME_APP (window), vbox);
	
	button = gtk_button_new_with_label ("Get id");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
		GTK_SIGNAL_FUNC (noact_callback), NULL);
	gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 5);

	button = gtk_button_new_with_label ("Get id of panel applet");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
		GTK_SIGNAL_FUNC (panel_callback), NULL);
	gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 5);
	
	gtk_widget_show_all (window);

	gtk_main ();
	
	return 0;
}
