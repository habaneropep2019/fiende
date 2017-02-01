/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  medusa-text-index-private.h : Utility functions for the text index
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

#ifndef MEDUSA_TEXT_INDEX_PRIVATE_H
#define MEDUSA_TEXT_INDEX_PRIVATE_H

#include <glib.h>
#include <stdio.h>

#include "medusa-hash.h"
#include "medusa-versioned-file.h"

#define NUMBER_OF_TEMP_INDEXES 16

struct MedusaTextIndex {
        /* The set of "semantic units" the text index knows about */
        MedusaHash *semantic_units;

        
        /* Temporary file information */
        GHashTable *last_occurrence;
        
        char *temp_index_name[NUMBER_OF_TEMP_INDEXES];

        gboolean temp_indices_are_memory_mapped;
        FILE *temp_index_stream[NUMBER_OF_TEMP_INDEXES];
        MedusaIOHandler *temp_index_io_handler[NUMBER_OF_TEMP_INDEXES];

        gint32 current_cell_number[NUMBER_OF_TEMP_INDEXES];

        /* Translates a semantic unit token
           into an offset (index?) in the
           location */
        char *start_index_name;
        MedusaVersionedFile *start_index;
        
        gint32 reverse_index_position;


        char *locations_index_name;
        MedusaVersionedFile *locations_index;

        /* Parsing tools for each mime type */
        GList *mime_modules;

        MedusaLogLevel log_level;

        /* Use this variable to remember whether we've filled
           out certain fields like the temporary index, and the
           in memory word hash that are only needed for index
           creation */
        gboolean creating_index;

        /* Reference count */
        int ref_count;
};


gint32     medusa_text_index_read_start_location_from_start_file  (MedusaTextIndex *text_index,
                                                                      MedusaToken word_token);
gint32     medusa_text_index_read_end_location_from_start_file    (MedusaTextIndex *text_index,
                                                                      MedusaToken word_token);



#endif /* MEDUSA_TEXT_INDEX_PRIVATE_H */
