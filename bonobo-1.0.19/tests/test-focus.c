/*
 * test-focus.c: A test application to sort focus issues.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2001 Ximian, Inc.
 */

#include "config.h"
#include <gnome.h>
#include <bonobo.h>
#include <liboaf/liboaf.h>

#include <bonobo/bonobo-widget.h>

poptContext ctx;

int
main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *tmp;
	CORBA_ORB  orb;

	free (malloc (8));

	gnome_init_with_popt_table (
		"container", VERSION,
		argc, argv, oaf_popt_options, 0, &ctx);

	textdomain (PACKAGE);

	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error (_("Could not initialize Bonobo!\n"));

	bonobo_activate ();

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Focus test");

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	tmp = gtk_entry_new ();
	gtk_box_pack_start_defaults (GTK_BOX (vbox), tmp);

	tmp = gtk_button_new_with_label ("In Container A");
	gtk_box_pack_start_defaults (GTK_BOX (vbox), tmp);

	tmp = bonobo_widget_new_control ("OAFIID:Bonobo_Sample_Entry", NULL);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), tmp);

	tmp = gtk_button_new_with_label ("In Container B");
	gtk_box_pack_start_defaults (GTK_BOX (vbox), tmp);

	tmp = bonobo_widget_new_control ("OAFIID:Bonobo_Sample_Entry", NULL);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), tmp);

	gtk_widget_show_all (window);

	gtk_main ();

	if (ctx)
		poptFreeContext (ctx);

	return 0;
}
