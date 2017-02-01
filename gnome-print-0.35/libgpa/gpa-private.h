/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2000 Jose M Celorio
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors :
 *  Chema Celorio <chema@celorio.com>
 *
 */
#ifndef __GNOME_PRINT_ADMIN_PRIVATE_H__
#define __GNOME_PRINT_ADMIN_PRIVATE_H__

BEGIN_GNOME_DECLS

#include <gtk/gtkobject.h>

#include "gpa-tags.h"
#include "gpa-enums.h"

#include "gpa-option.h"
#include "gpa-options.h"
#include "gpa-printer.h"
#include "gpa-model.h"
#include "gpa-settings.h"
#include "gpa-vendor.h"
#include "gpa-backend.h"
#include "gpa-model-info.h"
#include "gpa-code.h"



/* This structure is used when we load the printer index,
   since it might not be a good idea to initialize a
   GtkObject for each printer in the index, this just holds
   the required information to create the intallation dialogs
   and when the user actualy installs a printer, THEN we create
   the GtkObject GnomePrintModel */
struct _GpaModelInfo {
	gchar *name;
	gchar *id;
	const GpaVendor *vendor;
};



struct _GpaCodeFragment
{
	gchar *id;
	gchar *content;
	GpaEncoding encoding;
	GpaBackend *backend;
};




END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_PRIVATE_H__ */
