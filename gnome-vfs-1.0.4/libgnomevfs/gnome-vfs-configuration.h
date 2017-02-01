/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-configuration.h - Handling of the GNOME Virtual File System
   configuration.

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

   Author: Ettore Perazzoli <ettore@comm2000.it> */

#ifndef _GNOME_VFS_CONFIGURATION_H
#define _GNOME_VFS_CONFIGURATION_H

#include <glib.h>

void         gnome_vfs_configuration_add_directory   (const char *dir);
gboolean     gnome_vfs_configuration_init            (void);
void         gnome_vfs_configuration_uninit          (void);
const gchar *gnome_vfs_configuration_get_module_path (const gchar *method_name, const char ** args);

#endif /* _GNOME_VFS_CONFIGURATION_H */
