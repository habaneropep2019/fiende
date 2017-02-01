/* test-mime.c - Test for the gnome_vfs_find_directory call
   Virtual File System Library

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

   Author: Pavel Cisler <pavel@eazel.com>
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnome-vfs.h"
#include "gnome-vfs-uri.h"
#include "gnome-vfs-find-directory.h"

#include <stdio.h>
#include <string.h>

int
main (int argc, char **argv)
{
	GnomeVFSURI *uri;
	GnomeVFSURI *result;
	GnomeVFSResult error;
	char *path;
	gboolean create;

	create = FALSE;

	if (!gnome_vfs_init ()) {
		fprintf (stderr, "Cannot initialize gnome-vfs.\n");
		return 1;
	}

	if (argc == 1) {
		fprintf (stderr, "Usage: %s [-create] near_uri \n", *argv);
		return 1;
	}


	++argv;

	if (strcmp (*argv, "-create") == 0) {
		create = TRUE;
		++argv;
	}

	uri = gnome_vfs_uri_new (*argv);
	error = gnome_vfs_find_directory (uri, GNOME_VFS_DIRECTORY_KIND_TRASH, &result, create, 
		TRUE, 0777);
	if (error == GNOME_VFS_OK) {
		path = gnome_vfs_uri_to_string (result, GNOME_VFS_URI_HIDE_NONE);
		g_print ("found trash at %s\n", path);
		g_free (path);
		error = gnome_vfs_find_directory (uri, GNOME_VFS_DIRECTORY_KIND_TRASH, 
			&result, FALSE, FALSE, 0777);
		if (error == GNOME_VFS_OK) {
			path = gnome_vfs_uri_to_string (result, GNOME_VFS_URI_HIDE_NONE);
			g_print ("found it again in a cached entry at %s\n", path);
			g_free (path);
		} else {
			g_print ("error %s finding cached trash entry near %s\n", gnome_vfs_result_to_string (error),
				*argv);
		}
	} else {
		g_print ("error %s finding trash near %s\n", gnome_vfs_result_to_string (error),
			*argv);
	}
	
	return 0;
}
