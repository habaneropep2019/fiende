/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Rebecca Schulman <rebecka@eazel.com>
 */

/* medusa-file-index-encoders.h:  Functions to encode and decode
   the various fields in the file index database */

#ifndef MEDUSA_FILE_INDEX_ENCODERS_H
#define MEDUSA_FILE_INDEX_ENCODERS_H

#include "medusa-file-index.h"
#include <libgnomevfs/gnome-vfs-types.h>

void medusa_file_database_uri_number_encode  (char               *result,
                                              gint32             *uri_number,
                                              MedusaFileSystemDB *file_system_db);
void medusa_file_database_mime_type_encode   (char               *result,
                                              char               *mime_type,
                                              MedusaFileSystemDB *file_system_db);
void medusa_file_database_mtime_encode       (char               *result,
                                              time_t             *time,
                                              MedusaFileSystemDB *file_system_db);
void medusa_file_database_owner_encode       (char               *result,
                                              uid_t              *owner,
                                              MedusaFileSystemDB *file_system_db);
void medusa_file_database_group_encode       (char               *result,
                                              gid_t              *group,
                                              MedusaFileSystemDB *file_system_db);
void medusa_file_database_permissions_encode (char               *result,
                                              GnomeVFSFilePermissions *permissions,
                                              MedusaFileSystemDB *file_system_db);
void medusa_file_database_size_encode        (char               *result,
                                              size_t             *size,
                                              MedusaFileSystemDB *file_system_db);
void medusa_file_database_uri_number_decode  (int                *uri_number_result,
                                              char               *field,
                                              gpointer            data);
void medusa_file_database_mime_type_decode   (char               *mime_type_result,
                                              char               *field,
                                              MedusaFileSystemDB *file_system_db);
void medusa_file_database_keywords_decode    (char               *keywords_result,
                                              char               *field,
                                              MedusaFileSystemDB *file_system_db);
void medusa_file_database_keywords_encode    (char               *result,
                                              char               *keywords,
                                              MedusaFileSystemDB *file_system_db);
void medusa_file_database_mtime_decode       (time_t             *mtime,
                                              char               *field,
                                              gpointer            data);
void medusa_file_database_owner_decode       (uid_t              *owner,
                                              char               *field,
                                              gpointer            data);
void medusa_file_database_group_decode       (gid_t              *group,
                                              char               *field,
                                              gpointer            data);
void medusa_file_database_permissions_decode (GnomeVFSFilePermissions *permissions,
                                              char               *field,
                                              gpointer            data);
void medusa_file_database_size_decode        (size_t             *size,
                                              char               *field,
                                              gpointer            data);
void medusa_file_index_encoders_test         (void);

#endif /* MEDUSA_FILE_INDEX_ENCODERS_H */
