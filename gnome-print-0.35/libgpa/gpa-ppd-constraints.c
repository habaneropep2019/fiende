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

#include "text-utils.h"
#include "gpa-constraints.h"

#include "gpa-ppd.h"
#include "gpa-ppd-utils.h"
#include "gpa-ppd-private.h"
#include "gpa-ppd-constraints.h"

#include "gpa-tags.h"

#define GPA_TAG_UI_CONSTRAINTS "*UIConstraints"

gboolean
gpa_ppd_add_constraints (GpaPpdInfo *info)
{
	GpaConstraint *constraint;
	gchar *buffer;
	gchar *token1;
	gchar *token2;
	gchar *token3;
	gchar *token4;
	gchar *offending_path;
	gchar *constrained_path;
	gint buffer_length;
	gint offset;
	gint start_offset;

	g_return_val_if_fail (info != NULL, FALSE);
	
	buffer = info->ppd;
	buffer_length = info->ppd_size;


	offset = 0;
	while (TRUE) {
		if (!gpa_tu_token_next_search (buffer, buffer_length, &offset,
					       GPA_TAG_UI_CONSTRAINTS, TRUE))
			return TRUE;
		if (!gpa_tu_token_next_verify (buffer, buffer_length, &offset, ":"))
			return FALSE;
		start_offset = offset;
		token1 = gpa_tu_token_next_dup (buffer, buffer_length, &offset);
		token2 = gpa_tu_token_next_dup (buffer, buffer_length, &offset);
		token3 = gpa_tu_token_next_dup (buffer, buffer_length, &offset);
		token4 = gpa_tu_token_next_dup (buffer, buffer_length, &offset);

		if ((token1 [0] != '*') ||
		    (token3 [0] != '*')) {
			gchar *text;
			text = g_strdup_printf ("Invalid Constraint definition.\n"
						"Expected : \"*UIConstraints: *[UiGroup] [ItemId] *[UiGroup] ItemId");
			gpa_ppd_append_syntax_error (info, text, start_offset, FALSE);
			g_free (token1);
			g_free (token2);
			g_free (token3);
			g_free (token4);
			continue;
		}

		constrained_path = g_strconcat (token1 + 1, GPA_PATH_DELIMITER, token2, NULL);
		offending_path   = g_strconcat (token3 + 1, GPA_PATH_DELIMITER, token4, NULL);

#if 0	
		g_print ("Add constraint\n"
			 "Path :%s\n"
			 "Path :%s\n", constrained_path, offending_path);
#endif	
		constraint = gpa_constraint_new (offending_path,
						 constrained_path,
						 GPA_CONSTRAINT_TYPE_DOUBLE);

		g_free (constrained_path);
		g_free (offending_path);
		g_free (token1);
		g_free (token2);
		g_free (token3);
		g_free (token4);
	}

	return TRUE;
}

