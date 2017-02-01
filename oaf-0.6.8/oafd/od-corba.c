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

#include "oafd.h"
#include "oaf-i18n.h"
#include "liboaf/liboaf.h"
#include <time.h>
#include <glib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "oafd-corba-extensions.h"

/*** App-specific servant structures ***/

typedef struct
{
	POA_OAF_ObjectDirectory servant;
	PortableServer_POA poa;
	OAF_ServerInfoList attr_servers;
	OAF_CacheTime time_list_changed;

	GHashTable *by_iid;

	CORBA_char *attr_domain;
	const char *attr_hostID;

	guchar is_locked;

	GHashTable *active_servers;
	OAF_CacheTime time_active_changed;

        char **registry_source_directories;
        time_t time_did_stat;
        GHashTable *registry_directory_mtimes;

	CORBA_Object self;
}
impl_POA_OAF_ObjectDirectory;

/*** Implementation stub prototypes ***/

static OAF_ServerInfoListCache
	*
impl_OAF_ObjectDirectory__get_servers (impl_POA_OAF_ObjectDirectory * servant,
				       OAF_CacheTime only_if_newer,
				       CORBA_Environment * ev);

static OAF_ServerStateCache
	* impl_OAF_ObjectDirectory_get_active_servers
	(impl_POA_OAF_ObjectDirectory * servant, OAF_CacheTime only_if_newer,
	 CORBA_Environment * ev);

static CORBA_char
	* impl_OAF_ObjectDirectory__get_domain (impl_POA_OAF_ObjectDirectory *
						servant,
						CORBA_Environment * ev);
static CORBA_char
	* impl_OAF_ObjectDirectory__get_hostID (impl_POA_OAF_ObjectDirectory *
						servant,
						CORBA_Environment * ev);
static CORBA_char
	* impl_OAF_ObjectDirectory__get_username (impl_POA_OAF_ObjectDirectory
						  * servant,
						  CORBA_Environment * ev);

static CORBA_Object
impl_OAF_ObjectDirectory_activate (impl_POA_OAF_ObjectDirectory * servant,
				   OAF_ImplementationID iid,
				   OAF_ActivationContext ac,
				   OAF_ActivationFlags flags,
				   CORBA_Context ctx, CORBA_Environment * ev);

static void
impl_OAF_ObjectDirectory_lock (impl_POA_OAF_ObjectDirectory * servant,
			       CORBA_Environment * ev);

static void
impl_OAF_ObjectDirectory_unlock (impl_POA_OAF_ObjectDirectory * servant,
				 CORBA_Environment * ev);

static OAF_RegistrationResult
impl_OAF_ObjectDirectory_register (impl_POA_OAF_ObjectDirectory * servant,
				   OAF_ImplementationID iid,
				   CORBA_Object obj, CORBA_Environment * ev);

static void
impl_OAF_ObjectDirectory_unregister (impl_POA_OAF_ObjectDirectory * servant,
				     OAF_ImplementationID iid,
				     CORBA_Object obj,
				     OAF_ObjectDirectory_UnregisterType notify,
				     CORBA_Environment * ev);

/*** epv structures ***/

static PortableServer_ServantBase__epv impl_OAF_ObjectDirectory_base_epv = {
	NULL,			/* _private data */
	NULL,			/* finalize routine */
	NULL			/* default_POA routine */
};
static POA_OAF_ObjectDirectory__epv impl_OAF_ObjectDirectory_epv = {
	NULL,			/* _private */
	(gpointer) &impl_OAF_ObjectDirectory__get_servers,
	(gpointer) &impl_OAF_ObjectDirectory_get_active_servers,
	(gpointer) &impl_OAF_ObjectDirectory__get_username,
	(gpointer) &impl_OAF_ObjectDirectory__get_hostID,
	(gpointer) &impl_OAF_ObjectDirectory__get_domain,
	(gpointer) &impl_OAF_ObjectDirectory_activate,
	(gpointer) &impl_OAF_ObjectDirectory_lock,
	(gpointer) &impl_OAF_ObjectDirectory_unlock,
	(gpointer) &impl_OAF_ObjectDirectory_register,
	(gpointer) &impl_OAF_ObjectDirectory_unregister
};

/*** vepv structures ***/

static POA_OAF_ObjectDirectory__vepv impl_OAF_ObjectDirectory_vepv = {
	&impl_OAF_ObjectDirectory_base_epv,
	&impl_OAF_ObjectDirectory_epv
};

/*** Stub implementations ***/

#ifdef OAF_DEBUG
static void
od_dump_list (impl_POA_OAF_ObjectDirectory * od)
{
	int i, j, k;

	for (i = 0; i < od->attr_servers._length; i++) {
		g_print ("IID %s, type %s, location %s\n",
			 od->attr_servers._buffer[i].iid,
			 od->attr_servers._buffer[i].server_type,
			 od->attr_servers._buffer[i].location_info);
		for (j = 0; j < od->attr_servers._buffer[i].props._length;
		     j++) {
			OAF_Property *prop =
				&(od->attr_servers._buffer[i].
				  props._buffer[j]);
			g_print ("    %s = ", prop->name);
			switch (prop->v._d) {
			case OAF_P_STRING:
				g_print ("\"%s\"\n", prop->v._u.value_string);
				break;
			case OAF_P_NUMBER:
				g_print ("%f\n", prop->v._u.value_number);
				break;
			case OAF_P_BOOLEAN:
				g_print ("%s\n",
					 prop->v.
					 _u.value_boolean ? "TRUE" : "FALSE");
				break;
			case OAF_P_STRINGV:
				g_print ("[");
				for (k = 0;
				     k < prop->v._u.value_stringv._length;
				     k++) {
					g_print ("\"%s\"",
						 prop->v._u.
						 value_stringv._buffer[k]);
					if (k <
					    (prop->v._u.
					     value_stringv._length - 1))
						g_print (", ");
				}
				g_print ("]\n");
				break;
			}
		}
	}
}
#endif

static gboolean
registry_directory_needs_update (impl_POA_OAF_ObjectDirectory *servant,
                                 const char *directory)
{
        struct stat statbuf;
        time_t old_mtime;

        if (stat (directory, &statbuf) != 0) {
                return FALSE;
        }
 
        old_mtime = (time_t) g_hash_table_lookup (servant->registry_directory_mtimes,
                                                  directory);

        g_hash_table_insert (servant->registry_directory_mtimes,
                             (gpointer) directory,
                             (gpointer) statbuf.st_mtime);

        return (old_mtime != statbuf.st_mtime);
}

static void
update_registry (impl_POA_OAF_ObjectDirectory *servant)
{
        gboolean must_load;
        int i;

        /* FIXME bugzilla.eazel.com 2727: we should only reload those directories that have
         * actually changed instead of reloading all when any has
         * changed. 
         */

        must_load = FALSE;

        /* Don't stat more than once every 5 seconds or activation
           could be too slow. This works even on the first read
           because then `time_read is 0' */
        if (time (NULL) - 5 > servant->time_did_stat) {
                for (i = 0; servant->registry_source_directories[i] != NULL; i++) {
                        if (registry_directory_needs_update 
                            (servant, servant->registry_source_directories[i])) {
                                must_load = TRUE;
                                break;
                        }
                }

                servant->time_did_stat = time (NULL);
        }

        if (must_load) {
                OAF_ServerInfo_load (servant->registry_source_directories,
                                     &servant->attr_servers,
                                     &servant->by_iid,
                                     servant->attr_hostID,
                                     servant->attr_domain);

                servant->time_list_changed = time (NULL);

#ifdef OAF_DEBUG
                od_dump_list (servant);
#endif
        }
}

OAF_ObjectDirectory
OAF_ObjectDirectory_create (PortableServer_POA poa,
			    const char *domain,
			    const char *registry_path,
			    CORBA_Environment * ev)
{
	OAF_ObjectDirectory retval;
	impl_POA_OAF_ObjectDirectory *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0 (impl_POA_OAF_ObjectDirectory, 1);
	newservant->servant.vepv = &impl_OAF_ObjectDirectory_vepv;
	newservant->poa = poa;
	POA_OAF_ObjectDirectory__init ((PortableServer_Servant) newservant,
				       ev);
	objid = PortableServer_POA_activate_object (poa, newservant, ev);
	CORBA_free (objid);
	newservant->self = retval =
		PortableServer_POA_servant_to_reference (poa, newservant, ev);

	newservant->attr_domain = g_strdup (domain);
	newservant->attr_hostID = oaf_hostname_get ();
	newservant->by_iid = NULL;

        newservant->registry_source_directories = g_strsplit (registry_path, ":", -1);
        newservant->registry_directory_mtimes = g_hash_table_new (g_str_hash, g_str_equal);

        update_registry (newservant);

        newservant->active_servers =
                g_hash_table_new (g_str_hash, g_str_equal);

	return CORBA_Object_duplicate (retval, ev);
}

static OAF_ServerInfoListCache *
impl_OAF_ObjectDirectory__get_servers (impl_POA_OAF_ObjectDirectory * servant,
				       OAF_CacheTime only_if_newer,
				       CORBA_Environment * ev)
{
	OAF_ServerInfoListCache *retval;

        update_registry (servant);

	retval = OAF_ServerInfoListCache__alloc ();

	retval->_d = (only_if_newer < servant->time_list_changed);
	if (retval->_d) {
		retval->_u.server_list = servant->attr_servers;
		CORBA_sequence_set_release (&retval->_u.server_list,
					    CORBA_FALSE);
	}

	return retval;
}

typedef struct
{
	OAF_ImplementationID *seq;
	int last_used;
}
StateCollectionInfo;

static void
add_active_server (char *key, gpointer value, StateCollectionInfo * sci)
{
	sci->seq[(sci->last_used)++] = CORBA_string_dup (key);
}

static OAF_ServerStateCache *
impl_OAF_ObjectDirectory_get_active_servers (impl_POA_OAF_ObjectDirectory *
					     servant,
					     OAF_CacheTime only_if_newer,
					     CORBA_Environment * ev)
{
	OAF_ServerStateCache *retval;

	retval = OAF_ServerStateCache__alloc ();

	retval->_d = (only_if_newer < servant->time_active_changed);
	if (retval->_d) {
		StateCollectionInfo sci;

		retval->_u.active_servers._length =
			g_hash_table_size (servant->active_servers);
		retval->_u.active_servers._buffer = sci.seq =
			CORBA_sequence_OAF_ImplementationID_allocbuf
			(retval->_u.active_servers._length);
		sci.last_used = 0;

		g_hash_table_foreach (servant->active_servers,
				      (GHFunc) add_active_server, &sci);
		CORBA_sequence_set_release (&(retval->_u.active_servers),
					    CORBA_TRUE);
	}

	return retval;
}

static CORBA_char *
impl_OAF_ObjectDirectory__get_domain (impl_POA_OAF_ObjectDirectory * servant,
				      CORBA_Environment * ev)
{
	return CORBA_string_dup (servant->attr_domain);
}

static CORBA_char *
impl_OAF_ObjectDirectory__get_hostID (impl_POA_OAF_ObjectDirectory * servant,
				      CORBA_Environment * ev)
{
	return CORBA_string_dup (servant->attr_hostID);
}

static CORBA_char *
impl_OAF_ObjectDirectory__get_username (impl_POA_OAF_ObjectDirectory *
					servant, CORBA_Environment * ev)
{
	return CORBA_string_dup (g_get_user_name ());
}



static CORBA_Object 
od_get_active_server (impl_POA_OAF_ObjectDirectory * servant,
                       OAF_ImplementationID iid,
                       CORBA_Context ctx, CORBA_Environment * ev)
{
	CORBA_Object retval;
        char *display;
        char *display_iid;

        display = oafd_CORBA_Context_get_value (ctx, "display", NULL, ev);
        
        if (display != NULL) {
                display_iid = g_strconcat (display, ",", iid, NULL);
                
                retval = g_hash_table_lookup (servant->active_servers, display_iid);

		g_free (display);
                g_free (display_iid);
                
                if (!CORBA_Object_is_nil (retval, ev)
                    && !CORBA_Object_non_existent (retval, ev)) {
                        return CORBA_Object_duplicate (retval, ev);
                }
        }

        retval = g_hash_table_lookup (servant->active_servers, iid);
        
        if (!CORBA_Object_is_nil (retval, ev)
            && !CORBA_Object_non_existent (retval, ev)) {
                return CORBA_Object_duplicate (retval, ev);
        }

        return CORBA_OBJECT_NIL;
}

static CORBA_Object
impl_OAF_ObjectDirectory_activate (impl_POA_OAF_ObjectDirectory * servant,
				   OAF_ImplementationID iid,
				   OAF_ActivationContext ac,
				   OAF_ActivationFlags flags,
				   CORBA_Context ctx, CORBA_Environment * ev)
{
	CORBA_Object retval;
	OAF_ServerInfo *si;
	ODActivationInfo ai;
        OAF_GeneralError *errval;
        char *error_description;
        CORBA_Environment retry_ev;

	retval = CORBA_OBJECT_NIL;

        update_registry (servant);

        if (!(flags & OAF_FLAG_PRIVATE)) {
                retval = od_get_active_server (servant, iid, ctx, ev);

                if (retval != CORBA_OBJECT_NIL) {
                        return retval;
                }
        }

	if (flags & OAF_FLAG_EXISTING_ONLY) {
		return CORBA_OBJECT_NIL;
        }

	ai.ac = ac;
	ai.flags = flags;
	ai.ctx = ctx;

	si = g_hash_table_lookup (servant->by_iid, iid);

	if (si != NULL) {
		retval = od_server_activate (si, &ai, servant->self, ev);
                /* If we failed to activate - it may be because our
                 * request re-entered _during_ the activation
                 * process resulting in a second process being started
                 * but failing to register - so we'll look up again here
                 * to see if we can get it.
                 * FIXME: we should not be forking redundant processes
                 * while an activation of that same process is on the
                 * stack.
                 * FIXME: we only get away with this hack because we
                 * try and fork another process & thus allow the reply
                 * from the initial process to be handled in the event
                 * loop.
                 */

                if (ev->_major != CORBA_NO_EXCEPTION) {
                        CORBA_exception_init (&retry_ev);

                        retval = od_get_active_server (servant, iid, ctx, &retry_ev);

                        CORBA_exception_free (&retry_ev);

                        if (retval != CORBA_OBJECT_NIL) {
                                CORBA_exception_free (ev);
                        }
                }
        } else {
                errval = OAF_GeneralError__alloc ();

                error_description = 
                        g_strdup_printf (_("Couldn't find activation record for server `%s'. The likely cause is a missing or incorrectly installed .oaf file."), 
                                           iid);
		errval->description =
			CORBA_string_dup
			(error_description);
                g_free (error_description); 

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_OAF_GeneralError, errval);
        }

	return retval;
}

static void
impl_OAF_ObjectDirectory_lock (impl_POA_OAF_ObjectDirectory * servant,
			       CORBA_Environment * ev)
{
	while (servant->is_locked)
		g_main_iteration (TRUE);

	servant->is_locked = TRUE;
}

static void
impl_OAF_ObjectDirectory_unlock (impl_POA_OAF_ObjectDirectory * servant,
				 CORBA_Environment * ev)
{
	g_return_if_fail (servant->is_locked);

	servant->is_locked = FALSE;
}

static OAF_RegistrationResult
impl_OAF_ObjectDirectory_register (impl_POA_OAF_ObjectDirectory * servant,
				   OAF_ImplementationID iid,
				   CORBA_Object obj, CORBA_Environment * ev)
{
	CORBA_Object oldobj;
        OAF_ImplementationID actual_iid;

        actual_iid = strrchr (iid, ',');

        if (actual_iid == NULL) {
                actual_iid = iid;
        } else {
                actual_iid++;
        }


	oldobj = g_hash_table_lookup (servant->active_servers, iid);

	if (!CORBA_Object_is_nil (oldobj, ev)) {
		if (!CORBA_Object_non_existent (oldobj, ev))
			return OAF_REG_ALREADY_ACTIVE;
		else
			CORBA_Object_release (oldobj, ev);
	}


	if (!g_hash_table_lookup (servant->by_iid, actual_iid))
		return OAF_REG_NOT_LISTED;

	g_hash_table_insert (servant->active_servers,
			     oldobj ? iid : g_strdup (iid),
			     CORBA_Object_duplicate (obj, ev));
	servant->time_active_changed = time (NULL);

	return OAF_REG_SUCCESS;
}

static void
impl_OAF_ObjectDirectory_unregister (impl_POA_OAF_ObjectDirectory * servant,
				     OAF_ImplementationID iid,
				     CORBA_Object obj,
				     OAF_ObjectDirectory_UnregisterType notify,
				     CORBA_Environment * ev)
{
	char *orig_iid;
	CORBA_Object orig_obj;

	if (!g_hash_table_lookup_extended
	    (servant->active_servers, iid, (gpointer *) & orig_iid,
	     (gpointer *) & orig_obj)
	    || !CORBA_Object_is_equivalent (orig_obj, obj, ev))
		return;

	g_hash_table_remove (servant->active_servers, iid);

	g_free (orig_iid);
	CORBA_Object_release (orig_obj, ev);

	servant->time_active_changed = time (NULL);
}
