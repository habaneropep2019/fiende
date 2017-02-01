/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-mime-handlers.h - Mime type handlers for the GNOME Virtual
   File System.

   Copyright (C) 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Maciej Stachowiak <mjs@eazel.com> */

#ifndef GNOME_VFS_MIME_HANDLERS_H
#define GNOME_VFS_MIME_HANDLERS_H

#include <libgnomevfs/gnome-vfs-result.h>
#include <liboaf/liboaf.h>

typedef enum {
	GNOME_VFS_MIME_ACTION_TYPE_NONE,
	GNOME_VFS_MIME_ACTION_TYPE_APPLICATION,
	GNOME_VFS_MIME_ACTION_TYPE_COMPONENT
} GnomeVFSMimeActionType;

typedef enum {
	GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS,
	GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_PATHS,
	GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS_FOR_NON_FILES
} GnomeVFSMimeApplicationArgumentType;

typedef struct {
	char *id;
	char *name;
	char *command;
	gboolean can_open_multiple_files;
	GnomeVFSMimeApplicationArgumentType expects_uris;
	GList *supported_uri_schemes;
	gboolean requires_terminal;
} GnomeVFSMimeApplication;

typedef struct {
	GnomeVFSMimeActionType action_type;
	union {
		OAF_ServerInfo *component;
		void *dummy_component;
		GnomeVFSMimeApplication *application;
	} action;
} GnomeVFSMimeAction;

GnomeVFSMimeApplication *gnome_vfs_mime_application_copy                   (GnomeVFSMimeApplication *application);
GnomeVFSMimeActionType   gnome_vfs_mime_get_default_action_type            (const char              *mime_type);
GnomeVFSMimeAction *     gnome_vfs_mime_get_default_action                 (const char              *mime_type);
GnomeVFSMimeApplication *gnome_vfs_mime_get_default_application            (const char              *mime_type);
OAF_ServerInfo *         gnome_vfs_mime_get_default_component              (const char              *mime_type);
GList *                  gnome_vfs_mime_get_short_list_applications        (const char              *mime_type);
GList *                  gnome_vfs_mime_get_short_list_components          (const char              *mime_type);
GList *                  gnome_vfs_mime_get_all_applications               (const char              *mime_type);
GList *                  gnome_vfs_mime_get_all_components                 (const char              *mime_type);
GnomeVFSResult           gnome_vfs_mime_set_default_action_type            (const char              *mime_type,
									    GnomeVFSMimeActionType   action_type);
GnomeVFSResult           gnome_vfs_mime_set_default_application            (const char              *mime_type,
									    const char              *application_id);
GnomeVFSResult           gnome_vfs_mime_set_default_component              (const char              *mime_type,
									    const char              *component_iid);

const char  		*gnome_vfs_mime_get_icon 			   (const char 		    *mime_type);
GnomeVFSResult  	 gnome_vfs_mime_set_icon 			   (const char 		    *mime_type,
									    const char		    *filename);
const char 	        *gnome_vfs_mime_get_description   		   (const char              *mime_type);
GnomeVFSResult		 gnome_vfs_mime_set_description			   (const char		    *mime_type,
									    const char		    *description);

gboolean 	         gnome_vfs_mime_can_be_executable   		   (const char              *mime_type);
GnomeVFSResult	 	 gnome_vfs_mime_set_can_be_executable   	   (const char              *mime_type,
									    gboolean		     new_value);

/* Stored as delta to current user level - API function computes delta and stores in prefs */
GnomeVFSResult           gnome_vfs_mime_set_short_list_applications        (const char              *mime_type,
									    GList                   *application_ids);
GnomeVFSResult           gnome_vfs_mime_set_short_list_components          (const char              *mime_type,
									    GList                   *component_iids);
GnomeVFSResult           gnome_vfs_mime_add_application_to_short_list      (const char              *mime_type,
									    const char              *application_id);
GnomeVFSResult           gnome_vfs_mime_remove_application_from_short_list (const char              *mime_type,
									    const char              *application_id);
GnomeVFSResult           gnome_vfs_mime_add_component_to_short_list        (const char              *mime_type,
									    const char              *iid);
GnomeVFSResult           gnome_vfs_mime_remove_component_from_short_list   (const char              *mime_type,
									    const char              *iid);
GnomeVFSResult           gnome_vfs_mime_add_extension                      (const char              *mime_type,
									    const char              *extension);
GnomeVFSResult           gnome_vfs_mime_remove_extension                   (const char              *mime_type,
									    const char              *extension);


/* No way to override system list; can only add. */
GnomeVFSResult           gnome_vfs_mime_extend_all_applications            (const char              *mime_type,
									    GList                   *application_ids);
/* Only "user" entries may be removed. */
GnomeVFSResult           gnome_vfs_mime_remove_from_all_applications       (const char              *mime_type,
									    GList                   *application_ids);
GnomeVFSMimeApplication *gnome_vfs_mime_application_new_from_id            (const char              *id);
void                     gnome_vfs_mime_application_free                   (GnomeVFSMimeApplication *application);
void                     gnome_vfs_mime_action_free                        (GnomeVFSMimeAction      *action);

/* List manipulation helper functions */
void                     gnome_vfs_mime_application_list_free              (GList                   *list);
void                     gnome_vfs_mime_component_list_free                (GList                   *list);
gboolean                 gnome_vfs_mime_id_in_application_list             (const char              *id,
									    GList                   *applications);
gboolean                 gnome_vfs_mime_id_in_component_list               (const char              *iid,
									    GList                   *components);
GList *                  gnome_vfs_mime_remove_application_from_list       (GList                   *applications,
									    const char              *application_id,
									    gboolean                *did_remove);
GList *                  gnome_vfs_mime_remove_component_from_list         (GList                   *components,
									    const char              *iid,
									    gboolean                *did_remove);
GList *                  gnome_vfs_mime_id_list_from_component_list        (GList                   *components);
GList *                  gnome_vfs_mime_id_list_from_application_list      (GList                   *applications);

#endif /* GNOME_VFS_MIME_HANDLERS_H */
