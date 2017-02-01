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
#include <libgnome/gnome-util.h>

#include "xml-utils.h"

#include "gpa-private.h"
#include "gpa-printer.h"
#include "gpa-printer-private.h"
#include "gpa-include.h"
#include "gpa-backend.h"
#include "gpa-backend-private.h"
#include "gpa-settings.h"
#include "gpa-settings-private.h"
#include "gpa-utils.h"
#include "gpa-model.h"
#include "gpa-model-private.h"


GpaBackend *
gpa_backend_new (void)
{
	GpaBackend *backend;

	backend = g_new0 (GpaBackend, 1);

	backend->id     = NULL;
	backend->values = NULL;
	backend->def    = FALSE;

	return backend;
}

static GpaBackend *
gpa_backend_new_from_node (xmlNodePtr tree_, GList *list)
{
	GpaBackend *backend;
	xmlNodePtr tree;
	xmlNodePtr node;
	gchar *driver;
	gchar *def;
	gchar *id;
	
	debug (FALSE, "");

	tree = gpa_include_node (tree_);

	g_return_val_if_fail (tree_ != NULL, NULL);

	if (!gpa_xml_node_verify (tree, GPA_TAG_BACKEND))
		return NULL;

	id = gpa_xml_get_value_string_required (tree, GPA_TAG_ID, GPA_TAG_BACKEND);
	if (id == NULL)
		return NULL;
	driver = gpa_xml_get_value_string_required (tree, GPA_TAG_ID, GPA_TAG_DRIVER);
	if (driver == NULL)
		return NULL;

	backend = gpa_backend_new ();
	backend->id     = id;

	node = gpa_xml_search_child_required (tree, GPA_TAG_BACKEND_INFO);
	if (node == NULL)
		return NULL;

	backend->values = gpa_xml_utils_new_hash_from_node (node, GPA_TAG_BACKEND_INFO);

	/* If this value is default */
	def = gpa_xml_get_value_string (tree, GPA_TAG_DEFAULT);
	if ((def != NULL) &&
	    (strcmp (def, GPA_TAG_TRUE))) {
		backend->def = TRUE;
	}
	if (def) g_free (def);
	
		
	return backend;	
}

static gboolean
gpa_backend_free (GpaBackend *backend)
{
	debug (FALSE, "");

	g_return_val_if_fail (backend != NULL, FALSE);
	
	g_free (backend->id);
	

	g_free (backend);

	return TRUE;
}

gboolean
gpa_backend_list_free (GList *backend_list)
{
	GpaBackend *backend;
	GList *list;

	debug (FALSE, "");
	
	list = backend_list;
	for (; list != NULL; list = list->next) {
		backend = (GpaBackend *) list->data;
		if (!gpa_backend_free (backend))
		    return FALSE;
	}

	return TRUE;
}
		    
GList *
gpa_backend_list_new_from_node (xmlNodePtr tree_)
{
	GpaBackend *backend;
	xmlNodePtr tree;
	xmlNodePtr node;
	xmlNodePtr child;
	GList *list;

	debug (FALSE, "");

	tree = gpa_include_node (tree_);

	g_return_val_if_fail (tree != NULL, NULL);
	
	if (!gpa_xml_node_verify (tree_, GPA_TAG_PRINTER_MODEL))
		return NULL;

	child = gpa_xml_search_child_required (tree, GPA_TAG_BACKENDS);
	if (child == NULL)
		return NULL;

	node = child->childs;
	list = NULL;
	while (node != NULL) {
		skip_text (node);
		if (!gpa_xml_node_verify (node, GPA_TAG_BACKEND))
			return NULL;
		backend = gpa_backend_new_from_node (node, list);
		if (backend == NULL)
			return NULL;
		list = g_list_prepend (list, backend);
		node = node->next;
	}

	if (list == NULL)
		gpa_error ("A profile should contain at least 1 backend node");

	return g_list_reverse (list);
}

static xmlNodePtr
gpa_backend_write (XmlParseContext *context, GpaBackend *backend)
{
	xmlNodePtr node;
	xmlNodePtr values;
	
	debug (FALSE, "");

	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (backend != NULL, NULL);


	node = xmlNewDocNode (context->doc, context->ns, GPA_TAG_BACKEND, NULL);

	xmlNewChild (node, context->ns, GPA_TAG_ID, backend->id);

	values = gpa_hash_write (context, backend->values, GPA_TAG_BACKEND_INFO);
	if (values != NULL)
		xmlAddChild (node, values);

	return node;
}


xmlNodePtr
gpa_backend_list_write (XmlParseContext *context,
			GList *list_)
{
	GpaBackend *backend;
	xmlNodePtr child;
	xmlNodePtr node;
	GList *list;

	debug (FALSE, "");

	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (list_   != NULL, NULL);

	/* Write the FileInfo */
	node = xmlNewDocNode (context->doc, context->ns,
			      GPA_TAG_BACKENDS, NULL);

	/* Scan the list of options and if option->ty */
	list = list_;
	for ( ; list != NULL; list = list->next) {
		backend = (GpaBackend *)list->data;
		if (backend == NULL) {
			gpa_error ("Backend == NULL.");
			return NULL;
		}
		child = gpa_backend_write (context, backend);
		if (child == NULL)
			return NULL;
		xmlAddChild (node, child);
	}

	debug (FALSE, "end");
	
	return node;
}


static gboolean
gpa_backend_verify (GpaBackend *backend)
{
	debug (FALSE, "");

	g_return_val_if_fail (backend != NULL, FALSE);
	
       	if (backend->id == NULL) {
		gpa_error ("The Backend object was missing an id");
		return FALSE;
	}
	if (backend->values == NULL) {
		gpa_error ("The Backend object does not contain the values hash");
		return FALSE;
	}

	return TRUE;
}

gboolean
gpa_backend_list_verify (GList *backend_list)
{
	GpaBackend *backend;
	GList *list;

	g_return_val_if_fail (backend_list != NULL, FALSE);

	list = backend_list;
	for (;list != NULL; list = list->next) {
		backend = (GpaBackend *)list->data;
		if (!gpa_backend_verify (backend))
			return FALSE;
	}

	return TRUE;
}

static GpaBackend*
gpa_backend_copy (GpaBackend *backend)
{
	GpaBackend *new_backend;

	debug (FALSE, "");

	g_return_val_if_fail (backend != NULL, NULL);

	new_backend = gpa_backend_new ();
	new_backend->id     = g_strdup (backend->id);
	new_backend->values = gpa_hash_copy (backend->values);
	
	return backend;	
}
	
GList *
gpa_backend_list_copy (GList *backend_list)
{
	GpaBackend *backend;
	GpaBackend *new_backend;
	GList *new_list = NULL;
	GList *list;

	debug (FALSE, "");

	g_return_val_if_fail (backend_list != NULL, NULL);
	
	list = backend_list;
	for ( ; list != NULL; list = list->next) {
		backend = (GpaBackend *) list->data;
		new_backend = gpa_backend_copy (backend);
		if (new_backend == NULL)
			return NULL;
		new_list = g_list_prepend (new_list, new_backend);
	}

	return g_list_reverse (new_list);
}

GpaBackend *
gpa_backend_get_default (GpaModel *model)
{
	GpaBackend *backend = NULL;
	GpaBackend *def = NULL;
	GList *list;

	debug (FALSE, "");
	
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);

	list = model->backend_list;

	for (; list != NULL; list = list->next) {
		backend = (GpaBackend *) list->data;
		if ((def == NULL) ||
		    (backend->def))
			def = backend;
	}

	return def;
}

GpaBackend *
gpa_backend_get_from_id (GpaModel *model,
			 const gchar *id)
{
	GpaBackend *backend;
	GList *list;

	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);
	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (model->backend_list != NULL, NULL);

	list = model->backend_list;
	for (; list != NULL; list = list->next) {
		backend = (GpaBackend *)list->data;
		if (backend->id == NULL) {
			gpa_error ("Backend Id is null\n");
			return NULL;
		}
		if (strcmp (backend->id, id) == 0)
			return backend;
	}

	gpa_error ("Could not find backend \"%s\" for model \"%s\"",
		   id, model->id);
	
	return NULL;
}

GpaBackend *
gpa_backend_get_selected (GpaSettings *settings)
{
	GpaBackend *backend;
	GList *list;
	gchar *backend_id;
	
	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
			      
	backend_id = gpa_settings_value_dup (settings, GPA_TAG_BACKEND);
		
	list = settings->printer->model->backend_list;
	for (; list != NULL; list = list->next) {
		backend = (GpaBackend *) list->data;
		
		g_return_val_if_fail (backend->id != NULL, NULL);
		
		if (strcmp (backend->id, backend_id) == 0) {
			g_free (backend_id);
			return backend;
		}
	}

	gpa_error ("Could not find \"%s\" backend for settings \"%s\"\n",
		   backend_id, settings->name);

	g_free (backend_id);

	return NULL;
}




/* Access to the struct */
const gchar *
gpa_backend_get_id (GpaBackend *backend)
{
	g_return_val_if_fail (GPA_IS_BACKEND (backend), NULL);
	g_return_val_if_fail (backend->id != NULL, NULL);

	return backend->id;
}


