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

/* medusa-rdb-table.c:  The implementation of the relational database
   table object */

#include <medusa-rdb-table-private.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <medusa-conf.h>
#include <medusa-test.h>
#include <string.h>
#include <time.h>

MedusaRDBTable *      
medusa_rdb_table_all_rows (MedusaRDBFile *file)
{
	MedusaRDBTable *table;

	table = g_new0 (MedusaRDBTable, 1);
  
	table->file = file;
	table->number_of_records = file->number_of_records;

	return table;
}

void                         
medusa_rdb_table_insert (MedusaRDBTable *table,
			 gpointer data) 
{
	
	medusa_io_handler_append_data (table->file->io_handler, data, table->file->field_info->record_size);
	/* Assure that contents pointer is correct in case of remap */
	/* FIXME bugzilla.eazel.com 2886:  
	   This should be a method within medusa-rdb-file */
	table->file->header = table->file->io_handler->mapped_region + table->file->io_handler->header_length;
	table->file->metainfo = table->file->header + table->file->header_length;
	table->file->contents = table->file->metainfo + table->file->metainfo_length;
	
	table->file->number_of_records++;
	table->number_of_records++;
  
}

void
medusa_rdb_table_replace_record_by_number (MedusaRDBTable *table,
					   int record_number,
					   gpointer new_record)
{
	g_return_if_fail (record_number < table->file->number_of_records);
	memcpy (&table->file->contents[record_number * table->file->field_info->record_size],
		new_record,
		table->file->field_info->record_size);
}




void
medusa_rdb_table_free (MedusaRDBTable *table)
{
	medusa_rdb_file_free (table->file);
	g_free (table);
}

MedusaRDBRecord
medusa_rdb_record_number_to_record (MedusaRDBTable *table,
				    MedusaRDBRecordNumber number)
{
	g_return_val_if_fail (number >= 0, NULL);
	g_return_val_if_fail (number < table->file->number_of_records, NULL);

	return &table->file->contents[table->file->field_info->record_size * number];
}

int
medusa_rdb_table_get_number_of_records (MedusaRDBTable *table)
{
	return table->number_of_records;
}



