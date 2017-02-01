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
#include <unistd.h>

#include "medusa-file-index.h"

#include "medusa-conf.h"
#include "medusa-file-index-encoders.h"
#include "medusa-file-index-queries.h"
#include "medusa-file-information.h"
#include "medusa-index-filenames.h"
#include "medusa-keyword-set.h"
#include "medusa-query-clauses.h"
#include "medusa-search-uri.h"
#include "medusa-string-list.h"
#include <libgnomevfs/gnome-vfs-utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_ATTRIBUTE_DATABASE_PREFIX  "file-system-db"
#define MIME_TYPE_PREFIX  "mime-types"
#define KEYWORD_SETS_PREFIX  "keyword-sets"
#define FILE_INDEX_METAINFO_LENGTH 100

/* Check that the field information in the header (just the titles) matches what we
   expected. */
static MedusaRDBFieldInfo *  file_index_field_info                              (void);
static MedusaQueryClauses *  file_index_clauses_using_index_only                (void);
static MedusaQueryClauses *  file_index_clauses_using_index_and_uid             (void);
static void                  read_index_info_from_database                      (MedusaFileSystemDB *file_system_db);
static void                  write_index_info_to_database                       (MedusaFileSystemDB *file_system_db,
                                                                                 int last_indexing_time);

typedef void (* AddFieldCallback) (const char *title, int size,
                                   MedusaRDBEncodeFunc encoder,
                                   MedusaRDBDecodeFunc decoder,
                                   gpointer callback_data);

static void
add_fields (AddFieldCallback callback, gpointer callback_data)
{
        /* FIXME bugzilla.eazel.com 2676: 
           Casting function types is deprecated.  It would be
         * much better to use data type casting in the encode and
         * decode functions instead.
         */
        (* callback) (MEDUSA_FILE_INDEX_URI_NUMBER_FIELD_TITLE,
                      MEDUSA_FILE_INDEX_URI_NUMBER_FIELD_SIZE,
                      (MedusaRDBEncodeFunc) medusa_file_database_uri_number_encode,
                      (MedusaRDBDecodeFunc) medusa_file_database_uri_number_decode,
                      callback_data);
        (* callback) (MEDUSA_FILE_INDEX_MTIME_FIELD_TITLE,
                      MEDUSA_FILE_INDEX_MTIME_FIELD_SIZE,
                      (MedusaRDBEncodeFunc) medusa_file_database_mtime_encode,
                      (MedusaRDBDecodeFunc) medusa_file_database_mtime_decode,
                      callback_data);
        (* callback) (MEDUSA_FILE_INDEX_OWNER_FIELD_TITLE,
                      MEDUSA_FILE_INDEX_OWNER_FIELD_SIZE,
                      (MedusaRDBEncodeFunc) medusa_file_database_owner_encode,
                      (MedusaRDBDecodeFunc) medusa_file_database_owner_decode,
                      callback_data);
        (* callback) (MEDUSA_FILE_INDEX_GROUP_FIELD_TITLE,
                      MEDUSA_FILE_INDEX_GROUP_FIELD_SIZE,
                      (MedusaRDBEncodeFunc) medusa_file_database_group_encode,
                      (MedusaRDBDecodeFunc) medusa_file_database_group_decode,
                      callback_data);
        (* callback) (MEDUSA_FILE_INDEX_PERMISSIONS_FIELD_TITLE,
                      MEDUSA_FILE_INDEX_PERMISSIONS_FIELD_SIZE,
                      (MedusaRDBEncodeFunc) medusa_file_database_permissions_encode,
                      (MedusaRDBDecodeFunc) medusa_file_database_permissions_decode,
                      callback_data);
        (* callback) (MEDUSA_FILE_INDEX_SIZE_FIELD_TITLE,
                      MEDUSA_FILE_INDEX_SIZE_FIELD_SIZE,
                      (MedusaRDBEncodeFunc) medusa_file_database_size_encode,
                      (MedusaRDBDecodeFunc) medusa_file_database_size_decode,
                      callback_data);
        (* callback) (MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE,
                      MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_SIZE,
                      (MedusaRDBEncodeFunc) medusa_file_database_mime_type_encode,
                      (MedusaRDBDecodeFunc) medusa_file_database_mime_type_decode,
                      callback_data);
        (* callback) (MEDUSA_FILE_INDEX_KEYWORDS_FIELD_TITLE,
                      MEDUSA_FILE_INDEX_KEYWORDS_FIELD_SIZE,
                      (MedusaRDBEncodeFunc) medusa_file_database_keywords_encode,
                      (MedusaRDBDecodeFunc) medusa_file_database_keywords_decode,
                      callback_data);
}

static void
medusa_rdb_file_add_field_cover (const char *title, int size,
                                 MedusaRDBEncodeFunc encoder,
                                 MedusaRDBDecodeFunc decoder,
                                 gpointer callback_data)
{
        medusa_rdb_file_add_field ((MedusaRDBFile *) callback_data,
                                   title, size, encoder, decoder);
}

MedusaFileSystemDB *
medusa_file_system_db_new (const char *index_name)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBFile *db_file;
        char *file_system_db_file_name, *mime_type_list_name, *keyword_sets_list_name;
        
        file_system_db = g_new0 (MedusaFileSystemDB, 1);

        file_system_db_file_name = medusa_generate_index_filename (FILE_ATTRIBUTE_DATABASE_PREFIX, 
                                                                   index_name, 
                                                                   TRUE);
        db_file = medusa_rdb_file_new (file_system_db_file_name, FILE_INDEX_METAINFO_LENGTH);
        g_free (file_system_db_file_name);

        /* Create fields and reserved first record */
        add_fields (medusa_rdb_file_add_field_cover, db_file);
        
        file_system_db->file_database = medusa_rdb_table_all_rows (db_file);
        
        write_index_info_to_database (file_system_db, 0);
        file_system_db->version = g_strdup (MEDUSA_INDEX_FORMAT_VERSION);

        file_system_db->indexing_start_time = time (NULL);

        mime_type_list_name = medusa_generate_index_filename (MIME_TYPE_PREFIX,
                                                              index_name,
                                                              TRUE);
        keyword_sets_list_name = medusa_generate_index_filename (KEYWORD_SETS_PREFIX,
                                                              index_name,
                                                              TRUE);
        file_system_db->mime_types = medusa_string_list_new
                (mime_type_list_name, MEDUSA_STRING_LIST_STRING_TO_KEY_WRITE_CACHE);
        file_system_db->keyword_sets = medusa_string_list_new
                (keyword_sets_list_name, MEDUSA_STRING_LIST_STRING_TO_KEY_WRITE_CACHE);
        /* FIXME: No check whether the above operations (creating string lists) succeeded */
        g_free (mime_type_list_name);
        g_free (keyword_sets_list_name);
        file_system_db->keywords_by_file = g_hash_table_new
                (g_str_hash, g_str_equal);
        file_system_db->private_keyword_users_by_directory = g_hash_table_new
                (g_str_hash, g_str_equal);

        return file_system_db;
}


MedusaFileSystemDB *
medusa_file_system_db_open (const char *index_name)
{
        MedusaFileSystemDB *file_system_db;
        MedusaRDBFile *db_file;
        char *attribute_index_filename, *mime_type_filename, *keyword_sets_filename;
        


        attribute_index_filename = medusa_generate_index_filename (FILE_ATTRIBUTE_DATABASE_PREFIX,
                                                                   index_name,
                                                                   FALSE);
        db_file = medusa_rdb_file_open (attribute_index_filename, FILE_INDEX_METAINFO_LENGTH);
        g_free (attribute_index_filename);

        if (db_file == NULL) {
                /* FIXME: bugzilla.eazel.com 4557: log error here */
                g_warning ("Couldn't create file attribute index file\n");
                return NULL;
        }

        file_system_db = g_new0 (MedusaFileSystemDB, 1);
        
        /* Load the correct field info from the hard coded procedure */
        db_file->field_info = file_index_field_info ();
        file_system_db->file_database = medusa_rdb_table_all_rows (db_file);
        read_index_info_from_database (file_system_db);
        g_return_val_if_fail (file_system_db->version != NULL, NULL);

        file_system_db->clauses_using_index_only = file_index_clauses_using_index_only ();
        file_system_db->clauses_using_index_and_uid = file_index_clauses_using_index_and_uid ();
        file_system_db->indexing_start_time = time (NULL);

        mime_type_filename = medusa_generate_index_filename (MIME_TYPE_PREFIX,
                                                             index_name,
                                                             FALSE);
        file_system_db->mime_types = medusa_string_list_open
                (mime_type_filename, MEDUSA_STRING_LIST_CACHE_KEY_TO_STRING);
        g_free (mime_type_filename);
        if (file_system_db->mime_types == NULL) {
                /* FIXME: bugzilla.eazel.com 4557: log error here */
                g_warning ("Couldn't create mime type hash file\n");
                goto cleanup;
        }
        
        keyword_sets_filename = medusa_generate_index_filename (KEYWORD_SETS_PREFIX,
                                                                index_name,
                                                                FALSE);
        file_system_db->keyword_sets = medusa_string_list_open
                (keyword_sets_filename, MEDUSA_STRING_LIST_CACHE_KEY_TO_STRING);
        g_free (keyword_sets_filename);
        if (file_system_db->keyword_sets == NULL) {
                /* FIXME: bugzilla.eazel.com 4557: log error here */
                g_warning ("Couldn't create keyword hash file\n");
                goto cleanup;
        }       


        file_system_db->keywords_by_file = g_hash_table_new
                (g_str_hash, g_str_equal);
        file_system_db->private_keyword_users_by_directory = g_hash_table_new
                (g_str_hash, g_str_equal);

        return file_system_db;

 cleanup:
        if (file_system_db->mime_types != NULL) {
                medusa_string_list_destroy (file_system_db->mime_types);
        }
        g_free (file_system_db);
        return NULL;
}



int                  
medusa_file_system_db_get_number_of_records (MedusaFileSystemDB *db)
{
        return medusa_rdb_table_get_number_of_records (db->file_database);
}

MedusaRDBFieldInfo *
medusa_file_system_db_get_field_info (MedusaFileSystemDB *db)
{
        return db->file_database->file->field_info;
}

static void
keywords_by_file_destroy_entry (gpointer key, gpointer value, gpointer callback_data)
{
        g_assert (callback_data == NULL);
        
        g_free (key);
        medusa_keyword_set_destroy (value);
}

static void
users_by_directory_destroy_entry (gpointer key, gpointer value, gpointer callback_data)
{
        g_assert (callback_data == NULL);
        
        g_free (key);
        g_list_free (value);
}

void 
medusa_file_system_db_free (MedusaFileSystemDB *file_system_db)
{
        /* Write last indexing time information into the file */
        write_index_info_to_database (file_system_db,
                                      file_system_db->indexing_start_time);
        medusa_rdb_table_free (file_system_db->file_database);
        g_free (file_system_db->version);

        medusa_string_list_destroy (file_system_db->mime_types);
        medusa_string_list_destroy (file_system_db->keyword_sets);
        medusa_query_clauses_unref (file_system_db->clauses_using_index_only);
        medusa_query_clauses_unref (file_system_db->clauses_using_index_and_uid);
        g_hash_table_foreach (file_system_db->keywords_by_file,
                              keywords_by_file_destroy_entry, NULL);
        g_hash_table_destroy (file_system_db->keywords_by_file);
        g_hash_table_foreach (file_system_db->private_keyword_users_by_directory,
                              users_by_directory_destroy_entry, NULL);
        g_hash_table_destroy (file_system_db->private_keyword_users_by_directory);
        g_free (file_system_db);
}






void
medusa_file_system_db_index_file (MedusaFileSystemDB *file_system_db, 
                                  int uri_number,
                                  GnomeVFSFileInfo *file_info)
{
        MedusaFileAttributes *file_attributes;
        file_attributes = medusa_file_attributes_new (uri_number, file_info);
        medusa_index_file_attributes (file_attributes, file_system_db);
        medusa_file_attributes_free (file_attributes);
}

void                    
medusa_file_system_db_erase_constructed_index (const char         *index_name)
{
        medusa_erase_constructed_index_file (FILE_ATTRIBUTE_DATABASE_PREFIX,
                                             index_name);
        medusa_erase_constructed_index_file (MIME_TYPE_PREFIX,
                                             index_name);
        medusa_erase_constructed_index_file (KEYWORD_SETS_PREFIX,
                                             index_name);
}

void
medusa_file_system_db_move_completed_index_into_place (const char     *index_name)
{
        medusa_move_completed_index_file_into_place (FILE_ATTRIBUTE_DATABASE_PREFIX,
                                                     index_name);
        medusa_move_completed_index_file_into_place (MIME_TYPE_PREFIX,
                                                     index_name);
        medusa_move_completed_index_file_into_place (KEYWORD_SETS_PREFIX,
                                                     index_name);
}

gboolean                
medusa_file_system_db_index_files_are_still_valid     (const char *index_name,
                                                       time_t newest_valid_modified_time)
{
        return medusa_index_file_is_newer_than_time (FILE_ATTRIBUTE_DATABASE_PREFIX,
                                                     index_name,
                                                     newest_valid_modified_time) &&
                medusa_index_file_is_newer_than_time (MIME_TYPE_PREFIX,
                                                      index_name,
                                                      newest_valid_modified_time) &&
                medusa_index_file_is_newer_than_time (KEYWORD_SETS_PREFIX,
                                                      index_name,
                                                      newest_valid_modified_time);
}


static void
write_index_info_to_database (MedusaFileSystemDB *file_system_db, int last_index_time)
{
        char *metainfo; 

        metainfo = g_strdup_printf ("%s0%d0",MEDUSA_INDEX_FORMAT_VERSION,
                                    last_index_time);
        medusa_rdb_file_set_metainfo (file_system_db->file_database->file,
                                      metainfo);
        g_free (metainfo);
}

static void
read_index_info_from_database (MedusaFileSystemDB *file_system_db)
{
        char *metainfo, *version;
        int last_index_time;
        
        metainfo = medusa_rdb_file_get_metainfo (file_system_db->file_database->file);
        version = g_new (char, file_system_db->file_database->file->metainfo_length);
        sscanf (metainfo, "%s0%d0", version, &last_index_time);
        file_system_db->version = g_strdup (version);
        file_system_db->last_index_time = last_index_time;
        g_free (metainfo);
        g_free (version);

        /* FIXME bugzilla.eazel.com 2675: 
           Check version here. */
}
                              
static void
medusa_rdb_field_add_cover (const char *title, int size,
                            MedusaRDBEncodeFunc encoder,
                            MedusaRDBDecodeFunc decoder,
                            gpointer callback_data)
{
        medusa_rdb_field_add ((MedusaRDBFieldInfo *) callback_data,
                              title, size, encoder, decoder);
}

static MedusaRDBFieldInfo *
file_index_field_info (void)
{
        MedusaRDBFieldInfo *db_field_info;

        db_field_info = medusa_rdb_field_info_new ();
        add_fields (medusa_rdb_field_add_cover, db_field_info);
        return db_field_info;
}

static MedusaQueryClauses *
file_index_clauses_using_index_only (void)
{
        MedusaQueryClauses *clauses;

        clauses = medusa_query_clauses_new ();
        medusa_query_clauses_add_clause (clauses,
                                         "file_type",
                                         "is",
                                          medusa_file_index_is_of_type,
                                         MEDUSA_ARGUMENT_TYPE_STRING);
        medusa_query_clauses_add_clause (clauses,
                                         "file_type",
                                         "is_not",
                                          medusa_file_index_is_not_of_type,
                                         MEDUSA_ARGUMENT_TYPE_STRING);
        medusa_query_clauses_add_clause_with_check (clauses,
                                                    "mtime",
                                                    "is_before",
                                                     medusa_file_index_is_modified_before_time,
                                                    medusa_file_index_verify_file_is_modified_before_time,
                                                    MEDUSA_ARGUMENT_TYPE_NUMBER);
        medusa_query_clauses_add_clause_with_check (clauses,
                                                    "mtime",
                                                    "is_after",
                                                     medusa_file_index_is_modified_after_time,
                                                    medusa_file_index_verify_file_is_modified_after_time,
                                                    MEDUSA_ARGUMENT_TYPE_NUMBER);
        medusa_query_clauses_add_clause_with_check (clauses,
                                                    "modified",
                                                    "is_not",
                                                    medusa_file_index_is_not_modified_on_date,
                                                    medusa_file_index_verify_file_is_not_modified_on_date,
                                                    MEDUSA_ARGUMENT_TYPE_STRING);
        medusa_query_clauses_add_clause (clauses,
                                         "owner",
                                         "has_uid",
                                          medusa_file_index_has_uid,
                                         MEDUSA_ARGUMENT_TYPE_NUMBER);
        medusa_query_clauses_add_clause (clauses,
                                         "owner",
                                         "does_not_have_uid",
                                          medusa_file_index_does_not_have_uid,
                                         MEDUSA_ARGUMENT_TYPE_NUMBER);
        medusa_query_clauses_add_clause (clauses,
                                         "group",
                                         "has_gid",
                                          medusa_file_index_has_gid,
                                         MEDUSA_ARGUMENT_TYPE_NUMBER);
        medusa_query_clauses_add_clause (clauses,
                                         "group",
                                         "does_not_have_gid",
                                          medusa_file_index_does_not_have_gid,
                                         MEDUSA_ARGUMENT_TYPE_NUMBER);
        medusa_query_clauses_add_clause (clauses,
                                         "size",
                                         "larger_than",
                                          medusa_file_index_is_larger_than,
                                         MEDUSA_ARGUMENT_TYPE_NUMBER);
        medusa_query_clauses_add_clause (clauses,
                                         "size",
                                         "smaller_than",
                                          medusa_file_index_is_smaller_than,
                                         MEDUSA_ARGUMENT_TYPE_NUMBER);
        medusa_query_clauses_add_clause (clauses,
                                         "size",
                                         "is",
                                          medusa_file_index_is_of_size,
                                         MEDUSA_ARGUMENT_TYPE_NUMBER);
        medusa_query_clauses_add_clause (clauses,
                                         "permissions_to_read",
                                         "include_uid",
                                          medusa_file_index_uid_can_read_file,
                                         MEDUSA_ARGUMENT_TYPE_NUMBER);

        return clauses;
}

static MedusaQueryClauses *
file_index_clauses_using_index_and_uid (void)
{
        MedusaQueryClauses *clauses;

        clauses = medusa_query_clauses_new ();
        medusa_query_clauses_add_clause (clauses,
                                         "keywords",
                                         "include",
                                          medusa_file_index_marked_with_keyword,
                                         MEDUSA_ARGUMENT_TYPE_STRING);
        medusa_query_clauses_add_clause (clauses,
                                         "keywords",
                                         "do_not_include",
                                          medusa_file_index_not_marked_with_keyword,
                                         MEDUSA_ARGUMENT_TYPE_STRING);
        return clauses;
}


MedusaQueryClauses *
medusa_file_system_db_get_query_clauses_using_index_only (MedusaFileSystemDB *db)
{
        return db->clauses_using_index_only;
}

MedusaQueryClauses *
medusa_file_system_db_get_query_clauses_using_index_and_uid (MedusaFileSystemDB *db)
{
        return db->clauses_using_index_and_uid;
}

static MedusaKeywordSet *
get_keyword_set_for_file (MedusaFileSystemDB *db,
                          const char *path)
{
        MedusaKeywordSet *set;

        /* Find existing set. */
        set = g_hash_table_lookup (db->keywords_by_file, path);
        if (set != NULL) {
                return set;
        }

        /* Create a new empty one. */
        set = medusa_keyword_set_new ();
        g_hash_table_insert (db->keywords_by_file, g_strdup (path), set);
        return set;
}

void
medusa_file_system_db_add_public_keyword (MedusaFileSystemDB *db,
                                          const char *path,
                                          const char *keyword)
{
        medusa_keyword_set_add_public_keyword
                (get_keyword_set_for_file (db, path), keyword);
}

void
medusa_file_system_db_add_private_keyword (MedusaFileSystemDB *db,
                                           const char *path,
                                           const char *keyword,
                                           uid_t user)
{
        medusa_keyword_set_add_private_keyword
                (get_keyword_set_for_file (db, path), user, keyword);
}

void
medusa_file_system_db_add_private_keywords_directory (MedusaFileSystemDB *db,
                                                      const char *path,
                                                      uid_t user)
{
        GList *list;
        gpointer user_as_pointer;
        char *key;
        
        user_as_pointer = GINT_TO_POINTER ((int) user);

        /* Get the list and check if this user is already in there. */
        list = g_hash_table_lookup (db->private_keyword_users_by_directory, path);
        if (g_list_find (list, user_as_pointer) != NULL) {
                return;
        }

        /* Deciding whether to dup the key or not depends on whether
         * there was already an entry. Entries keep their existing key
         * if they already have one, strange as that may sound.
         */
        key = list == NULL ? g_strdup (path) : (char *) path;

        /* Add the user to the list. */
        list = g_list_prepend (list, user_as_pointer);
        g_hash_table_insert (db->private_keyword_users_by_directory,
                             key, list);
}

static void
add_users_to_keyword_set (MedusaKeywordSet *set, GList *users)
{
        GList *p;
        uid_t user;

        for (p = users; p != NULL; p = p->next) {
                user = GPOINTER_TO_INT (p->data);
                medusa_keyword_set_add_user_with_private_keywords (set, user);
        }
}

void
medusa_file_system_db_update_keywords (MedusaFileSystemDB *db,
                                       MedusaURIList *uri_list)
{
        MedusaRDBFieldInfo *field_info;
        int num_records, i;
        MedusaRDBRecord record;
        int uri_number;
        char *uri, *path, *parent_path, *keyword_set_as_string;
        MedusaKeywordSet *keyword_set;
        GList *user_list;

        /* Since we just index by record number, get the number first. */
        num_records = medusa_rdb_table_get_number_of_records (db->file_database);
        field_info = medusa_file_system_db_get_field_info (db);
        for (i = 0; i < num_records; i++) {
                /* Find the record. */
                record = medusa_rdb_record_number_to_record (db->file_database, i);
                
                /* Get the path. */
                medusa_rdb_record_get_field_value
                        (record, field_info,
                         MEDUSA_FILE_INDEX_URI_NUMBER_FIELD_TITLE,
                         db, &uri_number);
                uri = medusa_uri_number_to_uri (uri_list, uri_number);
                if (uri == NULL) {
                        continue;
                }
                path = gnome_vfs_get_local_path_from_uri (uri);
                g_free (uri);
                if (path == NULL) {
                        continue;
                }

                /* Get the keyword list and convert to string format. */
                keyword_set = g_hash_table_lookup (db->keywords_by_file, path);
                if (keyword_set == NULL) {
                        g_free (path);
                        continue;
                }
                parent_path = g_dirname (path);
                if (parent_path != NULL) {
                        user_list = g_hash_table_lookup (db->private_keyword_users_by_directory,
                                                         parent_path);
                        add_users_to_keyword_set (keyword_set, user_list);
                        g_free (parent_path);
                }
                g_free (path);
                keyword_set_as_string = medusa_keyword_set_get_string_form
                        (keyword_set);

                /* Store in the database. */
                if (keyword_set_as_string[0] != '\0') {
                        medusa_rdb_record_set_field_value
                                (record, field_info,
                                 MEDUSA_FILE_INDEX_KEYWORDS_FIELD_TITLE,
                                 db, keyword_set_as_string);
                }
                g_free (keyword_set_as_string);
        }
}

