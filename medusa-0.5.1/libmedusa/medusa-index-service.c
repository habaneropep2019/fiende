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


/* medusa-index-service.c -- API for externals users of the medusa
   indexing service. */

#include <config.h>
#include "medusa-index-service.h"
#include "medusa-index-service-private.h"
#include "medusa-log.h"
#include "medusa-service-private.h"


#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>



static time_t        get_last_index_update_time_by_hack                 (void);

time_t                      
medusa_index_service_get_last_index_update_time  (void)
{
        return get_last_index_update_time_by_hack ();
}


/* FIXME bugzilla.eazel.com 4883: this is a bad hack */
static time_t
get_last_index_update_time_by_hack (void)
{
        struct stat index_file_info;
        int stat_return_code;
        char *location_file_name; 

        location_file_name = g_strdup_printf ("%s/%s", MEDUSA_LOCALSTATEDIR,
				       "text-index-location-file");
        stat_return_code = stat (location_file_name, &index_file_info);
        if (stat_return_code != 0) {
                return 0;
        }
        g_free (location_file_name);

        return index_file_info.st_mtime;
}
