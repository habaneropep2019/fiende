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
 *  medusa-text-index-queries.c  Builds and performs queries on a medusa
 *  text index
 *
 */

#include <glib.h>
#include <string.h>

#include <libmedusa/medusa-utils.h>

#include "medusa-query-clauses.h"
#include "medusa-text-index.h"
#include "medusa-text-index-private.h"
#include "medusa-text-index-queries.h"
#include "medusa-versioned-file.h"

gint32 *           word_to_uri_numbers  (MedusaTextIndex *index,
                                         char *word,
                                         int *number_of_results);



gint32 *
medusa_text_index_get_uris_for_criterion (MedusaTextIndex *text_index,
                                          char *content_request,
                                          gboolean match_all_words,
                                          int *number_of_results)
{
        char **content_words;
        gint32 *current_results, *new_results, *merged_results;
        int number_of_new_results, number_of_merged_results;
        int i;
        

        content_words = g_strsplit (content_request, " ", 0);
        g_return_val_if_fail (content_words != NULL && content_words[0] != NULL, NULL);
        current_results = word_to_uri_numbers (text_index,
                                               content_words[0],
                                               number_of_results);
        for (i = 1; content_words[i] != NULL; i++) {
                new_results = word_to_uri_numbers (text_index,
                                                   content_words[i],
                                                   &number_of_new_results);
                if (match_all_words) {
                        merged_results = medusa_intersect_two_descending_integer_lists (current_results,
                                                                                        *number_of_results,
                                                                                         new_results,
                                                                                         number_of_new_results,
                                                                                         &number_of_merged_results);
                }
                else {
                        merged_results = medusa_union_of_two_descending_integer_lists (current_results,
                                                                                       *number_of_results,
                                                                                        new_results,
                                                                                        number_of_new_results,
                                                                                        &number_of_merged_results);
                }
                if (current_results != NULL) {
                        g_free (current_results);
                }
                g_free (new_results);
                current_results = merged_results;
                *number_of_results = number_of_merged_results;
                                                                         
        }
        
        g_strfreev (content_words);
        return current_results;
}
                                                   




gint32 *           
word_to_uri_numbers  (MedusaTextIndex *text_index,
                      char *word,
                      int *number_of_results)
{
        MedusaToken word_token;
        int begin_cell, end_cell;
        gint32 *result_array;

        /* If the word is not in the text index, just return nothing: */
        if (!medusa_string_has_token (text_index->semantic_units,
                                       word)) {
                *number_of_results = 0;
                return NULL;
        }

        /* Translate the word to a word number */
        word_token = medusa_string_to_token (text_index->semantic_units,
                                             word);
        /* Next look up the address of where locations for the word are stored,
         using the word number */
        begin_cell = medusa_text_index_read_start_location_from_start_file (text_index,
                                                                            word_token);
        if (begin_cell == -1) {
                g_warning ("Error reading start location. Aborting text index lookup\n");
                return NULL;
        }
        printf ("Begin location for word %s is %d\n", word, begin_cell);
        end_cell = medusa_text_index_read_end_location_from_start_file (text_index,
                                                                        word_token);
        if (end_cell == -1) {
                g_warning ("Error reading end location. Aborting text index lookup\n");
                return NULL;
        }
        printf ("End location for word %s is %d\n", word, end_cell);
        g_assert (end_cell > begin_cell);

        result_array = g_new0 (gint32, end_cell - begin_cell);
        /* FIXME bugzilla.eazel.com 4562: this should be a method */
        medusa_versioned_file_seek (text_index->locations_index,
                                    begin_cell * sizeof (gint32));
        medusa_versioned_file_read (text_index->locations_index,
                                    result_array,
                                    sizeof (gint32),
                                    (end_cell - begin_cell));
        *number_of_results = end_cell - begin_cell;

        return result_array;
}


 
