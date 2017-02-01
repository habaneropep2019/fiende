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
#include "gpa-vendor.h"
#include "gpa-vendor-private.h"
#include "gpa-printer.h"
#include "gpa-model-info.h"
#include "gpa-model-info-private.h"

#include "xml-utils.h"

static GList *
gpa_vendor_list_set_get (GList **vendors)
{
	static GList *list = NULL;

	debug (FALSE, "");
	
	if (vendors == NULL)
		return list;

	list = *vendors;

	return NULL;
}

GList *
gpa_vendor_list_get (void)
{
	return gpa_vendor_list_set_get (NULL);
}

static void
gpa_vendor_list_set (GList **vendor_list)
{
	debug (FALSE, "");

	g_return_if_fail (vendor_list != NULL);
	
	gpa_vendor_list_set_get (vendor_list);
}

GpaVendor *
gpa_vendor_new (const gchar * name)
{
	GpaVendor *vendor;

	debug (FALSE, "");

	vendor = g_new0 (GpaVendor, 1);

	vendor->name = g_strdup (name);
	vendor->models = NULL;

	return vendor;
}

static GpaVendor *
gpa_vendor_new_from_tree (XmlParseContext *context, xmlNodePtr tree)
{
	GpaVendor *vendor = NULL;
	gchar *name;
	xmlNodePtr child;
	
	debug (FALSE, "");

	if (strcmp (tree->name, GPA_TAG_VENDOR)) {
		gpa_error ("Invalid element type. Expected "
				  GPA_TAG_VENDOR ", ecountered %s",
				  tree->name);
		return NULL;
	}

	child = tree->childs;
	name = gpa_xml_get_value_string_required (tree, GPA_TAG_NAME, NULL);
	if (name == NULL)
		return NULL;

	vendor = gpa_vendor_new (name);

	child = gpa_xml_search_child (tree, GPA_TAG_MODELS);
	if (child)
		gpa_load_model_info_list_from_tree (context, child, &vendor->models, vendor);

	g_free (name);

	return vendor;
}


static gboolean
gpa_vendor_list_load_from_tree (XmlParseContext *context,
				xmlNodePtr tree)
{
	GpaVendor *vendor;
	xmlNodePtr child;
	xmlNodePtr node;
	GList *list = NULL;
	
	debug (FALSE, "");

	if (strcmp (tree->name, GPA_TAG_INDEX)) {
		gpa_error ("Invalid type in load_from_node."
				  "Expected " GPA_TAG_INDEX );
		return FALSE;
	}

	child = gpa_xml_search_child (tree, GPA_TAG_VENDORS);
	if (child == NULL) {
		gpa_error ("The file did not contained any Vendors");
		return FALSE;
	}

	node = child->childs;
	while (node != NULL) {
		vendor = gpa_vendor_new_from_tree (context, node);
		list  = g_list_prepend (list, vendor);
		node = node->next;
	}

	/* We set the vendors GList by calling get vendors */
	gpa_vendor_list_set (&list);

	return TRUE;
}	

static gboolean
gpa_vendor_list_new_from_file (const gchar *xml_file)
{
	xmlDocPtr doc;
	xmlNsPtr  name_space;
	XmlParseContext *context;

	debug (FALSE, "");

	g_return_val_if_fail (xml_file != NULL, FALSE);

	/* Laad the xml file, and check for success*/
	doc = xmlParseFile (xml_file);
	if (doc == NULL) {
		gpa_error ("File not found [%s] (gpa-vendors)", xml_file);
		return FALSE;
	}
	if (doc->root == NULL) {
		gpa_error ("Invalid xml File, tree empty [%s]", xml_file);
		xmlFreeDoc (doc);
		return FALSE;
	}

	/* Is the xml file the correct type ? */
	name_space = xmlSearchNsByHref (doc, doc->root, GPA_TAG_INDEX_NAME_SPACE);
	if (name_space == NULL) {
		gpa_error ("This is not a Printer Index file, did not contained a name_space [%s]", xml_file);

		xmlFreeDoc (doc);
		return FALSE;
	}
	/* Is this a printer index file ? */
	if ((doc->root->name == NULL) || (strcmp (doc->root->name, GPA_TAG_INDEX)!=0)) {
		gpa_error ("This is not a PrinterIndex file, bad root name,\n"
			   "Expected \"%s\", root name \"%s\"[%s]",
			   GPA_TAG_INDEX,
			   doc->root->name,
			   xml_file);
		xmlFreeDoc (doc);
		return FALSE;
	}
	
	context = gpa_xml_parse_context_new (doc, name_space);

	gpa_vendor_list_load_from_tree (context, doc->root);

	gpa_xml_parse_context_destroy (context);

	xmlFreeDoc (doc);

	return TRUE;
}


/**
 * gpa_vendor_list_load_all:
 * @void: 
 * 
 * Load all the vendors from disk
 * 
 * Return Value: 
 **/
gboolean 
gpa_vendor_list_load_all (void)
{
	if (!gpa_vendor_list_new_from_file (GPA_DATA_DIR GPA_TAG_INDEX ".xml"))
		return FALSE;

	return TRUE;
}


/**
 * gpa_vendor_add:
 * @vendor_name: 
 * 
 * Adds a vendor and appends it to the vendor list
 * 
 * Return Value: a pointer to the GpaVendor that was added, NULL on error
 **/
static GpaVendor *
gpa_vendor_add (const gchar *vendor_name)
{
	GpaVendor *vendor;
	GList *list;

	g_return_val_if_fail (vendor_name != NULL, FALSE);

	list = gpa_vendor_list_get ();

	vendor = gpa_vendor_new (vendor_name);

	list = g_list_prepend (list, vendor);

	gpa_vendor_list_set (&list);

	return vendor;
}

/**
 * gpa_vendor_get_from_name:
 * @vendor_name: 
 * @be_quiet: 
 * 
 * Returns a potiner to the GpaVendor name which matches @vendor_name
 * 
 * Return Value: a potiner to the GpaVendor
 **/
GpaVendor *
gpa_vendor_get_from_name (const gchar *vendor_name)
{
	GpaVendor *vendor;
	GList *list;

	debug (FALSE, "");

	g_return_val_if_fail (vendor_name != NULL, NULL);
	
	list = gpa_vendor_list_get ();
	for (; list != NULL; list = list->next) {
		vendor = (GpaVendor *) list->data;
		if (vendor->name == NULL) {
			gpa_error ("Vendor name is null while trying to get from name");
			return NULL;
		}
		if (strcmp (vendor->name, vendor_name) == 0)
			return vendor;
	}

	vendor = gpa_vendor_add (vendor_name);
	
	return vendor;
}


gboolean
gpa_vendor_verify (GpaVendor *vendor)
{
	debug (FALSE, "");

	g_return_val_if_fail (vendor != NULL, FALSE);
	
	if (vendor->name == NULL) {
		gpa_error ("The vendor does not contain an name");
		return FALSE;
	}

	return TRUE;
}
		


/* Access to the struct */
const gchar *
gpa_vendor_get_name (const GpaVendor *vendor)
{
	g_return_val_if_fail (GPA_IS_VENDOR (vendor), NULL);

	return vendor->name;
}
   
gchar *
gpa_vendor_dup_name (const GpaVendor *vendor)
{
	g_return_val_if_fail (GPA_IS_VENDOR (vendor), NULL);

	return g_strdup (vendor->name);
}

GList *
gpa_vendor_get_models_list (const GpaVendor *vendor)
{
	g_return_val_if_fail (GPA_IS_VENDOR (vendor), NULL);

	return vendor->models;
}
   
