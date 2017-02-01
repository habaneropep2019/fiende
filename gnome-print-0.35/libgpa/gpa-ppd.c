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

#include "gpa-private.h"
#include "gpa-model.h"
#include "gpa-utils.h"

#include "gpa-ppd.h"
#include "gpa-ppd-model.h"

gint source_ppd_number = 1;

GpaModel *
gpa_ppd_add_model (const gchar *full_path)
{
	GpaModel *model;

	debug (FALSE, "");
	
	if (!gpa_ppd_to_model (full_path, &model)) {
		g_warning ("Could not create Model");
		return NULL;
	}

	if (!gpa_model_save (model))
		return NULL;
	
	return model;
}
