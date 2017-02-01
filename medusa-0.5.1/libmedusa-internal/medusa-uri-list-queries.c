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
 *  medusa-uri-list-queries.c  Builds and performs common queries on a list of uri's
 *
 */

#include <glib.h>
#include <sys/types.h>
#include <regex.h>
#include <string.h>
#include <libmedusa/medusa-string.h>
#include "medusa-conf.h"
#include "medusa-rdb-record.h"
#include "medusa-uri-list.h"
#include "medusa-uri-list-queries.h"
#include "medusa-uri-list-encoders.h"

static char *           file_glob_to_regexp         (const char *file_glob);

gboolean
medusa_uri_list_is_in_directory (gpointer database,
                                 gpointer record,
                                 MedusaQueryArgument directory_argument)
{
        MedusaURIList *uri_list;
        MedusaRDBRecord database_record;
        char *directory_in_query, *directory_in_record;
        gboolean result;

        uri_list = (MedusaURIList *) database;
        database_record = (MedusaRDBRecord) record;
        directory_in_query = directory_argument.string;

        directory_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                           medusa_uri_list_get_field_info (uri_list),
                                           MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE,
                                           uri_list,
                                           directory_in_record);
        result = (strcasecmp (directory_in_record, 
                              directory_in_query) == 0);
        g_free (directory_in_record);

        return result;
        
}

gboolean
medusa_uri_list_is_in_directory_tree (gpointer database,
                                      gpointer record,
                                      MedusaQueryArgument directory_argument)
{
        MedusaURIList *uri_list;
        MedusaRDBRecord database_record;
        char *directory_in_query, *directory_in_record;
        gboolean result;

        uri_list = (MedusaURIList *) database;
        database_record = (MedusaRDBRecord) record;
        directory_in_query = directory_argument.string;

        directory_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                           medusa_uri_list_get_field_info (uri_list),
                                           MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE,
                                           database,
                                           directory_in_record);
        result = medusa_strcase_has_prefix (directory_in_record, 
                                            directory_in_query);
        g_free (directory_in_record);

        return result;
        
}

gboolean
medusa_uri_list_is_named (gpointer database,
                          gpointer record,
                          MedusaQueryArgument file_argument)
{
        MedusaURIList *uri_list;
        MedusaRDBRecord database_record;
        char *file_in_query, *file_in_record;
        gboolean result;

        uri_list = (MedusaURIList *) database;
        database_record = (MedusaRDBRecord) record;
        file_in_query = file_argument.string;

        file_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                              medusa_uri_list_get_field_info (uri_list),
                                              MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE,
                                              uri_list,
                                              file_in_record);
        result = (strcasecmp (file_in_record, file_in_query) == 0);
        g_free (file_in_record);

        return result;
        
}

gboolean
medusa_uri_list_has_name_regexp_matching (gpointer database,
                                          gpointer record,
                                          MedusaQueryArgument file_argument)
{
        MedusaURIList *uri_list;
        MedusaRDBRecord database_record;
        char *file_regular_expression, *file_in_record;
        static char *last_pattern = NULL;
        static regex_t *pattern_data = NULL;
        static gboolean regular_expression_is_valid;
        gboolean result;

        uri_list = (MedusaURIList *) database;
        database_record = (MedusaRDBRecord) record;
        file_regular_expression = file_argument.string;

        file_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                              medusa_uri_list_get_field_info (uri_list),
                                              MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE,
                                              uri_list,
                                              file_in_record);

        if (last_pattern == NULL ||
            strcasecmp (last_pattern, file_regular_expression)) {
                g_free (last_pattern);
                g_free (pattern_data);
                last_pattern = g_strdup (file_regular_expression);
                /* FIXME: Free these values at exit */
                pattern_data = g_new0 (regex_t, 1);
                regular_expression_is_valid = (regcomp (pattern_data,
                                                        file_regular_expression,
                                                        REG_ICASE | REG_NOSUB) == 0);
                
        }
        
        result = (regular_expression_is_valid && 
                  regexec (pattern_data, file_in_record, 0, NULL, 0) == 0);
        g_free (file_in_record);
        
        return result;
        
}

gboolean
medusa_uri_list_has_name_not_regexp_matching (gpointer database,
                                              gpointer record,
                                              MedusaQueryArgument file_argument)
{
        return !medusa_uri_list_has_name_regexp_matching (database,
                                                          record,
                                                          file_argument);
}

gboolean
medusa_uri_list_is_in_directory_regexp_matching (gpointer database,
                                                 gpointer record,
                                                 MedusaQueryArgument directory_argument)
{
        MedusaURIList *uri_list;
        MedusaRDBRecord database_record;
        char *directory_regular_expression, *directory_in_record;
        static char *last_pattern = NULL;
        static regex_t *pattern_data = NULL;
        static int regular_expression_is_valid = 0;
        gboolean result;

        uri_list = (MedusaURIList *) database;
        database_record = (MedusaRDBRecord) record;
        directory_regular_expression = directory_argument.string;

        directory_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                           medusa_uri_list_get_field_info (uri_list),
                                           MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE,
                                           uri_list,
                                           directory_in_record);
        
        if (last_pattern == NULL ||
            strcasecmp (last_pattern, directory_regular_expression)) {
                g_free (last_pattern);
                g_free (pattern_data);
                last_pattern = g_strdup (directory_regular_expression);
                /* FIXME: Free these values at exit */
                pattern_data = g_new0 (regex_t, 1);
                regular_expression_is_valid = regcomp (pattern_data,
                                                       directory_regular_expression,
                                                       REG_ICASE | REG_NOSUB);
                
        }
        
        result = (regular_expression_is_valid &&
                  regexec (pattern_data, directory_in_record, 0, NULL, 0) == 0);
        g_free (directory_in_record);
        
        return result;
        
}


gboolean
medusa_uri_list_is_not_in_directory_regexp_matching (gpointer database,
                                                     gpointer record,
                                                     MedusaQueryArgument directory_argument)
{
        return !medusa_uri_list_is_in_directory_regexp_matching (database,
                                                                 record,
                                                                 directory_argument);
}

gboolean
medusa_uri_list_has_name_containing (gpointer database,
                                     gpointer record,
                                     MedusaQueryArgument file_argument)
{
        MedusaURIList *uri_list;
        MedusaRDBRecord database_record;
        char *file_fragment_in_query, *file_in_record;
        gboolean result;

        uri_list = (MedusaURIList *) database;
        database_record = (MedusaRDBRecord) record;
        file_fragment_in_query = file_argument.string;

        file_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                           medusa_uri_list_get_field_info (uri_list),
                                           MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE,
                                           uri_list,
                                           file_in_record);
        result = medusa_strstr_case_insensitive (file_in_record, 
                                                 file_fragment_in_query);
        g_free (file_in_record);

        return result;
}

gboolean
medusa_uri_list_has_name_not_containing (gpointer database,
                                         gpointer record,
                                         MedusaQueryArgument file_argument)
{
        return !medusa_uri_list_has_name_containing (database,
                                                     record,
                                                     file_argument);
}

gboolean
medusa_uri_list_is_in_directory_containing (gpointer database,
                                            gpointer record,
                                            MedusaQueryArgument directory_argument)
{
        MedusaURIList *uri_list;
        MedusaRDBRecord database_record;
        char *directory_fragment_in_query, *directory_in_record;
        gboolean result;

        uri_list = (MedusaURIList *) database;
        database_record = (MedusaRDBRecord) record;
        directory_fragment_in_query = directory_argument.string;

        directory_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                              medusa_uri_list_get_field_info (uri_list),
                                              MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE,
                                              uri_list,
                                              directory_in_record);
        result = medusa_strstr_case_insensitive (directory_in_record, 
                                                 directory_fragment_in_query);
        g_free (directory_in_record);

        return result;
}

gboolean
medusa_uri_list_has_file_name_starting_with (gpointer database,
                                             gpointer record,
                                             MedusaQueryArgument file_argument)
{
        MedusaURIList *uri_list;
        MedusaRDBRecord database_record;
        char *file_prefix_in_query, *file_in_record;
        gboolean result;

        uri_list = (MedusaURIList *) database;
        database_record = (MedusaRDBRecord) record;
        file_prefix_in_query = file_argument.string;

        file_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                              medusa_uri_list_get_field_info (uri_list),
                                              MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE,
                                              uri_list,
                                              file_in_record);
        result = medusa_strcase_has_prefix (file_in_record, 
                                            file_prefix_in_query);
        g_free (file_in_record);

        return result;
        
}


gboolean
medusa_uri_list_has_file_name_ending_with (gpointer database,
                                           gpointer record,
                                           MedusaQueryArgument file_argument)
{
        MedusaURIList *uri_list;
        MedusaRDBRecord database_record;
        char *file_suffix_in_query, *file_in_record;
        gboolean result;

        uri_list = (MedusaURIList *) database;
        database_record = (MedusaRDBRecord) record;
        file_suffix_in_query = file_argument.string;

        file_in_record = g_new0 (char, FILE_NAME_MAXIMUM_LENGTH);
        medusa_rdb_record_get_field_value (database_record,
                                           medusa_uri_list_get_field_info (uri_list),
                                           MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE,
                                           uri_list,
                                           file_in_record);
        result = medusa_strcase_has_suffix (file_in_record, 
                                            file_suffix_in_query);
        g_free (file_in_record);

        return result;
        
}

gboolean
medusa_uri_list_has_name_glob_matching (gpointer database,
                                        gpointer record,
                                        MedusaQueryArgument file_glob_argument)
{
        MedusaQueryArgument pattern;
        char *file_glob;
        gboolean result;

        file_glob = file_glob_argument.string;
        pattern.string = file_glob_to_regexp (file_glob);
        
        result = medusa_uri_list_has_name_regexp_matching  (database,
                                                            record, 
                                                            pattern);
        g_free (pattern.string);
        return result;
}

gboolean
medusa_uri_list_has_full_file_name (gpointer database,
                                    gpointer record,
                                    MedusaQueryArgument full_path_argument)
{
        char *full_path;
        MedusaQueryArgument file_name, directory_name;
        gboolean result;
        
        full_path = full_path_argument.string;
        file_name.string = &full_path[strlen (full_path) - 1];
        while (*file_name.string != '/') {
                file_name.string--;
        }
        file_name.string++;

        directory_name.string = g_new0 (char, (int) (file_name.string - full_path));
        strncpy (directory_name.string, full_path, (int) (file_name.string - full_path));
        directory_name.string[(int) (file_name.string - full_path)] = 0;

        result = medusa_uri_list_is_in_directory (database,
                                                  record,
                                                  directory_name) &&
                medusa_uri_list_is_named (database,
                                          record,
                                          file_name);
        g_free (directory_name.string);
                
        return result;

        

}
                                


static char *
file_glob_to_regexp (const char *file_glob)
{
        char *literal, *literal_loc;

        literal = g_new0 (char, 4 * strlen (file_glob) + 3);
        /* Iterate through the file_glob, quoting '.' and changing * to .* */
        /* The cool bracket hack was the idea of Mike Engber <engber@eazel.com> */
        literal_loc = literal;
        *literal_loc++ = '^';
        /* FIXME: [] and {} operators don't work using this function */
        while (*file_glob) {
                if (*file_glob == '.') {
                        *literal_loc++ = '\\';
                        *literal_loc++ = '.';
                }
                else if (*file_glob == '*') {
                        *literal_loc++ = '.';
                        *literal_loc++ = '.';
                        *literal_loc++ = '*';
                }
                else if (*file_glob == '^') {
                        *literal_loc++ = '\\';
                        *literal_loc++ = '^';
                }
                else if (*file_glob == '?') {
                        *literal_loc++ = '.';
                }
                else {                
                        *literal_loc++ = '[';
                        *literal_loc++ = *file_glob;
                        *literal_loc++ = ']';
                }
                file_glob++;
        }
        *literal_loc++ = '$';
        return literal;
}


