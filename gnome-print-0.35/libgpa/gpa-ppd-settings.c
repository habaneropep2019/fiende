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

#include "gpa-defs.h"

#include <glib.h>

#include "gpa-tags.h"
#include "gpa-values.h"
#include "gpa-ppd-settings.h"

GList *
gpa_ppd_insert_default_settings (GList *list)
{
	/* FIXME : we might need to insert more stuff and use the know_settings struct */
	/* for now this will do */

	debug (FALSE, "");

	list = gpa_value_insert (list, GPA_TAG_ORIENTATION, GPA_TAG_PORTRAIT);
	
	return list;
}

