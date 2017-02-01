/*
 * bzip2-method.c - Bzip2 access method for the GNOME Virtual File
 *                  System.
 *
 * Copyright (C) 1999 Free Software Foundation
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite
 * 330, Boston, MA 02111-1307, USA.
 *
 * Author: Cody Russell <bratsche@dfw.net>
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <bzlib.h>
#include <time.h>
#include <string.h>

#include "gnome-vfs-handle.h"
#include "gnome-vfs-module.h"
#include "gnome-vfs-mime.h"
#include "gnome-vfs-ops.h"

#ifdef HAVE_OLDER_BZIP2
#define BZ2_bzDecompressInit  bzDecompressInit
#define BZ2_bzCompressInit    bzCompressInit
#define BZ2_bzDecompress      bzDecompress
#define BZ2_bzCompress        bzCompress
#endif

#define BZ_BUFSIZE   5000

struct _Bzip2MethodHandle {
	GnomeVFSURI      *uri;
	GnomeVFSHandle   *parent_handle;
	GnomeVFSOpenMode open_mode;

	BZFILE           *file;
	GnomeVFSResult   last_vfs_result;
	gint             last_bz_result;
	bz_stream        bzstream;
	guchar           *buffer;
};

typedef struct _Bzip2MethodHandle Bzip2MethodHandle;

static GnomeVFSResult do_open (GnomeVFSMethod *method,
			       GnomeVFSMethodHandle **method_handle,
			       GnomeVFSURI *uri,
			       GnomeVFSOpenMode mode,
			       GnomeVFSContext *context);

static GnomeVFSResult do_create (GnomeVFSMethod *method,
				 GnomeVFSMethodHandle **method_handle,
				 GnomeVFSURI *uri,
				 GnomeVFSOpenMode mode,
				 gboolean exclusive,
				 guint perm,
				 GnomeVFSContext *context);

static GnomeVFSResult do_close (GnomeVFSMethod *method,
				GnomeVFSMethodHandle *method_handle,
				GnomeVFSContext *context);

static GnomeVFSResult do_read (GnomeVFSMethod *method,
			       GnomeVFSMethodHandle *method_handle,
			       gpointer buffer,
			       GnomeVFSFileSize num_bytes,
			       GnomeVFSFileSize *bytes_read,
			       GnomeVFSContext *context);

static GnomeVFSResult do_write (GnomeVFSMethod *method,
				GnomeVFSMethodHandle *method_handle,
				gconstpointer buffer,
				GnomeVFSFileSize num_bytes,
				GnomeVFSFileSize *bytes_written,
				GnomeVFSContext *context);

static GnomeVFSResult do_get_file_info  (GnomeVFSMethod *method,
		                         GnomeVFSURI *uri,
				         GnomeVFSFileInfo *file_info,
					 GnomeVFSFileInfoOptions options,
					 GnomeVFSContext *context);

static gboolean do_is_local (GnomeVFSMethod *method, const GnomeVFSURI *uri);

static GnomeVFSMethod method = {
	sizeof (GnomeVFSMethod),
	do_open,
	do_create,
	do_close,
	do_read,
	do_write,
	NULL,		/* seek            */
	NULL,		/* tell            */
	NULL,		/* truncate_handle */
	NULL,		/* open_directory  */
	NULL,		/* close_directory */
	NULL,		/* read_directory  */
	do_get_file_info,
	NULL,		/* get_file_info_from_handle */
	do_is_local,
	NULL,		/* make_directory  */
	NULL,		/* remove_directory */
	NULL,		/* move */
	NULL,		/* unlink */
	NULL,		/* set_file_info */
	NULL, 		/* truncate */
	NULL, 		/* find_directory */
	NULL            /* create_symbolic_link */
};

#define RETURN_IF_FAIL(action)			\
G_STMT_START {					\
	GnomeVFSResult __tmp_result;		\
						\
	__tmp_result = (action);		\
	if (__tmp_result != GNOME_VFS_OK)	\
		return __tmp_result;		\
} G_STMT_END

#define VALID_URI(u) ((u)->parent!=NULL&&(((u)->text==NULL)||((u)->text[0]=='\0')||(((u)->text[0]=='/')&&((u)->text[1]=='\0'))))

static Bzip2MethodHandle *
bzip2_method_handle_new (GnomeVFSHandle *parent_handle,
			 GnomeVFSURI *uri,
			 GnomeVFSOpenMode open_mode)
{
	Bzip2MethodHandle *new;

	new = g_new (Bzip2MethodHandle, 1);

	new->parent_handle = parent_handle;
	new->uri = gnome_vfs_uri_ref (uri);
	new->open_mode = open_mode;

	new->buffer = NULL;

	return new;
}

static void
bzip2_method_handle_destroy (Bzip2MethodHandle *handle)
{
	gnome_vfs_uri_unref (handle->uri);
	g_free (handle->buffer);
	g_free (handle);
}

static gboolean
bzip2_method_handle_init_for_decompress (Bzip2MethodHandle *handle)
{
	handle->bzstream.bzalloc = NULL;
	handle->bzstream.bzfree  = NULL;
	handle->bzstream.opaque  = NULL;

	g_free (handle->buffer);

	handle->buffer = g_malloc (BZ_BUFSIZE);
	handle->bzstream.next_in = handle->buffer;
	handle->bzstream.avail_in = 0;

	/* FIXME bugzilla.eazel.com 1177: Make small, and possibly verbosity, configurable! */
	if (BZ2_bzDecompressInit (&handle->bzstream, 0, 0) != BZ_OK) {
		g_free (handle->buffer);
		return FALSE;
	}

	handle->last_bz_result = BZ_OK;
	handle->last_vfs_result = GNOME_VFS_OK;

	return TRUE;
}

static gboolean
bzip2_method_handle_init_for_compress (Bzip2MethodHandle *handle) G_GNUC_UNUSED;

static gboolean
bzip2_method_handle_init_for_compress (Bzip2MethodHandle *handle)
{
	handle->bzstream.bzalloc = NULL;
	handle->bzstream.bzfree  = NULL;
	handle->bzstream.opaque  = NULL;

	g_free (handle->buffer);

	handle->buffer = g_malloc (BZ_BUFSIZE);
	handle->bzstream.next_out = handle->buffer;
	handle->bzstream.avail_out = BZ_BUFSIZE;

	/* FIXME bugzilla.eazel.com 1174: We want this to be user configurable.  */
	if (BZ2_bzCompressInit (&handle->bzstream, 3, 0, 30) != BZ_OK) {
		g_free (handle->buffer);
		return FALSE;
	}

	handle->last_bz_result = BZ_OK;
	handle->last_vfs_result = GNOME_VFS_OK;

	return TRUE;
}

static GnomeVFSResult
result_from_bz_result (gint bz_result)
{
	switch (bz_result) {
	case BZ_OK:
	case BZ_STREAM_END:
		return GNOME_VFS_OK;

	case BZ_MEM_ERROR:
		return GNOME_VFS_ERROR_NO_MEMORY;

	case BZ_PARAM_ERROR:
		return GNOME_VFS_ERROR_BAD_PARAMETERS;

	case BZ_DATA_ERROR:
		return GNOME_VFS_ERROR_CORRUPTED_DATA;

	case BZ_UNEXPECTED_EOF:
		return GNOME_VFS_ERROR_EOF;

	case BZ_SEQUENCE_ERROR:
		return GNOME_VFS_ERROR_NOT_PERMITTED;

	default:
		return GNOME_VFS_ERROR_INTERNAL;
	}
}

static GnomeVFSResult
flush_write (Bzip2MethodHandle *bzip2_handle)
{
	GnomeVFSHandle *parent_handle;
	GnomeVFSResult result;
	gboolean done;
	bz_stream *bzstream;
	gint bz_result;

	bzstream = &bzip2_handle->bzstream;
	bzstream->avail_in = 0;
	parent_handle = bzip2_handle->parent_handle;

	done = FALSE;
	bz_result = BZ_OK;
	while (bz_result == BZ_OK || bz_result == BZ_STREAM_END) {
		GnomeVFSFileSize bytes_written;
		GnomeVFSFileSize len;

		len = BZ_BUFSIZE - bzstream->avail_out;

		result = gnome_vfs_write (parent_handle, bzip2_handle->buffer,
					  len, &bytes_written);
		RETURN_IF_FAIL (result);

		bzstream->next_out = bzip2_handle->buffer;
		bzstream->avail_out = BZ_BUFSIZE;

		if (done)
			break;

		bz_result = BZ2_bzCompress (bzstream, BZ_FINISH);

		done = (bzstream->avail_out != 0 || bz_result == BZ_STREAM_END);
	}

	if (bz_result == BZ_OK || bz_result == BZ_STREAM_END)
		return GNOME_VFS_OK;
	else
		return result_from_bz_result (bz_result);
}

/* Open */
/* TODO: Check that there is no subpath. */

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode open_mode,
	 GnomeVFSContext *context)
{
	GnomeVFSHandle *parent_handle;
	GnomeVFSURI *parent_uri;
	GnomeVFSResult result;
	Bzip2MethodHandle *bzip2_handle;

	_GNOME_VFS_METHOD_PARAM_CHECK (method_handle != NULL);
	_GNOME_VFS_METHOD_PARAM_CHECK (uri != NULL);

	/* Check that the URI is valid.  */
	if (!VALID_URI(uri)) return GNOME_VFS_ERROR_INVALID_URI;

	if (open_mode & GNOME_VFS_OPEN_WRITE)
		return GNOME_VFS_ERROR_INVALID_OPEN_MODE;

	parent_uri = uri->parent;

	if (open_mode & GNOME_VFS_OPEN_RANDOM)
		return GNOME_VFS_ERROR_NOT_SUPPORTED;

	result = gnome_vfs_open_uri (&parent_handle, parent_uri, open_mode);
	RETURN_IF_FAIL (result);

	bzip2_handle = bzip2_method_handle_new (parent_handle, uri, open_mode);

	if (result != GNOME_VFS_OK) {
		gnome_vfs_close (parent_handle);
		bzip2_method_handle_destroy (bzip2_handle);
		return result;
	}

	if (!bzip2_method_handle_init_for_decompress (bzip2_handle)) {
		gnome_vfs_close (parent_handle);
		bzip2_method_handle_destroy (bzip2_handle);
		return GNOME_VFS_ERROR_INTERNAL;
	}

	*method_handle = (GnomeVFSMethodHandle *) bzip2_handle;

	return GNOME_VFS_OK;
}

/* Create */

static GnomeVFSResult
do_create (GnomeVFSMethod *method,
	   GnomeVFSMethodHandle **method_handle,
	   GnomeVFSURI *uri,
	   GnomeVFSOpenMode mode,
	   gboolean exclusive,
	   guint perm,
	   GnomeVFSContext *context)
{
	_GNOME_VFS_METHOD_PARAM_CHECK (method_handle != NULL);
	_GNOME_VFS_METHOD_PARAM_CHECK (uri != NULL);

	return GNOME_VFS_ERROR_NOT_SUPPORTED;
}

/* Close */

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)
{
	Bzip2MethodHandle *bzip2_handle;
	GnomeVFSResult result;

	_GNOME_VFS_METHOD_PARAM_CHECK (method_handle != NULL);

	bzip2_handle = (Bzip2MethodHandle *) method_handle;

	if (bzip2_handle->open_mode & GNOME_VFS_OPEN_WRITE)
		result = flush_write (bzip2_handle);
	else
		result = GNOME_VFS_OK;

	if (result == GNOME_VFS_OK)
		result = gnome_vfs_close (bzip2_handle->parent_handle);

	bzip2_method_handle_destroy (bzip2_handle);

	return result;
}

/* Read */

static GnomeVFSResult
fill_buffer (Bzip2MethodHandle *bzip2_handle,
	     GnomeVFSFileSize num_bytes)
{
	GnomeVFSResult result;
	GnomeVFSFileSize count;
	bz_stream *bzstream;

	bzstream = &bzip2_handle->bzstream;

	if (bzstream->avail_in > 0)
		return GNOME_VFS_OK;

	result = gnome_vfs_read (bzip2_handle->parent_handle,
				 bzip2_handle->buffer,
				 BZ_BUFSIZE,
				 &count);

	if (result != GNOME_VFS_OK) {
		if (bzstream->avail_out == num_bytes)
			return result;
		bzip2_handle->last_vfs_result = result;
	} else {
		bzstream->next_in = bzip2_handle->buffer;
		bzstream->avail_in = count;
	}

	return GNOME_VFS_OK;
}

/* TODO: Concatenated Bzip2 file handling. */

static GnomeVFSResult
do_read (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read,
	 GnomeVFSContext *context)
{
	Bzip2MethodHandle *bzip2_handle;
	GnomeVFSResult result;
	bz_stream *bzstream;
	int bz_result;

	*bytes_read = 0;

	bzip2_handle = (Bzip2MethodHandle *) method_handle;
	bzstream = &bzip2_handle->bzstream;

	if (bzip2_handle->last_bz_result != BZ_OK) {
		if (bzip2_handle->last_bz_result == BZ_STREAM_END)
			return GNOME_VFS_OK;
		else
			return result_from_bz_result (bzip2_handle->last_bz_result);
	} else if (bzip2_handle->last_vfs_result != GNOME_VFS_OK) {
		return bzip2_handle->last_vfs_result;
	}

	bzstream->next_out = buffer;
	bzstream->avail_out = num_bytes;

	while (bzstream->avail_out != 0) {
		result = fill_buffer (bzip2_handle, num_bytes);
		RETURN_IF_FAIL (result);

		bz_result = BZ2_bzDecompress (&bzip2_handle->bzstream);

		if (bzip2_handle->last_bz_result != BZ_OK
		    && bzstream->avail_out == num_bytes) {
			bzip2_handle->last_bz_result = bz_result;
			return result_from_bz_result (bzip2_handle->last_bz_result);
		}

		*bytes_read = num_bytes - bzstream->avail_out;

		if (bz_result == BZ_STREAM_END) {
			bzip2_handle->last_bz_result = bz_result;
			break;
		}

	}

	return GNOME_VFS_OK;
}

/* Write. */

static GnomeVFSResult
do_write (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_written,
	  GnomeVFSContext *context)
{
	Bzip2MethodHandle *bzip2_handle;
	GnomeVFSResult result;
	bz_stream *bzstream;
	gint bz_result;

	bzip2_handle = (Bzip2MethodHandle *) method_handle;
	bzstream = &bzip2_handle->bzstream;

	bzstream->next_in = (gpointer) buffer;
	bzstream->avail_in = num_bytes;

	result = GNOME_VFS_OK;

	while (bzstream->avail_in != 0 && result == GNOME_VFS_OK) {
		if (bzstream->avail_out == 0) {
			GnomeVFSFileSize written;

			bzstream->next_out = bzip2_handle->buffer;
			result = gnome_vfs_write (bzip2_handle->parent_handle,
						  bzip2_handle->buffer,
						  BZ_BUFSIZE, &written);
			if (result != GNOME_VFS_OK)
				break;

			bzstream->avail_out += written;
		}

		bz_result = BZ2_bzCompress (bzstream, BZ_RUN);
		result = result_from_bz_result (bz_result);
	}

	*bytes_written = num_bytes - bzstream->avail_in;

	return result;
}

static gboolean
do_is_local (GnomeVFSMethod *method, const GnomeVFSURI *uri)
{
	g_return_val_if_fail (uri != NULL, FALSE);
	return gnome_vfs_uri_is_local (uri->parent);
}

static GnomeVFSResult 
do_get_file_info  (GnomeVFSMethod *method,
	           GnomeVFSURI *uri,
		   GnomeVFSFileInfo *file_info,
		   GnomeVFSFileInfoOptions options,
		   GnomeVFSContext *context) 
{
	GnomeVFSResult result;

	/* Check that the URI is valid.  */
	if (!VALID_URI(uri)) return GNOME_VFS_ERROR_INVALID_URI;

	result = gnome_vfs_get_file_info_uri(uri->parent, file_info, options);

	if(result == GNOME_VFS_OK) {
		gint namelen = strlen(file_info->name);
		
		/* work out the name */
		/* FIXME bugzilla.eazel.com 2790: handle uppercase */
		if(namelen > 4 &&
				file_info->name[namelen-1] == '2' &&
				file_info->name[namelen-2] == 'z' &&
				file_info->name[namelen-3] == 'b' &&
				file_info->name[namelen-4] == '.')
			file_info->name[namelen-4] = '\0';

		/* we can't tell the size without uncompressing it */
		//file_info->valid_fields &= ~GNOME_VFS_FILE_INFO_FIELDS_SIZE;

		/* guess the mime type of the file inside */
		/* FIXME bugzilla.eazel.com 2791: guess mime based on contents */
		g_free(file_info->mime_type);
		file_info->mime_type = g_strdup(gnome_vfs_mime_type_from_name(file_info->name));
	}

	return result;
}

GnomeVFSMethod *
vfs_module_init (const char *method_name, const char *args)
{
	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
	return;
}
