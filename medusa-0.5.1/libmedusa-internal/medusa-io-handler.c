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

/* medusa-io-handler.c:  Routines for handling creation and appending to
   data files using mmap */

#include "medusa-io-handler.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <glib.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static void           medusa_io_handler_create_file        (MedusaIOHandler *handler,
							    const char *file_name,
							    const char *magic_number,
							    const char *version_number);
static gboolean       medusa_io_handler_read_file_header   (MedusaIOHandler *handler);
static void           medusa_io_handler_write_header       (MedusaIOHandler *handler,
							    const char *magic_number,
							    const char *version_number,
							    int buffer_space_available);							   
static void           medusa_io_handler_adjust_buffer_size (MedusaIOHandler *handler,
							    int bytes_to_add);
static int            write_header_to_file_descriptor      (int file_descriptor,
							    const char *magic_number,
							    const char *version_number,
							    int buffer_space_available);
static void           medusa_adjust_filesize_to_pagesize   (MedusaIOHandler *handler);



static char zero[1] = "0";

MedusaIOHandler *
medusa_io_handler_new (const char *file_name,
                       const char *magic_number,
                       const char *version_number,
                       int initial_file_size)

{
        MedusaIOHandler *handler;

        /* If the file already exists, let's not overwrite it*/
        g_return_val_if_fail (access (file_name, F_OK) == -1, NULL);

        handler = g_new0 (MedusaIOHandler, 1);
        handler->magic_number = g_strdup (magic_number);
        handler->version = g_strdup (version_number);

        medusa_io_handler_create_file (handler, file_name, magic_number, version_number);
        g_return_val_if_fail (handler->file_descriptor != -1, NULL);
        if (initial_file_size > 0) {
                medusa_io_handler_write_blank_data (handler, initial_file_size);
        }

        medusa_adjust_filesize_to_pagesize (handler);

        handler->mapped_region = mmap (NULL, 
                                       handler->file_size,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED, 
                                       handler->file_descriptor, 
                                       0);
        handler->mapped_size = handler->file_size;
        g_return_val_if_fail (handler->mapped_region != GINT_TO_POINTER (-1), NULL);
        g_assert (handler->file_size - handler->buffer_space_available >= 0);
        handler->insertion_point = &handler->mapped_region[handler->file_size - 
                                                          handler->buffer_space_available];
        return handler;

}

MedusaIOHandler*      
medusa_io_handler_open (const char *file_name, 
                        const char *magic_number, 
                        const char *version_number)
{
        MedusaIOHandler *handler;
        int file_descriptor;
        struct stat file_info;

        g_return_val_if_fail (file_name != NULL, NULL);

        file_descriptor = open (file_name, O_RDWR);        
        if (file_descriptor == -1) {
                /* FIXME bugzilla.eazel.com 4557: Log this event */
                return NULL;
        }

        handler = g_new0 (MedusaIOHandler, 1);

        handler->file_descriptor = file_descriptor;
        handler->magic_number = g_strdup (magic_number);
        handler->version = g_strdup (version_number);

        stat (file_name, &file_info);
        handler->file_size = file_info.st_size;
        medusa_io_handler_read_file_header (handler);
        
        /* We shouldn't need to do this */
        /* medusa_adjust_filesize_to_pagesize (handler); */

        handler->mapped_region = mmap (NULL, 
                                       handler->file_size,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED, 
                                       handler->file_descriptor, 
                                       0);
        handler->mapped_size = handler->file_size;
        g_return_val_if_fail (handler->mapped_region != (gpointer) -1, NULL);
        g_assert (handler->file_size - handler->buffer_space_available >= 0);
        handler->insertion_point = &handler->mapped_region[handler->file_size - 
                                                          handler->buffer_space_available];
        return handler;
}

void                  
medusa_io_handler_free (MedusaIOHandler *handler)
{
        g_return_if_fail (handler != NULL);

        medusa_io_handler_write_header (handler,
                                        handler->magic_number,
                                        handler->version,
                                        handler->buffer_space_available);
        if  (munmap (handler->mapped_region, handler->mapped_size) == -1) {
                /* FIXME bugzilla.eazel.com 4557: Log error */
        }
        if (close (handler->file_descriptor) == -1) {
                /* FIXME bugzilla.eazel.com 4557: log error */
        }
        g_free (handler->magic_number);
        g_free (handler->version);
        g_free (handler);

}


static void                  
medusa_io_handler_write_header  (MedusaIOHandler *handler,
				 const char *magic_number,
				 const char *version_number,
				 int buffer_space_available)
{
        int header_length;

        header_length = write_header_to_file_descriptor (handler->file_descriptor, magic_number, version_number,
                                                         buffer_space_available);

        if (handler->file_size == 0) {
                handler->file_size = header_length;
        }

        handler->header_length = header_length;
}

static int 
write_header_to_file_descriptor (int file_descriptor,
				 const char *magic_number,
				 const char *version_number,
				 int buffer_space_available)
{
        char header[MEDUSA_HEADER_SIZE_MAX];
        int written_length;

        sprintf (header, "%s\t%s\t%5d\t", magic_number, version_number, buffer_space_available);
        lseek (file_descriptor, 0, SEEK_SET);
        written_length = write (file_descriptor, header, strlen (header));
        g_return_val_if_fail (written_length == strlen (header), written_length);
  
        return written_length;
}

static void                  
medusa_io_handler_adjust_buffer_size (MedusaIOHandler *handler,
				      int bytes_to_add)
{
        medusa_io_handler_write_blank_data (handler, bytes_to_add);
        handler->buffer_space_available += bytes_to_add;

}


void
medusa_io_handler_write_blank_data (MedusaIOHandler *handler,
				    int bytes_to_add) 
{
        int lseek_result, write_result;
        
        
        lseek_result = lseek (handler->file_descriptor, 
                              bytes_to_add,
                              SEEK_END);
        g_return_if_fail (lseek_result != -1);

        write_result = write (handler->file_descriptor, zero, 1);
        g_return_if_fail (write_result != -1);

        handler->file_size += bytes_to_add; 
      
}
				    

gpointer
medusa_io_handler_get_data_region (MedusaIOHandler *handler)
{
        return (char *) handler->mapped_region + handler->header_length;
}

FILE *
fopen_new_with_medusa_io_handler_header (const char *file_name,
					 const char *magic_number,
					 const char *version_number)
{
        int file_descriptor;
        FILE *fp;
        struct stat file_info;

        /* FIXME bugzilla.eazel.com 2696: 
           Perhaps we should do a check here,
           and if the file exists and is a medusa io
           handler file return a file pointer pointing
           to the beginning/end of the data region? */
        g_return_val_if_fail (access (file_name, F_OK) == -1, NULL);

        /* We open the file synchronously here to make sure the header gets
           written before returning the FILE * */
        file_descriptor = open (file_name, O_RDWR | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR);
        write_header_to_file_descriptor (file_descriptor, magic_number, version_number, 0);
        close (file_descriptor);				  
        stat (file_name, &file_info);
        fp = fopen (file_name, "r+");
        /* Go to the end, where we will be appending data */

        fseek (fp, 0, SEEK_END);
        return fp;
}

static void           
medusa_io_handler_create_file (MedusaIOHandler *handler,
			       const char *file_name,
			       const char *magic_number,
			       const char *version_number)
{
        handler->file_descriptor = open (file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        g_return_if_fail (handler->file_descriptor != -1);

        handler->buffer_space_available = 0;
  
        medusa_io_handler_write_header (handler, magic_number, version_number, 
                                        MEDUSA_DEFAULT_BUFFER_SIZE);
        medusa_io_handler_adjust_buffer_size (handler, MEDUSA_DEFAULT_BUFFER_SIZE);



}

static gboolean
medusa_io_handler_read_file_header  (MedusaIOHandler *handler)

{
        char *header;
        char **header_fields;

        header = g_malloc (MEDUSA_HEADER_SIZE_MAX * sizeof (char));

        read (handler->file_descriptor, header, MIN (handler->file_size,
                                                     MEDUSA_HEADER_SIZE_MAX));
        header_fields = g_strsplit (header, "\t", 3);

        /* Make sure magic number is correct */
        if (strcmp (header_fields[0], handler->magic_number)) {
                return FALSE;
        }
        /* Record version number.  Version number in handler is the
           version used for creating new files.  Existing files
           may have a different version number */
        handler->version = g_strdup (header_fields[1]);

        handler->buffer_space_available = strtol (header_fields[2], NULL, 10);
        if (handler->buffer_space_available < 0 || 
            handler->buffer_space_available == G_MININT ||
            handler->buffer_space_available == G_MAXINT) {
                return FALSE;
        }
  
        handler->header_length = strlen (header_fields[0]) + strlen (header_fields[1]) +
                strlen (header_fields[2]) + 3;
        g_strfreev (header_fields);
        g_free (header);
        return TRUE;
}
							   

char *
medusa_io_handler_append_data (MedusaIOHandler *handler, gpointer data, int data_size) 
{
        if (data_size < handler->buffer_space_available) {
                memcpy (handler->insertion_point, data, data_size);
                handler->buffer_space_available -= data_size;
                handler->insertion_point += data_size;
        }
        else {
                /* Need to remap to get more space */
                medusa_io_handler_remap (handler);
                medusa_io_handler_append_data (handler, data, data_size);
        }
        return handler->mapped_region;
}

void
medusa_io_handler_append_zeros (MedusaIOHandler *handler, int data_size) 
{
        if (data_size < handler->buffer_space_available) {
                memset (handler->insertion_point, 0, data_size);
                handler->buffer_space_available -= data_size;
                handler->insertion_point += data_size;
        }
        else {
                /* Need to remap to get more space */
                medusa_io_handler_remap (handler);
                medusa_io_handler_append_zeros (handler, data_size);
        }
}


void
medusa_io_handler_append_string (MedusaIOHandler *handler, const char *data)
{
        if (strlen (data) < handler->buffer_space_available) {
                strncpy (handler->insertion_point, data, strlen (data));
                handler->buffer_space_available -= strlen(data);
                handler->mapped_region[handler->file_size - handler->buffer_space_available]=0;
                handler->buffer_space_available--;
                handler->insertion_point += strlen (data) + 1;
        }
        else {
                /* Need to remap to get more space */
                medusa_io_handler_remap (handler);
                medusa_io_handler_append_string (handler, data);
        }
}

void           
medusa_io_handler_remap (MedusaIOHandler *handler)
{

        int used_addresses;
        used_addresses = handler->insertion_point - handler->mapped_region;

        g_return_if_fail (munmap (handler->mapped_region, handler->mapped_size) != -1);
#ifdef IO_HANDLER_DOUBLE_ON_REMAP
        g_print ("remapping and add %d bytes\n", used_addresses);
        medusa_io_handler_adjust_buffer_size (handler, used_addresses);
#else
        medusa_io_handler_adjust_buffer_size (handler, MEDUSA_DEFAULT_BUFFER_SIZE);
#endif
        medusa_adjust_filesize_to_pagesize (handler);
        /* Remap the larger file */
        handler->mapped_region = mmap (NULL, 
                                       handler->file_size,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED, 
                                       handler->file_descriptor, 
                                       0);
        handler->mapped_size = handler->file_size;
        g_return_if_fail (handler->mapped_region != (gpointer) -1);
        handler->insertion_point = &handler->mapped_region[used_addresses];
}

static void           
medusa_adjust_filesize_to_pagesize  (MedusaIOHandler *handler)
{
        /* Only need to add buffer space if it is not an integer multiple */
        if (handler->file_size % getpagesize ()) {
                medusa_io_handler_adjust_buffer_size (handler,
                                                      getpagesize () - (handler->file_size % getpagesize ()) - 1);
 
        }
}
