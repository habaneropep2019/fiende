/*
 * Sensitivity showing hack, ugly code
 *  -George
 *
 * Used to be:
 *
 * sample-control-container.c
 * 
 * Authors:
 *   Nat Friedman  (nat@helixcode.com)
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#include <gnome.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>


static void
app_destroy_cb (GtkWidget *app, BonoboUIContainer *uic)
{
	gtk_exit (0);
}

static void
toggle_sensitive (GtkWidget *w, GtkWidget *control)
{
	if (GTK_WIDGET_SENSITIVE (control))
		gtk_widget_set_sensitive (control, FALSE);
	else
		gtk_widget_set_sensitive (control, TRUE);
}

static GtkWidget *
make_inprocess_control (BonoboUIContainer *uic)
{
	BonoboControl *control;
	GtkWidget *controlw;
	GtkWidget *w = gtk_label_new (">>> this is a control <<<");
	gtk_widget_show (w);

	control = bonobo_control_new (w);

	controlw = bonobo_widget_new_control_from_objref
		(BONOBO_OBJREF (control), BONOBO_OBJREF (uic));
	
	return controlw;
}


static guint
container_create (void)
{
	GtkWidget       *control;
	GtkWidget       *box;
	BonoboUIContainer *uic;
	GtkWindow       *window;
	GtkWidget       *app;

	app = bonobo_window_new ("sample-control-container",
				 "Sample Bonobo Control Container");

	window = GTK_WINDOW (app);
	
	uic = bonobo_ui_container_new ();

	bonobo_ui_container_set_win (uic, BONOBO_WINDOW (app));

	gtk_window_set_default_size (window, 500, 440);
	gtk_window_set_policy (window, TRUE, TRUE, FALSE);

	gtk_signal_connect (GTK_OBJECT (window), "destroy",
			    GTK_SIGNAL_FUNC (app_destroy_cb), uic);

	box = gtk_vbox_new (FALSE, 5);
	bonobo_window_set_contents (BONOBO_WINDOW (app), box);

	control = bonobo_widget_new_control ("OAFIID:Bonobo_Sample_Clock",
					     BONOBO_OBJREF (uic));

	if (control) {
		GtkWidget *w;

		gtk_widget_set_sensitive (GTK_WIDGET (control), FALSE);
		
		gtk_box_pack_start (GTK_BOX (box), 
				    gtk_label_new ("Initially insensitive:"),
				    FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (box), control, TRUE, TRUE, 0);
		w = gtk_button_new_with_label ("toggle_sensitivity");
		gtk_box_pack_start (GTK_BOX (box), w, FALSE, FALSE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
				    GTK_SIGNAL_FUNC (toggle_sensitive),
				    control);
	}

	gtk_box_pack_start (GTK_BOX (box), 
			    gtk_hseparator_new (),
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), 
			    gtk_hseparator_new (),
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), 
			    gtk_hseparator_new (),
			    FALSE, FALSE, 0);

	control = bonobo_widget_new_control ("OAFIID:Bonobo_Sample_Clock",
					     BONOBO_OBJREF (uic));

	if (control) {
		GtkWidget *w;

		gtk_box_pack_start (GTK_BOX (box), 
				    gtk_label_new ("Initially sensitive:"),
				    FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (box), control, TRUE, TRUE, 0);
		w = gtk_button_new_with_label ("toggle_sensitivity");
		gtk_box_pack_start (GTK_BOX (box), w, FALSE, FALSE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
				    GTK_SIGNAL_FUNC (toggle_sensitive),
				    control);
	}

	gtk_box_pack_start (GTK_BOX (box), 
			    gtk_hseparator_new (),
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), 
			    gtk_hseparator_new (),
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), 
			    gtk_hseparator_new (),
			    FALSE, FALSE, 0);


	control = make_inprocess_control (uic);

	if (control) {
		GtkWidget *w;

		gtk_widget_set_sensitive (GTK_WIDGET (control), FALSE);
		
		gtk_box_pack_start (GTK_BOX (box), 
				    gtk_label_new ("inprocess initially insensitive:"),
				    FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (box), control, TRUE, TRUE, 0);
		w = gtk_button_new_with_label ("toggle_sensitivity");
		gtk_box_pack_start (GTK_BOX (box), w, FALSE, FALSE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
				    GTK_SIGNAL_FUNC (toggle_sensitive),
				    control);
	}

	gtk_box_pack_start (GTK_BOX (box), 
			    gtk_hseparator_new (),
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), 
			    gtk_hseparator_new (),
			    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), 
			    gtk_hseparator_new (),
			    FALSE, FALSE, 0);

	control = make_inprocess_control (uic);

	if (control) {
		GtkWidget *w;

		gtk_box_pack_start (GTK_BOX (box), 
				    gtk_label_new ("inprocess initially sensitive:"),
				    FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (box), control, TRUE, TRUE, 0);
		w = gtk_button_new_with_label ("toggle_sensitivity");
		gtk_box_pack_start (GTK_BOX (box), w, FALSE, FALSE, 0);
		gtk_signal_connect (GTK_OBJECT (w), "clicked",
				    GTK_SIGNAL_FUNC (toggle_sensitive),
				    control);
	}

	gtk_widget_show_all (GTK_WIDGET (window));

	return FALSE;
}

int
main (int argc, char **argv)
{
	CORBA_Environment ev;
	CORBA_ORB orb;
	CORBA_exception_init (&ev);

	/* Encorage -lefence to play ball */
	{ char *tmp = malloc (4); if (tmp) free (tmp); }

        gnome_init_with_popt_table ("test-sensitivity", "0.0",
				    argc, argv,
				    oaf_popt_options, 0, NULL); 
	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Could not initialize Bonobo");

	/*
	 * We can't make any CORBA calls unless we're in the main
	 * loop.  So we delay creating the container here.
	 */
	gtk_idle_add ((GtkFunction) container_create, NULL);

	bonobo_main ();

	return 0;
}
