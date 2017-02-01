/* FIXME
 * FIXME
 * FIXME
 * We are not closing parse contexts !!!!!!!
 */

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

#include <string.h>

#include <gtk/gtkwidget.h>
#include <libgnome/gnome-util.h>

#include "xml-utils.h"
#include "gpa-include.h"
#include "gpa-tags.h"

GHashTable * context_hash = NULL;

static xmlNodePtr
gpa_include_search_for_node (xmlNodePtr tree, const gchar *token,
			     const gchar *key, const gchar * value)
{
	xmlNodePtr child;
	gchar *tmp;
	
	g_return_val_if_fail (tree != NULL, NULL);
	g_return_val_if_fail (token != NULL, NULL);

	/* We search the node for child nodes named @token.
	 * when we find a node, we search for a child named key
	 * then we compare that the key, equals value. If it does
	 * we have a match, if it doesn't thanks for playing
	 */
	child = tree->childs;
	while (child != NULL) {
		if (strcmp (child->name, token) == 0) {
			/* If key and value are NULL, return when 
			   the first match if found. */
			if (!key || !value)
				return child;
			tmp = gpa_xml_get_value_string (child, key);
			if (strcmp (tmp, value) == 0) {
				g_free (tmp);
				return child;
			}
		}
		child = child->next;
	}

	return NULL;
}

static xmlNodePtr
gpa_include_open_file (const gchar *model_id)
{
	XmlParseContext *context;
	gchar *full_path;
	
	g_return_val_if_fail (model_id != NULL, NULL);

	/* Check if this file has not been opened before */
	context = g_hash_table_lookup (context_hash, model_id);
	if (context != NULL) {
#if 0	
		g_print ("This file was already opened\n");
#endif
		return context->doc->root;
	}

	/* Open and parse the file */
	full_path = g_strdup_printf (GPA_DATA_DIR "%s/%s%s",
				     "models", model_id, GPA_MODEL_EXTENSION_NAME);

	context = gpa_xml_parse_context_new_from_path (full_path,
						       GPA_TAG_MODEL_NAME_SPACE,
						       GPA_TAG_PRINTER_MODEL);

	if (context == NULL) {
		gpa_error ("Could not open context for %s\n", full_path);
		g_free (full_path);
		return NULL;
	}

	g_free (full_path);

	/* Keep this file handy for the next time */
	g_hash_table_insert (context_hash, g_strdup (model_id), context);
	
	return context->doc->root;
}

#define GPA_INCLUDE_PATH_DELIMITER '/'
static xmlNodePtr
gpa_include_node_get (const gchar *file, const gchar *path, const gchar *key, const gchar *value)
{
	xmlNodePtr node;
	gchar *token;
	gchar *end;
	gchar *n;
	gchar *free_me;
	gboolean first = TRUE;
	
	g_return_val_if_fail (file     != NULL, NULL);
	g_return_val_if_fail (path     != NULL, NULL);

	node = gpa_include_open_file (file);

	token = g_strdup (path);
	free_me = token;
	end = token + strlen (token);
	
	while (TRUE){
		n = strchr (token, GPA_INCLUDE_PATH_DELIMITER);
		if (n == NULL) break;
		*n = 0;
		
		if (first) {
			if (!gpa_xml_node_verify (node, token))
				goto gpa_include_node_get_error;
			first = FALSE;
		} else {
			node = gpa_xml_search_child_required (node, token);
		}
		
		if (!node) goto gpa_include_node_get_error;
		
		token = n + 1;
	}

	node = gpa_include_search_for_node (node, token, key, value);
	
	g_free (free_me);

	return node;

gpa_include_node_get_error:
	gpa_error ("Could not find path %s::%s\n", file, path);
	return NULL;
}

xmlNodePtr
gpa_include_node (xmlNodePtr node_in)
{
	static gboolean initialized = FALSE;
	xmlNodePtr ret;
	gchar *file;
	gchar *path;
	gchar *key;
	gchar *value;

	g_return_val_if_fail (node_in != NULL, NULL);

	if (!initialized) {
		context_hash = g_hash_table_new (g_str_hash, g_str_equal);
		initialized = TRUE;
	}
	
	file = (gchar *) xmlGetProp (node_in, "IncludeFrom"); /* FIXME: add to tags.h */
	if (file == NULL) {
		return node_in;
	}

	/* This is a include node. We have the filename, Get the path and
	   the Info for the node we need to include */
	/* FIXME ! use tags, not strings */
	path  = gpa_xml_get_value_string_required (node_in, "Path", "IncludedFrom");
	key   = gpa_xml_get_value_string (node_in, "Key");
	value = gpa_xml_get_value_string (node_in, "Value");

	/* Path is required. Value and key are optional, but they are used in pairs */
	if (!path || ((!key || !value) && (key || value))) {
		gpa_error ("The IncludeFrom could not be loaded. Parameter missing\n"
			   "Path:[%s] Key:[%s] Value:[%s]\n", path, key, value);
		ret = NULL;
		goto gpa_include_node_end;
	}

	ret = gpa_include_node_get (file, path, key, value);
	
gpa_include_node_end:
	if (file)  g_free (file);
	if (path)  g_free (path);
	if (key)   g_free (key);
	if (value) g_free (value);
	return ret;
}
