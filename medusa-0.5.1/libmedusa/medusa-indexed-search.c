/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 * 
 *  medusa-indexed-search.c -- Connect to the medusa search daemon and 
 *  request results
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
 *  Authors: Maciej Stachowiak <mjs@eazel.com> 
 *  
 */

#include <config.h>
#include "medusa-indexed-search.h"
#include "medusa-search-service-private.h"
#include "medusa-service-private.h"
#include "medusa-string.h"
#include "medusa-system-state.h"
#include "medusa-utils.h"
#include <errno.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-context.h>
#include <libgnomevfs/gnome-vfs-cancellation.h>
#ifdef DEBUG_INDEXED_SEARCH_SERVICE
#include <libgnomevfs/gnome-vfs-result.h>
#endif

struct MedusaIndexedSearch {
        int socket_fd;
        guint cookie;

        gboolean busy;
        /* Used by searches that on opening we determine have
           no valid results, like matching a nonexistent username */
        gboolean search_is_empty;
        char *buffer;
};


static int
read_cancellable (int file_descriptor, void *buffer,
                  size_t size, GnomeVFSContext *context)
{
        int read_length;

        do {
                read_length = read (file_descriptor, buffer, size);
                usleep (200);
        } while (read_length == -1
                 && errno == EAGAIN
                 && !gnome_vfs_context_check_cancellation (context));        

        return read_length;
}


static char *
get_search_socket_name (void)
{
        char *index_name;

        index_name = getenv ("MEDUSA_INDEX_NAME");
        if (index_name) {
                return g_strdup_printf ("%s-%s", SEARCH_SOCKET_PATH, index_name);
        }
        else {
                return g_strdup (SEARCH_SOCKET_PATH);
        }

}
        

static GnomeVFSResult
authenticate_connection (MedusaIndexedSearch *connection) {
	char *request;
        char *file_name;
        char acknowledgement_buffer[13];
        int cookie_fd, key;
        int read_result, write_result;

	/* Send request for cookie */
	request = g_strdup_printf ("%s\t%lu\t%lu\n", COOKIE_REQUEST, (unsigned long) getuid (), (unsigned long) getpid());
	write_result = write (connection->socket_fd, request, strlen (request));
        g_free (request);

        if (write_result == -1) {
#ifdef DEBUG_INDEXED_SEARCH_SERVICE
                g_print ("Writing cookie request to the search socket failed\n");
#endif
                return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
        }

        read_result = read_cancellable (connection->socket_fd, acknowledgement_buffer, 12, NULL);

        if (read_result == -1) {
#ifdef DEBUG_INDEXED_SEARCH_SERVICE
                g_print ("Reading response to the cookie request from the search socket failed\n");
#endif
                return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
        }

        if (medusa_str_has_prefix (acknowledgement_buffer, SEARCH_INDEX_ERROR_TRANSMISSION) ||
            medusa_str_has_prefix (acknowledgement_buffer, SEARCH_REQUEST_TIMEOUT_ERROR)) {
#ifdef DEBUG_INDEXED_SEARCH_SERVICE
                g_print ("The search service encountered an error while attempting to do a search.  The error was : %s\n",
                         acknowledgement_buffer);
#endif

                return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
        }


        /* Go look for cookie */
        file_name = g_strdup_printf ("%s/%lu_%lu", COOKIE_PATH, (unsigned long) getuid (), (unsigned long) getpid ());
        
        cookie_fd = open (file_name, O_RDONLY);
        
        /* FIXME bugzilla.eazel.com 3000: 
           maybe have search daemon send response once cookie
           file is created instead of this loop */
        
        /* Keep looking if cookie file isn't created yet */
        while (cookie_fd == -1) {
                cookie_fd = open (file_name, O_RDONLY);
                usleep (200);
        }
        
        g_free (file_name);
        
        read (cookie_fd, &key, sizeof (int));
        close (cookie_fd);
  
        connection->cookie = key;

        return GNOME_VFS_OK;
}

static void
initialize_connection_buffer (MedusaIndexedSearch *connection)
{
        g_assert (connection->buffer == NULL);
        
        connection->buffer = g_strdup ("");
}



static gboolean
refresh_read_buffer_if_needed (MedusaIndexedSearch *connection,
                               GnomeVFSContext *context)
{
        char tmpbuf[513];
        char *new_buffer;
        int len;
        

        g_assert (connection->buffer != NULL);

        while (strchr (connection->buffer, '\n') == NULL) {
                len = read_cancellable (connection->socket_fd, &tmpbuf, 512 * sizeof (char),
                                        context);
                if (len == -1) {
                        g_free (connection->buffer);
                        connection->buffer = NULL;
                        return FALSE;
                }
#ifdef DEBUG_SEARCH_API
                g_print ("read from socket %s: %d bytes\n", tmpbuf, len);
#endif
                
                tmpbuf[len] = '\0';
                new_buffer = g_strconcat (connection->buffer, tmpbuf, NULL);
                g_free (connection->buffer);
                connection->buffer = new_buffer;
        }

        return TRUE;
}

static char *
read_connection_line (MedusaIndexedSearch *connection,
                      GnomeVFSContext *context)
{
        const char *newline_pos;
        char *result_uri;
        char *new_buffer;

        if (refresh_read_buffer_if_needed (connection,
                                           context) == FALSE) {
                return NULL;
        }

        newline_pos = strchr (connection->buffer, '\n');
        g_assert (newline_pos != NULL);
        result_uri = g_strndup (connection->buffer, newline_pos - connection->buffer);

        /* move buffer forward */
        if (newline_pos + 1 < connection->buffer + strlen (connection->buffer)) {
                new_buffer = g_strdup (newline_pos + 1);
                g_free (connection->buffer);
                connection->buffer = new_buffer;
        }
        else {
                connection->buffer = g_strdup ("");
        }

        return result_uri;
}


static GnomeVFSResult
get_search_uri_parsing_result (MedusaIndexedSearch *connection) 
{
        char *search_daemon_return;

        search_daemon_return = read_connection_line (connection,
                                                     NULL);
        /* Connection was closed */
        if (search_daemon_return == NULL) {
                return GNOME_VFS_ERROR_INTERRUPTED;
        }
        if (strcmp (search_daemon_return, SEARCH_URI_OK_TRANSMISSION) == 0) {
                return GNOME_VFS_OK;
        }
        if (strcmp (search_daemon_return, SEARCH_URI_INDEX_OBSOLETE_ERROR_TRANSMISSION) == 0) {
                return GNOME_VFS_ERROR_SERVICE_OBSOLETE;
        }
        if (strcmp (search_daemon_return, SEARCH_URI_SYNTAX_ERROR_TRANSMISSION) == 0) {
                return GNOME_VFS_ERROR_INVALID_URI;
        }
        if (strcmp (search_daemon_return, SEARCH_URI_ALL_RESULTS_VALID_TRANSMISSION) == 0) {
                return GNOME_VFS_ERROR_TOO_BIG;
        }
        if (strcmp (search_daemon_return, SEARCH_URI_NO_RESULTS_TRANSMISSION) == 0) {
                /* We're done in this case */
                connection->search_is_empty = TRUE;
                return GNOME_VFS_OK;
        }
        g_warning ("Unknown search uri return code %s\n", search_daemon_return);
        return GNOME_VFS_ERROR_INTERNAL;

}

GnomeVFSResult
medusa_indexed_search_is_available ()
{
        char *search_socket_path;
        int socket_fd;

        search_socket_path = get_search_socket_name ();
        socket_fd = medusa_initialize_socket_for_requests (search_socket_path);
        g_free (search_socket_path);
        if (socket_fd == -1) {
                return GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
        }
        else {
                return GNOME_VFS_OK;
        }
}

gboolean
medusa_indexed_search_system_index_files_look_available (void)
{
        char *uri_list_filename, *text_index_location_filename;
        gboolean index_files_exist;

        /* FIXME: We should check that all of the index files are 
           available, not just two. We should also have a place
           to get the list of index file names from somewhere, rather than
           having a second copy of the name here.  */
        uri_list_filename = g_strdup_printf ("%s/%s", MEDUSA_LOCALSTATEDIR,
					     "uri-list");
        text_index_location_filename = g_strdup_printf ("%s/%s",
			MEDUSA_LOCALSTATEDIR, "text-index-location-file");

        index_files_exist = (access (uri_list_filename, F_OK) == 0) &&
                (access (text_index_location_filename, F_OK) == 0);
        g_free (uri_list_filename);
        g_free (text_index_location_filename);
        
        return index_files_exist;
}

static void
send_search_uri (MedusaIndexedSearch *connection, 
                     const char *search_uri)
{
        char *request;

        request = g_strdup_printf ("%lu %lu %d\t%s", (unsigned long) getuid(), (unsigned long) getpid(), connection->cookie, search_uri);

#ifdef DEBUG_SEARCH_API
        printf ("about to send %s\n", request);
#endif

        /* FIXME bugzilla.eazel.com 2999: check error code */
        write (connection->socket_fd, request, strlen(request));

        g_free (request);

}



MedusaIndexedSearch *
medusa_indexed_search_new (GnomeVFSResult *result,
                           GnomeVFSContext *context,
                           const char *uri)
{
        MedusaIndexedSearch *connection;
        char *search_socket_path;

        if (medusa_system_services_are_enabled () == FALSE) {
                *result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
#ifdef DEBUG_INDEXED_SEARCH_SERVICE
                g_print ("Medusa services are not enabled, returning service not available\n");
#endif
                return NULL;
        }
        
        connection = g_new0 (MedusaIndexedSearch, 1);

        search_socket_path = get_search_socket_name ();
        connection->socket_fd = medusa_initialize_socket_for_requests (search_socket_path);        
        /* Set this to nonblocking so that we can check for cancellation */
        fcntl (connection->socket_fd, F_SETFL, O_NONBLOCK);
        g_free (search_socket_path);


        if (connection->socket_fd == -1) {
#ifdef DEBUG_INDEXED_SEARCH_SERVICE
                g_print ("Couldn't connect to search socket.  Service isn't available\n");
#endif
                *result = GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE;
                return NULL;
        }


        *result = authenticate_connection (connection);
#ifdef DEBUG_INDEXED_SEARCH_SERVICE
        g_print ("Result for authenticating connection with search daemon was : %s\n",
                 gnome_vfs_result_to_string (*result));
#endif
        if (*result != GNOME_VFS_OK) {
                g_free (connection);
                return NULL;
        }
        initialize_connection_buffer (connection);
        send_search_uri (connection, uri);
        if (refresh_read_buffer_if_needed (connection,
                                           context) == FALSE) {
                g_free (connection);
                *result = GNOME_VFS_ERROR_INTERRUPTED;
                return NULL;
        }        
        if (*result == GNOME_VFS_OK) {
                *result = get_search_uri_parsing_result (connection);
#ifdef DEBUG_INDEXED_SEARCH_SERVICE
                g_print ("Result for search parsing was %s\n", 
                         gnome_vfs_result_to_string (*result));
#endif
                if (*result != GNOME_VFS_OK) {
                        medusa_indexed_search_destroy (connection);
                        return NULL;
                }
        }

        return connection;
}




GnomeVFSResult           
medusa_indexed_search_start_search (MedusaIndexedSearch *connection,
				    const char *search_uri)
{
        g_return_val_if_fail (connection != NULL, GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE);
        if (connection->busy) {
                return GNOME_VFS_ERROR_IN_PROGRESS;
        } else {
                connection->busy = TRUE;
                return GNOME_VFS_OK;
        }
}


GnomeVFSResult 
medusa_indexed_search_read_search_result (MedusaIndexedSearch *connection,
                                          GnomeVFSContext *context,
                                          char **result)
{
        char *result_uri;

        g_return_val_if_fail (connection != NULL, GNOME_VFS_ERROR_INTERNAL);
        if (connection->search_is_empty) {
                *result = NULL;
                return GNOME_VFS_ERROR_EOF;
        }

        if (!connection->busy) {
                *result = NULL;
                return GNOME_VFS_ERROR_EOF;
        }

        result_uri = read_connection_line (connection,
                                           context);

        if (result_uri == NULL) {
                return GNOME_VFS_ERROR_CANCELLED;
        }

        if (strcmp (result_uri, SEARCH_END_TRANSMISSION) == 0) {
                g_free (result_uri);
                *result = NULL;
                connection->busy = FALSE;
                return GNOME_VFS_ERROR_EOF;
        } else {
                *result = result_uri;
                return GNOME_VFS_OK;
        }
}



void medusa_indexed_search_destroy (MedusaIndexedSearch *connection)
{
        if (connection == NULL) {
                return;
        }
        close (connection->socket_fd);
        g_free (connection->buffer);
        g_free (connection);
}
