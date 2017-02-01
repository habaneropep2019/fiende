/* metadata.c - Test of metadata functions.

   Copyright (C) 1998, 1999 Tom Tromey

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
   Boston, MA 02111-1307, USA.  */

#include <gnome.h>
#include <unistd.h>
#include <string.h>

/* These variable are used for testing purposes only.  */
extern char *gnome_metadata_db_file_name;
extern char *gnome_metadata_app_dir;

#define Z "zardoz"
#define Y "yadda"
#define X "xenophobe"
#define W "widdershins"

#define PLAIN "text/plain"
#define FANCY "text/fancy"

struct {
        char *key;
        char *data;
} keys [] = {
        { "icon-filename", "/gnome/share/pixmaps/blabla.png" },
        { "icon-position", "/gnome/share/yadayuadayada" },
        { NULL, NULL }
};

int
main (int argc, char *argv[])
{
	char **vec;
	int i, len;
	char *buffer;

	gnome_metadata_db_file_name = "test.db";
	unlink (gnome_metadata_db_file_name);

	gnome_metadata_app_dir = g_strconcat (getenv ("srcdir"),
						 "/data", NULL);

	/* Populate the database.  */
	gnome_metadata_type_add (PLAIN, "frob-key", strlen (W) + 1, W);
	gnome_metadata_type_add (FANCY, "frob-key", strlen (X) + 1, X);

	gnome_metadata_regex_add ("\\.txt$", "type",
				  strlen (PLAIN) + 1, PLAIN);
	gnome_metadata_regex_add ("\\.joe$", "type",
				  strlen (FANCY) + 1, FANCY);
	gnome_metadata_regex_add ("f.*\\.joe$", "frob-key",
				  strlen (Y) + 1, Y);

	gnome_metadata_set ("foo.txt", "frob-key", strlen (Y) + 1, Y);
	gnome_metadata_set ("foo.txt", "slob-key", strlen (Z) + 1, Z);

	/* Now do some queries.  */
	vec = gnome_metadata_list ("foo.txt");
	/* We don't print elements since ordering is indeterminate.
	   FIXME Should just use qsort.  */
	for (i = 0; vec[i]; ++i)
		;
	printf ("vec has %d elements\n", i);
	g_strfreev (vec);

	gnome_metadata_get ("alpha.txt", "frob-key", &len, &buffer);
	printf ("alpha.txt has %s\n", buffer);
	g_free (buffer);

	gnome_metadata_get ("foo.txt", "frob-key", &len, &buffer);
	printf ("foo.txt has %s\n", buffer);
	g_free (buffer);

	gnome_metadata_get ("foo.joe", "frob-key", &len, &buffer);
	printf ("foo.joe has %s\n", buffer);
	g_free (buffer);

	gnome_metadata_get ("zoe.joe", "frob-key", &len, &buffer);
	printf ("zoe.joe has %s\n", buffer);
	g_free (buffer);

	gnome_metadata_remove ("foo.txt", "slob-key");
	vec = gnome_metadata_list ("foo.txt");
	for (i = 0; vec[i]; ++i)
		;
	printf ("now vec has %d elements\n", i);
	g_strfreev (vec);

	gnome_metadata_copy ("foo.txt", "bar");
	gnome_metadata_get ("bar", "frob-key", &len, &buffer);
	printf ("bar has %s\n", buffer);
	g_free (buffer);

	gnome_metadata_rename ("foo.txt", "delta");
	gnome_metadata_get ("foo.txt", "frob-key", &len, &buffer);
	printf ("now foo.txt has %s\n", buffer);
	g_free (buffer);

	/* The following results should be pulled out of the metadata
	   files.  */
	gnome_metadata_get ("foo.spud", "pigname", &len, &buffer);
	printf ("foo.spud has pig name %s\n", buffer);
	g_free (buffer);

	gnome_metadata_get ("foo.spud", "dogname", &len, &buffer);
	printf ("foo.spud has dog name %s\n", buffer);
	g_free (buffer);

        for (i = 0; keys [i].key || keys [i].data; i++){
                gnome_metadata_set (
                        "/tmp/a",
                        keys [i].key,
                        strlen (keys [i].data)+1,
                        keys [i].data);
        }
        gnome_metadata_rename ("/tmp/a", "/tmp/b");

	return 0;
}
