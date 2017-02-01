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
 *
 *  medusa-queue-test.c: Test files in the text indexer directory 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <unistd.h>

#include <medusa-versioned-file.h>

#define TEST_FILE_NAME "/tmp/version-file-test"
#define MAGIC_NUMBER "1010"
#define VERSION_NUMBER "9.8.7"

MedusaVersionedFile *file;
MedusaVersionedFileResult result;
const char *test_data = "abcdefg";

static void
cleanup_old_test_files (void)
{
        unlink (TEST_FILE_NAME);
}

static void 
test_creating_bad_file_name (void)
{
        file = medusa_versioned_file_create ("/tmp/bad_file/very_bad_file/so_bad",
                                           MAGIC_NUMBER,
                                           VERSION_NUMBER,
                                           &result);
        g_assert (result == MEDUSA_VERSIONED_FILE_ERROR_CANNOT_OPEN);

}

static void
test_creating_good_file (void)
{
        file = medusa_versioned_file_create (TEST_FILE_NAME,
                                           MAGIC_NUMBER,
                                           VERSION_NUMBER,
                                           &result);
        g_assert (result == MEDUSA_VERSIONED_FILE_OK);

}

static void
test_magic_and_version_numbers_were_written_right (void)
{
        g_assert (!strcmp (MAGIC_NUMBER, medusa_versioned_file_get_magic_number (file)));
        g_assert (!strcmp (VERSION_NUMBER, medusa_versioned_file_get_version_number (file)));

}

static void
test_writing_data (void)
{
        result = medusa_versioned_file_write (file, test_data, sizeof (char), strlen (test_data));

        g_assert (result == MEDUSA_VERSIONED_FILE_OK);
}

static void
test_reading_back_data (void)
{
        char test_buffer[100];
        memset (test_buffer, 0, 100);

        g_assert (medusa_versioned_file_seek (file, 0) 
                  == MEDUSA_VERSIONED_FILE_OK);
        

        g_assert (medusa_versioned_file_read (file, test_buffer, sizeof (char), strlen (test_data))
                  == MEDUSA_VERSIONED_FILE_OK);
        g_assert (!strcmp (test_buffer, test_data)); 
}

static void
test_closing_file (void)
{
        g_assert (medusa_versioned_file_destroy (file) == MEDUSA_VERSIONED_FILE_OK);
}

static void
test_reopening_file_with_wrong_magic_and_version_numbers (void)
{
        file = medusa_versioned_file_open (TEST_FILE_NAME,
                                           MAGIC_NUMBER,
                                           "nope!",
                                           &result);
        g_assert (result == MEDUSA_VERSIONED_FILE_ERROR_WRONG_VERSION);
        file = medusa_versioned_file_open (TEST_FILE_NAME,
                                           "nope!",
                                           VERSION_NUMBER,
                                           &result);
        g_assert (result == MEDUSA_VERSIONED_FILE_ERROR_BAD_MAGIC_NUMBER);
}

static void
test_reopening_good_file (void)
{

        file = medusa_versioned_file_open (TEST_FILE_NAME,
                                           MAGIC_NUMBER,
                                           VERSION_NUMBER,
                                           &result);
        g_assert (result == MEDUSA_VERSIONED_FILE_OK);
}


int main ()
{
        cleanup_old_test_files ();
        test_creating_bad_file_name ();
        test_creating_good_file ();
        test_magic_and_version_numbers_were_written_right ();
        test_writing_data ();
        test_reading_back_data ();
        test_closing_file ();
 
        test_reopening_file_with_wrong_magic_and_version_numbers ();
        test_reopening_good_file ();
        test_magic_and_version_numbers_were_written_right ();
        test_reading_back_data ();
        test_closing_file ();

        printf ("All versioned file tests passed\n");
        return EXIT_SUCCESS;
}



