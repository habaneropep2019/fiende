/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* search-method.c: Gnome-VFS interface to the medusa search service

   Copyright (C) 2000 Eazel

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

   Author: Rebecca Schulman <rebecka@eazel.com>
*/

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-directory.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <gtk/gtk.h>

#include <glib.h>
#include <stdio.h>

#include <medusa-utils.h>

volatile int async_event_count = 10;

static const char *
gnome_vfs_error_to_string (GnomeVFSResult result) 
{
	return gnome_vfs_result_to_string (result);
}


static void
async_directory_load_callback (GnomeVFSAsyncHandle *handle,
			      GnomeVFSResult result,
			      GList *list,
			      guint entries_read,
			      gpointer callback_data)
{
	GnomeVFSFileInfo *info;
	GList *node;
	int i;

	printf ("Loaded entries\n");

	if (result != GNOME_VFS_OK && result != GNOME_VFS_ERROR_EOF) {
		printf ("async load failed.  Error code was %s\n", gnome_vfs_error_to_string (result));
		if (async_event_count-- == 0) {
			gtk_main_quit ();
		}
		printf ("async count is %d\n", async_event_count);
		return;
	}

	if (entries_read == 24) {
		printf ("expecting more results\n");
	}
	for (i = 0, node = list; i < entries_read && node != NULL; i++, node = node->next) {
		info = node->data;
		printf ("Received file %s\n", info->name);
		printf ("File has mime type %s\n", info->mime_type);
	}
	if (result == GNOME_VFS_ERROR_EOF) {
		printf ("That's all folks.\n");
		if (async_event_count-- == 0) {
			gtk_main_quit ();
		}
		printf ("async count is %d\n", async_event_count);
		return;
	}
}


int 
main (int argc, char **argv)
{
#if 0
	GnomeVFSResult result;
	GnomeVFSDirectoryHandle *directory_handle;
	GnomeVFSFileInfo *info;
#endif
	GnomeVFSAsyncHandle *async_handle;
	int i;


	char *escaped_uri_contents, *uri;
	
	if (argc != 2) {
		fprintf (stderr, "Usage: %s <search uri contents>\n", argv[0]);
		return 1;
	}
	printf ("Reading search results for %s. Asking gnome vfs\n", argv[1]);

	gtk_init (&argc, &argv);
	
	gnome_vfs_init ();
	
	escaped_uri_contents = gnome_vfs_escape_string (argv[1]);
	uri = g_strdup_printf ("search:%s", escaped_uri_contents);
	g_free (escaped_uri_contents);
	printf ("%s\n", uri);
#if 0	
	result = gnome_vfs_directory_open (&directory_handle,
					   uri,
					   GNOME_VFS_FILE_INFO_GET_MIME_TYPE | GNOME_VFS_FILE_INFO_FOLLOW_LINKS,
					   NULL);

	if (result != GNOME_VFS_OK) {
		printf ("Couldn't open search uri %s\nError code was %s\n", argv[1],
			gnome_vfs_error_to_string (result));
		return 1;
	}

	printf ("Testing sync reading of directory\n");
	info = gnome_vfs_file_info_new ();
	while (gnome_vfs_directory_read_next (directory_handle, info) == GNOME_VFS_OK) {
		printf ("Received next file %s\n", info->name);
		printf ("mime type is %s\n", info->mime_type);
	}
	gnome_vfs_file_info_unref (info);

	result = gnome_vfs_directory_close (directory_handle);

	if (result != GNOME_VFS_OK) {
		printf ("Couldn't close search uri %s, received error code %s\n", argv[1],
			gnome_vfs_error_to_string (result));
		return 1;
	}
#endif
	printf ("Testing async reading of directory\n");
	for (i = 0; i < 10; i++) {
	gnome_vfs_async_load_directory (&async_handle,
					uri,
					GNOME_VFS_FILE_INFO_GET_MIME_TYPE | GNOME_VFS_FILE_INFO_FOLLOW_LINKS,
					GNOME_VFS_DIRECTORY_FILTER_NONE,
					GNOME_VFS_DIRECTORY_FILTER_DEFAULT,
					NULL,
					32,
					async_directory_load_callback,
					NULL);

	}
	/* gtk_main (); */
	printf ("Done reading directories\n");
	g_free (uri);
	gnome_vfs_shutdown ();
	return 0;
}

