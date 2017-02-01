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


#include <medusa-rdb-file.h>
#include <medusa-test-conf.h>
#include <medusa-test.h>
#include <medusa-byte.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



static int         medusa_rdb_file_read_header             (MedusaRDBFile *database);
static int         medusa_rdb_file_write_initial_header    (MedusaRDBFile *database);

static void        medusa_rdb_file_write_number_of_records (MedusaRDBFile *file,
                                                            int number_of_records);

static void        medusa_rdb_file_write_number_of_fields  (MedusaRDBFile *file,
                                                            int number_of_fields);


/* Test encoders and decoders */
static void        id_encode                               (char *result, 
                                                            int id, 
                                                            gpointer data);
static void        id_decode                               (int *id, 
                                                            char *field, 
                                                            gpointer data);
static void        number_encode                           (char *result, 
                                                            char number, 
                                                            gpointer data);
static void        number_decode                           (char *name, 
                                                            char *field, 
                                                            gpointer data);


MedusaRDBFile *
medusa_rdb_file_new (const char *file_name,
                     int metainfo_length)
{
        MedusaRDBFile *file;
        char *blank_data;

        file = g_new0 (MedusaRDBFile, 1);
  
        file->database_file_name = g_strdup (file_name);
        file->field_info = medusa_rdb_field_info_new ();
        file->metainfo_length = metainfo_length;

        file->io_handler = medusa_io_handler_new (file_name,
                                                  MEDUSA_RDB_FILE_MAGIC_NUMBER,
                                                  MEDUSA_RDB_FILE_VERSION,
                                                  0);
        /* Do database file initialization tasks */
        file->header_length = medusa_rdb_file_write_initial_header (file);
        blank_data = g_new0 (char, metainfo_length);
        medusa_io_handler_append_data (file->io_handler, blank_data, metainfo_length);
        g_free (blank_data);
        file->number_of_records = 0;
        file->header = file->io_handler->mapped_region + file->io_handler->header_length;
        file->metainfo = file->header + file->header_length;
        file->contents = file->metainfo + file->metainfo_length;

        file->version = file->io_handler->version;
        file->read_only = FALSE;

        return file;
}
 
MedusaRDBFile *
medusa_rdb_file_open (const char *file_name,
                     int metainfo_length) 
{
        MedusaRDBFile *file;

        file = g_new0 (MedusaRDBFile, 1);
  
        file->database_file_name = g_strdup (file_name);
        file->field_info = medusa_rdb_field_info_new ();
        file->metainfo_length = metainfo_length;

        if (access (file_name, R_OK | W_OK) != 0) {
                return NULL;
        }
        file->io_handler = medusa_io_handler_open (file_name,
                                                   MEDUSA_RDB_FILE_MAGIC_NUMBER,
                                                   MEDUSA_RDB_FILE_VERSION);
        file->header = file->io_handler->mapped_region + file->io_handler->header_length;
        file->header_length = medusa_rdb_file_read_header (file);
        file->metainfo = file->header + file->header_length;
        file->contents = file->metainfo + file->metainfo_length;

        file->version = file->io_handler->version;
        file->read_only = TRUE;
        return file;
}



gboolean
medusa_rdb_file_is_empty (MedusaRDBFile *file)
{
        return file->number_of_records == 0;
}

void
medusa_rdb_file_free (MedusaRDBFile *file)
{
        if (!file->read_only) {
                medusa_rdb_file_write_number_of_records (file, file->number_of_records);
                medusa_rdb_file_write_number_of_fields (file, file->field_info->number_of_fields);
        }
  
        medusa_io_handler_free (file->io_handler);
        g_free (file->database_file_name);
        medusa_rdb_field_info_free (file->field_info); 
        g_free (file); 
}



/* Header format for the database is as follows :
   (io handler header with version, etc. precede this)
   num_recordsnum_fields
   field name\0field name\0...\0
   This is not enough information to determine field info.
   This is all grabbed from static structures right now,
   but this is not done in this object.
   It is the responsibility of the code that uses the database 
   to file out the field_info structure */

static int        
medusa_rdb_file_read_header (MedusaRDBFile *file)
{

        /* Read the initial data: number of records in the database, and number of fields in the database */
        memcpy (&file->number_of_records, file->header, sizeof (int));
        memcpy (&file->field_info->number_of_fields, &file->header[sizeof(int)], sizeof (int));
#ifdef DATABASE_STATS
        g_message ("Number of records: %d\n", file->number_of_records);
        g_message ("Number of fields: %d\n", file->field_info->number_of_fields);
#endif
        g_return_val_if_fail (file->field_info->number_of_fields >= 0, 0);

        return 2*sizeof (int);
}


void
medusa_rdb_file_set_metainfo (MedusaRDBFile *file,
                              char *metainfo)
{
        g_return_if_fail (strlen (metainfo) < file->metainfo_length);
        memcpy (file->metainfo, metainfo, file->metainfo_length);
}


char *
medusa_rdb_file_get_metainfo (MedusaRDBFile *file)
{
        char *result;
        result = g_new0 (char, file->metainfo_length);
        memcpy (result, file->metainfo, file->metainfo_length);
        return result;
}


void
medusa_rdb_file_add_field (MedusaRDBFile *file,
                           const char *field_name,
                           int field_size,
                           MedusaRDBEncodeFunc encode,
                           MedusaRDBDecodeFunc decode) 
{
        char *temporary_buffer;
        int i;

        /* First update the database physically */
        temporary_buffer = g_new0 (char , file->field_info->record_size);

        /* Update the number of database fields in the database file */
        medusa_rdb_file_write_number_of_fields (file, file->field_info->number_of_fields + 1);
        medusa_io_handler_write_blank_data (file->io_handler, file->number_of_records * field_size + 
                                            strlen (field_name) + sizeof (int) + 1);
        for (i = file->number_of_records - 1; i > 0; i--) {
                memcpy (temporary_buffer, &file->contents[i * file->field_info->record_size], file->field_info->record_size);
                memcpy (&file->contents[i * (file->field_info->record_size + field_size)], temporary_buffer, file->field_info->record_size);
                memset (&file->contents[(i * (file->field_info->record_size + field_size)) + file->field_info->record_size], 0, field_size); 
        }


        medusa_rdb_field_add (file->field_info, field_name, field_size, encode, decode);


        g_free (temporary_buffer);
}

void
medusa_rdb_file_remove_field (MedusaRDBFile *file,
                              char *field_title) 
{
        int removed_offset, removed_size;
        int removed_field_offset,  removed_field_size;
        int i;

        removed_offset = medusa_rdb_field_get_offset (file->field_info, field_title);
        removed_size = medusa_rdb_field_get_size (file->field_info, field_title);
        removed_field_offset = medusa_rdb_field_get_header_offset (file->field_info, field_title);
        removed_field_size = medusa_rdb_field_get_header_size (file->field_info, field_title);
  
        for (i = 0; i < file->number_of_records; i++) {
                memcpy (&file->contents[i * (file->field_info->record_size - removed_size)], 
                        &file->contents[i * file->field_info->record_size],
                        removed_offset);
                memcpy (&file->contents[i * (file->field_info->record_size - removed_size) + removed_offset],
                        &file->contents[i * file->field_info->record_size + removed_offset], 
                        file->field_info->record_size - removed_offset - removed_size);
	    
        }

        /* Free the old space */
        memset (&file->contents[file->number_of_records * (file->field_info->record_size - removed_size)],
                0, removed_size * file->number_of_records);


        medusa_rdb_field_remove (file->field_info, field_title);
        file->header_length -= removed_field_size;
}


int
medusa_rdb_file_get_number_of_records (MedusaRDBFile *file)
{
        return file->number_of_records;
}


static int
medusa_rdb_file_write_initial_header (MedusaRDBFile *file)
{
        int zero;

        zero = 0;
        /* Write that Number of records is zero initially */
        medusa_io_handler_append_data (file->io_handler, &zero, sizeof (int));
        /* Write that number of fields is 0 for a new file */
        medusa_io_handler_append_data (file->io_handler, &zero, sizeof (int));


        return 2 * sizeof (int);
}

static void
medusa_rdb_file_write_number_of_records (MedusaRDBFile *file,
                                         int number_of_records)
{

        /* Write number of record information */
        printf ("Writing number of records: %d\n",file->number_of_records);
        memcpy (file->header, &number_of_records, sizeof (int));

}


static void
medusa_rdb_file_write_number_of_fields (MedusaRDBFile *file,
                                        int number_of_fields)
{

        /* Write number of record information */
        memcpy (&file->header[sizeof (int)], &number_of_fields, sizeof (int));

}



void 
medusa_rdb_file_test ()
{
        MedusaRDBFile *file;
        char data[200];
        /* First do tests on creation of a new file */
        /* Remove file if it is still sitting around */
        if (access (RDB_TEST_NEW_FILE, F_OK) != -1) {
                MEDUSA_TEST_SYSTEM_CALL (unlink (RDB_TEST_NEW_FILE));
        }
        MEDUSA_TEST_BOOLEAN_RESULT (access (RDB_TEST_NEW_FILE, F_OK) == -1);
        file = medusa_rdb_file_new (RDB_TEST_NEW_FILE, 100);
        MEDUSA_TEST_BOOLEAN_RESULT (medusa_rdb_file_is_empty (file));
        MEDUSA_TEST_BOOLEAN_RESULT (file->io_handler != NULL);
        MEDUSA_TEST_SYSTEM_CALL (access (RDB_TEST_NEW_FILE, R_OK | W_OK));
        /* File should have been read properly */
        MEDUSA_TEST_BOOLEAN_RESULT (read (file->io_handler->file_descriptor, data, 200) != -1);
        /* IO Header length = strlen (magic_number) + strlen(version) + strlen (buffer_space_available) + 3 tabs */
        MEDUSA_TEST_INTEGER_RESULT (file->io_handler->header_length, 15);
        /* File header length should be 2 * sizeof (int) -- number of fields, number of records */
        MEDUSA_TEST_INTEGER_RESULT (file->header_length, 2 * sizeof (int));
        MEDUSA_TEST_INTEGER_RESULT (file->number_of_records, 0);
        /* Contents should come right after the io_handler header and the file db header */
        MEDUSA_TEST_POINTER_VALUE (file->contents, 
                                   &file->io_handler->mapped_region[file->io_handler->header_length + file->header_length]);
        /* Version should be stored by both io_handler and file db object, since io handler writes it
           to disk */
        MEDUSA_TEST_STRING_RESULT (file->version , file->io_handler->version);
        /* file db header for a new file should be 0,0 */
        MEDUSA_TEST_INTEGER_RESULT (medusa_bytes_to_int (&file->contents[-4], 4), 0);
        MEDUSA_TEST_INTEGER_RESULT (medusa_bytes_to_int (&file->contents[-8], 4), 0);
        medusa_rdb_file_add_field (file, "number", 2, (MedusaRDBEncodeFunc) number_encode, 
                                   (MedusaRDBDecodeFunc) number_decode);
        MEDUSA_TEST_STRING_RESULT (&file->io_handler->mapped_region[file->io_handler->header_length + 2 * sizeof (int)], "number");
        medusa_rdb_file_add_field (file, "id", 4, (MedusaRDBEncodeFunc) id_encode, 
                                   (MedusaRDBDecodeFunc) id_decode);
        MEDUSA_TEST_STRING_RESULT (&file->io_handler->mapped_region[30], "id");
        medusa_rdb_file_remove_field (file, "number");
        MEDUSA_TEST_STRING_RESULT (&file->io_handler->mapped_region[23], "id");
        medusa_rdb_file_free (file);
        MEDUSA_TEST_SYSTEM_CALL (unlink (RDB_TEST_NEW_FILE));

        file = medusa_rdb_file_new (RDB_TEST_DATA_FILE, 100);
        MEDUSA_TEST_BOOLEAN_RESULT (file->io_handler != NULL);
        MEDUSA_TEST_SYSTEM_CALL (access (RDB_TEST_DATA_FILE, R_OK | W_OK));
        MEDUSA_TEST_BOOLEAN_RESULT (read (file->io_handler->file_descriptor, data, 200) != -1);
  
        MEDUSA_TEST_INTEGER_RESULT (file->io_handler->header_length, 15);
        MEDUSA_TEST_INTEGER_RESULT (file->header_length, 18);
        MEDUSA_TEST_INTEGER_RESULT (file->number_of_records, 2);
        MEDUSA_TEST_POINTER_VALUE (file->contents,
                                   &file->io_handler->mapped_region[file->io_handler->header_length + file->header_length]);
        MEDUSA_TEST_STRING_RESULT (file->version , file->io_handler->version);
        /* This requires loading of static field info to be fixed */
        /*  MEDUSA_TEST_INTEGER_RESULT (file->field_info->record_size, 6); */
        MEDUSA_TEST_INTEGER_RESULT (file->field_info->number_of_fields, 2);
        MEDUSA_TEST_NBYTE_STRING_RESULT (file->contents, "\0\0\01g", 5);
        MEDUSA_TEST_NBYTE_STRING_RESULT (&file->contents[5], "\0\0\125", 5);
        MEDUSA_TEST_INTEGER_RESULT  (file->contents[10], 0);
        /* Test the fields are in the right place */
        MEDUSA_TEST_NBYTE_STRING_RESULT (&file->contents[-10], "id", 3);
        /* Test the fields are in the right place, even after removing another one */
        /* medusa_rdb_file_remove_field (file, "number");
           MEDUSA_TEST_NBYTE_STRING_RESULT (&file->contents[-3], "id", 3); */
        medusa_rdb_file_free (file);


}
static void
id_encode (char *result, int id, gpointer data)
{
        strncpy (result, (char *) &id, 4);
}

static void 
id_decode (int *id, char *field, gpointer data)
{
        *id = * (int *) field; 
}

static void 
number_encode (char *result, char number, gpointer data)
{
        strncpy (result, &number, 1);
}

static void
number_decode (char *name, char *field, gpointer data)
{
        *name = *field;
}

  
 
	    
  


