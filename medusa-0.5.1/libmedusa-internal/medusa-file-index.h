/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

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

/* medusa-file-index.h:  Describes the file system database API. */

#ifndef MEDUSA_FILE_INDEX_H
#define MEDUSA_FILE_INDEX_H

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>
#include <stdio.h>
#include <libgnomevfs/gnome-vfs-types.h>

#include "medusa-query-clauses.h"
#include "medusa-rdb-record.h"
#include "medusa-rdb-table.h"
#include "medusa-string-list.h"
#include "medusa-uri-list.h"


typedef struct MedusaFileSystemDB MedusaFileSystemDB;
typedef MedusaRDBRecordNumbers * MedusaRecordHandle;

struct MedusaFileSystemDB {
	MedusaRDBTable *file_database;
	
	/* Root of the indexing. */
	char *root_directory;
	char *version;

	/* FIXME bugzilla.eazel.com 2677:
	   last_index_time is 0 if the index is new.  This is probably a bad 
	   idea, and maybe there should be a boolean in here */
	time_t last_index_time;
	time_t indexing_start_time;

	MedusaStringList *mime_types;
	MedusaStringList *keyword_sets;

	/* The index's vocabulary */
	MedusaQueryClauses *clauses_using_index_only;
	MedusaQueryClauses *clauses_using_index_and_uid;

	/* State for the multi-pass indexing of keywords. */
	GHashTable *keywords_by_file;
	GHashTable *private_keyword_users_by_directory;
};

MedusaFileSystemDB *    medusa_file_system_db_new                             (const char         *index_name);
MedusaFileSystemDB *    medusa_file_system_db_open                            (const char         *index_name);
void                    medusa_file_system_db_index_file                      (MedusaFileSystemDB *db,
									       int                 uri_number,
									       GnomeVFSFileInfo   *file_info);
void                    medusa_file_system_db_move_completed_index_into_place (const char         *index_name);
void                    medusa_file_system_db_erase_constructed_index         (const char         *index_name);
gboolean                medusa_file_system_db_index_files_are_still_valid     (const char *index_name,
									       time_t newest_valid_modified_time);

/* Keyword indexing. */
void                    medusa_file_system_db_add_public_keyword              (MedusaFileSystemDB *db,
									       const char         *file_path,
									       const char         *keyword);
void                    medusa_file_system_db_add_private_keyword             (MedusaFileSystemDB *db,
									       const char         *file_path,
									       const char         *keyword,
									       uid_t               user);
void                    medusa_file_system_db_add_private_keywords_directory  (MedusaFileSystemDB *db,
									       const char         *directory_path,
									       uid_t               user);
void                    medusa_file_system_db_update_keywords                 (MedusaFileSystemDB *db,
									       MedusaURIList      *uri_list);

/* FIXME bugzilla.eazel.com 2712: 
   Not implemented yet */
void                    medusa_file_system_db_update_file                    (MedusaFileSystemDB *db);
int                     medusa_file_system_db_get_number_of_records          (MedusaFileSystemDB *db);
MedusaRDBFieldInfo *    medusa_file_system_db_get_field_info                 (MedusaFileSystemDB *db);
MedusaQueryClauses *    medusa_file_system_db_get_query_clauses_using_index_only (MedusaFileSystemDB *db);
MedusaQueryClauses *    medusa_file_system_db_get_query_clauses_using_index_and_uid (MedusaFileSystemDB *db);
void                    medusa_file_system_db_free                           (MedusaFileSystemDB *db);
void                    medusa_file_index_test                               (void);

#endif /* MEDUSA_FILE_INDEX_H */
