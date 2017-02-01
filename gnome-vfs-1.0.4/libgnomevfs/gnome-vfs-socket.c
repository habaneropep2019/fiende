/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-socket.c
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
#include "gnome-vfs-socket.h"

struct GnomeVFSSocket {
	GnomeVFSSocketImpl *impl;
	gpointer connection;
};


GnomeVFSSocket* gnome_vfs_socket_new (GnomeVFSSocketImpl *impl, 
				      void               *connection) 
{
	GnomeVFSSocket *socket;
	
	socket = g_new0 (GnomeVFSSocket, 1);
	socket->impl = impl;
	socket->connection = connection; 
	
	return socket;
}

GnomeVFSResult  
gnome_vfs_socket_write (GnomeVFSSocket *socket, 
			gconstpointer buffer,
			int bytes, 
			GnomeVFSFileSize *bytes_written)
{
	return socket->impl->write (socket->connection,
				    buffer, bytes, bytes_written);
}

GnomeVFSResult  
gnome_vfs_socket_close (GnomeVFSSocket *socket)
{
	socket->impl->close (socket->connection);
	return GNOME_VFS_OK;
}

GnomeVFSResult  
gnome_vfs_socket_read  (GnomeVFSSocket *socket, 
			gpointer buffer, 
			GnomeVFSFileSize bytes,
			GnomeVFSFileSize *bytes_read)
{
	return socket->impl->read (socket->connection,
				   buffer, bytes, bytes_read);
}
