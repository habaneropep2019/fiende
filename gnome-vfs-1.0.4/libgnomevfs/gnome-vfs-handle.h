/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-handle.h - Handle object for GNOME VFS files.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@comm2000.it>
*/

#ifndef GNOME_VFS_HANDLE_H
#define GNOME_VFS_HANDLE_H

#include <libgnomevfs/gnome-vfs-method.h>

typedef struct GnomeVFSHandle GnomeVFSHandle;

GnomeVFSHandle * gnome_vfs_handle_new                (GnomeVFSURI             *uri,
						      GnomeVFSMethodHandle    *method_handle,
						      GnomeVFSOpenMode         open_mode);
void             gnome_vfs_handle_destroy            (GnomeVFSHandle          *handle);
GnomeVFSOpenMode gnome_vfs_handle_get_open_mode      (GnomeVFSHandle          *handle);
GnomeVFSResult   gnome_vfs_handle_do_close           (GnomeVFSHandle          *handle,
						      GnomeVFSContext         *context);
GnomeVFSResult   gnome_vfs_handle_do_read            (GnomeVFSHandle          *handle,
						      gpointer                 buffer,
						      GnomeVFSFileSize         num_bytes,
						      GnomeVFSFileSize        *bytes_read,
						      GnomeVFSContext         *context);
GnomeVFSResult   gnome_vfs_handle_do_write           (GnomeVFSHandle          *handle,
						      gconstpointer            buffer,
						      GnomeVFSFileSize         num_bytes,
						      GnomeVFSFileSize        *bytes_written,
						      GnomeVFSContext         *context);
GnomeVFSResult   gnome_vfs_handle_do_close_directory (GnomeVFSHandle          *handle,
						      GnomeVFSContext         *context);
GnomeVFSResult   gnome_vfs_handle_do_read_directory  (GnomeVFSHandle          *handle,
						      GnomeVFSFileInfo        *file_info,
						      GnomeVFSContext         *context);
GnomeVFSResult   gnome_vfs_handle_do_seek            (GnomeVFSHandle          *handle,
						      GnomeVFSSeekPosition     whence,
						      GnomeVFSFileSize         offset,
						      GnomeVFSContext         *context);
GnomeVFSResult   gnome_vfs_handle_do_tell            (GnomeVFSHandle          *handle,
						      GnomeVFSFileSize        *offset_return);
GnomeVFSResult   gnome_vfs_handle_do_get_file_info   (GnomeVFSHandle          *handle,
						      GnomeVFSFileInfo        *info,
						      GnomeVFSFileInfoOptions  options,
						      GnomeVFSContext         *context);
GnomeVFSResult   gnome_vfs_handle_do_truncate        (GnomeVFSHandle          *handle,
						      GnomeVFSFileSize         length,
						      GnomeVFSContext         *context);

#endif /* GNOME_VFS_HANDLE_H */
