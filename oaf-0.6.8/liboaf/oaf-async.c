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
 */

#include <config.h>

#include "oaf-async.h"
#include "oaf-async-corba.h"
#include "liboaf/oaf-activate.h"
#include "liboaf/oaf-activate-private.h"
#include "liboaf/liboaf-private.h"


/**
 * oaf_activate_async:
 * @requirements: the OAF query string.
 * @selection_order: preference array.
 * @flags: activation flags.
 * @callback: callback function.
 * @user_data: data to be poassed to the callback function.
 * @ev: exception structure.
 *
 * This function will asynchronously try to activate a component 
 * given the @requirements query string. When the component is 
 * activated or when the activation fails, it will call @callback 
 * with the given @user_data data as parameter.
 * callback will be called with a CORBA_OBJECT_NIL object if the 
 * activation fails. If the activation fails, the callback will be 
 * given a human-readable string containing a description of the 
 * error. In case of sucess, the error string value is undefined.
 *
 * @selection_order can be safely NULLed as well as @ev and 
 * @user_data. @flags can be set to 0 if you do not know what to 
 * use.
 */
void oaf_activate_async (const char *requirements,
			 char *const *selection_order,
			 OAF_ActivationFlags flags,
			 OAFActivationCallback callback,
			 gpointer user_data,
			 CORBA_Environment * ev)
{
        CORBA_Object callback_object;
        CORBA_Environment myev;
        GNOME_stringlist sel_order;
	OAF_ActivationContext activation_context;
        char *ext_requirements;

        g_return_if_fail (callback);

	if (ev == NULL) {
		ev = &myev;
	}
        
        CORBA_exception_init (ev);

        if (requirements == NULL) {
                callback (CORBA_OBJECT_NIL, "Requirements NULL", user_data);
                return;
        }

        /* get the Activation Context */
	activation_context = oaf_activation_context_get ();
        if (activation_context == CORBA_OBJECT_NIL) {
                callback (CORBA_OBJECT_NIL, "Could not get Activation Context", user_data);
                return;
        }

        ext_requirements = oaf_maybe_add_test_requirements (requirements);

        oaf_copy_string_array_to_GNOME_stringlist (selection_order, &sel_order);

        /* create the CORBA callback for this call It will destroy itelf later */
        callback_object = oaf_async_corba_callback_new (callback, user_data, ev);
        if (ev->_major != CORBA_NO_EXCEPTION
            || callback_object == CORBA_OBJECT_NIL) {
                callback (CORBA_OBJECT_NIL, "Could not create CORBA callback", user_data);
		if (ext_requirements != NULL) {
                	g_free (ext_requirements);
		}
		
                return;
        }


        /* make the OAF call :) */
        if (ext_requirements == NULL) {
                OAF_ActivationContext_activate_async (activation_context, 
                                                      requirements, &sel_order, 
                                                      flags, callback_object, 
                                                      oaf_context_get (), ev);
        } else {
                OAF_ActivationContext_activate_async (activation_context,
                                                      ext_requirements, &sel_order, 
                                                      flags, callback_object, 
                                                      oaf_context_get (), ev);
        }

	if (ext_requirements != NULL) {
		g_free (ext_requirements);
	}	

        if (ev->_major != CORBA_NO_EXCEPTION) {
                char *message;
                message = g_strconcat ("Could not contact Activation Context: ", 
                                       CORBA_exception_id (ev), NULL);
                callback (CORBA_OBJECT_NIL, message, user_data );
                g_free (message);
                return;
        }

        /* the end :) */
}

/**
 * oaf_activate_from_id_async:
 * @aid: the AID or IID of the component to activate.
 * @flags: activation flags.
 * @callback: callback function.
 * @user_data: data to be poassed to the callback function.
 * @ev: exception structure.
 *
 * This function will asynchronously try to activate a component 
 * with the given @aid. When the component is 
 * activated or when the activation fails, it will call @callback 
 * with the given @user_data data as parameter.
 * callback will be called with a CORBA_OBJECT_NIL object if the 
 * activation fails. If the activation fails, the callback will be 
 * given a human-readable string containing a description of the 
 * error. In case of sucess, the error string value is undefined.
 *
 * @flags can be 0 if you do not know what to set it to and 
 * @ev can be safely set to NULL.
 */
void oaf_activate_from_id_async (const OAF_ActivationID aid,
				 OAF_ActivationFlags flags,
				 OAFActivationCallback callback,
				 gpointer user_data,
				 CORBA_Environment * ev)
{
        CORBA_Object callback_object;
        CORBA_Environment myev;
	OAF_ActivationContext activation_context;
	OAFActivationInfo *ai;

        g_return_if_fail (callback);

	if (ev == NULL) {
		ev = &myev;
	}
        
        CORBA_exception_init (ev);

        if (aid == NULL) {
                callback (CORBA_OBJECT_NIL, "AID NULL", user_data);
                return;
        }

        /* get the Activation Context */
	activation_context = oaf_activation_context_get ();
        if (activation_context == CORBA_OBJECT_NIL) {
                callback (CORBA_OBJECT_NIL, "Could not get Activation Context", user_data);
                return;
        }

	ai = oaf_actid_parse (aid);

	if (ai != NULL) {		
                /* This is so that using an AID in an unactivated OD will work nicely */
                oaf_object_directory_get (ai->user, ai->host, ai->domain);

		oaf_actinfo_free (ai);
	}

        /* create the CORBA callback for this call It will destroy itelf later */
        callback_object = oaf_async_corba_callback_new (callback, user_data, ev);
        if (ev->_major != CORBA_NO_EXCEPTION
            || callback_object == CORBA_OBJECT_NIL) {
                callback (CORBA_OBJECT_NIL, "Could not create CORBA callback", user_data);
                return;
        }

        OAF_ActivationContext_activate_from_id_async (activation_context, 
                                                      aid, flags,
                                                      callback_object,
                                                      oaf_context_get (),
                                                      ev);

        if (ev->_major != CORBA_NO_EXCEPTION) {
                char *message;
                message = g_strconcat ("Could not contact Activation Context: ", 
                                       CORBA_exception_id (ev), NULL);
                callback (CORBA_OBJECT_NIL, message, user_data );
                g_free (message);
        }
            
}






