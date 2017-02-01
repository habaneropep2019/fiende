/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-find-directory.c - Public utility functions for the GNOME Virtual
   File System.

   Copyright (C) 2000 Eazel, Inc.

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

   Authors: Pavel Cisler <pavel@eazel.com>
*/

#include <config.h>
#include "gnome-vfs-find-directory.h"

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"


/**
 * gnome_vfs_find_directory:
 * @near_uri: find a well known directory on the same volume as @near_uri
 * @kind: kind of well known directory
 * @result: newly created URI of the directory we found
 * @create_if_needed: If directory we are looking for does not exist, try to create it
 * @create_if_needed: If we don't know where trash is yet, look for it.
 * @permissions: If creating, use these permissions
 * 
 * Used to return well known directories such as Trash, Desktop, etc. from different
 * file systems.
 * 
 * There is quite a complicated logic behind finding/creating a Trash directory
 * and you need to be aware of some implications:
 * Finding the Trash the first time when using the file method may be pretty 
 * expensive. A cache file is used to store the location of that Trash file
 * for next time.
 * If @ceate_if_needed is specified without @find_if_needed, you may end up
 * creating a Trash file when there already is one. Your app should start out
 * by doing a gnome_vfs_find_directory with the @find_if_needed to avoid this
 * and then use the @create_if_needed flag to create Trash lazily when it is
 * needed for throwing away an item on a given disk.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult	
gnome_vfs_find_directory (GnomeVFSURI 			*near_uri,
			  GnomeVFSFindDirectoryKind 	kind,
			  GnomeVFSURI 			**result,
			  gboolean 			create_if_needed,
			  gboolean			find_if_needed,
			  guint 			permissions)
{
	return gnome_vfs_find_directory_cancellable (near_uri, kind, result,
		create_if_needed, find_if_needed, permissions, NULL);
}
