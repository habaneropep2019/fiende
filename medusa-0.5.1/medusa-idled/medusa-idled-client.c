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
 *  medusa-idled-client.c - Daemon that runs as part of 
 *  an X session and notifies the medusa indexer (and possibly others)
 *  to back off when user activity happens
 */

/* Returns status the time of changes, and then produces
   events every time the status changes */

#include <glib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <libmedusa/medusa-service-private.h>
#include "medusa-idled-client.h"
#include "medusa-idled-private.h"

#define READ_BUFFER_SIZE 512
#undef MEDUSA_IDLED_CLIENT_DEBUG


#ifndef AF_LOCAL
#define AF_LOCAL PF_UNIX
#endif

#ifndef SUN_LEN
/* This system is not POSIX.1g.         */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
       + strlen ((ptr)->sun_path))
#endif

#ifndef AF_LOCAL
#define AF_LOCAL PF_UNIX
#endif

#ifndef SUN_LEN
/* This system is not POSIX.1g.         */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
       + strlen ((ptr)->sun_path))
#endif

struct MedusaIdledConnection {
        int file_descriptor;
        gboolean no_current_connection;
        MedusaIdleStatus current_status;
};

static MedusaIdleStatus
get_current_idle_status (const char *buffer)
{
        if (strcmp (buffer, MEDUSA_IDLE_SERVICE_NO_USER_ACTIVITY_MESSAGE) == 0) {
                return USER_ACTIVITY_ABSENT;
        }
        else if (strcmp (buffer, MEDUSA_IDLE_SERVICE_USER_ACTIVITY_MESSAGE) == 0) {
                return USER_ACTIVITY_PRESENT;
        }
        else {
                return USER_ACTIVITY_UNKNOWN;
        }
}

static int
find_last_buffer_line (const char *read_buffer,
                       const char **last_line_beginning)
{
        int last_line_length = 0;
        const char *buffer_location;

        buffer_location = read_buffer + READ_BUFFER_SIZE - 1;
        while (*buffer_location != '\n' &&
               buffer_location > read_buffer) {
                buffer_location--;
                last_line_length++;
        }
        if (buffer_location != read_buffer) {
                buffer_location--;
        }
        while (*buffer_location != '\n' &&
               buffer_location > read_buffer) {
                buffer_location--;
                last_line_length++;
        }
        if (buffer_location != read_buffer) {
                *last_line_beginning = buffer_location + 1;
        }
        else {
                *last_line_beginning = buffer_location;
        }
        return last_line_length;
}

static char *
read_most_current_status (int status_fd)
{
        char *read_buffer, *new_read_buffer;
        const char *last_line;
        const char *most_current_status;
        int last_line_length, bytes_read;
        
        read_buffer = g_new0 (char, READ_BUFFER_SIZE);

        bytes_read = read (status_fd, read_buffer, READ_BUFFER_SIZE);

        if (bytes_read == 0 || bytes_read == -1) {
                g_free (read_buffer);
                return NULL;
        }

        while (bytes_read == READ_BUFFER_SIZE) {
                last_line_length = find_last_buffer_line (read_buffer,
                                                          &last_line);
                new_read_buffer = g_new0 (char, READ_BUFFER_SIZE + last_line_length);
                strncpy (new_read_buffer, read_buffer, last_line_length);
                bytes_read = read (status_fd, new_read_buffer + last_line_length, READ_BUFFER_SIZE);
                g_free (read_buffer);
                read_buffer = new_read_buffer;
        }

        last_line_length = find_last_buffer_line (read_buffer, &most_current_status);

        return g_strndup (most_current_status, last_line_length);
}
                          
static int
initialize_client_socket (const char *socket_path)
{
        int request_port;
        int connect_result, write_result;
        struct sockaddr_un daemon_address;    

        request_port = socket (AF_LOCAL, SOCK_STREAM, 0);
        if (request_port == -1) {
#ifdef MEDUSA_IDLED_CLIENT_DEBUG
                g_warning ("request port failed\n");
#endif
        }
        g_return_val_if_fail (request_port != -1, -1);
        
        daemon_address.sun_family = AF_LOCAL;

#define POSIX_GUARANTEED_UNIX_SOCKET_PATH_SIZE 100

        strncpy (daemon_address.sun_path, socket_path, POSIX_GUARANTEED_UNIX_SOCKET_PATH_SIZE - 1);

#undef POSIX_GUARANTEED_UNIX_SOCKET_PATH_SIZE
        
        connect_result = connect (request_port, (struct sockaddr *) &daemon_address,
                                  SUN_LEN (&daemon_address));

        if (connect_result == -1) {
                close (request_port);
                return -1;
        }
#ifndef WAKE_UP_MESSAGE
#define WAKE_UP_MESSAGE "Wake Up!"
#endif
        write_result = write (request_port, WAKE_UP_MESSAGE, strlen (WAKE_UP_MESSAGE));
        if (write_result == -1) {
                close (request_port);
                return -1;
        }
        return request_port;
}


MedusaIdleStatus
medusa_idle_service_register (MedusaIdledConnection **idle_connection)
{
        MedusaIdledConnection *connection;
        const char *most_current_status;

        g_return_val_if_fail (idle_connection != NULL, USER_ACTIVITY_UNKNOWN);
        connection = g_new0 (MedusaIdledConnection, 1);
        connection->file_descriptor = initialize_client_socket (MEDUSA_IDLE_SERVICE_SOCKET_PATH);

        if (connection->file_descriptor == -1) {
                connection->no_current_connection = TRUE;
                connection->current_status = USER_ACTIVITY_UNKNOWN;
                *idle_connection = connection;
                return USER_ACTIVITY_UNKNOWN;
        }
        /* Wait for the service to report its status */
        most_current_status = read_most_current_status (connection->file_descriptor);
        
        if (most_current_status == NULL) {
                connection->no_current_connection = TRUE;
                connection->current_status = USER_ACTIVITY_UNKNOWN;
                close (connection->file_descriptor);
                *idle_connection = connection;
                return USER_ACTIVITY_UNKNOWN;
        }       
        connection->current_status = get_current_idle_status (most_current_status);

        if (connection->current_status == USER_ACTIVITY_UNKNOWN) {
                close (connection->file_descriptor);
                connection->no_current_connection = TRUE;
        }
        
        *idle_connection = connection;
        return connection->current_status;
}

static void
reregister_idled_connection_if_possible (MedusaIdledConnection *idled_connection)
{
        MedusaIdledConnection *new_connection;
        MedusaIdleStatus status_received_from_new_connection;
#ifdef MEDUSA_IDLED_CLIENT_DEBUG
        g_print ("Can't determine current status.  Trying to create a new connection to the idle service\n");
#endif
        g_assert (idled_connection->file_descriptor == -1);
        status_received_from_new_connection = medusa_idle_service_register (&new_connection);
        if (status_received_from_new_connection != USER_ACTIVITY_UNKNOWN) {
#ifdef MEDUSA_IDLED_CLIENT_DEBUG
                g_print ("Found new connection.  Starting now\n");
#endif
                close (idled_connection->file_descriptor);
                idled_connection->no_current_connection = FALSE;
                idled_connection->file_descriptor = new_connection->file_descriptor;
                idled_connection->current_status = new_connection->current_status;
        }
        g_free (new_connection);
}



MedusaIdleStatus 
medusa_idle_service_request_current_idle_status (MedusaIdledConnection *idled_connection)
{
        char *most_current_status;
        fd_set read_set;
        struct timeval select_wait_time;
        
        if (idled_connection->no_current_connection) {
                reregister_idled_connection_if_possible (idled_connection);
                return idled_connection->current_status;
        }

        FD_ZERO (&read_set);
        FD_SET (idled_connection->file_descriptor, &read_set);
        select_wait_time.tv_sec = 0;
        select_wait_time.tv_usec = 0;
        
        /* Get an update if it's available using select.  Otherwise, just return 
         what we had. */
        if (select (idled_connection->file_descriptor + 1, &read_set, NULL, &read_set, &select_wait_time)) {
                most_current_status = read_most_current_status (idled_connection->file_descriptor);
                
                if (most_current_status) {
                        idled_connection->current_status = get_current_idle_status (most_current_status);
                }
                /* There must be an exception.  Decide we've lost the idled daemon
                   again */
                else {
                        close (idled_connection->file_descriptor);
                        idled_connection->current_status = USER_ACTIVITY_UNKNOWN;
                        idled_connection->no_current_connection = TRUE;
                }
        }
        /* If status is unknown, someone may have logged out.
           Check to make sure a new idle daemon hasn't started 
           before we are sure we don't know what's going on. */
        if (idled_connection->current_status == USER_ACTIVITY_UNKNOWN) {
                reregister_idled_connection_if_possible (idled_connection);
        }
        return idled_connection->current_status;
        
}

#define IDLE_REQUEST_INTERVAL 15

void
medusa_idle_service_sleep_until_idle (MedusaIdledConnection *idled_connection)
{
#ifdef MEDUSA_IDLED_CLIENT_DEBUG
        g_print ("Making sure there is no user activity before continuing indexing\n");
#endif
        g_return_if_fail (idled_connection != NULL);

        idled_connection->current_status = 
                medusa_idle_service_request_current_idle_status (idled_connection);
        while (idled_connection->current_status == USER_ACTIVITY_PRESENT) {
                sleep (IDLE_REQUEST_INTERVAL);
                idled_connection->current_status = 
                        medusa_idle_service_request_current_idle_status (idled_connection);
        }
#ifdef MEDUSA_IDLED_CLIENT_DEBUG
        g_print ("Activity is not present.  Continuing with program\n");
#endif
}


void                    
medusa_idle_service_unregister_and_destroy_connection (MedusaIdledConnection *idled_connection)
{
        if (idled_connection != NULL) {
                close (idled_connection->file_descriptor);
                g_free (idled_connection);
        }
}
