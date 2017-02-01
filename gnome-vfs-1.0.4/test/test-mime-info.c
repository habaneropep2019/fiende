/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* test-mime.c - Test for the mime handler detection features of the GNOME
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

   Author: Maciej Stachowiak <mjs@eazel.com>
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnome-vfs.h"
#include "gnome-vfs-mime-handlers.h"
#include "gnome-vfs-mime-info.h"


#include <stdio.h>
#include <stdlib.h>


int
main (int argc, char **argv)
{
	const char *value;

	gnome_vfs_init ();

	/* test reading of gnome-vfs.keys file */
	value = gnome_vfs_mime_get_value ("x-directory/normal", "description");
	if (value == NULL || strcmp (value, "folder") != 0) {
		printf ("Error: get_value failed on \"description\".\n");
		exit (1);
	}
	value = gnome_vfs_mime_get_value ("x-directory/normal", "default_action_type");
	if (value == NULL || strcmp (value, "component") != 0) {
		printf ("Error: get_value failed on \"default_action_type\".\n");
		exit (1);
	}
	value = gnome_vfs_mime_get_value ("x-directory/normal", "default_component_iid");
	if (value == NULL || strcmp (value, "OAFIID:nautilus_file_manager_icon_view:42681b21-d5ca-4837-87d2-394d88ecc058") != 0) {
		printf ("Error: get_value failed on \"default_component_iid\".\n");
		exit (1);
	}
	value = gnome_vfs_mime_get_value ("x-directory/normal", "short_list_component_iids_for_novice_user_level");
	if (value == NULL || strcmp (value, "OAFIID:nautilus_file_manager_icon_view:42681b21-d5ca-4837-87d2-394d88ecc058,"
		    "OAFIID:nautilus_file_manager_list_view:521e489d-0662-4ad7-ac3a-832deabe111c,"
		    "OAFIID:nautilus_music_view:9456b5d2-60a8-407f-a56e-d561e1821391") != 0) {
		printf ("Error: get_value failed on \"short_list_component_iids_for_novice_user_level\".\n");
		exit (1);
	}
	value = gnome_vfs_mime_get_value ("x-directory/normal", "short_list_component_iids_for_intermediate_user_level");
	if (value == NULL || strcmp (value, "OAFIID:nautilus_file_manager_icon_view:42681b21-d5ca-4837-87d2-394d88ecc058,"
		    "OAFIID:nautilus_file_manager_list_view:521e489d-0662-4ad7-ac3a-832deabe111c,"
		    "OAFIID:nautilus_music_view:9456b5d2-60a8-407f-a56e-d561e1821391") != 0) {
		printf ("Error: get_value failed on \"short_list_component_iids_for_intermediate_user_level\".\n");
		exit (1);
	}
	value = gnome_vfs_mime_get_value ("x-directory/normal", "short_list_component_iids_for_advanced_user_level");
	if (value == NULL || strcmp (value, "OAFIID:nautilus_file_manager_icon_view:42681b21-d5ca-4837-87d2-394d88ecc058,"
		    "OAFIID:nautilus_file_manager_list_view:521e489d-0662-4ad7-ac3a-832deabe111c,"
		    "OAFIID:nautilus_music_view:9456b5d2-60a8-407f-a56e-d561e1821391") != 0) {
		printf ("Error: get_value failed on \"short_list_component_iids_for_advanced_user_level\".\n");
		exit (1);
	}
	value = gnome_vfs_mime_get_description ("application/mime-type-test");
	if (value == NULL || strcmp (value, "mon test a moi") != 0) {
		printf ("Error: description failed on \"application/mime-type-test\".\n");
		exit (1);
	}


	/* test reading of gnome-vfs.mime file */
	{
		GList *list, *temp;
		char *extensions[] = {"ps", "eps"};
		char **extension;
		char *ext;
		list = gnome_vfs_mime_get_extensions_list ("application/postscript");
		if (list == NULL) {
			printf ("Error: get_extensions_list failed on \"application/postscript\" list NULL.\n");
			exit (1);
		}
		for (temp = list, extension = extensions; temp != NULL; temp = temp->next, extension++) {
			if (strcmp (*extension, (char *)temp->data) != 0) {
				printf ("Error: get_extensions_list failed on \"application/postscript\".\n");
				printf ("Wrong value received was %s instead of %s\n", (char *) temp->data, *extension);
				exit (1);
			}
		}
		gnome_vfs_mime_extensions_list_free (list);
		
		ext = gnome_vfs_mime_get_extensions_string ("application/postscript");
		if (ext == NULL || strcmp (ext, "eps ps") != 0) {
			printf ("Error: get_extensions_string failed on \"application/postscript\".\n");
			exit (1);
		}
		ext = gnome_vfs_mime_get_extensions_pretty_string ("application/postscript");
		if (ext == NULL || strcmp (ext, ".ps, .eps") != 0) {
			printf ("Error: get_extensions_pretty_string failed on \"application/postscript\".\n");
			exit (1);
		}
	}

	/* test writing the users' user.keys file. It should overide the system default */
	{
		const char *value;
		gnome_vfs_mime_set_value ("application/postscript",
					      "foo",
					      "bar");
		value = gnome_vfs_mime_get_value ("application/postscript",
						      "foo");
		if (value == NULL || strcmp (value, "bar") != 0) {
			printf ("%s\n", value);
			printf ("Error: set_value failed on \"application/postscript\" / \"foo\".\n");
			exit (1);			
		}

		/* test freeze/thaw for this stuff */
		gnome_vfs_mime_freeze ();
		gnome_vfs_mime_set_value ("application/postscript",
					      "bar",
					      "foo");
		value = gnome_vfs_mime_get_value ("application/postscript",
						      "bar");
		if (value == NULL || strcmp (value, "foo") != 0) {
			printf ("%s\n", value);
			printf ("Error: set_value failed on \"application/postscript\" / \"bar\".\n");
			exit (1);			
		}
		gnome_vfs_mime_thaw ();
		value = gnome_vfs_mime_get_value ("application/postscript",
						      "bar");
		if (value == NULL || strcmp (value, "foo") != 0) {
			printf ("%s\n", value);
			printf ("Error: set_value failed on \"application/postscript\" / \"bar\".\n");
			exit (1);			
		}
		
		/* try to overide system settings.*/
		gnome_vfs_mime_set_value ("application/postscript",
					      "description",
					      "bar");
		value = gnome_vfs_mime_get_value ("application/postscript",
						      "description");
		if (value == NULL || strcmp (value, "bar") != 0) {
			printf ("%s\n", value);
			printf ("Error: set_value failed on \"application/postscript\" / \"description\".\n");
			exit (1);			
		}

	}


	/* test to try to modify the user.mime file */
	{
		char *value;
		gnome_vfs_mime_set_registered_type_key ("application/postscript", "ext", "foo");

		value = gnome_vfs_mime_get_extensions_string ("application/postscript");

		if (strstr (value, "foo") == NULL) {
			printf ("Error: cannot set mime type new extension.\n");
			exit (1);						
		}
		if (strstr (value, "ps") == NULL) {
			printf ("Error: deleted default mime type extension.\n");
			exit (1);						
		}
		gnome_vfs_mime_set_registered_type_key ("application/postscript", "ext", "");
	}

	{
		GList *mime_types_list;
		char *deleted_mime_type;
		GList *temp;
		gboolean found_mime_type;

		mime_types_list = gnome_vfs_get_registered_mime_types ();

		if (mime_types_list->data != NULL) {
			/* try to delete a mime type */
			deleted_mime_type = mime_types_list->data;
			gnome_vfs_mime_registered_mime_type_delete (deleted_mime_type);
			gnome_vfs_mime_registered_mime_type_list_free (mime_types_list);
			mime_types_list = gnome_vfs_get_registered_mime_types ();
			for (temp = mime_types_list; temp != NULL; temp = temp->next) {
				if (strcmp (deleted_mime_type, (char *)temp->data) == 0) {
					printf ("Error: could not delete mime type.\n");
					exit (1);
				}
			}
			gnome_vfs_mime_registered_mime_type_list_free (mime_types_list);
			
			/* reset to system defaults */
			gnome_vfs_mime_reset ();

			/* try to find the original mime type again */
			mime_types_list = gnome_vfs_get_registered_mime_types ();
			found_mime_type = FALSE;
			for (temp = mime_types_list; temp != NULL; temp = temp->next) {
				if (strcmp (deleted_mime_type, (char *)temp->data) == 0) {
					found_mime_type = TRUE;
					break;
				}
			}
			if (!found_mime_type) {
				printf ("Error: lost a mime type.\n");
				exit (1);
			}
			
		}
		
	}


	/* do hard stuff on the API */

	printf ("all mime-info-related tests succeeded\n");

	return 0;
}



