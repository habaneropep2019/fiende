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
 *  medusa-stdio-extensions.c - Functions to do standard local I/O
 *  tasks.
 */
#include <glib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "medusa-stdio-extensions.h"

/* Reads a whole file into a new buffer.
   Returns the number of bytes read */
int
medusa_read_whole_file (const char *file_name,
                        char **file_contents)
{
        FILE *stream;
        struct stat stat_info;
        int stat_return_code, bytes_read;
        char *buffer;
        
        stat_return_code = stat (file_name, &stat_info);
        
        if (stat_return_code == -1) {
                *file_contents = NULL;
                return -1;
        }
        
        stream = fopen (file_name, "r");
        if (stream == NULL) {
                *file_contents = NULL;
                return -1;
        }

        buffer = g_malloc (stat_info.st_size + 1);
        
        bytes_read = fread (buffer, 1, stat_info.st_size, stream);
        buffer[bytes_read] = 0;

        fclose (stream);

        if (bytes_read != stat_info.st_size) {
                g_free (buffer);
                *file_contents = NULL;
                return -1;
        }

        *file_contents = buffer;
        return bytes_read;
}

