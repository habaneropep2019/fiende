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

/* medusa-rdb-table.h:  the api for creating and manipulating relational
   database tables.  */

#ifndef MEDUSA_RDB_TABLE_H
#define MEDUSA_RDB_TABLE_H

#include "medusa-rdb-file.h"
 
typedef struct MedusaRDBTable MedusaRDBTable;

struct MedusaRDBTable {

  MedusaRDBFile *file;
  int number_of_records;

};



/* General note to the table manipulation functions:
   Select creates a virtual table, and so does not alter any files.
   Insert/delete/replace will alter files
   Functions denoted with the term "virtual" do not modify files,
   they simply add/delete/replace records in tables */

/* Creates a table object with the same rows as a physical database file */
MedusaRDBTable *     medusa_rdb_table_all_rows                 (MedusaRDBFile *file);

/* Inserts record into table, and also into physical database */
void                 medusa_rdb_table_insert                   (MedusaRDBTable *table,
								gpointer data); 

void                 medusa_rdb_table_replace_record_by_number (MedusaRDBTable *table,
								int record_number,
								gpointer new_record);

void                 medusa_rdb_table_free                     (MedusaRDBTable *table);

/* FIXME bugzilla.eazel.com 2891: 
   This should go in medusa-rdb-record.h */
MedusaRDBRecord      medusa_rdb_record_number_to_record        (MedusaRDBTable *table,
								MedusaRDBRecordNumber number);

int                  medusa_rdb_table_get_number_of_records    (MedusaRDBTable *table);


#endif /* MEDUSA_RDB_TABLE_H */
