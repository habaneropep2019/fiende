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

#include "gpa-code.h"
#include "gpa-code-private.h"
#include "gpa-private.h"
#include "gpa-include.h"
#include "gpa-option.h"
#include "gpa-option-private.h"
#include "gpa-options.h"
#include "gpa-options-private.h"
#include "gpa-printer-private.h"
#include "gpa-settings.h"
#include "gpa-settings-private.h"
#include "gpa-model.h"
#include "gpa-model-private.h"
#include "xml-utils.h"
#include "gpa-known.h"


static gint
gpa_options_determine_content (const gchar *id, gint group)
{
	gint n;
	gint number;


	number = sizeof(gpa_known_option_types) / sizeof(GpaKnownOptionType);
	for (n = 0; n < number; n++)
		if (strcmp (id, gpa_known_option_types[n].id) == 0)
			break;

	if (n == number)
		return GPA_CONTENT_GENERIC;

	if (gpa_known_option_types[n].group != group) {
		gpa_error ("The Option \"%s\" is in the wrong group", id);
		return GPA_CONTENT_ERROR;
	}

	return gpa_known_option_types[n].content;
}

GpaOptions *
gpa_options_new (GpaModel *model,
		 const gchar *name,
		 const gchar *id,
		 gint group)
{
	GpaOptions *options;
	GpaContent content;

	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (id   != NULL, NULL);

	/* Create and initilize the structure */
	options = g_new (GpaOptions, 1);
	options->children       = NULL;
	options->name           = g_strdup (name);
	options->id             = g_strdup (id);
	options->group          = group;
	options->model          = model;
	options->code_fragments = NULL;
	options->parent         = NULL;
	options->options_type   = GPA_OPTIONS_TYPE_ERROR;
	options->frame          = NULL;

	/* Determine the option type */
	content = gpa_options_determine_content (id, group);
	if (content == GPA_CONTENT_ERROR)
		return NULL;
	options->content = content;
	
	debug (FALSE, "end");
	
	return options;
}
	

GpaOptionsType
gpa_options_get_type_from_string (const gchar *text)
{
	GpaOptionsType type;
	
	if ( text == NULL )
		type   = GPA_OPTIONS_TYPE_PICKONE;
	else if (strcmp (text, "PickOne") == 0)
		type = GPA_OPTIONS_TYPE_PICKONE;
	else if (strcmp (text, "PickMany") == 0)
		type = GPA_OPTIONS_TYPE_PICKMANY;
	else if (strcmp (text, "Boolean") == 0)
		type = GPA_OPTIONS_TYPE_BOOLEAN;
	else if (strcmp (text, "Numeric") == 0)
		type = GPA_OPTIONS_TYPE_NUMERIC;
	else if (strcmp (text, "Float") == 0)
		type = GPA_OPTIONS_TYPE_DOUBLE;
	else {
		type = GPA_OPTIONS_TYPE_ERROR;
		gpa_error ("Invalid options type \"%s\"\n", text);
	}

	return type;
}


static GpaOptionsType
gpa_options_get_type_from_node (xmlNodePtr tree)
{
	gchar *text;
	GpaOptionsType type;

	debug (FALSE, "");

        text  = gpa_xml_get_value_string (tree, GPA_TAG_OPTIONS_TYPE);

	type = gpa_options_get_type_from_string (text);
	
	if (text != NULL)
		g_free (text);

	return type;
}

static void
gpa_options_load_from_node (xmlNodePtr tree_, GpaOptions **options_)
{
	GpaOptions *options = *options_;
	GpaOption  *option;
	xmlNodePtr tree;
	xmlNodePtr child;
	xmlNodePtr node;
	GList *list;

	debug (FALSE, "");

	tree = gpa_include_node (tree_);

	/* If we return on error, we want options_ to be null */
	*options_ = NULL;

	if (!gpa_xml_node_verify (tree, GPA_TAG_OPTIONS))
		return;

	/* Now scan the childs and load each option */
	child = gpa_xml_search_child_required (tree, GPA_TAG_ITEMS);
	if (child == NULL)
		return;

	node = child->childs;
	list = options->children;

	while (node != NULL) {
		skip_text (node);
		option = gpa_option_new_from_node (node,
						   options,
						   list);
		if (option == NULL)
			return;
		list = g_list_prepend (list, option);
		node = node->next;
	}
	options->children = g_list_reverse (list);

	if (g_list_length (options->children) == 0) {
		gpa_error ("Empty %s node", GPA_TAG_ITEMS);
		return;
	}

	*options_ = options;
}

static GpaOptionsGroup
gpa_options_get_group_from_string (const gchar *group)
{
	debug (FALSE, "");
	
	if (strcmp (group, GPA_TAG_PAPER_GROUP) == 0)
		return GPA_GROUP_PAPER;
	else if  (strcmp (group, GPA_TAG_QUALITY_GROUP) == 0)
		return GPA_GROUP_QUALITY;
	else if  (strcmp (group, GPA_TAG_INSTALLED_GROUP) == 0)
		return GPA_GROUP_INSTALLED;
	else if  (strcmp (group, GPA_TAG_SUB_GROUP) == 0)
		return GPA_GROUP_SUBGROUP;
	else if  (strcmp (group, GPA_TAG_COMPRESSION_GROUP) == 0)
		return GPA_GROUP_COMPRESSION;
	else if  (strcmp (group, GPA_TAG_PS_GROUP) == 0)
		return GPA_GROUP_PS;
	else {
		gpa_error ("Unrecognized group %s\n", group);
		return GPA_GROUP_ERROR;
	}
}

GpaOptions *
gpa_options_new_from_node (GpaModel *model,
			   xmlNodePtr tree_,
			   GpaOption *parent)
{
	GpaOptions *options;
	GpaOptionsGroup group_enum;
	xmlNodePtr tree;
	xmlNodePtr node;
	gchar *name;
	gchar *id;
	gchar *defined;
	gchar *group;
	gchar *frame;
	gint  options_type;

	debug (FALSE, "");

	tree = gpa_include_node (tree_);

	/* Should we skip text nodes ? */
	if (!gpa_xml_node_verify (tree, GPA_TAG_OPTIONS))
		return NULL;

	/* Get the name & id */
	name = gpa_xml_get_value_string_required (tree, GPA_TAG_NAME, NULL);
	if (name == NULL)
		return NULL;

	id   = gpa_xml_get_value_string (tree, GPA_TAG_ID);
	if (id == NULL || *id == 0)
		id = g_strdup (name);
	/* Find if there is another "options" object with this ID" */
	if (gpa_model_get_options_from_id (model, id) != NULL) {
		gpa_error ("The option \"%s\" is duplicated", id);
		return NULL;
	}

	/* Load the group */
	group = gpa_xml_get_value_string_required (tree, GPA_TAG_OPTIONS_GROUP, NULL);
	if (group == NULL)
		return NULL;
	group_enum = gpa_options_get_group_from_string (group);
	if (group_enum == GPA_GROUP_ERROR)
		return NULL;
	g_free (group);

	/* Create the new option */
	options = gpa_options_new (model, name, id, group_enum);
	if (options == NULL)
		return NULL;

	/* Load the frame */
	frame = gpa_xml_get_value_string (tree, GPA_TAG_FRAME);
	if (frame == NULL)
		frame = g_strdup (name);
	options->frame = frame;
	
	/* Is this a defined option */
	defined  = gpa_xml_get_value_string (tree, GPA_TAG_DEFINED);
	if (defined == NULL) {
		options->defined = FALSE;
	} else if (strcmp (defined, GPA_TAG_FALSE) == 0) {
		options->defined = FALSE;
	} else if (strcmp (defined, GPA_TAG_TRUE) == 0) {
		options->defined = TRUE;
	} else {
		gpa_error ("Invalid boolean value encountered\n"
			   "Expected (FALSE/TRUE), encountered \"%s\" for id \"%s\"",
			   defined, id);
	}
	
	/* We don't need this anymore */
	g_free (name);
	g_free (id);
	if (defined != NULL)
		g_free (defined);


	/* Load the options type */
	options_type = gpa_options_get_type_from_node (tree);
	if (options_type == GPA_OPTIONS_TYPE_ERROR)
		return NULL;
	options->options_type = options_type;

	/* Load the options */
	gpa_options_load_from_node (tree, &options);
	if (options == NULL)
		return NULL;

	/* Load Code Fragments */
	node = gpa_xml_search_child (tree, GPA_TAG_CODE_FRAGMENTS);
	if (node != NULL) {
		options->code_fragments = gpa_code_fragments_new_from_node (node, model);
		if (options->code_fragments == NULL)
			return NULL;
	}

	options->parent = parent;
	
	debug (FALSE, "end");

	return options;
}

GpaOptions *
gpa_options_copy (GpaOptions *options,
		  GpaModel *model)
{
	GpaOptions *new_options = NULL;
	GpaOption  *option;
	GpaOption  *new_option;
	GList *list;
	GList *new_list = NULL;

	if (options == NULL)
		return NULL;
	
 	new_options = gpa_options_new (options->model,
				       options->name,
				       options->id,
				       options->group);

	list = options->children;
	for ( ; list != NULL; list = list->next) {
		option = (GpaOption *) list->data;
		new_option = gpa_option_copy (option, new_options);
		new_list = g_list_prepend (new_list, new_option);
	}
	
	new_options->children = g_list_reverse (new_list);
	new_options->content  = options->content;
	new_options->options_type = options->options_type;
	new_options->model    = model;
	new_options->parent   = options->parent;

	new_options->code_fragments = gpa_code_fragments_copy (options->code_fragments);

	return new_options;
}

gboolean
gpa_options_list_copy (GpaPrinter *printer, GpaModel *new_model)
{
	GpaOptions *options;
	GpaOptions *new_options;
	GList *new_list = NULL;
	GList *list;

	list = printer->model->options_list;
	for ( ; list != NULL; list = list->next) {
		options = (GpaOptions *) list->data;
		new_options = gpa_options_copy (options, new_model);
		new_list = g_list_prepend (new_list, new_options);
	}
	new_list = g_list_reverse (new_list);

	new_model->options_list = new_list;

	return TRUE;
}


void
gpa_options_free (GpaOptions *options)
{
	GpaOption *option;
	GList *list;

	g_return_if_fail (options != NULL);
	g_return_if_fail (options->name != NULL);
	g_return_if_fail (options->id != NULL);

	list = options->children;
	for ( ; list != NULL; list = list->next) {
		option = (GpaOption *) list->data;
		gpa_option_free (option);
	}

	g_free (options->name);
	g_free (options->id);

	gpa_code_fragments_free (options->code_fragments);

}

void
gpa_options_list_free (GList * options_list)
{
	GpaOptions *options;
	GList *list;

	list = options_list;
	for ( ; list != NULL; list = list->next) {
		options = (GpaOptions *) list->data;
		gpa_options_free (options);
	}

}

static gchar *
gpa_options_group_string_dup (GpaOptions *options)
{
	debug (FALSE, "");

	switch (options->group) {
	case GPA_GROUP_PAPER:
		return g_strdup ("PaperGroup");
	case GPA_GROUP_QUALITY:
		return g_strdup ("QualityGroup");
	case GPA_GROUP_INSTALLED:
		return g_strdup ("InstalledGroup");
	case GPA_GROUP_SUBGROUP:
		return g_strdup ("SubGroup");
	case GPA_GROUP_ERROR:
	default:
		gpa_error ("Invalid group type. Could not "
			   "determine group string from group\n");
		return NULL;
	}
}


static gchar * 
gpa_options_dup_type (GpaOptions *options)
{
	debug (FALSE, "");

	switch (options->options_type) {
	case GPA_OPTIONS_TYPE_PICKONE:
		return g_strdup ("PickOne");
	case GPA_OPTIONS_TYPE_PICKMANY:
		return g_strdup ("PickMany");
	case GPA_OPTIONS_TYPE_BOOLEAN:
		return g_strdup ("Boolean");
	case GPA_OPTIONS_TYPE_NUMERIC:
		return g_strdup ("Numeric");
	case GPA_OPTIONS_TYPE_DOUBLE:
		return g_strdup ("Float");
	case GPA_OPTIONS_TYPE_ERROR:
	default:
		gpa_error ("Invalid options type. Could not determine "
			   "the option type string from options->type (dup_type)\n");
		return NULL;
	}
}
	
xmlNodePtr
gpa_options_write (XmlParseContext *context, GpaOptions *options)
{
	GpaOption *option;
	xmlNodePtr items;
	xmlNodePtr item;
	xmlNodePtr node;
	GList *list;
	gchar *type_str;
	gchar *options_group_str;
	
	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_OPTIONS (options), NULL);

	/* Write the basic info */
	node = xmlNewDocNode (context->doc, context->ns, GPA_TAG_OPTIONS, NULL);

	xmlNewChild (node, context->ns, GPA_TAG_NAME,  options->name);
	xmlNewChild (node, context->ns, GPA_TAG_ID,    options->id);
	if (options->frame != NULL)
		xmlNewChild (node, context->ns, GPA_TAG_FRAME, options->frame);
	xmlNewChild (node, context->ns, GPA_TAG_DEFINED,
		     options->defined ? GPA_TAG_TRUE:GPA_TAG_FALSE);
	
	/* We need to translate from enum to string */
	options_group_str = gpa_options_group_string_dup (options);
	if (options_group_str == NULL)
		return NULL;
	xmlNewChild (node, context->ns, GPA_TAG_OPTIONS_GROUP, options_group_str);
	g_free (options_group_str);

	/* Write the options type */
	type_str = gpa_options_dup_type (options);
	if (type_str == NULL)
		return NULL;
	xmlNewChild (node, context->ns, GPA_TAG_OPTIONS_TYPE, type_str);
	g_free (type_str);

	/* There are no options */
	if (options->children == NULL)
		return node;

	items = xmlNewChild (node, context->ns, GPA_TAG_ITEMS, NULL);

	/* Scan the list of option and write them */
	list = options->children;
	for ( ; list != NULL; list = list->next) {
		option = (GpaOption *)list->data;
 		item = gpa_option_write (context, option);
		if (item == NULL)
			return NULL;
		xmlAddChild (items, item);
	}

	/* Write the code fragments for this option */
	item = gpa_code_fragments_write (context, options->code_fragments);
	if (item != NULL)
		xmlAddChild (node, item);
	
	debug (FALSE, "end");
	
	return node;
}



/**
 * gpa_options_get_selected_option:
 * @settings: 
 * @options:
 * @fail: if TRUE, don't fix errors if found. Fail.
 * 
 * Returns the selected option from @options in @settings
 * the settings are not const because if there isn't an option
 * found, we need to fix the settings. This can happen if you
 * update a .model file and add a new "options" node.
 * 
 * Return Value: the seleceted option, NULL on error
 **/
GpaOption *
gpa_options_get_selected_option (GpaSettings *settings,
				 const GpaOptions *options,
				 gboolean fail)
{
	GpaOption *option;
	GList *list;

	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	g_return_val_if_fail (options != NULL, NULL);

	list = options->children;
	for ( ; list != NULL; list = list->next) {
		option = (GpaOption *) list->data;
		if (gpa_option_is_selected (settings, option))
			return option;
	}

	if (fail) {
		gpa_error ("Could not find a selected option for \"%s\" in \"%s\"",
			   gpa_options_get_name (options),
			   gpa_settings_get_name (settings));
		return NULL;
	}
	
	g_return_val_if_fail (options->children != NULL, NULL);

	option = (GpaOption *) options->children->data;

	if (gpa_settings_select_option (settings, option))
		gpa_error ("Could not find a selected option in \"%s\". FIXED ...\n", options->name);
	else
		gpa_error ("Could not find a selected option in \"%s\". Unable to FIX ...\n", options->name);
	
	debug (FALSE, "end");

	return option;
}

gboolean
gpa_options_verify (GpaOptions *options,
		    const GpaModel *model)
{
	GpaOption *option;
	gchar *temp;
	GList *list;

	debug (FALSE, "");

	if (options->name == NULL) {
		gpa_error ("The Options object did not contained a name");
		return FALSE;
	}
	if (options->id == NULL) {
		gpa_error ("The Options \"%s\"object did not contained a valid id",
			   options->name);
		return FALSE;
	}

	/* Verify that the option type is valid */
	temp = gpa_options_dup_type (options);
	if (temp == NULL)
		return FALSE;
	g_free (temp);
  	
	if ((options->group != GPA_GROUP_PAPER) &&
	    (options->group != GPA_GROUP_QUALITY) &&
	    (options->group != GPA_GROUP_INSTALLED) &&
	    (options->group != GPA_GROUP_SUBGROUP) &&
	    (options->group != GPA_GROUP_COMPRESSION) &&
	    (options->group != GPA_GROUP_PS)) {
		gpa_error ("The Options \"%s\" object contained an invalid group "
			   "[gpa_options_verify. L:%i]",
			   options->name,
			   __LINE__);
		return FALSE;
	}
	
	if ((options->content != GPA_CONTENT_GENERIC) &&
	    (options->content != GPA_CONTENT_PAPER_SIZE) &&
	    (options->content != GPA_CONTENT_PAPER_MEDIA) &&
	    (options->content != GPA_CONTENT_PAPER_SOURCE) &&
	    (options->content != GPA_CONTENT_PAPER_ORIENTATION) &&
	    (options->content != GPA_CONTENT_RESOLUTION) &&
	    (options->content != GPA_CONTENT_RESOLUTION_MODE) &&
	    (options->content != GPA_CONTENT_ERROR)) {
		gpa_error ("The Options \"%s\" object contained an invalid content id [%i]",
			   options->name, options->content);
		return FALSE;
	}

	if (options->children == NULL) {
		gpa_error ("The Options object \"%s\" does not contain children",
			   options->name);
		return FALSE;
	}
	list = options->children;
	for (; list != NULL; list = list->next) {
		option = (GpaOption *) list->data;
		if (!gpa_option_verify (option, model)) {
			return FALSE;
		}
	}

	if (!gpa_code_fragments_verify_with_options (options))
		return FALSE;

	return TRUE;
}

gboolean
gpa_options_verify_with_settings (GpaOptions *options,
				  GpaSettings *settings)
{
	GpaOptionsType type;
	GpaOption *option;
	GList *list;

	debug (FALSE, "");

	if (!gpa_options_verify (options, settings->printer->model))
		return FALSE;

	list = options->children;
	for (; list != NULL; list = list->next) {
		option = (GpaOption *) list->data;
		if (!gpa_option_verify_with_settings (option, settings)) {
			return FALSE;
		}
	}

	/* For pickone and boolean option types, verify that there
	 * is a selected  option */
	type = gpa_options_get_type (options);
	
	if ((type == GPA_OPTIONS_TYPE_PICKONE) ||
	    (type == GPA_OPTIONS_TYPE_BOOLEAN)) {
		
		option = gpa_options_get_selected_option (settings, options, TRUE);
		
		if (option == NULL) {
			gpa_error ("The \"%s\" options and \"%s\" Settings\n does not "
				   "have a selected option",
				   gpa_options_get_name (options),
				   gpa_settings_get_name (settings));
			return FALSE;
		}
	}

	return TRUE;
}


gboolean
gpa_options_list_verify (const GpaModel *model)
{
	GpaOptions *options;
	GList *list;
	gint number;
	gint n;

	debug (FALSE, "");
	
	list = model->options_list;
	for (; list != NULL; list = list->next) {
		options = (GpaOptions *) list->data;
		if (!gpa_options_verify (options, model))
			return FALSE;
	}


	number = sizeof(gpa_required_options) / sizeof(GpaRequiredOptions);
	for (n = 0; n < number; n++) {
		if (gpa_model_get_options_from_id (model,
						   gpa_required_options[n].id) == NULL) {
			gpa_error ("The required -options- \"%s\" could not be found",
				   gpa_required_options[n].id);
			return FALSE;
		}
	}
		
	return TRUE;
}

gboolean
gpa_options_list_verify_with_settings (const GpaModel *model,
				       GpaSettings *settings)
{
	GpaOptions *options;
	GList *list;
	gint number;
	gint n;

	debug (FALSE, "");
	
	list = model->options_list;
	for (; list != NULL; list = list->next) {
		options = (GpaOptions *) list->data;
		if (!gpa_options_verify_with_settings (options, settings))
			return FALSE;
	}


	number = sizeof(gpa_required_options) / sizeof(GpaRequiredOptions);
	for (n = 0; n < number; n++) {
		if (gpa_model_get_options_from_id (model,
						   gpa_required_options[n].id) == NULL) {
			gpa_error ("The required -options- \"%s\" could not be found",
				   gpa_required_options[n].id);
			return FALSE;
		}
	}
		
	return TRUE;
}

xmlNodePtr
gpa_options_write_paths_from_model (XmlParseContext *context,
				    GpaModel *model)
{
	const gchar *path;
	xmlNodePtr node;
	GList *list;

	debug (FALSE, "");

	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (model   != NULL, NULL);

	node = xmlNewDocNode (context->doc, context->ns,
			      GPA_TAG_PATHS, NULL);

	list = model->default_settings;
	for (; list != NULL; list = list->next) {
		path = list->data;
		xmlNewChild (node, NULL, GPA_TAG_PATH, path);
	}

	return node;
}

xmlNodePtr
gpa_options_list_write (XmlParseContext *context,
			GList *list_)
{
	GpaOptions *options;
	xmlNodePtr child;
	xmlNodePtr node;
	GList *list;

	debug (FALSE, "");

	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (list_   != NULL, NULL);
	
	/* Write the FileInfo */
	node = xmlNewDocNode (context->doc, context->ns,
			      GPA_TAG_OPTIONS_LIST, NULL);

	/* Scan the list of options and if option->ty */
	list = list_;
	for ( ; list != NULL; list = list->next) {
		options = (GpaOptions *)list->data;
		if (options == NULL) {
			gpa_error ("Options == NULL.");
			continue;
		}
		child = gpa_options_write (context, options);
		if (child != NULL)
			xmlAddChild (node, child);
	}

	debug (FALSE, "end");
	
	return node;
}

GList *
gpa_options_list_new_from_node (xmlNodePtr tree_, GpaModel *model)
{
	GpaOptions *options;
	xmlNodePtr child;
	xmlNodePtr node;
	xmlNodePtr tree;

	debug (FALSE, "");

	tree = gpa_include_node (tree_);

	if (!gpa_xml_node_verify (tree, GPA_TAG_PRINTER_MODEL))
		return NULL;
	
	child = gpa_xml_search_child_required (tree, GPA_TAG_OPTIONS_LIST);
	if (child == NULL)
		return NULL;

	node = child->childs;
	while (node != NULL) {
		skip_text (node);
		if (!gpa_xml_node_verify (node, GPA_TAG_OPTIONS))
			return NULL;
		options = gpa_options_new_from_node (model, node, NULL);
		if (options == NULL)
			return NULL;
		/* We need to load each option as we load it so that we can
		 * verify inside gpa_options_new_from_node that the option
		 * we are loading has an id that is not beeing used by another
		 * option in that model
		 */
		model->options_list = g_list_prepend (model->options_list, options);
		node = node->next;
	}

	debug (FALSE, "end");
	
	return g_list_reverse (model->options_list);
}



gchar *
gpa_options_dup_path (const GpaOptions *options)
{
	gchar *path;
	
	debug (FALSE, "");

	g_return_val_if_fail (options != NULL, NULL);

	if (options->parent != NULL) {
		if (options->parent->parent->parent != NULL) {
			gpa_error ("We only support 1st & 2nd level option");
		}
		path  = g_strdup_printf ("%s" GPA_PATH_DELIMITER "%s" GPA_PATH_DELIMITER "%s",
					 options->parent->parent->id,
					 options->parent->id,
					 options->id);
	} else {
		path = g_strdup_printf ("%s", options->id);
	}

	return path;
}


gboolean
gpa_options_have_children (const GpaOptions *options)
{
	GpaOption *option;
	GList *list;

	debug (FALSE, "");
	
	list = gpa_options_get_children (options);
	for (; list != NULL; list = list->next) {
		option = (GpaOption *)list->data;

		if (gpa_option_get_children (option) != NULL)
			return TRUE;
	}
	return FALSE;
}


GpaPickoneType
gpa_options_get_pickone_type (const GpaOptions *options)
{
	gint number, n;
	const gchar *id;

	if (gpa_options_have_children (options))
		return GPA_PICKONE_RADIO;

	id = gpa_options_get_id (options);

	number = sizeof(gpa_known_option_types) / sizeof(GpaKnownOptionType);
	for (n = 0; n < number; n++)
		if (strcmp (id, gpa_known_option_types[n].id) == 0)
			break;

	if (n == number)
		return GPA_PICKONE_COMBO;

	return gpa_known_option_types[n].pickone_type;
}
			      
/* Access to the struct */

GList *
gpa_options_get_children (const GpaOptions *options)
{
	g_return_val_if_fail (GPA_IS_OPTIONS (options), NULL);

	return options->children;
}

const gchar *
gpa_options_get_name (const GpaOptions *options)
{
	g_return_val_if_fail (GPA_IS_OPTIONS (options), NULL);

	return options->name;
}

const gchar *
gpa_options_get_id (const GpaOptions *options)
{
	g_return_val_if_fail (GPA_IS_OPTIONS (options), NULL);

	return options->id;
}

gchar *
gpa_options_dup_id (const GpaOptions *options)
{
	g_return_val_if_fail (GPA_IS_OPTIONS (options), NULL);

	return g_strdup (options->id);
}

const GpaOption *
gpa_options_get_parent (const GpaOptions *options)
{
	g_return_val_if_fail (GPA_IS_OPTIONS (options), NULL);

	return options->parent;
}

GpaOptionsType
gpa_options_get_type (const GpaOptions *options)
{
	g_return_val_if_fail (GPA_IS_OPTIONS (options), GPA_OPTIONS_TYPE_ERROR);

	return options->options_type;
}

GpaOptionsGroup
gpa_options_get_group (const GpaOptions *options)
{
	g_return_val_if_fail (GPA_IS_OPTIONS (options), GPA_GROUP_ERROR);

	return options->group;
}

GpaContent
gpa_options_get_content (const GpaOptions *options)
{
	g_return_val_if_fail (GPA_IS_OPTIONS (options), GPA_CONTENT_ERROR);

	return options->content;
}

const gchar *
gpa_options_get_frame (const GpaOptions *options)
{
	g_return_val_if_fail (GPA_IS_OPTIONS (options), NULL);

	return options->frame;
}

