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

#include "gpa-ppd.h"
#include "gpa-ppd-private.h"
#include "gpa-ppd-utils.h"

#define PPD_MAX_STRING_LENGTH 30

gchar *
gpa_ppd_utils_get_string (const gchar *tag, GpaPpdInfo *ppd_info)
{
	const guchar *buffer;
	gchar *string;
	gchar *real_tag;
	gint tag_length;
	gint pos1;
	gint n;

	debug (FALSE, "");
	
	g_return_val_if_fail (tag != NULL, NULL);
	g_return_val_if_fail (ppd_info != NULL, NULL);
	g_return_val_if_fail (ppd_info->ppd != NULL, NULL);

	real_tag = g_strdup_printf ("*%s: \"", tag);
	tag_length = strlen (real_tag);
		
	pos1 = gpa_tu_search_real (ppd_info->ppd, ppd_info->ppd_size,
			       real_tag, tag_length, TRUE);

	if (pos1 < 0)
		return NULL;

	buffer = ppd_info->ppd + pos1 + tag_length;
	for (n = 0; n < PPD_MAX_STRING_LENGTH; n++)
		if (buffer [n] == '\"')
			break;

	string = g_strndup (buffer, n);
	
	return string;
}

gchar *
gpa_ppd_utils_create_id (const gchar *name)
{
	gchar *id;
	gint length;
	gint n;
	gint m;

	debug (FALSE, "");

	length = strlen (name);

	g_return_val_if_fail (length > 0, NULL);
	
	id = g_new (gchar, length+1);

	m = 0;
	
	for (n = 0; n < length; n++)
		if (name [n] != ' ')
			id [m++] = name [n];

	id [m] = 0;
	
	return id;

}

/**
 * gpa_ppd_utils_get_ui:
 * @name: 
 * @info: 
 * @ui_:
 * @real_name : returns the real name of this OpenUI code. so if the OpenUI is :
 *              "*OpenUI *OutputMode/Print Quality: PickOne" the real name is "Print Quality"
 * @jcl: specifies if the search should be done with OpenUI: or JCLOpenUI:
 * 
 * given a name for the [JLC]OpenUi tag, it searches the ppd file (which it takes from info)
 * and returns the string between OpenUI & CloseUIit in "ui_", 
 * 
 * Return Value: TRUE on success, FALSE otherwise
 **/
gboolean
gpa_ppd_utils_get_ui (const gchar *name,
		      GpaPpdInfo *info,
		      gchar **ui_,
		      gchar **real_name_,
		      gboolean jcl)
{
	gchar *buffer;
	gchar *tag1;
	gchar *tag2;
	gchar *ui;
	gint buffer_length;
	gint tag1_length;
	gint tag2_length;
	gint pos1;
	gint pos2;
	gint n;

	debug (FALSE, "");

	*ui_ = NULL;
	*real_name_ = NULL;
	
	g_return_val_if_fail (name != NULL, FALSE);
	g_return_val_if_fail (info != NULL, FALSE);

	if (jcl)
		tag1 = g_strdup_printf ("*JCLOpenUI *%s", name);
	else
		tag1 = g_strdup_printf ("*OpenUI *%s", name);
	tag1_length = strlen (tag1);
	buffer = info->ppd;
	buffer_length = info->ppd_size;
	pos1 = gpa_tu_search_real (buffer, buffer_length,
			       tag1, tag1_length, TRUE);

	if (pos1 < 0) {
		g_free (tag1);
		return FALSE;
	}
	pos1 += tag1_length;
	/* Get the real name of this OpenUI/CloseUI group */
	if (buffer [pos1] == '/') {
		gchar temp [40];
		pos1++;
		n = 0;
		while ( (buffer [pos1] != ':')  &&
			(pos1 != buffer_length) &&
			(n < 40) ) {
			temp [n++] = buffer [pos1];
			pos1++;
		}
		temp [n] = 0;
		*real_name_ = g_strdup (temp);
	}
	while (buffer [pos1] != ':')
		pos1++;
	pos1++;

	if (jcl)
		tag2 = g_strdup_printf ("*JCLCloseUI: *%s", name);
	else
		tag2 = g_strdup_printf ("*CloseUI: *%s", name);
	tag2_length = strlen (tag2);
	pos2 = gpa_tu_search_real (buffer, buffer_length,
			       tag2, tag2_length, TRUE);

	if ((pos2 < 0) || (pos2 < pos1)) {
		gpa_ppd_error ("Could not found Closing UI tag \"%s\"",
			       tag2);
		g_free (tag1);
		g_free (tag2);
		return FALSE;
	}

	buffer_length = pos2 - pos1;
	buffer += pos1;
	
	g_return_val_if_fail (buffer_length > 0, FALSE);
			      
	ui = g_new (gchar, buffer_length + 1);
	memcpy (ui, buffer, buffer_length);
	ui [buffer_length] = 0;

	*ui_ = ui;

	g_free (tag1);
	g_free (tag2);

	debug (FALSE, "");
	
	return TRUE;
}


typedef struct _GpaPpdSyntaxError GpaPpdSyntaxError;

struct _GpaPpdSyntaxError {
	gchar *text;
	gint offset;
	gboolean fatal;
};

void
gpa_ppd_append_syntax_error (GpaPpdInfo *ppd, gchar *message, gint offset, gboolean fatal)
{
	GpaPpdSyntaxError *error;

	g_return_if_fail (ppd != NULL);
	g_return_if_fail (message != NULL);
	g_return_if_fail (offset < ppd->ppd_size);

	error = g_new (GpaPpdSyntaxError, 1);
	error->text = message;
	error->offset = offset;
	error->fatal = fatal;

	ppd->errors = g_list_prepend (ppd->errors, error);
}
