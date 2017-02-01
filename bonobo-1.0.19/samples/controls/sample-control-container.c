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
#include <liboaf/liboaf.h>
#include <bonobo.h>

Bonobo_PropertyBag pb = CORBA_OBJECT_NIL;

static void 
prop_changed_cb (BonoboListener    *listener,
		 char              *event_name, 
		 CORBA_any         *arg,
		 CORBA_Environment *ev,
		 gpointer           user_data)
{
	GtkCList *clist = GTK_CLIST (user_data);
	gchar    *value;

	value = g_strdup_printf ("%s", BONOBO_ARG_GET_BOOLEAN (arg) ? "TRUE" :"FALSE");

	gtk_clist_set_text (clist, 0, 1, value);

	g_free (value);

}

static void
populate_property_list (GtkWidget *bw, GtkCList *clist)
{
	GList *property_list, *l;

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
}

static void
edit_property (GtkCList *clist, GdkEventButton *event, BonoboWidget *bw)
{
	gchar *prop;
	gint row, col;
	GList *l;
	CORBA_TypeCode tc;

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
		populate_property_list (GTK_WIDGET (bw), clist);
	}

}

static GtkWidget *
create_proplist (GtkWidget *bw)
{
	gchar *clist_titles[] = {"Property Name", "Value"};
	GtkWidget *clist;

	/* Put the property CList on the bottom. */
	clist = gtk_clist_new_with_titles (2, clist_titles);
	gtk_signal_connect (GTK_OBJECT (clist), "button_press_event",
		GTK_SIGNAL_FUNC (edit_property), bw);
 
	populate_property_list (bw, GTK_CLIST (clist));

	bonobo_event_source_client_add_listener (pb, prop_changed_cb, 
	        "Bonobo/Property:change:running", NULL, clist); 

	return clist;
}

static void
incr_calc (GtkButton *button, BonoboWidget *control)
{
	CORBA_double i;

	bonobo_widget_get_property (control, "value", &i, NULL);
	i+= 0.37;
	bonobo_widget_set_property (control, "value", i, NULL);
}

static void
toggle_clock (GtkButton *button, BonoboWidget *control)
{
	CORBA_boolean state;

	bonobo_widget_get_property (control, "running", &state, NULL);

	bonobo_widget_set_property (control, "running", !state, NULL);
}

static void
app_destroy_cb (GtkWidget *app, BonoboUIContainer *uic)
{
	bonobo_object_unref (BONOBO_OBJECT (uic));
	if (pb != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (pb, NULL);
	pb = CORBA_OBJECT_NIL;

	gtk_main_quit ();
/*	g_warning ("Main level %d\n", gtk_main_level ());*/
}

static int
app_delete_cb (GtkWidget *widget, GdkEvent *event, gpointer dummy)
{
	gtk_widget_destroy (GTK_WIDGET (widget));
	return FALSE;
}

static guint
container_create (void)
{
	GtkWidget       *control;
	GtkWidget       *proplist;
	GtkWidget       *box;
	GtkWidget       *button;
	GtkWidget       *clock_button;
	BonoboUIContainer *uic;
	BonoboControlFrame *cf;
	GtkWindow       *window;
	GtkWidget       *app;

	app = bonobo_window_new ("sample-control-container",
			      "Sample Bonobo Control Container");

	window = GTK_WINDOW (app);
	
	uic = bonobo_ui_container_new ();

	bonobo_ui_container_set_win (uic, BONOBO_WINDOW (app));

	gtk_window_set_default_size (window, 500, 440);
	gtk_window_set_policy (window, TRUE, TRUE, FALSE);

	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			    GTK_SIGNAL_FUNC (app_delete_cb), NULL);

	gtk_signal_connect (GTK_OBJECT (window), "destroy",
			    GTK_SIGNAL_FUNC (app_destroy_cb), uic);

	box = gtk_vbox_new (FALSE, 0);
	bonobo_window_set_contents (BONOBO_WINDOW (app), box);

	control = bonobo_widget_new_control ("OAFIID:Bonobo_Sample_Calculator",
					     BONOBO_OBJREF (uic));

	if (control)
		gtk_box_pack_start (GTK_BOX (box), control, TRUE, TRUE, 0);

	button = gtk_button_new_with_label ("Increment result");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    (GtkSignalFunc)incr_calc, control);
	
	control = bonobo_widget_new_control ("OAFIID:Bonobo_Sample_Clock",
					     BONOBO_OBJREF (uic));

	if (control)
		gtk_box_pack_start (GTK_BOX (box), control, TRUE, TRUE, 0);

	clock_button = gtk_button_new_with_label ("Pause/Resume Clock");
	gtk_signal_connect (GTK_OBJECT (clock_button), "clicked",
			    (GtkSignalFunc) toggle_clock, control);

	gtk_box_pack_start (GTK_BOX (box), clock_button, TRUE, TRUE, 0);

	cf = bonobo_widget_get_control_frame (BONOBO_WIDGET (control));
	pb = bonobo_control_frame_get_control_property_bag (cf, NULL);
	
	proplist = create_proplist (control);

	control = bonobo_widget_new_control ("OAFIID:Bonobo_Sample_Entry",
					     BONOBO_OBJREF (uic));

	if (control)
		gtk_box_pack_start (GTK_BOX (box), control, TRUE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (box), proplist, TRUE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);

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

        gnome_init_with_popt_table ("sample-control-container", "0.0",
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
