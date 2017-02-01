/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 * 
 *  medusa.h: general header for programs that link with medusa
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

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-cancellation.h>
#include <libgnomevfs/gnome-vfs-context.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libmedusa/medusa-indexed-search.h>
#include <libmedusa/medusa-search-service.h>
#include <libmedusa/medusa-unindexed-search.h>
#include <libmedusa/medusa-search-service-private.h>
#include <libmedusa/medusa-service-private.h>
#include <libmedusa/medusa-string.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

typedef enum {
        SEARCH_MODE_UNINDEXED,
        SEARCH_MODE_INDEXED,
        SEARCH_MODE_INDEXED_AND_UNINDEXED
} SearchMode;

struct MedusaSearchServiceConnection {
        char *uri;
        MedusaIndexedSearch *indexed_search;
        MedusaUnindexedSearch *unindexed_search;
        MedusaSearchMethod method;
        SearchMode mode;
        gboolean finished_reading_indexed_results;
};

static gboolean               uri_has_valid_header                    (const char *uri);
static gboolean               vfs_uri_to_search_method                (GnomeVFSURI *uri,
                                                                       MedusaSearchMethod *method);
static gboolean               uri_to_search_method                    (const char *uri,
                                                                       MedusaSearchMethod *method);

static char *                 uri_to_string_remove_extra_slashes      (const GnomeVFSURI *uri);
GnomeVFSResult
medusa_search_service_connection_is_available_for_uri (GnomeVFSURI *uri)
{
        MedusaSearchMethod method;
        gboolean uri_looks_valid;

        uri_looks_valid = vfs_uri_to_search_method (uri,
                                                    &method);
        if (!uri_looks_valid) {
                return GNOME_VFS_ERROR_INVALID_URI;
        }
        if (method == MEDUSA_SEARCH_METHOD_INDEX_ONLY) {
                return medusa_indexed_search_is_available ();
        }
        else if (method == MEDUSA_SEARCH_METHOD_UNINDEXED_ONLY) {
                /* We can do an unindexed search only if there are no searches for content */
                return medusa_unindexed_search_is_available_for_uri (uri->text);
        }
        
        /* otherwise, a search is available if either indexed or unindexed search
           is available */
        if (medusa_indexed_search_is_available () == GNOME_VFS_OK) {
                return GNOME_VFS_OK;
        }
        return medusa_unindexed_search_is_available_for_uri (uri->text);
        
}


MedusaSearchServiceConnection *
medusa_search_service_connection_new (const char *uri,
                                      GnomeVFSContext *context,
                                      GnomeVFSResult *result)
{
        MedusaSearchServiceConnection *connection;
        GnomeVFSResult indexed_search_result, unindexed_search_result;
        gboolean uri_looks_valid;

        if (!uri_has_valid_header (uri)) {
                *result = GNOME_VFS_ERROR_INVALID_URI;
                return NULL;
        }

        connection = g_new0 (MedusaSearchServiceConnection, 1);
        connection->uri = g_strdup (uri);
        uri_looks_valid = uri_to_search_method (uri,
                                                &connection->method);
        if (!uri_looks_valid) {
                g_free (connection->uri);
                g_free (connection);
                *result = GNOME_VFS_ERROR_INVALID_URI;
                return NULL;
        }
        if (connection->method == MEDUSA_SEARCH_METHOD_INDEX_ONLY) {
                connection->indexed_search = medusa_indexed_search_new (result, context, uri);
                connection->unindexed_search = NULL;
                if (*result == GNOME_VFS_OK) {
                        connection->mode = SEARCH_MODE_INDEXED;
                        return connection;
                }
                else {
                        g_free (connection->uri);
                        g_free (connection);
                        return NULL;
                }
        }
        if (connection->method == MEDUSA_SEARCH_METHOD_INDEX_IF_AVAILABLE) {
                connection->indexed_search = medusa_indexed_search_new (result, context, uri);
                if (*result != GNOME_VFS_OK) {
                        connection->unindexed_search = medusa_unindexed_search_new (result, uri);
                        if (*result == GNOME_VFS_OK) {
                                connection->mode = SEARCH_MODE_UNINDEXED;
                        }
                        else {
                                g_free (connection->uri);
                                g_free (connection);
                                return NULL;
                        }
                }
                else {
                        connection->unindexed_search = NULL;
                        connection->mode = SEARCH_MODE_INDEXED;
                }
                return connection;
        }
        if (connection->method == MEDUSA_SEARCH_METHOD_FULL_SEARCH) {

                connection->indexed_search = medusa_indexed_search_new (&indexed_search_result, context, uri);
                connection->unindexed_search = medusa_unindexed_search_new (&unindexed_search_result, uri);
                if (indexed_search_result == GNOME_VFS_OK &&
                    unindexed_search_result == GNOME_VFS_OK) {
                        connection->mode = SEARCH_MODE_INDEXED_AND_UNINDEXED;
                        *result = GNOME_VFS_OK;

                }
                else if (unindexed_search_result == GNOME_VFS_OK) {
                        connection->mode = SEARCH_MODE_UNINDEXED;
                        *result = GNOME_VFS_OK;
                }
                else if (indexed_search_result == GNOME_VFS_OK) {
                        connection->mode = SEARCH_MODE_INDEXED;
                        *result = GNOME_VFS_OK;
                }
                else {
                        g_free (connection->uri);
                        g_free (connection);
                        *result = indexed_search_result;
                        return NULL;
                }
                return connection;
        }
        if (connection->method == MEDUSA_SEARCH_METHOD_UNINDEXED_ONLY) {
                connection->indexed_search = NULL;
                connection->unindexed_search = medusa_unindexed_search_new (result, uri);
                if (*result == GNOME_VFS_OK) {
                        connection->mode = SEARCH_MODE_UNINDEXED;
                        return connection;
                }
                else {
                        g_free (connection->uri);
                        g_free (connection);
                        return NULL;
                }
        }

        g_assert_not_reached ();
        return NULL;
}


GnomeVFSResult           
medusa_search_service_connection_start_search (MedusaSearchServiceConnection *connection)
{
        GnomeVFSResult result;

	result = GNOME_VFS_OK;
        if (connection->mode == SEARCH_MODE_INDEXED ||
            connection->mode == SEARCH_MODE_INDEXED_AND_UNINDEXED) {
                result = medusa_indexed_search_start_search (connection->indexed_search,
                                                             connection->uri);
        }

        return result;
}


GnomeVFSResult 
medusa_search_service_connection_read_search_result (MedusaSearchServiceConnection *connection,
                                                     GnomeVFSContext *context,
                                                     char **result)
{
        GnomeVFSResult read_result;

        if (gnome_vfs_context_check_cancellation (context)) {
                medusa_search_service_connection_destroy (connection);
                return GNOME_VFS_ERROR_CANCELLED;
        }
        if (connection->mode == SEARCH_MODE_INDEXED ||
            (connection->mode == SEARCH_MODE_INDEXED_AND_UNINDEXED &&
             !connection->finished_reading_indexed_results)) {
                read_result = medusa_indexed_search_read_search_result (connection->indexed_search,
                                                                        context,
                                                                        result);
                /* If we're doing a full search and we're out of 
                   results, switch gears into an unindexed search */
                if (read_result == GNOME_VFS_ERROR_EOF &&
                    connection->mode == SEARCH_MODE_INDEXED_AND_UNINDEXED) {
                        connection->finished_reading_indexed_results = TRUE;
                        return medusa_search_service_connection_read_search_result (connection, 
                                                                                    context,
                                                                                    result);
                } 
        }
        else {
                read_result = medusa_unindexed_search_read_search_result (connection->unindexed_search,
                                                                          context,
                                                                          result);
        }
        
        return read_result;
}



void 
medusa_search_service_connection_destroy (MedusaSearchServiceConnection *connection)
{
        g_free (connection->uri);
        medusa_indexed_search_destroy (connection->indexed_search);
        medusa_unindexed_search_destroy (connection->unindexed_search);
        g_free (connection);
}

static gboolean               
uri_has_valid_header (const char *uri)
{
        const char *method_string;

        if (!(medusa_str_has_prefix (uri,"search:") ||
              medusa_str_has_prefix (uri,"gnome-search:") ||
              medusa_str_has_prefix (uri,"medusa:"))) {
                return FALSE;
        }
        
        method_string = strchr (uri, ':') + 1;
        if (!(medusa_str_has_prefix (method_string, "index-only") ||
              medusa_str_has_prefix (method_string, "index-if-available") ||
              medusa_str_has_prefix (method_string, "index-with-backup") ||
              medusa_str_has_prefix (method_string, "unindexed-only") ||
              medusa_str_has_prefix (method_string, "[file:///]"))) {
                return FALSE;
        }

        return TRUE;
}

static gboolean
vfs_uri_to_search_method (GnomeVFSURI *gnome_vfs_uri,
                          MedusaSearchMethod *method)
{
        char *uri;
        char *unescaped_uri;
        gboolean uri_looks_valid;

        uri = uri_to_string_remove_extra_slashes (gnome_vfs_uri);
        unescaped_uri = gnome_vfs_unescape_string (uri, NULL);

        uri_looks_valid = uri_to_search_method (unescaped_uri,
                                                method);
        g_free (unescaped_uri);
        return uri_looks_valid;

}

static gboolean
uri_to_search_method (const char *uri,
                      MedusaSearchMethod *search_method)
{
        const char *method_string;
        
        /* We should assert these, because we checked that the uri was valid
           before sending it here */
        g_assert (medusa_str_has_prefix (uri, "search:") ||
                  medusa_str_has_prefix (uri, "gnome-search:") ||
                  medusa_str_has_prefix (uri, "medusa:"));
        method_string = strchr (uri, ':') + 1;

        /* The uri is too short, return FALSE, as it cannot possibly be
           valid */
        if (*method_string == 0) {
                return FALSE;
        }
        if (medusa_str_has_prefix (method_string, "index-only")) {
                *search_method = MEDUSA_SEARCH_METHOD_INDEX_ONLY;
        }
        else if (medusa_str_has_prefix (method_string, "index-if-available[")) {
                *search_method = MEDUSA_SEARCH_METHOD_INDEX_IF_AVAILABLE;
                return strchr (method_string, ']') != NULL;
        }
        else if (medusa_str_has_prefix (method_string, "index-with-backup[")) {
                *search_method = MEDUSA_SEARCH_METHOD_FULL_SEARCH;
                return strchr (method_string, ']') != NULL;
        } 
        else if (medusa_str_has_prefix (method_string, "unindexed-only[")) {
                *search_method = MEDUSA_SEARCH_METHOD_UNINDEXED_ONLY;
                return strchr (method_string, ']') != NULL;
        }
        /* Check also for the case of no stated method, and return the default */
        else if (*method_string == '[') {
                *search_method = MEDUSA_SEARCH_METHOD_INDEX_ONLY;
                return strchr (method_string, ']') != NULL;
        }
        else {
                return FALSE;
        }
        return TRUE;
}


/* FIXME bugzilla.eazel.com 2615: 
   This function works around a problem in gnome-vfs where you
 * can't get the original URI without added slashes. That problem
 * should be fixed and then this can be removed.
 */
static char *
uri_to_string_remove_extra_slashes (const GnomeVFSURI *uri)
{
	char *uri_text, *past_colon, *result;

	/* Remove the "//" after the ":" in this URI.
	 * It's safe to assume there there is a ":".
	 */
	uri_text = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	past_colon = strchr (uri_text, ':') + 1;
	if (strncmp (past_colon, "//", 2) == 0) {
		result = g_new (char, strlen (uri_text) - 2 + 1);
		memcpy (result, uri_text, past_colon - uri_text);
		strcpy (result + (past_colon - uri_text), past_colon + 2);
		g_free (uri_text);
	} else {
		result = uri_text;
	}

	return result;
}
