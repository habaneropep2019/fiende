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

#include "gpa-private.h"
#include "xml-utils.h"
#include "gpa-values.h"
#include "gpa-include.h"
#include "gpa-known.h"
#include "gpa-settings-private.h" /* GpaValue is defined there ..*/

static void
gpa_value_verify_printer (gpointer key_in, gpointer value_in, gpointer data)
{
	gboolean *success;
	gchar *key;
	gchar *value;
#if 0	
	gint number;
	gint n;
#endif	

	key   = (gchar *) key_in;
	value = (gchar *) value_in;
	success = data;

#warning FIXME	
#if 0	
	number = sizeof (gpa_known_printer_values) / sizeof (GpaKnownPrinterValues);
	for (n = 0; n < number; n++)
		if (strcmp (key, gpa_known_printer_values[n].name) == 0)
			break;

	if (n == number) {
		gpa_error ("Unknown Printer Value %s", key);
		*success = FALSE;
	}
#else
	*success = TRUE;
#endif	

}

gboolean
gpa_values_verify_printer (GHashTable *hash, gboolean from_profile)
{
	gboolean success = TRUE;
#if 0	
	gint n;
	gint number;
#endif	

	g_warning ("IMplement me !!!|");
	return TRUE;

	/* Check if the options are from known types */
	g_hash_table_foreach (hash, gpa_value_verify_printer, &success);

	if (!success)
		return FALSE;

#if 0	
	number = sizeof (gpa_known_printer_values) / sizeof (GpaKnownPrinterValues);
	for (n = 0; n < number; n++) {
		if (gpa_known_printer_values[n].in_profile || !from_profile) {
			if (g_hash_table_lookup (hash, gpa_known_printer_values[n].name)
			    ==NULL) {
				gpa_error ("Could not find the required value \"%s\" in the "
					   "PrinterInfo section\n",
					   gpa_known_printer_values[n].name);
				return FALSE;
			}
		}
	}

#endif	
	
	return success;
}

/**
 * gpa_values_verify_settings:
 * @settings: 
 * 
 * Verifies that the required values exits in the settings Object
 * 
 * Return Value: TRUE on success, FALSE on error.
 **/
gboolean
gpa_values_verify_settings (GpaSettings *settings)
{
	GpaValue *value;
	GList *list;
	gint number;
	gint n;

	number = sizeof (gpa_required_settings_values) / sizeof (GpaRequiredSettingsValues);
	for (n = 0; n < number; n++) {
		const gchar *key;
		key = gpa_required_settings_values [n].key;
		list = settings->values;
		for (; list != NULL; list = list->next) {
			value = (GpaValue *)list->data;
			if (strcmp (value->key, key) == 0)
				break;
		}
		if (list == NULL) {
			gpa_error ("The required key \"%s\" was not found "
				   "on the values for the \"%s\" settings",
				   key, gpa_settings_get_name (settings));
			return FALSE;
		}
	}

	return TRUE;
}

gboolean
gpa_values_verify_option (GpaOption *option)
{
	/* Nothing to verify, at least nothing I can think of */
	g_warning ("IMplement me !!!|");

	return TRUE;
}


#if 0
const gchar*
gpa_value_get (GList *list_in, const gchar *key)
{
	GpaValue *value;
	GList *list

	debug (FALSE, "");

	g_return_val_if_fail (values != NULL, NULL);
	g_return_val_if_fail (key != NULL, NULL);

	list = list_in;
	for (; list != NULL; list = list->next) {
		value = (GpaValue *)list->data;
		if (strcmp (value->key, key) == 0)
			return value->value;
	}
	
	gpa_error ("Could not find the \"%s\" key in list ", key);

	return NULL;
}
#endif

GpaValue *
gpa_value_new (const gchar *key, const gchar *val)
{
	GpaValue *value;

	value = g_new0 (GpaValue, 1);
	value->key   = g_strdup (key);
	value->value = g_strdup (val);

	return value;
}

GList *
gpa_value_insert (GList *list, const gchar *key, const gchar *val)
{
	GpaValue *value;

	g_return_val_if_fail (key  != NULL, list);
	g_return_val_if_fail (val != NULL, list);
	
	value = gpa_value_new (key, val);
	list = g_list_prepend (list, value);
	
	return list;
}

	
static GpaValue *
gpa_value_new_from_node (xmlNodePtr tree, const gchar *node_name)
{
	GpaValue *value;

	value = gpa_value_new (tree->name, NULL);
	value->value = xmlNodeGetContent(tree);

	if ((value->key == NULL) || (value->value == NULL)) {
		g_warning ("Could not load value for node \"%s\"[%s,%s]\n",
			   node_name, value->key, value->value);
		return NULL;
	}

	return value;
}

GList *
gpa_values_new_from_node (xmlNodePtr tree_, const gchar *node_name)
{
	xmlNodePtr node;
	xmlNodePtr tree;
	GpaValue *value;
	GList *list = NULL;

	g_return_val_if_fail (tree_ != NULL, NULL);
	g_return_val_if_fail (node_name != NULL, NULL);

	tree = gpa_include_node (tree_);
	
	if (!gpa_xml_node_verify (tree, node_name))
		return NULL;

	/* This is not an error, since the hash can be empty */
	if (tree->childs == NULL)
		return list;

	node = tree->childs;
	while (node != NULL) {
		skip_text (node);
		value = gpa_value_new_from_node (node, node_name);
		if (value == NULL)
			return NULL;
		list = g_list_prepend (list, value);
		node = node->next;
	}

	return g_list_reverse (list);
}


static GpaValue *
gpa_value_copy (const GpaValue *value)
{
	GpaValue *new_value;

	new_value = gpa_value_new (value->key, value->value);
	
	return new_value;
}

GList *
gpa_values_copy_list (GList *list_in)
{
	GpaValue *new_value;
	GpaValue *value;
	GList *new_list = NULL;
	GList *list;

	list = list_in;
	for (; list != NULL; list = list->next) {
		value = (GpaValue *)list->data;
		new_value = gpa_value_copy (value);
		new_list = g_list_prepend (new_list, new_value);
	}

	return g_list_reverse (new_list);
}
		
gboolean
gpa_values_free_list (GList *list_in)
{
	GList *list;

	list = list_in;
	for (; list != NULL; list = list->next) {
		GpaValue *value;
		value = (GpaValue *)list->data;
		g_free (value->key);
		g_free (value->value);
	}
	g_list_free (list_in);

	return TRUE;
}

xmlNodePtr
gpa_values_write_list (XmlParseContext *context,
		       GList *list_in,
		       const gchar *node_name)
{
	xmlNodePtr node;
	GpaValue *value;
	GList *list;

	g_return_val_if_fail (context   != NULL, NULL);
	g_return_val_if_fail (list_in   != NULL, NULL);
	g_return_val_if_fail (node_name != NULL, NULL);
		
	node = xmlNewDocNode (context->doc, context->ns, node_name, NULL);

	list = list_in;
	for (; list != NULL; list = list->next) {
		value = (GpaValue *)list->data;
		xmlNewChild (node, NULL, value->key, value->value);
	}

	return node;
}
