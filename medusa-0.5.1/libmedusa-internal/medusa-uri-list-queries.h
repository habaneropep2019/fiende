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
 *  medusa-uri-list-queries.h  Builds and performs common queries on a list of uri's
 *
 */

#ifndef MEDUSA_URI_LIST_QUERIES_H
#define MEDUSA_URI_LIST_QUERIES_H

#include <glib.h>


gboolean           medusa_uri_list_is_in_directory                     (gpointer database,
                                                                        gpointer record,
                                                                        MedusaQueryArgument directory_name);
gboolean           medusa_uri_list_is_named                            (gpointer database,
                                                                        gpointer record,
                                                                        MedusaQueryArgument file_name);
gboolean           medusa_uri_list_has_name_regexp_matching            (gpointer database,
                                                                        gpointer record,
                                                                        MedusaQueryArgument string);
gboolean           medusa_uri_list_has_name_not_regexp_matching        (gpointer database,
                                                                        gpointer record,
                                                                        
                                                                        MedusaQueryArgument string);


gboolean           medusa_uri_list_is_in_directory_regexp_matching     (gpointer database,
                                                                        gpointer record,
                                                                        MedusaQueryArgument string);


gboolean           medusa_uri_list_is_not_in_directory_regexp_matching (gpointer database,
                                                                        gpointer record,
                                                                        MedusaQueryArgument string);


gboolean           medusa_uri_list_has_name_containing                 (gpointer database,
                                                                        gpointer record,
                                                                        MedusaQueryArgument string);

gboolean           medusa_uri_list_has_name_not_containing             (gpointer database,
                                                                        gpointer record,
                                                                        MedusaQueryArgument string);


gboolean           medusa_uri_list_is_in_directory_containing          (gpointer database,
                                                                        gpointer record,
                                                                        MedusaQueryArgument string);


gboolean           medusa_uri_list_has_file_name_starting_with         (gpointer database,
                                                                        gpointer record,
                                                                        MedusaQueryArgument string);


gboolean           medusa_uri_list_has_file_name_ending_with          (gpointer database,
                                                                       gpointer record,
                                                                       MedusaQueryArgument string);


gboolean           medusa_uri_list_is_in_directory_tree               (gpointer database,
                                                                       gpointer record,
                                                                       MedusaQueryArgument string);


gboolean           medusa_uri_list_has_name_glob_matching             (gpointer database,
                                                                       gpointer record,
                                                                       MedusaQueryArgument string);


gboolean           medusa_uri_list_has_full_file_name                 (gpointer database,
                                                                       gpointer record,
                                                                       MedusaQueryArgument full_file_name);


#endif /* MEDUSA_URI_LIST_QUERIES_H */


