/* nfs-method.h - VFS modules for NFS

   Copyright (C) 2000 Free Software Foundation

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

   Author: Grahame Bowland <gbowland@gbowland.com>
           Ian McKellar <yakk@yakk.net>
*/


#ifndef NFS_METHOD_H
#define NFS_METHOD_H

#include "nfs-method_nfs_prot.h"
#include "nfs-method_mount.h"
#include "gnome-vfs-module.h"

typedef struct NfsServerConnection {
	char *hostname;
	enum {
		NFS_TCP, NFS_UDP
	} proto;
	struct timeval mount_timeval;
	struct timeval nfs_timeval;
	struct hostent *hp;
	struct sockaddr_in *mount_server_addr;
	struct sockaddr_in *nfs_server_addr;
	CLIENT *mount_client;
	CLIENT *nfs_client;
	int mount_sock, nfs_sock;
	GMutex *mount_sock_mutex, *nfs_sock_mutex;
	GHashTable *file_handle_hash;
	GMutex *file_handle_hash_mutex;
} NfsServerConnection;

typedef struct NfsFileHandle {
	GnomeVFSURI *uri;
	nfs_fh handle;
	int mounted;
} NfsFileHandle;

typedef struct NfsDirectoryHandle {
	GnomeVFSURI *uri;
	GList *files;
	GList *pointer;
} NfsDirectoryHandle;

typedef struct NfsOpenHandle {
	NfsServerConnection *conn;
	NfsFileHandle *handle;
	GnomeVFSURI *uri;
	enum {
		NFS_READ,
		NFS_WRITE
	} operation;
	int position;
} NfsOpenHandle;

#endif /* NFS_METHOD_H */
