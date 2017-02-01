/* app-background.c - Background configuration application.  */

#include <config.h>
#include <strings.h>
#include "capplet-widget.h"
#include <libgnomeui/gnome-window-icon.h>
#include <locale.h>

extern void background_init(void);

static gchar *background_image = NULL;

const struct poptOption options [] = {
	{ "background-image", 'b', POPT_ARG_STRING, &background_image, 0,
	  N_("Set background image."), N_("IMAGE-FILE") },
	{NULL, '\0', 0, NULL, 0}
};

gint
main (gint argc, char *argv[])
{
	GnomeClient *client = NULL;
        GnomeClientFlags flags;
	gchar *session_args[3];
	int token, init_results;
	poptContext ctx;
	char **args;

	setlocale(LC_ALL, "");	
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	init_results = gnome_capplet_init("background-properties", VERSION,
					  argc, argv, options, 0, &ctx);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-ccbackground.png");
	if (init_results < 0) {
                g_warning (_("an initialization error occurred while "
			   "starting 'background-properties-capplet'.\n"
                           "aborting...\n"));
                exit (1);
	}

	if (background_image) {
		gnome_config_set_string ("/Background/Default/wallpaper",
					 background_image);
		gnome_config_sync ();

		background_properties_init();
	}

	client = gnome_master_client ();
	flags = gnome_client_get_flags(client);

	if (flags & GNOME_CLIENT_IS_CONNECTED) {
		token = gnome_startup_acquire_token("GNOME_BACKGROUND_PROPERTIES",
						    gnome_client_get_id(client));
		
		if (token) {
			session_args[0] = argv[0];
			session_args[1] = "--init-session-settings";
			session_args[2] = NULL;
			gnome_client_set_priority (client, 20);
			gnome_client_set_restart_style (client, 
							GNOME_RESTART_ANYWAY);
			gnome_client_set_restart_command (client, 2, 
							  session_args);
		}
		else 
			gnome_client_set_restart_style (client, 
							GNOME_RESTART_NEVER);

                gnome_client_flush (client);
        }
	else
		token = 1;

        if(token) 
                background_properties_init();

	if (init_results != 1) {
		background_init();
	        capplet_gtk_main ();
	}
	return 0;
}
