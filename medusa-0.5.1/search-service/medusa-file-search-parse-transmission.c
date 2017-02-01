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

#include <glib.h>
#include <stdio.h>
#include <unistd.h>

#include "medusa-file-search-parse-transmission.h"
#include "medusa-authenticate.h"
#include "medusa-search-uri.h"
#include "medusa-master-db.h"
#include "medusa-search-service-private.h"
#include "medusa-utils.h"

#include <string.h>


static void                 handle_authenticated_request     (MedusaMasterDB *master_db,
                                                              const char *search_uri,
                                                              int uid, 
                                                              int client_fd);
static char *               get_uid_pid_key_information      (char *line,
                                                              int *uid, 
                                                              int *pid, 
                                                              int *key);


/* Filter the results of the query to what the
   user should be able to access, and send them back to the sender */
static void                 present_query_results             (GList *results,
                                                               int client_fd,
                                                               int uid);
static char *               goto_next_line                     (char *line);
/* Free entries from the read cache hash table */
static void                 free_entries                       (gpointer key,
                                                                gpointer value,
                                                                gpointer user_data);
static gboolean             another_transmission_line_exists   (char *line);
static void                 send_cookie_creation_acknowledgement (int client_fd);



void
medusa_file_search_parse_transmission (char *transmission, int client_fd,
				       MedusaMasterDB *db)
{
        char *line, *error_string;
        int uid, pid, key;
        
        g_assert (transmission != NULL);
        if (strlen (transmission) == 0) {
                return;
        }

        line = transmission;
        while (another_transmission_line_exists (line)) {
#ifdef SEARCH_DAEMON_DEBUG
                printf ("Received %s\n", line);
#endif
                if (medusa_authenticate_is_cookie_request (line)) {
                        medusa_authenticate_create_cookie_file (line); 
                        send_cookie_creation_acknowledgement (client_fd);
                } else  {
                        line = get_uid_pid_key_information (line, &uid, &pid, &key);
                        if (line == NULL) {
                                medusa_log_event ("Invalid search request transmission\n",
                                                  MEDUSA_DB_LOG_ERRORS);
                                return;
                        }
                        if (medusa_authenticate_is_correct_cookie (uid, pid, key)) {
#ifdef SEARCH_DAEMON_DEBUG
                                printf ("Key %d matched for uid %d and pid %d\n", key, uid, pid);
#endif
                                handle_authenticated_request (db, line, uid, client_fd);
                        }
                        else {
                                error_string = g_strdup_printf ("Invalid authentication received from user claiming to have uid %d", uid);
                                medusa_log_event (error_string, MEDUSA_DB_LOG_ERRORS);
                                g_free (error_string);
                        }
                }
                line = goto_next_line (line);
        }
}  

static gboolean
transmit_string (const char *string,
                 int fd)
{
        int write_result;

        g_assert (string != NULL);
        write_result = write (fd, string, strlen (string));
        if (write_result != strlen (string)) {
                return FALSE;
        }
        write_result = write (fd, "\n", 1);
        if (write_result != 1) {
                return FALSE;
        }
        
        return TRUE;
                
}

static gboolean
send_back_parsing_results (MedusaParsedSearchURI *parsed_search_uri,
                           int client_fd)
{
        switch (parsed_search_uri->error) {

        case MEDUSA_SEARCH_URI_NO_ERROR:
                transmit_string (SEARCH_URI_OK_TRANSMISSION, client_fd);
                return TRUE;
        case MEDUSA_SEARCH_URI_IS_ALWAYS_TRUE:
                transmit_string (SEARCH_URI_ALL_RESULTS_VALID_TRANSMISSION, client_fd);
                return FALSE;
        case MEDUSA_SEARCH_URI_IS_ALWAYS_FALSE:
                transmit_string (SEARCH_URI_NO_RESULTS_TRANSMISSION, client_fd);
                return FALSE;
        case MEDUSA_SEARCH_URI_OBSOLETES_INDEX:
                transmit_string (SEARCH_URI_INDEX_OBSOLETE_ERROR_TRANSMISSION, client_fd);
                return FALSE;
        case MEDUSA_SEARCH_URI_SYNTAX_ERROR:
                transmit_string (SEARCH_URI_SYNTAX_ERROR_TRANSMISSION, client_fd);
                return FALSE;
        default:
                g_assert_not_reached ();
                return FALSE;
        }
}
                           

static void
handle_authenticated_request (MedusaMasterDB *db, 
                              const char *search_uri,
                              int uid,
                              int client_fd) 
{
        MedusaParsedSearchURI *parsed_search_uri;
        GList *results;
        char *search_uri_to_send;
        
        printf ("Running query\n");
        /* Do initial parsing of uri, and send back errors that occur during 
           this process */
        /* Do preprocessing on the search uri, to add extra permission 
           or file type checks, if relevant */
        /* FIXME bugzilla.eazel.com 4887: This should be a query "optimization" and not here */
        search_uri_to_send = medusa_search_uri_add_extra_needed_criteria (search_uri,
                                                                          uid);

        parsed_search_uri = medusa_search_uri_parse (search_uri_to_send, db, uid);
        if (!send_back_parsing_results (parsed_search_uri,
                                        client_fd)) {
                g_free (search_uri_to_send);
                medusa_parsed_search_uri_free (parsed_search_uri);
                return;
        }
        g_free (search_uri_to_send);        
        results = medusa_master_db_run_search (db, parsed_search_uri);
        medusa_parsed_search_uri_free (parsed_search_uri);
        printf ("Done running query\n");
        present_query_results (results, client_fd, uid);

        
}


static char *
get_uid_pid_key_information (char *line, int *uid, int *pid, int *key)
{
        char *result;
        sscanf (line, "%d %d %d\t", uid, pid, key);
        result = strchr (line,'\t');
        if (result == NULL) {
                return result;
        } else {
                /* return the next meaningful character, rather than the tab */
                return result+1;
        }
}

static void
present_query_results (GList *results,
		       int client_fd, 
		       int uid)
{
        GList *iterator;
        GHashTable *read_cache;
        char send_buffer[MAX_LINE + 1];

#ifdef PRESENT_QUERY_RESULTS_DEBUG
        g_print ("Presenting query results\n");
#endif
        read_cache = g_hash_table_new (g_str_hash, g_str_equal);
        for (iterator = results ; iterator != NULL; iterator = iterator->next) {
                if (medusa_authenticate_user_can_see_file ((char *) iterator->data, uid, read_cache)) {
                        memset (send_buffer, 0, MAX_LINE);
                        strncpy (send_buffer, iterator->data, MAX_LINE - 1);
                        transmit_string (send_buffer, client_fd);
#ifdef PRESENT_QUERY_RESULTS_DEBUG
                        g_print ("Returning %s\n", (char *) iterator->data);
#endif
                        
                }
#ifdef PRESENT_QUERY_RESULTS_DEBUG
                else {
                        g_print ("Not returning %s because user cannot see it\n", 
                                 (char *) iterator->data);
                }
#endif
        }
        g_hash_table_foreach (read_cache, free_entries, NULL);
        g_hash_table_destroy (read_cache);
        transmit_string (SEARCH_END_TRANSMISSION, client_fd);
        medusa_g_list_free_deep (results);

        printf ("Finished sending search results\n");
        close (client_fd);
}

static void          
free_entries (gpointer key,
              gpointer value,
              gpointer user_data)
{
        char *directory_name;

        directory_name = (char *) key;
        g_free (directory_name);
}

static char *
goto_next_line (char *line)
{
        line = strchr (line, '\n');
        return line+1;
}

static gboolean      
another_transmission_line_exists (char *line)
{
        return (line - 1) != NULL && *line != 0;

}

static void                 
send_cookie_creation_acknowledgement (int client_fd)
{
        g_return_if_fail (client_fd != -1);
        write (client_fd, COOKIE_REQUEST, strlen (COOKIE_REQUEST));
}
