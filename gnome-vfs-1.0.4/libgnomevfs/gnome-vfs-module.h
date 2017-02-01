/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-method.h - All the VFS bits a module needs to include in one
   place.

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

   Author: Michael Meeks <michael@imaginator.com> */

#ifndef GNOME_VFS_MODULE_H
#define GNOME_VFS_MODULE_H

#include <libgnomevfs/gnome-vfs-method.h>

#define GNOME_VFS_MODULE_INIT      "vfs_module_init"
#define GNOME_VFS_MODULE_TRANSFORM "vfs_module_transform"
#define GNOME_VFS_MODULE_SHUTDOWN  "vfs_module_shutdown"

extern GnomeVFSMethod    *vfs_module_init	(const char *method_name, const char *args);
extern GnomeVFSTransform *vfs_module_transform	(const char *method_name, const char *args);
extern void               vfs_module_shutdown	(GnomeVFSMethod *method);

#endif /* _GNOME_VFS_MODULE_H */
