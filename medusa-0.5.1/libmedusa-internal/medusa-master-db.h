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

/* medusa-master-db.h:  The top level db
   that requests items from the file index and the text index */

#ifndef MEDUSA_MASTER_DB_H
#define MEDUSA_MASTER_DB_H

typedef struct MedusaMasterDB MedusaMasterDB;

#include <libgnomevfs/gnome-vfs-types.h>
#include <libmedusa/medusa-log.h>
#include "medusa-search-uri.h"
#include "medusa-text-index.h"

#ifndef MEDUSA_PARSED_SEARCH_URI_DEFINED
#define MEDUSA_PARSED_SEARCH_URI_DEFINED
typedef struct MedusaParsedSearchURI MedusaParsedSearchURI;
#endif

/* Create an index, starting with the root uri.
   Index name can be the empty string ("") */
void             medusa_master_db_create_index     (const char     *root_uri,
						    const char     *index_name,
						    gboolean       use_idle_service,
						    MedusaLogLevel log_level);

/* Open an existing database for reading */
MedusaMasterDB * medusa_master_db_open             (const char     *root_uri,
						    const char     *index_name,
						    MedusaLogLevel  log_level);

/* Swap out the old index for the newly constructed index */
void            medusa_master_db_move_completed_index_into_place        (const char *index_name);

/* This is used to erase temporary indices created for account or other purposes */
void            medusa_master_db_erase_constructed_index                (const char *index_name);


/* Returns a list of URIs */
GList *         medusa_master_db_run_search        (MedusaMasterDB *db,
						    MedusaParsedSearchURI *parsed_search_uri);

void            medusa_master_db_ref               (MedusaMasterDB *db);
void            medusa_master_db_unref             (MedusaMasterDB *db);


gboolean        medusa_master_db_index_files_are_still_valid            (const char *index_name,
									 time_t newest_valid_modified_time);


#endif /* MEDUSA_MASTER_DB_H */
