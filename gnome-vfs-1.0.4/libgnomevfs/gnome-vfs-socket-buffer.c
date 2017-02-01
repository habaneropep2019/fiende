/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-socket-buffer.c
 *
 * Copyright (C) 2001 Seth Nickell
 * Copyright (C) 2001 Maciej Stachowiak
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
 *
 */
/*
 * Authors: Seth Nickell <snickell@stanford.edu>
 *          Maciej Stachowiak <mjs@noisehavoc.org>
 *          (reverse-engineered from code by Ian McKellar <yakk@yakk.net>)
 */

#include <config.h>
#include "gnome-vfs-socket-buffer.h"

#include <string.h>



#define BUFFER_SIZE 4096

struct Buffer {
	gchar data[BUFFER_SIZE];
	guint offset;
	guint byte_count;
	GnomeVFSResult last_error;
};
typedef struct Buffer Buffer;


struct  GnomeVFSSocketBuffer {
        GnomeVFSSocket *socket;
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


GnomeVFSSocketBuffer*  
gnome_vfs_socket_buffer_new (GnomeVFSSocket *socket)
{
	GnomeVFSSocketBuffer *socket_buffer;

	g_return_val_if_fail (socket != NULL, NULL);

	socket_buffer = g_new (GnomeVFSSocketBuffer, 1);
	socket_buffer->socket = socket;

	buffer_init (&socket_buffer->input_buffer);
	buffer_init (&socket_buffer->output_buffer);

	return socket_buffer;
}

GnomeVFSResult   
gnome_vfs_socket_buffer_destroy  (GnomeVFSSocketBuffer *buffer, 
				  gboolean close_socket)
{
	gnome_vfs_socket_buffer_flush (buffer);

        if (close_socket) {
		gnome_vfs_socket_close (buffer->socket);
	}
	g_free (buffer);
	return GNOME_VFS_OK;
}




static gboolean
refill_input_buffer (GnomeVFSSocketBuffer *socket_buffer)
{
	Buffer *input_buffer;
	GnomeVFSResult result;
	GnomeVFSFileSize bytes_read;
	
	input_buffer = &socket_buffer->input_buffer;

	if (input_buffer->last_error != GNOME_VFS_OK
	    || input_buffer->byte_count > 0) {
		return FALSE;
	}

	input_buffer->offset = 0;

	result = gnome_vfs_socket_read (socket_buffer->socket,
					&input_buffer->data, 
					BUFFER_SIZE,
					&bytes_read);
	
	if (bytes_read == 0) {
		input_buffer->last_error = GNOME_VFS_ERROR_EOF;
		return FALSE;
	}

	input_buffer->byte_count = bytes_read;

	return TRUE;
}
 
GnomeVFSResult   
gnome_vfs_socket_buffer_read (GnomeVFSSocketBuffer *socket_buffer,
			      gpointer buffer,
			      GnomeVFSFileSize bytes,
			      GnomeVFSFileSize *bytes_read)
{
	Buffer *input_buffer;
	GnomeVFSResult result;
	GnomeVFSFileSize n;
	
	g_return_val_if_fail (socket_buffer != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (buffer != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	
	/* Quote from UNIX 98:
	   "If nbyte is 0, read() will return 0 and have no other results."
	*/
	if (bytes == 0) {
		*bytes_read = 0;
		return GNOME_VFS_OK;
	}
		
	input_buffer = &socket_buffer->input_buffer;

	result = GNOME_VFS_OK;

	if (input_buffer->byte_count == 0) {
		if (! refill_input_buffer (socket_buffer)) {
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
		if (bytes_read != NULL) {
			*bytes_read = n;
		}
	} else {
		if (bytes_read != NULL) {
			*bytes_read = 0;
		}
	}

	if (result == GNOME_VFS_ERROR_EOF) {
		result = GNOME_VFS_OK;
	}

	return result;
}

GnomeVFSResult
gnome_vfs_socket_buffer_peekc (GnomeVFSSocketBuffer *socket_buffer,
			       gchar *c)
{
	GnomeVFSResult result;
	Buffer *input_buffer;

	g_return_val_if_fail (socket_buffer != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (c != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	input_buffer = &socket_buffer->input_buffer;
	result = GNOME_VFS_OK;

	if (input_buffer->byte_count == 0) {
		if (!refill_input_buffer (socket_buffer)) {
			/* The buffer is empty but we had an error last time we
			   filled it, so we report the error.  */
			result = input_buffer->last_error;
			input_buffer->last_error = GNOME_VFS_OK;
		}
	}

	if (result == GNOME_VFS_OK) {
		*c = *input_buffer->data;
	}

	return result;
}



static GnomeVFSResult
flush (GnomeVFSSocketBuffer *socket_buffer)
{
	Buffer *output_buffer;
	GnomeVFSResult result;
	GnomeVFSFileSize bytes_written;

	output_buffer = &socket_buffer->output_buffer;

	while (output_buffer->byte_count > 0) {
		result = gnome_vfs_socket_write (socket_buffer->socket, 
						 output_buffer->data,
						 output_buffer->byte_count,
						 &bytes_written);
		output_buffer->last_error = result;
		output_buffer->byte_count -= bytes_written;
	}

	return GNOME_VFS_OK;
}


GnomeVFSResult   
gnome_vfs_socket_buffer_write (GnomeVFSSocketBuffer *socket_buffer, 
			       gconstpointer buffer,
			       GnomeVFSFileSize bytes,
			       GnomeVFSFileSize *bytes_written)
{
	Buffer *output_buffer;
	GnomeVFSFileSize write_count;
	GnomeVFSResult result;
	const gchar *p;

	g_return_val_if_fail (socket_buffer != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (buffer != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (bytes_written != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	output_buffer = &socket_buffer->output_buffer;

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
			result = flush (socket_buffer);
			if (result != GNOME_VFS_OK) {
				break;
			}
		}
	}

	if (bytes_written != NULL) {
		*bytes_written = write_count;
	}
		
	return result;
}

GnomeVFSResult   
gnome_vfs_socket_buffer_flush (GnomeVFSSocketBuffer *socket_buffer)
{
	g_return_val_if_fail (socket_buffer != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	return flush (socket_buffer);
}



