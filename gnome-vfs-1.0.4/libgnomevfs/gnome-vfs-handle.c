/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-handle.c - Handle object for GNOME VFS files.

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
#include "gnome-vfs-handle.h"

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"


struct GnomeVFSHandle {
	/* URI of the file being accessed through the handle.  */
	GnomeVFSURI *uri;

	/* Method-specific handle.  */
	GnomeVFSMethodHandle *method_handle;

	/* Open mode.  */
	GnomeVFSOpenMode open_mode;
};

#define CHECK_IF_OPEN(handle)			\
G_STMT_START{					\
	if (handle->uri == NULL)		\
		return GNOME_VFS_ERROR_NOT_OPEN;	\
}G_STMT_END

#define CHECK_IF_SUPPORTED(handle, what)		\
G_STMT_START{						\
	if (!VFS_METHOD_HAS_FUNC(handle->uri->method, what))		\
		return GNOME_VFS_ERROR_NOT_SUPPORTED;	\
}G_STMT_END

#define INVOKE(result, handle, what, params)		\
G_STMT_START{						\
	CHECK_IF_OPEN (handle);				\
	CHECK_IF_SUPPORTED (handle, what);		\
	(result) = handle->uri->method->what params;	\
}G_STMT_END

#define INVOKE_AND_RETURN(handle, what, params)		\
G_STMT_START{						\
        GnomeVFSResult __result;			\
							\
	INVOKE (__result, handle, what, params);	\
	return __result;				\
}G_STMT_END


GnomeVFSHandle *
gnome_vfs_handle_new (GnomeVFSURI *uri,
		      GnomeVFSMethodHandle *method_handle,
		      GnomeVFSOpenMode open_mode)
{
	GnomeVFSHandle *new;

	g_return_val_if_fail (uri != NULL, NULL);
	g_return_val_if_fail (method_handle != NULL, NULL);

	new = g_new (GnomeVFSHandle, 1);

	new->uri = gnome_vfs_uri_ref (uri);
	new->method_handle = method_handle;
	new->open_mode = open_mode;

	if ((open_mode & GNOME_VFS_OPEN_RANDOM) &&
	    !VFS_METHOD_HAS_FUNC (uri->method, seek)) {
		GnomeVFSMethodHandle *handle;
		handle = gnome_vfs_seek_emulate (new->uri, method_handle,
						 open_mode);
		if (handle) /* Successfully wrapped */
			new->method_handle = handle;
	}

	return new;
}

void
gnome_vfs_handle_destroy (GnomeVFSHandle *handle)
{
	g_return_if_fail (handle != NULL);

	gnome_vfs_uri_unref (handle->uri);

	g_free (handle);
}


GnomeVFSOpenMode
gnome_vfs_handle_get_open_mode (GnomeVFSHandle *handle)
{
	g_return_val_if_fail (handle != NULL, (GnomeVFSOpenMode) 0);

	return handle->open_mode;
}


/* Actions.  */

GnomeVFSResult
gnome_vfs_handle_do_close (GnomeVFSHandle *handle,
			   GnomeVFSContext *context)
{
	GnomeVFSResult result;

	INVOKE (result, handle, close, (handle->uri->method, handle->method_handle, context));

	/* Even if close has failed, we shut down the handle. */
	gnome_vfs_handle_destroy (handle);

	return result;
}

GnomeVFSResult
gnome_vfs_handle_do_read (GnomeVFSHandle *handle,
			  gpointer buffer,
			  GnomeVFSFileSize num_bytes,
			  GnomeVFSFileSize *bytes_read,
			  GnomeVFSContext *context)
{
	INVOKE_AND_RETURN (handle, read, (handle->uri->method, handle->method_handle,
					  buffer, num_bytes, bytes_read,
					  context));
}

GnomeVFSResult
gnome_vfs_handle_do_write (GnomeVFSHandle *handle,
			   gconstpointer buffer,
			   GnomeVFSFileSize num_bytes,
			   GnomeVFSFileSize *bytes_written,
			   GnomeVFSContext *context)
{
	INVOKE_AND_RETURN (handle, write, (handle->uri->method, handle->method_handle,
					   buffer, num_bytes, bytes_written,
					   context));
}


GnomeVFSResult
gnome_vfs_handle_do_seek (GnomeVFSHandle *handle,
			  GnomeVFSSeekPosition whence,
			  GnomeVFSFileSize offset,
			  GnomeVFSContext *context)
{
	INVOKE_AND_RETURN (handle, seek, (handle->uri->method, handle->method_handle,
					  whence, offset, context));
}

GnomeVFSResult
gnome_vfs_handle_do_tell (GnomeVFSHandle *handle,
			  GnomeVFSFileSize *offset_return)
{
	INVOKE_AND_RETURN (handle, tell, (handle->uri->method, handle->method_handle,
					  offset_return));
}


GnomeVFSResult
gnome_vfs_handle_do_get_file_info (GnomeVFSHandle *handle,
				   GnomeVFSFileInfo *info,
				   GnomeVFSFileInfoOptions options,
				   GnomeVFSContext *context)
{
	INVOKE_AND_RETURN (handle, get_file_info_from_handle,
			   (handle->uri->method, handle->method_handle, info, options,
			    context));
}

GnomeVFSResult gnome_vfs_handle_do_truncate     (GnomeVFSHandle *handle,
						 GnomeVFSFileSize length,
						 GnomeVFSContext *context)
{
	INVOKE_AND_RETURN (handle, truncate_handle, (handle->uri->method, handle->method_handle, length, context));
}
