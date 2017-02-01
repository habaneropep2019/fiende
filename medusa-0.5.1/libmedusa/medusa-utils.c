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
 *  medusa-utils.c -  Utility functions used across medusa
 */

#include "medusa-utils.h"

#include <grp.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <parser.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <xml-error.h>




static gboolean             str_ends_with_slash              (const char *directory_name);


char *
medusa_full_path_from_directory_and_file_name (const char *directory_name,
                                               const char *file_name)
{
        g_return_val_if_fail (directory_name != NULL, NULL);
        g_return_val_if_fail (file_name != NULL, NULL);

        /* This could probably be less horrid with canonical uri's */
        if (str_ends_with_slash (directory_name)) {
                return g_strdup_printf ("%s%s", directory_name, file_name);
        } else {
                return g_strdup_printf ("%s/%s", directory_name, file_name);
        }
}

char *
medusa_full_uri_from_directory_uri_and_file_name (const char *directory_uri,
                                                  const char *file_name)
{
        char *file_name_uri_encoded, *uri;

        g_return_val_if_fail (directory_uri != NULL, NULL);
        g_return_val_if_fail (file_name != NULL, NULL);

        file_name_uri_encoded = gnome_vfs_escape_string (file_name);

        if (str_ends_with_slash (directory_uri)) {
                uri = g_strdup_printf ("%s%s", directory_uri, file_name_uri_encoded);
        } else {
                uri = g_strdup_printf ("%s/%s", directory_uri, file_name_uri_encoded);
        }

        g_free (file_name_uri_encoded);
        return uri;
}

static gboolean
str_ends_with_slash (const char *string)
{
        int length;

        g_return_val_if_fail (string != NULL, FALSE);
        
        length = strlen (string);
        return length != 0 && string[length - 1] == '/';

}


/**
 * medusa_g_list_partition
 * 
 * Parition a list into two parts depending on whether the data
 * elements satisfy a provided predicate. Order is preserved in both
 * of the resulting lists, and the original list is consumed. A list
 * of the items that satisfy the predicate is returned, and the list
 * of items not satisfying the predicate is returned via the failed
 * out argument.
 * 
 * @list: List to partition.
 * @predicate: Function to call on each element.
 * @user_data: Data to pass to function.  
 * @removed: The GList * variable pointed to by this argument will be
 * set to the list of elements for which the predicate returned
 * false. 
 * Note: this is written by Maciej Stachowiak <mjs@eazel.com> and
 * clipped from nautilus/libnautilus-extensions/nautilus-glib-extensions.c */

GList *
medusa_g_list_partition (GList	          *list,
                         MedusaGPredicateFunc  predicate,
                         gpointer	           user_data,
                         GList                 **failed)
{
	GList *predicate_true;
	GList *predicate_false;
	GList *reverse;
	GList *p;
	GList *next;

	predicate_true = NULL;
	predicate_false = NULL;

	reverse = g_list_reverse (list);

	for (p = reverse; p != NULL; p = next) {
		next = p->next;
		
		if (next != NULL) {
			next->prev = NULL;
		}

		if (predicate (p->data, user_data)) {
			p->next = predicate_true;
 			if (predicate_true != NULL) {
				predicate_true->prev = p;
			}
			predicate_true = p;
		} else {
			p->next = predicate_false;
 			if (predicate_false != NULL) {
				predicate_false->prev = p;
			}
			predicate_false = p;
		}
	}

	*failed = predicate_false;
	return predicate_true;
}

GList *
medusa_g_list_remove_deep_custom (GList *list,
                                  MedusaGPredicateFunc removable,
                                  GDestroyNotify destroy_data,
                                  gpointer user_data)
{
        GList *new_list, *node;

        new_list = NULL;
        for (node = list; node != NULL; node = node->next) {
                if (!removable (node->data, user_data)) {
                        new_list = g_list_append (new_list, node->data);
                }
                else {
                        destroy_data (node->data);
                }
        }

        g_list_free (list);

        return new_list;
}

gboolean
medusa_g_list_forall (GList *list,
                      MedusaGPredicateFunc predicate,
                      gpointer user_data)
{
        GList *p;

        for (p = list; p != NULL; p = p->next) {
                if (!predicate (p->data, user_data)) {
                        return FALSE;
                }
        }
        
        return TRUE;
}

gint32 *             
medusa_intersect_two_descending_integer_lists (gint32 *first_list,
                                               int first_list_entry_count,
                                               gint32 *second_list,
                                               int second_list_entry_count,
                                               int *number_of_results)
{
        int first_list_position, second_list_position;
        gint32 *results;
        
        /* If there are no entries in one of the lists, we are done */
        if (first_list_entry_count == 0 || 
            second_list_entry_count == 0) {
                *number_of_results = 0;
                return NULL;
        }
        /* Since their entry counts are non-zero, these can't be NULL */
        g_assert (first_list != NULL && second_list != NULL);

        results = g_new0 (gint32, MIN (first_list_entry_count, second_list_entry_count));
        first_list_position = 0;
        second_list_position = 0;
        *number_of_results = 0;
        while (first_list_position < first_list_entry_count &&
               second_list_position < second_list_entry_count) {
                if (first_list[first_list_position] > second_list[second_list_position]) {
                        first_list_position++;
                        continue;
                }
                if (second_list[second_list_position] > first_list[first_list_position]) {
                        second_list_position++;
                        continue;
                }
                results[*number_of_results] = first_list[first_list_position++];
                (*number_of_results)++;
        }
        return results;
}


gint32 *             
medusa_union_of_two_descending_integer_lists (gint32 *first_list,
                                              int first_list_entry_count,
                                              gint32 *second_list,
                                              int second_list_entry_count,
                                              int *number_of_results)
{
        int first_list_position, second_list_position;
        gint32 *results;
        
        /* If there are no entries in one of the lists, we are done */
        if (first_list_entry_count == 0) {
                *number_of_results = second_list_entry_count;
                results = g_new0 (int, second_list_entry_count);
                memcpy (results, second_list, sizeof (gint32) * second_list_entry_count);
                return results;
        }
        if (second_list_entry_count == 0) {
                *number_of_results = first_list_entry_count;
                results = g_new0 (int, first_list_entry_count);
                memcpy (results, first_list, sizeof (gint32) * first_list_entry_count);
                return results;
        }
        /* Since their entry counts are non-zero, these can't be NULL */
        g_assert (first_list != NULL && second_list != NULL);

        results = g_new0 (gint32, first_list_entry_count + second_list_entry_count);
        first_list_position = 0;
        second_list_position = 0;
        *number_of_results = 0;
        while (first_list_position < first_list_entry_count &&
               second_list_position < second_list_entry_count) {
                while (first_list_position < first_list_entry_count &&
                       first_list[first_list_position] >= second_list[second_list_position]) {
                        results[(*number_of_results)++] = first_list[first_list_position++];
                }
                while (second_list_position < second_list_entry_count &&
                       first_list[first_list_position] < second_list[second_list_position]) {
                        results[(*number_of_results)++] = second_list[second_list_position++];
                }
        }
        while (first_list_position < first_list_entry_count) {
                results[(*number_of_results)++] = first_list[first_list_position++];
        }
        while (second_list_position < second_list_entry_count) {
                results[(*number_of_results)++] = second_list[second_list_position++];
        }
        return results;
}


gint32 *              
medusa_difference_of_two_descending_integer_lists (gint32 *first_list,
                                                   int first_list_entry_count,
                                                   gint32 *second_list,
                                                   int second_list_entry_count,
                                                   int *number_of_results)
{
        int first_list_position, second_list_position;
        gint32 *results;
        

        /* If there are no entries the first list, we are done */
        if (first_list_entry_count == 0) {
                *number_of_results = 0;
                return NULL;
        }
        /* Since their entry counts are non-zero, these can't be NULL */
        g_assert (first_list != NULL && second_list != NULL);
        
        results = g_new0 (gint32, first_list_entry_count);
        first_list_position = 0;
        second_list_position = 0;
        *number_of_results = 0;
        while (first_list_position < first_list_entry_count &&
               second_list_position < second_list_entry_count) {
                if (first_list[first_list_position] > second_list[second_list_position]) {
                        results[*number_of_results] = first_list[first_list_position];
                        (*number_of_results)++;
                        first_list_position++;
                        continue;
                }
                if (second_list[second_list_position] > first_list[first_list_position]) {
                        second_list_position++;
                        continue;
                }
                else {
                        first_list_position++;
                        second_list_position++;
                }
        }
        /* If there are still more entries left in the first list,
           they should all be included */
        while (first_list_position < first_list_entry_count) {
                results[*number_of_results] = first_list_position++;
                (*number_of_results)++;
        }
        return results;
}


/**
 * medusa_g_list_free_deep_custom
 *
 * Frees the elements of a list and then the list, using a custom free function.
 *
 * @list: List of elements that can be freed with the provided free function.
 * @element_free_func: function to call with the data pointer and user_data to free it.
 * @user_data: User data to pass to element_free_func
 **/
void
medusa_g_list_free_deep_custom (GList *list, GFunc element_free_func, gpointer user_data)
{
	g_list_foreach (list, element_free_func, user_data);
	g_list_free (list);
}

/**
 * medusa_g_list_free_deep
 *
 * Frees the elements of a list and then the list.
 * @list: List of elements that can be freed with g_free.
 **/
void
medusa_g_list_free_deep (GList *list)
{
	medusa_g_list_free_deep_custom (list, (GFunc) g_free, NULL);
}

/**
 * medusa_extract_keywords_from_metafile
 *
 * Call the callback function once for each file that has attached
 * keywords.
 **/

typedef struct {
        xmlSAXHandler handlers;
        MedusaExtractKeywordsCallback callback;
        gpointer callback_data;
        char *file_name;
        GList *keywords;
} ExtractKeywordsState;

static const xmlChar *
get_attribute (const xmlChar **attributes,
               const xmlChar *attribute_name)
{
        while (attributes[0] != NULL) {
                if (strcmp (attribute_name, attributes[0]) == 0) {
                        return attributes[1];
                }
                attributes += 2;
        }
        return NULL;
}

static const xmlChar *
get_name_attribute (const xmlChar **attributes)
{
        return get_attribute (attributes, "NAME");
}

static void
start_element (gpointer callback_data,
               const xmlChar *name,
               const xmlChar **attributes)
{
        ExtractKeywordsState *state;
        const xmlChar *keyword;

        state = callback_data;

        if (strcmp (name, "FILE") == 0) {
                /* The file name should be null, unless we have a bad
                 * XML file. But since that can happen, lets avoid the
                 * storage leak in that case.
                 */
                g_free (state->file_name);
                state->file_name = g_strdup (get_name_attribute (attributes));
        } else if (state->file_name != NULL
                   && strcmp (name, "KEYWORD") == 0) {
                keyword = get_name_attribute (attributes);
                state->keywords = g_list_prepend
                        (state->keywords, g_strdup (keyword));
        }
}

static void
end_element (gpointer callback_data,
             const xmlChar *name)
{
        ExtractKeywordsState *state;

        state = callback_data;

        if (state->file_name != NULL && strcmp (name, "FILE") == 0) {
                (* state->callback) (state->file_name,
                                     state->keywords,
                                     state->callback_data);
                medusa_g_list_free_deep (state->keywords); 
                state->keywords = NULL;
                g_free (state->file_name);
                state->file_name = NULL;
        }
}

static GnomeVFSResult
get_gnome_vfs_result_from_xml_error (int xml_error)
{
        switch (xml_error) {
        case XML_ERR_OK:
                return GNOME_VFS_OK;
        default:
                return GNOME_VFS_ERROR_GENERIC;
        }
}

GnomeVFSResult
medusa_extract_keywords_from_metafile (const char *metafile_path,
                                       MedusaExtractKeywordsCallback callback,
                                       gpointer callback_data)
{
        ExtractKeywordsState state;
        int xml_error;

        if (access (metafile_path, R_OK) != 0) {
                return gnome_vfs_result_from_errno ();
        }

        memset (&state, 0, sizeof (state));
        state.handlers.startElement = start_element;
        state.handlers.endElement = end_element;
        state.callback = callback;
        state.callback_data = callback_data;
        xml_error = xmlSAXUserParseFile
                (&state.handlers, &state, metafile_path);
        medusa_g_list_free_deep (state.keywords);
        g_free (state.file_name);
        return get_gnome_vfs_result_from_xml_error (xml_error);
}

/* Copy a list that has strings in it. We use str to mean C-style
 * string, as usual, to avoid confusing with GString, etc.
 */
static GList *
str_list_dup (GList *str_list)
{
        GList *p, *duplicate;

        duplicate = NULL;
        for (p = str_list; p != NULL; p = p->next) {
                duplicate = g_list_prepend (duplicate, g_strdup (p->data));
        }
        return g_list_reverse (duplicate);
}

/* Free a single entry. Used for the destroy function below. */
static void
free_str_to_str_list_hash_table_entry (gpointer key, gpointer value, gpointer callback_data)
{
        g_assert (callback_data == NULL);

        /* Free the str, then the str_list. */
        g_free (key);
        medusa_g_list_free_deep (value);
}

/* Destroy a hash table with strings as keys, and GLists of strings as
 * values. As usual, we use str to mean C string as opposed to
 * GString.
 */
void
medusa_str_to_str_list_hash_table_destroy (GHashTable *hash_table)
{
        g_return_if_fail (hash_table != NULL);

        /* Free the hash table entries, then the hash table. */
        g_hash_table_foreach (hash_table,
                              free_str_to_str_list_hash_table_entry,
                              NULL);
        g_hash_table_destroy (hash_table);
}

/* Fill in the hash table with a set of keywords. Passed to the
 * extract function as a callback.
 */
static void
keyword_hash_table_populate_callback (const char *file_name,
                                      GList *keywords,
                                      gpointer callback_data)
{
        GHashTable *hash_table;
        gpointer old_key, old_value;

        hash_table = callback_data;
        /* Check for an existing hash table entry. */
        if (g_hash_table_lookup_extended (hash_table, file_name,
                                          &old_key, &old_value)) {
                /* We make the last mention of a file win. In a
                 * well-formed metafile this won't ever happen, but we
                 * want to avoid runtime errors from bad files.
                 */
                g_hash_table_remove (hash_table, file_name);
                g_free (old_key);
                medusa_g_list_free_deep (old_value);
        }
        g_hash_table_insert (hash_table,
                             g_strdup (file_name),
                             str_list_dup (keywords));
}

/* Shared function so we don't repeat so much code for the public
 * and private metafile case.
 */
static GnomeVFSResult
keyword_hash_table_populate_from_metafile (GHashTable *keywords_hash_table,
                                           const char *metafile_path)
{
        return medusa_extract_keywords_from_metafile
                (metafile_path,
                 keyword_hash_table_populate_callback,
                 keywords_hash_table);
}

/* The logic needed to construct a private metafile's path name. */
static char *
construct_private_metafile_path (const char *directory_uri)
{
        char *uri_with_slashes_escaped, *path;

        uri_with_slashes_escaped = gnome_vfs_escape_slashes (directory_uri);
        path = g_strconcat (g_get_home_dir (),
                            MEDUSA_PRIVATE_METAFILE_DIRECTORY_PATH,
                            uri_with_slashes_escaped,
                            MEDUSA_PRIVATE_METAFILE_SUFFIX,
                            NULL);
        g_free (uri_with_slashes_escaped);
        return path;
}

/* Extract the keywords for a particular directory. This uses the same
 * rules about private and public metafiles that Nautilus does.
 */
GnomeVFSResult
medusa_keyword_hash_table_create_for_directory (GHashTable **keywords_hash_table,
                                                const char *directory_uri)
{
        char *directory_path, *private_metafile_path, *public_metafile_path;
        GnomeVFSResult result;
        GHashTable *hash_table;
        
        g_return_val_if_fail (keywords_hash_table != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
        g_return_val_if_fail (directory_uri != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
                
        /* Locate the directory. */
        directory_path = gnome_vfs_get_local_path_from_uri (directory_uri);
        if (directory_path == NULL) {
                *keywords_hash_table = NULL;
                return GNOME_VFS_ERROR_NOT_SUPPORTED;
        }
        /* Create the hash table */
        hash_table = g_hash_table_new (g_str_hash, g_str_equal);

        /* Look for a private metafile. */
        private_metafile_path = construct_private_metafile_path (directory_uri);
        result = keyword_hash_table_populate_from_metafile
                (hash_table, private_metafile_path);
        g_free (private_metafile_path);
        if (result == GNOME_VFS_ERROR_NOT_FOUND) {
                /* Look for a public metafile. */
                public_metafile_path = medusa_full_path_from_directory_and_file_name
                        (directory_path, MEDUSA_PUBLIC_METAFILE_NAME);
                result = keyword_hash_table_populate_from_metafile
                        (hash_table, public_metafile_path);
                g_free (public_metafile_path);
        }
        g_free (directory_path);
        /* Return or discard the hash table. */
        if (result == GNOME_VFS_OK) {
                *keywords_hash_table = hash_table;
        } else {
                medusa_keyword_hash_table_destroy (hash_table);
        }
        return result;
}

/* Check the hash table for a particular keyword. This is provided so
 * that clients don't have to know the details of how the hash table
 * is set up.
 */
gboolean
medusa_keyword_hash_table_file_has_keyword (GHashTable *keywords_hash_table,
                                            const char *file_name,
                                            const char *keyword)
{
        GList *list;

        g_return_val_if_fail (keywords_hash_table != NULL, FALSE);
        g_return_val_if_fail (file_name != NULL, FALSE);
        g_return_val_if_fail (keyword != NULL, FALSE);

        /* Get the keyword list from this file's hash table entry. */
        list = g_hash_table_lookup (keywords_hash_table, file_name);
        return g_list_find_custom (list, (char *) keyword,
                                   (GCompareFunc) strcmp) != NULL;
}

/* Destroy a keyword hash table. */
void
medusa_keyword_hash_table_destroy (GHashTable *keywords_hash_table)
{
        /* It's just a str-to-str-list hash table. */
        medusa_str_to_str_list_hash_table_destroy (keywords_hash_table);
}

/**
 * medusa_get_system_time
 * 
 * Return value: number of microseconds since the machine was turned on
 */
gint64
medusa_get_system_time (void)
{
	struct timeval tmp;

	gettimeofday (&tmp, NULL);
	return (gint64)tmp.tv_usec + (gint64)tmp.tv_sec * G_GINT64_CONSTANT (1000000);
}
