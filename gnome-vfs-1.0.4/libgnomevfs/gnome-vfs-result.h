/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-result.h - Result handling for the GNOME Virtual File System.

   Copyright (C) 1999, 2001 Free Software Foundation

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

   Author: Ettore Perazzoli <ettore@comm2000.it>
           Seth Nickell <snickell@stanford.edu>
*/

#ifndef GNOME_VFS_RESULT_H
#define GNOME_VFS_RESULT_H

#include <glib.h>

/* IMPORTANT NOTICE: If you add error types here, please also add the
   corresponsing descriptions in `gnome-vfs-result.c'.  Moreover, *always* add
   new values at the end of the list, and *never* remove values.  */
typedef enum {
	GNOME_VFS_OK,
	GNOME_VFS_ERROR_NOT_FOUND,
	GNOME_VFS_ERROR_GENERIC,
	GNOME_VFS_ERROR_INTERNAL,
	GNOME_VFS_ERROR_BAD_PARAMETERS,
	GNOME_VFS_ERROR_NOT_SUPPORTED,
	GNOME_VFS_ERROR_IO,
	GNOME_VFS_ERROR_CORRUPTED_DATA,
	GNOME_VFS_ERROR_WRONG_FORMAT,
	GNOME_VFS_ERROR_BAD_FILE,
	GNOME_VFS_ERROR_TOO_BIG,
	GNOME_VFS_ERROR_NO_SPACE,
	GNOME_VFS_ERROR_READ_ONLY,
	GNOME_VFS_ERROR_INVALID_URI,
	GNOME_VFS_ERROR_NOT_OPEN,
	GNOME_VFS_ERROR_INVALID_OPEN_MODE,
	GNOME_VFS_ERROR_ACCESS_DENIED,
	GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES,
	GNOME_VFS_ERROR_EOF,
	GNOME_VFS_ERROR_NOT_A_DIRECTORY,
	GNOME_VFS_ERROR_IN_PROGRESS,
	GNOME_VFS_ERROR_INTERRUPTED,
	GNOME_VFS_ERROR_FILE_EXISTS,
	GNOME_VFS_ERROR_LOOP,
	GNOME_VFS_ERROR_NOT_PERMITTED,
	GNOME_VFS_ERROR_IS_DIRECTORY,
	GNOME_VFS_ERROR_NO_MEMORY,
	GNOME_VFS_ERROR_HOST_NOT_FOUND,
	GNOME_VFS_ERROR_INVALID_HOST_NAME,
	GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS,
	GNOME_VFS_ERROR_LOGIN_FAILED,
	GNOME_VFS_ERROR_CANCELLED,
	GNOME_VFS_ERROR_DIRECTORY_BUSY,
	GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY,
	GNOME_VFS_ERROR_TOO_MANY_LINKS,
	GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM,
	GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM,
	GNOME_VFS_ERROR_NAME_TOO_LONG,
	GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE,
	GNOME_VFS_ERROR_SERVICE_OBSOLETE,
	GNOME_VFS_ERROR_PROTOCOL_ERROR,
	GNOME_VFS_NUM_ERRORS
} GnomeVFSResult;

const gchar	*gnome_vfs_result_to_string	  (GnomeVFSResult result);
GnomeVFSResult   gnome_vfs_result_from_errno_code (int errno_code);
GnomeVFSResult	 gnome_vfs_result_from_errno	  (void);
GnomeVFSResult   gnome_vfs_result_from_h_errno    (void);

#endif /* GNOME_VFS_RESULT_H */
