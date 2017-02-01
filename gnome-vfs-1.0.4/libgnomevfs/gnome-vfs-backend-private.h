/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-context-private.h

   Copyright (C) 2001 Eazel, Inc

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

   Author: Michael Fleming <mfleming@eazel.com> */

#ifndef GNOME_VFS_CONTEXT_PRIVATE_H
#define GNOME_VFS_CONTEXT_PRIVATE_H

#include <libgnomevfs/gnome-vfs-context.h>
#include <libgnomevfs/gnome-vfs-module-callback.h>

void		gnome_vfs_backend_get_current_context (/* OUT */ GnomeVFSContext **context);

void		gnome_vfs_backend_dispatch_module_callback (GnomeVFSAsyncModuleCallback    callback,
							    gconstpointer                  in,
							    gsize                          in_size,
							    gpointer                       out, 
							    gsize                          out_size,
							    gpointer                       user_data,
							    GnomeVFSModuleCallbackResponse response,
							    gpointer                       response_data);

#endif /* GNOME_VFS_CONTEXT_PRIVATE_H */
