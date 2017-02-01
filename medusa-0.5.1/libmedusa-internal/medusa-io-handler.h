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

/* medusa-io-handler.h:  API for the medusa mmaped structure
   handling object, the io-handler */

#ifndef MEDUSA_IO_HANDLER_H
#define MEDUSA_IO_HANDLER_H


/* The indexes in Medusa are accessed using mmap.
   This allows the most flexibility in terms of speed,
   and ability to use different amounts of memory, on
   different machines.  However, it has some bad points.
   One is that adding data to files is hard.  
   This structure is meant give a basic io api to the
   indexes so that these low level details are obscured */

/* One point should be taken when using this data structure:
   It is important to remember that after appending data to
   a file, the pointer to the data in the file may change.
   This happens during a remap.  */


/* io-handler is independent of the data stored in the file.
   It creates a file with the following format:
   header
   data
   blank space

   where header is composed of three tab separated numbers
   magic number, version number of the data, and the amount of
   buffer space available, in bytes */
   

#define MEDUSA_HEADER_SIZE_MAX        200

/* This is the amount of blank space that is added every time a new buffer
   is created.  When this space is used up, it is necessary to remap the
   whole file.  */
#define MEDUSA_DEFAULT_BUFFER_SIZE    (10 * getpagesize ())

#include <glib.h>
#include <stdio.h>

typedef struct MedusaIOHandler MedusaIOHandler;

struct MedusaIOHandler {
        /* kernel fd for the file */
        int file_descriptor;
        /* File size */
        int file_size;
        /* Area accessible to the io handler */
        char *mapped_region;
        /* Valid area size in mapped_region */
        int mapped_size;
        /* Point where the already written data ends, and new data can be appended */
        char *insertion_point;
        /* Blank space left to be written to in the file */
        int buffer_space_available;

        /* Version for the data in the file */
        char *version;
        /* Magic number for the data in the file */
        char *magic_number;
        /* Length of the file header */
        int header_length;
};

/* Use open for files that already exist, and new to create new files */
MedusaIOHandler *     medusa_io_handler_open                       (const char *file_name,
								    const char *magic_number,
								    const char *version_number);

MedusaIOHandler *     medusa_io_handler_new                        (const char *file_name,
                                                                    const char *magic_number,
								    const char *version_number,
								    int initial_file_size);
MedusaIOHandler *     medusa_io_handler_open                       (const char *file_name,
                                                                    const char *magic_number,
								    const char *version_number);
void                  medusa_io_handler_free                       (MedusaIOHandler *handler);

/* Adds data in gpointer onto the end of the file */
/* Returns new mapped region pointer, in order to deal with remaps */
char *                medusa_io_handler_append_data                (MedusaIOHandler *handler, 
								    gpointer data, 
								    int data_size);
/* FIXME bugzilla.eazel.com 2697:  
   These functions don't notify the user if 
   the file has been remapped. */

/* Adds the string given onto the end of the file.  Appends a 0 to the
   end, so that consecutive strings are separated */
void                  medusa_io_handler_append_string             (MedusaIOHandler *handler, 
								   const char *data);
/* Adds zeros to the file */
void                  medusa_io_handler_append_zeros              (MedusaIOHandler *handler, 
								   int bytes);

/* Returns a pointer to the beginning of the data you appended.
   Does no error checking for things like acceptable bounds */
gpointer               medusa_io_handler_get_data_region          (MedusaIOHandler *handler);


/* Returns a pointer to a new file, but puts in a header that the io handler
   can use later.  If the file already exists, returns NULL */
FILE *                 fopen_new_with_medusa_io_handler_header    (const char *file_name,
								   const char *magic_number,
								   const char *version_number);

/* FIXME bugzilla.eazel.com 2698: 
   This function should be deprecated */
void                  medusa_io_handler_write_blank_data          (MedusaIOHandler *handler,
								   int bytes_to_add);

/* FIXME bugzilla.eazel.com 2699: 
   This function should not be public */
/* Remaps the handler to have more space available */
void                  medusa_io_handler_remap                     (MedusaIOHandler *handler);

#endif /* MEDUSA_IO_HANDLER_H */

