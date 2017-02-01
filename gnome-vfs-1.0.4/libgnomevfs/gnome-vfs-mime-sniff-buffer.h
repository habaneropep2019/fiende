/*
 * gnome-vfs-mime-sniff-buffer.h
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

#ifndef GNOME_VFS_MIME_SNIFF_BUFFER_H
#define GNOME_VFS_MIME_SNIFF_BUFFER_H

#include <libgnomevfs/gnome-vfs-handle.h>

typedef GnomeVFSResult (* GnomeVFSSniffBufferSeekCall)(gpointer context, 
		GnomeVFSSeekPosition whence, GnomeVFSFileOffset offset);
typedef GnomeVFSResult (* GnomeVFSSniffBufferReadCall)(gpointer context, 
		gpointer buffer, GnomeVFSFileSize bytes, GnomeVFSFileSize *bytes_read);
		
typedef struct GnomeVFSMimeSniffBuffer GnomeVFSMimeSniffBuffer;

void			 gnome_vfs_mime_clear_magic_table 		(void);

GnomeVFSMimeSniffBuffer	*gnome_vfs_mime_sniff_buffer_new_from_handle 	
					(GnomeVFSHandle 		*file);
GnomeVFSMimeSniffBuffer	*gnome_vfs_mime_sniff_buffer_new_from_memory 
					(const guchar 			*buffer, 
					 gssize 			buffer_size);
GnomeVFSMimeSniffBuffer	*gnome_vfs_mime_sniff_buffer_new_from_existing_data 	
					(const guchar 			*buffer, 
					 gssize 			buffer_size);
GnomeVFSMimeSniffBuffer	*gnome_vfs_mime_sniff_buffer_new_generic 	
					(GnomeVFSSniffBufferSeekCall	seek_callback, 
					 GnomeVFSSniffBufferReadCall	read_callback,
					 gpointer			context);


void			 gnome_vfs_mime_sniff_buffer_free 
					(GnomeVFSMimeSniffBuffer	*buffer);

GnomeVFSResult		 gnome_vfs_mime_sniff_buffer_get
					(GnomeVFSMimeSniffBuffer	*buffer,
					 gssize				size);

const char  		*gnome_vfs_get_mime_type_for_buffer
					 (GnomeVFSMimeSniffBuffer	*buffer);

gboolean		 gnome_vfs_sniff_buffer_looks_like_text
					 (GnomeVFSMimeSniffBuffer	*buffer);
gboolean		 gnome_vfs_sniff_buffer_looks_like_mp3
					 (GnomeVFSMimeSniffBuffer	*buffer);
gboolean		 gnome_vfs_sniff_buffer_looks_like_gzip
					 (GnomeVFSMimeSniffBuffer 	*sniff_buffer,
					  const char 			*file_name);

#endif
