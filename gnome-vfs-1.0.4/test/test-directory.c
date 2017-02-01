/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* test-directory.c - Test program for directory reading in the GNOME
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

#include <stdio.h>

#include <gnome.h>

#include "gnome-vfs.h"


static int measure_speed = 0;

struct poptOption options[] = {
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
		NULL,
		0,
		0,
		NULL,
		0,
		NULL,
		NULL
	}
};



static void
show_result (GnomeVFSResult result, const gchar *what, const gchar *text_uri)
{
	fprintf (stderr, "%s `%s': %s\n",
		 what, text_uri, gnome_vfs_result_to_string (result));
	if (result != GNOME_VFS_OK)
		exit (1);
}

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
print_list (GList *list)
{
	GnomeVFSFileInfo *info;
	GList *node;

	if (list == NULL) {
		printf ("  (No files)\n");
		return;
	}

	for (node = list; node != NULL; node = node->next) {
		const gchar *mime_type;
	
		info = node->data;

		mime_type = gnome_vfs_file_info_get_mime_type (info);
		if (mime_type == NULL)
			mime_type = "(Unknown)";

		printf ("  File `%s'%s%s%s (%s, %s), size %ld, mode %04o\n",
			info->name,
			GNOME_VFS_FILE_INFO_SYMLINK (info) ? " [link: " : "",
			GNOME_VFS_FILE_INFO_SYMLINK (info) ? info->symlink_name : "",
			GNOME_VFS_FILE_INFO_SYMLINK (info) ? " ]" : "",
			type_to_string (info->type),
			mime_type,
			(glong) info->size,
			info->permissions);
	}
}

int
main (int argc, char **argv)
{
	GList *list;
	GnomeVFSResult result;
	GTimer *timer;
	const char **args;
	gchar *text_uri;
	poptContext popt_context;

	popt_context = poptGetContext ("test-directory", argc, argv,
				       options, 0);

	while (poptGetNextOpt (popt_context) != -1)
		;

	args = poptGetArgs (popt_context);
	if (args == NULL || args[1] != NULL) {
		fprintf (stderr, "Usage: %s [<options>] <uri>\n", argv[0]);
		return 1;
	}

	text_uri = g_strdup (args[0]);

	poptFreeContext (popt_context);

	gnome_vfs_init ();

	printf ("Loading directory...");
	fflush (stdout);

	if (measure_speed) {
		timer = g_timer_new ();
		g_timer_start (timer);
	} else {
		timer = NULL;
	}

	/* Load with no filters and without requesting any metadata.  */
	result = gnome_vfs_directory_list_load
		(&list, text_uri,
		 (GNOME_VFS_FILE_INFO_GET_MIME_TYPE
		  | GNOME_VFS_FILE_INFO_FORCE_FAST_MIME_TYPE
		  | GNOME_VFS_FILE_INFO_FOLLOW_LINKS),
		 NULL);

	if (result == GNOME_VFS_OK && measure_speed) {
		gdouble elapsed_seconds;
		guint num_entries;

		g_timer_stop (timer);
		elapsed_seconds = g_timer_elapsed (timer, NULL);
		num_entries = g_list_length (list);
		printf ("\n%.5f seconds for %d unsorted entries, %.5f entries/sec.\n",
			elapsed_seconds, num_entries,
			(double) num_entries / elapsed_seconds);
	}

	if (!measure_speed) {
		printf ("Ok\n");

		show_result (result, "load_directory", text_uri);

		printf ("Listing for `%s':\n", text_uri);
		print_list (list);
	}

	printf ("Destroying.\n");
	gnome_vfs_file_info_list_free (list);

	printf ("Done.\n");

	g_free (text_uri);

	return 0;
}
