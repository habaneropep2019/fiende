/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* test-async-directory.c - Test program for asynchronous directory
   reading with the GNOME Virtual File System.

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

#include <stdio.h>

#include <gnome.h>

#include <orb/orbit.h>
#include <libgnorba/gnorba.h>

#include "gnome-vfs.h"

static int measure_speed = 0;
static int sort = 0;
static int items_per_notification = 1;
static int read_files = 0;

static struct poptOption options[] = {
	{
		"chunk-size",
		'c',
		POPT_ARG_INT,
		&items_per_notification,
		0,
		"Number of items to send for every notification",
	        "NUM_ITEMS"
	},
	{
		"measure-speed",
		'm',
		POPT_ARG_NONE,
		&measure_speed,
		0,
		"Measure speed without displaying anything",
		NULL
	},
	{
		"sort",
		's',
		POPT_ARG_NONE,
		&sort,
		0,
		"Sort entries",
		NULL
	},
	{
		"read-files",
		'r',
		POPT_ARG_NONE,
		&read_files,
		0,
		"Test file reading",
		NULL
	},
	{
		NULL,
		0,
		0,
		NULL,
		0,
		NULL,
		NULL
	}
};

static const gchar *
type_to_string (GnomeVFSFileType type)
{
	switch (type) {
	case GNOME_VFS_FILE_TYPE_UNKNOWN:
		return "Unknown";
	case GNOME_VFS_FILE_TYPE_REGULAR:
		return "Regular";
	case GNOME_VFS_FILE_TYPE_DIRECTORY:
		return "Directory";
	case GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK:
		return "Symbolic Link";
	case GNOME_VFS_FILE_TYPE_FIFO:
		return "FIFO";
	case GNOME_VFS_FILE_TYPE_SOCKET:
		return "Socket";
	case GNOME_VFS_FILE_TYPE_CHARACTER_DEVICE:
		return "Character device";
	case GNOME_VFS_FILE_TYPE_BLOCK_DEVICE:
		return "Block device";
	default:
		return "???";
	}
}

static void
test_read_file_close_callback (GnomeVFSAsyncHandle *handle,
			  GnomeVFSResult result,
			  gpointer callback_data)
{
}


static void
test_read_file_succeeded (GnomeVFSAsyncHandle *handle)
{
	gnome_vfs_async_close (handle,
			       test_read_file_close_callback,
			       NULL);
}

static void
test_read_file_failed (GnomeVFSAsyncHandle *handle, GnomeVFSResult result)
{
	gnome_vfs_async_close (handle,
			       test_read_file_close_callback,
			       NULL);
}

/* A read is complete, so we might or might not be done. */
static void
test_read_file_read_callback (GnomeVFSAsyncHandle *handle,
				GnomeVFSResult result,
				gpointer buffer,
				GnomeVFSFileSize bytes_requested,
				GnomeVFSFileSize bytes_read,
				gpointer callback_data)
{
	/* Check for a failure. */
	if (result != GNOME_VFS_OK && result != GNOME_VFS_ERROR_EOF) {
		test_read_file_failed (handle, result);
		return;
	}

	/* If at the end of the file, we win! */
	test_read_file_succeeded (handle);
}

char buffer[256];

/* Start reading a chunk. */
static void
test_read_file_read_chunk (GnomeVFSAsyncHandle *handle)
{
	gnome_vfs_async_read (handle,
			      buffer,
			      10,
			      test_read_file_read_callback,
			      handle);
}

/* Once the open is finished, read a first chunk. */
static void
test_read_file_open_callback (GnomeVFSAsyncHandle *handle,
			 GnomeVFSResult result,
			 gpointer callback_data)
{
	if (result != GNOME_VFS_OK) {
		test_read_file_failed (handle, result);
		return;
	}

	test_read_file_read_chunk (handle);
}

/* Set up the read handle and start reading. */
static GnomeVFSAsyncHandle *
test_read_file_async (GnomeVFSURI *uri)
{
	GnomeVFSAsyncHandle *result;
	
	gnome_vfs_async_open_uri (&result,
			      uri,
			      GNOME_VFS_OPEN_READ,
			      test_read_file_open_callback,
			      NULL);

	return result;
}

typedef struct {
	const char *parent_uri;
	int num_entries_read;
} CallbackData;

volatile int async_task_counter; 

static void
directory_load_callback (GnomeVFSAsyncHandle *handle,
			 GnomeVFSResult result,
			 GList *list,
			 guint entries_read,
			 gpointer callback_data)
{
	CallbackData *data;
	GnomeVFSFileInfo *info;
	GnomeVFSURI *parent_uri;
	GnomeVFSURI *uri;
	GList *node;
	guint i;

	data = (CallbackData *)callback_data;


	if (!measure_speed) {
		printf ("Directory load callback: %s, %d entries, callback_data `%s'\n",
			gnome_vfs_result_to_string (result),
			entries_read,
			(gchar *) data->parent_uri);
	}

	parent_uri = gnome_vfs_uri_new (data->parent_uri);
	for (i = 0, node = list; i < entries_read && node != NULL; i++, node = node->next) {
		info = node->data;
		if (!measure_speed) {
			printf ("  File `%s'%s (%s, %s), "
				"size %"GNOME_VFS_SIZE_FORMAT_STR", mode %04o\n",
				info->name,
				(info->flags & GNOME_VFS_FILE_FLAGS_SYMLINK) ? " [link]" : "",
				type_to_string (info->type),
				gnome_vfs_file_info_get_mime_type (info),
				info->size, info->permissions);
			fflush (stdout);
		}
		if (read_files) {
			if ((info->type & GNOME_VFS_FILE_TYPE_REGULAR) != 0) {
				uri = gnome_vfs_uri_append_file_name (parent_uri, info->name);
				test_read_file_async (uri);
				gnome_vfs_uri_unref (uri);
			}
			if (!measure_speed) {
				printf ("reading a bit of %s\n", info->name);
			}
		}
	}

	
	data->num_entries_read += entries_read;

	gnome_vfs_uri_unref (parent_uri);
	if (result != GNOME_VFS_OK) {
		if (--async_task_counter == 0) {
			gtk_main_quit ();
		}
	}
}

int
main (int argc, char **argv)
{
	GnomeVFSAsyncHandle *handle;
	poptContext popt_context;
	const char **args;
	gchar *text_uri;
	GTimer *timer;
#ifdef WITH_CORBA
	CORBA_Environment ev;
#endif
	CallbackData callback_data;

#ifdef WITH_PTHREAD
	puts ("Initializing threads...");
	g_thread_init (NULL);
#endif

#ifdef WITH_CORBA
	CORBA_exception_init (&ev);
	puts ("Initializing gnome-libs with CORBA...");
	gnome_CORBA_init_with_popt_table ("test-vfs", "0.0", &argc, argv,
					  options, 0, &popt_context, 0, &ev);
#else
	puts ("Initializing gnome-libs...");
	gnome_init_with_popt_table ("test-vfs", "0.0", argc, argv,
				    options, 0, &popt_context);
#endif

	args = poptGetArgs (popt_context);
	if (args == NULL || args[1] != NULL) {
		fprintf (stderr, "Usage: %s [<options>] <uri>\n", argv[0]);
		return 1;
	}

	text_uri = g_strdup (args[0]);
	poptFreeContext (popt_context);

	puts ("Initializing gnome-vfs...");
	gnome_vfs_init ();

	printf ("%d item(s) per notification\n", items_per_notification);

	if (measure_speed) {
		timer = g_timer_new ();
		g_timer_start (timer);
	} else {
		timer = NULL;
	}

	callback_data.num_entries_read = 0;
	callback_data.parent_uri = text_uri;
	async_task_counter = 1;
	gnome_vfs_async_load_directory
		(&handle,
		 text_uri,
		 (GNOME_VFS_FILE_INFO_GET_MIME_TYPE | GNOME_VFS_FILE_INFO_FOLLOW_LINKS),
		 GNOME_VFS_DIRECTORY_FILTER_NONE,
		 0, NULL,
		 items_per_notification,
		 directory_load_callback,
		 &callback_data);

	if (!measure_speed)
		puts ("GTK+ main loop running.");

	gtk_main ();

	if (measure_speed) {
		gdouble elapsed_seconds;

		g_timer_stop (timer);
		elapsed_seconds = g_timer_elapsed (timer, NULL);
		printf ("%.5f seconds for %d entries, %.5f entries/sec.\n",
			elapsed_seconds, callback_data.num_entries_read,
			(double) callback_data.num_entries_read / elapsed_seconds);
	}

	if (!measure_speed)
		puts ("GTK+ main loop finished."); fflush (stdout);

#ifdef WITH_CORBA
	CORBA_exception_free (&ev);
#endif

	puts ("All done");

	gnome_vfs_shutdown ();

	return 0;
}
