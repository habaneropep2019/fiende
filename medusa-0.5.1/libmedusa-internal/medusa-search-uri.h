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

#ifndef MEDUSA_SEARCH_URI_H
#define MEDUSA_SEARCH_URI_H

#include <sys/types.h>

#include "medusa-master-db.h"
#include "medusa-file-index.h"
#include "medusa-uri-list.h"
#include "medusa-query-clauses.h"

#define MEDUSA_FILE_NAME_CRITERION "file_name"
#define MEDUSA_DIRECTORY_NAME_CRITERION "file_name"
#define MEDUSA_FILE_TYPE_CRITERION "file_type"
#define MEDUSA_MODIFIED_CRITERION "modified"
#define MEDUSA_OWNER_CRITERION "owner"
#define MEDUSA_GROUP_CRITERION "group"
#define MEDUSA_SIZE_CRITERION "size"
#define MEDUSA_READ_PERMISSIONS_CRITERION "permissions_to_read"
#define MEDUSA_CONTENTS_CRITERION "contents"
#define MEDUSA_EMBLEMS_CRITERION "emblems"


#ifndef MEDUSA_PARSED_SEARCH_URI_DEFINED
#define MEDUSA_PARSED_SEARCH_URI_DEFINED
typedef struct MedusaParsedSearchURI MedusaParsedSearchURI;
#endif
typedef struct MedusaParsedSearchCriterion MedusaParsedSearchCriterion;

typedef enum {
        MEDUSA_FILE_ATTRIBUTE_INDEX_RECORD,
        MEDUSA_URI_INDEX_RECORD
} MedusaDatabaseRecordType;

struct MedusaParsedSearchCriterion {
	MedusaQueryFunc query_func;  /* Function that runs a query on a record */
        MedusaResultCheckFunc check_func;  /* Function that checks positive results
                                              to make sure they are still applicable */
        gpointer query_data;
        gboolean free_query_data_when_finished;
        MedusaArgumentType argument_type;
        MedusaQueryArgument argument;
        MedusaDatabaseRecordType database_record_type;

        /* Special options for parsed content queries */
        gboolean is_content_request;
        char *content_request;
        gboolean return_matches;
        gboolean match_all_words;
};



typedef enum {
        MEDUSA_SEARCH_URI_NO_ERROR,
        MEDUSA_SEARCH_URI_IS_ALWAYS_TRUE,
        MEDUSA_SEARCH_URI_IS_ALWAYS_FALSE,
        MEDUSA_SEARCH_URI_SYNTAX_ERROR,
        MEDUSA_SEARCH_URI_OBSOLETES_INDEX,
} MedusaSearchURIError;

struct MedusaParsedSearchURI {
        GList *parsed_search_criteria;
        MedusaSearchURIError error;
};


gboolean                 medusa_uri_is_search_uri                    (const char                *uri);

MedusaParsedSearchURI *  medusa_search_uri_parse                     (const char                *search_uri,
                                                                      MedusaMasterDB            *master_db,
                                                                      uid_t                     uid_of_user_making_query);
void                     medusa_parsed_search_uri_free               (MedusaParsedSearchURI *parsed_search_uri);

/* Add extra criterion for content requests to
   make sure content requests have the appropriate
   security criteria, and size requests only
   return regular files */
char *                   medusa_search_uri_add_extra_needed_criteria (const char          *search_uri,
                                                                      uid_t               uid);

/* To be done when exiting */
void                     medusa_search_uri_clean_up_parse_data       (void);


/* Private methods for things that manipulate parsed search uris */
gboolean                 medusa_parsed_search_criterion_is_content_search     (gpointer             data,
                                                                               gpointer             use_data);
gboolean                 medusa_parsed_search_criterion_has_check             (MedusaParsedSearchCriterion *clause);
gboolean                 medusa_parsed_search_criterion_run_check             (MedusaParsedSearchCriterion *clause,
                                                                               const char         *uri);
void                     medusa_parsed_search_criterion_list_free             (GList *parsed_search_criteria);


#endif /* MEDUSA_SEARCH_URI_H */
