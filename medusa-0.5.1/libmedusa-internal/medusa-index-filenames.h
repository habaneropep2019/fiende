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
 *  medusa-path-string-functions.h -- Medusa functions that manipulate
 *  paths as strings
 */

#ifndef MEDUSA_INDEX_FILENAMES_H
#define MEDUSA_INDEX_FILENAMES_H

#include <glib.h>


char *         medusa_generate_index_filename                    (const char                     *index_file_prefix,
								  const char                     *index_name,
								  gboolean                       index_in_progress);

void           medusa_erase_constructed_index_file               (const char                     *index_file_prefix,
								  const char                     *index_name);
void           medusa_move_completed_index_file_into_place       (const char                     *index_file_prefix,
								  const char                     *index_name);

gboolean       medusa_index_file_is_newer_than_time              (const char                     *index_file_prefix,
								  const char                     *index_name,
								  time_t                         time_to_check);



#endif /* MEDUSA_INDEX_FILENAMES_H */
