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
 *
 *  medusa-file-index-queries.c  Builds and performs common queries on a file index
 *
 */

#include <config.h>
#include "medusa-file-index-queries.h"

#include "medusa-conf.h"
#include "medusa-keyword-set.h"
#include "medusa-query-clauses.h"
#include "medusa-rdb-record.h"
#include <glib.h>
#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libmedusa/medusa-file-info-utilities.h>
#include <libmedusa/medusa-string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


static gboolean medusa_file_index_is_file                            (gpointer database,
                                                                      gpointer record);
static gboolean medusa_file_index_is_text_file                       (gpointer database,
                                                                      gpointer record);
static gboolean medusa_file_index_is_application                     (gpointer database,
                                                                      gpointer record);
static gboolean medusa_file_index_is_directory                       (gpointer database,
                                                                      gpointer record);
static gboolean medusa_file_index_is_music                           (gpointer database,
                                                                      gpointer record);

gboolean
medusa_file_index_is_of_type (gpointer database,
                              gpointer record,
                              MedusaQueryArgument type_argument)
{
        const char *type;

        type = type_argument.string;

        if (strcasecmp (type, "file") == 0) {
                return medusa_file_index_is_file (database,
                                                  record);
        }
        if (strcasecmp (type, "text_file") == 0) {
                return medusa_file_index_is_text_file (database,
                                                       record);
        }
        if (strcasecmp (type, "application") == 0) {
                return medusa_file_index_is_application (database,
                                                         record);
        }
        if (strcasecmp (type, "music") == 0) {
                return medusa_file_index_is_music (database,
                                                   record);
        }
        if (strcasecmp (type, "directory") == 0) {
                return medusa_file_index_is_directory (database,
                                                       record);
        }

        return FALSE;
}

gboolean
medusa_file_index_is_not_of_type (gpointer database,
                                  gpointer record,
                                  MedusaQueryArgument type_argument)
{
        return !(medusa_file_index_is_of_type (database,
                                               record,
                                               type_argument));
}

static gboolean
medusa_file_index_is_file (gpointer database,
                           gpointer record)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        char *mime_type_in_record;
        gboolean result;
        
        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord ) record;

        mime_type_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE,
                                           file_system_db,
                                           mime_type_in_record);
        result = !medusa_str_has_prefix (mime_type_in_record, "x-special/")  &&
                !medusa_str_has_prefix (mime_type_in_record, "x-directory/");
        g_free (mime_type_in_record);
        
        return result;
}

static gboolean
medusa_file_index_is_text_file (gpointer database,
                                gpointer record)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        char *mime_type_in_record;
        gboolean result;
        
        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord ) record;

        mime_type_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE,
                                           file_system_db,
                                           mime_type_in_record);
        result = medusa_str_has_prefix (mime_type_in_record, "text/");
        g_free (mime_type_in_record);
        
        return result;
}

static gboolean
medusa_file_index_is_application (gpointer database,
                                  gpointer record)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        char *mime_type_in_record;
        gboolean result;
        
        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord ) record;

        mime_type_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE,
                                           file_system_db,
                                           mime_type_in_record);
        result = medusa_str_has_prefix (mime_type_in_record, "application/");
        g_free (mime_type_in_record);
        
        return result;
}

static gboolean
medusa_file_index_is_music (gpointer database,
                            gpointer record)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        char *mime_type_in_record;
        gboolean result;
        
        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord ) record;

        mime_type_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE,
                                           file_system_db,
                                           mime_type_in_record);
        result = medusa_str_has_prefix (mime_type_in_record, "audio/");
        g_free (mime_type_in_record);
        
        return result;
}

static gboolean
medusa_file_index_is_directory (gpointer database,
                                gpointer record)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        char *mime_type_in_record;
        gboolean result;
        
        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord ) record;

        mime_type_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE,
                                           file_system_db,
                                           mime_type_in_record);
        result = medusa_str_has_prefix (mime_type_in_record, "x-directory/");
        g_free (mime_type_in_record);
        
        return result;
}

gboolean
medusa_file_index_has_uid (gpointer database,
                           gpointer record,
                           MedusaQueryArgument argument)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        uid_t uid_in_record;
        long uid_in_query;
        
        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord ) record;
        uid_in_query = argument.number;

        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_OWNER_FIELD_TITLE,
                                           file_system_db,
                                           &uid_in_record);
        return uid_in_record == uid_in_query;

}

gboolean
medusa_file_index_has_gid (gpointer database,
                           gpointer record,
                           MedusaQueryArgument argument)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        gid_t gid_in_record;
        long gid_in_query;
        
        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord ) record;
        gid_in_query = argument.number;

        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_GROUP_FIELD_TITLE,
                                           file_system_db,
                                           &gid_in_record);
        return gid_in_record == gid_in_query;

}

gboolean
medusa_file_index_does_not_have_uid (gpointer database,
                                     gpointer record,
                                     MedusaQueryArgument argument)
{
        return !medusa_file_index_has_uid (database,
                                           record,
                                           argument);
}
gboolean
medusa_file_index_does_not_have_gid (gpointer database,
                                     gpointer record,
                                     MedusaQueryArgument argument)
{
        return !medusa_file_index_has_gid (database,
                                           record,
                                           argument);
}

gboolean
medusa_file_index_is_modified_before_time (gpointer database,
                                           gpointer record,
                                           MedusaQueryArgument argument)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        time_t time_in_record;
        long time_in_query;
        

        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord) record;
        time_in_query = argument.number; 

        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_MTIME_FIELD_TITLE,
                                           file_system_db,
                                           &time_in_record);
        return time_in_record < time_in_query;
       
}

gboolean
medusa_file_index_is_modified_after_time (gpointer database,
                                          gpointer record,
                                          MedusaQueryArgument argument)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        time_t time_in_record;
        long time_in_query;
        
        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord) record;
        time_in_query = argument.number; 

        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_MTIME_FIELD_TITLE,
                                           file_system_db,
                                           &time_in_record);
        return time_in_record > time_in_query;
       
}

gboolean
medusa_file_index_is_not_modified_on_date (gpointer database,
                                           gpointer record,
                                           MedusaQueryArgument argument)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        char *date_in_query;
        static char *last_date_in_query = NULL;
        static time_t first_time_corresponding_to_date;
        static time_t last_time_corresponding_to_date;
        time_t time_in_record;

        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord) record;
        date_in_query = argument.string;
        
        if (!last_date_in_query ||
            strcmp (last_date_in_query, date_in_query) != 0) {
                g_free (last_date_in_query);
                last_date_in_query = g_strdup (argument.string);
                first_time_corresponding_to_date = medusa_file_info_get_first_unix_time_occurring_on_date (date_in_query);
                last_time_corresponding_to_date = medusa_file_info_get_last_unix_time_occurring_on_date (date_in_query);
        }

        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_MTIME_FIELD_TITLE,
                                           file_system_db,
                                           &time_in_record);

        return (time_in_record > last_time_corresponding_to_date ||
                time_in_record < first_time_corresponding_to_date);
}

gboolean 
medusa_file_index_verify_file_is_not_modified_on_date (const char *uri,
                                                       MedusaQueryArgument argument)
{
        GnomeVFSFileInfo file_info;
        GnomeVFSResult get_info_result;
        char *date_in_query;
        static char *last_date_in_query = NULL;
        static time_t first_time_corresponding_to_date;
        static time_t last_time_corresponding_to_date;

        date_in_query = argument.string;

        if (!last_date_in_query ||
            strcmp (last_date_in_query, date_in_query) != 0) {
                g_free (last_date_in_query);
                last_date_in_query = g_strdup (argument.string);
                first_time_corresponding_to_date = medusa_file_info_get_first_unix_time_occurring_on_date (date_in_query);
                last_time_corresponding_to_date = medusa_file_info_get_last_unix_time_occurring_on_date (date_in_query);
        }


        get_info_result = gnome_vfs_get_file_info (uri,
                                                   &file_info,
                                                   GNOME_VFS_FILE_INFO_DEFAULT);

        /* If something happened to the file, don't return it */
        if (get_info_result != GNOME_VFS_OK) {
                return FALSE;
        }
        else {
                return file_info.mtime < first_time_corresponding_to_date ||
                        file_info.mtime > last_time_corresponding_to_date;;
        }
}


gboolean
medusa_file_index_is_larger_than (gpointer database,
                                  gpointer record,
                                  MedusaQueryArgument argument)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        size_t size_in_record;
        long size_in_query;
        
        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord) record;
        size_in_query = argument.number;

        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_SIZE_FIELD_TITLE,
                                           file_system_db,
                                           &size_in_record);
        return size_in_record > size_in_query;
        
}

gboolean
medusa_file_index_is_smaller_than (gpointer database,
                                   gpointer record,
                                   MedusaQueryArgument argument)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        size_t size_in_record;
        long size_in_query;
        
        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord) record;
        size_in_query = argument.number;

        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_SIZE_FIELD_TITLE,
                                           file_system_db,
                                           &size_in_record);
        return size_in_record < size_in_query;
        
}

gboolean
medusa_file_index_is_of_size (gpointer database,
                              gpointer record,
                              MedusaQueryArgument argument)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        size_t size_in_record;
        long size_in_query;
        

        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord) record;
        size_in_query = argument.number;
        
        medusa_rdb_record_get_field_value (database_record,
                                           medusa_file_system_db_get_field_info (file_system_db),
                                           MEDUSA_FILE_INDEX_SIZE_FIELD_TITLE,
                                           file_system_db,
                                           &size_in_record);
        return size_in_record == size_in_query;
        
}

gboolean
medusa_file_index_uid_can_read_file (gpointer database,
                                     gpointer record,
                                     MedusaQueryArgument argument)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        long  uid_in_query;
        uid_t uid_in_record;
        gid_t gid_in_record;
        GnomeVFSFilePermissions permissions;
        
        file_system_db = (MedusaFileSystemDB *) database;
        database_record = (MedusaRDBRecord) record;
        uid_in_query = argument.number;


        /* We can skip all of this stuff if it's root */
        if (uid_in_query == 0) {
#ifdef PERMISSIONS_CHECK_DEBUG
                g_print ("Returning true because requestor is superuser\n");
#endif
                return TRUE;
        }


        medusa_rdb_record_get_field_value
                (database_record,
                 medusa_file_system_db_get_field_info (file_system_db),
                 MEDUSA_FILE_INDEX_PERMISSIONS_FIELD_TITLE,
                 file_system_db,
                 &permissions);
        
        medusa_rdb_record_get_field_value
                (database_record,
                 medusa_file_system_db_get_field_info (file_system_db),
                 MEDUSA_FILE_INDEX_OWNER_FIELD_TITLE,
                 file_system_db,
                 &uid_in_record);
        
        if (uid_in_record == uid_in_query &&
            permissions & S_IRUSR) {
#ifdef PERMISSIONS_CHECK_DEBUG
                g_print ("Returning TRUE because file is owned by the user and the file is readable by the user.\n");
#endif

                return TRUE;
        }

        medusa_rdb_record_get_field_value
                (database_record,
                 medusa_file_system_db_get_field_info (file_system_db),
                 MEDUSA_FILE_INDEX_GROUP_FIELD_TITLE,
                 file_system_db,
                 &gid_in_record);

        if (permissions & S_IRGRP &&
            medusa_group_contains (gid_in_record, uid_in_query)) {
#ifdef PERMISSIONS_CHECK_DEBUG
                g_print ("Returning TRUE because file is group readable, and user is in group that owns the file\n");
#endif
                return TRUE;
        }

#ifdef PERMISSIONS_CHECK_DEBUG
        if (permissions & S_IOTH) {
                g_print ("Returning  because file is other readable\n");
        }
        else {
                g_print ("Returning FALSE; file is not readable by user %d\n", uid_in_query);
        }
#endif
        return permissions & S_IROTH;

}

gboolean
medusa_file_index_marked_with_keyword (gpointer data,
                                       gpointer record,
                                       MedusaQueryArgument keyword_argument)
{
        char *keyword;
        char keywords_string[FILE_NAME_MAXIMUM_LENGTH];
        MedusaDatabaseAndUIDData *database_and_uid;
        MedusaKeywordSet *keyword_set;
        MedusaFileSystemDB *file_system_db;
        MedusaRDBRecord database_record;
        gboolean marked;

        database_and_uid = (MedusaDatabaseAndUIDData *) data;
        file_system_db = (MedusaFileSystemDB *) database_and_uid->database;
        database_record = (MedusaRDBRecord) record;
        keyword = keyword_argument.string;
        
        medusa_rdb_record_get_field_value
                (database_record, 
                 medusa_file_system_db_get_field_info (file_system_db),
                 MEDUSA_FILE_INDEX_KEYWORDS_FIELD_TITLE,
                 file_system_db, keywords_string);
        
        /* Convert into a keyword set and od the match check. */
        keyword_set = medusa_keyword_set_new_from_string (keywords_string);
        marked = medusa_keyword_set_has_keyword
                (keyword_set, database_and_uid->uid_of_user_making_query, keyword);
        medusa_keyword_set_destroy (keyword_set);

        return marked;
}

gboolean
medusa_file_index_not_marked_with_keyword (gpointer data,
                                           gpointer record,
                                           MedusaQueryArgument keyword_argument)
{
        return !medusa_file_index_marked_with_keyword (data, record, keyword_argument);
}

gboolean 
medusa_file_index_verify_file_is_modified_before_time (const char *uri,
                                                       MedusaQueryArgument argument)
{
        GnomeVFSFileInfo file_info;
        GnomeVFSResult get_info_result;
        long time_to_be_before;

        time_to_be_before = argument.number;

        get_info_result = gnome_vfs_get_file_info (uri,
                                                   &file_info,
                                                   GNOME_VFS_FILE_INFO_DEFAULT);

        /* If something happened to the file, don't return it */
        if (get_info_result != GNOME_VFS_OK) {
                return FALSE;
        }
        else {
                return file_info.mtime < time_to_be_before;
        }
}


gboolean 
medusa_file_index_verify_file_is_modified_after_time (const char *uri,
                                                      MedusaQueryArgument argument)
{
        GnomeVFSFileInfo file_info;
        GnomeVFSResult get_info_result;
        long time_to_be_after;

        time_to_be_after = argument.number;

        get_info_result = gnome_vfs_get_file_info (uri,
                                                   &file_info,
                                                   GNOME_VFS_FILE_INFO_DEFAULT);

        /* If something happened to the file, don't return it */
        if (get_info_result != GNOME_VFS_OK) {
                return FALSE;
        }
        else {
                return file_info.mtime > time_to_be_after;
        }
}

