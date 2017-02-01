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
 *  medusa-index-progress.c -- Structures that keeps track of the
 *  total amount of disk space there is to index (or search)
 *  and reports back through a callback how much has been done.
 */

#ifndef MEDUSA_INDEX_PROGRESS_H
#define MEDUSA_INDEX_PROGRESS_H


typedef struct MedusaIndexProgress MedusaIndexProgress;

/* Functions the indexer uses to keep track of progress */

MedusaIndexProgress *         medusa_index_progress_new                     (char *root_directory);
void                          medusa_index_progress_update                  (MedusaIndexProgress *progress,
									     size_t new_bytes_processed);
void                          medusa_index_progress_destroy                 (MedusaIndexProgress *progress);

/* Functions for requestors of progress. */
gboolean                      medusa_indexing_is_currently_in_progress      (void);
int                           medusa_index_progress_get_percentage_complete (void);

#endif /* MEDUSA_INDEX_PROGRESS_H */
