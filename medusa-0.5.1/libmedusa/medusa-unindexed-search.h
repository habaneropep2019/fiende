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


/* medusa-unindexed-search.h -- Service to run a find that backs up an
 indexed search */

#ifndef MEDUSA_UNINDEXED_SEARCH_H
#define MEDUSA_UNINDEXED_SEARCH_H

#include <libgnomevfs/gnome-vfs-types.h>

#define MEDUSA_FILE_NAME_CRITERION "file_name"
#define MEDUSA_DIRECTORY_NAME_CRITERION "file_name"
#define MEDUSA_FILE_TYPE_CRITERION "file_type"
#define MEDUSA_MODIFIED_CRITERION "modified"
#define MEDUSA_OWNER_CRITERION "owner"
#define MEDUSA_GROUP_CRITERION "group"
#define MEDUSA_SIZE_CRITERION "size"
#define MEDUSA_READ_PERMISSIONS_CRITERION "permissions_to_read"
#define MEDUSA_CONTENTS_CRITERION "contents"
#define MEDUSA_KEYWORDS_CRITERION "keywords"


typedef struct MedusaUnindexedSearch MedusaUnindexedSearch;

MedusaUnindexedSearch *medusa_unindexed_search_new                    (GnomeVFSResult *result,
                                                                       const char *uri_without_scheme_or_method);
GnomeVFSResult         medusa_unindexed_search_is_available_for_uri   (const char *uri_without_scheme_or_method);
GnomeVFSResult         medusa_unindexed_search_read_search_result     (MedusaUnindexedSearch *search,
                                                                       GnomeVFSContext *context,
                                                                       char **result_uri);
void                   medusa_unindexed_search_destroy                (MedusaUnindexedSearch *search);

#endif /* MEDUSA_UNINDEXED_SEARCH_H */






