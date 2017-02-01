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

/* medusa-file-attributes.h:  API for the medusa structure that contains
   information about a file stored in the file index */

#ifndef MEDUSA_FILE_INFORMATION_H
#define MEDUSA_FILE_INFORMATION_H

#include <time.h>
#include <sys/types.h>
#include <glib.h>
#include <libgnomevfs/gnome-vfs-types.h>
#include <medusa-file-index-encoders.h>


typedef struct MedusaFileAttributes MedusaFileAttributes;

/* Adds metadata to the database */
	


MedusaFileAttributes *    medusa_file_attributes_new               (int uri_number,
								    GnomeVFSFileInfo *metadata);

void                      medusa_index_file_attributes             (MedusaFileAttributes *file_attributes, 
								    MedusaFileSystemDB *file_system_db);
void                      medusa_file_attributes_free              (MedusaFileAttributes *file_attributes);


#endif /* MEDUSA_FILE_INFORMATION_H */
