/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-uri.h - URI handling for the GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation

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

   Author: Ettore Perazzoli <ettore@comm2000.it>
*/

#ifndef GNOME_VFS_URI_H
#define GNOME_VFS_URI_H

#include <glib.h>

/* This describes a URI element.  */
typedef struct GnomeVFSURI {
	/* Reference count.  */
	guint ref_count;

	/* Text for the element: eg. some/path/name.  */
	gchar *text;

	/* Text for uri fragment: eg, #anchor  */
	gchar *fragment_id;
	
	/* Method string: eg. `gzip', `tar', `http'.  This is necessary as
	   one GnomeVFSMethod can be used for different method strings
	   (e.g. extfs handles zip, rar, zoo and several other ones).  */
	gchar *method_string;

	/* VFS method to access the element.  */
	struct GnomeVFSMethod *method;

	/* Pointer to the parent element, or NULL for toplevel elements.  */
	struct GnomeVFSURI *parent;
} GnomeVFSURI;

/* This is the toplevel URI element.  A toplevel method implementations should
   cast the `GnomeVFSURI' argument to this type to get the additional host/auth
   information.  If any of the elements is 0, it is unspecified.  */
typedef struct {
	/* Base object.  */
	GnomeVFSURI uri;

	/* Server location information.  */
	gchar *host_name;
	guint host_port;

	/* Authorization information.  */
	gchar *user_name;
	gchar *password;

	/* The parent URN, if it exists */
	gchar *urn;
} GnomeVFSToplevelURI;


/* This is used for hiding information when transforming the GnomeVFSURI into a
   string.  */
typedef enum {
	GNOME_VFS_URI_HIDE_NONE = 0,
	GNOME_VFS_URI_HIDE_USER_NAME = 1 << 0,
	GNOME_VFS_URI_HIDE_PASSWORD = 1 << 1,
	GNOME_VFS_URI_HIDE_HOST_NAME = 1 << 2,
	GNOME_VFS_URI_HIDE_HOST_PORT = 1 << 3,
	GNOME_VFS_URI_HIDE_TOPLEVEL_METHOD = 1 << 4,
	GNOME_VFS_URI_HIDE_FRAGMENT_IDENTIFIER = 1 << 8
} GnomeVFSURIHideOptions;


/* CONSTANTS */
#define GNOME_VFS_URI_MAGIC_CHR	'#'
#define GNOME_VFS_URI_MAGIC_STR "#"

#define GNOME_VFS_URI_PATH_CHR '/'
#define GNOME_VFS_URI_PATH_STR "/"

/* FUNCTIONS */
GnomeVFSURI 	     *gnome_vfs_uri_new                   (const gchar *text_uri);
GnomeVFSURI 	     *gnome_vfs_uri_resolve_relative      (const GnomeVFSURI *base,
							   const char *relative_reference);
GnomeVFSURI 	     *gnome_vfs_uri_ref                   (GnomeVFSURI *uri);
void        	      gnome_vfs_uri_unref                 (GnomeVFSURI *uri);

GnomeVFSURI          *gnome_vfs_uri_append_string         (const GnomeVFSURI *uri,
						           const char *path);
GnomeVFSURI          *gnome_vfs_uri_append_path           (const GnomeVFSURI *uri,
						           const char *path);
GnomeVFSURI          *gnome_vfs_uri_append_file_name      (const GnomeVFSURI *uri,
						           const char *filename);
char       	     *gnome_vfs_uri_to_string             (const GnomeVFSURI *uri,
						           GnomeVFSURIHideOptions hide_options);
GnomeVFSURI 	     *gnome_vfs_uri_dup                   (const GnomeVFSURI *uri);
gboolean    	      gnome_vfs_uri_is_local              (const GnomeVFSURI *uri);
gboolean	      gnome_vfs_uri_has_parent	          (const GnomeVFSURI *uri);
GnomeVFSURI	     *gnome_vfs_uri_get_parent            (const GnomeVFSURI *uri);

GnomeVFSToplevelURI *gnome_vfs_uri_get_toplevel           (const GnomeVFSURI *uri);

const char 	    *gnome_vfs_uri_get_host_name          (const GnomeVFSURI *uri);
const char          *gnome_vfs_uri_get_scheme             (const GnomeVFSURI *uri);
guint 	    	     gnome_vfs_uri_get_host_port          (const GnomeVFSURI *uri);
const char 	    *gnome_vfs_uri_get_user_name          (const GnomeVFSURI *uri);
const char	    *gnome_vfs_uri_get_password           (const GnomeVFSURI *uri);

void		     gnome_vfs_uri_set_host_name          (GnomeVFSURI *uri,
						           const char *host_name);
void 	    	     gnome_vfs_uri_set_host_port          (GnomeVFSURI *uri,
						           guint host_port);
void		     gnome_vfs_uri_set_user_name          (GnomeVFSURI *uri,
						           const char *user_name);
void		     gnome_vfs_uri_set_password           (GnomeVFSURI *uri,
						           const char *password);

gboolean	     gnome_vfs_uri_equal	          (const GnomeVFSURI *a,
						           const GnomeVFSURI *b);

gboolean	     gnome_vfs_uri_is_parent	          (const GnomeVFSURI *parent,
						           const GnomeVFSURI *item,
						           gboolean recursive);
				  
const char 	    *gnome_vfs_uri_get_path                (const GnomeVFSURI *uri);
const char 	    *gnome_vfs_uri_get_basename            (const GnomeVFSURI *uri);
const char 	    *gnome_vfs_uri_get_fragment_identifier (const GnomeVFSURI *uri);
char 		    *gnome_vfs_uri_extract_dirname         (const GnomeVFSURI *uri);
char		    *gnome_vfs_uri_extract_short_name      (const GnomeVFSURI *uri);
char		    *gnome_vfs_uri_extract_short_path_name (const GnomeVFSURI *uri);

gint		     gnome_vfs_uri_hequal 	           (gconstpointer a,
						            gconstpointer b);
guint		     gnome_vfs_uri_hash		           (gconstpointer p);

GList               *gnome_vfs_uri_list_ref                (GList *list);
GList               *gnome_vfs_uri_list_unref              (GList *list);
GList               *gnome_vfs_uri_list_copy               (GList *list);
void                 gnome_vfs_uri_list_free               (GList *list);

#endif /* GNOME_VFS_URI_H */
