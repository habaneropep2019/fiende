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


/* medusa-indexed-search.h -- Service to run a find that backs up an
 indexed search */


#ifndef MEDUSA_INDEXED_SEARCH_H
#define MEDUSA_INDEXED_SEARCH_H

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-context.h>

typedef struct MedusaIndexedSearch MedusaIndexedSearch;


MedusaIndexedSearch   *medusa_indexed_search_new                 (GnomeVFSResult *result,
                                                                  GnomeVFSContext *context,
                                                                  const char *uri);
GnomeVFSResult         medusa_indexed_search_is_available        (void);
gboolean               medusa_indexed_search_system_index_files_look_available (void);
GnomeVFSResult         medusa_indexed_search_start_search        (MedusaIndexedSearch *search,
								  const char *search_uri);
GnomeVFSResult         medusa_indexed_search_read_search_result  (MedusaIndexedSearch *search,
                                                                  GnomeVFSContext *context,
								  char **result);
void                   medusa_indexed_search_destroy             (MedusaIndexedSearch *search);

#endif /* MEDUSA_INDEXED_SEARCH_H */


