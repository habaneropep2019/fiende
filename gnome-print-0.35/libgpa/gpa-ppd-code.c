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

#include <stdlib.h> /* For atoi */

#include "gpa-private.h"

#include "gpa-model-private.h"

#include "gpa-options-private.h"

#include "text-utils.h"
#include "gpa-code.h"

#include "gpa-ppd.h"
#include "gpa-ppd-private.h"
#include "gpa-ppd-code.h"


static gboolean
gpa_ppd_add_code_fragment (const gchar *name, GpaPpdInfo *info)
{
	GpaCodeFragment *fragment;
	GList *list;
	gchar *buffer;
	gchar *tag;
	gchar *token;
	gint offset;
	gint buffer_length;
	gint tag_length;
	gint pos;

	debug (FALSE, "");

	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (info != NULL, FALSE);

	buffer = info->ppd;
	buffer_length = info->ppd_size;
	tag = g_strdup_printf ("*%s:", name);
	tag_length = strlen (tag);
	pos = gpa_tu_search_real (buffer, buffer_length,
			      tag, tag_length, TRUE);
	g_free (tag);
	
	/* This code fragment was not found in the file */
	if (pos < 0)
		return TRUE;

	buffer += pos + tag_length;
	buffer_length -= (pos + tag_length);
	offset = 0;
	if (!gpa_tu_token_next_verify (buffer, buffer_length, &offset, "\"")) {
		return FALSE;
	}

	token = gpa_tu_token_next_dup_till (buffer, buffer_length, &offset, '\"');

	fragment = gpa_code_fragment_new ();
	fragment->id       = g_strdup (name);
	fragment->encoding = GPA_ENCODING_UNENCODED;
	fragment->content  = token;
	fragment->backend  = info->backend;

	list = info->model->code_fragments;
	list = g_list_prepend (list, fragment);
	info->model->code_fragments = list;

	return TRUE;
}

static const gchar * gpa_known_ps_code_fragments [] = {
	"ExitServer",
	"Reset",
	"?FileSystem",
};

gboolean
gpa_ppd_add_code_fragments (GpaPpdInfo *info)
{
	GList *list;
	gint number;
	gint n;

	debug (FALSE, "");

	g_return_val_if_fail (info != NULL, FALSE);

	list = info->model->code_fragments;

	number = sizeof (gpa_known_ps_code_fragments) / sizeof(gchar *);
	for (n = 0; n < number; n++)
		if (!gpa_ppd_add_code_fragment (gpa_known_ps_code_fragments [n], info))
			return FALSE;

	info->model->code_fragments = g_list_reverse (info->model->code_fragments);
	
	return TRUE;
}
			    

gboolean
gpa_ppd_add_options_code_fragment (GpaPpdInfo *info,
				   gchar *code,
				   GpaOptions *options)
{
	GpaCodeFragment *fragment;
	
	debug (FALSE, "");

	g_return_val_if_fail (code != NULL, FALSE);
	g_return_val_if_fail (options != NULL, FALSE);

	fragment = gpa_code_fragment_new ();
	fragment->id       = g_strdup (GPA_TAG_QUERY);
	fragment->backend  = info->backend;
	fragment->content  = code;
	fragment->encoding = GPA_ENCODING_UNENCODED;

	options->code_fragments = g_list_prepend (options->code_fragments, fragment);

	return TRUE;
}

