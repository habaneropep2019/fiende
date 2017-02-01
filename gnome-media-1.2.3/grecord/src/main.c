/*
 * GNOME sound-recorder: a soundrecorder and soundplayer for GNOME.
 *
 * Copyright (C) 2000 :
 * Andreas Hyden <a.hyden@cyberpoint.se>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>

#include "gui.h"
#include "grec.h"
#include "prog.h"
#include "preferences.h"

static gchar* geometry = NULL;
static gboolean sfiles = FALSE;
static gboolean srecord = FALSE;
static gboolean splay = FALSE;

static gint grec_save_session (GnomeClient* client, gint phase,
			       GnomeSaveStyle save_stype,
			       gint is_shutdown, GnomeInteractStyle interact_style,
			       gint is_fast, gpointer client_data);

static gint grec_kill_session (GnomeClient* client, gpointer client_data);
static gint on_dontshowagain_dialog_destroy_activate (GtkWidget* widget, gpointer checkbutton);

struct poptOption grec_options[] = {
	{
		"geometry",
		'\0',
		POPT_ARG_STRING,
		&geometry,
		0,
		N_("Specify the geometry of the main window"),
		N_("GEOMETRY")
	},
	{
		"file",
		'f',
		POPT_ARG_NONE,
		&sfiles,
		0,
		N_("Specify a file to be opened"),
		NULL
	},
	{
		"record",
		'r',
		POPT_ARG_NONE,
		&srecord,
		0,
		N_("Specify a file to start recording"),
		NULL
	},
	{
		"play",
		'p',
		POPT_ARG_NONE,
		&splay,
		0,
		N_("Specify a file to start playing"),
		NULL
	},
	{
		NULL,
		'\0',
		0,
		NULL,
		0,
		NULL,
		NULL
	}
};

int
main (int argc, char *argv[])
{
	GtkWidget* grecord_window;
	poptContext pctx;
	GnomeClient* client;
	gchar** args = NULL;
	gboolean dont_show_warningmess;
	gint temp_count;
	gboolean fullpath = FALSE;
	
	/* i18n stuff ---------------------------------- */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);
	
	gnome_init_with_popt_table (_("GNOME Sound recorder"), VERSION, argc, argv,
				    grec_options, 0, &pctx);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-audio2.png");
	args = (gchar**) poptGetArgs (pctx);
	
	mwin.x = -1;
	mwin.y = -1;
	mwin.width = 400;
	mwin.height = 170;
	
	if (geometry)
		gnome_parse_geometry (geometry, &mwin.x, &mwin.y,
				      &mwin.width, &mwin.height);

	/* Session management ------------------------- */
	client = gnome_master_client ();
	gtk_signal_connect (GTK_OBJECT (client),
			    "save_yourself",
			    GTK_SIGNAL_FUNC (grec_save_session),
			    argv[0]);
	gtk_signal_connect (GTK_OBJECT (client),
			    "die",
			    GTK_SIGNAL_FUNC (grec_kill_session),
			    NULL);

	/* Load configuration */
	load_config_file ();

	if (sfiles)
		active_file = g_strdup (args[0]);
	else if (splay) {
		active_file = g_strdup (args[0]);
		on_play_activate_cb (NULL, NULL);
	}
	else if (srecord) {
		active_file = g_strdup (args[0]);
		on_record_activate_cb (NULL, NULL);
	}
	else
		active_file = g_concat_dir_and_file (temp_dir, temp_filename_play);

	poptFreeContext (pctx);
	
	/* Initate some vars */
	PlayEng.is_running = FALSE;
	RecEng.is_running = FALSE;
	
	/* Popup mainwindow */
	grecord_window = create_grecord_window ();
	gtk_widget_show (grecord_window);

	dont_show_warningmess = gnome_config_get_bool ("/grecord/Misc/dontshowwarningmess=FALSE");

	/* Check if the sox command is a path */
	temp_count = 0;
	while (sox_command[temp_count] != '\0') {
		if (sox_command[temp_count] == '/')
			fullpath = TRUE;
		temp_count++;
	}

	/* Check for program 'sox' ------------------- */
	if (fullpath) {
		if (!g_file_exists (sox_command) && !dont_show_warningmess) {
			GtkWidget* dont_show_again_checkbutton = gtk_check_button_new_with_label (_("Don't show this message again."));
			
			gchar* show_mess = g_strdup_printf (_("Could not find '%s'.\nSet the correct path to sox in preferences under the tab 'paths'.\n\nIf you don't have sox, you will not be able to record or do any effects."), sox_command);
			
			GtkWidget* mess = gnome_message_box_new (show_mess,
								 GNOME_MESSAGE_BOX_WARNING,
								 GNOME_STOCK_BUTTON_OK,
								 NULL);
			
			gtk_widget_show (dont_show_again_checkbutton);
			gtk_container_add (GTK_CONTAINER (GNOME_DIALOG (mess)->vbox), dont_show_again_checkbutton);
			
			/* Connect a signal on ok-button, so we can get the stat on the checkbutton */
			gtk_signal_connect (GTK_OBJECT (mess), "destroy",
					    GTK_SIGNAL_FUNC (on_dontshowagain_dialog_destroy_activate), dont_show_again_checkbutton);
			
			gnome_dialog_run (GNOME_DIALOG (mess));
			
			g_free (show_mess);
			on_preferences_activate_cb (NULL, NULL);
		}
	}

	gtk_main ();
	
	/* This funtions free's som strings */
	free_vars ();
	
	return 0;
}

static gint grec_save_session (GnomeClient* client, gint phase,
			       GnomeSaveStyle save_stype,
			       gint is_shutdown, GnomeInteractStyle interact_style,
			       gint is_fast, gpointer client_data)
{
	gchar** argv;
	guint argc;

	argv = g_malloc0(sizeof(gchar*)*4);
	argc = 1;

	argv[0] = client_data;

	if (sfiles && active_file != NULL) {
		argv[1] = "--file";
		argv[2] = active_file;
		argc = 3;
	}

	gnome_client_set_clone_command (client, argc, argv);
	gnome_client_set_restart_command (client, argc, argv);

	return TRUE;
}

static gint grec_kill_session (GnomeClient* client, gpointer client_data)
{
	gchar* file1 = g_concat_dir_and_file (temp_dir, temp_filename_record);
	gchar* file2 = g_concat_dir_and_file (temp_dir, temp_filename_play);
	gchar* file3 = g_concat_dir_and_file (temp_dir, temp_filename_backup);

	remove (file1);
	remove (file2);
	remove (file3);

	g_free (file1);
	g_free (file2);
	g_free (file3);

	gtk_main_quit ();

	return TRUE;
}

static gint on_dontshowagain_dialog_destroy_activate (GtkWidget* widget, gpointer checkbutton)
{
	gboolean stat = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));
	gnome_config_set_bool ("/grecord/Misc/dontshowwarningmess", stat);

	/* Save it */
	gnome_config_sync ();

	/* Exit the dialog */
	return TRUE;
}
