/*
 * moniker-test.c: Test program for monikers resolving to various interfaces.
 *
 * Author:
 *   Vladimir Vukicevic (vladimir@helixcode.com)
 *
 * Based on moniker-control-test.c, by Joe Shaw (joe@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <gnome.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>

static void display_as_interface (const char *moniker, CORBA_Environment *ev);
static void display_as_stream (const char *moniker, CORBA_Environment *ev);
static void display_as_storage_file_list (const char *moniker, CORBA_Environment *ev);
static void display_as_html (const char *moniker, CORBA_Environment *ev);
static void display_as_control (const char *moniker, CORBA_Environment *ev);

typedef enum {
    AS_NONE = 0,
    AS_INTERFACE,
    AS_STREAM,
    AS_STORAGE_FILE_LIST,
    AS_HTML,
    AS_CONTROL
} MonikerTestDisplayAs;

typedef void (*MonikerDisplayFunction) (const char *moniker, CORBA_Environment *ev);

typedef struct {
    MonikerTestDisplayAs   disp_as;
    MonikerDisplayFunction func;
} MonikerTestDisplayers;

MonikerTestDisplayers displayers[] = {
    {AS_INTERFACE, display_as_interface},
    {AS_STREAM, display_as_stream},
    {AS_STORAGE_FILE_LIST, display_as_storage_file_list},
    {AS_HTML, display_as_html},
    {AS_CONTROL, display_as_control},
    {0}
};

typedef struct {
    gchar *requested_interface;
    gchar *requested_moniker;
    MonikerTestDisplayAs display_as;
    gchar *moniker;

    int ps, pr, pc, ph;
} MonikerTestOptions;

MonikerTestOptions global_mto = { NULL };

struct poptOption moniker_test_options [] = {
    {"interface", 'i', POPT_ARG_STRING, &global_mto.requested_interface, 'i', "request specific interface", "interface"},
    {"stream",    's', POPT_ARG_NONE, &global_mto.ps, 's', "request Bonobo/Stream", NULL },
    {"storage",   'r', POPT_ARG_NONE, &global_mto.pr, 'r', "request Bonobo/Storage", NULL },
    {"control",   'c', POPT_ARG_NONE, &global_mto.pc, 'c', "request Bonobo/Control", NULL },
    {"html",      'h', POPT_ARG_NONE, &global_mto.ph, 'h', "request Bonobo/Stream and display as HTML", NULL },
    {NULL, 0, 0, NULL, 0, 0}
};


static void
do_moniker_magic (void)
{
    CORBA_Environment ev;
    MonikerTestDisplayers *iter = displayers;
    CORBA_exception_init (&ev);

    while (iter->disp_as) {
        if (iter->disp_as == global_mto.display_as) {
            (*iter->func) (global_mto.requested_moniker, &ev);
            CORBA_exception_free (&ev);
            return;
        }
        iter++;
    }

    g_error ("Didn't find handler!");
}

static void
display_as_interface (const char *moniker, CORBA_Environment *ev)
{
    Bonobo_Unknown the_unknown;

    the_unknown = bonobo_get_object (moniker, global_mto.requested_interface, ev);
    if (ev->_major == CORBA_NO_EXCEPTION && the_unknown) {
        fprintf (stderr, "Requesting interface %s: SUCCESS\n", global_mto.requested_interface);
        bonobo_object_release_unref (the_unknown, ev);
        return;
    }

    fprintf (stderr, "Requesting interface: %s: EXCEPTION: %s\n",
             global_mto.requested_interface,
             ev->_repo_id);
}

static void
display_as_stream (const char *moniker, CORBA_Environment *ev)
{
    Bonobo_Stream the_stream;

    the_stream = bonobo_get_object (moniker, "IDL:Bonobo/Stream:1.0", ev);
    if (ev->_major != CORBA_NO_EXCEPTION || !the_stream) {
        g_error ("Couldn't get Bonobo/Stream interface");
    }

    fprintf (stderr, "Writing stream to stdout...\n");
    do {
        Bonobo_Stream_iobuf *stream_iobuf;
        Bonobo_Stream_read (the_stream, 512, &stream_iobuf, ev);
        if (ev->_major != CORBA_NO_EXCEPTION) {
            bonobo_object_release_unref (the_stream, ev);
            g_error ("got exception %s while reading from stream!",
                     ev->_repo_id);
        }

        if (stream_iobuf->_length == 0) {
            CORBA_free (stream_iobuf);
            bonobo_object_release_unref (the_stream, ev);
            return;
        }

        fwrite (stream_iobuf->_buffer, stream_iobuf->_length, 1,
                stdout);

	CORBA_free (stream_iobuf);
    } while (1);
}

static void
display_as_storage_file_list (const char *moniker, CORBA_Environment *ev)
{
    Bonobo_Storage the_storage;
    Bonobo_Storage_DirectoryList *storage_contents;
    Bonobo_StorageInfo *bsi;
    int i;

    the_storage = bonobo_get_object (moniker, "IDL:Bonobo/Storage:1.0", ev);
    if (ev->_major != CORBA_NO_EXCEPTION || !the_storage) {
        g_error ("Couldn't get Bonobo/Storage interface");
    }

    storage_contents = Bonobo_Storage_listContents (the_storage,
                                                    "",
                                                    Bonobo_FIELD_CONTENT_TYPE |
                                                    Bonobo_FIELD_SIZE |
                                                    Bonobo_FIELD_TYPE,
                                                    ev);
    if (!storage_contents || (storage_contents && !storage_contents->_buffer)) {
        g_error ("got NULL storage_contents!\n");
    }

    bsi = storage_contents->_buffer;
    printf ("Storage List\n");
    printf ("------------\n");
    for (i = 0; i < storage_contents->_length; i++) {
        printf ("% 3d: %20s % 10d %c %15s\n",
                i,
                bsi[i].name,
                bsi[i].size,
                bsi[i].type == Bonobo_STORAGE_TYPE_DIRECTORY ? 'd' : 'r',
                bsi[i].content_type);
    }

    bonobo_object_release_unref (the_storage, ev);
    /* how do I free the silly dirlist? */
}

static void
display_as_html (const char *moniker, CORBA_Environment *ev)
{
    g_error ("Not implemented");
}

static void
display_as_control (const char *moniker, CORBA_Environment *ev)
{
	Bonobo_Control  the_control;
	GtkWidget      *widget;
	BonoboUIContainer *ui_container;

	GtkWidget *window;

	the_control = bonobo_get_object (moniker, "IDL:Bonobo/Control:1.0", ev);
	if (ev->_major != CORBA_NO_EXCEPTION || !the_control)
		g_error ("Couldn't get Bonobo/Control interface");

	window = bonobo_window_new ("moniker-test", moniker);
	ui_container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (ui_container, BONOBO_WINDOW (window));

	gtk_window_set_default_size (GTK_WINDOW (window), 400, 350);

	widget = bonobo_widget_new_control_from_objref (the_control,
		BONOBO_OBJREF (ui_container));
	
	bonobo_object_unref (BONOBO_OBJECT (ui_container));

	if (ev->_major != CORBA_NO_EXCEPTION || !widget)
		g_error ("Couldn't get a widget from the_control");

	bonobo_control_frame_control_activate (
		bonobo_widget_get_control_frame (BONOBO_WIDGET (widget)));

	bonobo_window_set_contents (BONOBO_WINDOW (window), widget);

	gtk_signal_connect (GTK_OBJECT (window), "destroy",
			    GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

	gtk_widget_show_all (window);
	gtk_main ();
}

int
main (int argc, char **argv)
{
	CORBA_ORB   orb;
	poptContext ctx = NULL;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	free (malloc (8)); /* -lefence */

	gnomelib_register_popt_table (oaf_popt_options, _("Oaf options"));

	gnome_init_with_popt_table ("moniker-test", "0.0", argc, argv,
				    moniker_test_options, 0, &ctx);

	if ((orb = oaf_init (argc, argv)) == NULL)
		g_error ("Cannot init oaf");

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Cannot init bonobo");

	if (global_mto.ps + global_mto.pr + global_mto.ph +
	    global_mto.pc > 1) {
		poptPrintUsage (ctx, stderr, 0);
		return 1;
	}

	if (global_mto.requested_interface)
		global_mto.display_as = AS_INTERFACE;
	else if (global_mto.ps)
		global_mto.display_as = AS_STREAM;
	else if (global_mto.pr)
		global_mto.display_as = AS_STORAGE_FILE_LIST;
	else if (global_mto.ph)
		global_mto.display_as = AS_HTML;
	else if (global_mto.pc)
		global_mto.display_as = AS_CONTROL;
	else {
		fprintf (stderr, "Usage: %s [-i interface] [-srch] <moniker>\n", argv [0]);
		fprintf (stderr, "Run %s --help for more info\n", argv [0]);
		return 1;
	}


	poptSetOtherOptionHelp (ctx, "<moniker>");
	global_mto.requested_moniker = g_strdup (poptGetArg (ctx));
	if (!global_mto.requested_moniker) {
		fprintf (stderr, "Usage: %s [-i interface] [-srch] <moniker>\n", argv[0]);
		fprintf (stderr, "Run %s --help for more info\n", argv[0]);
		return 1;
	}

	poptFreeContext (ctx);
	/* done with nasty popt stuff */

	fprintf (stderr, "Resolving moniker '%s' as ", global_mto.requested_moniker);
	switch (global_mto.display_as) {
        case AS_INTERFACE:
		fprintf (stderr, global_mto.requested_interface);
		break;
        case AS_STREAM:
		fprintf (stderr, "IDL:Bonobo/Stream:1.0");
		break;
        case AS_STORAGE_FILE_LIST:
		fprintf (stderr, "IDL:Bonobo/Storage:1.0");
		break;
        case AS_HTML:
		fprintf (stderr, "IDL:Bonobo/Control:1.0 (html)");
		break;
        case AS_CONTROL:
		fprintf (stderr, "IDL:Bonobo/Control:1.0");
		break;
        default:
		fprintf (stderr, "???");
		break;
	}
	fprintf (stderr, "\n");

	bonobo_activate ();

	do_moniker_magic ();
	return 0;
}
