/*
 * test-ui.c: A test application to hammer the Bonobo UI api.
 *
 * NB. To run this program and test the xml IO code you
 * need to do something like ln -s /{prefix}/bonobo/doc/std-ui.xml ~/.gnome/ui
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Ximian, Inc.
 */

#include "config.h"
#include <sys/time.h>
#include <gnome.h>
#include <bonobo.h>
#include <liboaf/liboaf.h>

#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-win.h>

poptContext ctx;

#define NUM_WIN  1
#define NUM_FILL 10

static void
test_nodes_same (BonoboUINode *a,
		 BonoboUINode *b)
{
#if 0
	BonoboUINode *la, *lb;

	g_assert (bonobo_ui_node_same_attrs (a, b));

	la = bonobo_ui_node_children (a);
	lb = bonobo_ui_node_children (b);

	if (la || lb)
		test_nodes_same (la, lb);
#endif
}

static void
print_time (FILE *file, const char *msg,
	    struct timeval *t1, struct timeval *t2)
{
	long usecdiff = t2->tv_usec - t1->tv_usec;
	long usecdiv = 1000 * 1000;

	fprintf (file, "%s %d:%.6d\n", msg,
		 (int)((t2->tv_sec - t1->tv_sec) + (usecdiff / usecdiv)),
		 (int)(usecdiff % usecdiv));
}

static void
flush (void)
{
	while (gtk_events_pending ())
		gtk_main_iteration ();
	gdk_flush ();
}

static BonoboUIComponent *
fill_window (BonoboWindow *win, BonoboUINode *ui)
{
	BonoboUIContainer *container = bonobo_ui_container_new ();
	BonoboUIComponent *component = bonobo_ui_component_new ("Foo");

	bonobo_ui_container_set_win (container, win);
	bonobo_ui_component_set_container (
		component, BONOBO_OBJREF (container));
	
	if (ui)
		bonobo_ui_component_set_tree (
			component, "/", ui, NULL);

	return component;
}

static void
speed_tests (void)
{
	GtkWidget     *wins [NUM_WIN];
	int            i;
	struct timeval t1, t2;

#if 0
	{ /* GtkWindow */
		g_assert (!gettimeofday (&t1, NULL));
		{
			for (i = 0; i < NUM_WIN; i++) {
				wins [i] = gtk_window_new (GTK_WINDOW_TOPLEVEL);
				gtk_widget_show (wins [i]);
			}
			flush ();
		}
		g_assert (!gettimeofday (&t2, NULL));

		print_time (stderr, "Normal GtkWindow create ", &t1, &t2);

		g_assert (!gettimeofday (&t1, NULL));
		{
			for (i = 0; i < NUM_WIN; i++)
				gtk_widget_destroy (wins [i]);
		
			flush ();
		}
		g_assert (!gettimeofday (&t2, NULL));

		print_time (stderr, "Normal GtkWindow destroy ", &t1, &t2);
	}

	{ /* BonoboWindow */
		g_assert (!gettimeofday (&t1, NULL));
		{
			for (i = 0; i < NUM_WIN; i++) {
				wins [i] = bonobo_window_new ("foo", "baa");
				gtk_widget_show (wins [i]);
			}
			flush ();
		}
		g_assert (!gettimeofday (&t2, NULL));

		print_time (stderr, "plain BonoboWindow create ", &t1, &t2);

		g_assert (!gettimeofday (&t1, NULL));
		{
			for (i = 0; i < NUM_WIN; i++)
				gtk_widget_destroy (wins [i]);
		
			flush ();
		}
		g_assert (!gettimeofday (&t2, NULL));

		print_time (stderr, "plain BonoboWindow destroy ", &t1, &t2);
	}
#endif

	{ /* BonoboWindow + XML */
		BonoboUINode *ui;
		BonoboUIComponent *components [NUM_WIN];

		ui = bonobo_ui_util_new_ui (
			NULL, "std-ui.xml", ".", "foobar");

		if (!ui)
			g_warning ("XML Speed tests failed");
		else {


			g_assert (!gettimeofday (&t1, NULL));
			{
				for (i = 0; i < NUM_WIN; i++) {
					wins [i] = bonobo_window_new ("foo", "baa");
					components [i] = fill_window (BONOBO_WINDOW (wins [i]), ui);
					gtk_widget_show (wins [i]);
				}
				flush ();
			}
			g_assert (!gettimeofday (&t2, NULL));

			print_time (stderr, "plain BonoboWindow create ", &t1, &t2);

			g_assert (!gettimeofday (&t1, NULL));
			{
				for (i = 0; i < NUM_WIN; i++) {
					int i2;
					for (i2 = 0; i2 < NUM_FILL; i2++)
						bonobo_ui_component_set_tree (
							components [i], "/", ui, NULL);
					
					flush ();
				}
			}
			g_assert (!gettimeofday (&t2, NULL));

			print_time (stderr, "BonoboWindow XML thrash ", &t1, &t2);

			g_assert (!gettimeofday (&t1, NULL));
			{
				for (i = 0; i < NUM_WIN; i++)
					gtk_widget_destroy (wins [i]);
		
				flush ();
			}
			g_assert (!gettimeofday (&t2, NULL));

			print_time (stderr, "plain BonoboWindow destroy ", &t1, &t2);

			bonobo_ui_node_free (ui);
		}
	}
}

BonoboUIComponent *global_component;

static void
cb_do_quit (GtkWindow *window, gpointer dummy)
{
	gtk_main_quit ();
}

static void
cb_do_dump (GtkWindow *window, BonoboWindow *win)
{
	bonobo_window_dump (win, "on User input");
}

static void
cb_do_popup (GtkWindow *window, BonoboWindow *win)
{
	GtkWidget *menu;

	menu = gtk_menu_new ();

	bonobo_window_add_popup (win, GTK_MENU (menu), "/popups/MyStuff");

	gtk_widget_show (GTK_WIDGET (menu));
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 3, 0);
}

static void
cb_do_hide_toolbar (GtkWindow *window, BonoboWindow *win)
{
	const char path [] = "/Toolbar";
	char *val;

	val = bonobo_ui_component_get_prop (global_component, path, "hidden", NULL);
	if (val && atoi (val))
		bonobo_ui_component_set_prop (global_component, path, "hidden", "0", NULL);
	else
		bonobo_ui_component_set_prop (global_component, path, "hidden", "1", NULL);
}

static void
cb_set_state (GtkEntry *state_entry, GtkEntry *path_entry)
{
	char *path, *state, *txt, *str;

	path = gtk_entry_get_text (path_entry);
	state = gtk_entry_get_text (state_entry);

	g_warning ("Set state on '%s' to '%s'", path, state);

	bonobo_ui_component_set_prop (
		global_component, path, "state", state, NULL);

	txt = bonobo_ui_component_get_prop (
		global_component, path, "state", NULL);

	g_warning ("Re-fetched state was '%s'", txt);

	str = g_strdup_printf ("The state is now '%s'", txt);
	bonobo_ui_component_set_status (global_component, str, NULL);
	g_free (str);

	g_free (txt);
}

static void
toggled_cb (BonoboUIComponent           *component,
	    const char                  *path,
	    Bonobo_UIComponent_EventType type,
	    const char                  *state,
	    gpointer                     user_data)
{
	fprintf (stderr, "toggled to '%s' type '%d' path '%s'\n",
		 state, type, path);
}

static void
disconnect_progress (GtkObject *progress, gpointer dummy)
{
	gtk_timeout_remove (GPOINTER_TO_UINT (dummy));
}

static gboolean
update_progress (GtkProgress *progress)
{
	float pos = gtk_progress_get_current_percentage (progress);

	if (pos < 0.95)
		gtk_progress_set_percentage (progress, pos + 0.05);
	else
		gtk_progress_set_percentage (progress, 0);

	return TRUE;
}

static void
slow_size_request (GtkWidget      *widget,
		   GtkRequisition *requisition,
		   gpointer        user_data)
{
/*	sleep (2);*/
}

static void
file_exit_cmd (BonoboUIComponent *uic,
	       gpointer           user_data,
	       const char        *verbname)
{
	exit (0);
}

static void
file_open_cmd (BonoboUIComponent *uic,
	       gpointer           user_data,
	       const char        *verbname)
{
	g_warning ("File Open");
}

BonoboUIVerb verbs [] = {
	BONOBO_UI_VERB ("FileExit", file_exit_cmd),
	BONOBO_UI_VERB ("FileOpen", file_open_cmd),

	BONOBO_UI_VERB_END
};

int
main (int argc, char **argv)
{
	BonoboWindow *win;
	CORBA_ORB  orb;
	BonoboUIComponent *componenta;
	BonoboUIComponent *componentb;
	BonoboUIComponent *componentc;
	BonoboUIContainer *container;
	Bonobo_UIContainer corba_container;
	CORBA_Environment  ev;
	char *txt, *fname;

	char simplea [] =
		"<menu>\n"
		"	<submenu name=\"File\" _label=\"_Gå\">\n"
		"		<menuitem name=\"open\" pos=\"bottom\" _label=\"_Open\" verb=\"FileOpen\"pixtype=\"stock\" pixname=\"Open\" _tip=\"Wibble\"/>\n"
		"		<control name=\"MyControl\"/>\n"
		"		<control name=\"ThisIsEmpty\"/>\n"
		"		<menuitem name=\"close\" noplace=\"1\" verb=\"FileExit\" _label=\"_CloseA\" _tip=\"hi\""
		"		pixtype=\"stock\" pixname=\"Close\" accel=\"*Control*q\"/>\n"
		"	</submenu>\n"
		"</menu>";
	char simpleb [] =
		"<submenu name=\"File\" _label=\"_File\">\n"
		"	<menuitem name=\"open\" _label=\"_OpenB\" pixtype=\"stock\" pixname=\"Open\" _tip=\"Open you fool\"/>\n"
		"       <separator/>\n"
		"       <menuitem name=\"toggle\" type=\"toggle\" id=\"MyFoo\" _label=\"_ToggleMe\" _tip=\"a\" accel=\"*Control*t\"/>\n"
		"       <placeholder name=\"Nice\" delimit=\"top\"/>\n"
		"	<menuitem name=\"close\" noplace=\"1\" verb=\"FileExit\" _label=\"_CloseB\" _tip=\"hi\""
		"        pixtype=\"stock\" pixname=\"Close\" accel=\"*Control*q\"/>\n"
		"</submenu>\n";
	char simplec [] =
		"<submenu name=\"File\" _label=\"_FileC\" _tip=\"what!\">\n"
		"    <placeholder name=\"Nice\" delimit=\"top\" hidden=\"1\">\n"
		"	<menuitem name=\"fooa\" _label=\"_FooA\" type=\"radio\" group=\"foogroup\" _tip=\"Radio1\"/>\n"
		"	<menuitem name=\"foob\" _label=\"_FooB\" type=\"radio\" group=\"foogroup\" _tip=\"kippers\"/>\n"
		"	<menuitem name=\"wibble\" verb=\"ThisForcesAnError\" _label=\"_Baa\""
		"        pixtype=\"stock\" pixname=\"Open\" sensitive=\"0\" _tip=\"fish\"/>\n"
		"       <separator/>\n"
		"    </placeholder>\n"
		"</submenu>\n";
	char simpled [] =
		"<menuitem name=\"save\" _label=\"_SaveD\" pixtype=\"stock\" pixname=\"Save\" _tip=\"tip1\"/>\n";
	char simplee [] =
		"<menuitem name=\"fish\" _label=\"_Inplace\" pixtype=\"stock\" pixname=\"Save\" _tip=\"tip2\"/>\n";
	char toola [] =
		"<dockitem name=\"Toolbar\" homogeneous=\"0\" vlook=\"icon\">\n"
		"	<toolitem type=\"toggle\" name=\"foo2\" id=\"MyFoo\"pixtype=\"stock\" pixname=\"Save\""
		"        _label=\"TogSave\" _tip=\"My tooltip\" priority=\"1\"/>\n"
		"	<separator/>\n"
		"	<toolitem name=\"baa\" pixtype=\"stock\" pixname=\"Open\" _label=\"baa\" _tip=\"My 2nd tooltip\" verb=\"testme\"/>\n"
		"	<control name=\"AControl\" _tip=\"a tip on a control\" hidden=\"0\" vdisplay=\"button\"\n"
		"	pixtype=\"stock\" pixname=\"Attach\"/>\n"
		"</dockitem>";
	char toolb [] =
		"<dockitem name=\"Toolbar\" look=\"icon\" relief=\"none\">\n"
		"	<toolitem name=\"foo1\" _label=\"Insensitive\" sensitive=\"0\" hidden=\"0\" priority=\"1\"/>\n"
		"	<toolitem type=\"toggle\" name=\"foo5\" id=\"MyFoo\" pixtype=\"stock\" pixname=\"Close\""
		"	 _label=\"TogSame\" _tip=\"My tooltip\"/>\n"
		"</dockitem>";
	char statusa [] =
		"<item name=\"main\">Kippers</item>\n";
	char statusb [] =
		"<status>\n"
		"	<item name=\"main\"/>\n"
		"	<control name=\"Progress\"/>\n"
		"</status>";
	BonoboUINode *accel;

	free (malloc (8));

	gnome_init_with_popt_table ("container", VERSION,
				    argc, argv, oaf_popt_options, 0, &ctx);

	textdomain (PACKAGE);

	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error (_("Could not initialize Bonobo!\n"));

	bonobo_activate ();

	{ /* Test encode / decode str */
		char *a, *b, *c;
		int   i;
		gboolean err;

		a = g_malloc (256);
		for (i = 0; i < 256; i++) {
			if (i == 255)
				*(a + i) = '\0';
			else
				*(a + i) = i + 1;
		}

		b = bonobo_ui_util_encode_str (a);
		c = bonobo_ui_util_decode_str (a, &err); /* sanity check */
		g_free (c);

		c = bonobo_ui_util_decode_str (b, &err);
		g_assert (err == FALSE);
		if (strcmp (a, c)) {
			g_warning ("Strings differ lengths %d should be %d",
				   strlen (c), strlen (a));

			for (i = 0; i < 256; i++) {
				if (a [i] != b [i])
					printf ("a [%d] (=%d) != b [%d] (=%d)\n",
						i, a [i], i, b [i]);
			}
		} /* else
		     printf ("String encode / decode worked\n"); */

		g_free (c);
		g_free (b);
		g_free (a);

		b = bonobo_ui_util_encode_str ("Hello World");
		c = bonobo_ui_util_decode_str (b, &err);
		g_assert (err == FALSE);

/*		printf ("Encode to '%s' and back to '%s'\n", b, c); */

		g_free (c);
		g_free (b);
	}

	{
		BonoboUINode *n = bonobo_ui_node_from_string (toola);
		test_nodes_same (n, n);
		bonobo_ui_node_free (n);
	}

	speed_tests ();

	win = BONOBO_WINDOW (bonobo_window_new ("Win", "My Test Application"));
	container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (container, win);
	
	bonobo_ui_engine_config_set_path (bonobo_window_get_ui_engine (win),
					  "/test-ui/UIConfig/kvps");

	corba_container = BONOBO_OBJREF (container);

	{
		GtkWidget *box = gtk_vbox_new (FALSE, 0);
		GtkWidget *button;
		GtkWidget *path_entry, *state_entry;

		button = gtk_button_new_with_label ("Press me to test!");
		gtk_signal_connect (GTK_OBJECT (button), "clicked",
				    (GtkSignalFunc) cb_do_quit, NULL);
		gtk_widget_show (GTK_WIDGET (button));
		gtk_box_pack_start_defaults (GTK_BOX (box), button);

		button = gtk_button_new_with_label ("Dump Xml tree");
		gtk_signal_connect (GTK_OBJECT (button), "clicked",
				    (GtkSignalFunc) cb_do_dump, win);
		gtk_widget_show (GTK_WIDGET (button));
		gtk_box_pack_start_defaults (GTK_BOX (box), button);

		button = gtk_button_new_with_label ("Popup");
		gtk_signal_connect (GTK_OBJECT (button), "clicked",
				    (GtkSignalFunc) cb_do_popup, win);
		gtk_widget_show (GTK_WIDGET (button));
		gtk_box_pack_start_defaults (GTK_BOX (box), button);

		button = gtk_button_new_with_label ("Hide toolbar");
		gtk_signal_connect (GTK_OBJECT (button), "clicked",
				    (GtkSignalFunc) cb_do_hide_toolbar, win);
		gtk_widget_show (GTK_WIDGET (button));
		gtk_box_pack_start_defaults (GTK_BOX (box), button);

		path_entry = gtk_entry_new ();
		gtk_entry_set_text (GTK_ENTRY (path_entry), "/menu/File/toggle");
		gtk_widget_show (GTK_WIDGET (path_entry));
		gtk_box_pack_start_defaults (GTK_BOX (box), path_entry);

		state_entry = gtk_entry_new ();
		gtk_entry_set_text (GTK_ENTRY (state_entry), "1");
		gtk_signal_connect (GTK_OBJECT (state_entry), "changed",
				    (GtkSignalFunc) cb_set_state, path_entry);
		gtk_widget_show (GTK_WIDGET (state_entry));
		gtk_box_pack_start_defaults (GTK_BOX (box), state_entry);

		gtk_widget_show (GTK_WIDGET (box));
		bonobo_window_set_contents (win, box);
	}

	gtk_signal_connect (GTK_OBJECT (win), "size_request", 
			    slow_size_request, NULL);

	componenta = bonobo_ui_component_new ("A");
	bonobo_object_unref (BONOBO_OBJECT (componenta));

	componenta = bonobo_ui_component_new ("A");
	componentb = bonobo_ui_component_new ("B");
	componentc = bonobo_ui_component_new ("C");

	bonobo_ui_component_add_verb_list_with_data (
		componenta, verbs, GUINT_TO_POINTER (12));

	bonobo_ui_component_remove_verb (componenta, "FileExit");
	bonobo_ui_component_remove_verb_by_data (componenta, GUINT_TO_POINTER (12));

	bonobo_ui_component_set_container (componenta, corba_container);
	bonobo_ui_component_set_container (componentb, corba_container);
	bonobo_ui_component_set_container (componentc, corba_container);

	global_component = componenta;

	CORBA_exception_init (&ev);

	fname = bonobo_ui_util_get_ui_fname (NULL, "std-ui.xml");
	if (fname && g_file_exists (fname)) {
		fprintf (stderr, "\n\n--- Add std-ui.xml ---\n\n\n");
		bonobo_ui_util_set_ui (componenta, NULL, "std-ui.xml",
				       "gnomecal");
		bonobo_ui_component_thaw (componenta, NULL);

/*		bonobo_ui_component_set_prop (
			componenta, "/menu/Preferences",
			"pixname", "/demo/a.xpm", NULL);*/

		/* NB. bad, bad practice */
		gtk_widget_show_all (GTK_WIDGET (win));

		gtk_main ();
	} else {
		g_warning ("Can't find ~/.gnome/ui/std-ui.xml, perhaps "
			   " you need to symlink bonobo/doc/std-ui.xml there");
		gtk_widget_show (GTK_WIDGET (win));
	}
	g_free (fname);

	bonobo_ui_component_freeze (componenta, NULL);

	fprintf (stderr, "\n\n--- Remove A ---\n\n\n");
	bonobo_ui_component_rm (componenta, "/", &ev);

	bonobo_ui_component_set_translate (componentb, "/status", statusa, &ev);

	bonobo_ui_component_set_translate (componenta, "/", simplea, &ev);

	bonobo_ui_component_set_translate (componentb, "/",
				 "<popups> <popup name=\"MyStuff\"/> </popups>", &ev);
	bonobo_ui_component_set_translate (componenta, "/popups/MyStuff", simpleb, &ev);

	bonobo_ui_component_set_translate (componentb, "/",   toola, &ev);

	{
		GtkWidget *widget = gtk_button_new_with_label ("My Label");
		BonoboControl *control = bonobo_control_new (widget);
		
		gtk_widget_show (widget);
		bonobo_ui_component_object_set (componenta,
						"/menu/File/MyControl",
						BONOBO_OBJREF (control),
						NULL);
	}

	{
		GtkWidget *widget = gtk_entry_new ();
		BonoboControl *control = bonobo_control_new (widget);
		
		gtk_entry_set_text (GTK_ENTRY (widget), "Example text");
		gtk_widget_show (widget);
		bonobo_ui_component_object_set (componenta,
						"/Toolbar/AControl",
						BONOBO_OBJREF (control),
						NULL);
	}

	bonobo_ui_component_add_listener (componentb, "MyFoo", toggled_cb, NULL);

	bonobo_ui_component_set_translate (componentb, "/",     statusb, &ev);

	/* Duplicate set */
	bonobo_ui_component_set_translate (componenta, "/", simplea, &ev);

	bonobo_ui_component_add_verb_list_with_data (
		componenta, verbs, GUINT_TO_POINTER (15));

	bonobo_ui_component_thaw (componenta, NULL);

	bonobo_ui_component_set_status (componenta, "WhatA1", &ev);
	bonobo_ui_component_set_status (componentb, "WhatB2", &ev);
	bonobo_ui_component_set_status (componenta, "WhatA3", &ev);
	bonobo_ui_component_rm (componenta, "/status", &ev);
	bonobo_ui_component_set_status (componentb, "WhatB4", &ev);
	bonobo_ui_component_set_status (componenta, "WhatA5", &ev);
	bonobo_ui_component_set_status (componenta, "WhatA6", &ev);
	bonobo_ui_component_set_status (componentb, "WhatB7", &ev);
	bonobo_ui_component_set_status (componentb, "", &ev);

	{
		char *txt = bonobo_ui_component_get (componenta, "/status/main", TRUE, NULL);
		if (!txt || strcmp (txt, "<?xml version=\"1.0\"?>\n<item name=\"main\">576861744136</item>\n")) {
			g_warning ("Broken merging code '%s'", txt);
			g_assert_not_reached ();
		}
	}

	gtk_main ();

	bonobo_ui_component_freeze (componenta, NULL);

	accel = bonobo_ui_util_build_accel (GDK_A, GDK_CONTROL_MASK, "KeyWibbleVerb");
	bonobo_ui_component_set_tree (componenta, "/keybindings", accel, &ev);
	bonobo_ui_node_free (accel);

	bonobo_ui_component_set_translate (componentb, "/menu", simpleb, &ev);
	bonobo_ui_component_set_translate (componenta, "/",     toolb, &ev);

	bonobo_ui_component_set_prop (componenta, "/menu/File", "label",
				      "_Gå", NULL);

	/* A 'transparent' node merge */
	txt = bonobo_ui_component_get_prop (componenta, "/Toolbar", "look", NULL);
	printf ("Before merge look '%s'\n", txt);
	bonobo_ui_component_set_translate (componenta, "/", "<dockitem name=\"Toolbar\"/>", &ev);
	g_free (txt);
	txt = bonobo_ui_component_get_prop (componenta, "/Toolbar", "look", NULL);
	printf ("After merge look '%s'\n", txt);
	if (txt == NULL || strcmp (txt, "icon"))
		g_warning ("Serious transparency regression");
	g_free (txt);

	bonobo_ui_component_set_translate (componenta, "/menu/File/Nice", simplee, &ev);

	{
		GtkWidget *widget = gtk_progress_bar_new ();
		BonoboControl *control = bonobo_control_new (widget);
		guint id;

		gtk_progress_bar_update (GTK_PROGRESS_BAR (widget), 0.5);
		gtk_widget_show (widget);
		bonobo_ui_component_object_set (componenta, "/status/Progress",
						BONOBO_OBJREF (control),
						NULL);

		id = gtk_timeout_add (100, (GSourceFunc) update_progress, widget);
		gtk_signal_connect (GTK_OBJECT (widget), "destroy",
				    disconnect_progress, GUINT_TO_POINTER (id));
	}

	bonobo_ui_component_set_status (componenta, "This is a very long status message "
					"that should cause the window to be resized if "
					"there is in fact a bug in it", NULL);

	bonobo_ui_component_thaw (componenta, NULL);
	gtk_main ();
	bonobo_ui_component_freeze (componenta, NULL);

	bonobo_ui_component_set_translate (componentc, "/commands",
				 "<cmd name=\"MyFoo\" sensitive=\"0\"/>", &ev);
	bonobo_ui_component_set_translate (componentc, "/menu", simplec, &ev);
	
	bonobo_ui_component_set_translate (componentc, "/menu/File", simpled, &ev);

	bonobo_ui_component_thaw (componenta, NULL);
	gtk_main ();
	bonobo_ui_component_freeze (componenta, NULL);

	fprintf (stderr, "\n\n--- Remove 2 ---\n\n\n");
	bonobo_ui_component_rm (componentb, "/", &ev);
	bonobo_ui_component_set_prop (componentc, "/menu/File/save",
				      "label", "SaveC", NULL);

	bonobo_ui_component_thaw (componenta, NULL);
	gtk_main ();
	bonobo_ui_component_freeze (componenta, NULL);

	fprintf (stderr, "\n\n--- Remove 3 ---\n\n\n");
	bonobo_ui_component_rm (componentc, "/", &ev);

	bonobo_ui_component_thaw (componenta, NULL);
	gtk_main ();
	bonobo_ui_component_freeze (componenta, NULL);

	fprintf (stderr, "\n\n--- Remove 1 ---\n\n\n");
	bonobo_ui_component_rm (componenta, "/", &ev);

	bonobo_ui_component_thaw (componenta, NULL);
	gtk_main ();

	bonobo_object_unref (BONOBO_OBJECT (componenta));
	bonobo_object_unref (BONOBO_OBJECT (componentb));
	bonobo_object_unref (BONOBO_OBJECT (componentc));

	bonobo_object_unref (BONOBO_OBJECT (container));
	gtk_widget_destroy (GTK_WIDGET (win));

	CORBA_exception_free (&ev);

	if (ctx)
		poptFreeContext (ctx);

	return 0;
}
