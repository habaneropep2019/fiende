/* $Id: main.c,v 1.19 2001/06/12 08:20:49 jirka Exp $ */

/*
 *  Mike Hughes <mfh@psilord.com>
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main program function
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <gnome.h>
#include <applet-widget.h>
#include <libgnomeui/gnome-window-icon.h>

#include "dict.h"
#include "gdict-pref.h"
#include "gdict-applet.h"
#include "gdict-app.h"

static gint save_yourself_cb (GnomeClient       *client,
                              gint               phase,
                              GnomeRestartStyle  save_style,
                              gint               shutdown,
                              GnomeInteractStyle interact_style,
                              gint               fast,
                              gpointer           client_data)
{
    gchar *argv[] = {NULL, NULL, NULL, NULL, NULL};
    gchar *word;
    gint argc = 1;

    argv[0] = (gchar *)client_data;
    if (gdict_applet_toggle)
        argv[argc++] = "-a";
    if ((word = gdict_defbox_get_word(defbox)) != NULL)
        argv[argc++] = word;

    gnome_client_set_restart_command(client, argc, argv);
    gnome_client_set_clone_command(client, 0, NULL);

    return TRUE;
}

int main (int argc, char *argv[])
{
    gint i;
	GDictApplet * applet = NULL;

    static struct poptOption args[] = {
        {"noapplet", 'a', POPT_ARG_NONE, 0, 0, "Startup without applet support", NULL},
        {NULL, 0, 0, NULL, 0, NULL, NULL}
    };

    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);

    gdict_applet_toggle = TRUE;
    for (i = 0;  i < argc;  i++)
        if (!strcmp(argv[i], "--noapplet")|| !strcmp(argv[i], "-a"))
            gdict_applet_toggle = FALSE;

    if (!gdict_applet_toggle) {
        GnomeClient *client;

        gnome_init_with_popt_table ("gdict", VERSION, argc, argv,
                                    args, 0, NULL);

        if ((client = gnome_master_client()) != NULL)
            gtk_signal_connect (GTK_OBJECT(client), "save_yourself",
                                GTK_SIGNAL_FUNC(save_yourself_cb),
				(gpointer) argv[0]);
    } else {
        applet_widget_init("gdict", VERSION, argc, argv,
                           args, 0, NULL );
    }

    gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gdict.png");
    gdict_app_create ();
    gdict_pref_load ();
    if ((gdict_init_context () < 0) && (gdict_applet_toggle == FALSE))
      {
            GtkWidget *dialog = gnome_error_dialog (_("Couldn't start the dictionary; most likely, http://www.dict.org could not be contacted."));
            gnome_dialog_run (GNOME_DIALOG (dialog));
	    return 1;
      }
    /* If we can not find the context then we may as well just quit
     * since nothing is usable anymore. But not if we're an applet */
    defbox->context = context;

    if (gdict_applet_toggle)
        applet = gdict_applet_create();

    if (!gdict_applet_toggle)
        for (i = 1;  i < argc;  i++)
            if (argv[i][0] != '-') {
                gdict_defbox_lookup(defbox, argv[i]);
                break;
            }

    if (!gdict_applet_toggle) {
        gtk_widget_show(gdict_app);
        gtk_main();
    } else {
        applet_widget_gtk_main();
		gdict_applet_delete(applet);
    }

    gdict_pref_save();
    dict_context_destroy (context); /* FIXME */

    return 0;
}

