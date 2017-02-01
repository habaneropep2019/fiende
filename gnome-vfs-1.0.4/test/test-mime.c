/* test-mime.c - Test for the mime type sniffing features of GNOME
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
#include "gnome-vfs-mime.h"
#include "gnome-vfs-mime-magic.h"

#include <stdio.h>
#include <string.h>

int
main (int argc, char **argv)
{
	GnomeVFSURI *uri;
	gboolean magic_only;
	gboolean suffix_only;
	gboolean dump_table;
	const char *result;
	const char *table_path;
	char *uri_string;
	struct stat tmp;

	table_path = NULL;
	magic_only = FALSE;
	dump_table = FALSE;
	suffix_only = FALSE;
	
	if (!gnome_vfs_init ()) {
		fprintf (stderr, "Cannot initialize gnome-vfs.\n");
		return 1;
	}

	if (argc == 1 || strcmp (argv[1], "--help") == 0) {
		fprintf (stderr, "Usage: %s [--magicOnly | --suffixOnly] [--dumpTable] "
			" [--loadTable <table path>] fileToCheck1 [fileToCheck2 ...] \n", *argv);
		return 1;
	}


	++argv;
	for (; *argv; argv++) {
		if (strcmp (*argv, "--magicOnly") == 0) {
			magic_only = TRUE;
		} else if (strcmp (*argv, "--suffixOnly") == 0) {
			suffix_only = TRUE;
		} else if (strcmp (*argv, "--dumpTable") == 0) {
			dump_table = TRUE;
		} else if (strcmp (*argv, "--loadTable") == 0) {
			++argv;
			if (!*argv) {
				fprintf (stderr, "Table path expected.\n");
				return 1;
			}
			table_path = *argv;
			if (stat(table_path, &tmp) != 0) {
				fprintf (stderr, "Table path %s not found.\n", table_path);
				return 1;
			}
		} else {
			break;
		}
	}


	if (table_path != NULL) {
		gnome_vfs_mime_test_get_magic_table (table_path);
	}

	if (dump_table) {
		gnome_vfs_mime_dump_magic_table ();
	}

	for (; *argv != NULL; argv++) {
		uri = gnome_vfs_uri_new (*argv);
		if (uri == NULL) {
		  uri_string = gnome_vfs_get_uri_from_local_path (*argv);
		  uri = gnome_vfs_uri_new (uri_string);
		  g_free (uri_string);
		}

		if (uri == NULL) {
		  printf ("%s is not a valid uri or file name\n", *argv);
		  return 1;
		}

		if (magic_only) {
			result = gnome_vfs_get_mime_type_from_file_data (uri);
		} else if (suffix_only) {
			result = gnome_vfs_get_mime_type_from_uri (uri);
		} else {
			result = gnome_vfs_get_mime_type (uri);
		}
	
		printf ("looks like %s is %s\n", *argv, result);
		gnome_vfs_uri_unref (uri);
	}
	
	return 0;
}
