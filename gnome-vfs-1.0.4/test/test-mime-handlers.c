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

#include <stdio.h>
#include <string.h>

static void
append_comma_and_scheme (gpointer scheme,
			 gpointer user_data)
{
	char **string;

	string = (char **) user_data;
	if (strlen (*string) > 0) {
		*string = g_strconcat (*string, ", ", scheme, NULL);
	}
	else {
		*string = g_strdup (scheme);
	}
}


static char *
format_supported_uri_schemes_for_display (GList *supported_uri_schemes)
{
	char *string;

	string = g_strdup ("");
	g_list_foreach (supported_uri_schemes,
			append_comma_and_scheme,
			&string);
	return string;
}

static const char *
format_mime_application_argument_type_for_display (GnomeVFSMimeApplicationArgumentType argument_type)
{
	switch (argument_type) {
	case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS:
		return "Always";
		break;
	case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_PATHS:
		return "No";
	case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS_FOR_NON_FILES:
		return "For non-files";
	default:
		g_assert_not_reached ();
	}
	return NULL;
}

static void
print_application (GnomeVFSMimeApplication *application)
{
        if (application == NULL) {
	        puts ("(none)");
	} else {
	        printf ("name: %s\ncommand: %s\ncan_open_multiple_files: %s\nexpects_uris: %s\nsupported_uri_schemes: %s\nrequires_terminal: %s\n", 
			application->name, application->command, 
			(application->can_open_multiple_files ? "TRUE" : "FALSE"),
			format_mime_application_argument_type_for_display (application->expects_uris),
			format_supported_uri_schemes_for_display (application->supported_uri_schemes),
			(application->requires_terminal ? "TRUE" : "FALSE"));
	}
}

static void
print_component (OAF_ServerInfo *component)
{
        if (component == NULL) {
	        puts ("(none)");
	} else {
	        printf ("iid: %s\n", component->iid);
	}
}

static void
print_action (GnomeVFSMimeAction *action)
{
        if (action == NULL) {
	        puts ("(none)");
	} else {
	  if (action->action_type == GNOME_VFS_MIME_ACTION_TYPE_APPLICATION) {
  	        puts ("type: application");
		print_application (action->action.application);
	  } else {
  	        puts ("type: component");
		print_component (action->action.component);
	  }
	}
}


static void 
print_component_list (GList *components)
{
       GList *p;
       if (components == NULL) {
	 puts ("(none)");
       } else {
	 for (p = components; p != NULL; p = p->next) {
	   print_component (p->data);
	   puts ("------");
	 }
	 
       }
}

static void 
print_application_list (GList *applications)
{
       GList *p;
       if (applications == NULL) {
	 puts ("(none)");
       } else {
	 for (p = applications; p != NULL; p = p->next) {
	   print_application (p->data);
	   puts ("------");
	 }
	 
       }
}


int
main (int argc, char **argv)
{
        const char *type;  
	GnomeVFSMimeApplication *default_application;
	OAF_ServerInfo *default_component;
	GnomeVFSMimeAction *default_action;
	const char *description;
	GList *all_components;
	GList *all_applications;
	GList *short_list_components;
	GList *short_list_applications;

	oaf_init (argc, argv);
	gnome_vfs_init ();

	if (argc != 2) {
		fprintf (stderr, "Usage: %s mime_type\n", *argv);
		return 1;
	}

	type = argv[1];
	
	description = gnome_vfs_mime_get_description (type);
	printf ("Description: %s\n", description);


	default_action = gnome_vfs_mime_get_default_action (type);
	puts ("Default Action");
	print_action (default_action);
	puts ("");

	default_application = gnome_vfs_mime_get_default_application (type);
	puts("Default Application");
	print_application (default_application);
	puts ("");
		
	default_component = gnome_vfs_mime_get_default_component (type);
	puts("Default Component");
	print_component (default_component);
	puts ("");

	short_list_applications = gnome_vfs_mime_get_short_list_applications (type); 
	puts("Short List Applications");
	print_application_list (short_list_applications);
	puts ("");

	short_list_components = gnome_vfs_mime_get_short_list_components (type); 
	puts("Short List Components");
	print_component_list (short_list_components);
	puts ("");

	all_applications = gnome_vfs_mime_get_all_applications (type); 
	puts("All Applications");
	print_application_list (all_applications);
	puts ("");

	all_components = gnome_vfs_mime_get_all_components (type); 
	puts("All Components");
	print_component_list (all_components);
	puts ("");

	return 0;
}


