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
 *           Maciej Stachowiak <mjs@eazel.com> 
 *  
 */


/* medusa-search-service.h -- API for externals users of the medusa
   search service to make requests and receive results */

#ifndef MEDUSA_SEARCH_SERVICE_H
#define MEDUSA_SEARCH_SERVICE_H

#include <libgnomevfs/gnome-vfs-types.h>

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

#ifndef SUN_LEN
/* This system is not POSIX.1g.         */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
       + strlen ((ptr)->sun_path))
#endif

typedef struct MedusaSearchServiceConnection MedusaSearchServiceConnection;

typedef enum {
        MEDUSA_SEARCH_METHOD_INDEX_ONLY, /* Search only the index.  Fail if it's not available. */
        MEDUSA_SEARCH_METHOD_INDEX_IF_AVAILABLE, /* Search the index if it's there, otherwise do a slow search */
        MEDUSA_SEARCH_METHOD_FULL_SEARCH,  /* Search the index, then do a slow search to confirm and add to results */
        MEDUSA_SEARCH_METHOD_UNINDEXED_ONLY /* Don't use the index at all */
} MedusaSearchMethod;

typedef enum {
        MEDUSA_SEARCH_OK,
        MEDUSA_SEARCH_ERROR_INVALID_URI,
        MEDUSA_SEARCH_ERROR_SERVICE_UNAVAILABLE,
        MEDUSA_SEARCH_ERROR_SERVICE_BUSY,
        MEDUSA_SEARCH_NO_RESULTS,
        MEDUSA_SEARCH_ALL_TRUE,                    /* Return this if the search would return every result in the database */
        MEDUSA_SEARCH_DATE_TOO_NEW,                /* Return this for an indexed search where the date requirements are
                                                      too new to possibly match any result in the database */
        MEDUSA_SEARCH_EOF
} MedusaSearchResult;

MedusaSearchServiceConnection *medusa_search_service_connection_new (const char *uri,
                                                                     GnomeVFSContext *context,
                                                                     GnomeVFSResult *result);

GnomeVFSResult medusa_search_service_connection_is_available_for_uri       (GnomeVFSURI *uri);


GnomeVFSResult medusa_search_service_connection_start_search       (MedusaSearchServiceConnection *connection);

GnomeVFSResult medusa_search_service_connection_read_search_result (MedusaSearchServiceConnection *connection,
                                                                    GnomeVFSContext *context,
                                                                    char **result);

void           medusa_search_service_connection_destroy            (MedusaSearchServiceConnection *connection);

#endif /* MEDUSA_SEARCH_SERVICE_H */


