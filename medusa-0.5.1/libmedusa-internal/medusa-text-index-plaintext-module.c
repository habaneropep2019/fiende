/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  medusa-text-index-plaintext-module.c :  procedure
 *  to parse plain text files
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

#include <ctype.h>
#include <glib.h>
#include <string.h>
#include <stdio.h>

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-ops.h>

#include "medusa-text-index-plaintext-module.h"

#define PLAINTEXT_MODULE_READ_SIZE 8192

/* FIXME bugzilla.eazel.com 4560:  Change these function arguments to be const char's where appropriate */
static char *    read_more_data                                 (GHashTable *word_hash,
                                                                 GnomeVFSHandle *uri_handle,
                                                                 int *words_read,
                                                                 char *last_word_fragment,
                                                                 gboolean *more_data_to_read);
static void      transfer_hash_table_to_word_array_and_free     (GHashTable *word_hash,
                                                                 char ***results);
static char *    enlarge_word_fragment                          (char *last_word_fragment, 
                                                                 char *data);
static char *    handle_word_fragment                           (char *last_word_fragment, 
                                                                 char *data, 
                                                                 GHashTable *word_hash, 
                                                                 int *words_read);
static char *    go_to_first_word_in_block                      (char *data) ;
static gboolean  block_is_one_word                              (char *block, 
                                                                 int size_of_block);
static gboolean  word_is_last_word_in_block                     (char *word, 
                                                                 char *end_of_block);
static gboolean  word_contains_digits                           (const char *word);
static char *    go_to_next_word_in_block                       (char *word);
static void      add_key_to_word_array                          (gpointer key,
                                                                 gpointer value,
                                                                 gpointer user_data);

/* We read data here in blocks,
   to avoid memory piggage. */
int medusa_text_index_parse_plaintext (char *uri,
				       char ***results,
				       gpointer data)
{

  GHashTable *word_hash;
  GnomeVFSHandle *uri_handle;
  GnomeVFSResult result;
  char *last_word_fragment;
  gboolean more_data_to_read;
  int words_read;
#ifdef TEXT_INDEX_DEBUG
  printf ("Parsing %s with the plain text content indexer\n", uri);
#endif
  result = gnome_vfs_open (&uri_handle,
                           uri,
                           GNOME_VFS_OPEN_READ);
  /* FIXME bugzilla.eazel.com 2993: 
     Need log level check here.  For now return nothing */
  if (result != GNOME_VFS_OK) {
          return 0;
  }
  words_read = 1;
  last_word_fragment = NULL;
  more_data_to_read = TRUE;
  word_hash = g_hash_table_new (g_str_hash,
                                g_str_equal);
  while (more_data_to_read) {
          last_word_fragment = read_more_data (word_hash,
                                               uri_handle,
                                               &words_read,
                                               last_word_fragment,
                                               &more_data_to_read);

  }
  if (last_word_fragment != NULL) {
          if (g_hash_table_lookup (word_hash, last_word_fragment) == NULL) {
#ifdef TEXT_INDEX_DEBUG
                  printf ("Actually inserting a last word!! with %s\n", last_word_fragment);
#endif
                  g_hash_table_insert (word_hash, last_word_fragment, GINT_TO_POINTER(words_read));
                  words_read++;
          }
  }
  transfer_hash_table_to_word_array_and_free (word_hash, results);
  /* No need to free keys, since they were put into the results array,
     and the values are integers, so they can't be freed. */
  /* So we just free the hash table */
  g_hash_table_destroy (word_hash);
  if (gnome_vfs_close (uri_handle) != GNOME_VFS_OK) {
          g_warning ("File close of uri %s failed\n", uri);
  }
  return words_read - 1;

}


static char *
read_more_data (GHashTable *word_hash,
                GnomeVFSHandle *uri_handle,
                int *words_read,
                char *last_word_fragment,
                gboolean *more_data_to_read)
{
        char data[PLAINTEXT_MODULE_READ_SIZE];
        char *location, *current_word;
        GnomeVFSFileSize bytes_read;
        GnomeVFSResult read_result;
        
        read_result = gnome_vfs_read (uri_handle,
                                      data,
                                      PLAINTEXT_MODULE_READ_SIZE,
                                      &bytes_read);
        /* FIXME bugzilla.eazel.com 2993: 
           need log check behavior here, and this
           isn't the right behaviour.  We need to do
           something with the last word fragment */
        if (read_result != GNOME_VFS_OK) {
                *more_data_to_read = FALSE;
                return NULL;
        }
        if (bytes_read == PLAINTEXT_MODULE_READ_SIZE) {
                *more_data_to_read = TRUE;
        }
        else {
                *more_data_to_read = FALSE;
        }
        
        /* Change all of the non-interesting characters to
           nulls to make parsing easier */
        for (location = data; location < &data[bytes_read]; location++) {
                if (!isalnum ((int)*location) && (*location != '_')) {
                        *location = 0;
                }
                else if (isalpha ((int)*location)) {
                        *location = tolower (*location);
                }
        }

        /* last_word_fragment is the end of the last block.
           The first word of this block is the combination of 
           that fragment and the rest of the word, which is
           the first part of the data in this block */
        if (block_is_one_word (data, bytes_read)) {
                return enlarge_word_fragment (last_word_fragment, data);
        }
        current_word = handle_word_fragment (last_word_fragment, data, word_hash, words_read);



        while (!word_is_last_word_in_block (current_word, &data[bytes_read])) {
                if (word_contains_digits (current_word)) {
                        current_word = go_to_next_word_in_block (current_word);
                        continue;
                }
                /* Uggh!  This should be optimized.
                   #$%#% g_hash code */
                if (!g_hash_table_lookup (word_hash, current_word)) {
#ifdef TEXT_INDEX_DEBUG
                        printf ("Inserting word %s into hash with addres %d\n", current_word, *words_read);
#endif
                        g_assert (strlen (current_word) > 0);
                        g_hash_table_insert (word_hash, g_strdup (current_word), 
                                             GINT_TO_POINTER(*words_read));
                        (*words_read)++;
                }
                else {
#ifdef TEXT_INDEX_DEBUG
                        printf ("Word %s is a duplicate; skipping \n", current_word);
#endif
                }
                current_word = go_to_next_word_in_block (current_word);
        }
        if (current_word < &data[bytes_read]) {
                return g_strndup (current_word, &data[bytes_read] - current_word);
        }
        else {
                return NULL;
        }
}

  
static char *
enlarge_word_fragment (char *last_word_fragment, char *data)
{
        char *result;

        /* Return a large word fragment, since we (still)
           haven't finished the word by reading this block */
        if (last_word_fragment == NULL) {
                return g_strdup (data);
        }
        else {
                result = g_strdup_printf ("%s%s", last_word_fragment, data);
                g_free (last_word_fragment);
                return result;
        }
}

static void
transfer_hash_table_to_word_array_and_free (GHashTable *hash_table,
                                            char ***results)
{
        *results = g_new0 (char *, g_hash_table_size (hash_table));

        g_hash_table_foreach (hash_table, add_key_to_word_array, results);

}
                         

static void
add_key_to_word_array (gpointer key,
                       gpointer value,
                       gpointer user_data)
{
        char *word;
        int position;
        char **word_array;
        
        word = (char *) key;
        position = GPOINTER_TO_INT (value);
        word_array = * (char ***) user_data;
        word_array[position - 1] = word;
}
                       
static char *
handle_word_fragment (char *last_word_fragment, char *data, GHashTable *word_hash, int *words_read)
{
        char *first_word;
        if (last_word_fragment == NULL) {
                return go_to_first_word_in_block (data);
        }
        else {
                first_word = g_strdup_printf ("%s%s", last_word_fragment, data);
                if (!g_hash_table_lookup (word_hash, first_word)) {
#ifdef TEXT_INDEX_DEBUG
                        printf ("Inserting word %s into hash with address %d\n", first_word, *words_read);
#endif
                        g_assert (strlen (first_word) > 0);
                        g_hash_table_insert (word_hash, first_word, GINT_TO_POINTER (*words_read));
                        (*words_read)++;
                }
                else {
                        g_free (first_word);
                }
                g_free (last_word_fragment);
                /* Necessary if *data == 0 */
                go_to_first_word_in_block (data);
                return go_to_next_word_in_block (data);
        }

        return NULL;
}

static char *
go_to_first_word_in_block (char *data) 
{
        while (*data == 0) {
                data++;
        }
        return data;
}

static gboolean
block_is_one_word (char *block, int size_of_block)
{
        return strlen (block) >= size_of_block;
}

     
static gboolean
word_is_last_word_in_block (char *word, char *end_of_block)
{
        return word + strlen (word) >= end_of_block;
}

static gboolean  
word_contains_digits (const char *word)
{
        while (*word != 0) {
                if (isdigit ((int)*word++)) {
                        return TRUE;
                }
        }
        return FALSE;
}

/* Word is not last word in block, so this approach is safe */
static char *
go_to_next_word_in_block (char *word)
{
        word = word + strlen (word);
        while (*word == 0) {
                word++;
        }

        return word;
}
