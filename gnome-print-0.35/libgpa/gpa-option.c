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
#include <stdlib.h> /* for atoi()/atof() */
#include <libgnome/gnome-util.h>


#include "text-utils.h"
#include "xml-utils.h"

#include "gpa-private.h"
#include "gpa-include.h"
#include "gpa-option.h"
#include "gpa-option-private.h"
#include "gpa-options.h"
#include "gpa-options-private.h"
#include "gpa-printer.h"
#include "gpa-model.h"
#include "gpa-model-private.h"
#include "gpa-printer-private.h"
#include "gpa-constraints.h"
#include "gpa-settings.h"
#include "gpa-settings-private.h"
#include "gpa-utils.h"



GpaOption*
gpa_option_get_from_id (GList *option_list, const gchar *id)
{
	GpaOption *option;
	GList *list;

	list = option_list;
	for (; list != NULL; list = list->next) {
		option = list->data;
		if (strcmp (option->id, id) == 0)
			return option;
	}

	return NULL;
}

GpaOption *
gpa_option_new (const gchar *name,
		const gchar * id,
		GpaOptions *parent)
{
	GpaOption *option;

	g_return_val_if_fail (name   != NULL, NULL);
	g_return_val_if_fail (id     != NULL, NULL);
	g_return_val_if_fail (parent != NULL, NULL);

	debug (FALSE, "");
	
	option = g_new (GpaOption, 1);

	option->name        = g_strdup (name);
	option->id          = g_strdup (id);
	option->content     = parent->content;
	option->parent      = parent;
	option->children    = NULL;
	option->values      = NULL;
	option->constraints = NULL;

	return option;
}

GpaOption *
gpa_option_new_from_node (xmlNodePtr tree_,
			  GpaOptions *parent,
			  GList *loaded_option_list)
{
	GpaOption *option;
	xmlNodePtr tree;
	xmlNodePtr child;
	gchar *name;
	gchar *id;

	debug (FALSE, "");

	tree = gpa_include_node (tree_);

	g_return_val_if_fail (tree != NULL, NULL);
	
	if (!gpa_xml_node_verify (tree, GPA_TAG_OPTION))
		return NULL;

	/* Get the name & id */
	name = gpa_xml_get_value_string_required (tree, GPA_TAG_NAME, NULL);
	id   = gpa_xml_get_value_string_required (tree, GPA_TAG_ID, parent->name);

	if (name == NULL)
		return NULL;
	if (id == NULL)
		return NULL;
	
	/* Find if there is another "option" object with this ID" */
	if (gpa_option_get_from_id (loaded_option_list, id) != NULL) {
		gpa_error ("The option \"%s\" is duplicated, under the "
			   "\"%s\" options.", id, parent->id);
		return NULL;
	}

	option = gpa_option_new (name, id, parent);

	g_free (name);
	g_free (id);

	/* Load the child options for this option */
	option->children = NULL;
	child = gpa_xml_search_child (tree, GPA_TAG_OPTIONS);
	if (child != NULL) {
		option->children = gpa_options_new_from_node (parent->model,
							      child,
							      option);
		if (option->children == NULL)
			return NULL;
	}

	/* Load the values */
	option->values = NULL;
	child = gpa_xml_search_child (tree, GPA_TAG_OPTION_INFO);
	if (child != NULL) {
		option->values = gpa_xml_utils_new_hash_from_node (child,
							       GPA_TAG_OPTION_INFO);
		if (option->values == NULL) {
			gpa_error ("The \"%s\" node not be loaded", option->name);
			return NULL;
		}
	}

	/* Load the constraints */
	option->constraints = NULL;
	child = gpa_xml_search_child (tree, GPA_TAG_CONSTRAINTS);
	if (child != NULL)
		g_warning ("Constraints are no longer children of Options");


	child = gpa_xml_search_child (tree, GPA_TAG_DEFAULT);
	if (child != NULL) {
		GpaModel *model;
		gchar *path;
		path = gpa_option_dup_path (option);
		model = option->parent->model;
		model->default_settings = g_list_prepend (model->default_settings,
							  path);
	}

	return option;
}

GpaOption *
gpa_option_copy (GpaOption *option,
		 GpaOptions *parent)
{
	GpaOption *new_option;

	new_option = gpa_option_new (option->name,
				     option->id,
				     parent);

	new_option->children    = gpa_options_copy (option->children, parent->model);
	new_option->values      = gpa_hash_copy (option->values);
	new_option->constraints = gpa_constraints_copy (option->constraints);

	return new_option;
}

void
gpa_option_free (GpaOption *option)
{
	g_return_if_fail (option->name != NULL);
	g_return_if_fail (option->id != NULL);


	g_free (option->name);
	g_free (option->id);

	gpa_hash_free (option->values);
	if (option->children != NULL)
		gpa_options_free (option->children);
	if (option->constraints)
		gpa_constraints_free (option->constraints);
}


xmlNodePtr
gpa_option_write (XmlParseContext *context, GpaOption *option)
{
	xmlNodePtr node;
	xmlNodePtr children;
	xmlNodePtr values = NULL;
	
	debug (FALSE, "");

	g_return_val_if_fail (option != NULL,         NULL);
	g_return_val_if_fail (option->values != NULL, NULL);

	/* Write the FileInfo */
	node = xmlNewDocNode (context->doc, context->ns, GPA_TAG_OPTION, NULL);

	/* Scan the list of options and if option->ty */
	xmlNewChild (node, context->ns, GPA_TAG_NAME, option->name);
	xmlNewChild (node, context->ns, GPA_TAG_ID,   option->id);

	/* Write the values */
	values = gpa_hash_write (context, option->values, GPA_TAG_OPTION_INFO);
	if (values != NULL)
		xmlAddChild (node, values);


#if 0 /* We write constraints under the model node, now ..*/
/* Write constraints */
	constraints = gpa_constraints_write (context, option);
	if (constraints != NULL)
		xmlAddChild (node, constraints);
#endif	

	/* There are no options */
	if (option->children == NULL)
		return node;

	/* The optional children structure */
	children = gpa_options_write (context, option->children);
	if (children != NULL)
		xmlAddChild (node, children);

	debug (FALSE, "");
	
	return node;
}


gboolean
gpa_option_verify (GpaOption *option, const GpaModel *model)
{
	debug (FALSE, "");

	g_return_val_if_fail (option != NULL, FALSE);

	if (option->name == NULL) {
		gpa_error ("The Option object did not contained a name");
		return FALSE;
	}
	if (option->id == NULL) {
		gpa_error ("The Option \"%s\"object did not contained a valid id",
			   option->name);
		return FALSE;
	}

	if ((option->content != GPA_CONTENT_GENERIC) &&
	    (option->content != GPA_CONTENT_PAPER_SIZE) &&
	    (option->content != GPA_CONTENT_PAPER_MEDIA) &&
	    (option->content != GPA_CONTENT_PAPER_SOURCE) &&
	    (option->content != GPA_CONTENT_PAPER_ORIENTATION) &&
	    (option->content != GPA_CONTENT_RESOLUTION) &&
	    (option->content != GPA_CONTENT_RESOLUTION_MODE) &&
	    (option->content != GPA_CONTENT_ERROR)) {
		gpa_error ("The Option \"%s\" object contained an invalid content id [%i]",
			   option->name, option->content);
		return FALSE;
	}

	if (option->constraints != NULL)
		if (!gpa_constraints_verify (option, model))
			return FALSE;

	if (!GPA_IS_OPTIONS (option->parent)) {
		gpa_error ("The option \"%s\" object did not cotained a reference to a parent",
			   option->name);
		return FALSE;
	}

	if (option->parent->options_type == GPA_OPTIONS_TYPE_BOOLEAN) {
		if ( (strcmp (option->id, GPA_TAG_TRUE) != 0) &&
		     (strcmp (option->id, GPA_TAG_FALSE) != 0)) {
			gpa_error ("The boolean option \"%s\" (under \"%s\") contained "
				   "an invalid id :\"%s\"\nExpected (%s/%s)",
				   option->name, option->parent->name, option->id,
				   GPA_TAG_TRUE, GPA_TAG_FALSE);
			return FALSE;
		}
	}
	    
		
#if 0	/* FIXME */
	if (option->values != NULL)
		if (!gpa_values_verify_option (option))
			return FALSE;
#endif	

	return TRUE;
}


gboolean
gpa_option_verify_with_settings (GpaOption *option,
				 GpaSettings *settings)
{

	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);
	g_return_val_if_fail (option != NULL, FALSE);
	
	if (!gpa_option_verify (option, settings->printer->model))
		return FALSE;
		
	if (option->constraints != NULL)
		if (!gpa_constraints_verify_with_settings (option, settings))
			return FALSE;

	return TRUE;
}


GpaOption *
gpa_option_get_from_printer_and_path  (const GpaPrinter *printer, const gchar *path_)
{
	g_return_val_if_fail (path_ != NULL, NULL);
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);

	return gpa_option_get_from_options_list_and_path
		(printer->model->options_list, path_);
}

GpaOption *
gpa_option_get_from_model_and_path  (const GpaModel *model, const gchar *path_)
{
	g_return_val_if_fail (path_ != NULL, NULL);
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);

	return gpa_option_get_from_options_list_and_path
		(model->options_list, path_);
}

GpaOption *
gpa_option_get_from_settings_and_path (const GpaSettings *settings, const gchar *path_)
{
	g_return_val_if_fail (path_ != NULL, NULL);
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);

	return gpa_option_get_from_options_list_and_path
		(settings->printer->model->options_list, path_);
}


static GpaOption *
gpa_option_get_from_options_and_path (GpaOptions *options, const gchar *path)
{
	GpaOption *option = NULL;
	GList *list;
	gchar *option_id = NULL;
	gint pos;

	pos = gpa_tu_search (path, GPA_PATH_DELIMITER, TRUE);
	if (pos == -1) {
		gpa_error ("Invalid path %s, does not contain an Option\n", path);
		return NULL;
	}

	option_id = g_strndup (path, pos);
	list = options->children;
	for (; list != NULL; list = list->next) {
		option = (GpaOption *)list->data;
		if (strcmp (option->id, option_id) == 0)
			break;
	}

	if (list == NULL) {
		gpa_error ("Option not found from path \"%s\" in \"%s\"(2)",
			   path, gpa_options_get_name (options));
		g_free (option_id);
		return NULL;
	}

	if (strlen(path + pos + 1) > 0) {
		GpaOption *sub_option;
		gchar *subpath;
		subpath = g_strdup (path+pos+1+strlen(option->children->id)+1);
		sub_option = gpa_option_get_from_options_and_path (option->children,
								   subpath);
		g_free (subpath);
		g_free (option_id);
		return sub_option;
	}
	
	g_free (option_id);

	return option;

}

GpaOption *
gpa_option_get_from_options_list_and_path (GList *options_list, const gchar *path_)
{
	GpaOptions *options = NULL;
	GpaOption *option = NULL;
	GList *list;
	gchar *options_id = NULL;
	gchar *path;
	gchar *remainder;
	gint pos;

	g_return_val_if_fail (options_list != NULL, NULL);
	g_return_val_if_fail (path_   != NULL, NULL);

	path = g_strdup_printf ("%s" GPA_PATH_DELIMITER, path_);
	
	pos = gpa_tu_search (path, GPA_PATH_DELIMITER, TRUE);
	if (pos == -1) {
		gpa_error ("Invalid path *%s*%s*\n", path, path_);
		goto gpa_option_get_from_options_list_and_path_error;
	}

	options_id = g_strndup (path, pos);
	list = options_list;
	for (; list != NULL; list = list->next) {
		options = (GpaOptions *)list->data;
		if (strcmp (options->id, options_id) == 0)
			break;
	}

	if (list == NULL) {
		gpa_error ("Options not found from path \"%s\" (1.531)[%s]", path, path_);
		goto gpa_option_get_from_options_list_and_path_error;
	}

	remainder = g_strdup (path+pos+1);
	option = gpa_option_get_from_options_and_path (options, remainder);
	g_free (remainder);

	g_free (options_id);
	g_free (path);

	return option;

gpa_option_get_from_options_list_and_path_error:

	if (options_id != NULL)
		g_free (options_id);
	g_free (path);
	
	return NULL;
}

gboolean 
gpa_option_is_selected (const GpaSettings *settings,
			const GpaOption *option_findme)
{
	GpaOption *option= NULL;
	GList *list;
	gchar *path;

	debug (FALSE, "");
	
	g_return_val_if_fail (option_findme != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);

	list = settings->selected_options;
	for (; list != NULL; list = list->next) {
		path = (gchar *) list->data;
		option = gpa_option_get_from_settings_and_path (settings, path);
		if (option == NULL)
			return FALSE;
		if (option == option_findme)
			return TRUE;
	}

	return FALSE;
}

gchar *
gpa_option_dup_path (GpaOption *option)
{
	gchar *path;
	
	debug (FALSE, "");

	g_return_val_if_fail (option != NULL, NULL);

	if (option->parent->parent != NULL) {
		if (option->parent->parent->parent->parent != NULL) {
			gpa_error ("We only support 1st & 2nd level option");
		}
		path  = g_strdup_printf ("%s" GPA_PATH_DELIMITER
					 "%s" GPA_PATH_DELIMITER
					 "%s" GPA_PATH_DELIMITER
					 "%s",
					 option->parent->parent->parent->id,
					 option->parent->parent->id,
					 option->parent->id,
					 option->id);
	} else {
		path = g_strdup_printf ("%s" GPA_PATH_DELIMITER "%s", option->parent->id, option->id);
	}
		
	return path;
}


gchar*
gpa_option_value_dup (const GpaOption *option, const gchar *key)
{
	GHashTable *hash;
	gchar *value;
	
	debug (FALSE, "");

	g_return_val_if_fail (option != NULL, NULL);
	g_return_val_if_fail (key != NULL,    NULL);

	hash = option->values;

	if (hash == NULL) {
		gpa_error ("Hash value not found. Option:\"%s\" Key:\"%s\"",
			   option->name,
			   key);
		return NULL;
	}

	value = g_hash_table_lookup (hash, key);

	debug (FALSE, "end");

	return g_strdup (value);
}


void
gpa_option_value_get_int (const GpaOption *option, const gchar *key, int *value)
{
	gchar *str_value;
	
	debug (FALSE, "");

	str_value = gpa_option_value_dup (option, key);

	if (str_value == NULL) 
		return;

	*value = atoi (str_value);

	g_free (str_value);
}
	
void
gpa_option_value_get_double (const GpaOption *option, const gchar *key, double *value)
{
	gchar *str_value;
	
	debug (FALSE, "");

	str_value = gpa_option_value_dup (option, key);

	if (str_value == NULL) 
		return;

	*value = atof (str_value);

	g_free (str_value);
}

/* Access to the structure */
gchar *
gpa_option_dup_name (const GpaOption *option)
{
	g_return_val_if_fail (GPA_IS_OPTION (option), NULL);

	return g_strdup (option->name);
}

const gchar *
gpa_option_get_name (const GpaOption *option)
{
	g_return_val_if_fail (GPA_IS_OPTION (option), NULL);

	return option->name;
}

const gchar *
gpa_option_get_id (const GpaOption *option)
{
	g_return_val_if_fail (GPA_IS_OPTION (option), NULL);

	return option->id;
}


const GpaOptions *
gpa_option_get_parent (const GpaOption *option)
{
	g_return_val_if_fail (GPA_IS_OPTION (option), NULL);

	return option->parent;
}

const GpaOptions *
gpa_option_get_children (const GpaOption *option)
{
	g_return_val_if_fail (GPA_IS_OPTION (option), NULL);

	return option->children;
}

