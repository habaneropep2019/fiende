/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-iobuf.h - Buffered I/O for the GNOME Virtual File System.

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

#ifndef GNOME_VFS_IOBUF_H
#define GNOME_VFS_IOBUF_H

#include <libgnomevfs/gnome-vfs-file-size.h>
#include <libgnomevfs/gnome-vfs-result.h>

typedef struct GnomeVFSIOBuf GnomeVFSIOBuf;

GnomeVFSIOBuf   *gnome_vfs_iobuf_new     (gint              fd);
void             gnome_vfs_iobuf_destroy (GnomeVFSIOBuf    *iobuf);
GnomeVFSResult   gnome_vfs_iobuf_read    (GnomeVFSIOBuf    *iobuf,
					  gpointer          buffer,
					  GnomeVFSFileSize  bytes,
					  GnomeVFSFileSize *bytes_read);
GnomeVFSResult   gnome_vfs_iobuf_peekc   (GnomeVFSIOBuf    *iobuf,
					  gchar            *c);
GnomeVFSResult   gnome_vfs_iobuf_write   (GnomeVFSIOBuf    *iobuf,
					  gconstpointer     buffer,
					  GnomeVFSFileSize  bytes,
					  GnomeVFSFileSize *bytes_written);
GnomeVFSResult   gnome_vfs_iobuf_flush   (GnomeVFSIOBuf    *iobuf);

#endif /* GNOME_VFS_IOBUF_H */
