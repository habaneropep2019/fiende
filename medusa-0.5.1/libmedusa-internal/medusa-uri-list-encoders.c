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

#include <string.h>
#include <glib.h>

#include "medusa-byte.h"
#include "medusa-conf.h"
#include "medusa-rdb-table.h"
#include "medusa-string-list.h"
#include "medusa-uri-list.h"
#include "medusa-uri-list-encoders.h"
#include "medusa-uri-list-private.h"

void
medusa_uri_list_filename_encode (char *result,
				      char *file_name,
				      MedusaURIList *uri_list)
{
  char *bytes; 
  bytes = medusa_token_to_bytes (medusa_string_list_store_string (uri_list->file_names,
                                                                  file_name),
				 medusa_rdb_field_get_size (uri_list->uri_names->file->field_info, 
							    MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE));
  memcpy (result,
          bytes,
          medusa_rdb_field_get_size (uri_list->uri_names->file->field_info, 
                                     MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE));
                                 g_free (bytes);
}



void
medusa_uri_list_directory_name_encode (char *result,
				       char *directory_name,
				       MedusaURIList *uri_list)
{
  char *bytes;
  bytes = medusa_token_to_bytes (medusa_string_list_store_string (uri_list->directory_names,
                                                                  directory_name),
				 medusa_rdb_field_get_size (uri_list->uri_names->file->field_info, 
							    MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE));
  memcpy (result,
	   bytes,
           medusa_rdb_field_get_size (uri_list->uri_names->file->field_info, 
				      MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE));
  g_free (bytes);

}


void 
medusa_uri_list_is_directory_encode (char *result,
				     gboolean is_directory,
				     MedusaURIList *uri_list)
{
  if (is_directory) {
    *result = 1;
  }
  else {
    *result = 0;
  }
}



void 
medusa_uri_list_filename_decode (char *file_name_result,
				 char *field,
				 MedusaURIList *uri_list)
{
        MedusaToken token;

        memcpy (&token, field, sizeof (MedusaToken));

        strncpy (file_name_result, 
                 medusa_string_list_get_string (uri_list->file_names,
                                                token),
                 FILE_NAME_MAXIMUM_LENGTH - 1);
        file_name_result[FILE_NAME_MAXIMUM_LENGTH - 1] = 0;
}



void 
medusa_uri_list_directory_name_decode (char *directory_name_result,
					    char *field,
					    MedusaURIList *uri_list)
{
        MedusaToken token;

        memcpy (&token, field, sizeof (MedusaToken));
        strncpy (directory_name_result, 
                 medusa_string_list_get_string (uri_list->directory_names,
                                                token),
                 FILE_NAME_MAXIMUM_LENGTH - 1);
        directory_name_result[FILE_NAME_MAXIMUM_LENGTH - 1] = 0;
}


void 
medusa_uri_list_is_directory_decode (gboolean *is_directory_result,
				     char *field,
				     MedusaURIList *uri_list)
{
  if (*field == 0) {
    *is_directory_result = FALSE;
  }
  else {
    *is_directory_result = TRUE;
  }
}

