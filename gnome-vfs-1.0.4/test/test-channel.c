/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* test-channel.c - Test for the `open_as_channel' functionality of the GNOME
   Virtual File System.

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

   Author: Ettore Perazzoli <ettore@comm2000.it> */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <libgnorba/gnorba.h>

#include "gnome-vfs.h"

#ifdef WITH_CORBA
CORBA_Environment ev;
#endif

#define BUFFER_SIZE 4096


static gboolean io_channel_callback (GIOChannel *source,
				     GIOCondition condition,
				     gpointer data)
{
	gchar buffer[BUFFER_SIZE + 1];
	guint bytes_read;

	printf ("\n\n************ IO Channel callback!\n");

	if (condition & G_IO_IN) {
		g_io_channel_read (source, buffer, sizeof (buffer) - 1, &bytes_read);
		buffer[bytes_read] = 0;
		printf ("---> Read %d byte(s):\n%s\n\n(***END***)\n",
			bytes_read, buffer);
		fflush (stdout);
	}

	if (condition & G_IO_NVAL) {
		g_warning ("channel_callback got NVAL condition.");
		return FALSE;
	}

	if (condition & G_IO_HUP) {
		printf ("\n----- EOF -----\n");
		fflush (stdout);
		g_io_channel_close (source);
		gtk_main_quit ();
		return FALSE;
	}

	return TRUE;
}

static void open_callback (GnomeVFSAsyncHandle *handle,
			   GIOChannel *channel,
			   GnomeVFSResult result,
			   gpointer data)
{
	if (result != GNOME_VFS_OK) {
		printf ("Error opening: %s.\n",
			gnome_vfs_result_to_string (result));
		return;
	}

	printf ("Open successful, callback data `%s'.\n", (gchar *) data);
	g_io_add_watch_full (channel,
			     G_PRIORITY_HIGH,
			     G_IO_IN | G_IO_NVAL | G_IO_HUP,
			     io_channel_callback, handle,
			     NULL);

	g_io_channel_unref (channel);
}


int
main (int argc, char **argv)
{
	GnomeVFSAsyncHandle *handle;

	if (argc < 2) {
		fprintf (stderr, "Usage: %s <uri>\n", argv[0]);
		return 1;
	}

#ifdef WITH_PTHREAD
	puts ("Initializing threads...");
	g_thread_init (NULL);
#endif

#ifdef WITH_CORBA
	CORBA_exception_init (&ev);
	puts ("Initializing gnome-libs with CORBA...");
	gnome_CORBA_init ("test-vfs", "0.0", &argc, argv, 0, &ev);
#else
	puts ("Initializing gnome-libs...");
	gnome_init ("test-vfs", "0.0", argc, argv);
#endif

	puts ("Initializing gnome-vfs...");
	gnome_vfs_init ();

	printf ("Starting open for `%s'...\n", argv[1]);
	gnome_vfs_async_open_as_channel (&handle, argv[1],
					 GNOME_VFS_OPEN_READ,
					 BUFFER_SIZE,
					 open_callback,
					 "open_callback");

	puts ("GTK+ main loop running.");
	gtk_main ();

	puts ("GTK+ main loop finished.");

#ifdef WITH_CORBA
	CORBA_exception_free (&ev);
#endif

	puts ("All done");

	while (1)
		;

	return 0;
}
