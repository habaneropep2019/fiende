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

/* medusa-uri-list-private.h  -- definition of the MedusaURIList structure */


#ifndef MEDUSA_URI_LIST_PRIVATE_H
#define MEDUSA_URI_LIST_PRIVATE_H

#include <libmedusa/medusa-log.h>
#include "medusa-query-clauses.h"
#include "medusa-string-list.h"

struct MedusaURIList {
        MedusaRDBTable *uri_names;

        MedusaRDBRecordNumbers *current_record;

        MedusaLogLevel log_level;
        MedusaStringList *file_names;
        MedusaStringList *directory_names;

        char *version;
        int indexing_start_time;
        int last_index_time;

        int ref_count;

        MedusaQueryClauses *clauses;
};

#endif /* MEDUSA_URI_LIST_PRIVATE_H */

