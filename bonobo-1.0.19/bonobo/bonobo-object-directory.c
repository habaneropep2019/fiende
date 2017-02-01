/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 c-set-style: linux -*- */
/**
 * bonobo-object-directory.c: abstract the object directory
 *
 * Authors:
 *    Michael Meeks     (michael@helixcode.com)
 *    Havoc Pennington  (hp@redhat.com)
 *    Anders Carlsson   (andersca@gnu.org)
 *    Maciej Stachowiak (mjs@eazel.com)
 *
 * Copyright 1999, 2000 Havoc Pennington, Anders Carlsson,
 *                      Eazel, Inc. Helix Code, Inc.
 */

#include "config.h"
#include <glib.h>
#include <libgnome/gnome-defs.h>
#define GNOME_EXPLICIT_TRANSLATION_DOMAIN PACKAGE
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-mime.h>
#include "bonobo-object-directory.h"
#include <liboaf/liboaf.h>

struct _ODServerInfo {
        guint refcount;
        gchar* iid;
	gchar* name;
        gchar* desc;
};

ODServerInfo*
bonobo_directory_new_server_info (const gchar *iid,
				  const gchar *name,
				  const gchar *desc)
{
        ODServerInfo *info;

        info = g_new (ODServerInfo, 1);

        info->refcount = 1;
        info->iid = iid ? g_strdup (iid) : NULL;
	info->name = name ? g_strdup (name) : NULL;
        info->desc = desc ? g_strdup (desc) : NULL;

        return info;
}

const gchar*
bonobo_directory_get_server_info_id (ODServerInfo *info)
{
        return info->iid;
}

const gchar*
bonobo_directory_get_server_info_name (ODServerInfo *info)
{
	return info->name;
}
			 
const gchar*
bonobo_directory_get_server_info_description (ODServerInfo *info)
{
        return info->desc;
}

void
bonobo_directory_server_info_ref (ODServerInfo *info)
{
        info->refcount += 1;
}

void
bonobo_directory_server_info_unref (ODServerInfo *info)
{
        g_return_if_fail(info != NULL);
        g_return_if_fail(info->refcount > 0);

        info->refcount -= 1;

        if (info->refcount == 0) {
                g_free (info->iid);
		g_free (info->name);
                g_free (info->desc);
                g_free (info);
        }
}

void
bonobo_directory_free_server_list (GList *list)
{
        GList *l;

	for (l = list; l; l = l->next)
                bonobo_directory_server_info_unref (l->data);

        g_list_free (list);
}


CORBA_ORB
bonobo_directory_get_orb (void)
{
	return oaf_orb_get ();
}

static char *
build_id_query_fragment (const char **required_ids)
{
        const char **required_ids_iter;
	const char **query_components_iter;
        char       **query_components;
	char        *query;
        guint        n_required = 0;

        /* We need to build a query up from the required_ids */
        required_ids_iter = required_ids;

        while (required_ids && *required_ids_iter) {
                ++n_required;
                ++required_ids_iter;
        }

        query_components = g_new0 (gchar*, n_required + 1);

        query_components_iter = (const gchar **) query_components;
        required_ids_iter = required_ids;

        while (*required_ids_iter) {
                *query_components_iter = g_strconcat ("repo_ids.has('",
                                                      *required_ids_iter,
                                                      "')",
                                                      NULL);
                ++query_components_iter;
                ++required_ids_iter;
        }

        query = g_strjoinv (" AND ", query_components);

        g_strfreev (query_components);

	return query;
}

GList *
bonobo_directory_get_server_list (const gchar **required_ids)
{
        GList *retval = NULL;
        gchar *query;
        OAF_ServerInfoList *servers;
        CORBA_Environment ev;
        guint i, j;
        
        g_return_val_if_fail (required_ids != NULL, NULL);
        g_return_val_if_fail (*required_ids != NULL, NULL);

	query = build_id_query_fragment (required_ids);

        CORBA_exception_init (&ev);
        servers = oaf_query (query, NULL, &ev);
        g_free (query);
        CORBA_exception_free (&ev);

        if (servers == NULL)
                return NULL;

	for (i = 0; i < servers->_length; i++) {
                OAF_ServerInfo *oafinfo = &servers->_buffer[i];
                ODServerInfo *info;
		gchar *name = NULL, *desc = NULL;

		for (j = 0; j < oafinfo->props._length; j++) {
			if (oafinfo->props._buffer[j].v._d != OAF_P_STRING)
				continue;

			if (strcmp (oafinfo->props._buffer[j].name, "name") == 0)
				name = oafinfo->props._buffer[j].v._u.value_string;

			else if (strcmp (oafinfo->props._buffer[j].name, "description") == 0)
				desc = oafinfo->props._buffer[j].v._u.value_string;

			/* FIXME: internationalize here */
		}

		/*
		 * If no name attribute exists, use the description attribute.
		 *  If no description attribute exists, use the name attribute.
		 *  If neither a description attribute nor a name attribute exists, use the oafiid
		 */
		if (!name && !desc)
			name = desc = oafinfo->iid;

		if (!name)
			name = desc;

		if (!desc)
			desc = name;
		
                info = bonobo_directory_new_server_info (oafinfo->iid,
					   name,
					   desc);

                retval = g_list_prepend (retval, info);
        }

        CORBA_free (servers);
        
        return g_list_reverse (retval);
}


char *
bonobo_directory_find_for_file (const char  *fname,
				const char **required_ids,
				char       **error)
{
	char *query, *interfaces;
	const char *mime_type;
	char *iid;
	CORBA_Environment ev;
        OAF_ServerInfoList *servers;
	OAF_ServerInfo *oafinfo;

	if (!fname) {
		if (error)
			*error = g_strdup (_("No filename"));
		return NULL;
	}

	if (!(mime_type = gnome_mime_type ((char *) fname))) {
		if (error)
			*error = g_strdup_printf (_("unknown mime type for '%s'"), fname);
		return CORBA_OBJECT_NIL;
	}

	interfaces = build_id_query_fragment (required_ids);

	if (required_ids && required_ids [0] && interfaces)
		query = g_strdup_printf ("%s AND bonobo:supported_mime_types.has ('%s')",
					 interfaces, mime_type);
	else
		query = g_strdup_printf ("bonobo:supported_mime_types.has ('%s')",
					 mime_type);

	g_free (interfaces);

        CORBA_exception_init (&ev);

        servers = oaf_query (query, NULL, &ev);
        g_free (query);

        CORBA_exception_free (&ev);

        if (servers == CORBA_OBJECT_NIL || !servers->_buffer) {
		if (error)
			*error = g_strdup_printf (
				_("no handlers for mime type '%s'"), mime_type);
                return NULL;
	}

	/* Just return the first one */
	oafinfo = &servers->_buffer [0];
	iid = g_strdup (oafinfo->iid);

        CORBA_free (servers);

	if (error)
		*error = NULL;
        
        return iid;
}

CORBA_Object
od_server_activate_with_id (const gchar       *iid,
			    gint               flags,
			    CORBA_Environment *ev)
{
	CORBA_Environment *real_ev, tmp_ev;
	CORBA_Object retval;
	
	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}
		
	retval = oaf_activate_from_id ((gchar *) iid, 0, NULL, real_ev);

	if (!ev)
		CORBA_exception_free (&tmp_ev);
	
	return retval;
}

ODRegistrationResult
bonobo_directory_register_server (CORBA_Object objref,
				  const gchar *iid)
{
        OAF_RegistrationResult result;
	ODRegistrationResult retval;

        result = oaf_active_server_register (iid, objref);

        switch (result) {
        case OAF_REG_SUCCESS:
                retval = OD_REG_SUCCESS;
                break;
                
        case OAF_REG_NOT_LISTED:
                retval = OD_REG_NOT_LISTED;
                break;

        case OAF_REG_ALREADY_ACTIVE:
                retval = OD_REG_ALREADY_ACTIVE;
                break;

        case OAF_REG_ERROR:
        default:
                retval = OD_REG_ERROR;
                break;
        }

	return retval;
}

ODRegistrationResult
bonobo_directory_unregister_server (CORBA_Object objref,
				    const gchar *iid)
{
        oaf_active_server_unregister (iid, objref);
        
        return OD_REG_SUCCESS;
}

CORBA_Object
bonobo_directory_get_name_service (CORBA_Environment *ev)
{
	return oaf_name_service_get (ev);
}
