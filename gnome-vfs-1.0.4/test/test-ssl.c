/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* test-sync.c - Test program for synchronous operation of the GNOME
   Virtual File System.

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

   Author: Ian McKellar <yakk@yakk.net> 
 */

#include "gnome-vfs.h"
#include "gnome-vfs-ssl.h"
#include "gnome-vfs-socket.h"
#include "gnome-vfs-socket-buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

enum {
	SSL, SOCKET, SOCKETBUFFER
} abstraction = SOCKETBUFFER;

static void
show_result (GnomeVFSResult result, 
	     const gchar *what, 
	     const gchar *host, 
	     gint port)
{
	fprintf (stderr, "%s `%s:%d': %s\n",
		 what, host, port, gnome_vfs_result_to_string (result));
	if (result != GNOME_VFS_OK)
		exit (1);
}

#define HTTP_REQUEST "GET / HTTP/1.0\r\n\r\n"

int
main (int argc, char **argv)
{
	GnomeVFSResult        result = GNOME_VFS_OK;
	gchar                 buffer[1024];
	gchar		     *host;
	gint                  port;
	GnomeVFSFileSize      bytes_read;
	GnomeVFSSSL          *ssl = NULL;
	GnomeVFSSocket       *socket = NULL;
	GnomeVFSSocketBuffer *socketbuffer = NULL;

	if (argc != 3) {
		printf ("Usage: %s <host> <port>\n", argv[0]);
		return 1;
	}

	host = argv[1];
	port = atoi (argv[2]);

	if (port <= 0) {
		printf ("Invalid port\n");
		return 1;
	}

	if (! gnome_vfs_init ()) {
		fprintf (stderr, "Cannot initialize gnome-vfs.\n");
		return 1;
	}

	switch (abstraction) {
		case SOCKETBUFFER:
			g_print ("Testing GnomeVFSSocketBuffer");
		case SOCKET:
			g_print (" and GnomeVFSSocket");
		case SSL:
			g_print (" and GnomeVFSSSL");
	}
	g_print (".\n");

	result = gnome_vfs_ssl_create (&ssl, host, port);

	show_result (result, "ssl_create", host, port);

	if (ssl == NULL) {
		fprintf (stderr, "couln't connect\n");
		return -1;
	}

	if (abstraction >= SOCKET) {
		socket = gnome_vfs_ssl_to_socket (ssl);
		if (socket == NULL) {
			fprintf (stderr, "couldn't create socket object\n");
			return -1;
		}

		if (abstraction == SOCKETBUFFER) {
			socketbuffer = gnome_vfs_socket_buffer_new (socket);
			if (socketbuffer == NULL) {
				fprintf (stderr, 
				       "couldn't create socketbuffer object\n");
				return -1;
			}
		}
	}

	switch (abstraction) {
		case SSL:
			result = gnome_vfs_ssl_write (ssl, HTTP_REQUEST, 
					strlen(HTTP_REQUEST), &bytes_read);
			break;
		case SOCKET:
			result = gnome_vfs_socket_write (socket, HTTP_REQUEST, 
					strlen(HTTP_REQUEST), &bytes_read);
			break;
		case SOCKETBUFFER:
			result = gnome_vfs_socket_buffer_write (socketbuffer,
					HTTP_REQUEST, strlen(HTTP_REQUEST),
					&bytes_read);
			gnome_vfs_socket_buffer_flush (socketbuffer);
			break;
	}

	show_result (result, "write", host, port);

	while( result==GNOME_VFS_OK ) {
		switch (abstraction) {
			case SSL:
				result = gnome_vfs_ssl_read (ssl, buffer, 
						sizeof buffer - 1, &bytes_read);
				break;
			case SOCKET:
				result = gnome_vfs_socket_read (socket, buffer, 
						sizeof buffer - 1, &bytes_read);
				break;
			case SOCKETBUFFER:
				result = gnome_vfs_socket_buffer_read (
						socketbuffer, buffer, 
						sizeof buffer - 1, &bytes_read);
				break;
		}
		show_result (result, "read", host, port);
	
		buffer[bytes_read] = 0;
		write (1,buffer,bytes_read);
		if(!bytes_read) break;
	}

	switch (abstraction) {
		case SSL:
			gnome_vfs_ssl_destroy (ssl);
			break;
		case SOCKET:
			gnome_vfs_socket_close (socket);
			break;
		case SOCKETBUFFER:
			gnome_vfs_socket_buffer_destroy (socketbuffer, TRUE);
			break;
	}

	return 0;
}
