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


/* medusa-search-service-private.h -- things that are internal to the
   search service client implementation and need to be shared with the
   search daemon. */

#ifndef MEDUSA_SEARCH_SERVICE_PRIVATE_H
#define MEDUSA_SEARCH_SERVICE_PRIVATE_H


#define SEARCH_SOCKET_PATH  "/tmp/medusa-search-server"
#define COOKIE_PATH "/tmp/medusa-cookies"

/* These are the strings that the client and server use to communicate back 
   and forth about the status of the search.  There are three communications steps:
   1.  Authentication via cookie
   2.  Opening the search uri, and reporting errors during this process
   3.  Reading the search results, and returning errors for this process */


/* Cookie protocol information */
#define COOKIE_REQUEST "gimme cookie"
#define SEARCH_COOKIE_ACKNOWELEDGMENT_TRANSMISSION "Got your cookie"
#define SEARCH_REQUEST_TIMEOUT_ERROR "timeout error"

/* Search URI opened transmissions */
#define SEARCH_URI_OK_TRANSMISSION "Search URI OK"
#define SEARCH_URI_ALL_RESULTS_VALID_TRANSMISSION "Whole Database Matches"
#define SEARCH_URI_NO_RESULTS_TRANSMISSION "Query returned no results"
#define SEARCH_URI_INDEX_OBSOLETE_ERROR_TRANSMISSION "Search URI obsoletes index"
#define SEARCH_URI_SYNTAX_ERROR_TRANSMISSION "Search URI syntax error"


/* Search URI results transmissions */
#define START_SEARCH_REQUEST "Start Search"
#define SEARCH_INDEX_ERROR_TRANSMISSION "Index Error"
#define SEARCH_END_TRANSMISSION "End"


#define MAX_LINE 512
#endif /* MEDUSA_SEARCH_SERVICE_PRIVATE_H */


