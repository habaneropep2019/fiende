/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-seekable.c - Emulation of seek / tell for non seekable filesystems.

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

   Author: Michael Meeks <michael@imaginator.com>
*/

/* TODO: Cancellation throughout!  */

#include <config.h>
#include "gnome-vfs-seekable.h"

#include "gnome-vfs-private.h"
#include "gnome-vfs.h"
#include <glib.h>
#include <stdio.h>
#include <stdlib.h> /* for mkstemp */
#include <string.h>
#include <unistd.h> /* for close */

static GnomeVFSResult	do_open		(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle **method_handle,
					 GnomeVFSURI *uri,
					 GnomeVFSOpenMode mode,
					 GnomeVFSContext *context);
static GnomeVFSResult	do_create 	(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle **method_handle,
					 GnomeVFSURI *uri,
					 GnomeVFSOpenMode mode,
					 gboolean exclusive,
					 guint perm,
					 GnomeVFSContext *context);
static GnomeVFSResult	do_close	(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSContext *context);
static GnomeVFSResult	do_read		(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 gpointer buffer,
					 GnomeVFSFileSize num_bytes,
					 GnomeVFSFileSize *bytes_read,
					 GnomeVFSContext *context);
static GnomeVFSResult	do_write	(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 gconstpointer buffer,
					 GnomeVFSFileSize num_bytes,
					 GnomeVFSFileSize *bytes_written,
					 GnomeVFSContext *context);
static GnomeVFSResult   do_seek		(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSSeekPosition whence,
					 GnomeVFSFileOffset offset,
					 GnomeVFSContext *context);
static GnomeVFSResult	do_tell		(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSFileOffset *offset_return);
static GnomeVFSResult	do_truncate_handle 	(GnomeVFSMethod *method,
						 GnomeVFSMethodHandle *method_handle,
						 GnomeVFSFileSize where,
						 GnomeVFSContext *context);

/* Our method_handle */
typedef struct  {
	/* Child chaining info */
	GnomeVFSMethodHandle *child_handle;
	GnomeVFSMethod      *child_method;

	/* Housekeeping info */
	GnomeVFSHandle       *tmp_file;
	gchar                *tmp_uri;
	GnomeVFSOpenMode      open_mode;
	gboolean              dirty;

	/* Each SeekableMethodHandle has a unique wrapper method */
	GnomeVFSMethod      *wrapper_method;
} SeekableMethodHandle;

#define CHECK_IF_SUPPORTED(method, what)		\
G_STMT_START{						\
	if (!VFS_METHOD_HAS_FUNC(method, what))	\
		return GNOME_VFS_ERROR_NOT_SUPPORTED;	\
}G_STMT_END

#define INVOKE_CHILD(result, method, what, params)	\
G_STMT_START{						\
	CHECK_IF_SUPPORTED (method->child_method, what);\
	(result) = method->child_method->what params;	\
}G_STMT_END

#define CHECK_INIT(handle)			\
G_STMT_START{					\
	if (!handle->tmp_file) {		\
		GnomeVFSResult result;		\
		result = init_seek (handle);	\
		if (result != GNOME_VFS_OK)	\
			return result;		\
	}					\
}G_STMT_END

#define BLK_SIZE 4096

static GnomeVFSResult
read_file (SeekableMethodHandle *mh)
{
	GnomeVFSResult   result;
	guint8           buffer[BLK_SIZE];
	GnomeVFSFileSize blk_read, blk_write;

	g_return_val_if_fail (mh != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	do {
		INVOKE_CHILD (result, mh, read, (mh->child_method, mh->child_handle, buffer, BLK_SIZE, &blk_read,
						 NULL));
		if (result != GNOME_VFS_OK)
			return result;
		result = gnome_vfs_write (mh->tmp_file, buffer, blk_read, &blk_write);
		if (result != GNOME_VFS_OK)
			return result;
		if (blk_write != blk_read)
			return GNOME_VFS_ERROR_NO_SPACE;
		
	} while (blk_read > 0);
	//} while (blk_read == BLK_SIZE);

	result = gnome_vfs_seek (mh->tmp_file, GNOME_VFS_SEEK_START, 0);

	return result;
}

static GnomeVFSResult
write_file (SeekableMethodHandle *mh)
{
	GnomeVFSResult   result;
	guint8           buffer[BLK_SIZE];
	GnomeVFSFileSize blk_read, blk_write;

	g_return_val_if_fail (mh != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	do {
		result = gnome_vfs_read (mh->tmp_file, buffer, BLK_SIZE,
					 &blk_read);
		if (result != GNOME_VFS_OK)
			return result;
		INVOKE_CHILD (result, mh, write, (mh->child_method, mh->child_handle, buffer,
						  blk_read, &blk_write, NULL));
		if (result != GNOME_VFS_OK)
			return result;
		if (blk_write != blk_read)
			return GNOME_VFS_ERROR_NO_SPACE;
		
	} while (blk_read == BLK_SIZE);

	return GNOME_VFS_OK;
}

#undef  BLK_SIZE

static GnomeVFSResult
init_seek (SeekableMethodHandle *mh)
{
	GnomeVFSResult   result;
	gchar            *stem;
	gint             fd;

	/* Create a temporary file name */
	stem = g_strdup("/tmp/gnome-vfs-seekable-temp-XXXXXX"); /* template */
	fd = mkstemp(stem);
	if (fd < 0) {
		g_free(stem);
		return GNOME_VFS_ERROR_NO_SPACE;
	}

	mh->tmp_uri = g_strdup_printf ("file:%s", stem);
	g_warning ("Opening temp seekable file '%s'", mh->tmp_uri);

	close(fd);
	g_free(stem);
	
	/* Open the file */
	result = gnome_vfs_open (&mh->tmp_file, mh->tmp_uri, 
				   GNOME_VFS_OPEN_READ | GNOME_VFS_OPEN_WRITE |
				   GNOME_VFS_OPEN_RANDOM);

	if (result != GNOME_VFS_OK)
		return result;

	mh->dirty = FALSE;

	if (mh->open_mode & GNOME_VFS_OPEN_READ)
		return read_file (mh);
	else
		return GNOME_VFS_OK;
}

GnomeVFSMethodHandle *
gnome_vfs_seek_emulate (GnomeVFSURI *uri, GnomeVFSMethodHandle *child_handle,
			GnomeVFSOpenMode open_mode)
{
	GnomeVFSMethod      *m  = g_new (GnomeVFSMethod, 1);
	SeekableMethodHandle *mh = g_new (SeekableMethodHandle, 1);

	g_return_val_if_fail (m != NULL, NULL);
	g_return_val_if_fail (mh != NULL, NULL);
	g_return_val_if_fail (uri != NULL, NULL);
	g_return_val_if_fail (uri->method != NULL, NULL);

	memcpy (m, uri->method, uri->method->method_table_size);

        /*
	 *  This subset of method contains those operations that we need
	 * to wrap in order to extract the neccessary information for
	 * seek / tell.
	 */
	m->open     = do_open;
	m->create   = do_create;
	m->close    = do_close;
	m->read     = do_read;
	m->write    = do_write;
	m->seek     = do_seek;
	m->tell     = do_tell;
	m->truncate_handle = do_truncate_handle;

	mh->child_handle   = child_handle;
	mh->child_method   = uri->method;
	mh->open_mode      = open_mode;
	mh->tmp_file       = NULL;
	mh->tmp_uri        = NULL;
	mh->wrapper_method = m;

	uri->method        = m;

	return (GnomeVFSMethodHandle *)mh;
}

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{
	g_warning ("FIXME bugzilla.eazel.com 1192: Unhandled re-open");
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}

static GnomeVFSResult
do_create (GnomeVFSMethod *method,
	   GnomeVFSMethodHandle **method_handle,
	   GnomeVFSURI *uri,
	   GnomeVFSOpenMode mode,
	   gboolean exclusive,
	   guint perm,
	   GnomeVFSContext *context)
{
	g_warning ("FIXME bugzilla.eazel.com 1192: Unhandled re-create");
	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)
{
	GnomeVFSResult result;
	SeekableMethodHandle *mh = (SeekableMethodHandle *)method_handle;

	if ((mh->open_mode & GNOME_VFS_OPEN_WRITE) &&
	    mh->dirty)
		write_file (mh);

	result = gnome_vfs_close (mh->tmp_file);
	mh->tmp_file = NULL;

	if (mh->tmp_uri) {
		if (result == GNOME_VFS_OK)
			result = gnome_vfs_unlink (mh->tmp_uri);
		g_free (mh->tmp_uri);
		mh->tmp_uri  = NULL;
	}

	INVOKE_CHILD (result, mh, close, (mh->child_method, mh->child_handle, NULL));

	/* Cover your back. */
	memset (mh->wrapper_method, 0xae, sizeof (GnomeVFSMethod));

	g_free (mh->wrapper_method);
	mh->wrapper_method = NULL;

	g_free (mh);

	return result;
}

static GnomeVFSResult
do_read (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read,
	 GnomeVFSContext *context)
{
	SeekableMethodHandle *mh = (SeekableMethodHandle *)method_handle;
	CHECK_INIT (mh);

	return gnome_vfs_read (mh->tmp_file, buffer, num_bytes, bytes_read);
}

static GnomeVFSResult
do_write (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_written,
	  GnomeVFSContext *context)
{
	SeekableMethodHandle *mh = (SeekableMethodHandle *)method_handle;
	CHECK_INIT (mh);
	mh->dirty = TRUE;
	return gnome_vfs_write (mh->tmp_file, buffer, num_bytes, bytes_written);
}

static GnomeVFSResult
do_seek (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSSeekPosition whence,
	 GnomeVFSFileOffset offset,
	 GnomeVFSContext *context)
{
	SeekableMethodHandle *mh = (SeekableMethodHandle *)method_handle;
	CHECK_INIT (mh);
	
	return gnome_vfs_seek (mh->tmp_file, whence, offset);
}

static GnomeVFSResult
do_tell (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSFileOffset *offset_return)
{
	SeekableMethodHandle *mh = (SeekableMethodHandle *)method_handle;
	CHECK_INIT (mh);

	return gnome_vfs_tell (mh->tmp_file, offset_return);
}

static GnomeVFSResult
do_truncate_handle (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSFileSize where,
		    GnomeVFSContext *context)
{
	SeekableMethodHandle *mh = (SeekableMethodHandle *)method_handle;
	CHECK_INIT (mh);

	g_warning ("FIXME bugzilla.eazel.com 1194: truncate needs implementing");
	mh->dirty = TRUE;

	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}
