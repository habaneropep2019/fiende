/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-ssl.h
 *
 * Copyright (C) 2001 Ian McKellar
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
/*
 * Authors: Ian McKellar <yakk@yakk.net>
 */

#ifndef GNOME_VFS_SSL_H
#define GNOME_VFS_SSL_H

#include <libgnomevfs/gnome-vfs-socket.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef struct GnomeVFSSSL GnomeVFSSSL;

gboolean        gnome_vfs_ssl_enabled        (void);
/* FIXME: add *some* kind of cert verification! */
GnomeVFSResult  gnome_vfs_ssl_create         (GnomeVFSSSL **handle_return,
		                              const char *host, 
		                              unsigned int port);
GnomeVFSResult  gnome_vfs_ssl_create_from_fd (GnomeVFSSSL **handle_return,
					      gint fd);
GnomeVFSResult  gnome_vfs_ssl_read           (GnomeVFSSSL *ssl,
					      gpointer buffer,
					      GnomeVFSFileSize bytes,
				      	      GnomeVFSFileSize *bytes_read);
GnomeVFSResult  gnome_vfs_ssl_write          (GnomeVFSSSL *ssl,
					      gconstpointer buffer,
					      GnomeVFSFileSize bytes,
					      GnomeVFSFileSize *bytes_written);
void            gnome_vfs_ssl_destroy        (GnomeVFSSSL *ssl);
GnomeVFSSocket *gnome_vfs_ssl_to_socket      (GnomeVFSSSL *ssl);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* GNOME_VFS_SSL_H */
