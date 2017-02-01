/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  oaf-async: A library for accessing oafd in a nice way.
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Mathieu Lacage <mathieu@eazel.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "oaf.h"
#include "liboaf.h"
#include "oaf-async.h"
#include "oaf-async-corba.h"
#include "oaf-i18n.h"

/*** App-specific servant structures ***/
typedef struct {

   POA_OAF_ActivationCallback servant;
   PortableServer_POA poa;
   OAFActivationCallback callback;
   gpointer user_data;

} impl_POA_OAF_ActivationCallback;

void
impl_OAF_ActivationCallback__destroy(impl_POA_OAF_ActivationCallback *servant, 
				     CORBA_Environment *ev);



/*** Implementation stub prototypes ***/

static void
impl_OAF_ActivationCallback_report_activation_failed
           (impl_POA_OAF_ActivationCallback * servant, 
	    CORBA_char * reason,
	    CORBA_Environment * ev);

static void
impl_OAF_ActivationCallback_report_activation_succeeded
           (impl_POA_OAF_ActivationCallback * servant, 
	    OAF_ActivationResult * result,
	    CORBA_Environment * ev);

/*** epv structures ***/

static PortableServer_ServantBase__epv impl_OAF_ActivationCallback_base_epv = {
   NULL,			/* _private data */
   NULL,			/* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_OAF_ActivationCallback__epv impl_OAF_ActivationCallback_epv = {
   NULL,			/* _private */
   (gpointer) & impl_OAF_ActivationCallback_report_activation_failed,

   (gpointer) & impl_OAF_ActivationCallback_report_activation_succeeded,

};

/*** vepv structures ***/

static POA_OAF_ActivationCallback__vepv impl_OAF_ActivationCallback_vepv = {
   &impl_OAF_ActivationCallback_base_epv,
   &impl_OAF_ActivationCallback_epv,
};

/*** Stub implementations ***/

CORBA_Object
oaf_async_corba_callback_new (OAFActivationCallback callback,
                              gpointer user_data,
                              CORBA_Environment * ev)
{
   CORBA_Object retval;
   impl_POA_OAF_ActivationCallback *newservant;
   PortableServer_ObjectId *objid;
   PortableServer_POA poa;
   PortableServer_POAManager manager;
   CORBA_ORB orb;

   orb = oaf_orb_get ();

   poa =  (PortableServer_POA) CORBA_ORB_resolve_initial_references (orb, "RootPOA", ev);
   manager = PortableServer_POA__get_the_POAManager (poa, ev);
   PortableServer_POAManager_activate (manager, ev);

   newservant = g_new0(impl_POA_OAF_ActivationCallback, 1);
   newservant->servant.vepv = &impl_OAF_ActivationCallback_vepv;
   newservant->poa = poa;
   newservant->callback = callback;
   newservant->user_data = user_data;

   POA_OAF_ActivationCallback__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

void
impl_OAF_ActivationCallback__destroy(impl_POA_OAF_ActivationCallback *servant, 
				     CORBA_Environment * ev)
{
   PortableServer_ObjectId *objid;

   objid = PortableServer_POA_servant_to_id(servant->poa, servant, ev);
   PortableServer_POA_deactivate_object(servant->poa, objid, ev);
   CORBA_free(objid);

   POA_OAF_ActivationCallback__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

static void
impl_OAF_ActivationCallback_report_activation_failed
   (impl_POA_OAF_ActivationCallback * servant, 
    CORBA_char * reason,
    CORBA_Environment * ev)
{
        char *message;

        if (servant->callback == NULL) {
                return;
        }

        message = g_strconcat ("Activation failed: ", reason, NULL);
        servant->callback (CORBA_OBJECT_NIL, message, servant->user_data);
        g_free (message);

        /* destroy this object */
        impl_OAF_ActivationCallback__destroy (servant, ev);
}

static void
impl_OAF_ActivationCallback_report_activation_succeeded
   (impl_POA_OAF_ActivationCallback * servant, 
    OAF_ActivationResult * result,
    CORBA_Environment * ev)
{
        CORBA_Object retval;

        retval = CORBA_OBJECT_NIL;

        if (servant->callback == NULL) {
                return;
        }

	switch (result->res._d) {
	case OAF_RESULT_SHLIB:
                retval = oaf_server_activate_shlib (result, ev);
		break;
	case OAF_RESULT_OBJECT:
		retval = CORBA_Object_duplicate (result->res._u.res_object, ev);
		break;
	case OAF_RESULT_NONE:
                retval = CORBA_OBJECT_NIL;
                break;
	default:
                g_assert_not_reached ();
		break;
	}

        if (retval == CORBA_OBJECT_NIL) {
                servant->callback (CORBA_OBJECT_NIL,
                                   _("No server corresponding to your query"), 
                                   servant->user_data);
        } else {
                servant->callback (retval, NULL, servant->user_data);
        }

        /* destroy this object */
        impl_OAF_ActivationCallback__destroy (servant, ev);
}
