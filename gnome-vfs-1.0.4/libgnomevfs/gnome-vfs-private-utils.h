/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-private-utils.h - Private utility functions for the GNOME Virtual
   File System.

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

#ifndef _GNOME_VFS_PRIVATE_UTILS_H
#define _GNOME_VFS_PRIVATE_UTILS_H

/* You should not use calls in here outside GnomeVFS. The APIs in here may
 * break even when the GnomeVFS APIs are otherwise frozen.
 */

#include <libgnomevfs/gnome-vfs-cancellation.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-process.h>
#include <libgnomevfs/gnome-vfs-uri.h>

gchar   	*gnome_vfs_canonicalize_pathname         (char *path);
GnomeVFSResult   gnome_vfs_remove_optional_escapes 	 (char *escaped_uri);

pid_t	 	 gnome_vfs_forkexec 	(const gchar *file_name,
					 const gchar * const argv[],
					 GnomeVFSProcessOptions options,
					 GnomeVFSProcessInitFunc init_func,
					 gpointer data);
GnomeVFSProcessRunResult
	 	gnome_vfs_process_run_cancellable
					(const gchar *file_name,
					 const gchar * const argv[],
					 GnomeVFSProcessOptions options,
					 GnomeVFSCancellation *cancellation,
					 guint *exit_value);

GnomeVFSResult	gnome_vfs_create_temp 	(const gchar *prefix,
					 gchar **name_return,
					 GnomeVFSHandle **handle_return);
gboolean	gnome_vfs_atotm		(const gchar *time_string,
					 time_t *value_return);

/* Wrapper around gnome_i18n_get_language_list(). */
GList	       *gnome_vfs_i18n_get_language_list
					(const gchar *category_name);

GnomeVFSURI    *gnome_vfs_uri_new_private (const gchar *text_uri, 
					   gboolean allow_unknown_method,
					   gboolean allow_unsafe_method,
					   gboolean allow_translate);


gboolean	gnome_vfs_istr_has_prefix (const char *haystack,
					   const char *needle);
gboolean	gnome_vfs_istr_has_suffix (const char *haystack,
					   const char *needle);

#endif /* _GNOME_VFS_PRIVATE_UTILS_H */
