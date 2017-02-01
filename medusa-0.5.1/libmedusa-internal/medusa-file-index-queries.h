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
 *  medusa-file-index-queries.h  Builds and performs common queries on a file index
 *
 */

#ifndef MEDUSA_FILE_INDEX_QUERIES_H
#define MEDUSA_FILE_INDEX_QUERIES_H

#include "medusa-file-index.h"

gboolean medusa_file_index_is_of_type                         (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument type_argument);
gboolean medusa_file_index_is_not_of_type                     (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument type_argument);

gboolean medusa_file_index_has_uid                            (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument               uid);
gboolean medusa_file_index_does_not_have_uid                  (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument               uid);
gboolean medusa_file_index_has_gid                            (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument               gid);
gboolean medusa_file_index_does_not_have_gid                  (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument               gid);

/* FIXME bugzilla.eazel.com 2649: 
   These should take time_t, not ints */

gboolean medusa_file_index_is_modified_before_time            (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument               unix_time);
gboolean medusa_file_index_is_modified_after_time             (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument                 unix_time);
gboolean medusa_file_index_is_not_modified_on_date            (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument argument);
gboolean medusa_file_index_verify_file_is_modified_before_time (const char *uri,
                                                                MedusaQueryArgument arg);
gboolean medusa_file_index_verify_file_is_modified_after_time  (const char *uri,
                                                                MedusaQueryArgument arg);
gboolean medusa_file_index_verify_file_is_not_modified_on_date (const char *uri,
                                                                MedusaQueryArgument arg);
                                                                
                                                                
gboolean medusa_file_index_uid_can_read_file                  (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument                 uid);
gboolean medusa_file_index_is_larger_than                     (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument                 size);
gboolean medusa_file_index_is_smaller_than                    (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument                 size);
gboolean medusa_file_index_is_of_size                         (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument                 size);
gboolean medusa_file_index_marked_with_keyword                (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument keyword);
gboolean medusa_file_index_not_marked_with_keyword            (gpointer database,
                                                               gpointer record,
                                                               MedusaQueryArgument keyword);

#endif /* MEDUSA_FILE_INDEX_QUERIES_H */
