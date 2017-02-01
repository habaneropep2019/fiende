/*
 * gnome-support.h - forward decls of GNOMEificating functions
 * written by Jaka Mocnik <jaka.mocnik@kiss.uni-lj.si>
 */

#ifndef SESSION_H
#define SESSION_H

#include <global.h>

BEGIN_GNOME_DECLS

extern const gchar *initial_geometry;

extern const struct poptOption gtop_options [];

int save_state      (GnomeClient        *client,
		     gint                phase,
		     GnomeRestartStyle   save_style,
		     gint                shutdown,
		     GnomeInteractStyle  interact_style,
		     gint                fast,
		     gpointer            client_data);

gint client_die (GnomeClient *client, gpointer client_data);

END_GNOME_DECLS

#endif
