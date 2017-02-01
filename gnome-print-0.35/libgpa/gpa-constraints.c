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
#include <stdlib.h> /* for atoi() */
#include <libgnome/gnome-util.h>

#include "xml-utils.h"

#include "gpa-private.h"
#include "gpa-include.h"
#include "gpa-option.h"
#include "gpa-option-private.h"
#include "gpa-options-private.h"
#include "gpa-utils.h"
#include "gpa-model.h"
#include "gpa-model-private.h"
#include "gpa-printer-private.h"
#include "gpa-constraints.h"
#include "gpa-constraints-private.h"
#include "gpa-settings.h"
#include "gpa-settings-private.h"

struct _GpaConstraint
{
	GpaConstraintType type;
	gchar *constrained_path;
	gchar *offending_path;
	gint value;
	/* So if you say :
	   You can't use Option A if you don't have option B,
	   A is the constrained option and
	   B is the offending one */
};

GpaConstraint *
gpa_constraint_new (const gchar *offending_path,
		    const gchar *constrained_path,
		    GpaConstraintType type)
{
	GpaConstraint *constraint;

	debug (FALSE, "");

	constraint = g_new0 (GpaConstraint, 1);

	constraint->type = type;
	constraint->constrained_path = g_strdup (constrained_path);
	constraint->offending_path = g_strdup (offending_path);
	constraint->value = 0;

	debug (FALSE, "end");
	
	return constraint;
}


#if 0
static GpaConstraintType
gpa_constraint_get_type_from_node (xmlNodePtr tree)
{
	gchar *type_str;
	GpaConstraintType type;

	debug (FALSE, "");
	
	type_str = gpa_xml_get_value_string (tree, GPA_TAG_CONSTRAINT_TYPE);

	if ( type_str == NULL ) {
		type  = GPA_CONSTRAINT_TYPE_DOUBLE;
	}
	else if (strcmp (type_str, "Single") == 0) {
		type  = GPA_CONSTRAINT_TYPE_SINGLE;
	}
	else if (strcmp (type_str, "Double") == 0) {
		type  = GPA_CONSTRAINT_TYPE_DOUBLE;
	}
	else if (strcmp (type_str, "AtLeast") == 0) {
		type  = GPA_CONSTRAINT_TYPE_ATLEAST;
	}
	else {
		type  = GPA_CONSTRAINT_TYPE_ERROR;
		gpa_error ("Invalid constraint type \"%s\"\n", type_str);
	}

	g_free (type_str);

	return type;
}
#endif

#if 0
static gchar *
gpa_constraint_string_from_type (GpaConstraintType type)
{
	debug (FALSE, "");

	switch (type) {
	case GPA_CONSTRAINT_TYPE_SINGLE:
		return g_strdup ("Single");
	case GPA_CONSTRAINT_TYPE_DOUBLE:
		return g_strdup ("Double");
	case GPA_CONSTRAINT_TYPE_ATLEAST:
		return g_strdup ("AtLeast");
	case GPA_CONSTRAINT_TYPE_ERROR:
		gpa_error ("Constraint type = Error");
	}

	return NULL;
}
#endif

#if 0
static GpaConstraint *
gpa_constraint_new_from_node (xmlNodePtr tree_, GpaOption *option)
{
	GpaConstraint *constraint;
	GpaConstraintType type;
	xmlNodePtr tree;
	gchar *offending_path;
	gchar *constrained_path;

	debug (FALSE, "");

	tree = gpa_include_node (tree_);

	if (!gpa_xml_node_verify (tree, GPA_TAG_CONSTRAINT))
		return NULL;

	/* Get the path */
	offending_path  = gpa_xml_get_value_string_required (tree, GPA_TAG_PATH, NULL);
	if (offending_path == NULL)
		return NULL;
	
	type = gpa_constraint_get_type_from_node (tree);
	if (type == GPA_CONSTRAINT_TYPE_ERROR)
		return NULL;

	constrained_path = gpa_option_path_dup (option);

	if (constrained_path == NULL) {
		gpa_error ("Could not generate a path for option \"%s\"\n",
			   option->name);
		return NULL;
	}

	constraint = gpa_constraint_new (offending_path,
					 constrained_path,
					 type);

	/* Load "ConstraintValue" */
	if (type == GPA_CONSTRAINT_TYPE_ATLEAST) {
		gchar *value;
		value = gpa_xml_get_value_string (tree, GPA_TAG_CONSTRAINT_VALUE);
		if (value != NULL)
			constraint->value = atoi (value); 
	}

	g_free (offending_path);
	g_free (constrained_path);
	
	return constraint;
}
#endif

static GpaConstraint *
gpa_constraint_copy (GpaConstraint *constraint)
{
	GpaConstraint *new_constraint;

	debug (FALSE, "");

	new_constraint = gpa_constraint_new (constraint->offending_path,
					     constraint->constrained_path,
					     constraint->type);
	new_constraint->value = constraint->value;

	return new_constraint;
}

GList*
gpa_constraints_copy (GList *source_list)
{
	GpaConstraint *constraint;
	GpaConstraint *new_constraint;
	GList *list;
	GList *new_list = NULL;

	debug (FALSE, "");
	
	if (source_list == NULL)
		return NULL;

	for (list = source_list; list != NULL; list = list->next) {
		constraint = (GpaConstraint *) list->data;
		new_constraint = gpa_constraint_copy (constraint);
		if (new_constraint == NULL)
			return NULL;
		new_list = g_list_prepend (new_list, new_constraint);
	}
	
	return new_list;
}

#if 0
GList *
gpa_constraints_new_from_node (xmlNodePtr tree_, GpaOption *option)
{
	GpaConstraint *constraint;
	xmlNodePtr tree;
	xmlNodePtr node;
	GList *list = NULL;

	tree = gpa_include_node (tree_);

	if (!gpa_xml_node_verify (tree, GPA_TAG_CONSTRAINTS))
		return NULL;

	node = tree->childs;
	while (node != NULL) {
		skip_text (node);
		constraint = gpa_constraint_new_from_node (node, option);

		if (constraint == NULL)
			return NULL;

		list = g_list_prepend (list, constraint);
		node = node->next;
	}

	return g_list_reverse (list);
}
#endif

static GpaOption *
gpa_constraint_is_in_conflict (GpaSettings *settings,
			       GpaConstraint *constraint,
			       GpaOption *option_in)
{
	GpaOption *option;
	const gchar *value;
	gchar *key;

	debug (FALSE, "");
	
	g_return_val_if_fail (constraint != NULL, NULL);
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), NULL);

	if (!gpa_option_is_selected (settings, option_in))
			return NULL;
		
	option  = gpa_option_get_from_settings_and_path (settings, constraint->offending_path);

	/* An option can't have a constraint with itself */
	g_return_val_if_fail (option != option_in, NULL);

	if (option == NULL) {
		gpa_error ("Could not find Option\n");
		return NULL;
	}

	switch (constraint->type) {
	case GPA_CONSTRAINT_TYPE_DOUBLE:
		gpa_error ("The should not be constraints of \"Double\" type loaded");
		return option;
	case GPA_CONSTRAINT_TYPE_ERROR:
		gpa_error ("Constraint type error");
		return option;
	case GPA_CONSTRAINT_TYPE_SINGLE:
		if (gpa_option_is_selected (settings, option))
			return option;
		break;
	case GPA_CONSTRAINT_TYPE_ATLEAST:
		/* We nees to :
		   1. verify that the offending path option type is numeric
		   2. get the option values->value (hash)
		   3. see if option value->value is at least constraint->value.
		*/
		if (option->parent->options_type != GPA_OPTIONS_TYPE_NUMERIC) {
			gpa_error ("\"At least\" type constraint with an ofending type "
				   "diferent from numeric");
			return option;
		}

		key = g_strdup_printf ("%s" GPA_PATH_DELIMITER "%s", option->parent->id, option->id);
		value = gpa_settings_value_dup_required (settings, key);
		g_free (key);
		if (value == NULL)
			return option;

		if (atoi (value) < constraint->value) {
			g_warning ("At least constriant not staisfied");
		/*if (option->selected)*/
			return option;
		}
		
		break;
	}
	
	return NULL;
}

GpaOption *
gpa_constraints_is_in_conflict (GpaSettings *settings,
				GpaOption *option)
{
	GpaConstraint *constraint;
	GpaOption *conflicted_option;
	GList *list;
	
	g_return_val_if_fail (option != NULL, NULL);

	/* If this option does not have any constraints, return */
	if (option->constraints == NULL)
		return NULL;

	/* Scan the list of constraints and if any one of
	   them has a conflict, return TRUE */
	for (list = option->constraints; list != NULL; list = list->next) {
		constraint = (GpaConstraint *) list->data;
		conflicted_option = gpa_constraint_is_in_conflict (settings, constraint, option);
		if (conflicted_option != NULL)
			return conflicted_option;
	}

	return NULL;
}

#if 0
xmlNodePtr
gpa_constraints_write (XmlParseContext *context, GpaOption *option)
{
	GpaConstraint *constraint;
	xmlNodePtr node;
	xmlNodePtr item;
	GList *list;
	gchar *type;

	debug (FALSE, "");

	if (option->constraints == NULL)
		return NULL;

	node = xmlNewDocNode (context->doc, context->ns, GPA_TAG_CONSTRAINTS, NULL);

	list = option->constraints;
	for (; list != NULL; list = list->next) {
		constraint = (GpaConstraint *) list->data;
		item = xmlNewChild (node, NULL, GPA_TAG_CONSTRAINT, NULL);
		type = gpa_constraint_string_from_type (constraint->type);
		gpa_xml_set_value (item, GPA_TAG_PATH, constraint->offending_path);
		gpa_xml_set_value (item, GPA_TAG_CONSTRAINT_TYPE, type);
		if (constraint->value > 0) {
			gchar *value_str;
			value_str = g_strdup_printf ("%i", constraint->value);
			gpa_xml_set_value (item, GPA_TAG_CONSTRAINT_VALUE, value_str);
			g_free (value_str);
		}
			
		g_free (type);
	}
		
	return node;
}
#endif

static gboolean
gpa_constraint_verify (GpaConstraint *constraint,
		       GpaOption *option_in,
		       const GpaModel *model)
{
	GpaOption *option;
	GpaOption *option_verify;
		
	debug (FALSE, "");

	g_return_val_if_fail (constraint != NULL, FALSE);
	g_return_val_if_fail (option_in  != NULL, FALSE);
	g_return_val_if_fail (constraint->constrained_path != NULL, FALSE);
	g_return_val_if_fail (constraint->offending_path   != NULL, FALSE);
	
	if (constraint->offending_path == NULL) {
		gpa_error ("The Constraint contained a NULL path");
		return FALSE;
	}

	option = gpa_option_get_from_model_and_path (model,
						     constraint->offending_path);
	if (option == NULL) {
		gpa_error ("The path \"%s\" could not be located",
			   constraint->offending_path);
		return FALSE;
	}

	option_verify = gpa_option_get_from_model_and_path (model,
							    constraint->constrained_path);
	if (option_verify != option_in) {
		gpa_error ("The constrained path \"%s\" does not match the option it "
			   "is attached to.",
			   constraint->constrained_path);
		return FALSE;
	}
	
	if ((constraint->type != GPA_CONSTRAINT_TYPE_SINGLE) &&
	    (constraint->type != GPA_CONSTRAINT_TYPE_DOUBLE) &&
	    (constraint->type != GPA_CONSTRAINT_TYPE_ATLEAST) &&
	    (constraint->type != GPA_CONSTRAINT_TYPE_ERROR)) {
		gpa_error ("The Constriaint path \"%s\" contained an invalid contraint"
			   "type", constraint->offending_path);
		return FALSE;
	}

	return TRUE;
}

static gboolean
gpa_constraint_verify_with_settings (GpaConstraint *constraint,
				     GpaOption *option_in,
				     GpaSettings *settings)
{
	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);
	g_return_val_if_fail (constraint != NULL, FALSE);
	g_return_val_if_fail (option_in  != NULL, FALSE);
	g_return_val_if_fail (constraint->constrained_path != NULL, FALSE);
	g_return_val_if_fail (constraint->offending_path   != NULL, FALSE);

	if (!gpa_constraint_verify (constraint, option_in, settings->printer->model))
		return FALSE;
	
	if (gpa_constraint_is_in_conflict (settings, constraint, option_in)) {
		gpa_error ("The constraint path \"%s\" is in conflict",
			   constraint->offending_path);
		return FALSE;
	}

	return TRUE;
}

gboolean
gpa_constraints_verify (GpaOption *option,
			const GpaModel *model)
{
	GpaConstraint *constraint;
	GList *list;

	debug (FALSE, "");
	
	g_return_val_if_fail (option != NULL, FALSE);

	list = option->constraints;
	for (; list != NULL; list = list->next) {
		constraint = (GpaConstraint *) list->data;
		if (!gpa_constraint_verify (constraint, option, model))
			return FALSE;
	}
		
	return TRUE;
}

gboolean
gpa_constraints_verify_with_settings (GpaOption *option,
				      GpaSettings *settings)
{
	GpaConstraint *constraint;
	GList *list;

	debug (FALSE, "");
	
	g_return_val_if_fail (GPA_IS_SETTINGS (settings), FALSE);
	g_return_val_if_fail (option != NULL, FALSE);

	list = option->constraints;
	for (; list != NULL; list = list->next) {
		constraint = (GpaConstraint *) list->data;
		if (!gpa_constraint_verify_with_settings (constraint, option, settings))
			return FALSE;
	}
		
	return TRUE;
}


static gboolean
gpa_constraints_option_add_doubles_to_list (GpaOption *option,
					     GList **doubles_list)
{
	GpaConstraint *constraint;
	GList *list;

	list = option->constraints;
	for (; list != NULL; list = list->next) {
		constraint = (GpaConstraint *) list->data;
		if (constraint->type == GPA_CONSTRAINT_TYPE_DOUBLE)
			*doubles_list = g_list_prepend (*doubles_list, constraint);
	}

	return TRUE;
}

static gboolean
gpa_constraints_options_add_doubles_to_list (GpaOptions *options,
					     GList **doubles_list)
{
	GpaOption *option;
	GList *list;

	list = options->children;
	for (; list != NULL; list = list->next) {
		option = (GpaOption *) list->data;
		if (!gpa_constraints_option_add_doubles_to_list (option, doubles_list))
			return FALSE;
	}

	return TRUE;
}

static GpaConstraint *
gpa_constraint_search_in_list (GList *list, gchar *offending_path, gchar *constrained_path)
{
	GpaConstraint *constraint;
	
	for (; list != NULL; list = list->next) {
		constraint = (GpaConstraint *)list->data;
		if ( (strcmp (offending_path,   constraint->offending_path)   == 0) &&
		     (strcmp (constrained_path, constraint->constrained_path) == 0)) {
			return constraint;
		}
	}

	return NULL;
}

static gboolean
gpa_constraint_load_brothers (GList **doubles_list,
			      GpaModel *model)
{
	GList *list;
	GpaOption *option;
	GpaConstraint *constraint;
	GpaConstraint *new_constraint;

	g_return_val_if_fail (GPA_IS_MODEL (model), FALSE);
	
	list = *doubles_list;
	for (; list != NULL; list = list->next) {
		constraint = (GpaConstraint *)list->data;
		new_constraint = gpa_constraint_new (constraint->constrained_path,
						     constraint->offending_path,
						     GPA_CONSTRAINT_TYPE_SINGLE);
		constraint->type = GPA_CONSTRAINT_TYPE_SINGLE;
		option = gpa_option_get_from_model_and_path (model,
							     new_constraint->constrained_path);
		if (option == NULL)
			return FALSE;

		option->constraints = g_list_prepend (option->constraints,
						       new_constraint);
		
	}

	return TRUE;
}

static gboolean
gpa_constraint_free (GpaConstraint *constraint)
{
	debug (FALSE, "");
	
	g_return_val_if_fail (constraint != NULL, FALSE);
	g_return_val_if_fail (constraint->constrained_path != NULL, FALSE);
	g_return_val_if_fail (constraint->offending_path != NULL, FALSE);

	g_free (constraint->constrained_path);
	g_free (constraint->offending_path);
	g_free (constraint);

	return TRUE;
}


static gboolean
gpa_constraint_double_clean_list (GList **doubles_list)
{
	GpaConstraint *constraint;
	GpaConstraint *constraint_found;
	GList *list;
	GList *prev;
	GList *removeme;

	debug (FALSE, "");

	list = *doubles_list;
	for (; list != NULL; list = list->next) {
		gchar *offending_path;
		gchar *constrained_path;
		
		constraint = (GpaConstraint *)list->data;
		offending_path   = constraint->constrained_path;
		constrained_path = constraint->offending_path;
		constraint_found = gpa_constraint_search_in_list (*doubles_list,
								  offending_path,
								  constrained_path);
		if (constraint_found == NULL)
			continue;

		prev = list->prev;
		/* remove items */
		*doubles_list = g_list_remove_link (*doubles_list, list);
		removeme = g_list_find (*doubles_list, constraint_found);
		if (removeme == NULL) {
			gpa_error ("Could not remove the sencond constraint");
			return FALSE;
		}
		*doubles_list = g_list_remove_link (*doubles_list, removeme);

		constraint->type       = GPA_CONSTRAINT_TYPE_SINGLE;
		constraint_found->type = GPA_CONSTRAINT_TYPE_SINGLE;

		/* load "list" with the correct pointer, since list is not
		 part of the list anymore */
		if (prev == NULL) {
			list = *doubles_list;
		} else {
			list = prev->next;
			if (prev->next == NULL)
				break;
		}
	}
		
	debug (FALSE, "false");
	
	return TRUE;
}

/**
 * gpa_constraints_link_doubles:
 * @printer: 
 * 
 * This function should be called after the printer is loaded so that the
 * Double constraints can be linked to it's counterpart
 * The way double constraints work is that if you say
 * you need A to use B
 * you implicitly say that you also need B to use A
 *
 * Return Value: FALSE on error, TRUE otherwise
 **/
gboolean 
gpa_constraints_double_links_model (GpaModel *model)
{
	GpaOptions *options;
	GList *list;
	GList *doubles_list = NULL;

	debug (FALSE, "");

	g_return_val_if_fail (GPA_IS_MODEL (model), FALSE);

	/* We need to
	  1. scan the list of options
	  2. scan the list of option
	  3. scan the list of constraints
	  4. Load all double constraints into a GList
	  
	  5. Delete from the list, those constraints that
	     have been double-double linked
	  (i.e) you need A to use B (double)
	        you need B to use A (double)
	  6. Scan the GList created to create the second part
	     of the constraints
	*/

	list = model->options_list;
	for (; list != NULL; list = list->next) {
		options = (GpaOptions*) list->data;
		gpa_constraints_options_add_doubles_to_list (options, &doubles_list);
	}
	if (!gpa_constraint_double_clean_list (&doubles_list))
		return FALSE;
	if (!gpa_constraint_load_brothers (&doubles_list, model))
		return FALSE;

	debug (FALSE, "end");
	
	return TRUE;
}
      

gboolean 
gpa_constraints_free (GList *constraints_list)
{
	GpaConstraint *constraint;
	GList *list;

	debug (FALSE, "");
	
	list = constraints_list;
	for (; list != NULL; list = list->next) {
		constraint = (GpaConstraint *)list->data;
		if (!gpa_constraint_free (constraint))
			return FALSE;
	}

	g_list_free (list);

	return TRUE;
}


gboolean
gpa_constraints_load_from_node (xmlNodePtr node_, GpaModel *model)
{
	debug (FALSE, "");
	
	g_return_val_if_fail (node_ != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_MODEL (model), FALSE);

#if 0	
	g_print ("Load constraints from node and attach them to the options...\n");
#endif	
	

	return TRUE;
}
