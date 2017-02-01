/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * gnome-vfs-mime-sniff-buffer.c
 * Utility for implementing gnome_vfs_mime_type_from_magic, and other
 * mime-type sniffing calls.
 *
 * Copyright (C) 2000 Eazel, Inc.
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
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
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include "gnome-vfs-mime-sniff-buffer.h"

#include "gnome-vfs-handle.h"
#include "gnome-vfs-mime-sniff-buffer-private.h"
#include "gnome-vfs-ops.h"
#include <string.h>

static GnomeVFSResult
handle_seek_glue (gpointer context, GnomeVFSSeekPosition whence, 
	GnomeVFSFileOffset offset)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle *)context;
	return gnome_vfs_seek (handle, whence, offset);
}

static GnomeVFSResult
handle_read_glue (gpointer context, gpointer buffer, 
	GnomeVFSFileSize bytes, GnomeVFSFileSize *bytes_read)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle *)context;
	return gnome_vfs_read (handle, buffer, bytes, bytes_read);
}

GnomeVFSMimeSniffBuffer *
gnome_vfs_mime_sniff_buffer_new_from_handle (GnomeVFSHandle *file)
{
	GnomeVFSMimeSniffBuffer *result;

	result = g_new0 (GnomeVFSMimeSniffBuffer, 1);
	result->owning = TRUE;
	result->context = file;
	result->seek = handle_seek_glue;
	result->read = handle_read_glue;

	return result;
}

GnomeVFSMimeSniffBuffer	*
gnome_vfs_mime_sniff_buffer_new_generic (GnomeVFSSniffBufferSeekCall seek_callback, 
					 GnomeVFSSniffBufferReadCall read_callback,
					 gpointer context)
{
	GnomeVFSMimeSniffBuffer	* result;
	
	result = g_new0 (GnomeVFSMimeSniffBuffer, 1);

	result->owning = TRUE;
	result->seek = seek_callback;
	result->read = read_callback;
	result->context = context;

	return result;
}

GnomeVFSMimeSniffBuffer * 
gnome_vfs_mime_sniff_buffer_new_from_memory (const guchar *buffer, 
					     gssize buffer_length)
{
	GnomeVFSMimeSniffBuffer *result;

	result = g_new0 (GnomeVFSMimeSniffBuffer, 1);
	result->owning = TRUE;
	result->buffer = g_malloc (buffer_length);
	result->buffer_length = buffer_length;
	memcpy (result->buffer, buffer, buffer_length);
	result->read_whole_file = TRUE;

	return result;
}

GnomeVFSMimeSniffBuffer	*
gnome_vfs_mime_sniff_buffer_new_from_existing_data (const guchar *buffer, 
					 	    gssize buffer_length)
{
	GnomeVFSMimeSniffBuffer *result;

	result = g_new0 (GnomeVFSMimeSniffBuffer, 1);
	result->owning = FALSE;
	result->buffer = (guchar *)buffer;
	result->buffer_length = buffer_length;
	result->read_whole_file = TRUE;

	return result;
}

void
gnome_vfs_mime_sniff_buffer_free (GnomeVFSMimeSniffBuffer *buffer)
{
	if (buffer->owning)
		g_free (buffer->buffer);
	g_free (buffer);
}

enum {
	GNOME_VFS_SNIFF_BUFFER_INITIAL_CHUNK = 256,
	GNOME_VFS_SNIFF_BUFFER_MIN_CHUNK = 128
};

GnomeVFSResult
gnome_vfs_mime_sniff_buffer_get (GnomeVFSMimeSniffBuffer *buffer,
				 gssize size)
{
	GnomeVFSResult result;
	GnomeVFSFileSize bytes_to_read, bytes_read;

	/* check to see if we already have enough data */
	if (buffer->buffer_length >= size) {
		return GNOME_VFS_OK;
	}

	/* if we've read the whole file, don't try to read any more */
	if (buffer->read_whole_file) {
		return GNOME_VFS_ERROR_EOF;
	}

	/* figure out how much to read */
	bytes_to_read = size - buffer->buffer_length;

	/* don't bother to read less than this */
	if (bytes_to_read < GNOME_VFS_SNIFF_BUFFER_MIN_CHUNK) {
		bytes_to_read = GNOME_VFS_SNIFF_BUFFER_MIN_CHUNK;
	}

	/* make room in buffer for new data */
	buffer->buffer = g_realloc (buffer->buffer,
				    buffer->buffer_length + bytes_to_read);

	/* read in more data */
	result = (* buffer->read) (buffer->context, 
				   buffer->buffer + buffer->buffer_length,
				   bytes_to_read,
				   &bytes_read);
	if (result == GNOME_VFS_ERROR_EOF) {
		/* only happens with 0-byte files, due to other logic */
		buffer->read_whole_file = TRUE;
	}
	if (result != GNOME_VFS_OK) {
		return result;
	}
	if (bytes_read < bytes_to_read) {
		buffer->read_whole_file = TRUE;
	}
	buffer->buffer_length += bytes_read;

	/* check to see if we have enough data */
	if (buffer->buffer_length >= size) {
		return GNOME_VFS_OK;
	}

	/* not enough data */
	return GNOME_VFS_ERROR_EOF;
}
