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

#include <string.h> /* For strncmp */

#include "gpa-private.h"
#include "gpa-model-private.h"
#include "gpa-option.h" 
#include "gpa-option-private.h" 
#include "gpa-options.h"
#include "gpa-options-private.h"
#include "gpa-code.h"
#include "text-utils.h"

#include "gpa-ppd.h"
#include "gpa-ppd-private.h"
#include "gpa-ppd-code.h"
#include "gpa-ppd-div.h"
#include "gpa-ppd-option.h"
#include "gpa-ppd-options.h"
#include "gpa-ppd-paper.h"
#include "gpa-ppd-utils.h"

typedef struct {
	const gchar    *ppd_name;
	const gchar    *name;
	gboolean        jlc;
	GpaOptionsGroup group;
} GpaKnownPpdOptions;

const GpaKnownPpdOptions gpa_known_ppd_options [] = {
	{"ManualFeed",      "Manual Feed",            FALSE, GPA_GROUP_PAPER},
	{"JCLPageProtect",  "Legal Frame Size",        TRUE, GPA_GROUP_PAPER},
	{"InputSlot",       "Paper Source",           FALSE, GPA_GROUP_PAPER},
	{"JCLEconomode",    "Economy Mode",            TRUE, GPA_GROUP_QUALITY},
	{"JCLRET",          "Resolution Enhancement",  TRUE, GPA_GROUP_QUALITY},
	{"InstalledMemory", "Installed Memory",       FALSE, GPA_GROUP_INSTALLED},
	{"EnvelopeFeed",    "Envelope Feeder",        FALSE, GPA_GROUP_INSTALLED},
	{"OutputMode",       NULL,                    FALSE, GPA_GROUP_QUALITY},
	{"Option1",          NULL,                    FALSE, GPA_GROUP_INSTALLED},
	{"Option2",          NULL,                    FALSE, GPA_GROUP_INSTALLED},
};


/**
 * gpa_ppd_set_default_option_from_token:
 * @info: 
 * @def: 
 * @options: 
 * 
 * Sets the default option inside @Options from the option specified in @def
 * 
 * Return Value: 
 **/
static gboolean
gpa_ppd_set_default_option_from_token (const GpaPpdInfo *info, const gchar *def, GpaOptions *options)
{
	GpaModel *model;
	GpaOption *option;
	GList *list;
	gchar *id;
	gchar *path;

	debug (FALSE, "");

	if (strcmp (def, "Unknown") == 0)
		return TRUE;

	model = info->model;
		
	id = gpa_ppd_utils_create_id (def);
	
	for (list = options->children; list != NULL; list = list->next) {
		option = list->data;
		if (strcasecmp (option->id, id) == 0) {
			path = gpa_option_dup_path (option);
			model->default_settings = g_list_prepend (model->default_settings, path);
			break;
		}
	}

	g_free (id);

	if (list == NULL) {
		gpa_ppd_error ("Could not set the default option \"%s\" for \"%s\"",
			       def, options->name);
	}

	return TRUE;
}


/**
 * gpa_options_add_setcode:
 * @options: 
 * @info: 
 * 
 * Adds the setcode (GpaCodeFragment) to @Options
 **/
static void
gpa_options_add_setcode (GpaOptions *options, GpaPpdInfo *info)
{
	const gchar *code;
	GpaCodeFragment *fragment;
	GpaOption *option;
	GpaPpdDiv *ppd_div;
	GList *list;
	gchar *setcode;

	ppd_div = gpa_ppd_div_new ();
	
	list = options->children;
	for (; list != NULL; list = list->next) {
		option = list->data;
		code = g_hash_table_lookup (option->values, GPA_PPD_TAG_SET_CODE);
		gpa_ppd_div_add (ppd_div, code, option->values);
	}

	gpa_ppd_div_solve (ppd_div, &setcode);
	
	if ((setcode == NULL) || (setcode [0] == 0))
		return;

	fragment = gpa_code_fragment_new ();
	fragment->id       = g_strdup (GPA_TAG_SET);
	fragment->backend  = info->backend;
	fragment->content  = setcode;
	fragment->encoding = GPA_ENCODING_UNENCODED;

	options->code_fragments = g_list_prepend (options->code_fragments, fragment);
	
}


/**
 * gpa_ppd_options_are_boolean:
 * @options: 
 * 
 * We treat boolean PPD options as Pickone, this function determines if the pickone
 * options that we just created are valid boolean options for out "idea" of boolean
 *
 * Return Value: TRUE if options are boolean, FALSE otherwise
 **/
static gboolean
gpa_ppd_options_are_boolean (GpaOptions *options)
{
	GpaOption *option;
	
	g_return_val_if_fail (GPA_IS_OPTIONS (options), FALSE);

	if (g_list_length (options->children) != 2)
		return FALSE;

	option = options->children->data;
	g_return_val_if_fail (GPA_IS_OPTION (option), FALSE);
	
	if ( (strcmp (option->id, GPA_TAG_TRUE) != 0) &&
	     (strcmp (option->id, GPA_TAG_FALSE) != 0))
		return FALSE;

	option = options->children->next->data;
	g_return_val_if_fail (GPA_IS_OPTION (option), FALSE);

	if ( (strcmp (option->id, GPA_TAG_TRUE) != 0) &&
	     (strcmp (option->id, GPA_TAG_FALSE) != 0))
		return FALSE;

	return TRUE;	
}


/**
 * gpa_ppd_add_options_pickone:
 * @info: PPD file info
 * @ui: 
 * @ui_length: 
 * @options: 
 * @name: 
 * @jcl: True if this is a *JCLFoo FALSe if it a *Foo
 * 
 * Parses and creates a PickOne Options object.
 * 
 * Return Value: 
 **/
static gboolean
gpa_ppd_add_options_pickone (GpaPpdInfo *info,
			     const gchar *ui,
			     gint ui_length,
			     GpaOptions *options,
			     const gchar *name,
			     gboolean jcl)
{
	const gchar *buffer;
	gboolean dump;
	gchar *token;
	gchar *token2;
	gchar *def = NULL;
	gchar *code = NULL;
	gchar *option_tag;
	gchar *code_tag;
	gchar *order_tag;
	gchar *end_tag;
	gchar *default_tag;
	gint option_tag_length;
	gint code_tag_length;
	gint order_tag_length;
	gint end_tag_length;
	gint default_tag_length;
	gint buffer_length;
	gint offset;

	debug (FALSE, "");

	g_return_val_if_fail (info != NULL,    FALSE);
	g_return_val_if_fail (ui != NULL,      FALSE);
	g_return_val_if_fail (options != NULL, FALSE);
	g_return_val_if_fail (name != NULL,    FALSE);

	buffer = ui;
	buffer_length = ui_length;
	offset = 0;

	option_tag  = g_strdup_printf ("*%s", name);
	code_tag    = g_strdup_printf ("*?%s:", name);
	order_tag   = g_strdup (GPA_PPD_TAG_ORDER_DEPENDENCY);
	end_tag     = g_strdup (GPA_PPD_TAG_END);
	default_tag = g_strdup (GPA_PPD_TAG_DEFAULT);

	option_tag_length  = strlen (option_tag);
	code_tag_length    = strlen (code_tag);
	order_tag_length   = strlen (order_tag);
	end_tag_length     = strlen (end_tag);
	default_tag_length = strlen (GPA_PPD_TAG_DEFAULT);

	dump = TRUE;
	
	while (TRUE) {
		/* Read token an do some basic checks */
		token = gpa_tu_token_next_dup (buffer, buffer_length, &offset);
		if (dump)
			g_print ("Token -->%s<--\n", token);
		/* Are we done ? */
		if ((token == NULL) || token [0] == 0)
			break;
		if (token [0] != '*') {
			gpa_ppd_error ("Parsing error while reading \"%s\". "
				       "Expected \"*\" as the first character", name);
			goto gpa_ppd_add_options_pickone_error;
		}
		/* If token specifies an option */
		if (strncmp (token, option_tag, option_tag_length) == 0) {
			g_free (token);
			token = gpa_tu_token_next_dup_till (buffer, buffer_length,
							&offset, ':');
			gpa_tu_token_next_till (buffer, buffer_length,
					    &offset, '\"');
			token2 = gpa_tu_token_next_dup_till (buffer, buffer_length,
							&offset, '\"');
			if (dump) {
				g_print ("Option token -->%s<--\n", token);
				g_print ("Option token -->%s<--\n", token2);
			}
			if (!gpa_ppd_create_option_from_token (token, token2, options))
				goto gpa_ppd_add_options_pickone_error;
			g_print ("Ok\n");
			g_free (token);
			g_free (token2);
		/* If token is a default token */
		} else if (strncmp (token, default_tag, default_tag_length)== 0) {
			if (def != NULL) {
				gpa_ppd_error ("The [JCL]OpenUI \"%s\" Contains 2 default "
					       "tags\n", name);
				goto gpa_ppd_add_options_pickone_error;
			}
			def = gpa_tu_token_next_dup (buffer, buffer_length, &offset);
			if (dump)
				g_print ("Default token -->%s<-->%s<--\n", token, def);
		/* If token is a query code token */
		} else 	if (strncmp (token, code_tag, code_tag_length) == 0) {
			gpa_tu_token_next_till (buffer, buffer_length,
					    &offset, '\"');
			if (dump)
				g_print ("Query token -->%s<--\n", token);			
			if (code != NULL) {
				gpa_ppd_error ("The [JCL]OpenUI \"%s\" Contains 2 code tags",
					       name);
				goto gpa_ppd_add_options_pickone_error;
			}
			code = gpa_tu_token_next_dup_till (buffer, buffer_length,
						       &offset, '\"');
			if (dump)
				g_print ("Query code -->%s<--\n", code);
			if (!gpa_ppd_add_options_code_fragment (info, code, options))
				goto gpa_ppd_add_options_pickone_error;
  	        /* If the token is a order dependency token */
		} else if (strncmp (token, order_tag, order_tag_length) == 0) {
			{
				static gboolean warned = FALSE;
				if (!warned) {
					g_warning ("Implement order dependency");
					warned = TRUE;
				}
			}
			gpa_tu_token_next_till_newline (buffer, buffer_length,
						    &offset);
			if (dump)
				g_print ("Order Dep token -->%s<--\n", token);
		/* If the token is a *end, ignore */
		} else if (strncmp (token, end_tag, end_tag_length) == 0) {
			gpa_tu_token_next_till_newline (buffer, buffer_length, &offset);
			if (dump)
				g_print ("*End token -->%s<--\n", token);
		/* If neither of them all, then we dunno what it is, prolly an error */
		} else {
			gpa_ppd_error ("Dunno what to do with this token -->%s<--\n",
				       token);
			return FALSE;
		}
	}

	options->children = g_list_reverse (options->children);
	gpa_options_add_setcode (options, info);

	if (token)
		g_free (token);
	if (def) {
		gpa_ppd_set_default_option_from_token (info, def, options);
		g_free (def);
	}

	g_free (option_tag);
	g_free (code_tag);
	g_free (order_tag);
	g_free (end_tag);

	if (gpa_ppd_options_are_boolean (options))
		options->options_type = GPA_OPTIONS_TYPE_BOOLEAN;
	else
		options->options_type = GPA_OPTIONS_TYPE_PICKONE;
	
	return TRUE;

gpa_ppd_add_options_pickone_error:
	if (token)
		g_free (token);
	if (option_tag)
		g_free (option_tag);
	
	return FALSE;
}

static gboolean
gpa_ppd_add_options_from_name (const gchar *ppd_name, const gchar *name,
			       GpaPpdInfo *info, GpaOptionsGroup group,
			       gboolean jcl)
{
	GpaOptions *options;
	GpaModel *model;
	GpaOptionsType options_type;
	gchar *ui;
	gchar *token;
	gchar *buffer;
	gchar *id;
	gchar *real_name;
	gint buffer_length;
	gint offset;

	debug (FALSE, "");

	g_return_val_if_fail (ppd_name != NULL, FALSE);
	g_return_val_if_fail (info     != NULL, FALSE);

	if (!gpa_ppd_utils_get_ui (ppd_name, info, &ui, &real_name, jcl))
		return TRUE;

	/* Set the variables */
	buffer = ui;
	buffer_length = strlen (buffer);
	offset = 0;

	/* Get the option type */
	token = gpa_tu_token_next_dup (buffer, buffer_length, &offset);
	options_type = gpa_options_get_type_from_string (token);
	g_free (token);
	if (options_type == GPA_OPTIONS_TYPE_ERROR) {
		gpa_ppd_error ("Could not get option_type");
		return FALSE;
	}
	
	/* Create the options */
	id = gpa_ppd_utils_create_id (ppd_name);
	if ((name == NULL) && (real_name == NULL))
		name = g_strdup (id);
	if (name == NULL)
		name = real_name;

	options = gpa_options_new (info->model, name, id, group);
	g_return_val_if_fail (options != NULL, FALSE);
	g_free (id);
	if (real_name)
		g_free (real_name);	

	buffer += offset;
	buffer_length -= offset;


	options->frame = g_strdup (_("Options"));
        /* Make sure the function sets the option type */
	options->options_type = GPA_OPTIONS_TYPE_ERROR;
	/* go, go, go ! */
	switch (options_type) {
	case GPA_OPTIONS_TYPE_BOOLEAN:
	case GPA_OPTIONS_TYPE_PICKONE:
		if (!gpa_ppd_add_options_pickone (info, buffer,
						  buffer_length,
						  options, ppd_name,
						  jcl)) {
			g_warning ("Could not load Options\n");
			return TRUE; /* Don't return as error, cause we where unable to parse
				      * this options, that does noe mean that the rest of the
				      * ppd is not usefull to us, just this option
				      */
		}
		break;
	case GPA_OPTIONS_TYPE_PICKMANY:
		g_warning ("Implement me. Options type PICKMANY not implemented");
		return FALSE;
	case GPA_OPTIONS_TYPE_DOUBLE:
		g_warning ("Implement me. Options type DOUBLE not implemented "
			   "(and never will be)");
		return FALSE;
	case GPA_OPTIONS_TYPE_ERROR:
	default:
		g_warning ("Option type error.");
		return FALSE;
	}
	g_return_val_if_fail (options->options_type != GPA_OPTIONS_TYPE_ERROR, FALSE);
	
	model = info->model;
	model->options_list = g_list_prepend (model->options_list, options);

	return TRUE;
}

static gboolean
gpa_ppd_add_options_real (GpaPpdInfo *info)
{
	const gchar *ppd_name;
	const gchar *name;
	GpaModel *model;
	GpaOptionsGroup group;
	gboolean jlc;
	gint num;
	gint n;
	
	debug (FALSE, "");

	g_return_val_if_fail (info != NULL, FALSE);

	num = sizeof (gpa_known_ppd_options) / sizeof (GpaKnownPpdOptions);
	for (n = 0; n < num; n++) {
		ppd_name = gpa_known_ppd_options[n].ppd_name;
		name     = gpa_known_ppd_options[n].name;
		group    = gpa_known_ppd_options[n].group;
		jlc      = gpa_known_ppd_options[n].jlc;
		if (!gpa_ppd_add_options_from_name (ppd_name, name,
						    info, group, jlc)) {
			return FALSE;
		}
	}
	
	model = info->model;
	model->options_list = g_list_reverse (model->options_list);		
	
	return TRUE;
}

gboolean
gpa_ppd_add_options (GpaPpdInfo *ppd_info)
{
	debug (FALSE, "");

	g_return_val_if_fail (ppd_info != NULL, FALSE);

	if (!gpa_ppd_add_paper_options (ppd_info))
		return FALSE;
	if (!gpa_ppd_add_options_real (ppd_info))
		return FALSE;

	return TRUE;
}

