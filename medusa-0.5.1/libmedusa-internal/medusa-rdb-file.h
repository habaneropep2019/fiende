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
 *
 *  medusa-rdb-file.h  -  The header file for database file level operations,
 *  the file object is responsible for keeping pointers to records in the
 *  right place in the actual database files.
 */

#ifndef MEDUSA_RDB_FILE_H
#define MEDUSA_RDB_FILE_H

/* medusa-rdb-file.h: The API for the relational database file, which
   is an on disk database of information, organized into records */

#include "medusa-rdb-fields.h"
#include "medusa-tokenize.h"

typedef struct MedusaRDBFile MedusaRDBFile;


#define MEDUSA_RDB_FILE_MAGIC_NUMBER        "9122"
#define MEDUSA_RDB_FILE_VERSION             "0.1"


/* The structure used to store the file database */
struct MedusaRDBFile {
	char *version;
	char *database_file_name;
	
	MedusaIOHandler *io_handler;
	char *header;
	int header_length;
	char *metainfo;
	int metainfo_length;

	
	/* Record content information */
	MedusaRDBFieldInfo *field_info;                
	
	char *contents;
	int number_of_records;

	gboolean read_only;

};



MedusaRDBFile *   medusa_rdb_file_new          (const char *file_name,
						int metainfo_length);
MedusaRDBFile *   medusa_rdb_file_open         (const char *file_name,
						int metainfo_length);

void              medusa_rdb_file_free         (MedusaRDBFile *file);
gboolean          medusa_rdb_file_is_empty     (MedusaRDBFile *file);

/* Sets extra data used to store apropos information,
   such as version number, last transaction time, etc */
void              medusa_rdb_file_set_metainfo (MedusaRDBFile *file,
						char *metainfo);
/* Gets a copy of the metainfo stored in the file;
   should be freed afterwards */
char *            medusa_rdb_file_get_metainfo (MedusaRDBFile *file);


						
void              medusa_rdb_file_add_field    (MedusaRDBFile *file,
						const char *field_name,
						int field_size,
						MedusaRDBEncodeFunc encode,
						MedusaRDBDecodeFunc decode);
void              medusa_rdb_file_remove_field (MedusaRDBFile *file,
						char *field_name);
int               medusa_rdb_file_get_number_of_records 
(MedusaRDBFile *file);

void              medusa_rdb_file_test         (void);

#endif /* MEDUSA_RDB_FILE_H */




