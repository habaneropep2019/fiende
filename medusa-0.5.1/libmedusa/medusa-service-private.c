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
 *  
 */


#include "medusa-service-private.h"

#include <errno.h>
#include <glib.h> 
#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

#ifndef SUN_LEN
/* This system is not POSIX.1g.         */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
       + strlen ((ptr)->sun_path))
#endif
#undef MEDUSA_SERVICE_PRIVATE_DEBUG

extern int errno;

static gboolean
root_owns_search_socket (const char *socket_path)
{
  struct stat stat_info;

  if (stat (socket_path, &stat_info) == -1) {
#ifdef MEDUSA_SERVICE_PRIVATE_DEBUG
    g_print ("The search socket doesn't exist.  Exiting now\n");
#endif
    /* FIXME bugzilla.eazel.com 4457: log error here */
    return FALSE;
  }

  return (stat_info.st_uid == 0);
}


int
medusa_initialize_socket_for_requests (const char *socket_path)
{
        int request_port;
        int connect_result;
        struct sockaddr_un daemon_address;    

        
	if (!root_owns_search_socket (socket_path)) {
#ifdef MEDUSA_SERVICE_PRIVATE_DEBUG
	  g_print ("If the search socket exists, it's not owned by root.  We can't connect to a socket at %s\n", socket_path);
#endif
	  /* FIXME bugzilla.eazel.com 4457: log this error */
	  return -1;
	}
        request_port = socket (AF_LOCAL, SOCK_STREAM, 0);
        if (request_port == -1) {
#ifdef MEDUSA_SERVICE_PRIVATE_DEBUG
	  g_print ("Couldn't create a socket when initializing socket at %s.  Error code was %d\n", socket_path, errno);
#endif
		return -1;
        }
        daemon_address.sun_family = AF_LOCAL;

#define POSIX_GUARANTEED_UNIX_SOCKET_PATH_SIZE 100

        strncpy (daemon_address.sun_path, socket_path, POSIX_GUARANTEED_UNIX_SOCKET_PATH_SIZE - 1);

#undef POSIX_GUARANTEED_UNIX_SOCKET_PATH_SIZE

        connect_result = connect (request_port, (struct sockaddr *) &daemon_address,
                                  SUN_LEN (&daemon_address));
        /* We might be supposed to "try again" if we get an EINTR
           instead of just accepting that case as a successful connection. */
        if (connect_result == -1 &&
            errno != EINTR) {
#ifdef MEDUSA_SERVICE_PRIVATE_DEBUG
	  g_print ("Connect failed on socket at %s.  Error code was %d\n", socket_path, errno);
#endif
                return -1;
        }
        
        return request_port;
}



