/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-ops.c - Synchronous operations for the GNOME Virtual File
   System.

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

   Author: Ettore Perazzoli <ettore@gnu.org> */

#include <config.h>
#include "gnome-vfs-ops.h"

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

/**
 * gnome_vfs_open:
 * @handle: A pointer to a pointer to a GnomeVFSHandle object
 * @text_uri: String representing the URI to open
 * @open_mode: Open mode
 * 
 * Open @text_uri according to mode @open_mode.  On return, @*handle will then
 * contain a pointer to a handle for the open file.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_open (GnomeVFSHandle **handle,
		const gchar *text_uri,
		GnomeVFSOpenMode open_mode)
{
	GnomeVFSURI *uri;
	GnomeVFSResult result;

	g_return_val_if_fail (handle != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (text_uri != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	uri = gnome_vfs_uri_new (text_uri);
	if (uri == NULL)
		return GNOME_VFS_ERROR_INVALID_URI;

	result = gnome_vfs_open_uri (handle, uri, open_mode);

	gnome_vfs_uri_unref (uri);

	return result;
}

/**
 * gnome_vfs_open_uri:
 * @handle: A pointer to a pointer to a GnomeVFSHandle object
 * @uri: URI to open
 * @open_mode: Open mode
 * 
 * Open @uri according to mode @open_mode.  On return, @*handle will then
 * contain a pointer to a handle for the open file.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_open_uri (GnomeVFSHandle **handle,
		    GnomeVFSURI *uri,
		    GnomeVFSOpenMode open_mode)
{
	return gnome_vfs_open_uri_cancellable (handle, uri, open_mode, NULL);
}

/**
 * gnome_vfs_create:
 * @handle: A pointer to a pointer to a GnomeVFSHandle object
 * @text_uri: String representing the URI to create
 * @open_mode: Open mode
 * @exclusive: Whether the file should be created in "exclusive" mode:
 * i.e. if this flag is nonzero, operation will fail if a file with the
 * same name already exists.
 * @perm: Bitmap representing the permissions for the newly created file
 * (Unix style).
 * 
 * Create @uri according to mode @open_mode.  On return, @*handle will then
 * contain a pointer to a handle for the open file.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_create (GnomeVFSHandle **handle,
		  const gchar *text_uri,
		  GnomeVFSOpenMode open_mode,
		  gboolean exclusive,
		  guint perm)
{
	GnomeVFSURI *uri;
	GnomeVFSResult result;

	g_return_val_if_fail (handle != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (text_uri != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	uri = gnome_vfs_uri_new (text_uri);
	if (uri == NULL)
		return GNOME_VFS_ERROR_INVALID_URI;

	result = gnome_vfs_create_uri (handle, uri, open_mode, exclusive, perm);

	gnome_vfs_uri_unref (uri);

	return result;
}

/**
 * gnome_vfs_create_uri:
 * @handle: A pointer to a pointer to a GnomeVFSHandle object
 * @uri: URI for the file to create
 * @open_mode: Open mode
 * @exclusive: Whether the file should be created in "exclusive" mode:
 * i.e. if this flag is nonzero, operation will fail if a file with the
 * same name already exists.
 * @perm: Bitmap representing the permissions for the newly created file
 * (Unix style).
 * 
 * Create @uri according to mode @open_mode.  On return, @*handle will then
 * contain a pointer to a handle for the open file.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_create_uri (GnomeVFSHandle **handle,
		      GnomeVFSURI *uri,
		      GnomeVFSOpenMode open_mode,
		      gboolean exclusive,
		      guint perm)
{
	return gnome_vfs_create_uri_cancellable (handle, uri, open_mode,
						 exclusive, perm, NULL);
}

/**
 * gnome_vfs_close:
 * @handle: A pointer to a GnomeVFSHandle object
 * 
 * Close file associated with @handle.
 * 
 * Return value: An integer representing the result of the operation.
 **/
GnomeVFSResult
gnome_vfs_close (GnomeVFSHandle *handle)
{
	return gnome_vfs_close_cancellable (handle, NULL);
}

/**
 * gnome_vfs_read:
 * @handle: Handle of the file to read data from
 * @buffer: Pointer to a buffer that must be at least @bytes bytes large
 * @bytes: Number of bytes to read
 * @bytes_read: Pointer to a variable that will hold the number of bytes
 * effectively read on return.
 * 
 * Read @bytes from @handle.  As with Unix system calls, the number of
 * bytes read can effectively be less than @bytes on return and will be
 * stored in @*bytes_read.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_read (GnomeVFSHandle *handle,
		gpointer buffer,
		GnomeVFSFileSize bytes,
		GnomeVFSFileSize *bytes_read)
{
	return gnome_vfs_read_cancellable (handle, buffer, bytes, bytes_read,
					   NULL);
}

/**
 * gnome_vfs_write:
 * @handle: Handle of the file to write data to
 * @buffer: Pointer to the buffer containing the data to be written
 * @bytes: Number of bytes to write
 * @bytes_write: Pointer to a variable that will hold the number of bytes
 * effectively written on return.
 * 
 * Write @bytes into the file opened through @handle.  As with Unix system
 * calls, the number of bytes written can effectively be less than @bytes on
 * return and will be stored in @*bytes_written.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_write (GnomeVFSHandle *handle,
		 gconstpointer buffer,
		 GnomeVFSFileSize bytes,
		 GnomeVFSFileSize *bytes_written)
{
	return gnome_vfs_write_cancellable (handle, buffer, bytes,
					    bytes_written, NULL);
}

/**
 * gnome_vfs_seek:
 * @handle: Handle for which the current position must be changed
 * @whence: Integer value representing the starting position
 * @offset: Number of bytes to skip from the position specified by @whence
 * (a positive value means to move forward; a negative one to move backwards)
 * 
 * Set the current position for reading/writing through @handle.
 * 
 * Return value: 
 **/
GnomeVFSResult
gnome_vfs_seek (GnomeVFSHandle *handle,
		GnomeVFSSeekPosition whence,
		GnomeVFSFileOffset offset)
{
	return gnome_vfs_seek_cancellable (handle, whence, offset, NULL);
}

/**
 * gnome_vfs_tell:
 * @handle: Handle for which the current position must be retrieved
 * @offset_return: Pointer to a variable that will contain the current position
 * on return
 * 
 * Return the current position on @handle.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_tell (GnomeVFSHandle *handle,
		GnomeVFSFileSize *offset_return)
{
	g_return_val_if_fail (handle != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	return gnome_vfs_handle_do_tell (handle, offset_return);
}

/**
 * gnome_vfs_get_file_info:
 * @text_uri: URI of the file for which information will be retrieved
 * @info: Pointer to a GnomeVFSFileInfo object that will hold the information
 * for the file on return
 * @options: Options for retrieving file information
 * to retrieve for the file
 * 
 * Retrieve information about @text_uri.  The information will be stored in
 * @*info.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_get_file_info (const gchar *text_uri,
			 GnomeVFSFileInfo *info,
			 GnomeVFSFileInfoOptions options)
{
	GnomeVFSURI *uri;
	GnomeVFSResult result;

	uri = gnome_vfs_uri_new (text_uri);

	if (uri == NULL)
		return GNOME_VFS_ERROR_NOT_SUPPORTED;
	
	result = gnome_vfs_get_file_info_uri(uri, info, options);
	gnome_vfs_uri_unref (uri);

	return result;
}

/**
 * gnome_vfs_get_file_info_uri:
 * @uri: URI of the file for which information will be retrieved
 * @info: Pointer to a GnomeVFSFileInfo object that will hold the information
 * for the file on return
 * @options: Options for retrieving file information
 * to retrieve for the file
 * 
 * Retrieve information about @text_uri.  The information will be stored in
 * @info.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_get_file_info_uri (GnomeVFSURI *uri,
			     GnomeVFSFileInfo *info,
			     GnomeVFSFileInfoOptions options)
{
	return gnome_vfs_get_file_info_uri_cancellable (uri, 
							info, 
							options,
							NULL);
}

/**
 * gnome_vfs_get_file_info_from_handle:
 * @handle: Handle of the file for which information must be retrieved
 * @info: Pointer to a GnomeVFSFileInfo object that will hold the information
 * for the file on return
 * @options: Options for retrieving file information
 * to retrieve for the file
 * 
 * Retrieve information about an open file.  The information will be stored in
 * @*info.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_get_file_info_from_handle (GnomeVFSHandle *handle,
				     GnomeVFSFileInfo *info,
				     GnomeVFSFileInfoOptions options)
{
	return gnome_vfs_get_file_info_from_handle_cancellable (handle, info,
								options,
								NULL);
}

GnomeVFSResult
gnome_vfs_truncate (const char *text_uri, GnomeVFSFileSize length)
{
	GnomeVFSURI *uri;
	GnomeVFSResult result;

	uri = gnome_vfs_uri_new (text_uri);

	if (uri == NULL)
		return GNOME_VFS_ERROR_NOT_SUPPORTED;

	result = gnome_vfs_truncate_uri(uri, length);
	gnome_vfs_uri_unref (uri);

	return result;
}


GnomeVFSResult
gnome_vfs_truncate_uri (GnomeVFSURI *uri, GnomeVFSFileSize length)
{
	return gnome_vfs_truncate_uri_cancellable(uri, length, NULL);
}

GnomeVFSResult
gnome_vfs_truncate_handle (GnomeVFSHandle *handle, GnomeVFSFileSize length)
{
	return gnome_vfs_truncate_handle_cancellable(handle, length, NULL);
}

/**
 * gnome_vfs_make_directory_for_uri:
 * @uri: URI of the directory to be created
 * @perm: Unix-style permissions for the newly created directory
 * 
 * Create @uri as a directory.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_make_directory_for_uri (GnomeVFSURI *uri,
				  guint perm)
{
	return gnome_vfs_make_directory_for_uri_cancellable (uri, perm, NULL);
}

/**
 * gnome_vfs_make_directory:
 * @text_uri: URI of the directory to be created
 * @perm: Unix-style permissions for the newly created directory
 * 
 * Create @text_uri as a directory.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_make_directory (const gchar *text_uri,
			  guint perm)
{
	GnomeVFSResult result;
	GnomeVFSURI *uri;

	g_return_val_if_fail (text_uri != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	uri = gnome_vfs_uri_new (text_uri);
	if (uri == NULL)
		return GNOME_VFS_ERROR_INVALID_URI;

	result = gnome_vfs_make_directory_for_uri (uri, perm);

	gnome_vfs_uri_unref (uri);

	return result;
}

/**
 * gnome_vfs_remove_directory_from_uri:
 * @uri: URI of the directory to be removed
 * 
 * Remove @uri.  @uri must be an empty directory.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_remove_directory_from_uri (GnomeVFSURI *uri)
{
	return gnome_vfs_remove_directory_from_uri_cancellable (uri, NULL);
}

/**
 * gnome_vfs_remove_directory_from:
 * @text_uri: URI of the directory to be removed
 * 
 * Remove @text_uri.  @uri must be an empty directory.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_remove_directory (const gchar *text_uri)
{
	GnomeVFSResult result;
	GnomeVFSURI *uri;

	g_return_val_if_fail (text_uri != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	uri = gnome_vfs_uri_new (text_uri);
	if (uri == NULL)
		return GNOME_VFS_ERROR_INVALID_URI;

	result = gnome_vfs_remove_directory_from_uri (uri);

	gnome_vfs_uri_unref (uri);

	return result;
}

/**
 * gnome_vfs_unlink_from_uri:
 * @uri: URI of the file to be unlinked
 * 
 * Unlink @uri.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_unlink_from_uri (GnomeVFSURI *uri)
{
	return gnome_vfs_unlink_from_uri_cancellable (uri, NULL);
}

/**
 * gnome_vfs_create_symbolic_link:
 * @uri: URI to create a link at
 * @target_reference: URI "reference" to point the link to (URI or relative path)
 *
 * Creates a symbolic link, or eventually, a URI link (as necessary) 
 * at @uri pointing to @target_reference
 *
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_create_symbolic_link (GnomeVFSURI *uri, const gchar *target_reference)
{
	return gnome_vfs_create_symbolic_link_cancellable (uri, target_reference, NULL);
}

/**
 * gnome_vfs_unlink_from_uri:
 * @text_uri: URI of the file to be unlinked
 * 
 * Unlink @text_uri.
 * 
 * Return value: An integer representing the result of the operation
 **/
GnomeVFSResult
gnome_vfs_unlink (const gchar *text_uri)
{
	GnomeVFSResult result;
	GnomeVFSURI *uri;

	g_return_val_if_fail (text_uri != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	uri = gnome_vfs_uri_new (text_uri);
	if (uri == NULL)
		return GNOME_VFS_ERROR_INVALID_URI;

	result = gnome_vfs_unlink_from_uri (uri);

	gnome_vfs_uri_unref (uri);

	return result;
}

/**
 * gnome_vfs_move_uri:
 * @old_uri: Source URI
 * @new_uri: Destination URI
 * 
 * Move a file from URI @old_uri to @new_uri.  This will only work if @old_uri 
 * and @new_uri are on the same file system.  Otherwise, it is necessary 
 * to use the more general %gnome_vfs_xfer_uri() function.
 * 
 * Return value: An integer representing the result of the operation.
 **/
GnomeVFSResult
gnome_vfs_move_uri (GnomeVFSURI *old_uri,
		    GnomeVFSURI *new_uri,
		    gboolean force_replace)
{
	return gnome_vfs_move_uri_cancellable (old_uri, new_uri, 
					       force_replace, NULL);
}

/**
 * gnome_vfs_move:
 * @old_text_uri: Source URI
 * @new_text_uri: Destination URI
 * 
 * Move a file from URI @old_text_uri to @new_text_uri.  This will only work 
 * if @old_text_uri and @new_text_uri are on the same file system.  Otherwise,
 * it is necessary to use the more general %gnome_vfs_xfer_uri() function.
 * 
 * Return value: An integer representing the result of the operation.
 **/
GnomeVFSResult
gnome_vfs_move (const gchar *old_text_uri,
		const gchar *new_text_uri,
		gboolean force_replace)
{
	GnomeVFSURI *old_uri, *new_uri;
	GnomeVFSResult retval;

	g_return_val_if_fail (old_text_uri != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (new_text_uri != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	old_uri = gnome_vfs_uri_new (old_text_uri);
	if (old_uri == NULL)
		return GNOME_VFS_ERROR_INVALID_URI;

	new_uri = gnome_vfs_uri_new (new_text_uri);
	if (new_uri == NULL) {
		gnome_vfs_uri_unref (old_uri);
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	retval = gnome_vfs_move_uri (old_uri, new_uri, force_replace);

	gnome_vfs_uri_unref (old_uri);
	gnome_vfs_uri_unref (new_uri);

	return retval;
}

/**
 * gnome_vfs_check_same_fs_uris:
 * @a: A URI
 * @b: Another URI
 * @same_fs_return: Pointer to a boolean variable which will be set to %TRUE
 * if @a and @b are on the same file system on return.
 * 
 * Check if @a and @b are on the same file system.
 * 
 * Return value: An integer representing the result of the operation.
 **/
GnomeVFSResult
gnome_vfs_check_same_fs_uris (GnomeVFSURI *a,
			      GnomeVFSURI *b,
			      gboolean *same_fs_return)
{
	return gnome_vfs_check_same_fs_uris_cancellable (a, b, same_fs_return,
							 NULL);
}

/**
 * gnome_vfs_check_same_fs:
 * @a: A URI
 * @b: Another URI
 * @same_fs_return: Pointer to a boolean variable which will be set to %TRUE
 * if @a and @b are on the same file system on return.
 * 
 * Check if @a and @b are on the same file system.
 * 
 * Return value: An integer representing the result of the operation.
 **/
GnomeVFSResult
gnome_vfs_check_same_fs (const gchar *a,
			 const gchar *b,
			 gboolean *same_fs_return)
{
	GnomeVFSURI *a_uri, *b_uri;
	GnomeVFSResult retval;

	g_return_val_if_fail (a != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (b != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (same_fs_return != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	*same_fs_return = FALSE;

	a_uri = gnome_vfs_uri_new (a);
	if (a_uri == NULL)
		return GNOME_VFS_ERROR_INVALID_URI;

	b_uri = gnome_vfs_uri_new (b);
	if (b_uri == NULL) {
		gnome_vfs_uri_unref (a_uri);
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	retval = gnome_vfs_check_same_fs_uris (a_uri, b_uri, same_fs_return);

	gnome_vfs_uri_unref (a_uri);
	gnome_vfs_uri_unref (b_uri);

	return retval;
}

/**
 * gnome_vfs_set_file_info_uri:
 * @uri: A URI
 * @info: Information that must be set for the file
 * @mask: Bit mask representing which fields of @info need to be set 
 * 
 * Set file information for @uri; only the information for which the
 * corresponding bit in @mask is set is actually modified.
 * 
 * Return value: An integer representing the result of the operation.
 **/
GnomeVFSResult
gnome_vfs_set_file_info_uri (GnomeVFSURI *uri,
			     GnomeVFSFileInfo *info,
			     GnomeVFSSetFileInfoMask mask)
{
	return gnome_vfs_set_file_info_cancellable (uri, info, mask, NULL);
}

/**
 * gnome_vfs_set_file_info:
 * @text_uri: A URI
 * @info: Information that must be set for the file
 * @mask: Bit mask representing which fields of @info need to be set 
 * 
 * Set file information for @uri; only the information for which the
 * corresponding bit in @mask is set is actually modified.
 * 
 * Return value: An integer representing the result of the operation.
 **/
GnomeVFSResult
gnome_vfs_set_file_info (const gchar *text_uri,
			 GnomeVFSFileInfo *info,
			 GnomeVFSSetFileInfoMask mask)
{
	GnomeVFSURI *uri;
	GnomeVFSResult result;

	uri = gnome_vfs_uri_new (text_uri);
	if (uri == NULL)
		return GNOME_VFS_ERROR_INVALID_URI;

	result = gnome_vfs_set_file_info_uri (uri, info, mask);

	gnome_vfs_uri_unref (uri);

	return result;
}

/**
 * gnome_vfs_uri_exists:
 * @uri: A URI
 * 
 * Check if the URI points to an existing entity.
 * 
 * Return value: TRUE if URI exists.
 **/
gboolean
gnome_vfs_uri_exists (GnomeVFSURI *uri)
{
	GnomeVFSFileInfo *info;
	GnomeVFSResult result;

	info = gnome_vfs_file_info_new ();
	result = gnome_vfs_get_file_info_uri (uri, info, GNOME_VFS_FILE_INFO_DEFAULT);
	gnome_vfs_file_info_unref (info);

	return result == GNOME_VFS_OK;
}
