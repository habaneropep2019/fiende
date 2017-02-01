/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-init.c - Initialization for the GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation

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

   Author: Ettore Perazzoli <ettore@gnu.org>
*/

#include <config.h>
#include "gnome-vfs-init.h"

#include "gnome-vfs.h"
#include "gnome-vfs-backend.h"
#include "gnome-vfs-private.h"
#include "gnome-vfs-ssl-private.h"
#include "gnome-vfs-mime.h"

#ifdef USING_OAF
#include <liboaf/liboaf.h>
#endif


static gboolean vfs_already_initialized = FALSE;
G_LOCK_DEFINE_STATIC (vfs_already_initialized);

static GPrivate * private_is_primary_thread;

gboolean 
gnome_vfs_init (void)
{
	gboolean retval;
	char *bogus_argv[2] = { "dummy", NULL };

	G_LOCK (vfs_already_initialized);

	if (!vfs_already_initialized) {
		if (oaf_orb_get() == NULL) {
			oaf_init (0, bogus_argv);
		}

		gnome_vfs_ssl_init ();

		retval = gnome_vfs_method_init ();
		if (retval) {
			retval = gnome_vfs_process_init ();
		}
		if (retval) {
			retval = gnome_vfs_configuration_init ();
		}
		if (retval) {
			gnome_vfs_backend_loadinit(NULL, NULL);
			retval = gnome_vfs_backend_init (TRUE);
		}
		if (retval) {
			signal (SIGPIPE, SIG_IGN);
		}

		if (g_thread_supported()) {
			private_is_primary_thread = g_private_new (NULL);
			g_private_set (private_is_primary_thread, GUINT_TO_POINTER (1));
		}
		
	} else {
		g_warning (_("GNOME VFS already initialized."));
		retval = TRUE;	/* Who cares after all.  */
	}

	vfs_already_initialized = TRUE;
	G_UNLOCK (vfs_already_initialized);

	return retval;
}


gboolean
gnome_vfs_initialized (void)
{
	gboolean out;

	G_LOCK (vfs_already_initialized);
	out = vfs_already_initialized;
	G_UNLOCK (vfs_already_initialized);
	return out;
}

void
gnome_vfs_shutdown (void)
{
	gnome_vfs_backend_shutdown ();
	gnome_vfs_mime_shutdown ();
}

void
gnome_vfs_loadinit(gpointer app, gpointer modinfo)
{
	gnome_vfs_backend_loadinit(app, modinfo);
}

void
gnome_vfs_preinit(gpointer app, gpointer modinfo)
{
}

void
gnome_vfs_postinit(gpointer app, gpointer modinfo)
{
	G_LOCK (vfs_already_initialized);

	gnome_vfs_method_init();
	gnome_vfs_process_init();
	gnome_vfs_configuration_init();
	gnome_vfs_backend_init(FALSE);
	signal(SIGPIPE, SIG_IGN);

	vfs_already_initialized = TRUE;
	G_UNLOCK (vfs_already_initialized);
}

gboolean
gnome_vfs_is_primary_thread (void)
{
	if (g_thread_supported()) {
		return GPOINTER_TO_UINT(g_private_get (private_is_primary_thread)) == 1;
	} else {
		return TRUE;
	}
}
