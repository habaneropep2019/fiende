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
 *  medusa-query-optimization.c -- Functions to replace query criteria with
 *  equivalent criteria that will take less time to run, or reorder the
 *  criteria
 */

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include <libmedusa/medusa-index-service.h>
#include <libmedusa/medusa-string.h>
#include <libmedusa/medusa-utils.h>
#include <libmedusa/medusa-file-info-utilities.h>

#include "medusa-query-optimizations.h"
#include "medusa-query-optimizations-private.h"

typedef char QueryCriterion;

static const char *modified_relations[] = {
        "is ",
        "is_not ",
        "is_before ",
        "is_after ",
        "is_within_a_week_of ",
        "is_within_a_month_of ",
        NULL
};

        

struct MedusaQueryOptimizationList {
        GList *run_first_optimizations;   /* of struct QueryOptimization */
        GList *run_normal_optimizations;
        GList *run_last_optimizations;
};

typedef struct MedusaQueryOptimization MedusaQueryOptimization;

struct MedusaQueryOptimization {
        MedusaOptimizationCheckFunc optimization_is_applicable;
        MedusaOptimizationFunc optimize;
};

static void                         query_optimization_free              (gpointer data,
                                                                          gpointer user_data);
static gint                         optimization_is_applicable           (gconstpointer data,
                                                                          gconstpointer user_data);
static gboolean                     result_is_error                      (MedusaOptimizationResult *result);

static MedusaOptimizationResult *   optimization_result_new              (void);

static char *                       query_criterion_get_object           (const QueryCriterion *criterion);
static const char *                 query_criterion_get_relation         (const QueryCriterion *criterion);
static gboolean                     query_relation_is_negative           (const char *query_relation);

static const char *                 username_relation_to_corresponding_uid_relation (const char *query_relation);
static gboolean                     owner_query_relation_is_invalid      (const char *query_relation);
static gboolean                     can_replace_username_with_uid        (MedusaUnparsedCriteria *criteria);
static MedusaOptimizationResult *   replace_username_with_uid_criterion  (MedusaUnparsedCriteria *criteria);

static gboolean                     request_is_too_new_for_database      (MedusaUnparsedCriteria *criteria);
static MedusaOptimizationResult *   set_error_request_obsoletes_index    (MedusaUnparsedCriteria *criteria);

static gboolean                     request_is_newer_than_the_present    (MedusaUnparsedCriteria *criteria);
static MedusaOptimizationResult *   set_error_always_false               (MedusaUnparsedCriteria *criteria);


static const char *                 group_relation_to_corresponding_gid_relation (const char *query_relation);
static gboolean                     group_query_relation_is_invalid      (const char *query_relation);
static gboolean                     can_replace_group_with_gid           (MedusaUnparsedCriteria *criteria);
static MedusaOptimizationResult *   replace_group_with_gid_criterion     (MedusaUnparsedCriteria *criteria);

static gboolean                     modified_query_relation_is_invalid   (const char *query_relation);
static gboolean                     modified_query_relation_has_no_upper_bound (const char *query_relation);
static gboolean                     modified_query_relation_has_no_lower_bound (const char *query_relation);
static char *                       make_new_criterion_with_time_t       (const char *type,
                                                                          const char *relation,
                                                                          time_t object);
static time_t                       first_time_corresponding_to_query_object (const char *query_relation,
                                                                              const char *object);
static time_t                       last_time_corresponding_to_query_object  (const char *query_relation,
                                                                              const char *object);
static gboolean                     can_replace_modified_with_mtime      (MedusaUnparsedCriteria *criteria);
static MedusaOptimizationResult *   replace_modified_with_mtime_criteria (MedusaUnparsedCriteria *criteria);

/*
static gboolean                     can_replace_file_type_with_mime       (MedusaUnparsedCriteria *criteria);
static gboolean                     replace_file_type_with_mime_criterion (MedusaUnparsedCriteria *criteria);
*/

MedusaQueryOptimizationList *      
medusa_query_optimizations_initialize  (void)
{
        MedusaQueryOptimizationList *optimizations;

        optimizations = medusa_query_optimizations_new ();

        medusa_query_optimizations_add (optimizations,
                                        can_replace_username_with_uid,
                                        replace_username_with_uid_criterion,
                                        MEDUSA_OPTIMIZATION_RUN_REGULAR);
        
        medusa_query_optimizations_add (optimizations,
                                        can_replace_group_with_gid,
                                        replace_group_with_gid_criterion,
                                        MEDUSA_OPTIMIZATION_RUN_REGULAR);
        medusa_query_optimizations_add (optimizations,
                                        can_replace_modified_with_mtime,
                                        replace_modified_with_mtime_criteria,
                                        MEDUSA_OPTIMIZATION_RUN_REGULAR);
        medusa_query_optimizations_add (optimizations,
                                        request_is_too_new_for_database,
                                        set_error_request_obsoletes_index,
                                        MEDUSA_OPTIMIZATION_RUN_LAST);
        medusa_query_optimizations_add (optimizations,
                                        request_is_newer_than_the_present,
                                        set_error_always_false,
                                        MEDUSA_OPTIMIZATION_RUN_LAST);
        return optimizations;
}

MedusaOptimizationResult *       
medusa_query_optimizations_perform_and_free_deep (MedusaQueryOptimizationList *optimizations,
                                                  MedusaUnparsedCriteria *criteria)
{
        GList *optimization_list;
        MedusaQueryOptimization *relevant_optimization;
        MedusaOptimizationResult *result;

        g_return_val_if_fail (optimizations != NULL, NULL);
        result = optimization_result_new ();

        result->criteria = criteria;

        optimization_list = optimizations->run_first_optimizations;
        while ((optimization_list = g_list_find_custom (optimization_list,
                                                        result->criteria,
                                                        optimization_is_applicable)) != NULL) {
                relevant_optimization = optimization_list->data;
                result = relevant_optimization->optimize (result->criteria);
                if (result_is_error (result)) {
                        return result;
                }
        }
        optimization_list = optimizations->run_normal_optimizations;
        while ((optimization_list = g_list_find_custom (optimization_list,
                                                        result->criteria,
                                                        optimization_is_applicable)) != NULL) {
                relevant_optimization = optimization_list->data;
                result = relevant_optimization->optimize (result->criteria);
                if (result_is_error (result)) {
                        return result;
                }
        }
        optimization_list = optimizations->run_last_optimizations;
        while ((optimization_list = g_list_find_custom (optimization_list,
                                                        result->criteria,
                                                        optimization_is_applicable)) != NULL) {
                relevant_optimization = optimization_list->data;
                result = relevant_optimization->optimize (result->criteria);
                if (result_is_error (result)) {
                        return result;
                }
        }
        return result;
}

                                                                   

MedusaQueryOptimizationList *                            
medusa_query_optimizations_new (void)
{
        MedusaQueryOptimizationList *optimizations;

        optimizations = g_new0 (MedusaQueryOptimizationList, 1);
        optimizations->run_first_optimizations = NULL;
        optimizations->run_normal_optimizations = NULL;
        optimizations->run_last_optimizations = NULL;

        return optimizations;
}

void
medusa_query_optimizations_destroy (MedusaQueryOptimizationList *optimizations)
{
        if (optimizations == NULL) {
                return;
        }
        if (optimizations->run_first_optimizations != NULL) {
                medusa_g_list_free_deep_custom (optimizations->run_first_optimizations,
                                                query_optimization_free,
                                                NULL);
        }
        if (optimizations->run_normal_optimizations != NULL) {
                medusa_g_list_free_deep_custom (optimizations->run_normal_optimizations,
                                                query_optimization_free,
                                                NULL);
        }
        if (optimizations->run_last_optimizations != NULL) {
                medusa_g_list_free_deep_custom (optimizations->run_last_optimizations,
                                                query_optimization_free,
                                                NULL);
        }
        g_free (optimizations);
}


void                            
medusa_query_optimizations_add (MedusaQueryOptimizationList *optimizations,
                                MedusaOptimizationCheckFunc optimization_is_applicable,
                                MedusaOptimizationFunc optimize,
                                MedusaOptimizationPriority priority)
{
        MedusaQueryOptimization *new_optimization;

        new_optimization = g_new0 (MedusaQueryOptimization, 1);
        new_optimization->optimization_is_applicable = optimization_is_applicable;
        new_optimization->optimize = optimize;

        switch (priority) {
        case MEDUSA_OPTIMIZATION_RUN_FIRST:
                optimizations->run_first_optimizations = g_list_prepend (optimizations->run_first_optimizations,
                                                                          new_optimization);
                break;
        case MEDUSA_OPTIMIZATION_RUN_REGULAR:
                optimizations->run_normal_optimizations = g_list_prepend (optimizations->run_normal_optimizations,
                                                                          new_optimization);
                break;
        case MEDUSA_OPTIMIZATION_RUN_LAST:
                optimizations->run_last_optimizations = g_list_prepend (optimizations->run_last_optimizations,
                                                                          new_optimization);
                break;
        }

}

void                            
medusa_optimization_result_destroy (MedusaOptimizationResult *optimization_result)
{
        g_strfreev (optimization_result->criteria);
        g_free (optimization_result);
}


static void                 
query_optimization_free (gpointer data,
                         gpointer user_data)
{
        MedusaQueryOptimization *optimization;

        g_return_if_fail (data != NULL);

        optimization = (MedusaQueryOptimization *) data;
        g_free (optimization);
        
}

static gint                 
optimization_is_applicable (gconstpointer data,
                            gconstpointer user_data)
{
        MedusaQueryOptimization *optimization;
        MedusaUnparsedCriteria *criteria;

        g_assert (data != NULL);
        optimization = (MedusaQueryOptimization *) data;
        criteria = (MedusaUnparsedCriteria *) user_data;

        /* You can't optimize nothing */
        if (criteria == NULL) {
                return 1;
        }
        if (optimization->optimization_is_applicable (criteria)) {
                return 0;
        }
        else {
                return 1;
        }
}

static gboolean                 
result_is_error (MedusaOptimizationResult *result)
{
        return result->error != MEDUSA_SEARCH_URI_NO_ERROR;
}


static MedusaOptimizationResult *
optimization_result_new (void)
{
        MedusaOptimizationResult *optimization_result;

        optimization_result = g_new0 (MedusaOptimizationResult, 1);
        optimization_result->error = MEDUSA_SEARCH_URI_NO_ERROR;
        optimization_result->criteria = NULL;

        return optimization_result;
}


static void
replace_old_criterion_with_new_and_free (MedusaUnparsedCriteria *criteria,
                                         QueryCriterion *old_criterion,
                                         QueryCriterion *new_criterion)
{
        int i;
        for (i = 0; criteria[i] != NULL; i++) {
                if (strcmp (criteria[i], old_criterion) == 0) {
                        criteria[i] = new_criterion;
                        g_free (old_criterion);
                        return;
                }
        }

        g_assert_not_reached ();
}

static MedusaUnparsedCriteria *
add_new_criterion_and_free (MedusaUnparsedCriteria *old_criteria,
                            QueryCriterion *new_criterion)
{
        MedusaUnparsedCriteria *new_criteria;
        int i, old_criteria_count;
        for (old_criteria_count = 0; old_criteria[old_criteria_count] != NULL; old_criteria_count++);
        new_criteria = g_new0 (MedusaUnparsedCriteria, old_criteria_count + 2);
        for (i = 0; i < old_criteria_count; i++) {
                new_criteria[i] = old_criteria[i];
        }
        new_criteria[i] = new_criterion;
        new_criteria[i + 1] = NULL;
        g_free (old_criteria);
        return new_criteria;


}
static QueryCriterion *
query_criterion_get_by_category (MedusaUnparsedCriteria *criteria,
                                 const char *type)
{
        int i;
        for (i = 0; criteria[i] != NULL; i++) {
                if (medusa_str_has_prefix (criteria[i], type)) {
                        return criteria[i];
                }
        }        
        return NULL;
}

static QueryCriterion *
query_criterion_get_by_prefix (MedusaUnparsedCriteria *criteria,
                               const char *prefix)
{
        int i;
        for (i = 0; criteria[i] != NULL; i++) {
                if (medusa_str_has_prefix (criteria[i], prefix)) {
                        return criteria[i];
                }
        }        
        return NULL;
}

static const char *
query_criterion_get_relation (const QueryCriterion *criterion)
{
        char *location;
        location = strchr (criterion,' ');
        g_return_val_if_fail (location != NULL, NULL);
        while (*location == ' ' && *location != 0) {
                location++;
        }
        g_return_val_if_fail (*location != 0, NULL);
        return location;

}

static char *
query_criterion_get_object (const QueryCriterion *criterion)
{
        char *location;
        location = strchr (criterion,' ');
        g_return_val_if_fail (location != NULL, NULL);
        while (*location == ' ' && *location != 0) {
                location++;
        }
        g_return_val_if_fail (*location != 0, NULL);
        location = strchr (location,' ');
        g_return_val_if_fail (location != NULL, NULL);
        while (*location == ' ' && *location != 0) {
                location++;
        }
        g_return_val_if_fail (*location != 0, NULL);
        return location;
}

static gboolean                     
query_relation_is_negative (const char *query_relation)
{
        return medusa_str_has_prefix (query_relation, "is_not ");
}

static char *
make_new_criterion_with_int (const char *type,
                             const char *relation,
                             int object)
{
        g_return_val_if_fail (type != NULL && relation != NULL, NULL);
        return g_strdup_printf ("%s %s %d", type, relation, object);
}

static gboolean                     
can_replace_username_with_uid (MedusaUnparsedCriteria *criteria)
{
        int i;
        for (i = 0; criteria[i] != NULL; i++) {
                if (medusa_str_has_prefix (criteria[i],"owner is ") ||
                    medusa_str_has_prefix (criteria[i],"owner is_not")) {
                        return TRUE;
                }
        }
        return FALSE;
}

static gboolean                     
request_is_too_new_for_database (MedusaUnparsedCriteria *criteria)
{
        int i;
        time_t after_time, time_index_was_made;
        const char *time_as_string;

        for (i = 0; criteria[i] != NULL; i++) {
                if (medusa_str_has_prefix (criteria[i], "mtime is_after")) {
                        time_as_string = criteria[i] + strlen ("mtime is_after");
                        after_time = strtol (time_as_string, NULL, 10);
                        time_index_was_made = medusa_index_service_get_last_index_update_time ();
                        if (after_time > time_index_was_made  &&
                            after_time <= time (NULL)) {
                                return TRUE;
                        }
                }
        }

        return FALSE;
}

static MedusaOptimizationResult *   
set_error_request_obsoletes_index (MedusaUnparsedCriteria *criteria)
{
        MedusaOptimizationResult *result;

        result = optimization_result_new ();
        g_strfreev (result->criteria);
        result->criteria = NULL;
        result->error = MEDUSA_SEARCH_URI_OBSOLETES_INDEX;
        return result;
}


static gboolean                     
request_is_newer_than_the_present (MedusaUnparsedCriteria *criteria)
{
        int i;
        time_t after_time;
        const char *time_as_string;

        for (i = 0; criteria[i] != NULL; i++) {
                if (medusa_str_has_prefix (criteria[i], "mtime is_after")) {
                        time_as_string = criteria[i] + strlen ("mtime is_after");
                        after_time = strtol (time_as_string, NULL, 10);
                        if (after_time > time (NULL)) {
                                return TRUE;
                        }
                }
        }

        return FALSE;
}

static MedusaOptimizationResult *   
set_error_always_false (MedusaUnparsedCriteria *criteria)
{
        MedusaOptimizationResult *result;

        result = optimization_result_new ();
        g_strfreev (result->criteria);
        result->criteria = NULL;
        result->error = MEDUSA_SEARCH_URI_IS_ALWAYS_FALSE;
        return result;
}


static gboolean
owner_query_relation_is_invalid (const char *query_relation)
{
        if (query_relation == NULL) {
                return FALSE;
        }
        return (medusa_str_has_prefix (query_relation, "is ") == FALSE &&
                medusa_str_has_prefix (query_relation, "is_not ") == FALSE);
}

static const char *
username_relation_to_corresponding_uid_relation (const char *query_relation)
{
        g_return_val_if_fail (owner_query_relation_is_invalid (query_relation) == FALSE, NULL);
        if (medusa_str_has_prefix (query_relation, "is ")) {
                return "has_uid";
        }
        if (medusa_str_has_prefix (query_relation, "is_not ")) {
                return "does_not_have_uid";
        }

        g_assert_not_reached ();
        return NULL;
}

static MedusaOptimizationResult *
replace_username_with_uid_criterion (MedusaUnparsedCriteria *criteria) 
{
        MedusaOptimizationResult *result;
        uid_t uid;
        QueryCriterion *old_criterion;
        QueryCriterion *new_criterion;
        const char *query_relation;  /* The relationship, eg, "is" */
        const char *query_object;    /* The subject, eg "root" */

        g_return_val_if_fail (can_replace_username_with_uid (criteria), NULL);
        result = optimization_result_new ();
        
        old_criterion = query_criterion_get_by_prefix (criteria,
                                                       "owner is");
        query_relation = query_criterion_get_relation (old_criterion);
        if (query_relation == NULL) {
                result->error = MEDUSA_SEARCH_URI_SYNTAX_ERROR;
                return result;
        }
        query_object = query_criterion_get_object (old_criterion);
        if (owner_query_relation_is_invalid (query_relation) ||
            query_object == NULL) {
                result->error = MEDUSA_SEARCH_URI_SYNTAX_ERROR;
                return result;
        }
        if (medusa_username_to_uid (query_object,
                                    &uid)) {
                new_criterion = make_new_criterion_with_int ("owner",
                                                             username_relation_to_corresponding_uid_relation (query_relation),
                                                             (int) uid);
                replace_old_criterion_with_new_and_free (criteria,
                                                         old_criterion,
                                                         new_criterion);
                result->criteria = criteria;
                
        }
        else {
                if (query_relation_is_negative (query_relation)) {
                        result->error = MEDUSA_SEARCH_URI_IS_ALWAYS_TRUE;
                }
                else {
                        result->error = MEDUSA_SEARCH_URI_IS_ALWAYS_FALSE;
                }
        }
        
        return result;
}

static const char *                 
group_relation_to_corresponding_gid_relation (const char *query_relation)
{
        g_return_val_if_fail (owner_query_relation_is_invalid (query_relation) == FALSE, NULL);
        if (medusa_str_has_prefix (query_relation, "is ")) {
                return "has_gid";
        }
        if (medusa_str_has_prefix (query_relation, "is_not ")) {
                return "does_not_have_gid";
        }

        g_assert_not_reached ();
        return NULL;

}

static gboolean                     
group_query_relation_is_invalid (const char *query_relation)
{
        if (query_relation == NULL) {
                return FALSE;
        }
        return (medusa_str_has_prefix (query_relation, "is ") == FALSE &&
                medusa_str_has_prefix (query_relation, "is_not ") == FALSE);
}

static gboolean                     
can_replace_group_with_gid (MedusaUnparsedCriteria *criteria)
{
        int i;

        for (i = 0; criteria[i] != NULL; i++) {
                if (medusa_str_has_prefix (criteria[i],"group is ") ||
                    medusa_str_has_prefix (criteria[i],"group is_not")) {
                        return TRUE;
                }
        }
        return FALSE;
}



static MedusaOptimizationResult *   
replace_group_with_gid_criterion (MedusaUnparsedCriteria *criteria)
{
        MedusaOptimizationResult *result;
        uid_t gid;
        QueryCriterion *old_criterion;
        QueryCriterion *new_criterion;
        const char *query_relation;  /* The relationship, eg, "is" */
        const char *query_object;    /* The subject, eg "root" */

        g_return_val_if_fail (can_replace_group_with_gid (criteria), NULL);
        result = optimization_result_new ();
        old_criterion = query_criterion_get_by_category (criteria,
                                                         "group is");
        query_relation = query_criterion_get_relation (old_criterion);
        query_object = query_criterion_get_object (old_criterion);
        if (group_query_relation_is_invalid (query_relation) ||
            query_object == NULL) {
                result->error = MEDUSA_SEARCH_URI_SYNTAX_ERROR;
                return result;
        }
        if (medusa_group_to_gid (query_object,
                                 &gid)) {
                new_criterion = make_new_criterion_with_int ("group",
                                                             group_relation_to_corresponding_gid_relation (query_relation),
                                                             (int) gid);
                replace_old_criterion_with_new_and_free (criteria,
                                                         old_criterion,
                                                         new_criterion);
                result->criteria = criteria;
                
        }
        else {
                if (query_relation_is_negative (query_relation)) {
                        result->error = MEDUSA_SEARCH_URI_IS_ALWAYS_TRUE;
                }
                else {
                        result->error = MEDUSA_SEARCH_URI_IS_ALWAYS_FALSE;
                }
        }
        
        return result;
}



static gboolean                     
modified_query_relation_is_invalid   (const char *query_relation)
{
        int i;
        for (i = 0; modified_relations[i] != NULL; i++) {
                if (medusa_str_has_prefix (query_relation, modified_relations[i])) {
                        return FALSE;
                }
        }
        return TRUE;
}

static gboolean                     
modified_query_relation_has_no_upper_bound (const char *query_relation)
{
        return medusa_str_has_prefix (query_relation, "is_after ");
}

static gboolean                     
modified_query_relation_has_no_lower_bound (const char *query_relation)
{
        return medusa_str_has_prefix (query_relation, "is_before ");
}

static char *                       
make_new_criterion_with_time_t (const char *type,
                                const char *relation,
                                time_t object)
{
        return g_strdup_printf ("%s %s %d", type, relation, (int) object);
}

static char *                  
object_to_date (const char *match_string)
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


static time_t                       
first_time_corresponding_to_query_object (const char *query_relation,
                                          const char *object)
{
        char *date;
        time_t relevant_time;
        
        date = object_to_date (object);
        if (medusa_str_has_prefix (query_relation,
                                   "is") ||
            medusa_str_has_prefix (query_relation,
                                   "is_not")) {
                relevant_time = medusa_file_info_get_first_unix_time_occurring_on_date (date);
        }
        else if (medusa_str_has_prefix (query_relation,
                                        "is_after")) {
                relevant_time = medusa_file_info_get_last_unix_time_occurring_on_date (date);
        }
        else if (medusa_str_has_prefix (query_relation,
                                        "is_within_a_week_of")) {
                relevant_time = medusa_file_info_get_unix_time_a_week_before_date (date);
        }
        else if (medusa_str_has_prefix (query_relation,
                                        "is_within_a_month_of")) {
                relevant_time = medusa_file_info_get_unix_time_a_month_before_date (date);
        }                          
        else {
                g_assert_not_reached ();
                relevant_time = 0;
        }
        g_free (date);
        return relevant_time;

}

static time_t                       
last_time_corresponding_to_query_object (const char *query_relation,
                                         const char *object)
{
        char *date;
        time_t relevant_time;

         date = object_to_date (object);
         
         if (medusa_str_has_prefix (query_relation,
                                         "is_before")) {
                 relevant_time = medusa_file_info_get_first_unix_time_occurring_on_date (date);
         }
         else if (medusa_str_has_prefix (query_relation,
                                         "is_within_a_week_of")) {
                 relevant_time = medusa_file_info_get_unix_time_a_week_after_date (date);
         }
         else if (medusa_str_has_prefix (query_relation,
                                         "is_within_a_month_of")) {
                 relevant_time = medusa_file_info_get_unix_time_a_month_after_date (date);
         }                          
         else if (medusa_str_has_prefix (query_relation,
                                         "is") ||
                  medusa_str_has_prefix (query_relation,
                                         "is_not")) {
                 relevant_time = medusa_file_info_get_last_unix_time_occurring_on_date (date);
         }
 
         else {
                  relevant_time = 0;
         }
         g_free (date);
         return relevant_time;
         
}
         
static gboolean                     
can_replace_modified_with_mtime (MedusaUnparsedCriteria *criteria)
{
        int i;
        
        for (i = 0; criteria[i] != NULL; i++) {
                if (medusa_str_has_prefix (criteria[i], "modified ") &&
                    !medusa_str_has_prefix (criteria[i], "modified is_not")) {
                        return TRUE;
                }
        }
        return FALSE;
}

static MedusaOptimizationResult *   
replace_modified_with_mtime_criteria (MedusaUnparsedCriteria *criteria)
 {
         MedusaOptimizationResult *result;
         QueryCriterion *old_criterion;
         QueryCriterion *first_new_criterion, *second_new_criterion;
         QueryCriterion *new_criterion;
         const char *query_relation;  /* The relationship, eg, "is" */
         const char *query_object;    /* The subject, eg "root" */

         g_return_val_if_fail (can_replace_modified_with_mtime (criteria), NULL);
         result = optimization_result_new ();

         old_criterion = query_criterion_get_by_category (criteria,
                                                          "modified");

         query_relation = query_criterion_get_relation (old_criterion);
         query_object = query_criterion_get_object (old_criterion);
         if (modified_query_relation_is_invalid (query_relation) ||
             query_object == NULL) {
                 result->error = MEDUSA_SEARCH_URI_SYNTAX_ERROR;
                 return result;
         }

         if (modified_query_relation_has_no_upper_bound (query_relation)) {
                 new_criterion = make_new_criterion_with_time_t ("mtime",
                                                                 "is_after",
                                                                 last_time_corresponding_to_query_object (query_relation,
                                                                                                           query_object));
                 replace_old_criterion_with_new_and_free (criteria,
                                                          old_criterion,
                                                          new_criterion);
         }
         else if (modified_query_relation_has_no_lower_bound (query_relation)) {
                 new_criterion = make_new_criterion_with_time_t ("mtime",
                                                                 "is_before",
                                                                 first_time_corresponding_to_query_object (query_relation,
                                                                                                          query_object));
                 replace_old_criterion_with_new_and_free (criteria,
                                                          old_criterion,
                                                          new_criterion);
         }                                                       
         else {
                 first_new_criterion = make_new_criterion_with_time_t ("mtime",
                                                                       "is_before",
                                                                       last_time_corresponding_to_query_object (query_relation,
                                                                                                                 query_object));
                 second_new_criterion = make_new_criterion_with_time_t ("mtime",
                                                                        "is_after",
                                                                        first_time_corresponding_to_query_object (query_relation,
                                                                                                                 query_object));
                 replace_old_criterion_with_new_and_free (criteria,
                                                          old_criterion,
                                                          first_new_criterion);
                 criteria = add_new_criterion_and_free (criteria,
                                                        second_new_criterion);
        }

        result->criteria = criteria;
        return result;
                
        
}










