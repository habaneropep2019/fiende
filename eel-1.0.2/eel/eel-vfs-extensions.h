/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-vfs-extensions.h - gnome-vfs extensions.  Its likely some of these will 
                          be part of gnome-vfs in the future.

   Copyright (C) 1999, 2000 Eazel, Inc.

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

   Authors: Darin Adler <darin@eazel.com>
	    Pavel Cisler <pavel@eazel.com>
	    Mike Fleming  <mfleming@eazel.com>
            John Sullivan <sullivan@eazel.com>
*/

#ifndef EEL_VFS_EXTENSIONS_H
#define EEL_VFS_EXTENSIONS_H

#include <libgnome/gnome-defs.h>
#include <libgnomevfs/gnome-vfs-types.h>

BEGIN_GNOME_DECLS

#define	EEL_TRASH_URI "trash:"

typedef void     (* EelReadFileCallback) (GnomeVFSResult result,
					  GnomeVFSFileSize file_size,
					  char *file_contents,
					  gpointer callback_data);
typedef gboolean (* EelReadMoreCallback) (GnomeVFSFileSize file_size,
					  const char *file_contents,
					  gpointer callback_data);

typedef struct EelReadFileHandle EelReadFileHandle;

/* Read an entire file at once with gnome-vfs. */
GnomeVFSResult     eel_read_entire_file                  (const char           *uri,
							  int                  *file_size,
							  char                **file_contents);
EelReadFileHandle *eel_read_entire_file_async            (const char           *uri,
							  EelReadFileCallback   callback,
							  gpointer              callback_data);
EelReadFileHandle *eel_read_file_async                   (const char           *uri,
							  EelReadFileCallback   callback,
							  EelReadMoreCallback   read_more_callback,
							  gpointer              callback_data);
void               eel_read_file_cancel                  (EelReadFileHandle    *handle);

gboolean           eel_uri_is_trash                      (const char           *uri);
gboolean           eel_uri_is_trash_folder               (const char           *uri);
gboolean           eel_uri_is_in_trash                   (const char           *uri);

char *             eel_format_uri_for_display            (const char           *uri);
char *             eel_make_uri_from_input               (const char           *location);
char *             eel_make_uri_from_shell_arg           (const char           *location);
char *             eel_make_uri_canonical                (const char           *uri);
char *             eel_make_uri_canonical_strip_fragment (const char           *uri);
char *             eel_make_uri_from_half_baked_uri      (const char           *half_baked_uri);
gboolean           eel_uris_match                        (const char           *uri_1,
							  const char           *uri_2);
gboolean           eel_uris_match_ignore_fragments       (const char           *uri_1,
							  const char           *uri_2);
char *             eel_uri_get_basename                  (const char           *uri);
char *             eel_uri_get_scheme                    (const char           *uri);
char *             eel_uri_make_full_from_relative       (const char           *base_uri,
							  const char           *uri);

/* Convenience routine for simple file copying using text-based uris */
GnomeVFSResult     eel_copy_uri_simple                   (const char           *source_uri,
							  const char           *dest_uri);

/* gnome-vfs cover to make a directory and parents */
GnomeVFSResult     eel_make_directory_and_parents        (GnomeVFSURI          *uri,
							  guint                 permissions);

/* Convenience routine to test if a string is a remote URI. */
gboolean           eel_is_remote_uri                     (const char           *uri);

END_GNOME_DECLS

#endif /* EEL_VFS_EXTENSIONS_H */
