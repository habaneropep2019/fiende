/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* test-xfer.c - Test program for the xfer functions in the GNOME Virtual File
   System.

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

static int recursive = 0;
static int remove_source = 0;

struct poptOption options[] = {
	POPT_AUTOHELP
	{
		"recursive",
		'r',
		POPT_ARG_NONE,
		&recursive,
		0,
		"Copy directories recursively",
		NULL
	},
	{
		"delete-source",
		'd',
		POPT_ARG_NONE,
		&remove_source,
		0,
		"Delete source files",
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
show_result (GnomeVFSResult result, const gchar *what)
{
	fprintf (stderr, "%s: %s\n", what, gnome_vfs_result_to_string (result));
	if (result != GNOME_VFS_OK)
		exit (1);
}

static gint
xfer_progress_callback (GnomeVFSXferProgressInfo *info,
			gpointer data)
{
	switch (info->status) {
	case GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR:
		printf ("VFS Error: %s\n",
			gnome_vfs_result_to_string (info->vfs_status));
		exit (1);
		break;
	case GNOME_VFS_XFER_PROGRESS_STATUS_OVERWRITE:
		printf ("Overwriting `%s' with `%s'\n",
			info->target_name, info->source_name);
		exit (1);
		break;
	case GNOME_VFS_XFER_PROGRESS_STATUS_OK:
		printf ("Status: OK\n");
		switch (info->phase) {
		case GNOME_VFS_XFER_PHASE_INITIAL:
			printf ("Initial phase\n");
			return TRUE;
		case GNOME_VFS_XFER_PHASE_COLLECTING:
			printf ("Collecting file list\n");
			return TRUE;
		case GNOME_VFS_XFER_PHASE_READYTOGO:
			printf ("Ready to go!\n");
			return TRUE;
		case GNOME_VFS_XFER_PHASE_OPENSOURCE:
			printf ("Opening source\n");
			return TRUE;
		case GNOME_VFS_XFER_PHASE_OPENTARGET:
			printf ("Opening target\n");
			return TRUE;
		case GNOME_VFS_XFER_PHASE_COPYING:
			printf ("Transferring `%s' to `%s' (file %ld/%ld, byte %ld/%ld in file, "
				"%" GNOME_VFS_SIZE_FORMAT_STR "/%" GNOME_VFS_SIZE_FORMAT_STR " total)\n",
				info->source_name,
				info->target_name,
				info->file_index,
				info->files_total,
				(glong) info->bytes_copied,
				(glong) info->file_size,
				info->total_bytes_copied,
				info->bytes_total);
			return TRUE;
		case GNOME_VFS_XFER_PHASE_CLOSESOURCE:
			printf ("Closing source\n");
			return TRUE;
		case GNOME_VFS_XFER_PHASE_CLOSETARGET:
			printf ("Closing target\n");
			return TRUE;
		case GNOME_VFS_XFER_PHASE_FILECOMPLETED:
			printf ("Done with `%s' -> `%s', going next\n",
				info->source_name, info->target_name);
			return TRUE;
		case GNOME_VFS_XFER_PHASE_COMPLETED:
			printf ("All done.\n");
			return TRUE;
		default:
			printf ("Unexpected phase %d\n", info->phase);
			return TRUE; /* keep going anyway */
		}
	case GNOME_VFS_XFER_PROGRESS_STATUS_DUPLICATE:
		break;
	}

	printf ("Boh!\n");
	return FALSE;
}

int
main (int argc, char **argv)
{
	const char **args;
	poptContext popt_context;
	GnomeVFSURI *src_uri, *dest_uri;
	GList *src_uri_list, *dest_uri_list;
	GnomeVFSResult result;
	GnomeVFSXferOptions xfer_options;

	if (! gnome_vfs_init ()) {
		fprintf (stderr,
			 "Cannot initialize the GNOME Virtual File System.\n");
		return 1;
	}

	popt_context = poptGetContext ("test-directory", argc, argv,
				       options, 0);

	while (poptGetNextOpt (popt_context) != -1)
		;

	args = poptGetArgs (popt_context);
	if (args == NULL || args[1] == NULL || args[2] != NULL) {
		fprintf (stderr, "Usage: %s [<options>] <src> <target>\n",
			 argv[0]);
		return 1;
	}

	src_uri = gnome_vfs_uri_new (args[0]);
	if (src_uri == NULL) {
		fprintf (stderr, "%s: invalid URI\n", args[0]);
		return 1;
	}
	dest_uri = gnome_vfs_uri_new (args[1]);
	if (dest_uri == NULL) {
		fprintf (stderr, "%s: invalid URI\n", args[1]);
		return 1;
	}

	poptFreeContext (popt_context);


	xfer_options = 0;
	if (recursive) {
		fprintf (stderr, "Warning: Recursive xfer of directories.\n");
		xfer_options |= GNOME_VFS_XFER_RECURSIVE;
	}
	if (remove_source) {
		fprintf (stderr, "Warning: Removing source files.\n");
		xfer_options |= GNOME_VFS_XFER_REMOVESOURCE;
	}

	src_uri_list = g_list_append (NULL, src_uri);
	dest_uri_list = g_list_append (NULL, dest_uri);
	result = gnome_vfs_xfer_uri_list (src_uri_list, dest_uri_list,
					  xfer_options,
					  GNOME_VFS_XFER_ERROR_MODE_QUERY,
					  GNOME_VFS_XFER_OVERWRITE_MODE_QUERY,
					  xfer_progress_callback,
					  NULL);

	show_result (result, "gnome_vfs_xfer");

	gnome_vfs_uri_list_free (src_uri_list);
	gnome_vfs_uri_list_free (dest_uri_list);

	return 0;
}
