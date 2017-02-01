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


#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <libmedusa/medusa-string.h>
#include <libmedusa/medusa-utils.h>

#include "medusa-master-db.h"
#include "medusa-master-db-private.h"
#include "medusa-query-optimizations.h"
#include "medusa-uri-list.h"
#include "medusa-search-uri.h"


static gboolean              bypass_search_method                         (char **string);
static char *                get_next_root_uri                            (char **string);
static char **               get_search_criteria                          (const char *string);
static MedusaParsedSearchCriterion * parse_search_criterion               (const char *criterion,                       
                                                                           MedusaMasterDB *master_db,
                                                                           uid_t uid_of_user_making_query);
static gboolean              criterion_is_content_request                 (const char *criterion);
static char *                get_verb_from_criterion                      (const char *criterion);
static char *                get_direct_object_from_criterion             (const char *criterion);
static gboolean              request_wants_all_words_to_match             (const char *verb);
static gboolean              request_is_for_positive_matches              (const char *verb);

static gboolean              search_uri_contains_content_requests         (const char *search_uri);
static gboolean              search_uri_contains_size_requests            (const char *search_uri);

static char *                search_uri_add_user_can_read                 (const char *search_uri, 
                                                                           uid_t uid);
static char *                search_uri_add_type_is_regular_file          (const char *search_uri);
static void                  medusa_parsed_search_criterion_free          (MedusaParsedSearchCriterion *parsed_search_criterion);

static MedusaQueryOptimizationList *optimizations = NULL;



MedusaParsedSearchURI *
medusa_search_uri_parse (const char *search_uri,
                         MedusaMasterDB *master_db,
                         uid_t uid_of_user_making_query)
{
        char *location;
        char *root_uri;
        gboolean search_method_is_valid;
        char **criteria;
        MedusaOptimizationResult *optimization_result;
        int i;
        MedusaParsedSearchURI *parsed_structure;
        MedusaParsedSearchCriterion *parsed_search_criterion;

        parsed_structure = g_new0 (MedusaParsedSearchURI, 1);

        if (!medusa_uri_is_search_uri (search_uri)) {
                parsed_structure->error = MEDUSA_SEARCH_URI_SYNTAX_ERROR;
                return parsed_structure;
        }
        
        location = strchr (search_uri, ':');
        location++;
        
        search_method_is_valid = bypass_search_method (&location);
        if (!search_method_is_valid) {
                parsed_structure->error = MEDUSA_SEARCH_URI_SYNTAX_ERROR;
                return parsed_structure;
        }
        
        root_uri = get_next_root_uri (&location);

        /* We shouldn't be getting any other kinds of uri's except
         * for a single root with root = "file:///".
         */
        if (root_uri == NULL ||
            strcmp (root_uri, "file:///") ||
            get_next_root_uri (&location) != NULL) {
                parsed_structure->error = MEDUSA_SEARCH_URI_SYNTAX_ERROR;
                return parsed_structure;
        }
        
        g_free (root_uri);
        
        location = strchr (search_uri, ']');
        location++;
        
        /* For now we don't except or queries here */
        if (strchr (location, '|') != NULL) {
                parsed_structure->error = MEDUSA_SEARCH_URI_SYNTAX_ERROR;
                return parsed_structure;
        }

        parsed_structure = g_new0 (MedusaParsedSearchURI, 1);        

        g_strdown (location);
        criteria = get_search_criteria (location);
        if  (criteria == NULL) {
                parsed_structure->error = MEDUSA_SEARCH_URI_SYNTAX_ERROR;
                return parsed_structure;
        }
        if (optimizations == NULL) {
                optimizations = medusa_query_optimizations_initialize ();
        }
        optimization_result = medusa_query_optimizations_perform_and_free_deep (optimizations,
                                                                                criteria);

        parsed_structure->error = optimization_result->error;
        if (parsed_structure->error != MEDUSA_SEARCH_URI_NO_ERROR) {
                return parsed_structure;
        }
        
        for (i=0; optimization_result->criteria[i] != NULL; i++) {
                parsed_search_criterion =  parse_search_criterion (optimization_result->criteria[i],
                                                                   master_db,
                                                                   uid_of_user_making_query);
                /* Detect syntax errors */
                if (parsed_search_criterion == NULL) {
                        parsed_structure->error = MEDUSA_SEARCH_URI_SYNTAX_ERROR;
                        medusa_optimization_result_destroy (optimization_result);
                        return parsed_structure;
                }
                parsed_structure->parsed_search_criteria = g_list_prepend (parsed_structure->parsed_search_criteria, parsed_search_criterion);
                                                               
        }
        medusa_optimization_result_destroy (optimization_result);
        return parsed_structure;
}

void
medusa_parsed_search_uri_free (MedusaParsedSearchURI *parsed_search_uri)
{
        g_free (parsed_search_uri);
        
}

void
medusa_search_uri_clean_up_parse_data (void)
{
        medusa_query_optimizations_destroy (optimizations);
        optimizations = NULL;
}

static MedusaDatabaseAndUIDData *
database_and_uid_data_new (gpointer database,
                           uid_t uid)
{
        MedusaDatabaseAndUIDData *data;
        
        data = g_new0 (MedusaDatabaseAndUIDData, 1);
        data->database = database;
        data->uid_of_user_making_query = uid;

        return data;
}

static MedusaParsedSearchCriterion *
parse_search_criterion (const char *criterion,
                        MedusaMasterDB *master_db,
                        uid_t uid_of_user_making_query)
{
        MedusaQueryFunc evaluate;
        MedusaResultCheckFunc verify;
        MedusaParsedSearchCriterion *closure;
        MedusaArgumentType type;
        char *verb, *direct_object;

        verb = get_verb_from_criterion (criterion);
        if (verb == NULL) {
                return NULL;
        }
        direct_object = get_direct_object_from_criterion (criterion);
        if (direct_object == NULL) {
                return NULL;
        }
        closure = NULL;
        medusa_query_clauses_get_function (medusa_uri_list_get_query_clauses (master_db->uri_list),
                                           criterion,
                                           &evaluate,
                                           &verify,
                                           &type);
        if (evaluate != NULL) {
                closure = g_new0 (MedusaParsedSearchCriterion, 1);
                closure->argument_type = type;
                closure->query_func = evaluate;
                closure->check_func = verify;
                closure->database_record_type = MEDUSA_URI_INDEX_RECORD;                
                closure->query_data = master_db->uri_list;

        }
        
        medusa_query_clauses_get_function (medusa_file_system_db_get_query_clauses_using_index_only (master_db->file_system_db),
                                           criterion,
                                           &evaluate,
                                           &verify,
                                           &type);
        if (evaluate != NULL) {
                closure = g_new0 (MedusaParsedSearchCriterion, 1);
                closure->argument_type = type;
                closure->query_func = evaluate;
                closure->check_func = verify;
                closure->query_data = master_db->file_system_db;
                closure->database_record_type = MEDUSA_FILE_ATTRIBUTE_INDEX_RECORD;

        }
        medusa_query_clauses_get_function (medusa_file_system_db_get_query_clauses_using_index_and_uid (master_db->file_system_db),
                                           criterion,
                                           &evaluate,
                                           &verify,
                                           &type);
        if (evaluate != NULL) {
                closure = g_new0 (MedusaParsedSearchCriterion, 1);
                closure->argument_type = type;
                closure->query_func = evaluate;
                closure->check_func = verify;
                closure->query_data = database_and_uid_data_new (master_db->file_system_db,
                                                                 uid_of_user_making_query);
                closure->free_query_data_when_finished = TRUE;
                closure->database_record_type = MEDUSA_FILE_ATTRIBUTE_INDEX_RECORD;

        }
        if (closure) {
                switch (type) {
                case MEDUSA_ARGUMENT_TYPE_STRING:
                        closure->argument.string = g_strdup (direct_object);
                        break;
                case MEDUSA_ARGUMENT_TYPE_NUMBER:
                        closure->argument.number = strtol (direct_object, NULL, 10);
                        break;
                default:
                        g_assert_not_reached ();
                        return NULL;
                }
                closure->is_content_request = FALSE;
                g_free (verb);
                g_free (direct_object);
                return closure;
        }

        if (criterion_is_content_request (criterion)) {
                closure = g_new0 (MedusaParsedSearchCriterion, 1);
                closure->argument_type = MEDUSA_ARGUMENT_TYPE_NONE;
                closure->query_func = evaluate;
                closure->is_content_request = TRUE;
                closure->match_all_words = request_wants_all_words_to_match (verb);
                closure->content_request = g_strdup (direct_object);
                closure->return_matches = request_is_for_positive_matches (verb);
                g_free (verb);
                g_free (direct_object);
                return closure;
        }

        g_free (verb);
        g_free (direct_object);
        return NULL;
}

char *
medusa_search_uri_add_extra_needed_criteria (const char *search_uri,
                                             uid_t uid)
{
        char *current_uri;
        char *new_uri;

        current_uri = g_strdup (search_uri);
        if (search_uri_contains_content_requests (current_uri)) {
                new_uri = search_uri_add_user_can_read (current_uri,
                                                        uid);
                g_free (current_uri);
                current_uri = new_uri;
        }
        if (search_uri_contains_size_requests (current_uri)) {
                new_uri = search_uri_add_type_is_regular_file (current_uri);
                g_free (current_uri);
                current_uri = new_uri;
        }
        
        return current_uri;
                                                               
}

static gboolean              
search_uri_contains_content_requests (const char *search_uri)
{
        char *text_request, *text_anti_request, *file_type_is;
        text_request = strstr (search_uri,"content includes");
        text_anti_request = strstr (search_uri, "content does_not_include");
        file_type_is = strstr (search_uri, "file_type is");
        return (text_request != NULL ||
                text_anti_request != NULL ||
                file_type_is != NULL);
}

static char *                
search_uri_add_user_can_read (const char *search_uri, 
                              uid_t uid)
{
        char *new_criterion;
        char *extended_search_uri;

        new_criterion = g_strdup_printf ("permissions_to_read include_uid %ld", (long int) uid);
        if (strstr (search_uri, new_criterion)) {
                g_free (new_criterion);
                return g_strdup (search_uri);
        }
        else {
                extended_search_uri = g_strdup_printf ("%s & %s", search_uri, new_criterion);
                g_free (new_criterion);
                return extended_search_uri;
        }
}


static gboolean              
search_uri_contains_size_requests (const char *search_uri)
{
        char *size_larger, *size_smaller, *size_is;
        size_larger = strstr (search_uri,"size larger_than ");
        size_smaller = strstr (search_uri, "size smaller_than ");
        size_is = strstr (search_uri, "size is");
        return (size_larger != NULL ||
                size_smaller != NULL ||
                size_is != NULL);
}

static char *                
search_uri_add_type_is_regular_file (const char *search_uri)

{
        char *extended_search_uri;

        if (strstr (search_uri, "file_type is file")) {
                return g_strdup (search_uri);
        }
        else {
                extended_search_uri = g_strdup_printf ("%s & file_type is file", search_uri);
                return extended_search_uri;
        }
}

static void                  
parsed_search_criterion_free_cover (gpointer data,
                                  gpointer user_data)
{
        g_return_if_fail (data != NULL);

        medusa_parsed_search_criterion_free (data);
}

static void                  
medusa_parsed_search_criterion_free (MedusaParsedSearchCriterion *closure)
{
        if (closure->argument_type == MEDUSA_ARGUMENT_TYPE_STRING) {
                g_free (closure->argument.string);
        }

        if (closure->is_content_request) {
                g_free (closure->content_request);
        }
        if (closure->free_query_data_when_finished) {
                g_free (closure->query_data);
        }
        g_free (closure);
}

void
medusa_parsed_search_criterion_list_free (GList *parsed_search_criteria)
{
        g_list_foreach (parsed_search_criteria, parsed_search_criterion_free_cover, NULL);
        g_list_free (parsed_search_criteria);
}

gboolean              
medusa_parsed_search_criterion_is_content_search (gpointer data,
                                         gpointer user_data)
{
        MedusaParsedSearchCriterion *parsed_search_criterion;

        parsed_search_criterion = (MedusaParsedSearchCriterion *) data;
        g_return_val_if_fail (parsed_search_criterion != NULL, FALSE);

        g_assert (parsed_search_criterion->is_content_request == TRUE ||
                  parsed_search_criterion->is_content_request == FALSE);

        return parsed_search_criterion->is_content_request;
}

gboolean                 
medusa_parsed_search_criterion_has_check (MedusaParsedSearchCriterion *criterion)
{
        g_return_val_if_fail (criterion != NULL, FALSE);

        return criterion->check_func != NULL;
}

gboolean                 
medusa_parsed_search_criterion_run_check (MedusaParsedSearchCriterion *criterion,
                                          const char *uri)
{
        g_return_val_if_fail (criterion != NULL, FALSE);
        g_return_val_if_fail (criterion->check_func != NULL, FALSE);

        return criterion->check_func (uri,
                                   criterion->argument);
}


gboolean
medusa_uri_is_search_uri (const char *uri)
{
        char *trimmed_uri;
        gboolean result;

        /* Remove leading spaces */
        trimmed_uri = g_strdup (uri);
        trimmed_uri = g_strchug (trimmed_uri);
        result = FALSE;

        if (strncmp (trimmed_uri, "gnome-search:", strlen ("gnome-search:")) ==  0) {
                result = TRUE;
        }
        if (strncmp (trimmed_uri, "search:", strlen ("search:")) == 0) {
                result = TRUE;
        }
        if (strncmp (trimmed_uri, "medusa:", strlen ("medusa:")) == 0) {
                result = TRUE;
        }
        g_free (trimmed_uri);
        return result;
}

static gboolean
bypass_search_method (char **string)
{
        if (**string == '[') {
                /* There's no method, we're fine */
                return TRUE;
        }
        if (medusa_str_has_prefix (*string,
                                   "index-only")) {
                *string += strlen ("index-only");
                return TRUE;
        }
        if (medusa_str_has_prefix (*string,
                                   "index-if-available")) {
                *string += strlen ("index-if-available");
                return TRUE;
        }                          
        if (medusa_str_has_prefix (*string,
                                   "index-with-backup")) {
                *string += strlen ("index-with-backup");
                return TRUE;

        }
        return FALSE;
}

/* Gets the next bracket enclosed segment.
   does no checking for uri validity */
static char *
get_next_root_uri (char **string)
{
        char *close_bracket;
        char *uri;
#ifdef SEARCH_URI_DEBUG
        printf ("Trying to parse string %s\n", *string);
#endif
        g_return_val_if_fail (*string != NULL, NULL);

        /* Return if there are no more uri's */
        if (*string[0] != '[') {
                return NULL;
        }

        close_bracket = strchr (*string, ']');

        if (close_bracket == NULL) {
                return NULL;
        }
        
        uri = g_strndup (*string + 1, close_bracket - (*string + 1));
#ifdef SEARCH_URI_DEBUG
        printf ("Next root uri is %s\n", uri);
#endif
        *string = close_bracket + 1;
        return uri;
}

static char **
get_search_criteria (const char *string)
{
        g_return_val_if_fail (string != NULL, NULL);
        return g_strsplit (string, " & ", G_MAXINT);
}


static gboolean              
criterion_is_content_request (const char *criterion)
{
        return medusa_str_has_prefix (criterion, "content");
}

static char *                
get_verb_from_criterion (const char *criterion)
{
        const char *end_of_word;

        if (strchr (criterion, ' ') == NULL) {
                return NULL;
        }
        for ( ; *criterion != ' '; criterion++);
        for ( ; *criterion == ' '; criterion++);
        if (strchr (criterion, ' ') == NULL) {
                return NULL;
        }
        for (end_of_word = criterion; *end_of_word != ' '; end_of_word++);

        return g_strndup (criterion, end_of_word - criterion);

}

static char *                
get_direct_object_from_criterion (const char *criterion)
{
        g_assert (strchr (criterion, ' ') != NULL);
        for ( ; *criterion != ' '; criterion++);
        for ( ; *criterion == ' '; criterion++);
        if (strchr (criterion, ' ') == NULL) {
                return NULL;
        }
        for ( ; *criterion != ' '; criterion++);
        for ( ; *criterion == ' '; criterion++);

        return g_strdup (criterion);

}

static gboolean              
request_wants_all_words_to_match (const char *verb)
{
        if (strcmp (verb, "includes_all_of") == 0 ||
            strcmp (verb, "does_not_include_all_of") == 0) {
                return TRUE;
        }
        else {
                g_return_val_if_fail (strcmp (verb, "includes_any_of") == 0 ||
                                      strcmp (verb, "does_not_include_any_of") == 0,
                                      FALSE);
                return FALSE;
        }
}

static gboolean              
request_is_for_positive_matches (const char *verb)
{
        if (strcmp (verb, "includes_all_of") == 0 ||
            strcmp (verb, "includes_any_of") == 0) {
                return TRUE;
        }
        else {
                g_return_val_if_fail (strcmp (verb, "does_not_include_all_of") == 0 ||
                                      strcmp (verb, "does_not_include_any_of") == 0,
                                      FALSE);
                return FALSE;
        }
}

