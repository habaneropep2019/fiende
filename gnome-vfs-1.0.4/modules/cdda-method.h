/* -*- Mode: C; tab-width: 8; indent-tabs-mode: 8; c-basic-offset: 8 -*- */

/* cdda-method.h

   Copyright (C) 2000, Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Gene Z. Ragan <gzr@eazel.com> */


#ifndef CDDA_METHOD_H
#define CDDA_METHOD_H

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdarg.h>

#include "gnome-vfs-module.h"

typedef struct {
	GnomeVFSURI *uri;
	GnomeVFSFileInfo *file_info;
	cdrom_drive *drive;	
	int access_count;
	unsigned int cddb_discid;
	gboolean use_cddb;
	DiscData disc_data;
} CDDAContext;

typedef struct {
	GnomeVFSURI *uri;
	gboolean inited;
	gboolean wrote_header;
	cdrom_paranoia *paranoia;
	long cursor;
	long first_sector, last_sector;
} ReadHandle;

#endif /* CDDA_METHOD_H */
