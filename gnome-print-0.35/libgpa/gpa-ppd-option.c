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

#include <string.h> /* For strcasecmp */

#include <glib.h>

#include "gpa-private.h"
#include "gpa-option.h"
#include "gpa-option-private.h"
#include "gpa-options-private.h"
#include "text-utils.h"

#include "gpa-ppd.h"
#include "gpa-ppd-option.h"
#include "gpa-ppd-utils.h"



gboolean
gpa_ppd_create_option_from_token (const gchar *token,
				  const gchar *token2,
				  GpaOptions *parent)
{
	GpaOption *option;
	GHashTable *hash;
	gchar *name;
	gchar *id;
	gchar *code;
	gint pos1;
	gint pos2;

	debug (FALSE, "");

	g_return_val_if_fail (token != NULL, FALSE);
	g_return_val_if_fail (parent != NULL, FALSE);

	pos1 = strlen (token);
	pos2 = gpa_tu_strnchr (token, '/');
	
	if (pos2 > 0) {
		id   = g_strndup (token, pos2 - 1);
		name = g_strndup (token + pos2, pos1 - pos2 );
	} else {
		name = g_strndup (token, pos1);
		id   = gpa_ppd_utils_create_id (name);
	}

	/* For boolean options, change "False/True" to "FALSE/TRUE" */
	if (strcasecmp (id, GPA_TAG_TRUE) == 0) {
		g_free (id);
		id = g_strdup (GPA_TAG_TRUE);
	} else if (strcasecmp (id, GPA_TAG_FALSE) == 0) {
		g_free (id);
		id = g_strdup (GPA_TAG_FALSE);
	}
	
	option = gpa_option_new (name, id, parent);
	hash = g_hash_table_new (g_str_hash, g_str_equal);
	option->values = hash;

	if ( *token2 != 0 ) {
		code = g_strdup (token2);
		g_hash_table_insert (hash, GPA_PPD_TAG_SET_CODE, code);
	}
	
	g_free (id);
	g_free (name);

	parent->children = g_list_prepend (parent->children, option);
	
	return TRUE;
}
