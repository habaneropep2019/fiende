/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* check-program.c: A simple driver for eel self checks.

   Copyright (C) 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#include <config.h>

#include <gnome-xml/parser.h>
#include <gnome.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <eel/eel-debug.h>
#include <eel/eel-glib-extensions.h>
#include <eel/eel-lib-self-check-functions.h>
#include <eel/eel-self-checks.h>

int
main (int argc, char *argv[])
{
#ifndef EEL_OMIT_SELF_CHECK
	/* Make criticals and warnings stop in the debugger if
	 * NAUTILUS_DEBUG is set. Unfortunately, this has to be done
	 * explicitly for each domain.
	 */
	if (g_getenv ("NAUTILUS_DEBUG") != NULL) {
		eel_make_warnings_and_criticals_stop_in_debugger
			(G_LOG_DOMAIN, g_log_domain_glib,
			 "Bonobo",
			 "Gdk",
			 "GnomeUI",
			 "GnomeVFS",
			 "GnomeVFS-CORBA",
			 "GnomeVFS-pthread",
			 "Gtk",
			 "Eel",
			 "Eel-Authenticate",
			 "Eel-Tree",
			 "ORBit",
			 NULL);
	}
	
	/* Initialize gettext support */
	/* Sadly, we need this ifdef because otherwise the following
	 * lines cause empty statement warnings.
	 */
#ifdef ENABLE_NLS
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

#endif
        gnome_init ("eel", VERSION, argc, argv);

	gdk_rgb_init ();

	/* Initialize the services that we use. */
	LIBXML_TEST_VERSION
	g_atexit (xmlCleanupParser);
	g_thread_init (NULL);

	gnome_vfs_init ();

	/* Run the checks for eel twice. */
	eel_run_lib_self_checks ();
	eel_exit_if_self_checks_failed ();

	eel_run_lib_self_checks ();
	eel_exit_if_self_checks_failed ();

	gnome_vfs_shutdown ();
#endif

	return EXIT_SUCCESS;
}
