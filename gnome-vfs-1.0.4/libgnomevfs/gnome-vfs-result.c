/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-error.c - Error handling for the GNOME Virtual File System.

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

   Author: Ettore Perazzoli <ettore@gnu.org>
*/

#include <config.h>
#include "gnome-vfs-result.h"

#include <netdb.h>
#include <errno.h>

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"


extern int h_errno;


static gchar *status_strings[] = {
	/* GNOME_VFS_OK */				N_("No error"),
	/* GNOME_VFS_ERROR_NOT_FOUND */			N_("File not found"),
	/* GNOME_VFS_ERROR_GENERIC */			N_("Generic error"),
	/* GNOME_VFS_ERROR_INTERNAL */			N_("Internal error"),
	/* GNOME_VFS_ERROR_BAD_PARAMETERS */		N_("Invalid parameters"),
	/* GNOME_VFS_ERROR_NOT_SUPPORTED */		N_("Unsupported operation"),
	/* GNOME_VFS_ERROR_IO */			N_("I/O error"),
	/* GNOME_VFS_ERROR_CORRUPTED_DATA */		N_("Data corrupted"),
	/* GNOME_VFS_ERROR_WRONG_FORMAT */		N_("Format not valid"),
	/* GNOME_VFS_ERROR_BAD_FILE */			N_("Bad file handle"),
	/* GNOME_VFS_ERROR_TOO_BIG */			N_("File too big"),
	/* GNOME_VFS_ERROR_NO_SPACE */			N_("No space left on device"),
	/* GNOME_VFS_ERROR_READ_ONLY */			N_("Read-only file system"),
	/* GNOME_VFS_ERROR_INVALID_URI */		N_("Invalid URI"),
	/* GNOME_VFS_ERROR_NOT_OPEN */			N_("File not open"),
	/* GNOME_VFS_ERROR_INVALID_OPEN_MODE */		N_("Open mode not valid"),
	/* GNOME_VFS_ERROR_ACCESS_DENIED */		N_("Access denied"),
	/* GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES */	N_("Too many open files"),
	/* GNOME_VFS_ERROR_EOF */			N_("End of file"),
	/* GNOME_VFS_ERROR_NOT_A_DIRECTORY */		N_("Not a directory"),
	/* GNOME_VFS_ERROR_IN_PROGRESS */		N_("Operation in progress"),
	/* GNOME_VFS_ERROR_INTERRUPTED */		N_("Operation interrupted"),
	/* GNOME_VFS_ERROR_FILE_EXISTS */		N_("File exists"),
	/* GNOME_VFS_ERROR_LOOP */			N_("Looping links encountered"),
	/* GNOME_VFS_ERROR_NOT_PERMITTED */		N_("Operation not permitted"),
	/* GNOME_VFS_ERROR_IS_DIRECTORY */       	N_("Is a directory"),
        /* GNOME_VFS_ERROR_NO_MEMMORY */             	N_("Not enough memory"),
	/* GNOME_VFS_ERROR_HOST_NOT_FOUND */		N_("Host not found"),
	/* GNOME_VFS_ERROR_INVALID_HOST_NAME */		N_("Host name not valid"),
	/* GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS */  	N_("Host has no address"),
	/* GNOME_VFS_ERROR_LOGIN_FAILED */		N_("Login failed"),
	/* GNOME_VFS_ERROR_CANCELLED */			N_("Operation cancelled"),
	/* GNOME_VFS_ERROR_DIRECTORY_BUSY */     	N_("Directory busy"),
	/* GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY */ 	N_("Directory not empty"),
	/* GNOME_VFS_ERROR_TOO_MANY_LINKS */		N_("Too many links"),
	/* GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM */	N_("Read only file system"),
	/* GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM */	N_("Not on the same file system"),
	/* GNOME_VFS_ERROR_NAME_TOO_LONG */		N_("Name too long"),
	/* GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE */     N_("Service not available"),
	/* GNOME_VFS_ERROR_SERVICE_OBSOLETE */          N_("Request obsoletes service's data"),
	/* GNOME_VFS_ERROR_PROTOCOL_ERROR */		N_("Protocol error")
};


GnomeVFSResult
gnome_vfs_result_from_errno_code (int errno_code)
{
	/* Please keep these in alphabetical order.  */
	switch (errno_code) {
	case E2BIG:     return GNOME_VFS_ERROR_TOO_BIG;
	case EACCES:	return GNOME_VFS_ERROR_ACCESS_DENIED;
	case EBUSY:	return GNOME_VFS_ERROR_DIRECTORY_BUSY;
	case EBADF:	return GNOME_VFS_ERROR_BAD_FILE;
	case EEXIST:	return GNOME_VFS_ERROR_FILE_EXISTS;
	case EFAULT:	return GNOME_VFS_ERROR_INTERNAL;
	case EFBIG:	return GNOME_VFS_ERROR_TOO_BIG;
	case EINTR:	return GNOME_VFS_ERROR_INTERRUPTED;
	case EINVAL:	return GNOME_VFS_ERROR_BAD_PARAMETERS;
	case EIO:	return GNOME_VFS_ERROR_IO;
	case EISDIR:	return GNOME_VFS_ERROR_IS_DIRECTORY;
	case ELOOP:	return GNOME_VFS_ERROR_LOOP;
	case EMFILE:	return GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES;
	case EMLINK:	return GNOME_VFS_ERROR_TOO_MANY_LINKS;
	case ENFILE:	return GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES;
	case ENOTEMPTY: return GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY;
	case ENOENT:	return GNOME_VFS_ERROR_NOT_FOUND;
	case ENOMEM:	return GNOME_VFS_ERROR_NO_MEMORY;
	case ENOSPC:	return GNOME_VFS_ERROR_NO_SPACE;
	case ENOTDIR:	return GNOME_VFS_ERROR_NOT_A_DIRECTORY;
	case EPERM:	return GNOME_VFS_ERROR_NOT_PERMITTED;
	case EROFS:	return GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM;
	case EXDEV:	return GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM;
		/* FIXME bugzilla.eazel.com 1191: To be completed.  */
	default:	return GNOME_VFS_ERROR_GENERIC;
	}
}

GnomeVFSResult
gnome_vfs_result_from_errno (void)
{
       return gnome_vfs_result_from_errno_code(errno);
}
 

GnomeVFSResult
gnome_vfs_result_from_h_errno (void)
{
	switch (h_errno) {
	case HOST_NOT_FOUND:	return GNOME_VFS_ERROR_HOST_NOT_FOUND;
	case NO_ADDRESS:	return GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS;
	case TRY_AGAIN:		/* FIXME bugzilla.eazel.com 1190 */
	case NO_RECOVERY:	/* FIXME bugzilla.eazel.com 1190 */
	default:
		return GNOME_VFS_ERROR_GENERIC;
	}
}


const gchar *
gnome_vfs_result_to_string (GnomeVFSResult error)
{
	if ((guint) error >= (guint) (sizeof (status_strings)
				      / sizeof (*status_strings)))
		return _("Unknown error");

	return _(status_strings[(guint) error]);
}
