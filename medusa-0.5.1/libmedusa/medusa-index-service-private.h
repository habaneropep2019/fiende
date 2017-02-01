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


/* medusa-index-service-private.h -- Internal constants related to
   the communication protocol */

#ifndef MEDUSA_INDEX_SERVICE_PRIVATE_H
#define MEDUSA_INDEX_SERVICE_PRIVATE_H

#define INDEX_SOCKET_PATH "/tmp/medusa-index-server"

#define MEDUSA_REINDEX_REQUEST      "Create new index now\n"
#define MEDUSA_REINDEX_REQUEST_ACCEPTED "Reindexing now\n"
#define MEDUSA_REINDEX_REQUEST_DENIED_BUSY "Cannot reindex right now\n"

#define MEDUSA_INDEX_SERVICE_AVAILABILITY_REQUEST "Is index service available?\n"
#define MEDUSA_INDEX_SERVICE_AVAILABILITY_UP "Index service is available.\n"
#define MEDUSA_INDEX_SERVICE_AVAILABILITY_BUSY "An index is currently being created\n"

#define MEDUSA_LAST_INDEX_UPDATE_TIME_REQUEST "Get last indexing time\n"


#endif /* MEDUSA_INDEX_SERVICE_PRIVATE_H */
