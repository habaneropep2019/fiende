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
#include <glib.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-popt.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <popt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/vfs.h>

#ifdef HAVE_STATVFS
#include <sys/statvfs.h>
#elif defined(HAVE_STATFS)
#include <sys/statfs.h>
#endif /* HAVE_STATVFS */

#include <time.h>
#include <unistd.h>

#include <libmedusa-internal/medusa-conf.h>
#include <libmedusa-internal/medusa-debug.h>
#include <libmedusa/medusa-lock.h>
#include <libmedusa-internal/medusa-master-db.h>
#include <libmedusa/medusa-search-service-private.h>
#include <libmedusa/medusa-index-progress.h>
#include <libmedusa/medusa-index-service.h>
#include <libmedusa/medusa-index-service-private.h>
#include <libmedusa/medusa-log.h>

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif


#ifndef SUN_LEN
/* This system is not POSIX.1g.         */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
       + strlen ((ptr)->sun_path))
#endif

#define ACCOUNT_INDEX_NAME "account"

/* Creates a totally new file index, and writes it over the old one */
static void             do_full_indexing                  (const char *root_directory,
                                                           const char *index_name,
                                                           gboolean use_idle_service);

static void             do_index_accounting               (void);
static void             register_command_line_options     (struct poptOption *medusa_options);
/* Do clean up, etc. */
static void             exit_indexer                      (void);




extern int errno;
static gboolean create_index_even_if_disk_is_full;

/* Main index loop.  
   Creates three process to wait for signals, 
   do fast reindexing, and do slow reindexing */
int
main (int argc, char *argv[])
{
        poptContext popt_context;
        const char **unparsed_arguments;
        gboolean text_index_accounting;
        gboolean dont_use_idle_service;
        gboolean run_in_listener_debug_mode;
        const char *root_directory, *index_name;
        struct poptOption command_line_options[] = {
                { "folder", 'f', POPT_ARG_STRING, &root_directory, 
                  0, N_("Specify the root folder to start indexing at"),
                  N_("PATH") },
                { "named-index", 'n', POPT_ARG_STRING, &index_name,
                  0, N_("Create a named index"),
                  N_("NAME") },
                { "do-text-accounting", '\0', POPT_ARG_NONE, &text_index_accounting,
                  0, N_("Count the space taken in the text index by file name and mime type"),
                  NULL },
                { "without-idle-checks", '\0', POPT_ARG_NONE, &dont_use_idle_service,
                  0, N_("Don't sleep when medusa-idled reports user activity"),
                  NULL },
                { "force-indexing", '\0', POPT_ARG_NONE, &create_index_even_if_disk_is_full,
                  0, N_("Create index even if the disk looks too full to support it"),
                  NULL },
                { "debug-listener", '\0', POPT_ARG_NONE, &run_in_listener_debug_mode,
                  0, N_("Don't fork the listener. Run it in debugging mode"),
                  NULL },
                POPT_AUTOHELP
                { NULL, '\0', 0, NULL, 0, NULL, NULL }
        };



        if (getuid () != 0) {
                puts ("The medusa index daemon must be run as root.  Exiting now\n");
                exit (1);
        }
        
        if (g_getenv ("MEDUSA_DEBUG") != NULL) {
		medusa_make_warnings_and_criticals_stop_in_debugger
			(G_LOG_DOMAIN, g_log_domain_glib,
			 "GnomeVFS",
                         "Medusa",
			 NULL);
	}

        /* Set these to defaults */
        root_directory = "/";
        index_name = "";
        text_index_accounting = FALSE;
        dont_use_idle_service = FALSE;
        run_in_listener_debug_mode = FALSE;

        nice (19);
        gnome_vfs_init ();
        
        register_command_line_options (command_line_options);
        popt_context = gnomelib_parse_args (argc, argv, 0);

        /* Get the remaining arguments */
        unparsed_arguments = poptGetArgs (popt_context);
        if (unparsed_arguments != NULL) {
                fprintf (stderr, "Invalid argument: %s\n", unparsed_arguments[0]);
                fprintf (stderr, "Exiting\n");
                exit_indexer ();
        }
        poptFreeContext (popt_context);

        if (text_index_accounting) {
                do_index_accounting ();
                exit_indexer ();
        }
        
        do_full_indexing (root_directory, index_name, dont_use_idle_service); 
        exit_indexer ();
        
        return 1;
}



void
do_index_accounting (void)
{
        char *root_uri;
  
        root_uri = gnome_vfs_get_uri_from_local_path (ROOT_DIRECTORY);

        medusa_master_db_erase_constructed_index (ACCOUNT_INDEX_NAME);

        medusa_master_db_create_index (root_uri,
                                       ACCOUNT_INDEX_NAME,
                                       FALSE,
                                       MEDUSA_DB_LOG_TEXT_INDEX_DATA);
        g_free (root_uri);
        printf ("Finished indexing \n");

        medusa_master_db_erase_constructed_index (ACCOUNT_INDEX_NAME);
        printf ("Removed account files\n");
}  
		     

static void
exit_if_disk_is_too_full ()
{
#ifdef HAVE_STATVFS
        struct statvfs stat_buffer;
#elif defined(HAVE_STATFS)
        struct statfs stat_buffer;
#else 
#warning "You don't have statfs or statfs installed on your system.  The indexer will not be able to determine "
        "if your disk is too full for indexing"
#endif
        char *error_message;
        int stat_result;
        unsigned long number_of_blocks, number_of_free_blocks, block_size;
        
#ifdef HAVE_STATVFS
        stat_result = statvfs (MEDUSA_LOCALSTATEDIR, &stat_buffer); 
#elif defined(HAVE_STATFS)
        stat_result = statfs (MEDUSA_LOCALSTATEDIR, &stat_buffer); 
#endif

        if (stat_result == -1) {
                error_message = g_strdup_printf ("Can't determine file system "
			"status for %s, so the disk may be too full for an "
			"index. Indexing is continuing", MEDUSA_LOCALSTATEDIR);
                medusa_log_error (error_message);
                g_free (error_message);
                return;
        }

        number_of_blocks = (unsigned long) stat_buffer.f_blocks;
        number_of_free_blocks = (unsigned long) stat_buffer.f_bfree;
        block_size = (unsigned long) stat_buffer.f_bsize;

        g_print ("File system %s has %ld blocks, %ld free\n", 
                 MEDUSA_LOCALSTATEDIR, number_of_blocks, number_of_free_blocks);
        /* We'll need 40 megs or so for a conservative index estimate */
        if ((number_of_blocks / number_of_free_blocks) > 20 ||
            number_of_free_blocks * block_size < (40<<20)) {
                error_message = g_strdup_printf ("Indexing aborted because "
			"disk is either more than 95%% full or 40 megs of "
			"space are not available at location %s",
                                                 MEDUSA_LOCALSTATEDIR);
                medusa_log_error (error_message);
                g_free (error_message);
                exit (1);
        }
        
}

static void
do_full_indexing (const char *root_directory,
                  const char *index_name,
                  gboolean dont_use_idle_service)
{
        char *root_uri;
        MedusaWriteLock *write_lock;
        MedusaLogLevel log_level_from_environment;

        /* Remove old constructed, incomplete indices */
        medusa_master_db_erase_constructed_index (index_name);
        
        if (!create_index_even_if_disk_is_full) {
                exit_if_disk_is_too_full ();
        }
  
        write_lock = medusa_write_lock_get (index_name);
        if (write_lock == NULL) {
                medusa_log_fatal_error ("Failed to acquire lock file for indexing\n");
                exit (1);
        }

        root_uri = gnome_vfs_get_uri_from_local_path (root_directory);
        log_level_from_environment = medusa_log_get_current_log_level ();

        printf ("Indexing...\n");
        medusa_master_db_create_index (root_uri,
                                       index_name,
                                       dont_use_idle_service,
                                       log_level_from_environment);
        g_free (root_uri);
        printf ("Finished indexing \n");

        medusa_master_db_move_completed_index_into_place (index_name);
        printf ("Done moving index files to correct locations\n");

        medusa_write_lock_release (write_lock);
        printf ("Released lock.\n");

        printf ("Exiting\n");
        exit_indexer ();
}

static void             
register_command_line_options (struct poptOption *medusa_options)
{
        gnomelib_register_popt_table (medusa_options, "Medusa Indexer Options");
}
static void             
exit_indexer (void)
{
        gnome_vfs_shutdown ();
        exit (0);
}



