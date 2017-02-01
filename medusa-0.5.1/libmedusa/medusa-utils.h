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
 *  medusa-utils.h -  Utility functions used across medusa
 */

#ifndef MEDUSA_UTILS_H
#define MEDUSA_UTILS_H

#include <libgnomevfs/gnome-vfs-types.h>

/* This constant is needed to index metadata. */
#define MEDUSA_PUBLIC_METAFILE_NAME ".nautilus-metafile.xml"
#define MEDUSA_PRIVATE_METAFILE_DIRECTORY_PATH "/.nautilus/metafiles/"
#define MEDUSA_PRIVATE_METAFILE_SUFFIX ".xml"

typedef gboolean (* MedusaGPredicateFunc) 	(gpointer	data,
						 gpointer	user_data);

typedef void     (* MedusaExtractKeywordsCallback) (const char *file_name,
                                                    GList *keywords,
                                                    gpointer callback_data);


char *         medusa_full_path_from_directory_and_file_name     (const char                     *directory_name,
                                                                  const char                     *file_name);
char *         medusa_full_uri_from_directory_uri_and_file_name  (const char                     *directory_uri,
                                                                  const char                     *file_name);


GList *        medusa_g_list_partition                           (GList                          *list,
                                                                  MedusaGPredicateFunc            predicate,
                                                                  gpointer                        user_data,
                                                                  GList                         **failed);
GList *        medusa_g_list_remove_deep_custom                  (GList                          *list,
                                                                  MedusaGPredicateFunc           removable,
                                                                  GDestroyNotify                 destroy_data,
                                                                  gpointer                       user_data);

/* Returns true if every item in the list returns true for the predicate function */
gboolean       medusa_g_list_forall                              (GList                          *list,
                                                                  MedusaGPredicateFunc           predicate,
                                                                  gpointer                       user_data);
/* Takes the intersection of two integer lists, both sorted in descending order.
   returns the merged list and 
   stores the number of entries in the merged_list in number_of_results */
gint32 *       medusa_intersect_two_descending_integer_lists     (gint32                         *first_list,
                                                                  int                             first_list_entry_count,
                                                                  gint32                         *second_list,
                                                                  int                             second_list_entry_count,
                                                                  int                            *number_of_results);

/* Takes the union of two integer lists,
 same information flow as with intersect */
gint32 *       medusa_union_of_two_descending_integer_lists      (gint32                         *first_list,
                                                                  int                             first_list_entry_count,
                                                                  gint32                         *second_list,
                                                                  int                             second_list_entry_count,
                                                                  int                            *number_of_results);

/* Takes the difference of two integer lists, both sorted in descending order.
   (ie the set of element in the first list, but not in the second ),
   same information flow as  with intersect */
gint32 *       medusa_difference_of_two_descending_integer_lists (gint32                         *first_list,
                                                                  int                             first_list_entry_count,
                                                                  gint32                         *second_list,
                                                                  int                             second_list_entry_count,
                                                                  int                            *number_of_results);
void           medusa_g_list_free_deep_custom                    (GList                          *list,
                                                                  GFunc                           element_free_func,
                                                                  gpointer                        user_data);
void           medusa_g_list_free_deep                           (GList                          *list);

void           medusa_str_to_str_list_hash_table_destroy         (GHashTable                     *hash_table);

/* Read the keywords from the metafile. For the moment this is the only I/O we do on metafiles. */
GnomeVFSResult medusa_extract_keywords_from_metafile             (const char                     *metafile_path,
                                                                  MedusaExtractKeywordsCallback   callback,
                                                                  gpointer                        callback_data);

/* This call can't be used for indexing because it depends on the current user's home directory.
 * For indexing, we must index the keywords as seen by each user.
 */
GnomeVFSResult medusa_keyword_hash_table_create_for_directory    (GHashTable                    **keywords_hash_table,
                                                                  const char                     *directory_uri);
gboolean       medusa_keyword_hash_table_file_has_keyword        (GHashTable                     *keywords_hash_table,
                                                                  const char                     *file_name,
                                                                  const char                     *keyword);
void           medusa_keyword_hash_table_destroy                 (GHashTable                     *keywords_hash_table);


gint64         medusa_get_system_time                            (void);
#endif /* MEDUSA_UTILS_H */
