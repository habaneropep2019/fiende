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
 */

/* medusa-conf.h: These are hard wired constants.  Some of them may
   need to be tuned for particular systems.  Please feel free to do
   so. */


#ifndef MEDUSA_CONF_H
#define MEDUSA_CONF_H

#include <config.h>
#include <libgnomevfs/gnome-vfs-types.h>

#define ROOT_DIRECTORY "/"

#define MEDUSA_ROOT_INDEX_NAME ""
#define MEDUSA_ACCOUNT_INDEX_NAME "account"
#define MEDUSA_TEST_INDEX_NAME "test"

#define INDEX_LOG_ERRORS
#define MEDUSA_FILE_INDEX_BACKUP_LOCK_FILE "/var/lock/medusa-index-in-progress"

#define MEDUSA_MAXIMUM_SEARCH_QUERY_LENGTH 1024

#define MEDUSA_INDEX_FORMAT_VERSION "0.4"

#define MEDUSA_URI_LIST_FILE_NAME_FIELD_TITLE      "File_Name"
#define MEDUSA_URI_LIST_FILE_NAME_FIELD_SIZE       sizeof (MedusaToken)
#define MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_TITLE "Directory_Name"
#define MEDUSA_URI_LIST_DIRECTORY_NAME_FIELD_SIZE  sizeof (MedusaToken)

#define MEDUSA_FILE_INDEX_URI_NUMBER_FIELD_TITLE   "URI_Number"
#define MEDUSA_FILE_INDEX_URI_NUMBER_FIELD_SIZE    sizeof (gint32)
#define MEDUSA_FILE_INDEX_MTIME_FIELD_TITLE        "Modification_Time"
#define MEDUSA_FILE_INDEX_MTIME_FIELD_SIZE         sizeof (time_t)
#define MEDUSA_FILE_INDEX_OWNER_FIELD_TITLE        "Owner"
#define MEDUSA_FILE_INDEX_OWNER_FIELD_SIZE         sizeof (uid_t)
#define MEDUSA_FILE_INDEX_GROUP_FIELD_TITLE        "Group"
#define MEDUSA_FILE_INDEX_GROUP_FIELD_SIZE         sizeof (gid_t)
#define MEDUSA_FILE_INDEX_PERMISSIONS_FIELD_TITLE  "Permissions"
#define MEDUSA_FILE_INDEX_PERMISSIONS_FIELD_SIZE   sizeof (GnomeVFSFilePermissions)
#define MEDUSA_FILE_INDEX_SIZE_FIELD_TITLE         "Size"
#define MEDUSA_FILE_INDEX_SIZE_FIELD_SIZE          sizeof (size_t)
#define MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_TITLE    "Mime_Type"
#define MEDUSA_FILE_INDEX_MIME_TYPE_FIELD_SIZE     sizeof (MedusaToken)
#define MEDUSA_FILE_INDEX_KEYWORDS_FIELD_TITLE     "keywords"
#define MEDUSA_FILE_INDEX_KEYWORDS_FIELD_SIZE      sizeof (MedusaToken)

/* These aren't being used yet, but maybe we should
   use these instead of mime types to deal with problems
   of certain fields being useless for certain types of
   files, like size */
#define MEDUSA_FILE_INDEX_FILE_TYPE_FIELD_TITLE   "File_Type"
#define MEDUSA_FILE_INDEX_FILE_TYPE_FIELD_SIZE    1

#define FILE_NAME_MAXIMUM_LENGTH 1024

#endif /* MEDUSA_CONF_H */
