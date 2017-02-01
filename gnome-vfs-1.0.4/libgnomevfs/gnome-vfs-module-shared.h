/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-module-shared.h - code shared between the different modules
   place.

   Copyright (C) 2001 Free Software Foundation

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

   Author: Seth Nickell <snickell@stanford.edu
 */

#ifndef GNOME_VFS_MODULE_SHARED_H
#define GNOME_VFS_MODULE_SHARED_H

#include <libgnomevfs/gnome-vfs-file-info.h>

const gchar *   gnome_vfs_mime_type_from_mode 	(mode_t mode);

void            gnome_vfs_stat_to_file_info	(GnomeVFSFileInfo *file_info,
					    	 const struct stat *statptr);

GnomeVFSResult  gnome_vfs_set_meta          	(GnomeVFSFileInfo *info,
					    	 const gchar *file_name,
					    	 const gchar *meta_key);

GnomeVFSResult  gnome_vfs_set_meta_for_list 	(GnomeVFSFileInfo *info,
					    	 const gchar *file_name,
					    	 const GList *meta_keys);
	
const char     *gnome_vfs_get_special_mime_type (GnomeVFSURI *uri);

#endif
