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
#include "gpa-backend.h"
#include "gpa-backend-private.h"
#include "gpa-model-private.h"

#include "gpa-ppd.h"
#include "gpa-ppd-private.h"
#include "gpa-ppd-backend.h"

gboolean
gpa_ppd_add_backend (GpaPpdInfo *info)
{
	GpaBackend *backend;
	
	debug (FALSE, "");

	g_return_val_if_fail (info != NULL, FALSE);

	backend = gpa_backend_new ();
	
	backend->id     = g_strdup ("gnome-ps");
	backend->def    = TRUE;
	backend->values = g_hash_table_new (g_str_hash, g_str_equal);

	info->model->backend_list = g_list_prepend (info->model->backend_list,
						     backend);

	info->backend = backend;
	
	return TRUE;
}
