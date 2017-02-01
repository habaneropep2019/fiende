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

/* medusa-file-index-encoders.h:  Functions to encode and decode
   the various fields in the file index database */

#ifndef MEDUSA_URI_LIST_ENCODERS_H
#define MEDUSA_URI_LIST__ENCODERS_H

#include <medusa-uri-list.h>

void                 medusa_uri_list_filename_encode            (char *result,
								 char *file_name,
								 MedusaURIList *uri_list);
void                 medusa_uri_list_directory_name_encode      (char *result,
								 char *directory_name,
								 MedusaURIList *uri_list);

void                 medusa_uri_list_is_directory_encode        (char *result,
								 gboolean is_directory,
								 MedusaURIList *uri_list);

void                 medusa_uri_list_filename_decode            (char *file_name_result,
								 char *field,
								 MedusaURIList *uri_list);

void                 medusa_uri_list_directory_name_decode      (char *directory_name_result,
								 char *field,
								 MedusaURIList *uri_list);
void                 medusa_uri_list_is_directory_decode        (gboolean *is_directory_result,
								 char *field,
								 MedusaURIList *uri_list);

#endif /* MEDUSA_URI_LIST_ENCODERS_H */
