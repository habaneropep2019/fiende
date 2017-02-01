/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-directory.h - Directory handling for the GNOME Virtual
   File System.

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

   Author: Ettore Perazzoli <ettore@comm2000.it> */

#ifndef GNOME_VFS_DIRECTORY_H
#define GNOME_VFS_DIRECTORY_H

#include <libgnomevfs/gnome-vfs-directory-filter.h>

typedef struct GnomeVFSDirectoryHandle GnomeVFSDirectoryHandle;


GnomeVFSResult	gnome_vfs_directory_open
					(GnomeVFSDirectoryHandle **handle,
					 const gchar *text_uri,
					 GnomeVFSFileInfoOptions options,
					 const GnomeVFSDirectoryFilter *filter);
GnomeVFSResult	gnome_vfs_directory_open_from_uri
					(GnomeVFSDirectoryHandle **handle,
					 GnomeVFSURI *uri,
					 GnomeVFSFileInfoOptions options,
					 const GnomeVFSDirectoryFilter *filter);
GnomeVFSResult	gnome_vfs_directory_read_next
					(GnomeVFSDirectoryHandle *handle,
					 GnomeVFSFileInfo *info);
GnomeVFSResult	gnome_vfs_directory_close
					(GnomeVFSDirectoryHandle *handle);


GnomeVFSResult  gnome_vfs_directory_visit
					(const gchar *uri,
					 GnomeVFSFileInfoOptions info_options,
					 const GnomeVFSDirectoryFilter *filter,
					 GnomeVFSDirectoryVisitOptions
					 	visit_options,
					 GnomeVFSDirectoryVisitFunc callback,
					 gpointer data);

GnomeVFSResult  gnome_vfs_directory_visit_uri
					(GnomeVFSURI *uri,
					 GnomeVFSFileInfoOptions info_options,
					 const GnomeVFSDirectoryFilter *filter,
					 GnomeVFSDirectoryVisitOptions
					 	visit_options,
					 GnomeVFSDirectoryVisitFunc callback,
					 gpointer data);

GnomeVFSResult	gnome_vfs_directory_visit_files
					(const gchar *text_uri,
					 GList *file_list,
					 GnomeVFSFileInfoOptions info_options,
					 const GnomeVFSDirectoryFilter *filter,
					 GnomeVFSDirectoryVisitOptions
					 	visit_options,
					 GnomeVFSDirectoryVisitFunc callback,
					 gpointer data);

GnomeVFSResult	gnome_vfs_directory_visit_files_at_uri
					(GnomeVFSURI *uri,
					 GList *file_list,
					 GnomeVFSFileInfoOptions info_options,
					 const GnomeVFSDirectoryFilter *filter,
					 GnomeVFSDirectoryVisitOptions
					 	visit_options,
					 GnomeVFSDirectoryVisitFunc callback,
					 gpointer data);

GnomeVFSResult gnome_vfs_directory_list_load
					(GList **list,
				         const gchar *text_uri,
				         GnomeVFSFileInfoOptions options,
				         const GnomeVFSDirectoryFilter *filter);

#endif
