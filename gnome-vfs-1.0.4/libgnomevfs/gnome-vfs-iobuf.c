/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-iobuf.c - Buffered I/O for the GNOME Virtual File System.

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
#include "gnome-vfs-iobuf.h"

#include <unistd.h>
#include <string.h>

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"


#define BUFFER_SIZE 4096

struct Buffer {
	gchar data[BUFFER_SIZE];
	guint offset;
	guint byte_count;
	GnomeVFSResult last_error;
};
typedef struct Buffer Buffer;

struct GnomeVFSIOBuf {
	gint fd;

	Buffer input_buffer;
	Buffer output_buffer;
};


static void
buffer_init (Buffer *buffer)
{
	buffer->byte_count = 0;
	buffer->offset = 0;
	buffer->last_error = GNOME_VFS_OK;
}

GnomeVFSIOBuf *
gnome_vfs_iobuf_new (gint fd)
{
	GnomeVFSIOBuf *new;

	g_return_val_if_fail (fd >= 0, NULL);

	new = g_new (GnomeVFSIOBuf, 1);

	new->fd = fd;

	buffer_init (&new->input_buffer);
	buffer_init (&new->output_buffer);

	return new;
}

void
gnome_vfs_iobuf_destroy (GnomeVFSIOBuf *iobuf)
{
	g_free (iobuf);
}

static gboolean
refill_input_buffer (GnomeVFSIOBuf *iobuf)
{
	Buffer *input_buffer;
	gint r;

	input_buffer = &iobuf->input_buffer;
	if (input_buffer->last_error != GNOME_VFS_OK
	    || input_buffer->byte_count > 0)
		return FALSE;

	input_buffer->offset = 0;

	r = read (iobuf->fd, &input_buffer->data, BUFFER_SIZE);
	if (r == -1) {
		input_buffer->last_error = gnome_vfs_result_from_errno ();
		return FALSE;
	}
	if (r == 0) {
		input_buffer->last_error = GNOME_VFS_ERROR_EOF;
		return FALSE;
	}

	input_buffer->byte_count = r;

	return TRUE;
}

GnomeVFSResult
gnome_vfs_iobuf_read (GnomeVFSIOBuf *iobuf,
		      gpointer buffer,
		      GnomeVFSFileSize bytes,
		      GnomeVFSFileSize *bytes_read)
{
	Buffer *input_buffer;
	GnomeVFSResult result;
	GnomeVFSFileSize n;

	g_return_val_if_fail (iobuf != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (buffer != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	/* Quote from UNIX 98:
	   "If nbyte is 0, read() will return 0 and have no other results."
	*/
	if (bytes == 0) {
		*bytes_read = 0;
		return GNOME_VFS_OK;
	}
		
	input_buffer = &iobuf->input_buffer;

	result = GNOME_VFS_OK;

	if (input_buffer->byte_count == 0) {
		if (! refill_input_buffer (iobuf)) {
			/* The buffer is empty but we had an error last time we
			   filled it, so we report the error.  */
			result = input_buffer->last_error;
			input_buffer->last_error = GNOME_VFS_OK;
		}
	}

	if (input_buffer->byte_count != 0) {
		n = MIN (bytes, input_buffer->byte_count);
		memcpy (buffer, input_buffer->data + input_buffer->offset, n);
		input_buffer->byte_count -= n;
		input_buffer->offset += n;
		if (bytes_read != NULL)
			*bytes_read = n;
	} else {
		if (bytes_read != NULL)
			*bytes_read = 0;
	}

	if (result == GNOME_VFS_ERROR_EOF)
		result = GNOME_VFS_OK;
	return result;
}

GnomeVFSResult
gnome_vfs_iobuf_peekc (GnomeVFSIOBuf *iobuf,
		       gchar *c)
{
	GnomeVFSResult result;
	Buffer *input_buffer;

	g_return_val_if_fail (iobuf != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (c != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	input_buffer = &iobuf->input_buffer;
	result = GNOME_VFS_OK;

	if (input_buffer->byte_count == 0) {
		if (! refill_input_buffer (iobuf)) {
			/* The buffer is empty but we had an error last time we
			   filled it, so we report the error.  */
			result = input_buffer->last_error;
			input_buffer->last_error = GNOME_VFS_OK;
		}
	}

	if (result == GNOME_VFS_OK)
		*c = *input_buffer->data;

	return result;
}

static GnomeVFSResult
flush (GnomeVFSIOBuf *iobuf)
{
	Buffer *output_buffer;
	gint r;

	output_buffer = &iobuf->output_buffer;

	while (output_buffer->byte_count > 0) {
		r = write (iobuf->fd, output_buffer->data,
			   output_buffer->byte_count);
		if (r == -1) {
			output_buffer->last_error =
				gnome_vfs_result_from_errno ();
			return output_buffer->last_error;
		} else {
			output_buffer->byte_count -= r;
		}
	}

	return GNOME_VFS_OK;
}

GnomeVFSResult
gnome_vfs_iobuf_write (GnomeVFSIOBuf *iobuf,
		       gconstpointer buffer,
		       GnomeVFSFileSize bytes,
		       GnomeVFSFileSize *bytes_written)
{
	Buffer *output_buffer;
	GnomeVFSFileSize write_count;
	GnomeVFSResult result;
	const gchar *p;

	g_return_val_if_fail (iobuf != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (buffer != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (bytes_written != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	output_buffer = &iobuf->output_buffer;

	result = GNOME_VFS_OK;

	p = buffer;
	write_count = 0;
	while (write_count < bytes) {
		if (output_buffer->byte_count < BUFFER_SIZE) {
			GnomeVFSFileSize n;

			n = MIN (BUFFER_SIZE - output_buffer->byte_count,
				 bytes);
			memcpy (output_buffer->data + output_buffer->byte_count,
				p, n);
			p += n;
			write_count += n;
			output_buffer->byte_count += n;
		} else {
			result = flush (iobuf);
			if (result != GNOME_VFS_OK)
				break;
		}
	}

	if (bytes_written != NULL)
		*bytes_written = write_count;

	return result;
}

GnomeVFSResult
gnome_vfs_iobuf_flush (GnomeVFSIOBuf *iobuf)
{
	g_return_val_if_fail (iobuf != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	return flush (iobuf);
}
