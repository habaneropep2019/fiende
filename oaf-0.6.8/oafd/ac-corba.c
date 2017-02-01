/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  oafd: OAF CORBA dameon.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 1999, 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors: Elliot Lee <sopwith@redhat.com>,
 *
 */

#include "config.h"

#include <stdio.h>
#include <time.h>

#include "oafd.h"
#include "oaf-i18n.h"
#include "liboaf/liboaf.h"
#include "ac-query-expr.h"
#include "oafd-corba-extensions.h"

#define OAF_LINK_TIME_TO_LIVE 256

typedef struct
{
	OAF_ObjectDirectory obj;

	char *hostname, *username, *domain;

	OAF_ServerInfoList *list;
	OAF_CacheTime time_list_pulled;
	GHashTable *by_iid;

	GHashTable *active_servers;	/* It is assumed that accesses to this
					 * hash table are atomic - i.e. a CORBA 
                                         * call cannot come in while
					 * checking a value in this table */
	OAF_ServerStateCache *active_server_list;
	OAF_CacheTime time_active_pulled;

	guchar locked;
}
ChildODInfo;



/* forward declarations */
char     *ac_aid_to_query_string       (OAF_ActivationID aid);
void      ac_context_to_string_array   (CORBA_Context context, 
                                        char *sort_criteria[4],
                                        CORBA_Environment *ev);




static ChildODInfo *
child_od_info_new (OAF_ObjectDirectory obj, CORBA_Environment * ev)
{
	ChildODInfo *retval;
	char *host, *domain, *user;

	host = OAF_ObjectDirectory__get_hostname (obj, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		goto errhost;

	domain = OAF_ObjectDirectory__get_domain (obj, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		goto errdomain;

	user = OAF_ObjectDirectory__get_username (obj, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		goto erruser;

	retval = g_new0 (ChildODInfo, 1);

	retval->obj = CORBA_Object_duplicate (obj, ev);
	retval->hostname = host;
	retval->username = user;
	retval->domain = domain;

	return retval;

      erruser:
	CORBA_free (domain);
      errdomain:
	CORBA_free (host);
      errhost:
	return NULL;
}

static void
child_od_exception (ChildODInfo * child, CORBA_Environment * ev)
{
	CORBA_Object_release (child->obj, ev);
	child->obj = CORBA_OBJECT_NIL;
}

static void
child_od_update_active (ChildODInfo * child, CORBA_Environment * ev)
{
	int i;
	OAF_ServerStateCache *cache;

	cache = OAF_ObjectDirectory_get_active_servers (child->obj,
							child->time_active_pulled,
							ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
		child_od_exception (child, ev);
		return;
	}

	if (cache->_d) {
		if (child->active_servers) {
			g_hash_table_destroy (child->active_servers);
			CORBA_free (child->active_server_list);
		}

		child->active_server_list = cache;

		child->time_active_pulled = time (NULL);
		child->active_servers =
			g_hash_table_new (g_str_hash, g_str_equal);
		g_hash_table_freeze (child->active_servers);
		for (i = 0; i < cache->_u.active_servers._length; i++)
			g_hash_table_insert (child->active_servers,
					     cache->_u.
					     active_servers._buffer[i],
					     GINT_TO_POINTER (1));
		g_hash_table_thaw (child->active_servers);
	} else
		CORBA_free (cache);
}

static void
child_od_update_list (ChildODInfo * child, CORBA_Environment * ev)
{
	int i;
	OAF_ServerInfoListCache *cache;

	cache = OAF_ObjectDirectory_get_servers (child->obj,
						 child->time_list_pulled, ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
		child->list = NULL;
		child_od_exception (child, ev);
		return;
	}

	if (cache->_d) {
		if (child->by_iid)
			g_hash_table_destroy (child->by_iid);
		if (child->list) {
			CORBA_sequence_set_release (child->list, CORBA_TRUE);
			CORBA_free (child->list);
			child->list = NULL;
		}

		child->list = OAF_ServerInfoList__alloc ();
		*(child->list) = cache->_u.server_list;
		CORBA_sequence_set_release (child->list, CORBA_FALSE);
		CORBA_sequence_set_release (&(cache->_u.server_list),
					    CORBA_FALSE);

		child->time_list_pulled = time (NULL);
		child->by_iid = g_hash_table_new (g_str_hash, g_str_equal);
		g_hash_table_freeze (child->by_iid);
		for (i = 0; i < child->list->_length; i++)
			g_hash_table_insert (child->by_iid,
					     child->list->_buffer[i].iid,
					     &(child->list->_buffer[i]));
		g_hash_table_thaw (child->by_iid);
	}

	CORBA_free (cache);
}

static void
child_od_info_free (ChildODInfo * child, CORBA_Environment * ev)
{
	CORBA_Object_release (child->obj, ev);
	CORBA_free (child->hostname);
	CORBA_free (child->username);
	CORBA_free (child->domain);
	g_free (child);
}


static char *
ac_CORBA_Context_get_value (CORBA_Context         ctx, 
                            const char           *propname,
                            CORBA_Environment    *ev)
{
        return oafd_CORBA_Context_get_value (ctx, 
                                             propname,
                                             ex_OAF_ActivationContext_IncompleteContext,
                                             ev);
}


/*** App-specific servant structures ***/

typedef struct
{
	POA_OAF_ActivationContext servant;
	PortableServer_POA poa;

	GSList *dirs;

	int total_servers;

	gint refs;		/* This is a use count, so we don't accidentally go
				 * updating our server list and invalidating memory */
	OAF_ActivationContext me;
}
impl_POA_OAF_ActivationContext;

/*** Implementation stub prototypes ***/

static OAF_ObjectDirectoryList
	* impl_OAF_ActivationContext__get_directories
	(impl_POA_OAF_ActivationContext * servant, CORBA_Environment * ev);

static void
impl_OAF_ActivationContext_add_directory (impl_POA_OAF_ActivationContext *
					  servant, OAF_ObjectDirectory dir,
					  CORBA_Environment * ev);

static void
impl_OAF_ActivationContext_remove_directory (impl_POA_OAF_ActivationContext *
					     servant, OAF_ObjectDirectory dir,
					     CORBA_Environment * ev);

static OAF_ActivationResult
	*
impl_OAF_ActivationContext_activate (impl_POA_OAF_ActivationContext * servant,
				     CORBA_char * requirements,
				     GNOME_stringlist * selection_order,
				     OAF_ActivationFlags flags,
				     CORBA_Context ctx,
				     CORBA_Environment * ev);

static void
impl_OAF_ActivationContext_activate_async(impl_POA_OAF_ActivationContext *
					  servant, CORBA_char * requirements,
					  GNOME_stringlist * selection_order,
					  OAF_ActivationFlags flags,
					  OAF_ActivationCallback
					  callback_object, CORBA_Context ctx,
					  CORBA_Environment * ev);

static OAF_ServerInfoList
	* impl_OAF_ActivationContext__get_servers
	(impl_POA_OAF_ActivationContext * servant, CORBA_Environment * ev);

static OAF_ServerInfoList
	* impl_OAF_ActivationContext_query (impl_POA_OAF_ActivationContext *
					    servant,
					    CORBA_char * requirements,
					    GNOME_stringlist *
					    selection_order,
					    CORBA_Context ctx,
					    CORBA_Environment * ev);

static OAF_ActivationResult
	* impl_OAF_ActivationContext_activate_from_id
	(impl_POA_OAF_ActivationContext * servant, OAF_ActivationID aid,
	 OAF_ActivationFlags flags, CORBA_Context ctx,
	 CORBA_Environment * ev);

static void
impl_OAF_ActivationContext_activate_from_id_async
   (impl_POA_OAF_ActivationContext * servant, OAF_ActivationID aid,
    OAF_ActivationFlags flags, OAF_ActivationCallback callback_object,
    CORBA_Context ctx, CORBA_Environment * ev);


static void
ac_query_run (impl_POA_OAF_ActivationContext * servant,
	      CORBA_char * requirements,
	      GNOME_stringlist * selection_order,
	      CORBA_Context ctx,
	      OAF_ServerInfo ** items, CORBA_Environment * ev);

static ChildODInfo *ac_find_child_for_server (impl_POA_OAF_ActivationContext *
					      servant,
					      OAF_ServerInfo * server,
					      CORBA_Environment * ev);
static void ac_update_lists (impl_POA_OAF_ActivationContext * servant,
			     CORBA_Environment * ev);

/*** epv structures ***/

static PortableServer_ServantBase__epv impl_OAF_ActivationContext_base_epv = {
	NULL,			/* _private data */
	NULL,			/* finalize routine */
	NULL			/* default_POA routine */
};
static POA_OAF_ActivationContext__epv impl_OAF_ActivationContext_epv = {
	NULL,			/* _private */
	(gpointer) &impl_OAF_ActivationContext__get_directories,
	(gpointer) &impl_OAF_ActivationContext_add_directory,
	(gpointer) &impl_OAF_ActivationContext_remove_directory,
	(gpointer) &impl_OAF_ActivationContext_activate,
        (gpointer) &impl_OAF_ActivationContext_activate_async,
	(gpointer) &impl_OAF_ActivationContext__get_servers,
	(gpointer) &impl_OAF_ActivationContext_query,
	(gpointer) &impl_OAF_ActivationContext_activate_from_id,
        (gpointer) &impl_OAF_ActivationContext_activate_from_id_async
};

/*** vepv structures ***/

static POA_OAF_ActivationContext__vepv impl_OAF_ActivationContext_vepv = {
	&impl_OAF_ActivationContext_base_epv,
	&impl_OAF_ActivationContext_epv
};

/*** Stub implementations ***/

OAF_ActivationContext
OAF_ActivationContext_create (PortableServer_POA poa, CORBA_Environment * ev)
{
	OAF_ActivationContext retval;
	impl_POA_OAF_ActivationContext *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0 (impl_POA_OAF_ActivationContext, 1);
	newservant->servant.vepv = &impl_OAF_ActivationContext_vepv;
	newservant->poa = poa;
	POA_OAF_ActivationContext__init ((PortableServer_Servant) newservant,
					 ev);
	objid = PortableServer_POA_activate_object (poa, newservant, ev);
	CORBA_free (objid);
	retval = newservant->me =
		PortableServer_POA_servant_to_reference (poa, newservant, ev);

	return CORBA_Object_duplicate (retval, ev);
}

static void
ac_update_list (impl_POA_OAF_ActivationContext * servant,
		ChildODInfo * child, CORBA_Environment * ev)
{
	int prev, new;

	if (servant->refs > 0)
		return;

	if (child->list)
		prev = child->list->_length;
	else
		prev = 0;

	child_od_update_list (child, ev);

	if (child->list)
		new = child->list->_length;
	else
		new = 0;

	servant->total_servers += (new - prev);
}

static OAF_ObjectDirectoryList *
impl_OAF_ActivationContext__get_directories (impl_POA_OAF_ActivationContext *
					     servant, CORBA_Environment * ev)
{
	OAF_ObjectDirectoryList *retval;
	int i;
	GSList *cur;

	retval = OAF_ObjectDirectoryList__alloc ();
	retval->_length = g_slist_length (servant->dirs);
	retval->_buffer =
		CORBA_sequence_OAF_ObjectDirectory_allocbuf (retval->_length);

	for (i = 0, cur = servant->dirs; cur; cur = cur->next, i++) {
		ChildODInfo *child;
		child = cur->data;
		retval->_buffer[i] = CORBA_Object_duplicate (child->obj, ev);
	}

	CORBA_sequence_set_release (retval, CORBA_TRUE);

	return retval;
}

static void
impl_OAF_ActivationContext_add_directory (impl_POA_OAF_ActivationContext *
					  servant, OAF_ObjectDirectory dir,
					  CORBA_Environment * ev)
{
	GSList *cur;
	ChildODInfo *new_child;

	for (cur = servant->dirs; cur; cur = cur->next) {
		ChildODInfo *child;
		child = cur->data;
		if (CORBA_Object_is_equivalent (dir, child->obj, ev)) {
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_OAF_ActivationContext_AlreadyListed,
					     NULL);
			return;
		}
	}

	new_child = child_od_info_new (dir, ev);
	if (new_child)
		servant->dirs = g_slist_append (servant->dirs, new_child);
}

static void
impl_OAF_ActivationContext_remove_directory (impl_POA_OAF_ActivationContext *
					     servant, OAF_ObjectDirectory dir,
					     CORBA_Environment * ev)
{
	GSList *cur;

	for (cur = servant->dirs; cur; cur = cur->next) {
		ChildODInfo *child;
		child = cur->data;

		if (CORBA_Object_is_equivalent (dir, child->obj, ev)) {
			if (servant->refs) {
				CORBA_Object_release (child->obj, ev);
				child->obj = CORBA_OBJECT_NIL;
			} else {
				servant->dirs =
					g_slist_remove (servant->dirs, child);
				child_od_info_free (child, ev);
			}
			break;
		}
	}

	if (cur == NULL) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_OAF_ActivationContext_NotListed,
				     NULL);
        }
}

static void
ac_do_activation (impl_POA_OAF_ActivationContext *servant,
		  OAF_ServerInfo *server,
		  OAF_ActivationResult *out,
		  OAF_ActivationFlags flags,
		  const char *hostname,
		  CORBA_Context ctx, 
                  CORBA_Environment *ev)
{
	ChildODInfo *child;
	OAF_ServerInfo *activatable;
	int num_layers;

	/* When doing checks for shlib loadability, we 
         * have to find the info on the factory object in case
	 * a factory is inside a shlib 
         */
	child = ac_find_child_for_server (servant, server, ev);

	if (!child || !child->obj || ev->_major != CORBA_NO_EXCEPTION) {
		OAF_GeneralError *errval = OAF_GeneralError__alloc ();
		errval->description =
			CORBA_string_dup
			(_("Couldn't find which child the server was listed in"));
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_OAF_GeneralError, errval);
		return;
	}

	for (num_layers = 0, activatable = server;
             activatable && activatable->server_type &&
                     !strcmp (activatable->server_type, "factory") &&
             num_layers < OAF_LINK_TIME_TO_LIVE; num_layers++) {

		activatable = g_hash_table_lookup (child->by_iid, activatable->location_info);
	}

	if (activatable == NULL) {		
		OAF_GeneralError *errval = OAF_GeneralError__alloc ();
		errval->description = CORBA_string_dup ("Couldn't find the factory server");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_OAF_GeneralError, errval);
		return;
	} 
	else if (num_layers == OAF_LINK_TIME_TO_LIVE) {
		OAF_GeneralError *errval = OAF_GeneralError__alloc ();
		errval->description = CORBA_string_dup ("Location loop");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_OAF_GeneralError, errval);
		return;
        }

	/* A shared library must be on the same host as the activator
	 * in order for loading to work properly (no, we're not going
	 * to bother with loading a remote shlib into a process - it
	 * gets far too complicated far too quickly :-) 
         */
	
	if (activatable && !strcmp (activatable->server_type, "shlib")
	    && !(flags & OAF_FLAG_NO_LOCAL)
	    && (hostname && !strcmp (activatable->hostname, hostname))) {
		int j;
		char tbuf[512];
		
		out->res._d = OAF_RESULT_SHLIB;		

		/* Here is an explanation as to why we add 2 to num_layers.
		 * At the end of the string list, after all the factory iids are added
		 * to the string list, we then add the iid of the shaed library and the 
		 * location info.  This data is later used in oaf_server_activate_shlib
		 * to activate the component
		 */		 
		out->res._u.res_shlib._length = num_layers + 2;
		out->res._u.res_shlib._buffer = CORBA_sequence_CORBA_string_allocbuf (num_layers + 2);

		/* Copy over factory info */
		for (j = 0, activatable = server; activatable
		     && !strcmp (activatable->server_type, "factory"); j++) {
			out->res._u.res_shlib._buffer[j] = CORBA_string_dup (activatable->iid);
			activatable = g_hash_table_lookup (child->by_iid,
						     	   activatable->location_info);
		}

		/* Copy shlib iid into buffer */
		out->res._u.res_shlib._buffer[j] = CORBA_string_dup (activatable->iid);

		/* Copy location into last buffer slot for use in later activation */
		out->res._u.res_shlib._buffer[j+1] = CORBA_string_dup (activatable->location_info);
		
		g_snprintf (tbuf, sizeof (tbuf), "OAFAID:[%s,%s,%s,%s]",
			    activatable->iid,
			    activatable->username,
			    activatable->hostname, activatable->domain);
		out->aid = CORBA_string_dup (tbuf);
	} else {
		CORBA_Object retval;

		retval =
			OAF_ObjectDirectory_activate (child->obj, server->iid,
						      servant->me, flags, ctx,
						      ev);
		if (ev->_major == CORBA_NO_EXCEPTION) {
			char tbuf[512];
			out->res._d = OAF_RESULT_OBJECT;
			out->res._u.res_object = retval;
			g_snprintf (tbuf, sizeof (tbuf),
				    "OAFAID:[%s,%s,%s,%s]", activatable->iid,
				    activatable->username,
				    activatable->hostname,
				    activatable->domain);
			out->aid = CORBA_string_dup (tbuf);
		}
	}
}


static OAF_ActivationResult *
impl_OAF_ActivationContext_activate (impl_POA_OAF_ActivationContext * servant,
				     CORBA_char * requirements,
				     GNOME_stringlist * selection_order,
				     OAF_ActivationFlags flags,
				     CORBA_Context ctx,
				     CORBA_Environment * ev)
{
	OAF_ActivationResult *retval = NULL;
	OAF_ServerInfo **items, *curitem;
	int i;
	char *hostname;

	hostname = ac_CORBA_Context_get_value (ctx, "hostname", ev);
	ac_update_lists (servant, ev);

	servant->refs++;

	items = oaf_alloca (servant->total_servers *
			    sizeof (OAF_ServerInfo *));
	ac_query_run (servant, requirements, selection_order, ctx, items, ev);

	if (ev->_major != CORBA_NO_EXCEPTION) {
		goto out;
        }

	retval = OAF_ActivationResult__alloc ();
	retval->res._d = OAF_RESULT_NONE;

        if (items[0] == NULL) {
		OAF_GeneralError *errval = OAF_GeneralError__alloc ();
		errval->description =
			CORBA_string_dup
			(_("Nothing matched the requirements."));
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_OAF_GeneralError, errval);
                goto out;
        }

	for (i = 0; (retval->res._d == OAF_RESULT_NONE) && items[i] != NULL
	     && (i < servant->total_servers); i++) {
		curitem = items[i];

		ac_do_activation (servant, curitem, retval, flags, hostname,
				  ctx, ev);
                if (ev->_major != CORBA_NO_EXCEPTION) {
                        goto out;
                }
	}
        

	if (retval->res._d == OAF_RESULT_NONE)
		retval->aid = CORBA_string_dup ("");

      out:
	g_free (hostname);

	servant->refs--;

	return retval;
}


static void
impl_OAF_ActivationContext_activate_async(impl_POA_OAF_ActivationContext *
					  servant, CORBA_char * requirements,
					  GNOME_stringlist * selection_order,
					  OAF_ActivationFlags flags,
					  OAF_ActivationCallback callback_object, 
                                          CORBA_Context ctx,
					  CORBA_Environment * ev)
{
	OAF_ActivationResult *retval = NULL;
	OAF_ServerInfo **items, *curitem;
	int i;
	char *hostname;

	hostname = ac_CORBA_Context_get_value (ctx, "hostname", ev);
	ac_update_lists (servant, ev);

	servant->refs++;

	items = oaf_alloca (servant->total_servers *
			    sizeof (OAF_ServerInfo *));
	ac_query_run (servant, requirements, selection_order, ctx, items, ev);

	if (ev->_major != CORBA_NO_EXCEPTION) {
                char *message;
                g_free (hostname);
                servant->refs--;
                
                message = g_strconcat(_("Query failed: "), CORBA_exception_id (ev), NULL);
                OAF_ActivationCallback_report_activation_failed (callback_object,
                                                                 message, ev);
                g_free (message);
                return;
        }

	retval = OAF_ActivationResult__alloc ();
	retval->res._d = OAF_RESULT_NONE;

	for (i = 0; (retval->res._d == OAF_RESULT_NONE) && items[i]
	     && (i < servant->total_servers); i++) {
		curitem = items[i];

		ac_do_activation (servant, curitem, retval, flags, hostname,
				  ctx, ev);
	}

	if (retval->res._d == OAF_RESULT_NONE)
		retval->aid = CORBA_string_dup ("");


        if (ev->_major != CORBA_NO_EXCEPTION) {
                char *message;
                g_free (hostname);
                CORBA_free (retval);
                servant->refs--;
                
                message = g_strconcat(_("Activation failed: "), CORBA_exception_id (ev), NULL);
                OAF_ActivationCallback_report_activation_failed (callback_object,
                                                                 message, ev);
                g_free (message);
                return;
        }

	g_free (hostname);

        /* return the correct value back to the client */
        OAF_ActivationCallback_report_activation_succeeded (callback_object, retval, ev);
}


static void
ac_update_lists (impl_POA_OAF_ActivationContext * servant,
		 CORBA_Environment * ev)
{
	GSList *cur;

	if (servant->refs > 0)
		return;

	for (cur = servant->dirs; cur; cur = cur->next)
		ac_update_list (servant, cur->data, ev);
}

static OAF_ServerInfoList *
impl_OAF_ActivationContext__get_servers (impl_POA_OAF_ActivationContext *
					 servant, CORBA_Environment * ev)
{
	OAF_ServerInfoList *retval;
	GSList *cur;
	int i;
	int total;

	ac_update_lists (servant, ev);

	total = servant->total_servers;

	retval = OAF_ServerInfoList__alloc ();
	retval->_length = total;
	retval->_buffer = CORBA_sequence_OAF_ServerInfo_allocbuf (total);
	CORBA_sequence_set_release (retval, CORBA_TRUE);

	for (i = 0; i < total;) {
		for (cur = servant->dirs; cur; cur = cur->next) {
			ChildODInfo *child;
			int j;

			child = cur->data;

			for (j = 0; j < child->list->_length; j++, i++)
				OAF_ServerInfo_copy (&retval->_buffer[i],
						     &child->
						     list->_buffer[j]);
		}
	}

	return retval;
}

static ChildODInfo *
ac_find_child_for_server (impl_POA_OAF_ActivationContext * servant,
			  OAF_ServerInfo * server, CORBA_Environment * ev)
{
	GSList *cur;

	for (cur = servant->dirs; cur; cur = cur->next) {
		ChildODInfo *child = cur->data;

		if (CORBA_Object_is_nil (child->obj, ev) || !child->list)
			continue;

		if ((server >= child->list->_buffer)
		    && (server <
			(child->list->_buffer +
			 child->list->_length))) return child;
	}

	return NULL;
}

static QueryExprConst
ac_query_get_var (OAF_ServerInfo * si, const char *id, QueryContext * qctx)
{
	ChildODInfo *child;
	QueryExprConst retval;

	retval.value_known = FALSE;
	retval.needs_free = FALSE;

	child = ac_find_child_for_server (qctx->user_data, si, NULL);
	if (!child)
		goto out;

	if (!strcasecmp (id, "_active")) {
		CORBA_Environment ev;

		CORBA_exception_init (&ev);
		child_od_update_active (child, &ev);
		CORBA_exception_free (&ev);

		retval.value_known = TRUE;
		retval.type = CONST_BOOLEAN;
		retval.u.v_boolean =
			g_hash_table_lookup (child->active_servers,
					     si->iid) ? TRUE : FALSE;
	}

      out:

	return retval;
}

/* This function should only be called by
 * impl_OAF_ActivationContext_query and
 * impl_OAF_ActivationContext_activate - hairy implicit preconditions
 * exist. */
static void
ac_query_run (impl_POA_OAF_ActivationContext * servant,
	      CORBA_char * requirements,
	      GNOME_stringlist * selection_order,
	      CORBA_Context ctx,
	      OAF_ServerInfo ** items, CORBA_Environment * ev)
{
	int total, i;
	GSList *cur;
	QueryContext qctx;

	OAF_ServerInfo **orig_items;
	int item_count, orig_item_count;
	char *errstr;
	OAF_ActivationContext_ParseFailed *ex;

	QueryExpr *qexp_requirements;
	QueryExpr **qexp_sort_items;

	/* First, parse the query */
	errstr = (char *) qexp_parse (requirements, &qexp_requirements);
	if (errstr) {
		puts (errstr);

		g_strstrip (errstr);
		ex = OAF_ActivationContext_ParseFailed__alloc ();
		ex->description = CORBA_string_dup (errstr);
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_OAF_ActivationContext_ParseFailed,
				     ex);
		return;
	}

	qexp_sort_items =
		oaf_alloca (selection_order->_length * sizeof (QueryExpr *));
	for (i = 0; i < selection_order->_length; i++) {
		errstr =
			(char *) qexp_parse (selection_order->_buffer[i],
					     &qexp_sort_items[i]);

		if (errstr) {
			qexp_free (qexp_requirements);
			for (i--; i >= 0; i--)
				qexp_free (qexp_sort_items[i]);

			g_strstrip (errstr);
			ex = OAF_ActivationContext_ParseFailed__alloc ();
			ex->description = CORBA_string_dup (errstr);

			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_OAF_ActivationContext_ParseFailed,
					     ex);
			return;
		}
	}

	total = servant->total_servers;
	orig_items = oaf_alloca (total * sizeof (OAF_ServerInfo *));

	for (item_count = 0, cur = servant->dirs; cur; cur = cur->next) {
		ChildODInfo *child;
		int i;

		child = cur->data;

		if (child->obj == CORBA_OBJECT_NIL)
			continue;

		for (i = 0; i < child->list->_length; i++, item_count++)
			items[item_count] = &child->list->_buffer[i];
	}

	memcpy (orig_items, items, item_count * sizeof (OAF_ServerInfo *));
	orig_item_count = item_count;

	qctx.sil = orig_items;
	qctx.nservers = orig_item_count;
	qctx.cctx = ctx;
	qctx.id_evaluator = ac_query_get_var;
	qctx.user_data = servant;

	for (i = 0; i < item_count; i++) {
		if (!qexp_matches (items[i], qexp_requirements, &qctx))
			items[i] = NULL;
	}

	qexp_sort (items, item_count, qexp_sort_items,
		   selection_order->_length, &qctx);

        qexp_free (qexp_requirements);
        for (i = 0; i < selection_order->_length; i++)
                qexp_free (qexp_sort_items[i]);
}

static OAF_ServerInfoList *
impl_OAF_ActivationContext_query (impl_POA_OAF_ActivationContext * servant,
				  CORBA_char * requirements,
				  GNOME_stringlist * selection_order,
				  CORBA_Context ctx, CORBA_Environment * ev)
{
	OAF_ServerInfoList *retval;
	OAF_ServerInfo **items;
	int item_count;
	int i, j, total;

	retval = OAF_ServerInfoList__alloc ();
	retval->_length = 0;
	retval->_buffer = NULL;
	CORBA_sequence_set_release (retval, CORBA_TRUE);

	/* Pull in new lists from OD servers */
	ac_update_lists (servant, ev);
	servant->refs++;

	items =
		oaf_alloca (servant->total_servers *
			    sizeof (OAF_ServerInfo *));
	item_count = servant->total_servers;

	ac_query_run (servant, requirements, selection_order, ctx, items, ev);

	if (ev->_major == CORBA_NO_EXCEPTION) {
		for (total = i = 0; i < item_count; i++) {
			if (items[i])
				total++;
		}

		retval->_length = total;
		retval->_buffer =
			CORBA_sequence_OAF_ServerInfo_allocbuf (total);

		for (i = j = 0; i < item_count; i++) {
			if (!items[i])
				continue;

			OAF_ServerInfo_copy (&retval->_buffer[j], items[i]);

			j++;
		}
	}

	servant->refs--;

	return retval;
}

char *
ac_aid_to_query_string (OAF_ActivationID aid)
{
        char *tmp_aid;
        char *requirements;
        char *iid_requirement;
        char *username_requirement;
        char *hostname_requirement;
        char *domain_requirement;
	OAFActivationInfo *ainfo;

        /* FIXME bugzilla.eazel.com 4659: this is completely broken.
		   We activate by AID, not IID. */
        if (strncmp ("OAFIID:", aid, 7) == 0) {
                tmp_aid = g_strconcat ("OAFAID:[", aid, "]", NULL);
                ainfo = oaf_actid_parse (tmp_aid);
                g_free (tmp_aid);
        } else {
                ainfo = oaf_actid_parse (aid);
        }

	if (ainfo == NULL) {
                return NULL;
	}

        iid_requirement = g_strconcat ("iid == \'", ainfo->iid, "\' ", NULL);

        if (ainfo->user) {
                username_requirement = g_strconcat ("AND username == \'", ainfo->user, "\'", NULL);
        } else {
                username_requirement = g_strdup ("");
        }
        
        if (ainfo->host) {
                hostname_requirement = g_strconcat ("AND hostname == \'", ainfo->host, "\'", NULL);
        } else {
                hostname_requirement = g_strdup ("");
        }
        
        if (ainfo->domain) {
                domain_requirement = g_strconcat ("AND domain == \'", ainfo->domain, "\'", NULL);
        } else {
                domain_requirement = g_strdup ("");
        }

        requirements = g_strconcat (iid_requirement, username_requirement, 
                                    hostname_requirement, domain_requirement, NULL);

        g_free (iid_requirement);
        g_free (username_requirement);
        g_free (hostname_requirement);
        g_free (domain_requirement);
        oaf_actinfo_free (ainfo);

        return requirements;
}

void
ac_context_to_string_array (CORBA_Context context, char **sort_criteria, CORBA_Environment *ev)
{
	char *context_username;
	char *context_hostname;
	char *context_domain;

        context_username = ac_CORBA_Context_get_value (context, "username", ev);
        context_hostname = ac_CORBA_Context_get_value (context, "hostname", ev);
        context_domain = ac_CORBA_Context_get_value (context, "domain", ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
                return;
        }
        
        sort_criteria[0] = g_strconcat ("username == \'", context_username, "\'", NULL);
        sort_criteria[1] = g_strconcat ("hostname == \'", context_hostname, "\'", NULL);
        sort_criteria[2] = g_strconcat ("session == \'", context_domain, "\'", NULL);
        sort_criteria[3] = NULL;

        g_free (context_username);
        g_free (context_hostname);
        g_free (context_domain);
}


static OAF_ActivationResult *
impl_OAF_ActivationContext_activate_from_id (impl_POA_OAF_ActivationContext *servant, 
                                             OAF_ActivationID aid,
					     OAF_ActivationFlags flags,
					     CORBA_Context ctx,
					     CORBA_Environment *ev)
{
	OAF_ActivationResult *retval;
        char *requirements;
        char *sort_criteria[4];
        GNOME_stringlist selection_order;
        OAF_ActivationContext_ParseFailed *ex;

	ac_update_lists (servant, ev);
        if (ev->_major != CORBA_NO_EXCEPTION) {
                return CORBA_OBJECT_NIL;
        }

	servant->refs++;

        requirements = ac_aid_to_query_string (aid);
        if (requirements == NULL) {
                servant->refs--;
                ex = OAF_ActivationContext_ParseFailed__alloc ();
		ex->description = CORBA_string_dup ("Invalid AID or IID");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_OAF_ActivationContext_ParseFailed,
				     ex);
                return CORBA_OBJECT_NIL;
        }

        ac_context_to_string_array (ctx, sort_criteria, ev);
        if (ev->_major != CORBA_NO_EXCEPTION) {
                servant->refs--;            
                return CORBA_OBJECT_NIL;
        }

        selection_order._length = 3;
        selection_order._buffer = sort_criteria;
        CORBA_sequence_set_release (&selection_order, CORBA_FALSE);

        retval = impl_OAF_ActivationContext_activate (servant,
                                                      requirements,
                                                      &selection_order,
                                                      flags,
                                                      ctx, ev);        
        g_free (sort_criteria[0]);
        g_free (sort_criteria[1]);
        g_free (sort_criteria[2]);
        g_free (requirements);

        servant->refs--;

        return retval;
}


static void
impl_OAF_ActivationContext_activate_from_id_async
                (impl_POA_OAF_ActivationContext * servant, 
                 OAF_ActivationID aid,
                 OAF_ActivationFlags flags, 
                 OAF_ActivationCallback callback_object,
                 CORBA_Context ctx, 
                 CORBA_Environment * ev)
{
	OAF_ActivationResult *retval;
        char *requirements;
        char *sort_criteria[4];
        GNOME_stringlist selection_order;

	ac_update_lists (servant, ev);

	servant->refs++;

        requirements = ac_aid_to_query_string (aid);
        if (requirements == NULL) {
                servant->refs--;
                OAF_ActivationCallback_report_activation_failed (callback_object,
                                                                 _("Could not parse AID"), ev);
                return;
        }


        ac_context_to_string_array (ctx, sort_criteria, ev);
        if (ev->_major != CORBA_NO_EXCEPTION) {
                char *message;
		g_free (requirements);
                servant->refs--;
                message = g_strconcat (_("Could not parse context: "), CORBA_exception_id (ev), NULL);
                OAF_ActivationCallback_report_activation_failed (callback_object,
                                                                 message, ev);
                g_free (message);
                return;
        }

        selection_order._length = 3;
        selection_order._buffer = sort_criteria;
        CORBA_sequence_set_release (&selection_order, CORBA_FALSE);

        retval = impl_OAF_ActivationContext_activate (servant,
                                                      requirements,
                                                      &selection_order,
                                                      flags,
                                                      ctx, ev);
        g_free (sort_criteria[0]);
        g_free (sort_criteria[1]);
        g_free (sort_criteria[2]);
        g_free (requirements);

        if (ev->_major != CORBA_NO_EXCEPTION) {
                char *message;
                servant->refs--;
                message = g_strconcat (_("Could not activate server: "), CORBA_exception_id (ev), NULL);
                OAF_ActivationCallback_report_activation_failed (callback_object,
                                                                 message, ev);
                g_free (message);
                return;
        }

        servant->refs--;

        OAF_ActivationCallback_report_activation_succeeded (callback_object,
                                                            retval, ev);
}



