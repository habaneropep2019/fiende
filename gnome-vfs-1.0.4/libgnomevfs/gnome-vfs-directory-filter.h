/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-directory-filter.h - Directory filter for the GNOME
   Virtual File System.

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

#ifndef GNOME_VFS_DIRECTORY_FILTER_H
#define GNOME_VFS_DIRECTORY_FILTER_H

#include <libgnomevfs/gnome-vfs-file-info.h>

typedef enum {
	GNOME_VFS_DIRECTORY_FILTER_NONE,
	GNOME_VFS_DIRECTORY_FILTER_SHELLPATTERN,
	GNOME_VFS_DIRECTORY_FILTER_REGEXP
} GnomeVFSDirectoryFilterType;
typedef enum {
	GNOME_VFS_DIRECTORY_FILTER_DEFAULT = 0,
	GNOME_VFS_DIRECTORY_FILTER_NODIRS = 1 << 0,
	GNOME_VFS_DIRECTORY_FILTER_DIRSONLY = 1 << 1,
	GNOME_VFS_DIRECTORY_FILTER_NODOTFILES = 1 << 2,
	GNOME_VFS_DIRECTORY_FILTER_IGNORECASE = 1 << 3,
	GNOME_VFS_DIRECTORY_FILTER_EXTENDEDREGEXP =  1 << 4,
	GNOME_VFS_DIRECTORY_FILTER_NOSELFDIR = 1 << 5,
	GNOME_VFS_DIRECTORY_FILTER_NOPARENTDIR = 1 << 6,
	GNOME_VFS_DIRECTORY_FILTER_NOBACKUPFILES = 1 << 7
} GnomeVFSDirectoryFilterOptions;

typedef enum {
	GNOME_VFS_DIRECTORY_FILTER_NEEDS_NOTHING = 0,
	GNOME_VFS_DIRECTORY_FILTER_NEEDS_NAME = 1 << 0,
	GNOME_VFS_DIRECTORY_FILTER_NEEDS_TYPE = 1 << 1,
	GNOME_VFS_DIRECTORY_FILTER_NEEDS_STAT = 1 << 2,
	GNOME_VFS_DIRECTORY_FILTER_NEEDS_MIMETYPE = 1 << 3,
} GnomeVFSDirectoryFilterNeeds;

typedef enum {
	GNOME_VFS_DIRECTORY_VISIT_DEFAULT = 0,
	GNOME_VFS_DIRECTORY_VISIT_SAMEFS = 1 << 0,
	GNOME_VFS_DIRECTORY_VISIT_LOOPCHECK = 1 << 1
} GnomeVFSDirectoryVisitOptions;

typedef struct GnomeVFSDirectoryFilter GnomeVFSDirectoryFilter;

typedef gboolean (* GnomeVFSDirectoryFilterFunc) (const GnomeVFSFileInfo *info,
						  gpointer data);
typedef gboolean (* GnomeVFSDirectoryVisitFunc)	 (const gchar *rel_path,
						  GnomeVFSFileInfo *info,
						  gboolean recursing_will_loop,
						  gpointer data,
						  gboolean *recurse);

GnomeVFSDirectoryFilter *
	gnome_vfs_directory_filter_new	(GnomeVFSDirectoryFilterType type,
					 GnomeVFSDirectoryFilterOptions
					         options,
					 const gchar *filter_pattern);
GnomeVFSDirectoryFilter *
	gnome_vfs_directory_filter_new_custom
					(GnomeVFSDirectoryFilterFunc func,
					 GnomeVFSDirectoryFilterNeeds needs,
					 gpointer func_data);
void	gnome_vfs_directory_filter_destroy
					(GnomeVFSDirectoryFilter *filter);
gboolean
	gnome_vfs_directory_filter_apply
					(const GnomeVFSDirectoryFilter *filter,
					 GnomeVFSFileInfo *info);

GnomeVFSDirectoryFilterNeeds
	gnome_vfs_directory_filter_get_needs
					(const GnomeVFSDirectoryFilter *filter);

#endif /* GNOME_VFS_DIRECTORY_FILTER_H */
