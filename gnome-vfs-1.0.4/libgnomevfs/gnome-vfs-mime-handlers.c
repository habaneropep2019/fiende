/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-mime-handlers.c - Mime type handlers for the GNOME Virtual
   File System.

   Copyright (C) 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Maciej Stachowiak <mjs@eazel.com> */

#include <config.h>
#include "gnome-vfs-mime-handlers.h"

#include "gnome-vfs-application-registry.h"
#include "gnome-vfs-mime-info.h"
#include "gnome-vfs-mime.h"
#include "gnome-vfs-result.h"
#include "gnome-vfs-utils.h"
#ifdef HAVE_GCONF
#include <gconf/gconf.h>
#endif
#include <stdio.h>
#include <string.h>

#ifdef HAVE_GCONF
static GConfEngine *gconf_engine = NULL;
#endif

static char *         get_user_level                          (void);
static GList *        OAF_ServerInfoList_to_ServerInfo_g_list (OAF_ServerInfoList *info_list);
static GList *        copy_str_list                           (GList *string_list);
static GList *        comma_separated_str_to_str_list         (const char         *str);
static GList *        str_list_difference                     (GList              *a,
							       GList              *b);
static char *         str_list_to_comma_separated_str         (GList              *list);
static void           g_list_free_deep                        (GList              *list);
static GList *        prune_ids_for_nonexistent_applications  (GList              *list);
static GnomeVFSResult gnome_vfs_mime_edit_user_file           (const char         *mime_type,
							       const char         *key,
							       const char         *value);
static gboolean       application_known_to_be_nonexistent     (const char         *application_id);

/**
 * gnome_vfs_mime_get_description:
 * @mime_type: the mime type
 *
 * Query the MIME database for a description of the specified MIME type.
 *
 * Return value: A description of MIME type @mime_type
 */
const char *
gnome_vfs_mime_get_description (const char *mime_type)
{
	return gnome_vfs_mime_get_value (mime_type, "description");
}

/**
 * gnome_vfs_mime_set_description:
 * @mime_type: A const char * containing a mime type
 * @description: A description of this MIME type
 * 
 * Set the description of this MIME type in the MIME database. The description
 * should be something like "Gnumeric spreadsheet".
 * 
 * Return value: GnomeVFSResult indicating the success of the operation or any
 * errors that may have occurred.
 **/
GnomeVFSResult
gnome_vfs_mime_set_description (const char *mime_type, const char *description)
{
	return gnome_vfs_mime_edit_user_file
		(mime_type, "description", description);
}

/**
 * gnome_vfs_mime_get_default_action_type:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * 
 * Query the MIME database for the type of action to be performed on a particular MIME type by default.
 * 
 * Return value: The type of action to be performed on a file of 
 * MIME type, @mime_type by default.
 **/
GnomeVFSMimeActionType
gnome_vfs_mime_get_default_action_type (const char *mime_type)
{
	const char *action_type_string;

	action_type_string = gnome_vfs_mime_get_value (mime_type, "default_action_type");

	if (action_type_string != NULL && strcasecmp (action_type_string, "application") == 0) {
		return GNOME_VFS_MIME_ACTION_TYPE_APPLICATION;
	} else if (action_type_string != NULL && strcasecmp (action_type_string, "component") == 0) {
		return GNOME_VFS_MIME_ACTION_TYPE_COMPONENT;
	} else {
		return GNOME_VFS_MIME_ACTION_TYPE_NONE;
	}

}

/**
 * gnome_vfs_mime_get_default_action:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * 
 * Query the MIME database for default action associated with a particular MIME type @mime_type.
 * 
 * Return value: A GnomeVFSMimeAction representing the default action to perform upon
 * file of type @mime_type.
 **/
GnomeVFSMimeAction *
gnome_vfs_mime_get_default_action (const char *mime_type)
{
	GnomeVFSMimeAction *action;

	action = g_new0 (GnomeVFSMimeAction, 1);

	action->action_type = gnome_vfs_mime_get_default_action_type (mime_type);

	switch (action->action_type) {
	case GNOME_VFS_MIME_ACTION_TYPE_APPLICATION:
		action->action.application = 
			gnome_vfs_mime_get_default_application (mime_type);
		if (action->action.application == NULL) {
			g_free (action);
			action = NULL;
		}
		break;
	case GNOME_VFS_MIME_ACTION_TYPE_COMPONENT:
		action->action.component = 
			gnome_vfs_mime_get_default_component (mime_type);
		if (action->action.component == NULL) {
			g_free (action);
			action = NULL;
		}
		break;
	case GNOME_VFS_MIME_ACTION_TYPE_NONE:
		g_free (action);
		action = NULL;
		break;
	default:
		g_assert_not_reached ();
	}

	return action;
}

/**
 * gnome_vfs_mime_get_default_application:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Query the MIME database for the application to be executed on files of MIME type
 * @mime_type by default.
 * 
 * Return value: A GnomeVFSMimeApplication representing the default handler of @mime_type
 **/
GnomeVFSMimeApplication *
gnome_vfs_mime_get_default_application (const char *mime_type)
{
	const char *default_application_id;
	GnomeVFSMimeApplication *default_application;
	GList *short_list;

	default_application = NULL;

	/* First, try the default for the mime type */
	default_application_id = gnome_vfs_mime_get_value
		(mime_type, "default_application_id");

	if (default_application_id != NULL
	    && default_application_id[0] != '\0'
	    && !application_known_to_be_nonexistent (default_application_id)) {
		default_application =
			gnome_vfs_application_registry_get_mime_application (default_application_id);
	}

	if (default_application == NULL) {
		/* Failing that, try something from the short list */

		short_list = gnome_vfs_mime_get_short_list_applications (mime_type);

		if (short_list != NULL) {
			default_application = gnome_vfs_mime_application_copy
				((GnomeVFSMimeApplication *) (short_list->data));
			gnome_vfs_mime_application_list_free (short_list);
		}
	}


	return default_application;
}

/**
 * gnome_vfs_mime_get_icon:
 * @mime_type: A const char * containing a  MIME type
 *
 * Query the MIME database for an icon representing the specified MIME type.
 *
 * Return value: The filename of the icon as listed in the MIME database. This is
 * usually a filename without path information, e.g. "i-chardev.png", and sometimes
 * does not have an extension, e.g. "i-regular" if the icon is supposed to be image
 * type agnostic between icon themes. Icons are generic, and not theme specific. These
 * will not necessarily match with the icons a user sees in Nautilus, you have been warned.
 */
const char *
gnome_vfs_mime_get_icon (const char *mime_type)
{
	return gnome_vfs_mime_get_value (mime_type, "icon_filename");
}

/**
 * gnome_vfs_mime_set_icon:
 * @mime_type: A const char * containing a  MIME type
 * @filename: a const char * containing an image filename
 *
 * Set the icon entry for a particular MIME type in the MIME database. Note that
 * icon entries need not necessarily contain the full path, and do not necessarily need to
 * specify an extension. So "i-regular", "my-special-icon.png", and "some-icon"
 * are all valid icon filenames.
 *
 * Return value: A GnomeVFSResult indicating the success of the operation
 * or any errors that may have occurred.
 */
GnomeVFSResult
gnome_vfs_mime_set_icon (const char *mime_type, const char *filename)
{
	return gnome_vfs_mime_edit_user_file
		(mime_type, "icon_filename", filename);
}


/**
 * gnome_vfs_mime_can_be_executable:
 * @mime_type: A const char * containing a mime type
 * 
 * Check whether files of this MIME type might conceivably be executable.
 * Default for known types if FALSE. Default for unknown types is TRUE.
 * 
 * Return value: gboolean containing TRUE if some files of this MIME type
 * are registered as being executable, and false otherwise.
 **/
gboolean
gnome_vfs_mime_can_be_executable (const char *mime_type)
{
	const char *result_as_string;
	gboolean result;
	
	result_as_string = gnome_vfs_mime_get_value (mime_type, "can_be_executable");
	if (result_as_string != NULL) {
		result = strcmp (result_as_string, "TRUE") == 0;
	} else {
		/* If type is not known, we treat it as potentially executable.
		 * If type is known, we use default value of not executable.
		 */
		result = !gnome_vfs_mime_type_is_known (mime_type);
	}

	return result;
}

/**
 * gnome_vfs_mime_set_can_be_executable:
 * @mime_type: A const char * containing a mime type
 * @new_value: A boolean value indicating whether @mime_type could be executable.
 * 
 * Set whether files of this MIME type might conceivably be executable.
 * 
 * Return value: GnomeVFSResult indicating the success of the operation or any
 * errors that may have occurred.
 **/
GnomeVFSResult
gnome_vfs_mime_set_can_be_executable (const char *mime_type, gboolean new_value)
{
	return gnome_vfs_mime_edit_user_file
		(mime_type, "can_be_executable", new_value ? "TRUE" : "FALSE");
}

/**
 * gnome_vfs_mime_get_default_component:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Query the MIME database for the default Bonobo component to be activated to 
 * view files of MIME type @mime_type.
 * 
 * Return value: An OAF_ServerInfo * representing the OAF server to be activated
 * to get a reference to the proper component.
 **/
OAF_ServerInfo *
gnome_vfs_mime_get_default_component (const char *mime_type)
{
	const char *default_component_iid;
	OAF_ServerInfoList *info_list;
	OAF_ServerInfo *default_component;
	CORBA_Environment ev;
	char *supertype;
	char *query;
	char *sort[6];
        GList *short_list;
	GList *p;
	char *prev;

	if (mime_type == NULL) {
		return NULL;
	}

	CORBA_exception_init (&ev);

	supertype = gnome_vfs_get_supertype_from_mime_type (mime_type);

	/* Find a component that supports either the exact mime type,
           the supertype, or all mime types. */

	/* First try the component specified in the mime database, if available. 
	   gnome_vfs_mime_get_value looks up the value for the mime type and the supertype.  */
	default_component_iid = gnome_vfs_mime_get_value
		(mime_type, "default_component_iid");

	query = g_strconcat ("bonobo:supported_mime_types.has_one (['", mime_type, 
			     "', '", supertype,
			     "', '*'])", NULL);


	if (default_component_iid != NULL) {
		sort[0] = g_strconcat ("iid == '", default_component_iid, "'", NULL);
	} else {
		sort[0] = g_strdup ("true");
	}

	short_list = gnome_vfs_mime_get_short_list_components (mime_type);
	short_list = g_list_concat (short_list,
				    gnome_vfs_mime_get_short_list_components (supertype));
	if (short_list != NULL) {
		sort[1] = g_strdup ("prefer_by_list_order(iid, ['");

		for (p = short_list; p != NULL; p = p->next) {
 			prev = sort[1];
			
			if (p->next != NULL) {
				sort[1] = g_strconcat (prev, ((OAF_ServerInfo *) (p->data))->iid, 
								    "','", NULL);
			} else {
				sort[1] = g_strconcat (prev, ((OAF_ServerInfo *) (p->data))->iid, 
								    "'])", NULL);
			}
			g_free (prev);
		}
		gnome_vfs_mime_component_list_free (short_list);
	} else {
		sort[1] = g_strdup ("true");
	}


	/* Prefer something that matches the exact type to something
           that matches the supertype */
	sort[2] = g_strconcat ("bonobo:supported_mime_types.has ('", mime_type, "')", NULL);

	/* Prefer something that matches the supertype to something that matches `*' */
	sort[3] = g_strconcat ("bonobo:supported_mime_types.has ('", supertype, "')", NULL);

	sort[4] = g_strdup ("name");
	sort[5] = NULL;

	info_list = oaf_query (query, sort, &ev);
	
	default_component = NULL;
	if (ev._major == CORBA_NO_EXCEPTION) {
		if (info_list != NULL && info_list->_length > 0) {
			default_component = OAF_ServerInfo_duplicate (&info_list->_buffer[0]);
		}
		CORBA_free (info_list);
	}

	g_free (supertype);
	g_free (query);
	g_free (sort[0]);
	g_free (sort[1]);
	g_free (sort[2]);
	g_free (sort[3]);
	g_free (sort[4]);

	CORBA_exception_free (&ev);

	return default_component;
}

static GList *
gnome_vfs_mime_str_list_merge (GList *a, 
			       GList *b)
{
	GList *pruned_b;
	GList *extended_a;
	GList *a_copy;

	pruned_b = str_list_difference (b, a);

	a_copy = g_list_copy (a);
	extended_a = g_list_concat (a_copy, pruned_b);

	/* No need to free a_copy or
         * pruned_b since they were concat()ed into
         * extended_a 
	 */

	return extended_a;
}


static GList *
gnome_vfs_mime_str_list_apply_delta (GList *list_to_process, 
				     GList *additions, 
				     GList *removals)
{
	GList *extended_original_list;
	GList *processed_list;

	extended_original_list = gnome_vfs_mime_str_list_merge (list_to_process, additions);

	processed_list = str_list_difference (extended_original_list, removals);
	
	g_list_free (extended_original_list);

	return processed_list;
}

static const char *
gnome_vfs_mime_get_value_for_user_level (const char *mime_type, 
					 const char *key_prefix)
{
	char *user_level;
	char *full_key;
	const char *value;

	/* Base list depends on user level. */
	user_level = get_user_level ();
	full_key = g_strconcat (key_prefix,
				"_for_",
				user_level,
				"_user_level",
				NULL);
	g_free (user_level);
	value = gnome_vfs_mime_get_value (mime_type, full_key);
	g_free (full_key);

	return value;
}


static GList *gnome_vfs_mime_do_short_list_processing (GList *short_list, 
						       GList *additions,
						       GList *removals, 
						       GList *supertype_short_list, 
						       GList *supertype_additions, 
						       GList *supertype_removals)
{
	GList *processed_supertype_list;
	GList *merged_system_and_supertype;
	GList *final_list;

	processed_supertype_list = gnome_vfs_mime_str_list_apply_delta (supertype_short_list,
									supertype_additions,
									supertype_removals);

	merged_system_and_supertype = gnome_vfs_mime_str_list_merge (short_list,
					     			     processed_supertype_list);

	final_list = gnome_vfs_mime_str_list_apply_delta (merged_system_and_supertype,
							  additions,
							  removals);
	
	g_list_free (processed_supertype_list);
	g_list_free (merged_system_and_supertype);

	return final_list;
}


/* sort_application_list
 *
 * Sort list alphabetically
 */
 
static int
sort_application_list (gconstpointer a, gconstpointer b)
{
	GnomeVFSMimeApplication *application1, *application2;

	application1 = (GnomeVFSMimeApplication *) a;
	application2 = (GnomeVFSMimeApplication *) b;

	return g_strcasecmp (application1->name, application2->name);
}

/**
 * gnome_vfs_mime_get_short_list_applications:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Return an alphabetically sorted list of GnomeVFSMimeApplication
 * data structures for the requested mime type.	The short list contains
 * "select" applications recommended for handling this MIME type, appropriate for
 * display to the user.
 * 
 * Return value: A GList * where the elements are GnomeVFSMimeApplication *
 * representing various applications to display in the short list for @mime_type.
 **/ 
GList *
gnome_vfs_mime_get_short_list_applications (const char *mime_type)
{
	GList *system_short_list;
	GList *short_list_additions;
	GList *short_list_removals;
	char *supertype;
	GList *supertype_short_list;
	GList *supertype_additions;
	GList *supertype_removals;
	GList *id_list;
	GList *p;
	GnomeVFSMimeApplication *application;
	GList *preferred_applications;

	if (mime_type == NULL) {
		return NULL;
	}


	system_short_list = comma_separated_str_to_str_list (gnome_vfs_mime_get_value_for_user_level 
							     (mime_type, 
							      "short_list_application_ids"));
	system_short_list = prune_ids_for_nonexistent_applications
		(system_short_list);

	/* get user short list delta (add list and remove list) */

	short_list_additions = comma_separated_str_to_str_list (gnome_vfs_mime_get_value
								(mime_type,
								 "short_list_application_user_additions"));
	short_list_additions = prune_ids_for_nonexistent_applications (short_list_additions);
	short_list_removals = comma_separated_str_to_str_list (gnome_vfs_mime_get_value
							       (mime_type,
								"short_list_application_user_removals"));

	/* Only include the supertype in the short list if we came up empty with
	   the specific types */
	supertype = gnome_vfs_get_supertype_from_mime_type (mime_type);

	if (!gnome_vfs_mime_type_is_supertype (mime_type) && system_short_list == NULL) {
		supertype_short_list = comma_separated_str_to_str_list 
			(gnome_vfs_mime_get_value_for_user_level 
			 (supertype, 
			  "short_list_application_ids"));
		supertype_short_list = prune_ids_for_nonexistent_applications
			(supertype_short_list);

		/* get supertype short list delta (add list and remove list) */

		supertype_additions = comma_separated_str_to_str_list 
			(gnome_vfs_mime_get_value
			 (supertype,
			  "short_list_application_user_additions"));
		supertype_removals = comma_separated_str_to_str_list 
			(gnome_vfs_mime_get_value
			 (supertype,
			  "short_list_application_user_removals"));
	} else {
		supertype_short_list = NULL;
		supertype_additions = NULL;
		supertype_removals = NULL;
	}
	g_free (supertype);


	/* compute list modified by delta */

	id_list = gnome_vfs_mime_do_short_list_processing (system_short_list, 
							   short_list_additions,
							   short_list_removals, 
							   supertype_short_list, 
							   supertype_additions, 
							   supertype_removals);

	preferred_applications = NULL;

	for (p = id_list; p != NULL; p = p->next) {
		application = gnome_vfs_application_registry_get_mime_application (p->data);
		if (application != NULL) {
			preferred_applications = g_list_prepend
				(preferred_applications, application);
		}
	}


	preferred_applications = g_list_reverse (preferred_applications);

	g_list_free_deep (system_short_list);
	g_list_free_deep (short_list_additions);
	g_list_free_deep (short_list_removals);
	g_list_free_deep (supertype_short_list);
	g_list_free_deep (supertype_additions);
	g_list_free_deep (supertype_removals);
	g_list_free (id_list);

	/* Sort list alphabetically by application name */
	preferred_applications = g_list_sort (preferred_applications, sort_application_list);
	
	return preferred_applications;
}



static char *
join_str_list (const char *separator, GList *list)
{
	char **strv;
	GList *p;
	int i;
	char *retval;

	/* Convert to a strv so we can use g_strjoinv.
	 * Saves code but could be made faster if we want.
	 */
	strv = g_new0 (char *, g_list_length (list) + 1);
	for (p = list, i = 0; p != NULL; p = p->next, i++) {
		strv[i] = (char *) p->data;
	}
	strv[i] = NULL;

	retval = g_strjoinv (separator, strv);

	g_free (strv);

	return retval;
}


/**
 * gnome_vfs_mime_get_short_list_components:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Return an unsorted sorted list of OAF_ServerInfo *
 * data structures for the requested mime type.	The short list contains
 * "select" components recommended for handling this MIME type, appropriate for
 * display to the user.
 * 
 * Return value: A GList * where the elements are OAF_ServerInfo *
 * representing various components to display in the short list for @mime_type.
 **/ 
GList *
gnome_vfs_mime_get_short_list_components (const char *mime_type)
{
	GList *system_short_list;
	GList *short_list_additions;
	GList *short_list_removals;
	char *supertype;
	GList *supertype_short_list;
	GList *supertype_additions;
	GList *supertype_removals;
	GList *iid_list;
	char *query;
	char *sort[2];
	char *iids_delimited;
	CORBA_Environment ev;
	OAF_ServerInfoList *info_list;
	GList *preferred_components;

	if (mime_type == NULL) {
		return NULL;
	}


	/* get short list IIDs for that user level */
	system_short_list = comma_separated_str_to_str_list (gnome_vfs_mime_get_value_for_user_level 
							     (mime_type, 
							      "short_list_component_iids"));

	/* get user short list delta (add list and remove list) */

	short_list_additions = comma_separated_str_to_str_list (gnome_vfs_mime_get_value
								(mime_type,
								 "short_list_component_user_additions"));
	
	short_list_removals = comma_separated_str_to_str_list (gnome_vfs_mime_get_value
							       (mime_type,
								"short_list_component_user_removals"));


	supertype = gnome_vfs_get_supertype_from_mime_type (mime_type);
	
	if (strcmp (supertype, mime_type) != 0) {
		supertype_short_list = comma_separated_str_to_str_list 
			(gnome_vfs_mime_get_value_for_user_level 
			 (supertype, 
			  "short_list_component_iids"));

		/* get supertype short list delta (add list and remove list) */

		supertype_additions = comma_separated_str_to_str_list 
			(gnome_vfs_mime_get_value
			 (supertype,
			  "short_list_component_user_additions"));
		supertype_removals = comma_separated_str_to_str_list 
			(gnome_vfs_mime_get_value
			 (supertype,
			  "short_list_component_user_removals"));
	} else {
		supertype_short_list = NULL;
		supertype_additions = NULL;
		supertype_removals = NULL;
	}

	/* compute list modified by delta */

	iid_list = gnome_vfs_mime_do_short_list_processing (system_short_list, 
							    short_list_additions,
							    short_list_removals, 
							    supertype_short_list, 
							    supertype_additions, 
							    supertype_removals);



	/* Do usual query but requiring that IIDs be one of the ones
           in the short list IID list. */
	
	preferred_components = NULL;
	if (iid_list != NULL) {
		CORBA_exception_init (&ev);

		iids_delimited = join_str_list ("','", iid_list);

		query = g_strconcat ("bonobo:supported_mime_types.has_one (['", mime_type, 
				     "', '", supertype,
				     "', '*'])",
				     " AND has(['", iids_delimited, "'], iid)", NULL);
		
		sort[0] = g_strconcat ("prefer_by_list_order(iid, ['", iids_delimited, "'])", NULL);
		sort[1] = NULL;
		
		info_list = oaf_query (query, sort, &ev);
		
		if (ev._major == CORBA_NO_EXCEPTION) {
			preferred_components = OAF_ServerInfoList_to_ServerInfo_g_list (info_list);
			CORBA_free (info_list);
		}

		g_free (iids_delimited);
		g_free (query);
		g_free (sort[0]);

		CORBA_exception_free (&ev);
	}

	g_free (supertype);
	g_list_free_deep (system_short_list);
	g_list_free_deep (short_list_additions);
	g_list_free_deep (short_list_removals);
	g_list_free_deep (supertype_short_list);
	g_list_free_deep (supertype_additions);
	g_list_free_deep (supertype_removals);
	g_list_free (iid_list);

	return preferred_components;
}


/**
 * gnome_vfs_mime_get_all_applications:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Return an alphabetically sorted list of GnomeVFSMimeApplication
 * data structures representing all applications in the MIME database registered
 * to handle files of MIME type @mime_type (and supertypes).
 * 
 * Return value: A GList * where the elements are GnomeVFSMimeApplication *
 * representing applications that handle MIME type @mime_type.
 **/ 
GList *
gnome_vfs_mime_get_all_applications (const char *mime_type)
{
	GList *applications, *node, *next;
	char *application_id;
	GnomeVFSMimeApplication *application;

	g_return_val_if_fail (mime_type != NULL, NULL);

	applications = gnome_vfs_application_registry_get_applications (mime_type);

	/* We get back a list of const char *, but the prune function
	 * wants a list of strings that we own.
	 */
	for (node = applications; node != NULL; node = node->next) {
		node->data = g_strdup (node->data);
	}

	/* Remove application ids representing nonexistent (not in path) applications */
	applications = prune_ids_for_nonexistent_applications (applications);

	/* Convert to GnomeVFSMimeApplication's (leaving out NULLs) */
	for (node = applications; node != NULL; node = next) {
		next = node->next;

		application_id = node->data;
		application = gnome_vfs_application_registry_get_mime_application (application_id);

		/* Replace the application ID with the application */
		if (application == NULL) {
			applications = g_list_remove_link (applications, node);
			g_list_free_1 (node);
		} else {
			node->data = application;
		}

		g_free (application_id);
	}

	return applications;
}

/**
 * gnome_vfs_mime_get_all_components:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * 
 * Return an alphabetically sorted list of OAF_ServerInfo
 * data structures representing all Bonobo components registered
 * to handle files of MIME type @mime_type (and supertypes).
 * 
 * Return value: A GList * where the elements are OAF_ServerInfo *
 * representing components that can handle MIME type @mime_type.
 **/ 
GList *
gnome_vfs_mime_get_all_components (const char *mime_type)
{
	OAF_ServerInfoList *info_list;
	GList *components_list;
	CORBA_Environment ev;
	char *supertype;
	char *query;
	char *sort[2];

	if (mime_type == NULL) {
		return NULL;
	}

	CORBA_exception_init (&ev);

	/* Find a component that supports either the exact mime type,
           the supertype, or all mime types. */

	/* FIXME bugzilla.eazel.com 1142: should probably check for
           the right interfaces too. Also slightly semantically
           different from nautilus in other tiny ways.
	*/
	supertype = gnome_vfs_get_supertype_from_mime_type (mime_type);
	query = g_strconcat ("bonobo:supported_mime_types.has_one (['", mime_type, 
			     "', '", supertype,
			     "', '*'])", NULL);
	g_free (supertype);
	
	/* Alphebetize by name, for the sake of consistency */
	sort[0] = g_strdup ("name");
	sort[1] = NULL;

	info_list = oaf_query (query, sort, &ev);
	
	if (ev._major == CORBA_NO_EXCEPTION) {
		components_list = OAF_ServerInfoList_to_ServerInfo_g_list (info_list);
		CORBA_free (info_list);
	} else {
		components_list = NULL;
	}

	g_free (query);
	g_free (sort[0]);

	CORBA_exception_free (&ev);

	return components_list;
}

static GnomeVFSResult
gnome_vfs_mime_edit_user_file_full (const char *mime_type, GList *keys, GList *values)
{
	GnomeVFSResult result;
	GList *p, *q;
	const char *key, *value;

	if (mime_type == NULL) {
		return GNOME_VFS_OK;
	}

	result = GNOME_VFS_OK;

	gnome_vfs_mime_freeze ();
	for (p = keys, q = values; p != NULL && q != NULL; p = p->next, q = q->next) {
		key = p->data;
		value = q->data;
		if (value == NULL) {
			value = "";
		}
		gnome_vfs_mime_set_value (mime_type, g_strdup (key), g_strdup (value));
	}
	gnome_vfs_mime_thaw ();

	return result;
}

static GnomeVFSResult
gnome_vfs_mime_edit_user_file_args (const char *mime_type, va_list args)
{
	GList *keys, *values;
	char *key, *value;
	GnomeVFSResult result;

	keys = NULL;
	values = NULL;
	for (;;) {
		key = va_arg (args, char *);
		if (key == NULL) {
			break;
		}
		value = va_arg (args, char *);
		keys = g_list_prepend (keys, key);
		values = g_list_prepend (values, value);
	}

	result = gnome_vfs_mime_edit_user_file_full (mime_type, keys, values);

	g_list_free (keys);
	g_list_free (values);

	return result;
}

static GnomeVFSResult
gnome_vfs_mime_edit_user_file_multiple (const char *mime_type, ...)
{
	va_list args;
	GnomeVFSResult result;

	va_start (args, mime_type);
	result = gnome_vfs_mime_edit_user_file_args (mime_type, args);
	va_end (args);

	return result;
}

static GnomeVFSResult
gnome_vfs_mime_edit_user_file (const char *mime_type, const char *key, const char *value)
{
	g_return_val_if_fail (key != NULL, GNOME_VFS_OK);
	return gnome_vfs_mime_edit_user_file_multiple (mime_type, key, value, NULL);
}

/**
 * gnome_vfs_mime_set_default_action_type:
 * @mime_type: A const char * containing a mime type, e.g. "image/png"
 * @action_type: A GnomeVFSMimeActionType containing the action to perform by default
 * 
 * Sets the default action type to be performed on files of MIME type @mime_type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_set_default_action_type (const char *mime_type,
					GnomeVFSMimeActionType action_type)
{
	const char *action_string;

	switch (action_type) {
	case GNOME_VFS_MIME_ACTION_TYPE_APPLICATION:
		action_string = "application";
		break;		
	case GNOME_VFS_MIME_ACTION_TYPE_COMPONENT:
		action_string = "component";
		break;
	case GNOME_VFS_MIME_ACTION_TYPE_NONE:
	default:
		action_string = "none";
	}

	return gnome_vfs_mime_edit_user_file
		(mime_type, "default_action_type", action_string);
}

/**
 * gnome_vfs_mime_set_default_application:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_id: A key representing an application in the MIME database 
 * (GnomeVFSMimeApplication->id, for example)
 * 
 * Sets the default application to be run on files of MIME type @mime_type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_set_default_application (const char *mime_type,
					const char *application_id)
{
	GnomeVFSResult result;

	result = gnome_vfs_mime_edit_user_file
		(mime_type, "default_application_id", application_id);

	/* If there's no default action type, set it to match this. */
	if (result == GNOME_VFS_OK
	    && application_id != NULL
	    && gnome_vfs_mime_get_default_action_type (mime_type) == GNOME_VFS_MIME_ACTION_TYPE_NONE) {
		result = gnome_vfs_mime_set_default_action_type (mime_type, GNOME_VFS_MIME_ACTION_TYPE_APPLICATION);
	}

	return result;
}

/**
 * gnome_vfs_mime_set_default_component:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @component_iid: The OAFIID of a component
 * 
 * Sets the default component to be activated for files of MIME type @mime_type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_set_default_component (const char *mime_type,
				      const char *component_iid)
{
	GnomeVFSResult result;

	result = gnome_vfs_mime_edit_user_file
		(mime_type, "default_component_iid", component_iid);

	/* If there's no default action type, set it to match this. */
	if (result == GNOME_VFS_OK
	    && component_iid != NULL
	    && gnome_vfs_mime_get_default_action_type (mime_type) == GNOME_VFS_MIME_ACTION_TYPE_NONE) {
		gnome_vfs_mime_set_default_action_type (mime_type, GNOME_VFS_MIME_ACTION_TYPE_COMPONENT);
	}

	return result;
}

/**
 * gnome_vfs_mime_set_short_list_applications:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_ids: GList of const char * application ids
 * 
 * Set the short list of applications for the specified MIME type. The short list
 * contains applications recommended for possible selection by the user.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_set_short_list_applications (const char *mime_type,
					    GList *application_ids)
{
	char *user_level, *id_list_key;
	char *addition_string, *removal_string;
	GList *short_list_id_list;
	GList *short_list_addition_list;
	GList *short_list_removal_list;
	GnomeVFSResult result;

	/* Get base list. */
	/* Base list depends on user level. */
	user_level = get_user_level ();
	id_list_key = g_strconcat ("short_list_application_ids_for_",
				   user_level,
				   "_user_level",
				   NULL);
	g_free (user_level);
	short_list_id_list = comma_separated_str_to_str_list
		(gnome_vfs_mime_get_value (mime_type, id_list_key));
	g_free (id_list_key);

	/* Compute delta. */
	short_list_addition_list = str_list_difference (application_ids, short_list_id_list);
	short_list_removal_list = str_list_difference (short_list_id_list, application_ids);
	addition_string = str_list_to_comma_separated_str (short_list_addition_list);
	removal_string = str_list_to_comma_separated_str (short_list_removal_list);
	g_list_free_deep (short_list_id_list);
	g_list_free (short_list_addition_list);
	g_list_free (short_list_removal_list);

	/* Write it. */
	result = gnome_vfs_mime_edit_user_file_multiple
		(mime_type,
		 "short_list_application_user_additions", addition_string,
		 "short_list_application_user_removals", removal_string,
		 NULL);

	g_free (addition_string);
	g_free (removal_string);

	return result;
}

/**
 * gnome_vfs_mime_set_short_list_components:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @component_iids: GList of const char * OAF IIDs
 * 
 * Set the short list of components for the specified MIME type. The short list
 * contains companents recommended for possible selection by the user. * 
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_set_short_list_components (const char *mime_type,
					  GList *component_iids)
{
	char *user_level, *id_list_key;
	char *addition_string, *removal_string;
	GList *short_list_id_list;
	GList *short_list_addition_list;
	GList *short_list_removal_list;
	GnomeVFSResult result;

	/* Get base list. */
	/* Base list depends on user level. */
	user_level = get_user_level ();
	id_list_key = g_strconcat ("short_list_component_iids_for_",
				   user_level,
				   "_user_level",
				   NULL);
	g_free (user_level);
	short_list_id_list = comma_separated_str_to_str_list
		(gnome_vfs_mime_get_value (mime_type, id_list_key));
	g_free (id_list_key);

	/* Compute delta. */
	short_list_addition_list = str_list_difference (component_iids, short_list_id_list);
	short_list_removal_list = str_list_difference (short_list_id_list, component_iids);
	addition_string = str_list_to_comma_separated_str (short_list_addition_list);
	removal_string = str_list_to_comma_separated_str (short_list_removal_list);
	g_list_free (short_list_addition_list);
	g_list_free (short_list_removal_list);

	/* Write it. */
	result = gnome_vfs_mime_edit_user_file_multiple
		(mime_type,
		 "short_list_component_user_additions", addition_string,
		 "short_list_component_user_removals", removal_string,
		 NULL);

	g_free (addition_string);
	g_free (removal_string);

	return result;
}

/* FIXME bugzilla.eazel.com 1148: 
 * The next set of helper functions are all replicated in nautilus-mime-actions.c.
 * Need to refactor so they can share code.
 */
static gint
gnome_vfs_mime_application_has_id (GnomeVFSMimeApplication *application, const char *id)
{
	return strcmp (application->id, id);
}

static gint
gnome_vfs_mime_id_matches_application (const char *id, GnomeVFSMimeApplication *application)
{
	return gnome_vfs_mime_application_has_id (application, id);
}

#ifdef USING_OAF
static gint
gnome_vfs_mime_id_matches_component (const char *iid, OAF_ServerInfo *component)
{
	return strcmp (component->iid, iid);
}
#endif

static gint 
gnome_vfs_mime_application_matches_id (GnomeVFSMimeApplication *application, const char *id)
{
	return gnome_vfs_mime_id_matches_application (id, application);
}

#ifdef USING_OAF
static gint 
gnome_vfs_mime_component_matches_id (OAF_ServerInfo *component, const char *iid)
{
	return gnome_vfs_mime_id_matches_component (iid, component);
}
#endif

/**
 * gnome_vfs_mime_id_in_application_list:
 * @id: An application id.
 * @applications: A GList * whose nodes are GnomeVFSMimeApplications, such as the
 * result of gnome_vfs_mime_get_short_list_applications().
 * 
 * Check whether an application id is in a list of GnomeVFSMimeApplications.
 * 
 * Return value: TRUE if an application whose id matches @id is in @applications.
 */
gboolean
gnome_vfs_mime_id_in_application_list (const char *id, GList *applications)
{
	return g_list_find_custom
		(applications, (gpointer) id,
		 (GCompareFunc) gnome_vfs_mime_application_matches_id) != NULL;
}

/**
 * gnome_vfs_mime_id_in_component_list:
 * @iid: A component iid.
 * @components: A GList * whose nodes are OAF_ServerInfos, such as the
 * result of gnome_vfs_mime_get_short_list_components().
 * 
 * Check whether a component iid is in a list of OAF_ServerInfos.
 * 
 * Return value: TRUE if a component whose iid matches @iid is in @components.
 */
gboolean
gnome_vfs_mime_id_in_component_list (const char *iid, GList *components)
{
	return g_list_find_custom
		(components, (gpointer) iid,
		 (GCompareFunc) gnome_vfs_mime_component_matches_id) != NULL;
	return FALSE;
}

/**
 * gnome_vfs_mime_id_list_from_application_list:
 * @applications: A GList * whose nodes are GnomeVFSMimeApplications, such as the
 * result of gnome_vfs_mime_get_short_list_applications().
 * 
 * Create a list of application ids from a list of GnomeVFSMimeApplications.
 * 
 * Return value: A new list where each GnomeVFSMimeApplication in the original
 * list is replaced by a char * with the application's id. The original list is
 * not modified.
 */
GList *
gnome_vfs_mime_id_list_from_application_list (GList *applications)
{
	GList *result;
	GList *node;

	result = NULL;
	
	for (node = applications; node != NULL; node = node->next) {
		result = g_list_append 
			(result, g_strdup (((GnomeVFSMimeApplication *)node->data)->id));
	}

	return result;
}

/**
 * gnome_vfs_mime_id_list_from_component_list:
 * @components: A GList * whose nodes are OAF_ServerInfos, such as the
 * result of gnome_vfs_mime_get_short_list_components().
 * 
 * Create a list of component iids from a list of OAF_ServerInfos.
 * 
 * Return value: A new list where each OAF_ServerInfo in the original
 * list is replaced by a char * with the component's iid. The original list is
 * not modified.
 */
GList *
gnome_vfs_mime_id_list_from_component_list (GList *components)
{
	GList *list = NULL;
	GList *node;

	for (node = components; node != NULL; node = node->next) {
		list = g_list_prepend 
			(list, g_strdup (((OAF_ServerInfo *)node->data)->iid));
	}
	return g_list_reverse (list);
}

static void
g_list_free_deep (GList *list)
{
	g_list_foreach (list, (GFunc) g_free, NULL);
	g_list_free (list);
}

/**
 * gnome_vfs_mime_add_application_to_short_list:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_id: const char * containing the application's id in the MIME database
 * 
 * Add an application to the short list for MIME type @mime_type. The short list contains
 * applications recommended for display as choices to the user for a particular MIME type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_add_application_to_short_list (const char *mime_type,
					      const char *application_id)
{
	GList *old_list, *new_list;
	GnomeVFSResult result;

	old_list = gnome_vfs_mime_get_short_list_applications (mime_type);

	if (gnome_vfs_mime_id_in_application_list (application_id, old_list)) {
		result = GNOME_VFS_OK;
	} else {
		new_list = g_list_append (gnome_vfs_mime_id_list_from_application_list (old_list), 
					  g_strdup (application_id));
		result = gnome_vfs_mime_set_short_list_applications (mime_type, new_list);
		g_list_free_deep (new_list);
	}

	gnome_vfs_mime_application_list_free (old_list);

	return result;
}

/**
 * gnome_vfs_mime_remove_application_from_list:
 * @applications: A GList * whose nodes are GnomeVFSMimeApplications, such as the
 * result of gnome_vfs_mime_get_short_list_applications().
 * @application_id: The id of an application to remove from @applications.
 * @did_remove: If non-NULL, this is filled in with TRUE if the application
 * was found in the list, FALSE otherwise.
 * 
 * Remove an application specified by id from a list of GnomeVFSMimeApplications.
 * 
 * Return value: The modified list. If the application is not found, the list will 
 * be unchanged.
 */
GList *
gnome_vfs_mime_remove_application_from_list (GList *applications, 
					     const char *application_id,
					     gboolean *did_remove)
{
	GList *matching_node;
	
	matching_node = g_list_find_custom 
		(applications, (gpointer)application_id,
		 (GCompareFunc) gnome_vfs_mime_application_matches_id);
	if (matching_node != NULL) {
		applications = g_list_remove_link (applications, matching_node);
		gnome_vfs_mime_application_list_free (matching_node);
	}

	if (did_remove != NULL) {
		*did_remove = matching_node != NULL;
	}

	return applications;
}

/**
 * gnome_vfs_mime_remove_application_from_short_list:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_id: const char * containing the application's id in the MIME database
 * 
 * Remove an application from the short list for MIME type @mime_type. The short list contains
 * applications recommended for display as choices to the user for a particular MIME type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_remove_application_from_short_list (const char *mime_type,
						   const char *application_id)
{
	GnomeVFSResult result;
	GList *old_list, *new_list;
	gboolean was_in_list;

	old_list = gnome_vfs_mime_get_short_list_applications (mime_type);
	old_list = gnome_vfs_mime_remove_application_from_list 
		(old_list, application_id, &was_in_list);

	if (!was_in_list) {
		result = GNOME_VFS_OK;
	} else {
		new_list = gnome_vfs_mime_id_list_from_application_list (old_list);
		result = gnome_vfs_mime_set_short_list_applications (mime_type, new_list);
		g_list_free_deep (new_list);
	}

	gnome_vfs_mime_application_list_free (old_list);

	return result;
}

/**
 * gnome_vfs_mime_add_component_to_short_list:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @iid: const char * containing the component's OAF IID
 * 
 * Add a component to the short list for MIME type @mime_type. The short list contains
 * components recommended for display as choices to the user for a particular MIME type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_add_component_to_short_list (const char *mime_type,
					    const char *iid)
{
	GnomeVFSResult result;
	GList *old_list, *new_list;

	old_list = gnome_vfs_mime_get_short_list_components (mime_type);

	if (gnome_vfs_mime_id_in_component_list (iid, old_list)) {
		result = GNOME_VFS_OK;
	} else {
		new_list = g_list_append (gnome_vfs_mime_id_list_from_component_list (old_list), 
					  g_strdup (iid));
		result = gnome_vfs_mime_set_short_list_components (mime_type, new_list);
		g_list_free_deep (new_list);
	}

	gnome_vfs_mime_component_list_free (old_list);

	return result;
}

/**
 * gnome_vfs_mime_remove_component_from_list:
 * @components: A GList * whose nodes are OAF_ServerInfos, such as the
 * result of gnome_vfs_mime_get_short_list_components().
 * @iid: The iid of a component to remove from @components.
 * @did_remove: If non-NULL, this is filled in with TRUE if the component
 * was found in the list, FALSE otherwise.
 * 
 * Remove a component specified by iid from a list of OAF_ServerInfos.
 * 
 * Return value: The modified list. If the component is not found, the list will 
 * be unchanged.
 */
GList *
gnome_vfs_mime_remove_component_from_list (GList *components, 
					   const char *iid,
					   gboolean *did_remove)
{
	GList *matching_node;
	
	matching_node = g_list_find_custom 
		(components, (gpointer)iid,
		 (GCompareFunc) gnome_vfs_mime_component_matches_id);
	if (matching_node != NULL) {
		components = g_list_remove_link (components, matching_node);
		gnome_vfs_mime_component_list_free (matching_node);
	}

	if (did_remove != NULL) {
		*did_remove = matching_node != NULL;
	}
	return components;
}

/**
 * gnome_vfs_mime_remove_component_from_short_list:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @iid: const char * containing the component's OAF IID
 * 
 * Remove a component from the short list for MIME type @mime_type. The short list contains
 * components recommended for display as choices to the user for a particular MIME type.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_remove_component_from_short_list (const char *mime_type,
						 const char *iid)
{
	GnomeVFSResult result;
	GList *old_list, *new_list;
	gboolean was_in_list;

	old_list = gnome_vfs_mime_get_short_list_components (mime_type);
	old_list = gnome_vfs_mime_remove_component_from_list 
		(old_list, iid, &was_in_list);

	if (!was_in_list) {
		result = GNOME_VFS_OK;
	} else {
		new_list = gnome_vfs_mime_id_list_from_component_list (old_list);
		result = gnome_vfs_mime_set_short_list_components (mime_type, new_list);
		g_list_free_deep (new_list);
	}

	gnome_vfs_mime_component_list_free (old_list);

	return result;
}

/**
 * gnome_vfs_mime_add_extension:
 * @extension: The extension to add (e.g. "txt")
 * @mime_type: The mime type to add the mapping to.
 * 
 * Add a file extension to the specificed MIME type in the MIME database.
 * 
 * Return value: GnomeVFSResult indicating the success of the operation or any
 * errors that may have occurred.
 **/
GnomeVFSResult
gnome_vfs_mime_add_extension (const char *mime_type, const char *extension)
{
	GnomeVFSResult result;
	GList *list, *element;
	gchar *extensions, *old_extensions;

	extensions = NULL;
	old_extensions = NULL;

	result = GNOME_VFS_OK;
	
	list = gnome_vfs_mime_get_extensions_list (mime_type);	
	if (list == NULL) {
		/* List is NULL. This means there are no current registered extensions. 
		 * Add the new extension and it will cause the list to be created next time.
		 */
		result = gnome_vfs_mime_set_registered_type_key (mime_type, "ext", extension);
		return result;
	}

	/* Check for duplicates */
	for (element = list; element != NULL; element = element->next) {
		if (strcmp (extension, (char *)element->data) == 0) {					
			gnome_vfs_mime_extensions_list_free (list);
			return result;
		}
	}

	/* Add new extension to list */
	for (element = list; element != NULL; element = element->next) {		
		if (extensions != NULL) {
			old_extensions = extensions;
			extensions = g_strdup_printf ("%s %s", old_extensions, (char *)element->data);
			g_free (old_extensions);
		} else {
			extensions = g_strdup_printf ("%s", (char *)element->data);
		}
	}
	
	if (extensions != NULL) {
		old_extensions = extensions;
		extensions = g_strdup_printf ("%s %s", old_extensions, extension);
		g_free (old_extensions);

		/* Add extensions to hash table and flush into the file. */
		gnome_vfs_mime_set_registered_type_key (mime_type, "ext", extensions);
	}
	
	gnome_vfs_mime_extensions_list_free (list);

	return result;
}

/**
 * gnome_vfs_mime_remove_extension:
 * @extension: The extension to remove
 * @mime_type: The mime type to remove the extension from
 * 
 * Removes a file extension from the specificed MIME type in the MIME database.
 * 
 * Return value: GnomeVFSResult indicating the success of the operation or any
 * errors that may have occurred.
 **/
GnomeVFSResult
gnome_vfs_mime_remove_extension (const char *mime_type, const char *extension)
{
	GList *list, *element;
	gchar *extensions, *old_extensions;
	gboolean in_list;
	GnomeVFSResult result;

	result = GNOME_VFS_OK;
	extensions = NULL;
	old_extensions = NULL;
	in_list = FALSE;
	
	list = gnome_vfs_mime_get_extensions_list (mime_type);	
	if (list == NULL) {
		return result;
	}

	/* See if extension is in list */
	for (element = list; element != NULL; element = element->next) {
		if (strcmp (extension, (char *)element->data) == 0) {					
			/* Remove extension from list */			
			in_list = TRUE;
			list = g_list_remove (list, element->data);
			g_free (element->data);
			element = NULL;
		}

		if (in_list) {
			break;
		}
	}

	/* Exit if we found no match */
	if (!in_list) {
		gnome_vfs_mime_extensions_list_free (list);
		return result;
	}
	
	/* Create new extension list */
	for (element = list; element != NULL; element = element->next) {		
		if (extensions != NULL) {
			old_extensions = extensions;
			extensions = g_strdup_printf ("%s %s", old_extensions, (char *)element->data);
			g_free (old_extensions);
		} else {
			extensions = g_strdup_printf ("%s", (char *)element->data);
		}
	}
	
	if (extensions != NULL) {
		/* Add extensions to hash table and flush into the file */
		gnome_vfs_mime_set_registered_type_key (mime_type, "ext", extensions);
	}
	
	gnome_vfs_mime_extensions_list_free (list);

	return result;
}

/**
 * gnome_vfs_mime_extend_all_applications:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_ids: a GList of const char * containing application ids
 * 
 * Register @mime_type as being handled by all applications list in @application_ids.
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_extend_all_applications (const char *mime_type,
					GList *application_ids)
{
	GList *li;

	g_return_val_if_fail (mime_type != NULL, GNOME_VFS_ERROR_INTERNAL);

	for (li = application_ids; li != NULL; li = li->next) {
		const char *application_id = li->data;
		gnome_vfs_application_registry_add_mime_type (application_id,
							      mime_type);
	}

	return gnome_vfs_application_registry_sync ();
}

/**
 * gnome_vfs_mime_remove_from_all_applications:
 * @mime_type: A const char * containing a mime type, e.g. "application/x-php"
 * @application_ids: a GList of const char * containing application ids
 * 
 * Remove @mime_type as a handled type from every application in @application_ids
 * 
 * Return value: A GnomeVFSResult indicating the success of the operation or reporting 
 * any errors encountered.
 **/
GnomeVFSResult
gnome_vfs_mime_remove_from_all_applications (const char *mime_type,
					     GList *application_ids)
{
	GList *li;

	g_return_val_if_fail (mime_type != NULL, GNOME_VFS_ERROR_INTERNAL);

	for (li = application_ids; li != NULL; li = li->next) {
		const char *application_id = li->data;
		gnome_vfs_application_registry_remove_mime_type (application_id,
								 mime_type);
	}

	return gnome_vfs_application_registry_sync ();
}


/**
 * gnome_vfs_mime_application_copy:
 * @application: The GnomeVFSMimeApplication to be duplicated.
 * 
 * Creates a newly referenced copy of a GnomeVFSMimeApplication object.
 * 
 * Return value: A copy of @application
 **/
GnomeVFSMimeApplication *
gnome_vfs_mime_application_copy (GnomeVFSMimeApplication *application)
{
	GnomeVFSMimeApplication *result;

	if (application == NULL) {
		return NULL;
	}
	
	result = g_new0 (GnomeVFSMimeApplication, 1);
	result->id = g_strdup (application->id);
	result->name = g_strdup (application->name);
	result->command = g_strdup (application->command);
	result->can_open_multiple_files = application->can_open_multiple_files;
	result->expects_uris = application->expects_uris;
	result->supported_uri_schemes = copy_str_list (application->supported_uri_schemes);
	result->requires_terminal = application->requires_terminal;

	return result;
}

/**
 * gnome_vfs_mime_application_free:
 * @application: The GnomeVFSMimeApplication to be freed
 * 
 * Frees a GnomeVFSMimeApplication *.
 * 
 **/
void
gnome_vfs_mime_application_free (GnomeVFSMimeApplication *application) 
{
	if (application != NULL) {
		g_free (application->name);
		g_free (application->command);
		g_list_foreach (application->supported_uri_schemes,
				(GFunc) g_free,
				NULL);
		g_list_free (application->supported_uri_schemes);
		g_free (application->id);
		g_free (application);
	}
}

/**
 * gnome_vfs_mime_action_free:
 * @action: The GnomeVFSMimeAction to be freed
 * 
 * Frees a GnomeVFSMimeAction *.
 * 
 **/
void
gnome_vfs_mime_action_free (GnomeVFSMimeAction *action) 
{
	switch (action->action_type) {
	case GNOME_VFS_MIME_ACTION_TYPE_APPLICATION:
		gnome_vfs_mime_application_free (action->action.application);
		break;
	case GNOME_VFS_MIME_ACTION_TYPE_COMPONENT:
		CORBA_free (action->action.component);
		break;
	default:
		g_assert_not_reached ();
	}

	g_free (action);
}

/**
 * gnome_vfs_mime_application_list_free:
 * @list: a GList of GnomeVFSApplication * to be freed
 * 
 * Frees lists of GnomeVFSApplications, as returned from functions such
 * as gnome_vfs_get_all_applications().
 * 
 **/
void
gnome_vfs_mime_application_list_free (GList *list)
{
	g_list_foreach (list, (GFunc) gnome_vfs_mime_application_free, NULL);
	g_list_free (list);
}

/**
 * gnome_vfs_mime_component_list_free:
 * @list: a GList of OAF_ServerInfo * to be freed
 * 
 * Frees lists of OAF_ServerInfo * (as returned from functions such
 * as @gnome_vfs_get_all_components)
 * 
 **/
void
gnome_vfs_mime_component_list_free (GList *list)
{
	g_list_foreach (list, (GFunc) CORBA_free, NULL);
	g_list_free (list);
}

/**
 * gnome_vfs_mime_application_new_from_id:
 * @id: A const char * containing an application id
 * 
 * Fetches the GnomeVFSMimeApplication associated with the specified
 * application ID from the MIME database.
 *
 * Return value: GnomeVFSMimeApplication * corresponding to @id
 **/
GnomeVFSMimeApplication *
gnome_vfs_mime_application_new_from_id (const char *id)
{
	return gnome_vfs_application_registry_get_mime_application (id);
}

static gboolean
application_known_to_be_nonexistent (const char *application_id)
{
	const char *command;

	g_return_val_if_fail (application_id != NULL, FALSE);

	command = gnome_vfs_application_registry_peek_value
		(application_id,
		 GNOME_VFS_APPLICATION_REGISTRY_COMMAND);

	if (command == NULL) {
		return TRUE;
	}

	return !gnome_vfs_is_executable_command_string (command);
}

static GList *
prune_ids_for_nonexistent_applications (GList *list)
{
	GList *p, *next;

	for (p = list; p != NULL; p = next) {
		next = p->next;

		if (application_known_to_be_nonexistent (p->data)) {
			list = g_list_remove_link (list, p);
			g_free (p->data);
			g_list_free_1 (p);
		}
	}

	return list;
}

static GList *
OAF_ServerInfoList_to_ServerInfo_g_list (OAF_ServerInfoList *info_list)
{
	GList *retval;
	int i;
	
	retval = NULL;
	if (info_list != NULL && info_list->_length > 0) {
		for (i = 0; i < info_list->_length; i++) {
			retval = g_list_prepend (retval, OAF_ServerInfo_duplicate (&info_list->_buffer[i]));
		}
		retval = g_list_reverse (retval);
	}

	return retval;
}

#ifdef HAVE_GCONF
static void
unref_gconf_engine (void)
{
	gconf_engine_unref (gconf_engine);
}
#endif /* HAVE_GCONF */

/* Returns the Nautilus user level, a string.
 * This does beg the question: Why does gnome-vfs have the Nautilus
 * user level coded into it? Eventually we might want to call this the
 * GNOME user level or something if we can figure out a clear concept
 * that works across GNOME.
 */
static char *
get_user_level (void)
{
	char *user_level = NULL;

#ifdef HAVE_GCONF
	/* Create the gconf engine once. */
	if (gconf_engine == NULL) {
		/* This sequence is needed in case no one has initialized GConf. */
		if (!gconf_is_initialized ()) {
			char *fake_argv[] = { "gnome-vfs", NULL };
			gconf_init (1, fake_argv, NULL);
		}

		gconf_engine = gconf_engine_get_default ();
		g_atexit (unref_gconf_engine);
	}

	user_level = gconf_engine_get_string (gconf_engine, "/apps/nautilus/user_level", NULL);
#endif

	if (user_level == NULL) {
		user_level = g_strdup ("novice");
	}

	/* If value is invalid, assume "novice". */
	if (strcmp (user_level, "novice") != 0 &&
	    strcmp (user_level, "intermediate") != 0 &&
	    strcmp (user_level, "advanced") != 0) {
		g_free (user_level);
		user_level = g_strdup ("novice");
	}

	return user_level;
}

static char **
strsplit_handle_null (const char *str, const char *delim, int max)
{
	return g_strsplit ((str == NULL ? "" : str), delim, max);
}

static GList *
strsplit_to_list (const char *str, const char *delim, int max)
{
	char **strv;
	GList *retval;
	int i;

	strv = strsplit_handle_null (str, delim, max);

	retval = NULL;

	for (i = 0; strv[i] != NULL; i++) {
		retval = g_list_prepend (retval, strv[i]);
	}

	retval = g_list_reverse (retval);
	
	/* Don't strfreev, since we didn't copy the individual strings. */
	g_free (strv);

	return retval;
}

static char *
strjoin_from_list (const char *separator, GList *list)
{
	char **strv;
	int i;
	GList *p;
	char *retval;

	strv = g_new0 (char *, (g_list_length (list) + 1));

	for (p = list, i = 0; p != NULL; p = p->next, i++) {
		strv[i] = p->data;
	}

	retval = g_strjoinv (separator, strv);

	g_free (strv);

	return retval;
}

static GList *
comma_separated_str_to_str_list (const char *str)
{
	return strsplit_to_list (str, ",", 0);
}

static char *
str_list_to_comma_separated_str (GList *list)
{
	return strjoin_from_list (",", list);
}

static GList *
str_list_difference (GList *a, GList *b)
{
	GList *node, *result;

	/* Uses an N^2 algorithm rather than a more efficient one
	 * that sorts or creates a hash table.
	 */

	result = NULL;

	for (node = a; node != NULL; node = node->next) {
		if (g_list_find_custom (b, node->data, (GCompareFunc) strcmp) == NULL) {
			result = g_list_prepend (result, node->data);
		}
	}

	return g_list_reverse (result);
}

static GList *
copy_str_list (GList *string_list)
{
	GList *copy, *node;
       
	copy = NULL;
	for (node = string_list; node != NULL; node = node->next) {
		copy = g_list_prepend (copy, 
				       g_strdup ((char *) node->data));
				       }
	return g_list_reverse (copy);
}
