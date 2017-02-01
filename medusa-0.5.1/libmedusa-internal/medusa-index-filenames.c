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
 *  medusa-index-filenames.c -- Functions for creating and managing the status of
 *  index filenames 
 */

#include <config.h>

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include <libmedusa/medusa-file-info-utilities.h>

#include "medusa-index-filenames.h"

#define INDEX_IN_PROGRESS_SUFFIX "in_progress"

/* FIXME bugzilla.eazel.com 4882: At some point the index filenames should be centrally located here, so that each type
   of index doesn't have to have its own set of (basically identical) management methods */

char *
medusa_generate_index_filename (const char *index_file_type,
				const char *index_name,
				gboolean use_in_progress_name)
{
        char *not_in_progress_index_name;
	char *in_progress_index_name;


	if (strlen (index_name) > 0) {
		not_in_progress_index_name = g_strdup_printf ("%s/%s-%s", 
							 MEDUSA_LOCALSTATEDIR, 
							 index_file_type, 
							 index_name);
	}
	else {
		not_in_progress_index_name = g_strdup_printf ("%s/%s", 
							  MEDUSA_LOCALSTATEDIR, 
							  index_file_type);
	}
	
	if (use_in_progress_name) {
		in_progress_index_name = g_strdup_printf ("%s.%s",
							  not_in_progress_index_name,
							  INDEX_IN_PROGRESS_SUFFIX);
		g_free (not_in_progress_index_name);
		return in_progress_index_name;
	}
	else {
		return not_in_progress_index_name;
	}

}

void
medusa_erase_constructed_index_file (const char *index_file_prefix,
                                     const char *index_name)
{
        char *index_filename;
        
        index_filename = medusa_generate_index_filename (index_file_prefix,
                                                         index_name,
                                                         TRUE);
        remove (index_filename);
        g_free (index_filename);
}

/* FIXME bugzilla.eazel.com 4557: Return errors here */
void
medusa_move_completed_index_file_into_place (const char *index_file_prefix,
                                             const char *index_name)
{
        char *in_progress_index_filename, *completed_index_filename;

        /* FIXME bugzilla.eazel.com 4884:  Assert we actually have write lock for index here */
        in_progress_index_filename = medusa_generate_index_filename (index_file_prefix,
                                                                     index_name,
                                                                     TRUE);
        completed_index_filename = medusa_generate_index_filename (index_file_prefix,
                                                                   index_name,
                                                                   FALSE);
        remove (completed_index_filename);
        rename (in_progress_index_filename, completed_index_filename);
        
        g_free (in_progress_index_filename);
        g_free (completed_index_filename);
        
}

gboolean
medusa_index_file_is_newer_than_time  (const char *index_file_prefix,
                                       const char *index_name,
                                       time_t time_to_check)
{
        char *filename;
        gboolean result;

        filename = medusa_generate_index_filename (index_file_prefix,
                                                   index_name,
                                                   FALSE);
        result = medusa_file_is_newer_than_time (filename, time_to_check);
        g_free (filename);

        return result;

        
}
