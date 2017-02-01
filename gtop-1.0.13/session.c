/*
 * gnome-support.c - GNOMEificating code for ghex (actually only SM)
 * written by Jaka Mocnik <jaka.mocnik@kiss.uni-lj.si>
 */

#include <session.h>
#include <properties.h>

static gchar *geometry = NULL;

const struct poptOption gtop_options [] = {
	{ "geometry", 'g', POPT_ARG_STRING, &initial_geometry, 0,
	  N_("Set initial geometry to GEOMETRY where\n"
	     "GEOMETRY is <width>x<height>+<x>+<y>.\n"
	     "Disabled when using session management."),
	  N_("GEOMETRY") },
	{NULL, '\0', 0, NULL, 0}
};

const gchar *initial_geometry = NULL;

/* Session management */

int save_state (GnomeClient        *client,
                gint                phase,
                GnomeRestartStyle   save_style,
                gint                shutdown,
                GnomeInteractStyle  interact_style,
                gint                fast,
                gpointer            client_data)
{
	gchar *prefix= gnome_client_get_config_prefix (client);
	gchar *argv[]= { "rm", "-r", NULL };

	/* Save the state using gnome-config stuff. */
	gnome_config_push_prefix (prefix);

	gnome_mdi_save_state (mdi, "MDI Session");

	gnome_config_set_int ("MDI Session/mdi_mode",
			      gtop_properties.global.mdi_mode);

	gnome_config_pop_prefix();
	gnome_config_sync();

	/* Here is the real SM code. We set the argv to the parameters needed
	   to restart/discard the session that we've just saved and call
	   the gnome_session_set_*_command to tell the session manager it. */
	argv[2] = gnome_config_get_real_path (prefix);
	gnome_client_set_discard_command (client, 3, argv);

	/* Set commands to clone and restart this application.  Note that we
	   use the same values for both -- the session management code will
	   automatically add whatever magic option is required to set the
	   session id on startup.  */
	argv[0] = (char*) client_data;
	gnome_client_set_clone_command (client, 1, argv);
	gnome_client_set_restart_command (client, 1, argv);
	
	return TRUE;
}

gint
client_die (GnomeClient *client, gpointer client_data)
{
  gtk_exit (0);
  
  return FALSE;
}
