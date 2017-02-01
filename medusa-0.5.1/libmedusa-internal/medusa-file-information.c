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

#include <config.h>
#include "medusa-file-information.h"

#include <medusa-file-index-queries.h>
#include <glib.h>
#include <string.h>
#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <medusa-byte.h>
#include <medusa-conf.h>


struct MedusaFileAttributes {
        gint32 uri_number;
        /* File type (regular, directory, pipe, etc) */
        GnomeVFSFileType type;
        /* Date of last modification */
        time_t mtime; 
        /* Owner */
        uid_t uid;
        /* Group */
        gid_t gid;
        /* Permissions */
        GnomeVFSFilePermissions permissions;
        /* Size (Defined only for regular files) */
        size_t size;
        /* mime type */
        char *mime_type;
};

MedusaFileAttributes *
medusa_file_attributes_new (int uri_number,
			    GnomeVFSFileInfo *metadata) {

        MedusaFileAttributes *file_information; 

        file_information = g_new0 (MedusaFileAttributes, 1);
        file_information->uri_number = uri_number;
        file_information->mtime = metadata->mtime;
        file_information->uid = metadata->uid;
        file_information->gid = metadata->gid;
        file_information->permissions = metadata->permissions;
        file_information->size = metadata->size;
        file_information->mime_type = g_strdup (metadata->mime_type);

        return file_information;
}

static void
append_file_information_field (char *record, gpointer field_data, MedusaRDBFieldInfo *field_info,
			       char *field_title, MedusaFileSystemDB *file_system_db)
{
        MedusaRDBEncodeFunc encode;
        
        encode = medusa_rdb_field_get_encoder  (field_info, field_title);
        (* encode) (record, field_data, file_system_db);
}

void
medusa_index_file_attributes (MedusaFileAttributes *file_attributes, 
			      MedusaFileSystemDB *file_system_db)

{
        char *record;
        int offset;
        MedusaRDBFieldInfo *field_info;

        field_info = file_system_db->file_database->file->field_info;
	
        record = g_new0 (char, field_info->record_size);
        
        offset = 0;

        append_file_information_field (&record[offset], 
                                       &file_attributes->uri_number,
                                       field_info,
                                       MEDUSA_FILE_INDEX_URI_NUMBER_FIELD_TITLE,
                                       file_system_db);
        offset += MEDUSA_FILE_INDEX_URI_NUMBER_FIELD_SIZE;
        append_file_information_field (&record[offset],
                                       &file_attributes->mtime,
                                       field_info,
                                       MEDUSA_FILE_INDEX_MTIME_FIELD_TITLE,
                                       file_system_db);
        offset += MEDUSA_FILE_INDEX_MTIME_FIELD_SIZE;
        append_file_information_field (&record[offset],
                                       &file_attributes->uid,
                                       field_info,
                                       MEDUSA_FILE_INDEX_OWNER_FIELD_TITLE,
                                       file_system_db);
        offset += MEDUSA_FILE_INDEX_OWNER_FIELD_SIZE;
        append_file_information_field (&record[offset],
                                       &file_attributes->gid,
                                       field_info,
                                       MEDUSA_FILE_INDEX_GROUP_FIELD_TITLE,
                                       file_system_db);
        offset += MEDUSA_FILE_INDEX_GROUP_FIELD_SIZE;
        append_file_information_field (&record[offset],
                                       &file_attributes->permissions,
                                       field_info,
                                       MEDUSA_FILE_INDEX_PERMISSIONS_FIELD_TITLE,
                                       file_system_db);
        offset += MEDUSA_FILE_INDEX_PERMISSIONS_FIELD_SIZE;
        append_file_information_field (&record[offset],
                                       &file_attributes->size,
                                       field_info,
                                       MEDUSA_FILE_INDEX_SIZE_FIELD_TITLE,
                                       file_system_db);
        offset += MEDUSA_FILE_INDEX_SIZE_FIELD_SIZE;
        append_file_information_field (&record[offset],
                                       file_attributes->mime_type,
                                       field_info,
                                       MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE,
                                       file_system_db);
        offset += MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_SIZE;
        /* Keywords get written later. */
        offset += MEDUSA_FILE_INDEX_KEYWORDS_FIELD_SIZE;
        
        g_assert (field_info->record_size == offset);
        medusa_rdb_table_insert (file_system_db->file_database, record);
        g_free (record);
}

void
medusa_file_attributes_free (MedusaFileAttributes *file_attributes)
{
	g_free (file_attributes->mime_type);
	g_free (file_attributes);
}
