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
   binary files that keep version and magic numbers */

#ifndef MEDUSA_VERSIONED_FILE_H
#define MEDUSA_VERSIONED_FILE_H

#include <glib.h>

/* FIXME bugzilla.eazel.com 4564:  It would be better to have errors determined
   by problems rather than when they occurred.
   We'd need a routine to convert errno into a 
   versioned file result for this */
typedef enum {
  MEDUSA_VERSIONED_FILE_OK,
  MEDUSA_VERSIONED_FILE_ERROR_WRONG_VERSION,
  MEDUSA_VERSIONED_FILE_ERROR_BAD_PARAMETERS,
  MEDUSA_VERSIONED_FILE_ERROR_READING,
  MEDUSA_VERSIONED_FILE_ERROR_SEEKING,
  MEDUSA_VERSIONED_FILE_ERROR_CANNOT_OPEN,
  MEDUSA_VERSIONED_FILE_ERROR_WRITING,
  MEDUSA_VERSIONED_FILE_ERROR_NOT_FOUND,
  MEDUSA_VERSIONED_FILE_ERROR_CLOSING,
  MEDUSA_VERSIONED_FILE_ERROR_FILE_EXISTS,
  MEDUSA_VERSIONED_FILE_ERROR_BAD_MAGIC_NUMBER
} MedusaVersionedFileResult;

typedef struct MedusaVersionedFile MedusaVersionedFile;

MedusaVersionedFile *             medusa_versioned_file_create              (const char *file_name,
									     const char *magic_number,
									     const char *version_number,
									     MedusaVersionedFileResult *result);
MedusaVersionedFile *             medusa_versioned_file_open                (const char *file_name,
									     const char *magic_number,
									     const char *version_number,
									     MedusaVersionedFileResult *result);
char *                             medusa_versioned_file_get_magic_number   (MedusaVersionedFile *file);
char *                             medusa_versioned_file_get_version_number (MedusaVersionedFile *file);
MedusaVersionedFileResult          medusa_versioned_file_seek               (MedusaVersionedFile *file,
									    int offset);
MedusaVersionedFileResult          medusa_versioned_file_write              (MedusaVersionedFile *file,
									     gconstpointer data,
									     int item_size,
									     int number_of_items);
MedusaVersionedFileResult          medusa_versioned_file_read               (MedusaVersionedFile *file,
									     gpointer buffer,
									     int item_size,
									     int number_of_items);
MedusaVersionedFileResult          medusa_versioned_file_append_zeros       (MedusaVersionedFile *file,
									     int size);
void                               medusa_versioned_file_error_notify       (char *parent_string,
                                                                             MedusaVersionedFileResult result);
char *                             medusa_versioned_file_result_to_string   (MedusaVersionedFileResult result);
MedusaVersionedFileResult          medusa_versioned_file_destroy            (MedusaVersionedFile *file);


#endif /* MEDUSA_VERSIONED_FILE_H */
