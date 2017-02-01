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

#include <gtk/gtkmain.h>
#include <libgnome/gnome-util.h>

#include <unistd.h> /* For getpid () */
#include <time.h>   /* For time () */
#include <string.h> /* For strncmp */

#include "gpa-private.h"
#include "gpa-include.h"
#include "gpa-backend.h"
#include "gpa-backend-private.h"
#include "gpa-code.h"
#include "gpa-code-private.h"
#include "gpa-constraints.h"
#include "gpa-constraints-private.h"
#include "gpa-model.h"
#include "gpa-model-private.h"
#include "gpa-option.h"
#include "gpa-option-private.h"
#include "gpa-options.h"
#include "gpa-options-private.h"
#include "gpa-settings.h"
#include "gpa-settings-private.h"
#include "gpa-utils.h"
#include "gpa-vendor.h"
#include "gpa-vendor-private.h"
#include "xml-utils.h"
#include "text-utils.h"
#include "gpa-values.h"

/* Static functions */
static void gpa_model_init       (GpaModel *model);


static void gpa_model_class_init (GpaModelClass *klass);
static void gpa_model_finalize   (GtkObject *object);

static GtkObjectClass *parent_class = NULL;

/* Ciclic dependency */
GtkType
gpa_model_get_type (void)
{
	static GtkType model_type = 0;

	if (!model_type)
	{
		GtkTypeInfo model_info =
		{
			"GpaModel",
			sizeof (GpaModel),
			sizeof (GpaModelClass),
			(GtkClassInitFunc)  gpa_model_class_init,
			(GtkObjectInitFunc) gpa_model_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		model_type = gtk_type_unique (gtk_object_get_type (), &model_info);
	}
	
	return model_type;
}

static void
gpa_model_class_init (GpaModelClass *class)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass*) class;

	parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->finalize = gpa_model_finalize;
}


static void
gpa_model_init (GpaModel *model)
{
	model->name   = NULL;
	model->id     = NULL;
	model->vendor = NULL;

        model->code_fragments   = NULL;
        model->backend_list     = NULL;
        model->options_list     = NULL;
        model->default_settings = NULL;

	model->model_info     = NULL;
	model->default_values = NULL;
}

static void
gpa_model_finalize (GtkObject *object)
{
	GpaModel *model;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GPA_IS_MODEL (object));

	model = GPA_MODEL (object);

	/* FIXME: Free everything */
	
        (* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}


GpaModel *
gpa_model_new (void)
{
	GpaModel *model;

        model = gtk_type_new (gpa_model_get_type ());

	return model;
}



const gchar*
gpa_model_get_info (const GpaModel *model, const gchar *id)
{
	const gchar *info;

	debug (FALSE, "");

	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (model != NULL, NULL);

	info = gpa_hash_item_get (model->model_info, id);

	if (info == NULL) {
		gpa_error ("Could not find the \"%s\" key in the models's hash table",
			   id);
		return NULL;
	}

	return info;
}


gboolean
gpa_model_verify_with_settings (const GpaModel *model,
				GpaSettings *settings)
{
	if (!gpa_model_verify (model))
		return FALSE;

	if (!gpa_options_list_verify_with_settings (model,
						    settings))
		return FALSE;

	return TRUE;
}

	
gboolean
gpa_model_verify (const GpaModel *model)
{
	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_MODEL (model), FALSE);
	
	if (model->name == NULL) {
		gpa_error ("The model does not contain an name");
		return FALSE;
	}
	if (model->vendor == NULL) {
		gpa_error ("The model does not contain an vendor");
		return FALSE;
	}
	if (model->id == NULL) {
		gpa_error ("The model does not contain an id");
		return FALSE;
	}

	if (!gpa_hash_verify (model->model_info, gpa_known_model_values, FALSE, "Model Info"))
		return FALSE;

	if (!gpa_options_list_verify (model))
		return FALSE;

	if (!gpa_backend_list_verify (model->backend_list))
		return FALSE;

#warning Check if the defalt settings list do not contain duplicated entries, Copy the code from
#warning somwehere else where we are looking for duplicated paths.
	
	debug (FALSE, "end");
	
	return TRUE;
}


/**
 * gpa_model_add_missing_setting_from_options:
 * @options: options to add to the list if the list does not contains it
 * @list_: 
 * 
 * If the list_ does not contains a path for the options object, it is added
 * to the list. see 
 * 
 * Return Value: TRUE on success, FALSE otherwise
 **/
static gboolean
gpa_model_add_missing_settings_from_options (GpaOptions *options, GList **list_)
{
	const gchar *test_path;
	GpaOption *option;
	GList *list;
	gchar *options_path;
	gchar *option_path;
	gint size;
	
	g_return_val_if_fail (GPA_IS_OPTIONS (options), FALSE);

	/* Scan the list of options to check the suboptions */
	list = options->children;
	for (; list != NULL; list = list->next) {
		option = list->data;
		if (option->children != NULL)
			gpa_model_add_missing_settings_from_options (option->children, list_);
	}
			
	
	/* Scan the list of options to see if this option is already included */
	options_path = gpa_options_dup_path (options);
	list = *list_;
	for (; list != NULL; list = list->next) {
		test_path = list->data;
		size = tu_get_pos_of_last_delimiter (test_path, GPA_PATH_DELIMITER_CHAR);
		if (size < 0)
			g_print ("test path %s\n", test_path);
		g_return_val_if_fail (size > 0, FALSE);
		if (strncmp (options_path, test_path, size) == 0) {
			g_free (options_path);
			return TRUE;
		}
	}
	g_free (options_path);

	/* If it was not found, add it to the list */
	list = *list_;
	g_return_val_if_fail (options->children != NULL, FALSE);
	option = options->children->data;
	option_path = gpa_option_dup_path (option);
	list = g_list_prepend (list, option_path);
	*list_ = list;
	
	return TRUE;
}


/**
 * gpa_model_add_mising_default_settings_and_values:
 * @model: 
 * 
 * The xml file describing the model contains a list of Default settings.
 * This funciton adds to model the default settings the where not specified
 * by the xml file.
 * 
 * Return Value: 
 **/
gboolean
gpa_model_add_missing_default_settings_and_values (GpaModel *model)
{
	GpaOptions *options;
	GpaValue *value;
	GList *settings_list;
	GList *list;
	gint number;
	gint n;

	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_MODEL (model), FALSE);
	g_return_val_if_fail (model->options_list != NULL, FALSE);

	settings_list = model->default_settings;
	list = model->options_list;
	for (; list != NULL; list = list->next) {
		options = list->data;
		if (!gpa_model_add_missing_settings_from_options (options, &settings_list))
			return FALSE;
	}
	model->default_settings = settings_list;

	/* Now add the required values if they are not found */
	number = sizeof (gpa_required_settings_values) / sizeof (GpaRequiredSettingsValues);
	for (n = 0; n < number; n++) {
		const gchar *key;
		key = gpa_required_settings_values [n].key;
		list = model->default_values;
		for (; list != NULL; list = list->next) {
			value = (GpaValue *)list->data;
			if (strcmp (value->key, key) == 0)
				break;
		}
		if (list == NULL) {
			const gchar *default_value = gpa_required_settings_values[n].default_value;
			value = gpa_value_new (key, default_value);
			model->default_values = g_list_prepend (model->default_values, value);
		}
	}
	

	return TRUE;
}


static gboolean 
gpa_model_load_from_node (XmlParseContext *context,
			  xmlNodePtr tree,
			  GpaModel *model)
{
	GpaVendor *vendor;
	xmlNodePtr child;
	xmlNodePtr child_child;
	gint ret = TRUE;
	gchar *vendor_name;
	gchar *name;
	gchar *id;
	
	debug (FALSE, "");

	if (!gpa_xml_node_verify (tree, GPA_TAG_PRINTER_MODEL))
		return FALSE;

	child = gpa_xml_search_child_required (tree, GPA_TAG_MODEL_INFO);
	if (child == NULL)
		return FALSE;
	name = gpa_xml_get_value_string_required (child, GPA_TAG_MODEL_NAME, NULL);
	if (name == NULL)
		return FALSE;

	id = gpa_xml_get_value_string_required (child, GPA_TAG_MODEL_ID, NULL);
	if (id == NULL)
		return FALSE;

	vendor_name = gpa_xml_get_value_string_required (child, GPA_TAG_VENDOR_NAME, NULL);
	if (vendor_name == NULL)
		return FALSE;

	vendor = gpa_vendor_get_from_name (vendor_name);
	g_return_val_if_fail (vendor != NULL, FALSE);

	g_free (vendor_name);
	
	model->name   = name;
	model->id     = id;
	model->vendor = vendor;
	
	/* Load the backedns */
	model->backend_list   = gpa_backend_list_new_from_node (tree);
	if (model->backend_list == NULL)
		ret = FALSE;

	/* Load options list */
	model->options_list   = gpa_options_list_new_from_node (tree, model);
	if (model->options_list == NULL)
		ret = FALSE;

	/* Load the values */
	child = gpa_xml_search_child_required (tree, GPA_TAG_MODEL_INFO);
	if (child == NULL)
		return FALSE;
	model->model_info = gpa_xml_utils_new_hash_from_node (child,
							  GPA_TAG_MODEL_INFO);

	/* Load the default settings */
	child = gpa_xml_search_child_required (tree, GPA_TAG_DEFAULT_SETTINGS);
	if (child == NULL)
		return FALSE;
	if (!gpa_settigns_load_default_paths_from_node (child, model))
		return FALSE;
	
	/* Load the SettingsInfo */
	child_child = gpa_xml_search_child_required (child, GPA_TAG_SETTINGS_INFO);
	if (child_child == NULL)
		return FALSE;
	model->default_values = gpa_values_new_from_node (child_child,
							  GPA_TAG_SETTINGS_INFO);

	/* We have loaded the deafult settings from the xml file, now we need
	   to add the paths of the settings that where not specified in the xml file.
	   And add a default value for the numerical options */
	if (!gpa_model_add_missing_default_settings_and_values (model))
		return FALSE;

	
	if (model->model_info == NULL)
		ret = FALSE;
	if (model->default_values == NULL)
		ret = FALSE;

	/* Load the constraints */
	child = gpa_xml_search_child_required (tree, GPA_TAG_CONSTRAINTS);
	if (child == NULL)
		return FALSE;
	if (!gpa_constraints_load_from_node (child, model))
		return FALSE;

	/* Load the code fragments */
	child = gpa_xml_search_child (tree, GPA_TAG_CODE_FRAGMENTS);
	if (child != NULL) {
		model->code_fragments = gpa_code_fragments_new_from_node (child, model);
		if (model->code_fragments == NULL)
			ret = FALSE;
	}

	if (!gpa_constraints_double_links_model (model))
		ret = FALSE;


	return ret;
}

static GpaModel *
gpa_model_new_from_id (const gchar *model_id)
{
	GpaModel *model;
	XmlParseContext *context;
	gchar *full_path;

	debug (FALSE, "");

	full_path = g_strdup_printf (GPA_DATA_DIR "%s/%s%s",
				     "models", model_id, GPA_MODEL_EXTENSION_NAME);

	model = gpa_model_new ();
	
	context = gpa_xml_parse_context_new_from_path (full_path,
						       GPA_TAG_MODEL_NAME_SPACE,
						       GPA_TAG_PRINTER_MODEL);
	if (context == NULL)
		return NULL;

	if (!gpa_model_load_from_node (context, context->doc->root, model)) {
		gpa_error ("The model \"%s\" could not be loaded", model_id);
		return NULL;
	}

	gpa_xml_parse_context_free (context);

	if (!gpa_model_verify (model)) {
		gpa_error ("The model was not loaded from \"%s\" because it contained "
			   "at least one error", full_path);
		return NULL;
	}

	g_free (full_path);
	
	return model;
}


GpaModel *
gpa_model_get_from_id (const gchar *model_id)
{
	static GList *loaded_models = NULL;
	GpaModel *model;
	GList *list;

	debug (FALSE, "");

	g_return_val_if_fail (model_id    != NULL, NULL);

	/* First we scan the list of loaded models to se if the
	   model we are looking for is already loaded */
	list = loaded_models;
	for (; list != NULL; list = list->next) {
		model = (GpaModel *) list->data;
		if (strcmp (model->id, model_id) == 0)
		    return model;
	}

	model = gpa_model_new_from_id (model_id);
	if (model == NULL) {
		gpa_error ("Could not create model from id \"%s\"\n",
			   model_id);
		return NULL;
	}

	loaded_models = g_list_prepend (loaded_models, model);

	return model;
}

static xmlNodePtr
gpa_model_write (XmlParseContext *context, GpaModel *model)
{
	xmlNodePtr node;
	xmlNodePtr child;
	xmlNodePtr child_child;
	xmlNsPtr gmr;
	
	g_return_val_if_fail (context != NULL,                NULL);
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);
	g_return_val_if_fail (model->model_info != NULL,      NULL);
	g_return_val_if_fail (model->default_values != NULL,  NULL);

	if (!gpa_model_verify (model)) {
		gpa_error ("Could not save model, because it could not be verified");
		return NULL;
	}

	node = xmlNewDocNode (context->doc, context->ns, GPA_TAG_PRINTER_MODEL, NULL);
	if (node == NULL)
		return NULL;

	gmr = xmlNewNs (node, GPA_TAG_MODEL_NAME_SPACE, "gmr");
	xmlSetNs (node, gmr);

	context->ns = gmr;

	child = xmlNewDocNode (context->doc, context->ns, GPA_TAG_DEFAULT_SETTINGS, NULL);
	if (child != NULL)
		xmlAddChild (node, child);

	child_child = gpa_options_write_paths_from_model (context, model);
	if (child != NULL)
		xmlAddChild (child, child_child);

	child_child = xmlNewDocNode (context->doc, context->ns, GPA_TAG_SETTINGS_INFO, NULL);
	if (child != NULL)
		xmlAddChild (child, child_child);
	
	child = gpa_hash_write (context, model->model_info, GPA_TAG_MODEL_INFO);
	if (child != NULL)
		xmlAddChild (node, child);

	child = gpa_backend_list_write (context, model->backend_list);
	if (child != NULL)
		xmlAddChild (node, child);

	child = xmlNewDocNode (context->doc, context->ns, GPA_TAG_CONSTRAINTS, NULL);
	if (child != NULL)
		xmlAddChild (node, child);
	
	child = gpa_options_list_write (context, model->options_list);
	if (child != NULL)
		xmlAddChild (node, child);

	child = gpa_code_fragments_write (context, model->code_fragments);
	if (child != NULL)
		xmlAddChild (node, child);

	return node;
}


gboolean
gpa_model_save (GpaModel *model)
{
	XmlParseContext *context;
	xmlDocPtr xml;
	gchar *full_path;
	gint ret;

	debug (FALSE, "");
	
	g_return_val_if_fail (model != NULL, FALSE);

	full_path = g_strdup_printf (GPA_DATA_DIR "%s/%s%s",
				     "models", model->id, GPA_MODEL_EXTENSION_NAME);

	g_print ("Saving in %s\n", full_path);
	
	xml = xmlNewDoc ("1.0");
	if (xml == NULL) {
		gpa_error ("Could not create xml doc");
		return FALSE;
	}

	context = gpa_xml_parse_context_new (xml, NULL);
	xml->root = gpa_model_write (context, model);
	gpa_xml_parse_context_destroy (context);
	if (xml->root == NULL)
		return FALSE;

	ret = xmlSaveFile (full_path, xml);
	xmlFreeDoc (xml);

	if (ret < 0) {
		gpa_error ("Could not save File");
		return FALSE;
	}

	return TRUE;
}




/* Access to the struct */
GpaOptions *
gpa_model_get_options_from_id (const GpaModel *model, const gchar *id)
{
	GpaOptions *options;
	GList *list;

	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);
	g_return_val_if_fail (id != NULL, NULL);
	
	list = model->options_list;

	for (; list != NULL; list = list->next) {
		options = (GpaOptions*) list->data;
		if (strcmp (options->id, id) == 0)
			return options;
	}

	debug (FALSE, "end");
	
	return NULL;
}

GList *
gpa_model_get_options_list (const GpaModel *model)
{
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);

	return model->options_list;
}

const GpaVendor *
gpa_model_get_vendor (const GpaModel *model)
{
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);

	return model->vendor;
}


const gchar *
gpa_model_get_name (const GpaModel *model)
{
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);

	return model->name;
}

gchar *
gpa_model_dup_name (const GpaModel *model)
{
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);

	return g_strdup (model->name);
}

const gchar *
gpa_model_get_id (const GpaModel *model)
{
	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);

	return model->id;
}
