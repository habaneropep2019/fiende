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

#include <gtk/gtkwidget.h>
#include <libgnome/gnome-util.h>

#include <dirent.h> /* For the DIR structure stuff */

#include "gpa-private.h"
#include "gpa-model-info.h"
#include "gpa-model-info-private.h"


GpaModelInfo *
gpa_model_info_new (const gchar *name, const gchar *id, const GpaVendor *vendor)
{
	GpaModelInfo *model_info;

	debug (FALSE, "");

	model_info = g_new0 (GpaModelInfo, 1);

	model_info->name   = g_strdup (name);
	model_info->id     = g_strdup (id);
	model_info->vendor = vendor;

	/* FIXME: Should we ref vendor ? */
	
	return model_info;
}

static GpaModelInfo *
gpa_load_model_info_from_tree (XmlParseContext *context,
			       xmlNodePtr tree,
			       GpaVendor *vendor)
{
	GpaModelInfo *model_info = NULL;
	xmlNodePtr child;
	gchar *name;
	gchar *id;
	gchar *full_path;
	
	debug (FALSE, "");

	if (!gpa_xml_node_verify (tree, GPA_TAG_MODEL_INFO))
		return NULL;

	child = tree->childs;

	name = gpa_xml_get_value_string_required (tree, GPA_TAG_NAME, NULL);
	if (name == NULL) 
		return NULL;

	id = gpa_xml_get_value_string_required (tree, GPA_TAG_ID, NULL);
	if (name == NULL)
		return NULL;

	/* Verify if the model file exists on disk, if it does not
	 * return NULL. We only want to load the models we are going
	 * to be able to use
	 */
	full_path = g_strdup_printf (GPA_DATA_DIR "%s/%s%s",
				     "models", id, GPA_MODEL_EXTENSION_NAME);
	if (g_file_exists (full_path))
		model_info = gpa_model_info_new (name, id, vendor);
#if 0	/* FIXME */
	else
		g_print ("File not found [%s]\n", full_path);
#endif	

	g_free (full_path);
	g_free (name);
	g_free (id);

	return model_info;
}

gboolean
gpa_load_model_info_list_from_tree (XmlParseContext *context,
				    xmlNodePtr tree,
				    GList **model_info_list,
				    GpaVendor *vendor)
{
	GpaModelInfo *model_info;
	xmlNodePtr node;
	GList *list = *model_info_list;
	
	debug (FALSE, "");

	g_return_val_if_fail (tree != NULL, FALSE);
	
	if (strcmp (tree->name, GPA_TAG_MODELS)) {
		gpa_error ("Invalid type in load_from_node.\n"
			   "Expected " GPA_TAG_MODELS ", encountered %s",
			   tree->name);
		return FALSE;
	}

	if (tree == NULL) {
		gpa_error ("The file did not contained any models");
		return FALSE;
	}

	node = tree->childs;
	while (node != NULL) {
		model_info = gpa_load_model_info_from_tree (context, node, vendor);
		if (model_info != NULL)
			list  = g_list_prepend (list, model_info);
		node = node->next;
	}

	*model_info_list = list;

	return TRUE;
}

/* Access to the struct */
const gchar *
gpa_model_info_get_name (GpaModelInfo *mi)
{
	g_return_val_if_fail (GPA_IS_MODEL_INFO (mi), NULL);

	return mi->name;
}

gchar *
gpa_model_info_dup_name (GpaModelInfo *mi)
{
	g_return_val_if_fail (GPA_IS_MODEL_INFO (mi), NULL);

	return g_strdup (mi->name);
}

const gchar *
gpa_model_info_get_id (GpaModelInfo *mi)
{
	g_return_val_if_fail (GPA_IS_MODEL_INFO (mi), NULL);

	return mi->id;
}


const GpaVendor *
gpa_model_info_get_vendor (GpaModelInfo *mi)
{
	g_return_val_if_fail (GPA_IS_MODEL_INFO (mi), NULL);

	return mi->vendor;
}
