/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-socket.h
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

#ifndef GNOME_VFS_SOCKET_H
#define GNOME_VFS_SOCKET_H

#include <libgnomevfs/gnome-vfs-file-size.h>
#include <libgnomevfs/gnome-vfs-result.h>

typedef struct GnomeVFSSocket GnomeVFSSocket;


typedef GnomeVFSResult (*GnomeVFSSocketReadFunc)  (gpointer connection,
						   gpointer buffer, 
						   GnomeVFSFileSize bytes, 
						   GnomeVFSFileSize *bytes_read);
typedef GnomeVFSResult (*GnomeVFSSocketWriteFunc) (gpointer connection, 
						   gconstpointer buffer,
						   GnomeVFSFileSize bytes,
						   GnomeVFSFileSize *bytes_written);

typedef void           (*GnomeVFSSocketCloseFunc) (gpointer connection);

typedef struct {
  GnomeVFSSocketReadFunc read;
  GnomeVFSSocketWriteFunc write;
  GnomeVFSSocketCloseFunc close;
} GnomeVFSSocketImpl;


GnomeVFSSocket* gnome_vfs_socket_new     (GnomeVFSSocketImpl *impl, 
					  void               *connection);
GnomeVFSResult  gnome_vfs_socket_write   (GnomeVFSSocket     *socket, 
					  gconstpointer       buffer,
					  int                 bytes, 
					  GnomeVFSFileSize   *bytes_written);
GnomeVFSResult  gnome_vfs_socket_close   (GnomeVFSSocket     *socket);
GnomeVFSResult  gnome_vfs_socket_read    (GnomeVFSSocket     *socket, 
					  gpointer            buffer, 
					  GnomeVFSFileSize    bytes, 
					  GnomeVFSFileSize   *bytes_read);
#endif /* GNOME_VFS_SOCKET_H */
