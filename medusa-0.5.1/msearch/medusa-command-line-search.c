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
 *          Maciej Stachowiak <mjs@eazel.com>
 */

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libmedusa-internal/medusa-debug.h>
#include <libmedusa/medusa-search-service.h>
#include <unistd.h>
#include <stdlib.h>
#include <glib.h>


static void
output_connection_error_message_and_exit (GnomeVFSResult error_result)
{
        switch (error_result) {
        case GNOME_VFS_ERROR_INVALID_URI:
                g_print ("The URI you entered was invalid.  If you are unsure of how to form a search uri, consult the search_uri_rfc, that is part of medusa's documentation.\n");
                break;
        case GNOME_VFS_ERROR_TOO_BIG:
                g_print ("The URI you entered matches every file in the index\n");
                break;
        case GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE:
                g_print ("The search service is unavailable right now.  It can be started by running /gnome/bin/medusa-searchd as root.  \n\nYour index may also be missing or corrupt, in which case you should rerun the medusa indexer.  The indexer can be restarted by running /gnome/bin/medusa-indexd as root.\n");
                break;
        default:
                g_print ("An error occurred while doing the search: %s\n", gnome_vfs_result_to_string (error_result));
        }
        exit (1);
}


int
main (int argc, char *argv[])
{
        MedusaSearchServiceConnection *connection;
        char *result_uri, *unescaped_local_path;
        const char *escaped_local_path;
        GnomeVFSURI *result_vfs_uri;
        GnomeVFSResult connection_opened_result, read_result;

        gnome_vfs_init ();
        if (argc < 2) {
                g_print ("usage: msearch SEARCH_URI\n");
                return -1;
        }

        medusa_make_warnings_and_criticals_stop_in_debugger
                (G_LOG_DOMAIN, g_log_domain_glib,
                 "GnomeVFS",
                 "Medusa",
                 NULL);
        
        connection = medusa_search_service_connection_new (argv[1],
                                                           NULL,
                                                           &connection_opened_result);
        if (connection_opened_result != GNOME_VFS_OK) {
                output_connection_error_message_and_exit (connection_opened_result);
        }
        
        medusa_search_service_connection_start_search (connection);

        read_result = medusa_search_service_connection_read_search_result (connection, NULL, &result_uri);
        while (read_result == GNOME_VFS_OK) {
                result_vfs_uri = gnome_vfs_uri_new (result_uri);
                escaped_local_path = gnome_vfs_uri_get_path (result_vfs_uri);
                unescaped_local_path = gnome_vfs_unescape_string_for_display (escaped_local_path);
                g_print ("%s\n", unescaped_local_path);
                g_free (unescaped_local_path);
                gnome_vfs_uri_unref (result_vfs_uri);
                g_free (result_uri);
                read_result = medusa_search_service_connection_read_search_result (connection, NULL, &result_uri);
        }

        medusa_search_service_connection_destroy (connection);
        gnome_vfs_shutdown ();
        return 0;
}











