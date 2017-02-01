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
#include <libart_lgpl/art_point.h>
#include <libart_lgpl/art_rect.h>

#include "gpa-ppd.h"
#include "gpa-ppd-private.h"
#include "gpa-ppd-paper.h"
#include "gpa-ppd-div.h"
#include "gpa-ppd-utils.h"
#include "text-utils.h"

#include "gpa-private.h"

#include "gpa-code.h"
#include "gpa-option.h"
#include "gpa-option-private.h"
#include "gpa-options.h"
#include "gpa-options-private.h"

#include "gpa-model-private.h"

#define GPA_PPD_TAG_VALUE_WIDTH  "$VALUE-Width-"
#define GPA_PPD_TAG_VALUE_LENGTH "$VALUE-Length-"
#define GPA_PPD_TAG_DICT "dict"
#define GPA_PPD_TAG_ID "$ID"
#define GPA_PPD_TAG_FOREACH "$FOREACH\r\n"
#define GPA_PPD_TAG_END_FOREACH "$ENDFOREACH\r\n"

typedef struct _GpaPpdPaper GpaPpdPaper;

struct  _GpaPpdPaper {
	gchar *name;
	gchar *full_name; /* Includes translation */
	gchar *code;
	ArtPoint dimension;
	ArtDRect region;

	gboolean add_code;
};


#define GPA_TAG_PAPER_SET_CODE "PaperSetCode"

/**
 * gpa_ppd_papers_set_code_not_cleaned:
 * @list: 
 * 
 * Ok, we tried to clean the code but we where unable to, just add all the setcode as a variable
 * and set the setcode to "$VALUE-SetCode-"
 * 
 * Return Value: 
 **/
static gboolean
gpa_ppd_papers_set_code_not_cleaned (GList *list_in, gchar **set_code, const gchar *start_tag, const gchar *end_tag)
{
	GpaPpdPaper *paper;
	GList *list;
		
	*set_code = g_strdup_printf ("%s" GPA_TAG_PAPER_SET_CODE "%s", start_tag, end_tag);

	list = list_in;
	for (; list != NULL; list = list->next) {
		paper = (GpaPpdPaper *)list->data;
		paper->add_code = TRUE;
	}

	return TRUE;
}

static gboolean
gpa_ppd_papers_get_set_code (GList *papers, gchar **set_code)
{
	const gchar *start_tag = "$VALUE-";
	const gchar *end_tag = "-";
	GpaPpdPaper *paper;
	GpaPpdSimpleDiv *ppd_div;
	GList *list;
	gchar **labels;
	gint size;
	
	size = 2;
	labels = g_new (gchar *, size);
	labels [0] = g_strdup (GPA_TAG_WIDTH);
	labels [1] = g_strdup (GPA_TAG_LENGTH);

	ppd_div = gpa_ppd_simple_div_new (size, labels);

	for (list = papers; list != NULL; list = list->next) {
		paper   = list->data;
		labels [0] = g_strdup_printf ("%g", paper->dimension.x);
		labels [1] = g_strdup_printf ("%g", paper->dimension.y);
		gpa_ppd_simple_div_add (ppd_div, labels, paper->code);
	}
	
	if (!gpa_ppd_simple_div_solve (ppd_div, start_tag, end_tag, set_code)) {
		/* Don't optimize, just add the paper code to each paper */
		return gpa_ppd_papers_set_code_not_cleaned (papers, set_code, start_tag, end_tag);
	}

	/* NO need to free the strings pointed by labels, those are freed by
	   gpa_ppd_div_solve */
	g_free (labels);
	
	debug (FALSE, "end");

	return TRUE;
}

static gboolean
gpa_ppd_paper_replace_values_with_dimensions (gchar **buffer_, GpaPpdPaper *paper)
{
	gchar *buffer;
	gchar *tag;
	gint replacements;

	buffer = *buffer_;
	tag = g_strdup_printf ("%g", paper->dimension.x);
	replacements = gpa_tu_replace_all (GPA_PPD_TAG_VALUE_WIDTH,
				       tag,
				       &buffer,
				       TRUE);
	g_free (tag);
	if (replacements != 1) {
		gpa_ppd_error ("Could not replace \"%s\" inside \"%s\"(4.4)\n",
			       GPA_PPD_TAG_VALUE_WIDTH,
			       buffer);
		g_free (buffer);
		return FALSE;
	}
	tag = g_strdup_printf ("%g", paper->dimension.y);
	replacements = gpa_tu_replace_all (GPA_PPD_TAG_VALUE_LENGTH,
				       tag,
				       &buffer,
				       TRUE);
	g_free (tag);
	if (replacements != 1) {
		gpa_ppd_error ("Could not replace \"%s\" inside \"%s\" (4.5)\n",
			       GPA_PPD_TAG_VALUE_LENGTH,
			       buffer);
		g_free (buffer);
		return FALSE;
	}

	*buffer_ = buffer;

	return TRUE;
}

static gboolean
gpa_ppd_papers_get_query_code  (const gchar *code, gint code_size, gchar **query_code_)
{
	const gchar *buffer;
	gchar *query_code;
	gchar *tag;
	gint buffer_length;
	gint tag_length;
	gint pos;
	gint offset;
	
	g_return_val_if_fail (code != NULL, FALSE);

	*query_code_ = NULL;
	tag = g_strdup_printf ("*?PageSize: \"");
	tag_length = strlen (tag);
	pos = gpa_tu_search_real (code, code_size,
				  tag, tag_length, TRUE);
	/* We don't require query code, so if we didn't find it just return */
	if (pos < 0) {
		*query_code_ = NULL;
		g_free (tag);
		return TRUE;
	}

	buffer = code + pos + tag_length;
	buffer_length = code_size - pos - tag_length;
	offset = 0;
	query_code  = gpa_tu_token_next_dup_till (buffer, buffer_length,
					     &offset,  '"');

	*query_code_ = query_code;

	g_free (tag);
	
	return TRUE;
}

static gboolean 
gpa_ppd_paper_replace_dimension_with_values (const gchar *buffer_in,
					     gchar **buffer_out_,
					     GpaPpdPaper *paper)
{
	const gchar *buffer;
	gchar *pre;
	gchar *post;
	gchar *set_code;
	gchar *tag;
	gint pos;
	gint offset;
	gint tag_length;
	gint buffer_length;
	
	g_return_val_if_fail (buffer_in != NULL, FALSE);
	tag = g_strdup_printf ("%g", paper->dimension.x);
	tag_length = strlen (tag);
	buffer_length = strlen (buffer_in);
	pos = gpa_tu_search_real (buffer_in, buffer_length,
			      tag, tag_length, TRUE);
	if (pos == -1) {
		gpa_ppd_error ("Could not find %s (4.1)\n", tag);
		g_free (tag);
		return FALSE;
	}
	g_free (tag);

	buffer = buffer_in + tag_length + pos;
	buffer_length -= tag_length + pos;
	offset = 0;
	tag = g_strdup_printf ("%g]", paper->dimension.y);
	if (!gpa_tu_token_next_verify (buffer, buffer_length, &offset, tag)) {
		gpa_ppd_error ("Expected \"%s\" not found in getting setcode for paper\n",
			       tag);
		g_free (tag);
		return FALSE;
	}

	pre  = g_strndup (buffer_in, pos);
	post = g_strndup (buffer + offset - 1, buffer + offset - buffer_in);
	set_code = g_strdup_printf ("%s%s %s%s",
				    pre,
				    GPA_PPD_TAG_VALUE_WIDTH,
				    GPA_PPD_TAG_VALUE_LENGTH,
				    post);

	g_free (pre);
	g_free (post);

	*buffer_out_ = set_code;

	return TRUE;
}

/* This function is _REALLY_ messsy but it works. I have no clue on how to beautify it,
   patches wellcomed. Chema */
static gboolean
gpa_ppd_papers_clean_query_code (GList *papers, gchar **query_code_)
{
	const gchar *buffer;
	const gchar *buffer_max;
	GpaPpdPaper *paper;
	GList *list;
	gchar *query_code;
	gchar *token;
	gchar *line;
	gchar *pre;
	gchar *body;
 	gchar *post;
	gint token_length;
	gint tag_length;
	gint buffer_length;
	gint pos;
	gint offset;
	gint num;
	gint n;

	query_code = *query_code_;
	if (query_code == NULL)
		return TRUE;
	
	g_return_val_if_fail (papers != NULL, FALSE);

	/* 1. search for "dict"
	   2. get the previous token and verify that it corresponds
	      to the number of papers we have
	   3. get the next line after the dict line
	   4. determine from the line, the paper size it refers to
	      by scaning the list of papers and searching for it
	      inside the line
	   5. replace in that line with $VALUE-...etc
	   6. verify that if we do the replacement the line will
	      be the same
	   7. get the body
	   8. get the pre and replace # with $OPTIONSNUM
	   9. get the post
	   10. generate the query_code with foreach and endforeach,
	       based in body, pre & post
	*/

	/* 1 */
	buffer = query_code;
	buffer_length = strlen (query_code);
	tag_length = strlen (" " GPA_PPD_TAG_DICT);
	pos = gpa_tu_search_real (buffer, buffer_length,
				  " " GPA_PPD_TAG_DICT, tag_length, TRUE);
	if (pos < 0) {
		gpa_ppd_error ("Could not find \"%s\" in query_code (4.7)",
			       " " GPA_PPD_TAG_DICT);
		return FALSE;
	}
	/* 2 */
	buffer = buffer + pos - 1;
	buffer_length = pos;
	offset = 0;
	token = gpa_tu_token_previous_dup (buffer, buffer_length, &offset);
	num = atoi (token);
	g_free (token);
	if (num != g_list_length (papers)) {
		gpa_ppd_error ("The papers dict lenght does not equal the papers "
			       "found [num:%i,list_length:%i]", num, g_list_length (papers));
		return FALSE;
	}
	/* 3 */
	while ((*buffer != '\r') &&
	       (*buffer != '\n'))
		buffer++;
	if ((*buffer == '\r') ||
	    (*buffer == '\n'))
		buffer ++;
	offset = 0;

	/* 4 */
	buffer_length = strlen (query_code) - (buffer-query_code);
	token = gpa_tu_token_next_dup_till_newline (buffer, buffer_length, &offset);
	token_length = strlen (token);
	for (list = papers; list != NULL; list = list->next) {
		paper = list->data;
		pos = gpa_tu_search_real (token, token_length,
					      paper->name, 0, TRUE);
		if (pos > -1)
			break;
	}
	if (list == NULL) {
		gpa_ppd_error ("Could not determine the paper from the"
			       " query_code line \"%s\" (4.9)", token);
		g_free (token);
		return FALSE;
	}

	/* 5 */
	if (!gpa_ppd_paper_replace_dimension_with_values (token, &line, paper))
		return FALSE;

	/* 7 */
	body = g_strdup_printf ("%s\r\n", line);
	if (1 != gpa_tu_replace_all (paper->name,
					 GPA_PPD_TAG_ID,
					 &body,
					 TRUE)) {
		gpa_ppd_error ("Could not replace \"%s\" in \"%s\" (4.3)",
			       paper->name, body);
		return FALSE;
	}

	/* 6 */
	if (!gpa_ppd_paper_replace_values_with_dimensions (&line, paper))
		return FALSE;
	if (strcmp (token, line) != 0) {
		gpa_ppd_error ("The line generated by replacing and the actual line "
			       "does not match [->%s<-,->%s<-]", line, token);
		g_free (body);
		g_free (line);
		g_free (token);
		return FALSE;
	}
	g_free (line);
	g_free (token);		

	/* 8 */
	buffer = query_code;
	buffer_length = strlen (query_code);
	tag_length = strlen (GPA_PPD_TAG_DICT);
	pos = gpa_tu_search_real (buffer, buffer_length,
				      GPA_PPD_TAG_DICT, tag_length, TRUE);
	if (pos < 0) {
		gpa_ppd_error ("Could not find \"%s\" in query_code (4.2)",
			       GPA_PPD_TAG_DICT);
		return FALSE;
	}
	if ((buffer [pos + tag_length + 0] == '\n') ||
	    (buffer [pos + tag_length + 0] == '\r'))
		buffer++;
	if ((buffer [pos + tag_length + 1] == '\n') ||
	    (buffer [pos + tag_length + 1] == '\r'))
		buffer++;
	pre = g_strndup (buffer, pos + tag_length);
	token = g_strdup_printf ("%i dict", g_list_length (papers));
	if (1 !=  gpa_tu_replace_all (token, "$OPTIONUM dict",
					  &pre, TRUE)) {
		gpa_ppd_error ("Error while replacing OPTIONSNUM inside pre. [%s][%s]",
			       pre, token);
		g_free (token);
		g_free (pre);
		return FALSE;
	}
	g_free (token);
	
	/* 9 */
	buffer = query_code + pos;
	buffer_length = strlen (buffer);
	buffer_max = buffer + buffer_length;
	for (n = 0; n < num+1 ; n++)
		while (*buffer++ != '\n')
			if (buffer > buffer_max ) {
				g_warning ("Could not deterimine post code for paper,"
					   " overflow");
				return FALSE;
			}
	post = g_strdup (buffer);

	/* 10 */
	g_free (query_code);
	query_code = g_strdup_printf ("%s%s%s%s%s",
				      pre,
				      GPA_PPD_TAG_FOREACH,
				      body,
				      GPA_PPD_TAG_END_FOREACH,
				      post);
	g_free (pre);
	g_free (body);
	g_free (post);

	*query_code_ = query_code;

	
	return TRUE;	
}




static GpaCodeFragment *
gpa_ppd_paper_code_fragment_new (GpaPpdInfo *info,
				 const gchar *code,
				 gboolean is_query_code)
{
	GpaCodeFragment *fragment;
	
	g_return_val_if_fail (info          != NULL, NULL);
	g_return_val_if_fail (info->backend != NULL, NULL);
	g_return_val_if_fail (code          != NULL, NULL);

	fragment = gpa_code_fragment_new ();
	if (is_query_code)
		fragment->id = g_strdup (GPA_TAG_QUERY);
	else
		fragment->id = g_strdup (GPA_TAG_SET);
	fragment->content = g_strdup (code);
	fragment->backend = info->backend;
		
	return fragment;
}


static gboolean
gpa_ppd_papers_get_region (GpaPpdInfo *ppd_info, GList *papers)
{
	GpaPpdPaper *paper;
	GList *list;
	guchar *buffer;
	gchar *tag;
	gchar *token;
	gint buffer_size;
	gint tag_length;
	gint pos;
	gint offset;
	gboolean dump = FALSE;
	
	debug (FALSE, "");

	g_return_val_if_fail (ppd_info != NULL, FALSE);
	g_return_val_if_fail (papers != NULL, FALSE);

	for (list = papers; list != NULL; list = list->next) {
		buffer = ppd_info->ppd;
		buffer_size = ppd_info->ppd_size;
		paper = list->data;
		g_return_val_if_fail (paper->name != NULL, FALSE);
		tag = g_strdup_printf ("*ImageableArea %s", paper->full_name);
		tag_length = strlen (tag);
		while (TRUE) {
			pos = gpa_tu_search_real (buffer, buffer_size,
						      tag, tag_length, TRUE);
			if (pos == -1) {
				gpa_ppd_error ("Could not find \"%s\" (1.5)\n", tag);
				g_free (tag);
				return FALSE;
			}
			/* We do this since we might be looking for "D5" and will
			   find "D5 Envelope" as correct, so look again till we find
			   the real string we are lookgin for (Chema) */
			if ((buffer [pos + tag_length] != ':') &&
			    (buffer [pos + tag_length + 1] != ':') &&
			    (buffer [pos + tag_length + 2] != ':')) {
				/* Isn't this hacky ? :-)*/
				buffer += pos + tag_length;
				continue;
			}
			break;
		}
		g_free (tag);
		buffer += pos + tag_length;
		offset = 0;
		if (!gpa_tu_token_next_till (buffer, buffer_size, &offset, ':')) {
			gpa_ppd_error ("Could not find %s (1.4)\n", tag);
			return FALSE;
		}
		if (!gpa_tu_token_next_till (buffer, buffer_size, &offset, '"')) {
			gpa_ppd_error ("Could not find %s (1.4.2)\n", tag);
			return FALSE;
		}

		/* 1 */
		token = gpa_tu_token_next_dup (buffer, buffer_size, &offset);
		if (dump)
			g_print ("Imageable area 1 :%s\n", token);
		if (gpa_tu_string_contains_newline (token))
			goto gpa_ppd_paper_get_region_error;
		paper->region.x0 = atof (token);
		g_free (token);

		/* 2 */
		token = gpa_tu_token_next_dup (buffer, buffer_size, &offset);
		if (dump)
			g_print ("Imageable area 2 :%s\n", token);
		if (gpa_tu_string_contains_newline (token))
			goto gpa_ppd_paper_get_region_error;
		paper->region.y0 = atof (token);
		g_free (token);

		/* 3 */
		token = gpa_tu_token_next_dup (buffer, buffer_size, &offset);
		if (dump)
			g_print ("Imageable area 3 :%s\n", token);
		if (gpa_tu_string_contains_newline (token))
			goto gpa_ppd_paper_get_region_error;
		paper->region.x1 = atof (token);
		g_free (token);

		/* 4 */
		token = gpa_tu_token_next_dup_till (buffer, buffer_size, &offset, '"');
		if (dump)
			g_print ("Imageable area 4 :%s\n", token);
		if (gpa_tu_string_contains_newline (token))
			goto gpa_ppd_paper_get_region_error;
		paper->region.y1 = atof (token);
		g_free (token);

	}

	return TRUE;

gpa_ppd_paper_get_region_error:
	gpa_error ("An error was encountered while trying to determine the ImageableArea for \"%s\"",
		   paper->full_name);
	return FALSE;
	
}

static gboolean
gpa_ppd_papers_get_dimension (GpaPpdInfo *ppd_info, GList *papers)
{
	GpaPpdPaper *paper;
	GList *list;
	guchar *buffer;
	gchar *tag;
	gchar *token;
	gint buffer_size;
	gint tag_length;
	gint pos;
	gint offset;
	gboolean dump = FALSE;
	
	debug (FALSE, "");

	g_return_val_if_fail (ppd_info != NULL, FALSE);
	g_return_val_if_fail (papers != NULL, FALSE);

	for (list = papers; list != NULL; list = list->next) {
		buffer = ppd_info->ppd;
		buffer_size = ppd_info->ppd_size;
		paper = list->data;
		g_return_val_if_fail (paper->name != NULL, FALSE);
		tag = g_strdup_printf ("*PaperDimension %s", paper->full_name);
		tag_length = strlen (tag);
		while (TRUE) {
			pos = gpa_tu_search_real (buffer, buffer_size,
						      tag, tag_length, TRUE);
			if (pos == -1) {
				gpa_ppd_error ("Could not find \"%s\" (1.5.2)-->%s<--\n", tag, buffer);
				g_free (tag);
				return FALSE;
			}
			/* We do this since we might be looking for "D5" and will
			   find "D5 Envelope" as correct, so look again till we find
			   the real string we are lookgin for (Chema) */
			if ((buffer [pos + tag_length] != ':') &&
			    (buffer [pos + tag_length + 1] != ':') &&
			    (buffer [pos + tag_length + 2] != ':')) {
				/* Isn't this hacky ;-) ? */
				buffer += pos + tag_length;
				continue;
			}
			break;
		}
		g_free (tag);
			
		buffer += pos + tag_length;
		offset = 0;
		if (!gpa_tu_token_next_till (buffer, buffer_size, &offset, ':')) {
			gpa_ppd_error ("Could not find %s (1.6)\n", tag);
			return FALSE;
		}
		if (!gpa_tu_token_next_till (buffer, buffer_size, &offset, '"')) {
			gpa_ppd_error ("Could not find %s (1.6.2)\n", tag);
			return FALSE;
		}
		
		/* 1 */
		token = gpa_tu_token_next_dup (buffer, buffer_size, &offset);
		if (dump)
			g_print ("Paper Dimesion 1 :%s\n", token);
		if (gpa_tu_string_contains_newline (token))
			goto gpa_ppd_paper_get_dimension_error;
		paper->dimension.x = atof (token);
		g_free (token);

		/* 2 */
		token = gpa_tu_token_next_dup (buffer, buffer_size, &offset);
		if (dump)
			g_print ("Paper Dimesion 2 :%s\n", token);
		if (gpa_tu_string_contains_newline (token))
			goto gpa_ppd_paper_get_dimension_error;
		paper->dimension.y = atof (token);
		g_free (token);
		
		g_return_val_if_fail (paper->dimension.x != 0, FALSE);
		g_return_val_if_fail (paper->dimension.y != 0, FALSE);
	}

	return TRUE;

gpa_ppd_paper_get_dimension_error:
	gpa_error ("An error was encountered while trying to determine the PaperDimension for \"%s\"",
		   paper->full_name);
	return FALSE;
}


static GList *
gpa_ppd_get_papers (const gchar *code, gint code_size)
{
	GpaPpdPaper *paper;
	const gchar *buffer;
	gchar *paper_name;
	gchar *paper_full_name;
	gchar *paper_code;
	gchar *token;
	gchar *tag;
	GList *list = NULL;
	gint buffer_size;
	gint tag_length;
	gint pos = 0;
	gint offset = 0;

	debug (FALSE, "");

	g_return_val_if_fail (code != NULL, NULL);
	
	buffer = code;
	buffer_size = code_size;
	tag = g_strdup_printf ("*%s ", GPA_PPD_TAG_PAGE_SIZE);
	tag_length = strlen (tag);

	while (TRUE) {
		pos = gpa_tu_search_real (buffer, buffer_size,
					      tag, tag_length, TRUE);
		if (pos == -1)
			break;
		/* We advance the buffer, since we don't care about the past */
		buffer = buffer + pos + tag_length;
		buffer_size = buffer_size - pos - tag_length;
		offset = 0;
		
		/* Get & verify the paper by tokenizing it */
		token = gpa_tu_token_next_dup_till (buffer, buffer_size,
						    &offset, ':');

		pos = gpa_tu_search_real (token, strlen (token),
					      "/", 1, TRUE);
		paper_full_name = g_strdup (token);
		gpa_tu_remove_trailing_spaces (&paper_full_name);
 		if (pos == -1) {
			paper_name = token;
		} else {
			paper_name = g_strndup (token, pos);
			g_free (token);
		}
		if (buffer [offset] == ' ')
			offset++;
		
		if (buffer [offset] != '\"') {
			offset = 0;
			buffer_size = code_size - (buffer-code);
			token = gpa_tu_token_next_dup_till_newline (buffer,	buffer_size,
								    &offset);
			gpa_ppd_error ("Expected \"\"\" not found in -->%s<-- (2.2)",
				       token);
			g_free (token);
			return NULL;
		}
		offset++;
		
		paper_code  = gpa_tu_token_next_dup_till (buffer, buffer_size,
						     &offset, '"');
		if (strlen (paper_code) == 0) {
			g_free (paper_code);
			paper_code = NULL;
		}

		paper = g_new (GpaPpdPaper, 1);
		paper->name = paper_name;
		paper->code = paper_code;
		paper->full_name = paper_full_name;
		paper->add_code = FALSE;
		list = g_list_prepend (list, paper);
	}

	g_free (tag);
	
	return g_list_reverse (list);
}

static GpaOption *
gpa_ppd_get_option_from_paper (GpaPpdPaper *paper, GpaOptions *parent)
{
	GpaOption *option;
	GHashTable *hash;
	gchar *id;
	gchar *width;
	gchar *length;
	gchar *region_x0;
	gchar *region_y0;
	gchar *region_x1;
	gchar *region_y1;
	
	debug (FALSE, "");

	g_return_val_if_fail (paper != NULL, NULL);

	id = gpa_ppd_utils_create_id (paper->name);
	if (id == NULL)
		return NULL;
	option = gpa_option_new (paper->name, id, parent);
	hash = g_hash_table_new (g_str_hash, g_str_equal);

	width  = g_strdup_printf ("%g", paper->dimension.x);
	length = g_strdup_printf ("%g", paper->dimension.y);
	region_x0 = g_strdup_printf ("%g", paper->region.x0);
	region_y0 = g_strdup_printf ("%g", paper->region.y0);
	region_x1 = g_strdup_printf ("%g", paper->region.x1);
	region_y1 = g_strdup_printf ("%g", paper->region.y1);
	
	g_hash_table_insert (hash, GPA_TAG_LENGTH, length);
	g_hash_table_insert (hash, GPA_TAG_WIDTH,  width);
	g_hash_table_insert (hash, GPA_TAG_REGION_X0, region_x0);
	g_hash_table_insert (hash, GPA_TAG_REGION_Y0, region_y0);
	g_hash_table_insert (hash, GPA_TAG_REGION_X1, region_x1);
	g_hash_table_insert (hash, GPA_TAG_REGION_Y1, region_y1);

	if (paper->add_code)
		g_hash_table_insert (hash, GPA_TAG_PAPER_SET_CODE, paper->code);
		
	option->values = hash;
	
	g_free (id);
	
	return option;
}

static gint
gpa_ppd_add_paper_code_fragments (GpaPpdInfo *info, GpaOptions *options,
				  const gchar *query_code, const gchar *set_code)
{
	GpaCodeFragment *fragment;
	GList *fragments_list;

	debug (FALSE, "");

	g_return_val_if_fail (info != NULL, FALSE);
	g_return_val_if_fail (options != NULL, FALSE);
	g_return_val_if_fail (options->code_fragments == NULL, FALSE);
     
	fragments_list = options->code_fragments;

	if (query_code != NULL) {
		fragment = gpa_ppd_paper_code_fragment_new (info, query_code, TRUE);
		if (fragment == NULL)
			return FALSE;
		fragments_list = g_list_prepend (fragments_list, fragment);
	}
	if (set_code != NULL) {
		fragment = gpa_ppd_paper_code_fragment_new (info, set_code, FALSE);
		if (fragment == NULL)
			return FALSE;
		fragments_list = g_list_prepend (fragments_list, fragment);
	}

	options->code_fragments = g_list_reverse (fragments_list);

	return TRUE;
}


static gchar*
gpa_ppd_dup_ui_code (GpaPpdInfo *ppd_info, gint *size, const gchar *ui_name)
{
	gchar *tag;
	gchar *buffer;
	gchar *code;
	gint buffer_size;
	gint tag_length;
	gint pos_ini;
	gint pos_end;

	debug (FALSE, "");

	g_return_val_if_fail (ppd_info != NULL, NULL);
	g_return_val_if_fail (ui_name != NULL, NULL);

	tag = g_strdup_printf ("*OpenUI *%s", ui_name);
	tag_length = strlen (tag);

	buffer = ppd_info->ppd;
	buffer_size = ppd_info->ppd_size;
	
	pos_ini = gpa_tu_search_real (buffer, buffer_size,
				      tag, tag_length, TRUE);

	if (pos_ini < 0) {
		gpa_ppd_error ("Could not find %s (1.1)", tag);
		return NULL;
	}

	/* Advance till we find a newline */
	while (buffer [pos_ini] != '\n' &&
	       buffer [pos_ini] != '\r')
		pos_ini ++;
	while (buffer [pos_ini] == '\n' ||
	       buffer [pos_ini] == '\r')
		pos_ini ++;

	g_free (tag);

	
	tag = g_strdup_printf ("*CloseUI: *%s", ui_name);
	tag_length = strlen (tag);
	buffer += pos_ini;
	buffer_size += pos_ini;
	
	pos_end = gpa_tu_search_real (buffer, buffer_size,
					  tag, tag_length, TRUE);

	if (pos_end < 0) {
		gpa_ppd_error ("Could not find %s (1.2)", tag);
		return NULL;
	}

	*size = pos_end;
	code = g_strndup (buffer, *size);

	return code;
}


gboolean
gpa_ppd_add_paper_options (GpaPpdInfo *ppd_info)
{
	GpaOptions *options;
	GpaOption *option;
	GpaPpdPaper *paper;
	GList *options_list;
	GList *papers;
	GList *list;
	gchar *ui_code;
	gchar *set_code;
	gchar *query_code;
	gint ui_code_size;
	/* We need to :
	   1. Get the papers and put them in the GList
	   2. Get the each paper dimension
	   3. Get each paper imageable region
	   4. Get the set & query code from the papers
	   5. Macth them with a GnomePaper
	   7. Get the PaperDimensions
	   8. Get the Imageable Area
	   9. Get the Constaints
	 */

	debug (FALSE, "");

	ui_code = gpa_ppd_dup_ui_code (ppd_info, &ui_code_size, GPA_PPD_TAG_PAGE_SIZE);
	if (ui_code == NULL)
		return FALSE;
	
	papers = gpa_ppd_get_papers    (ui_code, ui_code_size);
	if (papers == NULL)
		return FALSE;
	if (!gpa_ppd_papers_get_dimension    (ppd_info, papers))
		return FALSE;
	if (!gpa_ppd_papers_get_region       (ppd_info, papers))
		return FALSE;
	if (!gpa_ppd_papers_get_set_code     (papers, &set_code))
		return FALSE;
	if (!gpa_ppd_papers_get_query_code   (ui_code, ui_code_size, &query_code))
		return FALSE;
	if (!gpa_ppd_papers_clean_query_code (papers, &query_code)) {
		gpa_ppd_warning ("The query code could not be cleaned. We will just include all the code here\n");
	}

	options = gpa_options_new (ppd_info->model,
				   "Paper Size",
				   GPA_TAG_MEDIA_SIZE,
				   GPA_GROUP_PAPER);
	options->options_type = GPA_OPTIONS_TYPE_PICKONE;

	ppd_info->model->options_list = g_list_prepend (ppd_info->model->options_list,
							 options);

	
	options_list = options->children;
	for (list = papers; list != NULL; list = list->next) {
		paper = list->data;
		option = gpa_ppd_get_option_from_paper (paper, options);
		options_list = g_list_prepend (options_list, option);
	}
	options->children = g_list_reverse (options_list);

	/* Add the code fragments */
	gpa_ppd_add_paper_code_fragments (ppd_info, options, query_code, set_code);

	return TRUE;
}

