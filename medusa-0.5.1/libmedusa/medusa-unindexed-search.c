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
 *  Authors: Rebecca Schulman <rebecka@eazel.com>
 *  
 */


/* medusa-unindexed-search.h -- Service to run a find that backs up an
 indexed search */

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <glib.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-cancellation.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-context.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "medusa-file-info-utilities.h"
#include "medusa-string.h"
#include "medusa-service-private.h"
#include "medusa-unindexed-search.h"
#include "medusa-unsearched-locations.h"
#include "medusa-utils.h"


typedef int MedusaEmblemData;

#define UNINDEXED_RESULTS_FILE "/tmp/unindexed-results-"
#define END_OF_UNINDEXED_SEARCH_RESULTS "--End--\n"

struct MedusaUnindexedSearch {
        /* FIXME bugzilla.eazel.com 2988:  Perhaps we only need the
           partitioned criteria? */
        /* Parsed list of criteria */
        GList *criteria;
        /* Criteria that are easy to match */
        GList *name_criteria;
        /* Criteria that require stat info */
        GList *inode_criteria;
        /* Criteria that require mime type */
        GList *mime_type_criteria;
        /* Criteria that require Nautilus keywords information */
        GList *keywords_criteria;

        /* Root directory for the search */
        char *root_uri;

        /* "Stack" of directory stream pointers, so we remember where
           we were reading in order to get the next search result */
        GList *directory_name_stack;
        GList *directory_stream_stack;
        /* Also keep a "stack" of keyword hash tables */
        GList *keywords_stack;

        /* Keep a boolean for a bad / illogical search,
           so we know not to bother looking */
        gboolean invalid_search;

        /* Keep field about whether we should
           try and open everything as a directory,
           or whether the search will stat the files and tell us */
        gboolean search_discovers_file_type;
        
        gboolean search_is_running;
};

typedef struct {
        gboolean directory_has_keywords;
        GHashTable *hash_table;
} DirectoryKeywords;

static char *                  unbracket_root_uri             (const char *seach_uri_without_scheme_or_method);
static char *                  uri_to_search_string           (const char *uri);
static void                    assume_file_is_directory_unless_a_link (const char *uri,
                                                                       gboolean *file_is_directory);
static GList *                 search_string_to_criteria      (char *search_string);
static char *                  search_directory               (MedusaUnindexedSearch *search,
                                                               const char *directory_uri,
                                                               GnomeVFSContext *context,
                                                               gboolean *search_was_cancelled);
static char *                  search_directory_contents_in_progress (MedusaUnindexedSearch *search,
                                                                      GnomeVFSContext *context,
                                                                      gboolean *search_was_cancelled);
static void                    keywords_information_destroy          (DirectoryKeywords *keywords_information);
static void                    pop_keyword_information_and_directory_from_stacks (MedusaUnindexedSearch *search);
static gboolean                file_matches_criteria          (const char *file_uri,
                                                               MedusaUnindexedSearch *search,
                                                               gboolean *file_is_directory,
                                                               GHashTable *keywords_hash_table,
                                                               gboolean directory_has_keywords) ;

static gboolean                matches_all_name_criteria      (GList *name_criteria,
                                                               const char *file_uri,
                                                               MedusaUnindexedSearch *search);
static gboolean                matches_all_file_info_criteria (GList *name_criteria,
                                                               const char *file_uri,
                                                               MedusaUnindexedSearch *search,
                                                               gboolean *file_is_directory);

static gboolean                matches_all_mime_criteria      (GList *name_criteria,
                                                               const char *file_uri,
                                                               MedusaUnindexedSearch *search,
                                                               gboolean *file_is_directory);

static gboolean                matches_all_keywords_criteria  (GList *name_criteria,
                                                               const char *file_uri,
                                                               GHashTable *keywords_hash_table,
                                                               gboolean directory_has_keywords);

static gboolean                matches_one_name                 (const char *criterion,
                                                               const char *file_uri);
static gboolean                matches_one_file_info            (const char *criterion,
                                                               const char *file_uri,
                                                               GnomeVFSFileInfo *file_info);
static gboolean                matches_one_mime_type            (const char *criterion,
                                                               const char *file_uri,
                                                               const char *mime_type);
static gboolean                matches_one_keywords_criterion (const char *criterion,
                                                               const char *file_uri,
                                                               GHashTable *keywords_hash_table,
                                                               gboolean keywords_hash_table_exists_for_directory);
static gboolean                matches_date_modified_criterion  (const char *criterion,
                                                               const char *file_uri,
                                                               GnomeVFSFileInfo *file_info);
static gboolean                matches_owner_criterion          (const char *criterion,
                                                               const char *file_uri,
                                                               GnomeVFSFileInfo *file_info);
static gboolean                matches_group_criterion          (const char *criterion,
                                                               const char *file_uri,
                                                               GnomeVFSFileInfo *file_info);
static gboolean                matches_size_criterion           (const char *criterion,
                                                               const char *file_uri,
                                                               GnomeVFSFileInfo *file_info);
static gboolean                criterion_is_name_criterion    (gpointer criterion_string,
                                                               gpointer user_data);
static gboolean                criterion_is_inode_criterion   (gpointer criterion_string,
                                                               gpointer user_data);
static gboolean                criterion_is_mime_criterion    (gpointer criterion_string,
                                                               gpointer user_data);
static gboolean                criterion_is_keywords_criterion(gpointer criterion_string,
                                                               gpointer user_data);
static gboolean                criterion_field_is             (const char *criterion,
                                                               const char *field_name);   
static char *                  match_string_to_date           (const char *match_string);

static const char *            second_word_of                 (const char *criterion);
static const char *            last_word_of                   (const char *criterion);

MedusaUnindexedSearch *
medusa_unindexed_search_new (GnomeVFSResult *result,
                             const char *search_uri_without_scheme_or_method)
{
        char *search_string;
        GList *other_criteria;
        MedusaUnindexedSearch *search;
  
        if (medusa_unindexed_search_is_available_for_uri (search_uri_without_scheme_or_method) != GNOME_VFS_OK) {
                *result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
                return NULL;
        }
        search = g_new0 (MedusaUnindexedSearch, 1);
        search->root_uri = unbracket_root_uri (search_uri_without_scheme_or_method);
        search_string = uri_to_search_string (search_uri_without_scheme_or_method);
        /* Avoid case issues now */
        g_strdown (search_string);
        search->criteria = search_string_to_criteria (search_string);
        g_free (search_string);
        /* FIXME bugzilla.eazel.com 3004: 
           We should do criteria preprocessing here,
           instead of processing each criterion for each file */
        /* Partition criteria into various difficulties so
           we can look at the easiest ones first */
        search->name_criteria = medusa_g_list_partition (search->criteria,
                                                         criterion_is_name_criterion,
                                                         NULL,
                                                         &other_criteria);
        search->inode_criteria = medusa_g_list_partition (other_criteria,
                                                          criterion_is_inode_criterion,
                                                          NULL,
                                                          &other_criteria);
        search->mime_type_criteria = medusa_g_list_partition (other_criteria,
                                                              criterion_is_mime_criterion,
                                                              NULL,
                                                              &other_criteria);
        search->keywords_criteria = medusa_g_list_partition (other_criteria,
                                                             criterion_is_keywords_criterion,
                                                             NULL,
                                                             &other_criteria);
        search->search_discovers_file_type = ((search->inode_criteria != NULL) ||
                                              (search->mime_type_criteria != NULL));
        /* For now, we can tell a search won't work
           if it contains criteria we don't know about */
        search->invalid_search = (g_list_length (other_criteria) > 0);
        medusa_g_list_free_deep (other_criteria);
        
        if (search->invalid_search) {
                *result = GNOME_VFS_ERROR_INVALID_URI;
        }
        else { 
                *result = GNOME_VFS_OK;
        }

        medusa_unsearched_locations_initialize ();
        return search;
        
}

GnomeVFSResult
medusa_unindexed_search_is_available_for_uri (const char *uri)
{
        g_return_val_if_fail (uri != NULL, GNOME_VFS_ERROR_INTERNAL);
        if (strstr (uri, "& content ") == NULL &&
            strstr (uri, "]content ") == NULL) {
                return GNOME_VFS_OK;
        }
        return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
}

static gboolean
cleanup_current_directory_information (MedusaUnindexedSearch *search)
{
        
        if (search->directory_stream_stack == NULL) {
                return FALSE;
        }

        closedir (search->directory_stream_stack->data);
        keywords_information_destroy (search->keywords_stack->data);
        g_free (search->directory_name_stack->data);
        
        pop_keyword_information_and_directory_from_stacks (search);

        return search->directory_stream_stack != NULL;
}

GnomeVFSResult         
medusa_unindexed_search_read_search_result (MedusaUnindexedSearch *search,
                                            GnomeVFSContext *context,
                                            char **result_uri)
{ 
        gboolean search_was_cancelled;

        *result_uri = NULL;
        search_was_cancelled = FALSE;

        if (search->directory_stream_stack == NULL) {
                *result_uri = search_directory (search, search->root_uri, context, &search_was_cancelled);
        }
        else {
                *result_uri = search_directory_contents_in_progress (search, context, &search_was_cancelled);
        }
        
        if (search_was_cancelled) {
                return GNOME_VFS_ERROR_CANCELLED;
        }

        /* If we didn't find anything in the current directory,
           go up the stack frame until we find a place with results,
           or no more results */
        while (*result_uri == NULL) {
                if (!cleanup_current_directory_information (search)) {
                        return GNOME_VFS_ERROR_EOF;
                }
                *result_uri = search_directory_contents_in_progress (search, context, &search_was_cancelled);
                if (search_was_cancelled) {
                        return GNOME_VFS_ERROR_CANCELLED;
                }
        }

        return GNOME_VFS_OK;
}

static void
closedir_cover (gpointer node_data,
                gpointer extra_data)
{
        closedir (node_data);
}

static void
keywords_information_destroy_cover (gpointer node_data,
                                    gpointer extra_data)
{
        g_assert (node_data != NULL);

        keywords_information_destroy (node_data);
}

void                   
medusa_unindexed_search_destroy (MedusaUnindexedSearch *search)
{
        if (search == NULL) {
                return;
        }

        /* Should free unsearched locations here */
        medusa_g_list_free_deep (search->name_criteria);
        medusa_g_list_free_deep (search->inode_criteria);
        medusa_g_list_free_deep (search->mime_type_criteria);
        medusa_g_list_free_deep (search->keywords_criteria);
        
        medusa_g_list_free_deep_custom (search->directory_stream_stack, closedir_cover, NULL);
        medusa_g_list_free_deep_custom (search->keywords_stack, keywords_information_destroy_cover, NULL);
        medusa_g_list_free_deep (search->directory_name_stack);

        g_free (search->root_uri);
        g_free (search);
}


static GList *
g_list_remove_first_link (GList *list)
{
        return g_list_remove (list, list->data);
}

static void
pop_keyword_information_and_directory_from_stacks (MedusaUnindexedSearch *search)
{
        search->directory_stream_stack = g_list_remove_first_link (search->directory_stream_stack);
        search->directory_name_stack = g_list_remove_first_link (search->directory_name_stack);
        search->keywords_stack = g_list_remove_first_link (search->keywords_stack);
}

static char *
search_directory_contents_in_progress (MedusaUnindexedSearch *search,
                                       GnomeVFSContext *context,
                                       gboolean *search_was_cancelled)
{
        gboolean file_is_directory;
        char *file_path;
        char *file_uri, *file_uri_child_result;
        struct dirent *directory_entry;

        DIR *directory_stream;
        DirectoryKeywords *keyword_information;
        char *directory_path;

        g_assert (search->directory_stream_stack != NULL);
        g_assert (search->keywords_stack != NULL);
        g_assert (search->directory_name_stack != NULL);

        directory_stream = search->directory_stream_stack->data;
        keyword_information = search->keywords_stack->data;
        directory_path = search->directory_name_stack->data;

	while ((directory_entry = readdir (directory_stream)) != NULL) {
		if (!strcmp (directory_entry->d_name,".") || 
		    !strcmp (directory_entry->d_name,"..")) {
			continue;
		}

                if (gnome_vfs_context_check_cancellation (context)) {
                        *search_was_cancelled = TRUE;
                        return NULL;
                }

		file_path = medusa_full_path_from_directory_and_file_name (directory_path,
                                                                           directory_entry->d_name);
                file_uri = gnome_vfs_get_uri_from_local_path (file_path);

                keyword_information = search->keywords_stack->data;
                if (!medusa_is_unsearched_location (file_uri) &&
                    file_matches_criteria (file_uri, 
                                           search, 
                                           &file_is_directory,
                                           keyword_information->hash_table,
                                           keyword_information->directory_has_keywords)) {
                        g_free (file_path);
                        return file_uri;
                }

                if (file_is_directory) {
                        file_uri_child_result = search_directory (search, file_uri, context, search_was_cancelled);
                        if (*search_was_cancelled ||
                            file_uri_child_result != NULL) {
                                return file_uri_child_result;
                        }
                }
                g_free (file_uri);
        }

        return NULL;
}

static void
keywords_information_destroy (DirectoryKeywords *keyword_information)
{
        if (keyword_information->directory_has_keywords) {
                medusa_keyword_hash_table_destroy (keyword_information->hash_table);
        }
        
        g_free (keyword_information);

}

static char *
search_directory (MedusaUnindexedSearch *search, 
                  const char *directory_uri,
                  GnomeVFSContext *context,
                  gboolean *search_was_cancelled)
{
        DIR *directory_stream;
        char *directory_path;
        GnomeVFSResult keywords_hash_table_result;
        DirectoryKeywords *keyword_information;
        char *search_result;

        if (medusa_is_unsearched_location (directory_uri)) {
                return NULL;
        }

        directory_path = gnome_vfs_get_local_path_from_uri (directory_uri);
        g_return_val_if_fail (directory_path != NULL, NULL);
	directory_stream = opendir (directory_path);

	/* If the directory is unreadable, don't bother */
        if (directory_stream == NULL) {
                g_free (directory_path);
                return NULL;
        }
        search->directory_name_stack = g_list_prepend (search->directory_name_stack,
                                                       directory_path);

        search->directory_stream_stack = g_list_prepend (search->directory_stream_stack,
                                                         directory_stream);

        keyword_information = g_new0 (DirectoryKeywords, 1);
        if (search->keywords_criteria != NULL) {
                /* We need to collect the whole directory's
                   keywords before going through the directory */
                keywords_hash_table_result =
                        medusa_keyword_hash_table_create_for_directory (&keyword_information->hash_table,
                                                                        directory_uri);
                keyword_information->directory_has_keywords = keywords_hash_table_result == GNOME_VFS_OK;
        }
        else {
                keyword_information->directory_has_keywords = FALSE;
        }
        
        search->keywords_stack = g_list_prepend (search->keywords_stack,
                                                 keyword_information);

        search_result = search_directory_contents_in_progress (search, context, search_was_cancelled);

        if (*search_was_cancelled ||
            search_result != NULL) {
                return search_result;
        }
        
        cleanup_current_directory_information (search);

        return NULL;
}

static gboolean
file_matches_criteria (const char *file_uri,
                       MedusaUnindexedSearch *search,
                       gboolean *file_is_directory,
                       GHashTable *keywords_hash_table,
                       gboolean directory_has_keywords) 
{

        /* If we don't need to check that the file is a directory,
           we just report that it is a directory, unless
           it's a symbolic link.  That way we don't
           follow circular links */
        /* First check file name criteria */
        if (search->name_criteria != NULL &&
            !matches_all_name_criteria (search->name_criteria,
                                        file_uri,
                                        search)) {

                assume_file_is_directory_unless_a_link (file_uri,
                                                        file_is_directory);
                return FALSE;
        }
        if (search->keywords_criteria != NULL &&
            !matches_all_keywords_criteria (search->keywords_criteria,
                                            file_uri,
                                            keywords_hash_table,
                                            directory_has_keywords)) {
                assume_file_is_directory_unless_a_link (file_uri,
                                                        file_is_directory);
                return FALSE;
        }

        if (search->inode_criteria != NULL &&
            !matches_all_file_info_criteria (search->inode_criteria,
                                             file_uri,
                                             search,
                                             file_is_directory)) {
                return FALSE;
        }
        if (search->mime_type_criteria != NULL &&
            !matches_all_mime_criteria (search->mime_type_criteria,
                                        file_uri,
                                        search,
                                        file_is_directory)) {
                return FALSE;
        }
        if (!search->search_discovers_file_type) {
                assume_file_is_directory_unless_a_link (file_uri,
                                                        file_is_directory);
        }
        return TRUE;
}

static gboolean
matches_all_name_criteria (GList *name_criteria,
                           const char *file_uri,
                           MedusaUnindexedSearch *search)
{
        GList *current_criterion;
        char *criterion_string;
        for (current_criterion = name_criteria; 
             current_criterion != NULL;
             current_criterion = current_criterion->next) {
                criterion_string = (char *) current_criterion->data;
                if (!matches_one_name (criterion_string, file_uri)) {
                        return FALSE;
                }
        }
        return TRUE;
}


static gboolean
matches_all_file_info_criteria (GList *name_criteria,
                                const char *file_uri,
                                MedusaUnindexedSearch *search,
                                gboolean *file_is_directory)
{
        GList *current_criterion;
        GnomeVFSFileInfo *file_info;
        GnomeVFSResult result;
        char *criterion_string;

        file_info = gnome_vfs_file_info_new ();
        result = gnome_vfs_get_file_info (file_uri,
                                          file_info,
                                          GNOME_VFS_FILE_INFO_DEFAULT);
        *file_is_directory = (file_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY);
        if (result != GNOME_VFS_OK) {
                return FALSE;
        }

        
        for (current_criterion = name_criteria; 
             current_criterion != NULL;
             current_criterion = current_criterion->next) {
                criterion_string = (char *) current_criterion->data;
                if (!matches_one_file_info (criterion_string, file_uri, file_info)) {
                        gnome_vfs_file_info_unref (file_info);
                        return FALSE;
                }
        }
        gnome_vfs_file_info_unref (file_info);
        return TRUE;
}


static gboolean
matches_all_mime_criteria (GList *name_criteria,
                           const char *file_uri,
                           MedusaUnindexedSearch *search,
                           gboolean *file_is_directory)
{
        GList *current_criterion;
        GnomeVFSFileInfo *file_info;
        GnomeVFSResult result;
        char *criterion_string;

        file_info = gnome_vfs_file_info_new ();
        result = gnome_vfs_get_file_info (file_uri,
                                          file_info,
                                          GNOME_VFS_FILE_INFO_GET_MIME_TYPE);
        if (result != GNOME_VFS_OK) {
                return FALSE;
        }
        *file_is_directory = (file_info->type == GNOME_VFS_FILE_TYPE_DIRECTORY);
        for (current_criterion = name_criteria; 
             current_criterion != NULL;
             current_criterion = current_criterion->next) {
                criterion_string = (char *) current_criterion->data;
                if (!matches_one_mime_type (criterion_string, 
                                          file_uri, 
                                          file_info->mime_type)) {
                        gnome_vfs_file_info_unref (file_info);
                        return FALSE;
                }
        }
        gnome_vfs_file_info_unref (file_info);
        return TRUE;
}

static gboolean 
matches_all_keywords_criteria (GList *keywords_criteria,
                               const char *file_uri,
                               GHashTable *keywords_hash_table,
                               gboolean directory_has_keywords)
{
        GList *current_criterion;
        const char *criterion_string;

        for (current_criterion = keywords_criteria;
             current_criterion != NULL;
             current_criterion = current_criterion->next) {
                criterion_string = (char *) current_criterion->data;
                if (!matches_one_keywords_criterion (criterion_string,
                                                     file_uri,
                                                     keywords_hash_table,
                                                     directory_has_keywords)) {
                        return FALSE;
                }
                
        }
        return TRUE;
}


static gboolean
matches_one_name (const char *criterion,
                  const char *file_uri)
            
{
        const char *name;
        const char *match_string;
        const char *match_relation;
        static GHashTable *regex_patterns = NULL;
        regex_t *pattern_data;
        GnomeVFSURI *vfs_uri;

        vfs_uri = gnome_vfs_uri_new (file_uri);
        /* FIXME: This value is leaked in the get_dirname case */
        if (criterion_field_is (criterion, MEDUSA_FILE_NAME_CRITERION)) {
                name = gnome_vfs_uri_get_basename (vfs_uri);
        }
        else {
                name = gnome_vfs_uri_extract_dirname (vfs_uri);
        }
        gnome_vfs_uri_unref (vfs_uri);
        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);
        if (medusa_str_has_prefix (match_relation, "is")) {
                return (strcasecmp (match_string, name) == 0);
        }
        if (medusa_str_has_prefix (match_relation, "is_not")) {
                return (strcasecmp (match_string, name) != 0);
        }
        if (medusa_str_has_prefix (match_relation, "contains")) {
                return medusa_strstr_case_insensitive (name, match_string);
        }
        if (medusa_str_has_prefix (match_relation, "does_not_contain")) {
                return !medusa_strstr_case_insensitive (name, match_string);
        }
        if (medusa_str_has_prefix (match_relation, "begins_with")) {
                return medusa_strcase_has_prefix (name, match_string);
        }
        if (medusa_str_has_prefix (match_relation, "ends_with")) {
                return medusa_strcase_has_suffix (name, match_string);
        }
        if (medusa_str_has_prefix (match_relation, "matches")) {
                return (fnmatch (match_string, name, 0));
        }
        if (medusa_str_has_prefix (match_relation, "regexp_matches")) {
                if (regex_patterns == NULL) {
                        regex_patterns = g_hash_table_new (g_str_hash, g_str_equal);
                }
                pattern_data = g_hash_table_lookup (regex_patterns,
                                                    match_string);
                if (pattern_data == NULL) {
                        pattern_data = g_new0 (regex_t, 1);
                        regcomp (pattern_data, match_string, REG_ICASE | REG_NOSUB);
                        g_hash_table_insert (regex_patterns,
                                             g_strdup (match_string),
                                             pattern_data);
                        return (regexec (pattern_data, name, 0, NULL, 0) == 0);
                }
        }

        return FALSE;
}

static gboolean
matches_one_file_info (const char *criterion,
                     const char *file_uri,
                     GnomeVFSFileInfo *file_info)
{
        if (criterion_field_is (criterion, MEDUSA_MODIFIED_CRITERION)) {
                return matches_date_modified_criterion (criterion, file_uri, file_info);
        }
        if (criterion_field_is (criterion, MEDUSA_OWNER_CRITERION)) {
                return matches_owner_criterion (criterion, file_uri, file_info);
        }
        if (criterion_field_is (criterion, MEDUSA_GROUP_CRITERION)) {
                return matches_group_criterion (criterion, file_uri, file_info);
        }
        if (criterion_field_is (criterion, MEDUSA_SIZE_CRITERION)) {
                return matches_size_criterion (criterion, file_uri, file_info);
        }

        return FALSE;
}

static gboolean
matches_date_modified_criterion (const char *criterion,
                                 const char *file_uri,
                                 GnomeVFSFileInfo *file_info)
{
        const char *match_string;
        const char *match_relation;
        char *date;
        gboolean result;

        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);

        date = match_string_to_date (match_string);
        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */
        if (medusa_str_has_prefix (match_relation, "is ")) {
                result = (file_info->mtime > medusa_file_info_get_first_unix_time_occurring_on_date (date)) && 
                        (file_info->mtime <= medusa_file_info_get_last_unix_time_occurring_on_date (date));
        } else if (medusa_str_has_prefix (match_relation, "is_not ")) {
                result = (file_info->mtime < medusa_file_info_get_first_unix_time_occurring_on_date (date)) ||
                        (file_info->mtime >= medusa_file_info_get_last_unix_time_occurring_on_date (date));
        } else if (medusa_str_has_prefix (match_relation, "is_before ")) {
                result = file_info->mtime < medusa_file_info_get_first_unix_time_occurring_on_date (date);
        } else if (medusa_str_has_prefix (match_relation, "is_after ")) {
                result = file_info->mtime > medusa_file_info_get_last_unix_time_occurring_on_date (date);
        } else if (medusa_str_has_prefix (match_relation, "is_within_a_week_of ")) {
                result = (file_info->mtime > medusa_file_info_get_unix_time_a_week_before_date (date)) &&
                        (file_info->mtime < medusa_file_info_get_unix_time_a_week_after_date (date));
        } else if (medusa_str_has_prefix (match_relation, "is_within_a_month_of ")) {
                result = (file_info->mtime > medusa_file_info_get_unix_time_a_month_before_date (date)) &&
                        (file_info->mtime < medusa_file_info_get_unix_time_a_month_after_date (date));
        } else {
		result = FALSE;
        }

        g_free (date);
        return result;
        
}

static gboolean
matches_owner_criterion (const char *criterion,
                       const char *file_uri,
                       GnomeVFSFileInfo *file_info)
{
        const char *match_string;
        const char *match_relation;
        long user_id;
        uid_t uid;

        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);

        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */
        if (medusa_str_has_prefix (match_relation, "is ")) {
                return (medusa_username_to_uid (match_string, &uid) &&
                        file_info->uid == uid);
        }
        if (medusa_str_has_prefix (match_relation, "is_not ")) {
                return (!medusa_username_to_uid (match_string, &uid) ||
                        file_info->uid != uid);
        }
        if (medusa_str_has_prefix (match_relation, "has_uid ")) {
                user_id = strtol (match_string, NULL, 10);
                return (file_info->uid == (uid_t) user_id);
        }
        if (medusa_str_has_prefix (match_relation, "does_not_have_uid ")) {
                user_id = strtol (match_string, NULL, 10);
                return (file_info->uid != (uid_t) user_id);
        }               
        return FALSE;
}

static gboolean
matches_group_criterion (const char *criterion,
                       const char *file_uri,
                       GnomeVFSFileInfo *file_info)
{
        const char *match_string;
        const char *match_relation;
        long group_id;
        gid_t gid;

        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);
        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */
        if (medusa_str_has_prefix (match_relation, "is ")) {
                return (medusa_group_to_gid (match_string, &gid) &&
                        file_info->gid == gid);
        }
        if (medusa_str_has_prefix (match_relation, "is_not ")) {
                return (!medusa_group_to_gid (match_string, &gid) ||
                        file_info->gid != gid);
        }
        if (medusa_str_has_prefix (match_relation, "has_uid ")) {
                group_id = strtol (match_string, NULL, 10);
                return (file_info->uid == (gid_t) group_id);
        }
        if (medusa_str_has_prefix (match_relation, "does_not_have_uid ")) {
                group_id = strtol (match_string, NULL, 10);
                return (file_info->uid != (gid_t) group_id);
        }               
        return FALSE;
}

static gboolean
matches_size_criterion (const char *criterion,
                      const char *file_uri,
                      GnomeVFSFileInfo *file_info)
{
        const char *match_string;
        const char *match_relation;
        long size;

        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);

        size = strtol (match_string, NULL, 10);

        /* If the file has no valid size, then we should return false */
        if (!(file_info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)) {
                return FALSE;
        }
        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */
        if (medusa_str_has_prefix (match_relation, "larger_than ")) {
                return (file_info->size > size);
        }
        if (medusa_str_has_prefix (match_relation, "smaller_than ")) {
                return (file_info->size < size);
        }
        if (medusa_str_has_prefix (match_relation, "is ")) {
                return (file_info->size == size);
        }
        g_warning ("Invalid size request\n");
        return FALSE;
}

static gboolean                
matches_one_mime_type (const char *criterion,
                     const char *file_uri,
                     const char *mime_type)
{
        const char *match_string;
        const char *match_relation;
        gboolean is_negative_query;

        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);
        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */        
        g_return_val_if_fail (medusa_str_has_prefix (match_relation, "is ") ||
                              medusa_str_has_prefix (match_relation, "is_not "),
                              FALSE);
        is_negative_query = medusa_str_has_prefix (match_relation, "is_not  ");

        if (strcmp (match_string, "application") == 0) {
                return medusa_str_has_prefix (mime_type, "application/") ^
                        is_negative_query;
        }
        if (strcmp (match_string, "music") == 0) {
                return medusa_str_has_prefix (mime_type, "audio/") ^
                        is_negative_query;
        }
        if (strcmp (match_string, "text_file") == 0) {
                return medusa_str_has_prefix (mime_type, "text/") ^
                        is_negative_query;
        }
        if (strcmp (match_string, "file") == 0) {
                return !medusa_str_has_prefix (mime_type, "x-directory/") ^
                        is_negative_query;
        }
        if (strcmp (match_string, "directory") == 0) {
                return medusa_str_has_prefix (mime_type, "x-directory/") ^
                        is_negative_query;
        }
        g_warning ("Invalid file type query\n");
        return FALSE;
        
}

static gboolean                
matches_one_keywords_criterion (const char *criterion,
                                const char *file_uri,
                                GHashTable *keywords_hash_table,
                                gboolean keywords_hash_table_exists_for_directory)
{
        const char *match_string;
        const char *match_relation;
        char *file_name;
        gboolean is_negative_query, query_result;
        GnomeVFSURI *vfs_uri;
        
        match_relation = second_word_of (criterion);
        match_string = last_word_of (criterion);

        /* We don't copy the string in second_word_of,
           so we must check prefix rather than using
           a strcmp */        
        g_return_val_if_fail (medusa_str_has_prefix (match_relation, "include ") ||
                              medusa_str_has_prefix (match_relation, "do_not_include "),
                              FALSE);
        is_negative_query = medusa_str_has_prefix (match_relation, "do_not_include  ");

        if (!keywords_hash_table_exists_for_directory) {
                return is_negative_query;
        }
        else {
                vfs_uri = gnome_vfs_uri_new (file_uri);
                file_name = gnome_vfs_uri_extract_short_name (vfs_uri);
                query_result =  is_negative_query ^ 
                        medusa_keyword_hash_table_file_has_keyword (keywords_hash_table, 
                                                                    file_name,
                                                                    match_string);
                g_free (file_name);
                gnome_vfs_uri_unref (vfs_uri);
                return query_result;
        }
}
        



static char *  
unbracket_root_uri (const char *bracketed_uri)
{
        char *root_uri;
        
        g_return_val_if_fail (strchr (bracketed_uri, '[') != NULL, NULL);
        root_uri = strchr (bracketed_uri, '[') + 1;
        g_assert (strchr (root_uri, ']') != NULL);
        return g_strndup (root_uri, 
                          strchr (root_uri, ']') - root_uri);
       
}

static char *  
uri_to_search_string (const char *uri)
{
        char *search_string;

        search_string = strchr (uri, '[');
        g_return_val_if_fail (search_string != NULL, NULL);
        search_string = strchr (search_string, ']');
        g_return_val_if_fail (search_string != NULL, NULL);
        search_string++;
        /* For now, we can have only one root directory */
        g_return_val_if_fail (strchr (search_string, '[') == NULL, NULL);
        return g_strdup (search_string);
        
}

static void                    
assume_file_is_directory_unless_a_link (const char *file_uri,
                                        gboolean *file_is_directory)
{
        char *local_path;
        char link_placeholder[1];
        
        local_path = gnome_vfs_get_local_path_from_uri (file_uri);
        *file_is_directory = (readlink (local_path, link_placeholder, 1) == -1);
        g_free (local_path);

}

static GList *
search_string_to_criteria (char *search_string)
{
        char **split_array;
        int i;
        GList *criteria;
        split_array = g_strsplit (search_string, " & ", 0);
        criteria = NULL;
        for (i = 0; split_array[i] != NULL; i++) {
                criteria = g_list_prepend (criteria,
                                           split_array[i]);
        }
        g_free (split_array);
        return criteria;
} 


static gboolean                
criterion_is_name_criterion (gpointer criterion_string,
                             gpointer user_data)
{
        char *criterion;

        criterion = (char *) criterion_string;
        return (criterion_field_is (criterion, MEDUSA_FILE_NAME_CRITERION) ||
                criterion_field_is (criterion, MEDUSA_DIRECTORY_NAME_CRITERION));
}

static gboolean                
criterion_is_inode_criterion (gpointer criterion_string,
                              gpointer user_data)
{
        char *criterion;
        
        criterion = (char *) criterion_string;
        return  (criterion_field_is (criterion_string, MEDUSA_MODIFIED_CRITERION) ||
                 criterion_field_is (criterion_string, MEDUSA_OWNER_CRITERION) ||
                 criterion_field_is (criterion_string, MEDUSA_GROUP_CRITERION) ||
                 criterion_field_is (criterion_string, MEDUSA_SIZE_CRITERION));


}

static gboolean                
criterion_is_mime_criterion (gpointer criterion_string,
                             gpointer user_data)
{
        char *criterion;
        
        criterion = (char *) criterion_string;
        return (criterion_field_is (criterion_string, MEDUSA_FILE_TYPE_CRITERION));
}

static gboolean                
criterion_is_keywords_criterion (gpointer criterion_string,
                                 gpointer user_data)
{
        char *criterion;
        criterion = (char *) criterion_string;

        return (criterion_field_is (criterion_string, MEDUSA_KEYWORDS_CRITERION));

}

static gboolean                
criterion_field_is (const char *criterion,
                    const char *field_name)
{
        return medusa_str_has_prefix (criterion, field_name);
}


static char *                  
match_string_to_date (const char *match_string)
{
        struct tm  *time_struct;
        time_t relevant_time;

        if (strcmp (match_string, "today") == 0) {
                relevant_time = time (NULL);
                time_struct = localtime (&relevant_time);
                return g_strdup_printf ("%d/%d/%d", time_struct->tm_mon + 1,
                                        time_struct->tm_mday, time_struct->tm_year + 1900);
        }
        if (strcmp (match_string, "yesterday") == 0) {
                relevant_time = time (NULL) - 86400;
                time_struct = localtime (&relevant_time);
                return g_strdup_printf ("%d/%d/%d", time_struct->tm_mon + 1,
                                        time_struct->tm_mday, time_struct->tm_year + 1900);
        }
        return g_strdup (match_string);
}

static const char *            
second_word_of (const char *criterion)
{
        const char *location;

        location = criterion;
        while (!isspace ((guchar) *location)) {
                location++;
        }

        return location + 1;

}

static const char *            
last_word_of (const char *criterion)
{
        const char *location;

        location = criterion + strlen (criterion);
        while (!isspace ((guchar) *location)) {
                location--;
        }
        return location+1;
}

