/*
 * gnome-vfs-mime-sniff-buffer-private.h
 * Utility for implementing gnome_vfs_mime_type_from_magic, and other
 * mime-type sniffing calls.
 *
 * Copyright (C) 2000 Eazel, Inc.
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
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

#ifndef GNOME_VFS_MIME_SNIFF_BUFFER_PRIVATE_H
#define GNOME_VFS_MIME_SNIFF_BUFFER_PRIVATE_H

#include <libgnomevfs/gnome-vfs-mime-sniff-buffer.h>

struct GnomeVFSMimeSniffBuffer {
	guchar *buffer;
	gssize buffer_length;
        gboolean read_whole_file;
	gboolean owning;

	GnomeVFSSniffBufferSeekCall seek;
	GnomeVFSSniffBufferReadCall read;
	gpointer context;
};

const char *gnome_vfs_get_mime_type_internal         (GnomeVFSMimeSniffBuffer *buffer,
						      const char              *file_name);
const char *gnome_vfs_mime_get_type_from_magic_table (GnomeVFSMimeSniffBuffer *buffer);

#endif
