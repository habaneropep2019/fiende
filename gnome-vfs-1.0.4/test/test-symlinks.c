/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* test-symlinks.c: verifies that symlinks are being created properly
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

   Author: Seth Nickell <snickell@stanford.edu> */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <gnome.h>

#include <orb/orbit.h>
#include <libgnorba/gnorba.h>

#include "gnome-vfs.h"

#define WITH_PTHREAD

typedef struct {
	GnomeVFSResult expected_result;
	const char *uri;
	const char *target_uri;
	const char *target_reference;
} CallbackData;

static int measure_speed = 0;
static int sort = 0;
static int items_per_notification = 1;

struct poptOption options[] = {
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
		NULL,
		0,
		0,
		NULL,
		0,
		NULL,
		NULL
	}
};

static int
deal_with_result (GnomeVFSResult result, GnomeVFSResult expected_result, 
	const char *uri, const char *target_uri, const char *target_reference, 
	gboolean unlink) 
{
	char read_buffer[1024];
	const char *write_buffer = "this is test data...we should read the same thing";
	GnomeVFSHandle *handle;
	GnomeVFSFileSize bytes_written, temp;
	GnomeVFSFileInfo info_uri, info_target;
	int return_value = 1;
	const gchar *result_string;
	GnomeVFSResult error;	
	GnomeVFSURI *real_uri, *real_uri_target;
	GnomeVFSFileInfo *info;

	real_uri = gnome_vfs_uri_new (uri);
	real_uri_target = gnome_vfs_uri_new (target_uri);

	if (result != expected_result) {
		result_string = gnome_vfs_result_to_string (result);
		printf ("creating a link from %s to %s returned %s instead of %s.\n", uri, target_reference,
			result_string, gnome_vfs_result_to_string (expected_result));
		return_value = 0;
	} else if (result == GNOME_VFS_OK) { 
		info = gnome_vfs_file_info_new();
		error = gnome_vfs_get_file_info_uri (real_uri, info, GNOME_VFS_FILE_INFO_DEFAULT);
		if ((error != GNOME_VFS_OK) || (info->type != GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK)) {
			printf ("Symlink problem: gnome_vfs_file_info returns wrong for link %s\n", uri);
		} else {
			/* our link seems to have been created correctly - lets see if its real */
			error = gnome_vfs_open_uri (&handle, real_uri_target, GNOME_VFS_OPEN_WRITE);
			if (error == GNOME_VFS_ERROR_NOT_FOUND) 
				error = gnome_vfs_create_uri (&handle, real_uri_target, GNOME_VFS_OPEN_WRITE, 0, GNOME_VFS_PERM_USER_ALL);
			if (error == GNOME_VFS_OK) {
				/* write stuff to our link location */
				error = gnome_vfs_write (handle, write_buffer, strlen (write_buffer) + 1, &bytes_written);
				error = gnome_vfs_close (handle);
				error = gnome_vfs_open_uri (&handle, real_uri, GNOME_VFS_OPEN_READ);
				if (error == GNOME_VFS_OK) {
					error = gnome_vfs_read (handle, read_buffer, bytes_written, &temp);
					read_buffer[temp] = 0;
					error = gnome_vfs_close (handle);
					if (strcmp (read_buffer, write_buffer) != 0) {
						printf ("Symlink problem: value written is not the same as the value read!\n");
						printf ("Written to %s: #%s# \nRead from link %s: #%s#\n", 
							target_uri, write_buffer, uri, read_buffer);
						return_value = 0;
					}
				}
			}
			gnome_vfs_get_file_info_uri (real_uri, &info_uri, GNOME_VFS_FILE_INFO_FOLLOW_LINKS);
			gnome_vfs_get_file_info_uri (real_uri_target, &info_target, GNOME_VFS_FILE_INFO_FOLLOW_LINKS);
			if (info_uri.inode != info_target.inode) {
				printf ("Symlink problem: link following is not working\n");
				printf ("File: %s   Link: %s\n", target_uri, uri);
			}
			gnome_vfs_get_file_info_uri (real_uri, &info_uri, GNOME_VFS_FILE_INFO_DEFAULT);
			gnome_vfs_get_file_info_uri (real_uri_target, &info_target, GNOME_VFS_FILE_INFO_DEFAULT);
			if (info_uri.inode == info_target.inode) {
				printf ("Symlink problem: link following is happening when it shouldn't be.\n");
				printf ("File: %s   Link: %s\n", target_uri, uri);
			}
		}
		gnome_vfs_file_info_unref (info);
		if (unlink) {
			gnome_vfs_unlink_from_uri (real_uri_target);
			error = gnome_vfs_unlink_from_uri (real_uri);
			if (error != GNOME_VFS_OK) {
				printf ("Problem unlinking URI %s", uri);
			}
		}
	}


	gnome_vfs_uri_unref (real_uri);
	gnome_vfs_uri_unref (real_uri_target);

	return return_value;
}

static void
create_link_callback (GnomeVFSAsyncHandle *handle,
		      GnomeVFSResult result,
		      gpointer callback_data)
{
	const char *uri, *target_uri, *target_reference;
	GnomeVFSResult expected_result;
	CallbackData *info;

	info = (CallbackData*) callback_data;
	
	uri = info->uri;
	target_uri = info->target_uri;
	expected_result = info->expected_result;
	target_reference = info->target_reference;

	deal_with_result (result, expected_result, uri, target_uri, target_reference, TRUE);	

	g_free (callback_data);
	gtk_main_quit ();
}


static int
make_link (const char *uri, const char *target_reference, const char *target_uri, GnomeVFSResult expected_result, gboolean unlink)
{
	GnomeVFSURI *real_uri, *real_uri_target;
	GnomeVFSResult result;

	int return_value = 1;

	real_uri = gnome_vfs_uri_new (uri);
	real_uri_target = gnome_vfs_uri_new (target_uri);

	result = gnome_vfs_create_symbolic_link (real_uri, target_reference);

	return_value = deal_with_result(result, expected_result, uri, target_uri, target_reference, unlink);


	
	gnome_vfs_uri_unref (real_uri);
	gnome_vfs_uri_unref (real_uri_target);

	return return_value;
}

static void
make_link_async (const char *uri, const char *target_reference, const char *target_uri, GnomeVFSResult expected_result)
{
	CallbackData *info;
	GnomeVFSAsyncHandle *handle;

	info = g_malloc (sizeof (CallbackData));
	info->uri = uri;
	info->target_uri = target_uri;
	info->expected_result = expected_result;
	info->target_reference = target_reference;

	gnome_vfs_async_create_symbolic_link (&handle, gnome_vfs_uri_new(uri), target_reference, create_link_callback, info);
}

static void
check_broken_links (const char *uri)
{
	GnomeVFSHandle *handle;
	GnomeVFSResult error;
	GnomeVFSURI *real_uri, *real_uri_target;

	real_uri = gnome_vfs_uri_new (uri);
	real_uri_target = gnome_vfs_uri_new ("file:///tmp/deadlink");

	gnome_vfs_unlink_from_uri (real_uri_target);
	gnome_vfs_create_symbolic_link (real_uri, "deadlink");

	error = gnome_vfs_open_uri (&handle, real_uri, GNOME_VFS_OPEN_READ);
	if (error != GNOME_VFS_ERROR_NOT_FOUND) {
		printf ("GNOME_VFS_BROKEN_SYMLINK not returned open attempting to open a broken symlink.\n");
		printf ("Value returned: %d\n", error);
	}

	gnome_vfs_unlink_from_uri (real_uri);
	gnome_vfs_unlink_from_uri (real_uri_target);

	gnome_vfs_uri_unref (real_uri);
	gnome_vfs_uri_unref (real_uri_target);
}


int
main (int argc, char **argv)
{
	GnomeVFSURI *directory, *file_to_delete;

	poptContext popt_context;
#ifdef WITH_CORBA
	CORBA_Environment ev;
#endif

#ifdef WITH_PTHREAD
	g_thread_init (NULL);
#endif

#ifdef WITH_CORBA
	CORBA_exception_init (&ev);
	gnome_CORBA_init_with_popt_table ("test-vfs", "0.0", &argc, argv,
					  options, 0, &popt_context, 0, &ev);
#else
	gnome_init_with_popt_table ("test-vfs", "0.0", argc, argv,
	  options, 0, &popt_context);
#endif


	if (argc != 2) {
		fprintf (stderr, "Usage: %s <directory>\n", argv[0]);
		return 1;
	}

	gnome_vfs_init ();
	directory = gnome_vfs_uri_new ("file:///tmp/tmp");

	gnome_vfs_make_directory_for_uri (directory, GNOME_VFS_PERM_USER_ALL);

	make_link ("file:///tmp/link_to_ditz", "file:///tmp/ditz", "file:///tmp/ditz", GNOME_VFS_OK, TRUE);
	make_link ("file:///tmp/link_to_ditz_relative", "ditz", "file:///tmp/ditz", GNOME_VFS_OK, TRUE);
	make_link ("file:///tmp/tmp/link_to_ditz", "../ditz", "file:///tmp/ditz", GNOME_VFS_OK, FALSE);
	make_link ("file:///tmp/link_to_link", "tmp/link_to_ditz", "file:///tmp/tmp/link_to_ditz", GNOME_VFS_OK, TRUE);
				
	gnome_vfs_remove_directory_from_uri (directory);
	gnome_vfs_uri_unref (directory);

	file_to_delete = gnome_vfs_uri_new ("file:///tmp/ditz");
	gnome_vfs_unlink_from_uri (file_to_delete);
	gnome_vfs_uri_unref (file_to_delete);

	check_broken_links("file:///tmp/link");

	make_link ("file:///tmp/link_to_ditz_offfs", "http://www.a.com/ditz", "http://www.a.com/ditz", GNOME_VFS_ERROR_NOT_SUPPORTED, TRUE);
	make_link ("http://www.eazel.com/link_to_ditz", "file:///tmp/ditz", "file:///tmp/ditz", GNOME_VFS_ERROR_NOT_SUPPORTED, TRUE);
	make_link ("http://www.a.com/link_to_ditz_relative", "ditz", "http://www.a.com/ditz", GNOME_VFS_ERROR_NOT_SUPPORTED, TRUE);

	make_link_async ("file:///tmp/async_link", "file:///tmp/link", "file:///tmp/link", GNOME_VFS_OK);

	gtk_main ();

#ifdef WITH_CORBA
	CORBA_exception_free (&ev);
#endif


	return 0;
}
