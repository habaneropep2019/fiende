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

/* medusa-versioned-file.c: Routines for handling creating and dealing with
   binary files that keep version and
   magic numbers */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>

#include <libmedusa/medusa-string.h>
#include "medusa-versioned-file.h"

struct MedusaVersionedFile {
        FILE *fp;
        char *magic_number;
        char *version_number;
        int header_length;
};

static char *         make_file_header             (const char *magic_number,
                                                    const char *version_number);
static char *         read_file_header             (FILE *fp);
static gboolean       ensure_header_version_number (const char *header,
                                                    const char *version_number);
static gboolean       ensure_header_magic_number   (const char *header,
                                                    const char *magic_number);

#define INITIAL_HEADER_SIZE_GUESS 20

MedusaVersionedFile *   
medusa_versioned_file_create (const char *file_name,
			      const char *magic_number,
			      const char *version_number,
                              MedusaVersionedFileResult *result)
{
        MedusaVersionedFile *versioned_file;
        char *header;

        if (magic_number == NULL ||
            version_number == NULL) {
                *result = MEDUSA_VERSIONED_FILE_ERROR_BAD_PARAMETERS;
                return NULL;
        }
        if (access (file_name, F_OK) != -1) {
                *result = MEDUSA_VERSIONED_FILE_ERROR_FILE_EXISTS;
                return NULL;
        }

        versioned_file = g_new0 (MedusaVersionedFile, 1);
        
        versioned_file->fp = fopen (file_name, "w+");
        chmod (file_name, S_IRUSR | S_IWUSR);
        if (versioned_file->fp == NULL) {
                g_free (versioned_file);
                *result = MEDUSA_VERSIONED_FILE_ERROR_CANNOT_OPEN;
                return NULL;
        }
        
        
        header = make_file_header (magic_number, version_number);
        versioned_file->header_length = strlen (header);

        if (fwrite (header, strlen (header), sizeof (char), versioned_file->fp) == -1) {
                g_free (header);
                fclose (versioned_file->fp);
                g_free (versioned_file);
                *result = MEDUSA_VERSIONED_FILE_ERROR_WRITING;
                return NULL;
        }

        g_free (header);
        versioned_file->magic_number = g_strdup (magic_number);
        versioned_file->version_number = g_strdup (version_number);
        
        *result = MEDUSA_VERSIONED_FILE_OK;
        medusa_versioned_file_seek (versioned_file, 0);
        return versioned_file;
}


MedusaVersionedFile *   
medusa_versioned_file_open (const char *file_name,
                            const char *magic_number,
                            const char *version_number,
                            MedusaVersionedFileResult *result)
{
        MedusaVersionedFile *versioned_file;
        char *header;
        
        if (magic_number == NULL ||
            version_number == NULL) {
                *result = MEDUSA_VERSIONED_FILE_ERROR_BAD_PARAMETERS;
                return NULL;
        }
        if (access (file_name, F_OK) == -1) {
                *result = MEDUSA_VERSIONED_FILE_ERROR_NOT_FOUND;
                return NULL;
        }

        versioned_file = g_new0 (MedusaVersionedFile, 1);
        
        versioned_file->fp = fopen (file_name, "r+");
        if (versioned_file->fp == NULL) {
                g_warning ("Result opening versioned file %s\n", file_name);
                g_free (versioned_file);
                *result = MEDUSA_VERSIONED_FILE_ERROR_CANNOT_OPEN;
                return NULL;
        }
        
        
        header = read_file_header (versioned_file->fp);
        versioned_file->header_length = strlen (header);

        if (!ensure_header_magic_number (header, magic_number)) {
                g_free (header);
                fclose (versioned_file->fp);
                g_free (versioned_file);
                *result = MEDUSA_VERSIONED_FILE_ERROR_BAD_MAGIC_NUMBER;
                return NULL;
        }
        if (!ensure_header_version_number (header, version_number)) {
                g_free (header);
                fclose (versioned_file->fp);
                g_free (versioned_file);
                *result = MEDUSA_VERSIONED_FILE_ERROR_WRONG_VERSION;
                return NULL;
        }

        g_free (header);
        versioned_file->magic_number = g_strdup (magic_number);
        versioned_file->version_number = g_strdup (version_number);
        
        *result = MEDUSA_VERSIONED_FILE_OK;
        medusa_versioned_file_seek (versioned_file, 0);
        return versioned_file;


}
char *                             
medusa_versioned_file_get_magic_number   (MedusaVersionedFile *file)
{
        return file->magic_number;
}

char *                             
medusa_versioned_file_get_version_number (MedusaVersionedFile *file)
{
        return file->version_number;
}

MedusaVersionedFileResult
medusa_versioned_file_seek (MedusaVersionedFile *file,
                            int offset)
{
        int seek_result;

        seek_result = fseek (file->fp, offset + file->header_length, SEEK_SET);
        if (seek_result == 0) {
                return MEDUSA_VERSIONED_FILE_OK;
        }
        else {
                return MEDUSA_VERSIONED_FILE_ERROR_SEEKING;
        }
        
}


MedusaVersionedFileResult          
medusa_versioned_file_write (MedusaVersionedFile *file,
                             gconstpointer data,
                             int item_size,
                             int number_of_items)
{
        int write_result;

        write_result = fwrite (data, item_size, number_of_items, file->fp);
        if (write_result == number_of_items) {
                return MEDUSA_VERSIONED_FILE_OK;
        }
        else {
                return MEDUSA_VERSIONED_FILE_ERROR_WRITING;
        }
}

MedusaVersionedFileResult         
medusa_versioned_file_read (MedusaVersionedFile *file,
                            gpointer buffer,
                            int item_size,
                            int number_of_items)
{
        int read_result;
        read_result = fread (buffer, item_size, number_of_items, file->fp);
        if (read_result == number_of_items) {
                return MEDUSA_VERSIONED_FILE_OK;
        }
        else {
                return MEDUSA_VERSIONED_FILE_ERROR_READING;
        }

}

MedusaVersionedFileResult
medusa_versioned_file_append_zeros (MedusaVersionedFile *file,
                                    int size)
{
        int fseek_result, fwrite_result;
        unsigned char buffer[8192];
        const unsigned char zero = 0;

        memset (buffer, 0, 8192);
        fseek_result = fseek (file->fp, size - 1, SEEK_END);
        if (fseek_result == -1) {
                return MEDUSA_VERSIONED_FILE_ERROR_SEEKING;
        }
        fwrite_result = fwrite (&zero, 1, 1, file->fp);
        if (fwrite_result != 1) {
                return MEDUSA_VERSIONED_FILE_ERROR_WRITING;
        }
        return MEDUSA_VERSIONED_FILE_OK;
}

void
medusa_versioned_file_error_notify (char *parent_string,
                                    MedusaVersionedFileResult result)
{
        char *error_message;

        error_message = medusa_versioned_file_result_to_string (result);
        g_warning ("%s: %s\n", parent_string, error_message);
        g_free (error_message);
}


char *                             
medusa_versioned_file_result_to_string (MedusaVersionedFileResult result)
{
        switch (result) {
        case MEDUSA_VERSIONED_FILE_OK:
                return g_strdup ("Everything was ok");
        case MEDUSA_VERSIONED_FILE_ERROR_WRONG_VERSION:
                return g_strdup ("File you want to open has the wrong version");
        case MEDUSA_VERSIONED_FILE_ERROR_BAD_PARAMETERS:
                return g_strdup ("Versioned File function received bad paramters");
        case MEDUSA_VERSIONED_FILE_ERROR_READING:
                return g_strdup ("Error occurred while reading file");
        case MEDUSA_VERSIONED_FILE_ERROR_SEEKING:
                return g_strdup ("Error occurred while seeking in the file");
        case MEDUSA_VERSIONED_FILE_ERROR_CANNOT_OPEN:
                return g_strdup ("Unable to open the file");
        case MEDUSA_VERSIONED_FILE_ERROR_NOT_FOUND:
                return g_strdup ("File not found");
        case MEDUSA_VERSIONED_FILE_ERROR_CLOSING:
                return g_strdup ("Could not close file");
        case MEDUSA_VERSIONED_FILE_ERROR_FILE_EXISTS:
                return g_strdup ("File is not new; it already exists");
        case MEDUSA_VERSIONED_FILE_ERROR_BAD_MAGIC_NUMBER:
                return g_strdup ("File has bad magic number");
        case MEDUSA_VERSIONED_FILE_ERROR_WRITING:
                return g_strdup ("Error occurred while writing to a file");
        }
        g_assert_not_reached ();
        return NULL;
}

MedusaVersionedFileResult
medusa_versioned_file_destroy (MedusaVersionedFile *file)
{
        int close_result;
        
        g_free (file->magic_number);
        g_free (file->version_number);
        close_result = fclose (file->fp);
        g_free (file);
        if (close_result == 0) {
                return MEDUSA_VERSIONED_FILE_OK;
        }
        else {
                return MEDUSA_VERSIONED_FILE_ERROR_CLOSING;
        }
        
}
								  

static char *         
make_file_header (const char *magic_number,
                  const char *version_number)
{
        return g_strdup_printf ("%s\t%s\n", magic_number, version_number);
}

static char *         
read_file_header (FILE *fp)
{
        char *buffer;
        char *result;
        int header_size;
 
        header_size = INITIAL_HEADER_SIZE_GUESS;
        buffer = g_new0 (char, header_size);
        fread (buffer, sizeof (char), header_size, fp);
        while (strchr (buffer, '\t') == NULL ||
               strchr (buffer, '\n') == NULL ||
               strchr (buffer, '\t') >= strchr (buffer, '\n')) {
                header_size *=2;
                buffer = g_new0 (char, header_size);
                fseek (fp, 0, SEEK_SET);
                fread (buffer, sizeof (char), header_size, fp);
        }
        result = g_strndup (buffer, strchr (buffer, '\n') - buffer + 1);
        g_free (buffer);

        return result;
}

static gboolean
ensure_header_version_number (const char *header, 
                              const char *version_number)
{
        char *expected_suffix;
        gboolean result;

        expected_suffix = g_strdup_printf ("%s\n",
                                           version_number);
        result = medusa_str_has_suffix (header,
                                        expected_suffix);

        g_free (expected_suffix);
        return result;
}

static gboolean
ensure_header_magic_number (const char *header,
                            const char *magic_number)
{
        return medusa_str_has_prefix (header,
                                      magic_number);
}

