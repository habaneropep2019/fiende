/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-seekable.h - seek / tell emulation layer for non seekable filesystems.

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

   Author: Michael Meeks <michael@imaginator.com>
*/

#ifndef GNOME_SEEKABLE_H
#define GNOME_SEEKABLE_H

#include <libgnomevfs/gnome-vfs-method.h>

GnomeVFSMethodHandle *gnome_vfs_seek_emulate (GnomeVFSURI          *handle,
					      GnomeVFSMethodHandle *child_handle,
					      GnomeVFSOpenMode      open_mode);

#endif /* GNOME_VFS_SEEKABLE_H */
