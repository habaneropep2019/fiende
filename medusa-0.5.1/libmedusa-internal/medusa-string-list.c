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

#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "medusa-hash.h"
#include "medusa-io-handler.h"
#include <libmedusa/medusa-stdio-extensions.h>
#include "medusa-string-list.h"

#define STRING_LIST_MAGIC_NUMBER "912\124"
#define STRING_LIST_VERSION_NUMBER "0.1"

struct MedusaStringList {
        MedusaIOHandler *file;
        gboolean writing_list;

        MedusaStringListReadCacheType read_cache_type;
        char *in_memory_strings;
        
        MedusaStringListWriteCacheType write_cache_type;
        GHashTable *in_memory_string_locations;
        char *single_cached_element;
        MedusaToken single_cached_token;

        int current_offset;        
        int total_length;
};


static gboolean
string_list_file_without_header (const char *file_text,
                                 const char **beginning_of_list,
                                 int *header_length)
{
        int i;
        *beginning_of_list = file_text;

        /* FIXME: This actual hack should be an io/handler method,
           if we use it at all */
        for (i = 0; i < 3; i++) {
                *beginning_of_list = strchr (*beginning_of_list, '\t');
                if (*beginning_of_list == NULL) {
                        return FALSE;
                }
                (*beginning_of_list)++;
        }
        *header_length = *beginning_of_list - file_text;
        g_assert (*header_length >= 0);

        return TRUE;
}

MedusaStringList *
medusa_string_list_open (const char *file_name,
                         MedusaStringListReadCacheType cache_type)
{
        MedusaIOHandler *io_handler;
        MedusaStringList *string_list;
        const char *end_of_header;
        char *string_list_file;
        int string_list_file_size, header_length;
        gboolean string_list_file_contains_valid_header;

        string_list = g_new0 (MedusaStringList, 1);
        string_list->read_cache_type = cache_type;
        string_list->writing_list = FALSE;
        switch (cache_type) {
        case MEDUSA_STRING_LIST_NO_READ_CACHE:
                io_handler = medusa_io_handler_open (file_name,
                                                     STRING_LIST_MAGIC_NUMBER,
                                                     STRING_LIST_VERSION_NUMBER);
                if (io_handler == NULL) {
                        g_free (string_list);
                        return NULL;
                }
                string_list->file = io_handler;
                break;
        case MEDUSA_STRING_LIST_CACHE_KEY_TO_STRING:
                /* Read the whole file and create cache */
                string_list_file_size = medusa_read_whole_file (file_name,
                                                                &string_list_file);
                if (string_list_file_size == -1) {
                        g_free (string_list);
                        return NULL;
                }
                string_list_file_contains_valid_header = 
                        string_list_file_without_header (string_list_file,
                                                         &end_of_header,
                                                         &header_length);
                if (!string_list_file_contains_valid_header) {
                        g_free (string_list_file);
                        g_free (string_list);
                        return NULL;
                }
                string_list->in_memory_strings =
                        memmove (string_list_file,
                                 end_of_header, string_list_file_size - header_length);
                break;
        default:
                g_assert_not_reached ();
        }
        
        return string_list;
}

MedusaStringList *
medusa_string_list_new (const char *file_name,
                        MedusaStringListWriteCacheType cache_type)
{
        MedusaIOHandler *io_handler;
        MedusaStringList *string_list;

        string_list = g_new0 (MedusaStringList, 1);
        string_list->write_cache_type = cache_type;
        string_list->writing_list = TRUE; 
        switch (cache_type) {
        case MEDUSA_STRING_LIST_STRING_TO_KEY_WRITE_CACHE:
                /* Just create the cache for now, and not the on disk part */
                string_list->in_memory_string_locations = g_hash_table_new (g_str_hash,
                                                                            g_str_equal);
                /* Fall through */
        case MEDUSA_STRING_LIST_SINGLE_ELEMENT_WRITE_CACHE:
                /* Fall through */
        case MEDUSA_STRING_LIST_NO_WRITE_CACHE:
                io_handler = medusa_io_handler_new (file_name,
                                                    STRING_LIST_MAGIC_NUMBER,
                                                    STRING_LIST_VERSION_NUMBER, 1);
                if (io_handler == NULL) {
                        if (string_list->in_memory_string_locations) {
                                g_free (string_list->in_memory_string_locations);
                        }
                        g_free (string_list);
                        return NULL;
                }
                string_list->file = io_handler;
        
                break;
        default:
                g_assert_not_reached ();
        }

        string_list->current_offset = 1; 
        return string_list;
}


const char *
medusa_string_list_get_string (MedusaStringList *string_list,
                               MedusaToken key)
{
        if (string_list->writing_list) {
                return &string_list->file->mapped_region[key + string_list->file->header_length];
        }

        switch (string_list->read_cache_type) {
        case MEDUSA_STRING_LIST_CACHE_KEY_TO_STRING:
                return string_list->in_memory_strings + key;
        case MEDUSA_STRING_LIST_NO_READ_CACHE:
                return &string_list->file->mapped_region[key + string_list->file->header_length];
        default:
                g_assert_not_reached ();
                return NULL;
        }
}

static MedusaToken
store_string_on_disk (MedusaStringList *string_list,
                           const char *string)
{
        MedusaToken return_token;

        medusa_io_handler_append_string (string_list->file,
                                         string);
        return_token = (MedusaToken) string_list->current_offset;
        string_list->current_offset += strlen (string) + 1;
        
        return return_token;
}

MedusaToken 
medusa_string_list_store_string (MedusaStringList *string_list,
                                 const char *string)
{
        MedusaToken return_token;

        g_return_val_if_fail (string_list != NULL, 0);
        g_return_val_if_fail (string_list->writing_list, 0);
        g_return_val_if_fail (string != NULL, 0);
        
        switch (string_list->write_cache_type) {
        case MEDUSA_STRING_LIST_NO_WRITE_CACHE:
                return store_string_on_disk (string_list,
                                             string);
                break;
        case MEDUSA_STRING_LIST_STRING_TO_KEY_WRITE_CACHE:
                return_token =  GPOINTER_TO_UINT (g_hash_table_lookup (string_list->in_memory_string_locations,
                                                                       string));
                if (return_token) {
                        return return_token;
                }
                else {
                        return_token = store_string_on_disk (string_list, string);
                        g_hash_table_insert (string_list->in_memory_string_locations,
                                             g_strdup (string),
                                             GUINT_TO_POINTER (return_token));
                        return return_token;
                }
                break;
        case MEDUSA_STRING_LIST_SINGLE_ELEMENT_WRITE_CACHE:
                if (string_list->single_cached_element && 
                    strcmp (string_list->single_cached_element, string) == 0) {
                        return string_list->single_cached_token;
                }
                else {
                        if (string_list->single_cached_element) {
                                g_free (string_list->single_cached_element);
                        }
                        string_list->single_cached_element = g_strdup (string);
                        string_list->single_cached_token = store_string_on_disk (string_list, string);
                        return string_list->single_cached_token;
                }
                break;
        default:
                g_assert_not_reached ();
        }
        
        return 0;
}

static void
free_cache_entry (gpointer key,
                  gpointer value,
                  gpointer data)
{
        g_assert (key != NULL);
        g_free (key);
}

void
medusa_string_list_destroy (MedusaStringList *string_list)
{
        g_return_if_fail (string_list != NULL);
        g_free (string_list->in_memory_strings);
        if (string_list->in_memory_string_locations) {
                g_hash_table_foreach (string_list->in_memory_string_locations,
                                      free_cache_entry, NULL);
                g_hash_table_destroy (string_list->in_memory_string_locations);
        }
        g_free (string_list->single_cached_element);
        medusa_io_handler_free (string_list->file);
        g_free (string_list);
}

void                 
medusa_convert_bytes_to_token (MedusaToken *token,
                               const char bytes[4])
{
        g_assert (sizeof (MedusaToken) == 4);
        memcpy (token, bytes, sizeof (MedusaToken));
}
                                                        
void                 
medusa_convert_token_to_bytes (char bytes[4],
                               MedusaToken token)
{
        memcpy (bytes, &token, sizeof (MedusaToken));
}
