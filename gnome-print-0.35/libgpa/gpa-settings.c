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
#include <string.h> /* for strncmp () */
#include <gtk/gtkmain.h>
#include <libgnome/gnome-util.h>

#include "gpa-private.h"
#include "xml-utils.h"
#include "text-utils.h"
#include "gpa-include.h"
#include "gpa-settings.h"
#include "gpa-settings-private.h"
#include "gpa-model.h"
#include "gpa-model-private.h"
#include "gpa-option.h"
#include "gpa-option-private.h"
#include "gpa-options.h"
#include "gpa-options-private.h"
#include "gpa-utils.h"
#include "gpa-printer.h"
#include "gpa-printer-private.h"
#include "gpa-backend.h"
#include "gpa-backend-private.h"
#include "gpa-values.h"


/* FIXME : this should go somewhere else. Is there a function that does
 * this already ?
 */
static gint
gpa_text_utils_search_backwards (const gchar *buffer, gchar findme)
{
	gint length;
	gint n;

	length = strlen (buffer);
	
	for (n = length; n > 0; n--)
		if (buffer [n-1] == findme)
			return n - 1;
	
	return -1;
}

static void gpa_settings_init (GpaSettings *settings);


static void gpa_settings_class_init (GpaSettingsClass *klass);
static void gpa_settings_finalize   (GtkObject *object);

static GtkObjectClass *parent_class = NULL;

GtkType
gpa_settings_get_type (void)
{
	static GtkType settings_type = 0;

	if (!settings_type)
	{
		GtkTypeInfo settings_info =
		{
			"GpaSettings",
			sizeof (GpaSettings),
			sizeof (GpaSettingsClass),
			(GtkClassInitFunc)  gpa_settings_class_init,
			(GtkObjectInitFunc) gpa_settings_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		settings_type = gtk_type_unique (gtk_object_get_type (), &settings_info);
	}
	
	return settings_type;
}

static void
gpa_settings_class_init (GpaSettingsClass *class)
{
	GtkObjectClass *object_class;

	object_class = (GtkObjectClass*) class;

	parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->finalize = gpa_settings_finalize;
}

static void
gpa_settings_init (GpaSettings *settings)
{
	settings->name = NULL;
	settings->command = NULL;
	settings->selected_options = NULL;
	settings->values = NULL;
	settings->printer = NULL;
	settings->selected = FALSE;
}

static void
gpa_settings_finalize (GtkObject *object)
{
	GpaSettings *settings;
	GList *list;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GPA_IS_SETTINGS (object));

	settings = GPA_SETTINGS (object);

	g_free (settings->name);
	g_free (settings->command);

	/* Free the Options paths*/	
	list = settings->selected_options;
	for (; list != NULL; list = list->next) {
		gchar *path = (gchar *) list->data;
		g_free (path);
	}
	g_list_free (settings->selected_options);

	/* Free the GpaValues */
	gpa_values_free_list (settings->values);
	settings->values = NULL;
	
	(* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}

#if 0 /* FIXME: Implement */
static gboolean
gpa_settings_list_free (GList *settings_list)
{
	GpaSettings *settings;
	GList *list;

	g_return_val_if_fail (settings_list != NULL, FALSE);

	list = settings_list;
	for (; list != NULL; list = list->next) {
		settings = GPA_SETTINGS (list->data);
		if (settings == NULL)
			return FALSE;
		gtk_object_unref (GTK_OBJECT (settings));
	}

	g_list_free (settings_list);

	return TRUE;
}
#endif


static GpaSettings *
gpa_settings_new (void)
{
	GpaSettings *settings;

	settings = gtk_type_new (gpa_settings_get_type ());

	return settings;
}

static GList *
gpa_settings_paths_copy (GList *source)
{
	GList *dest = NULL;
	GList *list;
	gchar *path;

	if (source == NULL)
		return NULL;
	g_return_val_if_fail (source != NULL, NULL);

	list = source;
	for (; list != NULL; list = list->next) {
		path = list->data;
		dest = g_list_prepend (dest, g_strdup (path));
	}

	return g_list_reverse (dest);
}

GpaSettings *
gpa_settings_new_from_model (GpaModel *model,
			     GpaPrinter *printer)
{
	GpaSettings *settings;
	GpaBackend *backend;

	g_return_val_if_fail (GPA_IS_MODEL (model), NULL);
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);
		
	settings = gpa_settings_new ();
	
	settings->name             = g_strdup (_("Default Settings"));
	settings->command          = g_strdup ("lpr");
	settings->printer          = printer;
	settings->selected_options = gpa_settings_paths_copy (model->default_settings);
	settings->values           = gpa_values_copy_list (model->default_values);
	settings->selected         = TRUE;

	/* Add the default Backend */
	backend = gpa_backend_get_default (model);
	gpa_settings_value_insert (settings, GPA_TAG_BACKEND, backend->id);

	if (!gpa_settings_verify (settings, TRUE)) {
		gpa_error ("Could not create settings from model \"%s\"\n",
			   gpa_model_get_name (model));
		return NULL;
	}

	return settings;
}

static xmlNodePtr
gpa_settings_options_write (XmlParseContext *context, GpaSettings *settings)
{
	xmlNodePtr node;
	GList *list;
	gchar *path;

	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);

	node = xmlNewDocNode (context->doc, context->ns, GPA_TAG_SELECTED_OPTIONS, NULL);

	list = settings->selected_options;
	for (; list != NULL; list = list->next) {
		path = (gchar *) list->data;
		xmlNewChild (node, context->ns, GPA_TAG_PATH, path);
	}

	return node;
}

static xmlNodePtr
gpa_settings_write (XmlParseContext *context, GpaSettings *settings)
{
	xmlNodePtr node;
	xmlNodePtr child;

	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);

	node = xmlNewDocNode (context->doc, context->ns, GPA_TAG_SETTINGS, NULL);

	xmlNewChild (node, context->ns, GPA_TAG_NAME, settings->name);
	xmlNewChild (node, context->ns, GPA_TAG_COMMAND, settings->command);
	xmlNewChild (node, context->ns, GPA_TAG_SELECTED,
		     settings->selected ? GPA_TAG_TRUE: GPA_TAG_FALSE);

	child = gpa_settings_options_write (context, settings);
	if (child == NULL)
		return NULL;
	xmlAddChild (node, child);

	child = gpa_values_write_list (context, settings->values,
				       GPA_TAG_SETTINGS_INFO);
	if (child == NULL)
		return NULL;
	xmlAddChild (node, child);

	return node;
}

xmlNodePtr
gpa_settings_list_write (XmlParseContext *context, GList *settings_list)
{
	GpaSettings *settings = NULL;
	xmlNodePtr child;
	xmlNodePtr node;
	GList *list;

	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (settings_list != NULL, NULL);

	node = xmlNewDocNode (context->doc,
			      context->ns,
			      GPA_TAG_SETTINGS_LIST,
			      NULL);
	
	list = settings_list;
	for (; list != NULL; list = list->next) {
		settings = (GpaSettings *) list->data;
		if (!GPA_IS_SETTINGS (settings))
			return NULL;
		child = gpa_settings_write (context, settings);
		if (child == NULL)
			return NULL;
		xmlAddChild (node, child);
	}

	return node;
}
		
		

gboolean
gpa_settings_verify (GpaSettings *settings, gboolean fail)
{
	GpaModel *model;
	GpaOption *option;
	GList *list;
	GList *list2;
	GList *remove = NULL;
	gchar *path;
	gchar *path2;
	
	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);

	model = settings->printer->model;

	g_return_val_if_fail (GPA_IS_MODEL (model), FALSE);

	if (settings->selected_options == NULL) {
		if (settings->printer->model->options_list != NULL) {
			gpa_error ("The settings \"%s\", does not contain a list of selected options",
				   settings->name);
		return FALSE;
		}
	}

	list = settings->selected_options;
	for (; list != NULL; list = list->next) {
		path = (gchar *)list->data;
		option = gpa_option_get_from_model_and_path (model, path);
		if (option == NULL) {
			g_warning ("Removing the %s path from the selected_options list. FIXED ..",
				   path);
			if (fail)
				return FALSE;
			remove = g_list_prepend (remove, path);
			continue;
		}
		
		/* Now verify that this option is not repeated in the list */
		list2 = settings->selected_options;
		for (; list2 != NULL; list2 = list2->next) {
			gint n,m;
			path2 = (gchar *)list2->data;
			if (path2 == path)
				continue;
			n = gpa_text_utils_search_backwards (path,  GPA_PATH_DELIMITER_CHAR);
			m = gpa_text_utils_search_backwards (path2, GPA_PATH_DELIMITER_CHAR);
			if (strncmp (path, path2, MAX(n,m)) == 0) {
				g_warning ("The path \"%s\" was duplicated in the \"%s\" settings. FIXED ..\n",
					   path, gpa_settings_get_name (settings));
				if (fail)
					return FALSE;
				if (!g_list_find (remove, path2))
				    remove = g_list_prepend (remove, path2);
				continue;
			}
		}
		
		if (!gpa_option_verify (option, settings->printer->model))
			return FALSE;
	}

	/* Dump the current list of settings for debuggin */
	if (remove != NULL) {
		g_print ("Here is the list of settings before cleaning:\n");
		list = settings->selected_options;
		for (; list != NULL; list = list->next) {
			path = (gchar *)list->data;
			g_print ("[%s]\n", path);
		}
		g_print ("\n");
	}

	/* Now remove the paths that are invalid. They point to an option that does
	 * not exist. This can happen if the user updates a .model file and an option
	 * that was present before, is no longer found. (or it's id was renamed)
	 */
	list = remove;
	for (; list != NULL; list = list->next) {
		path = (gchar *)list->data;
		settings->selected_options = g_list_remove (settings->selected_options, path);
		g_free (path);
	}

	/* Dump the current list of settings for debuggin */
	if (remove != NULL) {
		g_print ("Here is the list of settings after cleaning :\n");
		list = settings->selected_options;
		for (; list != NULL; list = list->next) {
			path = (gchar *)list->data;
			g_print ("[%s]\n", path);
		}
		g_print ("\n");
	}

	g_list_free (remove);

	if (!gpa_values_verify_settings (settings))
		return FALSE;
	
	if (!gpa_model_verify_with_settings (settings->printer->model, settings))
		return FALSE;
	
	return TRUE;
}

gboolean 
gpa_settings_list_verify (GList *settings_list, gboolean fail)
{
	GpaSettings *settings;
	GList *list;
	gint selected = 0;

	debug (FALSE, "");

	g_return_val_if_fail (settings_list != NULL, FALSE);

	list = settings_list;
	for (; list != NULL; list = list->next) {
		settings = GPA_SETTINGS (list->data);
		if (!gpa_settings_verify (settings, fail))
			return FALSE;
	}

	/* Verify that there is one and only one settings seletected */
	list = settings_list;
	for (; list != NULL; list = list->next) {
		settings = GPA_SETTINGS (list->data);
		if (settings->selected)
			selected++;
			
	}

	if (selected != 1) {
		gpa_error ("There needs to be one and only one selected settings "
			   "there are [%i] settings selected.", selected);
		return FALSE;
	}
	
	return TRUE;
}

/* FIXME: this is a problem
 * what if 2 programs are using different settings to print
 * to the same printer, For this reason, we can't determine
 * the selected settings for a printer.
 */
GpaSettings *
gpa_settings_get_selected (GList *settings_list)
{
	GpaSettings *settings = NULL;
	GList *list;

	debug (FALSE, "");

	g_return_val_if_fail (settings_list != NULL, NULL);

	list = settings_list;
	for (; list != NULL; list = list->next) {
		settings = GPA_SETTINGS (list->data);
		if (settings == NULL)
			return NULL;
		if (settings->selected)
			return settings;
	}

	gpa_error ("Could not determine the selected settings, (auto-fixed)");

	/* Fix the problem */
	if (GPA_IS_SETTINGS (settings)) {
		settings->selected = TRUE;
		return settings;
	}
		
	return NULL;
}


static GList *
gpa_selected_options_new_from_node (xmlNodePtr tree)
{
	xmlNodePtr child;
	GList *list = NULL;
	gchar *path;
	
	debug (FALSE, "");

	g_return_val_if_fail (tree != NULL, NULL);

	if (!gpa_xml_node_verify (tree, GPA_TAG_SELECTED_OPTIONS))
		return NULL;


	child = tree->childs;
	while (child != NULL) {
		skip_text (child);
		path = xmlNodeGetContent (child);
		list = g_list_prepend (list, path);
		child = child->next;
	}

	return g_list_reverse (list);
}

static GpaSettings *
gpa_settings_new_from_node (xmlNodePtr tree, GpaPrinter *printer)
{
	GpaSettings *settings;
	xmlNodePtr child;
	gchar *name;
	gchar *command;
	gchar *selected;
	
	debug (FALSE, "");
	
	g_return_val_if_fail (tree != NULL, NULL);

	if (!gpa_xml_node_verify (tree, GPA_TAG_SETTINGS))
		return NULL;

	name = gpa_xml_get_value_string_required (tree, GPA_TAG_NAME, GPA_TAG_SETTINGS);
	if (name == NULL)
		return NULL;
	command = gpa_xml_get_value_string_required (tree, GPA_TAG_COMMAND, GPA_TAG_SETTINGS);
	if (name == NULL)
		return NULL;
	
	settings = gpa_settings_new ();

	selected = gpa_xml_get_value_string (tree, GPA_TAG_SELECTED);
	if ((selected != NULL) && strcmp (selected, GPA_TAG_TRUE) == 0)
		settings->selected = TRUE;
	g_free (selected);

	settings->name    = name;
	settings->command = command;
	settings->printer = printer;

	/* Load Selected options */
	child = gpa_xml_search_child_required (tree, GPA_TAG_SELECTED_OPTIONS);
	if (child == NULL)
		return FALSE;
	settings->selected_options = gpa_selected_options_new_from_node (child);


	child = gpa_xml_search_child_required (tree, GPA_TAG_SETTINGS_INFO);
	if (child == NULL)
		return FALSE;
	settings->values = gpa_values_new_from_node (child,
						     GPA_TAG_SETTINGS_INFO);
	
	if (!gpa_settings_verify (settings, TRUE))
		return NULL;
	
	return settings;
}


/**
 * gpa_settigns_load_default_paths_from_node:
 * @tree: the node which we should load the settings from
 * 
 * Given a DefaultSettingsNode, it loads all the settings into a GList
 * of gchar containing settings paths. Like "PaperSize-Letter" or "Resolution-300"
 * 
 * Return Value: a GList of gchar pointers
 **/
gboolean
gpa_settigns_load_default_paths_from_node (xmlNodePtr tree_, GpaModel *model)
{
	xmlNodePtr tree;
	xmlNodePtr child;
	xmlNodePtr child_child;
	GList *paths;
	GList *list;
	gchar *path;
	gchar *test_path;
	gint size;
	
	debug (FALSE, "");

	g_return_val_if_fail (tree_ != NULL, FALSE);

	tree = gpa_include_node (tree_);

	if (!gpa_xml_node_verify (tree, GPA_TAG_DEFAULT_SETTINGS))
		return FALSE;

	paths = model->default_settings;
	
	child_child = gpa_xml_search_child_required (tree, GPA_TAG_PATHS);
	if (child_child == NULL)
		return FALSE;

	child = child_child->childs;
	
	for (; child != NULL; child = child->next) {
		skip_text (child);
		path = xmlNodeGetContent (child);
		
		/* Search for the path to see if we are duplicating it,
		 * if we are, remove the old path and add the new one.
		 * we need this since you can specify default settings inside
		 * the nodes or inside the <default_settings> node.
		 * we take the nodes default settings, unless there is an
		 * entry in the <DefaultSettings->Paths> */
		for (list = paths; list != NULL; list = list->next) {
			test_path = list->data;
			size = tu_get_pos_of_last_delimiter (test_path, GPA_PATH_DELIMITER_CHAR);
			g_return_val_if_fail (size > 0, FALSE);
			if (strncmp (path, test_path, size) == 0) {
				g_free (test_path);
				list->data = path;
				path = NULL;
				break;
			}
		}

		if (path == NULL)
			continue;
		
		paths = g_list_prepend (paths, path);
	}

	model->default_settings = g_list_reverse (paths);

	return TRUE;
}

GList *
gpa_settings_list_load_from_node (xmlNodePtr tree, GpaPrinter *printer)
{
	GpaSettings *settings;
	xmlNodePtr child;
	GList *list = NULL;

	debug (FALSE, "");
	
	g_return_val_if_fail (tree != NULL, NULL);
	g_return_val_if_fail (GPA_IS_PRINTER (printer), NULL);
	
	if (!gpa_xml_node_verify (tree, GPA_TAG_SETTINGS_LIST))
		return NULL;

	child = tree->childs;

	while (child != NULL) {
		skip_text (child);
		settings = gpa_settings_new_from_node (child, printer);
		if (!GPA_IS_SETTINGS (settings))
			return NULL;
		list = g_list_prepend (list, settings);
		child = child->next;
	}
	
	return g_list_reverse (list);
}


static GList *
gpa_selected_options_copy (GList *selected_options)
{
	GList *new_list = NULL;
	GList *list;
	gchar *new_path;
	gchar *path;

	list = selected_options;
	for (; list != NULL; list = list->next) {
		path = (gchar *)list->data;
		new_path = g_strdup (path);
		new_list = g_list_prepend (new_list, new_path);
	}

	return g_list_reverse (new_list);
}

GpaSettings *
gpa_settings_copy (GpaSettings *settings)
{
	GpaSettings *new_settings;

	new_settings = gpa_settings_new ();

	new_settings->name             = g_strdup (settings->name);
	new_settings->command          = g_strdup (settings->command);
	new_settings->printer          = settings->printer;
	new_settings->selected_options = gpa_selected_options_copy (settings->selected_options);
	new_settings->values           = gpa_values_copy_list (settings->values);
	new_settings->selected         = settings->selected;

	return new_settings;
}

GList *
gpa_settings_list_copy (GList *settings_list)
{
	GpaSettings *settings;
	GpaSettings *new_settings;
	GList *new_settings_list = NULL;
	GList *list;

	list = settings_list;
	for (; list != NULL; list = list->next) {
		settings = (GpaSettings *) list->data;
		
		if (!GPA_IS_SETTINGS (settings))
			return NULL;

		new_settings = gpa_settings_copy (settings);
		if (new_settings == NULL)
			return NULL;
		new_settings_list = g_list_prepend (new_settings_list,
						     new_settings);
	}

	return g_list_reverse (new_settings_list);
}

		
gchar*
gpa_settings_value_dup (GpaSettings *settings, const gchar *key)
{
	GpaValue *value;
	GList *list;
	
	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	g_return_val_if_fail (key != NULL,    NULL);

	list = settings->values;
	for (; list != NULL; list = list->next) {
		value = (GpaValue *)list->data;
		if (strcmp (value->key, key) == 0)
			return g_strdup (value->value);
	}

	g_warning ("Could not find the \"%s\" key in the \"%s\" settings values list\n",
		   key, gpa_settings_get_name (settings));
	
	return NULL;
}

gchar*
gpa_settings_value_dup_required (GpaSettings *settings, const gchar *key)
{
	gchar *value;
	
	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	g_return_val_if_fail (key != NULL,    NULL);

	value = gpa_settings_value_dup (settings, key);

	if (value == NULL)
		gpa_error ("The required value \"%s\" was not found in the "
			   "\"%s\" settings hash table", key, settings->name);
	

	debug (FALSE, "end");

	return g_strdup (value);
}

gboolean
gpa_settings_value_get_from_option_double (GpaSettings *settings,
					   GpaOption *option,
					   gdouble *value)
{
	gchar *str_value;
	gchar *key;

	debug (FALSE, "");

	*value = 0.0;

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);
	g_return_val_if_fail (option != NULL, FALSE);

	key = g_strdup_printf ("%s" GPA_PATH_DELIMITER "%s", option->parent->id, option->id);

	str_value = gpa_settings_value_dup (settings, key);

	if (str_value == NULL) 
		return FALSE;

	*value = atof (str_value);

	g_free (str_value);

	return TRUE;
}

gboolean
gpa_settings_value_get_from_option_int (GpaSettings *settings,
					GpaOption *option,
					gint *value)
{
	gchar *str_value;
	gchar *key;

	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);
	g_return_val_if_fail (option != NULL, FALSE);

	key = g_strdup_printf ("%s" GPA_PATH_DELIMITER "%s", option->parent->id, option->id);

	str_value = gpa_settings_value_dup (settings, key);

	if (str_value == NULL) 
		return FALSE;

	*value = atoi (str_value);

	g_free (str_value);

	return TRUE;
}

gboolean
gpa_settings_value_replace (GpaSettings *settings, const gchar *key, const gchar *new_value)
{
	GpaValue *value;
	GList *list;
	
	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);
	g_return_val_if_fail (key != NULL,    FALSE);


	list = settings->values;
	for (; list != NULL; list = list->next) {
		value = (GpaValue *)list->data;
		if (strcmp (value->key, key) == 0) {
			g_free (value->value);
			value->value = g_strdup (new_value);
			return TRUE;
		}
	}
	
	gpa_error ("Value not found. Settings:\"%s\" Key:\"%s\"",
		   gpa_settings_get_name (settings), key);

	return TRUE;
}

gboolean
gpa_settings_value_insert (GpaSettings *settings, const gchar *key, const gchar *value)
{
	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);
	g_return_val_if_fail (key != NULL,    FALSE);

	settings->values = gpa_value_insert (settings->values, key, value);

	return TRUE;
}



gboolean
gpa_settings_select (GpaSettings *settings_in,
		     GList *settings_list)
{
	GpaSettings *settings = NULL;
	GList *list;
	gboolean found = FALSE;

	g_return_val_if_fail (GPA_IS_SETTINGS (settings_in), FALSE);
	g_return_val_if_fail (settings_list != NULL, FALSE);

	list = settings_list;
	for (; list != NULL; list = list->next) {
		settings = list->data;
		if (!GPA_IS_SETTINGS (settings)) {
			return FALSE;
		}
		if (settings == settings_in) {
			found = TRUE;
			settings->selected = TRUE;
		} else {
			settings->selected = FALSE;
		}
	}

	if (!found) {
		gpa_error ("Could not select settings, settings not found in list");
		return FALSE;
	}

	return TRUE;
}

gboolean
gpa_settings_name_taken (GList *settings_list, const gchar *name)
{
	GpaSettings *settings;
	GList *list;

	g_return_val_if_fail (settings_list != NULL, TRUE);
	g_return_val_if_fail (name != NULL, TRUE);

	list = settings_list;
	for (; list != NULL; list = list->next) {
		settings = GPA_SETTINGS (list->data);
		if (settings == NULL)
			return FALSE;
		if (strcmp (settings->name, name) == 0)
			return TRUE;
	}

	return FALSE;
}

gchar *
gpa_settings_get_free_name (GpaSettings *settings, GList *settings_list)
{
	gint number = 2;
	gchar *name;

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	g_return_val_if_fail (settings_list != NULL, NULL);
	
	name = g_strdup_printf ("Copy of %s", settings->name);

	while (gpa_settings_name_taken (settings_list, name)) {
		g_free (name);
		name = g_strdup_printf ("Copy of %s (%i)", settings->name, number++);
	}

	return name;
}



/* Access to the struct from the world */
const GpaModel*
gpa_settings_get_model (GpaSettings *settings)
{
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	g_return_val_if_fail (GPA_IS_PRINTER (settings->printer), NULL);
	
	return settings->printer->model;
}

gchar *
gpa_settings_dup_name (GpaSettings *settings)
{
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	g_return_val_if_fail (settings->name != NULL, NULL);
	
	return g_strdup (settings->name);
}

const gchar *
gpa_settings_get_name (GpaSettings *settings)
{
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	g_return_val_if_fail (settings->name != NULL, NULL);
	
	return settings->name;
}


gchar *
gpa_settings_dup_command (GpaSettings *settings)
{
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	g_return_val_if_fail (settings->command != NULL, NULL);
	
	return g_strdup (settings->command);
}

const gchar *
gpa_settings_get_command (GpaSettings *settings)
{
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	g_return_val_if_fail (settings->command != NULL, NULL);
	
	return settings->command;
}

void
gpa_settings_name_replace (GpaSettings *settings, const gchar *name)
{
	g_return_if_fail (GPA_IS_SETTINGS (settings));
	g_return_if_fail (name != NULL);

	if (settings->name == NULL)
		g_warning ("Settings name should not be null. [gpa-settings.c]");
	else
		g_free (settings->name);

	settings->name = g_strdup (name);
	
}

gboolean
gpa_settings_is_selected (const GpaSettings *settings)
{
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);

	return settings->selected;
}


GList *
gpa_settings_get_selected_options (GpaSettings *settings)
{
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);

	return settings->selected_options;
}

void
gpa_settings_set_selected_options (GpaSettings *settings,
				   GList *selected_options)
{
	g_return_if_fail (GPA_IS_SETTINGS (settings));

	settings->selected_options = selected_options;
}






/**
 * gpa_options_unselect:
 * @options: 
 * @settings: 
 * 
 * Removes from the settings List of selected paths
 * the selected option for @options. This is used when
 * a selected option is changed, we must first remove
 * the old selected option and then add the new option
 * path to the GList
 * 
 * Return Value: TRUE on success, FALSE on error
 **/
static gboolean
gpa_settings_unselect_options (GpaSettings *settings,
			       const GpaOptions *options)
{
	GList *selected_options;
	GList *list;
	gchar *path;
	gchar *findme;
	gchar *options_path;
	gint pos;

	debug (FALSE, "");

	selected_options = gpa_settings_get_selected_options (settings);

	g_return_val_if_fail (selected_options != NULL, FALSE);
	g_return_val_if_fail (options != NULL, FALSE);

	/* get the option path with the last token stripped */
	options_path = gpa_options_dup_path (options);
	
	g_return_val_if_fail (options_path != NULL, FALSE);

	list = selected_options;
	for (; list != NULL; list = list->next) {
		path = (gchar *)list->data;
		pos = gpa_text_utils_search_backwards (path, GPA_PATH_DELIMITER_CHAR);
		if (pos == -1) {
			gpa_error ("Invalid path %s (1)\n", path);
			return FALSE;
		}
		findme = g_strndup (path, pos);

		if (strcmp (options_path, findme) == 0) {
			/* FIXME: use the list directly */
			selected_options = g_list_remove_link (selected_options, list);
			g_free (path);
			g_free (findme);
			g_free (options_path);
			gpa_settings_set_selected_options (settings, selected_options);
			return TRUE;
		}
		g_free (findme);
	}

	g_free (options_path);
	
	return FALSE;
}

/**
 * gpa_settings_selected_option:
 * @settings: 
 * @option: 
 * 
 * Select the @option in the @settings Object
 **/
gboolean
gpa_settings_select_option (GpaSettings *settings,
			    GpaOption *option)
{
	gchar *new_path;
	
 	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);
	g_return_val_if_fail (GPA_IS_OPTION (option), FALSE);

	/* Remove the previously selected option */
	if (!gpa_settings_unselect_options (settings, option->parent)) {
		gpa_error ("Could not unselect the Option from the %s Options\n",
			   gpa_options_get_name (option->parent));
	}

	new_path = gpa_option_dup_path (option);

	settings->selected_options = g_list_prepend (settings->selected_options,
						     new_path);

	return TRUE;
}
								      


void
gpa_settings_replace_command (GpaSettings *settings,  const gchar *command)
{
	g_return_if_fail (GPA_IS_SETTINGS (settings));
	g_return_if_fail (command != NULL);

	g_free (settings->command);
	settings->command = g_strdup (command);
		
}



/**
 * gpa_settings_query_options:
 * @settings: 
 * @options_id: 
 * 
 * Query a Settings object for a selected option.
 * the settings are not const because if there isn't a selected
 * option, we need to select one. This can happen
 * if ... (see : gpa_options_get_selected_option)
 * 
 * Return Value: a pointer to the selected option id
 **/
const gchar *
gpa_settings_query_options (GpaSettings *settings,
			    const gchar *options_id)
{
	const GpaModel *model;
	GpaOptions *options;
	GpaOption *option;
	
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);
	g_return_val_if_fail (options_id != NULL, NULL);

	model = settings->printer->model;
	
	options = gpa_model_get_options_from_id (model, options_id);

	if (options == NULL) {
		gpa_error ("Could not find the \"%s\" options in the \"%s\" model "
			   "options list",
			   options_id,
			   gpa_model_get_name (settings->printer->model));
	}

	option = gpa_options_get_selected_option (settings, options, FALSE);

	g_return_val_if_fail (option != NULL, NULL);

	return gpa_option_get_id (option);
}


/**
 * gpa_settings_query_options_boolean:
 * @settings: 
 * @options_id: 
 * 
 * 
 * Return Value: a boolean setting.
 **/
gboolean
gpa_settings_query_options_boolean (GpaSettings *settings,
				    const gchar *options_id)
{
	const gchar *id;

	id = gpa_settings_query_options (settings, options_id);

	if (strcmp (id, GPA_TAG_FALSE) == 0)
		return FALSE;
	else if (strcmp (id, GPA_TAG_TRUE) == 0)
		return TRUE;

	gpa_error ("Invalid boolean option (Options [%s], OptionId [%s]\n",
		   options_id, id);

	return FALSE;

}


