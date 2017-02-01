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
 *  medusa-rdb-index.h  -  The header file for database file level operations,
 *  the file object is responsible for keeping pointers to records in the
 *  right place in the actual database files.
 */

#ifndef MEDUSA_RDB_INDEX
#define MEDUSA_RDB_INDEX

#include <medusa-relational-database.h>

typedef struct _MedusaRelationalDatabaseIndex MedusaRelationalDatabaseIndex;
typedef GSList MedusaRelationalDatabaseIndexes;

/* Indexing on specific fields */
/* This is a mapping from a set of fields to single records */
struct _MedusaRelationalDatabaseIndex {
  char *version;
  MedusaRelationalDatabase *database;
  MedusaRelationalDatabaseFields *index_fields;
};

#endif MEDUSA_RDB_INDEX
