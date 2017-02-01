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
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <glib.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-popt.h>

#include <libmedusa/medusa-lock.h>
#include <libmedusa-internal/medusa-conf.h>
#include <libmedusa-internal/medusa-debug.h>
#include <libmedusa-internal/medusa-master-db.h>

#include <libmedusa/medusa-log.h>
#include <libmedusa/medusa-search-service-private.h>
#include <libmedusa/medusa-service-private.h>


#include "medusa-authenticate.h"
#include "medusa-file-search-parse-transmission.h"

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

#ifndef SUN_LEN
/* This system is not POSIX.1g.         */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
       + strlen ((ptr)->sun_path))
#endif

static MedusaMasterDB *master_db; 
time_t index_validity_time;
const char *index_name;
gboolean run_in_debug_mode;

static void             ensure_index_validity                       (MedusaMasterDB **master_db);
static int              initialize_socket                           (struct sockaddr_un *daemon_address,
                                                                     const char *index_name);
static void             register_command_line_options               (struct poptOption *medusa_options);
static void             query_loop                                  (int search_listening_fd,
                                                                     struct sockaddr_un *daemon_address,
                                                                     int address_length);
static void             remove_listening_socket_on_interrupt        (int signal_number);

static void             
exit_search_service (void)
{
        medusa_master_db_unref (master_db);
        gnome_vfs_shutdown ();
        exit (0);
}

static void
daemon_init (void)
{
        /* FIXME: We should use the daemon_init from medusa-idled,
           after it has been refactored into medusa-service-private
           instead of this one, as it is more correct (closes file descriptors,
           successfully ditches the terminal), and portable (supports BSD)
           than this one */
          
        pid_t parent_process;
        
        parent_process = fork ();
        /* check if fork failed */
        if (parent_process == -1) {
                puts ("Forking a process to set up the daemon failed.  Exiting now\n");
                exit (1);
        }
        /* only keep the child process */
        if (parent_process != 0) {
                exit (0);
        }
        
        setsid ();

        /* Fork a second time to be sure that the process can't be a session leader */
        parent_process = fork ();
        if (parent_process != 0) {
                exit (0);
        }

        chdir ("/");
        umask (0);
        
        
}

static char *
get_search_socket_name (const char *index_name)
{
        if (strlen (index_name) > 0) {
                return g_strdup_printf ("%s-%s", SEARCH_SOCKET_PATH, index_name);
        }
        else {
                return g_strdup (SEARCH_SOCKET_PATH);
        }
}

static gboolean
can_write_to_existing_socket (const char *socket_name)
{
        int request_port;

        request_port = medusa_initialize_socket_for_requests (socket_name);
        if (request_port == -1) {
                return FALSE;
        }
        
        close (request_port);
        return TRUE;
        
}

static gboolean
make_sure_search_socket_is_available (void)
{
        char *socket_name;
        struct stat socket_info;
        int stat_return_code, unlink_return_code;

        socket_name = get_search_socket_name (index_name);

        stat_return_code = stat (socket_name, &socket_info);
        if (stat_return_code == -1) {
                g_free (socket_name);
                return TRUE;
        }

        if (socket_info.st_uid != 0 ||
            !S_ISSOCK (socket_info.st_mode) ||
            !can_write_to_existing_socket (socket_name)) {
                unlink_return_code = unlink (socket_name);
                g_free (socket_name);
                return (unlink_return_code != -1);
        }

        /* Otherwise it appears that the socket is in use 
           by another search daemon */
        g_free (socket_name);
        return FALSE;

        
}

static void
remove_listening_socket_on_interrupt (int signal_number)
{
        char *socket_name;
        int unlink_return_code;

        socket_name = get_search_socket_name (index_name);
        unlink_return_code = unlink (socket_name);

        if (unlink_return_code != 0) {
                /* FIXME bugzilla.eazel.com 4457: log this error */
        }
        g_free (socket_name);

        exit (0);
}

static void
exit_if_client_closes_socket (int signal_number)
{
        exit_search_service ();
}

int main (int argc, char *argv[])
{
	int search_listening_fd;
	struct sockaddr_un *daemon_address;
	int address_length;
        poptContext popt_context;
        const char **unparsed_arguments;
        char *search_socket_name;
        struct poptOption command_line_options[] = {
                { "debug", 'd', POPT_ARG_NONE, &run_in_debug_mode,
                  0, N_("Don't fork the daemon. Run in debugging mode"),
                  NULL },
                { "named-index", 'n', POPT_ARG_STRING, &index_name,
                  0, N_("Do searches on a named index"),
                  N_("NAME") },
                POPT_AUTOHELP
                { NULL, '\0', 0, NULL, 0, NULL, NULL }
        };

        if (getuid () != 0) {
                puts ("The medusa search daemon must be run as root. Exiting now\n");
                exit (1);
        }

        run_in_debug_mode = FALSE;
        index_name = "";

        register_command_line_options (command_line_options);
        popt_context = gnomelib_parse_args (argc, argv, 0);
        
        /* Get the remaining arguments */
        unparsed_arguments = poptGetArgs (popt_context); 
       if (unparsed_arguments != NULL) {
                fprintf (stderr, "Invalid argument:%s\n", unparsed_arguments[0]);
                fprintf (stderr, "Exiting\n");
                exit_search_service ();
        }
        poptFreeContext (popt_context);


        if (!run_in_debug_mode) {
                /* init taken from Advanced Programming in the UNIX Environment */
                daemon_init ();
        }
        else {
                medusa_make_warnings_and_criticals_stop_in_debugger
			(G_LOG_DOMAIN, g_log_domain_glib,
			 "GnomeVFS",
                         "Medusa",
			 NULL);
	}

        g_atexit (medusa_close_log_file_on_exit);
        

	gnome_vfs_init ();

        /* Catch problems on startup */
        if (!make_sure_search_socket_is_available ()) {
                medusa_log_error ("Search daemon exiting because another is already running");
                exit (1);
        }

        signal (SIGINT, remove_listening_socket_on_interrupt);	        
        signal (SIGTERM, remove_listening_socket_on_interrupt);	        
        /* Ignore the termination status of child processes explicitly,
           so that they don't become zombies (see Stevens, "Advanced Programming 
           in the Unix Environment", page 280) */
        signal (SIGCLD, SIG_IGN);

        /* Finish setting up all of the socket internals */

	daemon_address = g_new0 (struct sockaddr_un, 1);
	search_listening_fd = initialize_socket  (daemon_address,
                                                  index_name);

        search_socket_name = get_search_socket_name (index_name);

        g_free (search_socket_name);
	address_length = SUN_LEN (daemon_address);

	master_db = NULL;	
        query_loop (search_listening_fd,
                    daemon_address,
                    address_length);

	/* Just here for completeness */
	medusa_master_db_unref (master_db); 
	return 0;
}

#ifdef SEARCH_DAEMON_DEBUG
#define SEARCH_TIMEOUT 10000
#else
#define SEARCH_TIMEOUT 1000
#endif
#define SELECT_DELAY 5000

static gboolean
client_has_sent_information (int client_fd)
{
        gboolean fd_has_changed;
        fd_set client_fd_set;
        struct timeval select_wait_time;
        int wait_time;
        
        fd_has_changed = FALSE;
        for (wait_time = 0; wait_time < SEARCH_TIMEOUT; wait_time++) {
                FD_ZERO (&client_fd_set);
                FD_SET (client_fd, &client_fd_set);
                select_wait_time.tv_sec = 0;
                select_wait_time.tv_usec = SELECT_DELAY;
                if (select (client_fd + 1, &client_fd_set, NULL, &client_fd_set, &select_wait_time)) {
                        fd_has_changed = TRUE;
                        break;
                }
        }
        return fd_has_changed;
}

static void
query_loop (int search_listening_fd,
            struct sockaddr_un *daemon_address,
            int address_length)
{
	MedusaReadLock *read_lock;
	char transmission[MAX_LINE];
        char *transmission_log_message;
	int transmission_length;
        int client_fd;
        int search_count = 0;
        pid_t forked_pid;
        
        g_print  ("Ready to receive queries\n");
	for (; ;) {
                /* Receive request */

                client_fd = accept (search_listening_fd, (struct sockaddr *) daemon_address, 
			  	    &address_length);

                if (client_fd == -1) {
                        continue;
                }
                
                if (master_db == NULL) {
                        master_db = medusa_master_db_open (ROOT_DIRECTORY,
                                                           index_name,
                                                           medusa_log_get_current_log_level ());
                }
                if (master_db == NULL) {
                        medusa_log_error ("Could not open the index to do a search");
                        write (client_fd, SEARCH_INDEX_ERROR_TRANSMISSION, sizeof (SEARCH_INDEX_ERROR_TRANSMISSION) - 1);
                        close (client_fd);
                        continue;
                }
                else {
                        index_validity_time = time (NULL);
                }
                
                
                ensure_index_validity (&master_db); 

                forked_pid = 0;
                if (!run_in_debug_mode) {
                        forked_pid = fork ();
                        if (forked_pid == -1) {
                                medusa_log_error ("Fork for search request failed");
                                continue;
                        }
                }

                if (forked_pid == 0) {
                        signal (SIGPIPE, exit_if_client_closes_socket);
                        read_lock = medusa_read_lock_get (index_name);
                        medusa_master_db_ref (master_db);
                        if (!client_has_sent_information (client_fd)) {
                                write (client_fd, SEARCH_REQUEST_TIMEOUT_ERROR, sizeof (SEARCH_REQUEST_TIMEOUT_ERROR) - 1);
                                close (client_fd);
                                medusa_read_lock_release (read_lock);
                                if (run_in_debug_mode) { 
                                        continue;
                                }
                                else {
                                        exit_search_service ();
                                }
                        }
                        transmission_length = read (client_fd, transmission, MAX_LINE - 1);
                        transmission[transmission_length] = 0;
                        transmission_log_message = g_strdup_printf ("Received first transmission %s", transmission);
                        medusa_log_event (transmission_log_message, MEDUSA_DB_LOG_ABBREVIATED);
                        g_free (transmission_log_message);
                        medusa_file_search_parse_transmission (transmission, client_fd, master_db);
                        
                        if (!client_has_sent_information (client_fd)) {
                                write (client_fd, SEARCH_REQUEST_TIMEOUT_ERROR, sizeof (SEARCH_REQUEST_TIMEOUT_ERROR) - 1);
                                close (client_fd);
                                medusa_read_lock_release (read_lock);
                                if (run_in_debug_mode) { 
                                        continue;
                                }
                                else {
                                        exit_search_service ();
                                }
                        }
                        transmission_length = read (client_fd, transmission, MAX_LINE - 1);
                        transmission[transmission_length] = 0;
                        transmission_log_message = g_strdup_printf ("Received second transmission %s", transmission);
                        medusa_log_event (transmission_log_message, MEDUSA_DB_LOG_ABBREVIATED);
                        g_free (transmission_log_message);
                        
                        medusa_file_search_parse_transmission (transmission, client_fd, master_db);
                        medusa_log_event ("End of transmissions for this query", MEDUSA_DB_LOG_ABBREVIATED);
                        /* release lock */
                        medusa_read_lock_release (read_lock);
                        close (client_fd);
                        if (!run_in_debug_mode) { 
                                exit_search_service ();
                        }

                }
                search_count++;
                if (getenv ("MEDUSA_SEARCH_ONCE") != NULL) {
                        exit_search_service ();
                }
                if (getenv ("MEDUSA_SEARCH_TEN") != NULL &&
                    search_count == 10) {
                        exit_search_service ();
                }
        }
        
}

static void
ensure_index_validity (MedusaMasterDB **master_db) 
{
        if (medusa_master_db_index_files_are_still_valid (index_name, index_validity_time)) {
                medusa_master_db_unref (*master_db);
                
                *master_db = medusa_master_db_open (ROOT_DIRECTORY,
                                                    index_name,
                                                    medusa_log_get_current_log_level ());
                if (master_db == NULL) {
                        medusa_log_error ("The index has disappeared, and a new one cannot be loaded.");
                        sleep (10);
                }

  }
}


static void             
register_command_line_options (struct poptOption *medusa_options)
{
        gnomelib_register_popt_table (medusa_options, "Medusa Search Daemon Options");
}


static int
initialize_socket (struct sockaddr_un *daemon_address,
                   const char *index_name)
{
        int search_listening_fd; 
        char *search_socket_name;
        int listen_return_code, bind_return_code;

        search_listening_fd = socket (AF_LOCAL, SOCK_STREAM, 0);
        g_return_val_if_fail (search_listening_fd != -1, -1);

        daemon_address->sun_family = AF_LOCAL;
        /* FIXME bugzilla.eazel.com 2635:  
           This number (108) sucks, but it has no #define in the header.
           What to do? (POSIX requires 100 bytes at least, here)  */
        search_socket_name = get_search_socket_name (index_name);

        strncpy (daemon_address->sun_path, search_socket_name, 100);
        daemon_address->sun_path[99] = 0;

        g_free (search_socket_name);

        /* If socket currently exists, delete it */
        unlink (daemon_address->sun_path);

        bind_return_code = bind (search_listening_fd, (struct sockaddr *) daemon_address, 
                                 SUN_LEN (daemon_address));
        chmod (daemon_address->sun_path, S_IRWXU | S_IRWXG | S_IRWXO);
        if (bind_return_code == -1) {
                return -1;
        }
  
        listen_return_code = listen (search_listening_fd, 5);
        if (listen_return_code == -1) {
                return -1;
        }

        return search_listening_fd;
} 


