/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  ORBit: A CORBA v2.2 ORB
 *
 *  Copyright (C) 1998 Richard H. Porter, and Red Hat Software
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
 *  Author: Dick Porter <dick@cymru.net>
 *          Elliot Lee <sopwith@redhat.com>
 *
 */

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "orbit.h"
#include "orbit_poa.h"
#include "genrand.h"

PortableServer_ThreadPolicyValue
PortableServer_ThreadPolicy__get_value(PortableServer_ThreadPolicy  obj, CORBA_Environment *ev)
{
	if(!obj) {
		ev->_major = 2 ;
		goto error_exit;
	}
	ev->_major = CORBA_NO_EXCEPTION;

	return obj->value;

error_exit:
	CORBA_exception_set_system(ev, 0, CORBA_COMPLETED_NO);
	return 0;
}

PortableServer_LifespanPolicyValue
PortableServer_LifespanPolicy__get_value(PortableServer_LifespanPolicy  obj, CORBA_Environment *ev)
{
	if(!obj) {
		ev->_major = 2 ;
		goto error_exit;
	}

	ev->_major = CORBA_NO_EXCEPTION;
	return obj->value;

error_exit:
	CORBA_exception_set_system(ev, 0, CORBA_COMPLETED_NO);
	return 0;
}

PortableServer_IdUniquenessPolicyValue
PortableServer_IdUniquenessPolicy__get_value(PortableServer_IdUniquenessPolicy  obj, CORBA_Environment *ev)
{
	if(!obj) {
		ev->_major = 2 ;
		goto error_exit;
	}

	ev->_major = CORBA_NO_EXCEPTION;
	return obj->value;

error_exit:
	CORBA_exception_set_system(ev, 0, CORBA_COMPLETED_NO);
	return 0;
}

PortableServer_IdAssignmentPolicyValue
PortableServer_IdAssignmentPolicy__get_value(PortableServer_IdAssignmentPolicy  obj, CORBA_Environment *ev)
{
	if(!obj) {
		ev->_major = 2 ;
		goto error_exit;
	}

	ev->_major = CORBA_NO_EXCEPTION;
	return obj->value;

error_exit:
	CORBA_exception_set_system(ev, 0, CORBA_COMPLETED_NO);
	return 0;
}

PortableServer_ImplicitActivationPolicyValue
PortableServer_ImplicitActivationPolicy__get_value(PortableServer_ImplicitActivationPolicy  obj, CORBA_Environment *ev)
{
	if(!obj) {
		ev->_major = 2 ;
		goto error_exit;
	}

	ev->_major = CORBA_NO_EXCEPTION;
	return obj->value;

error_exit:
	CORBA_exception_set_system(ev, 0, CORBA_COMPLETED_NO);
	return 0;
}

PortableServer_ServantRetentionPolicyValue
PortableServer_ServantRetentionPolicy__get_value(PortableServer_ServantRetentionPolicy  obj, CORBA_Environment *ev)
{
	if(!obj) {
		ev->_major = 2 ;
		goto error_exit;
	}

	ev->_major = CORBA_NO_EXCEPTION;
	return obj->value;

error_exit:
	CORBA_exception_set_system(ev, 0, CORBA_COMPLETED_NO);
	return 0;
}

PortableServer_RequestProcessingPolicyValue
PortableServer_RequestProcessingPolicy__get_value(PortableServer_RequestProcessingPolicy  obj, CORBA_Environment *ev)
{
	if(!obj) {
		ev->_major = 2 ;
		goto error_exit;
	}

	ev->_major = CORBA_NO_EXCEPTION;
	return obj->value;

error_exit:
	CORBA_exception_set_system(ev, 0, CORBA_COMPLETED_NO);
	return 0;
}

/* make emacs happy; */

PortableServer_POAManager_State
PortableServer_POAManager_get_state(PortableServer_POAManager obj,
				    CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev,
					   ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return -1;
	}

	ev->_major = CORBA_NO_EXCEPTION;
	return obj->state;
}

/**** PortableServer_POAManager_activate
      Inputs: 'obj' - a POAManager to activate
      Outputs: '*ev' - result of the activate operation

      Side effect: Clears the 'held_requests' lists for all POA's
                   associated with the 'obj' POAManager.

      Description: Sets the POAManager state to 'ACTIVE', then
                   goes through all the POA's associated with this
		   POAManager, and makes them re-process their
		   'held_requests'
 */
void
PortableServer_POAManager_activate(PortableServer_POAManager obj,
				   CORBA_Environment *ev)
{
	GSList *todo;
	GSList *curitem;
	PortableServer_POA curpoa;

	if(!obj) {
		CORBA_exception_set_system(ev,
					   ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return;
	}

	if(obj->state == PortableServer_POAManager_INACTIVE) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POAManager_AdapterInactive,
				    NULL);
		return;
	}

	obj->state = PortableServer_POAManager_ACTIVE;

	for(curitem = obj->poa_collection; curitem;
	    curitem = g_slist_next(curitem)) {
		curpoa = (PortableServer_POA)curitem->data;

		todo = curpoa->held_requests;
		curpoa->held_requests = NULL;

		g_slist_foreach(todo, (GFunc)ORBit_POA_handle_request,
				curpoa);
		g_slist_foreach(todo, (GFunc)giop_recv_buffer_unuse,
				NULL);

		g_slist_free(todo);
	}
	ev->_major = CORBA_NO_EXCEPTION;
}

void
PortableServer_POAManager_hold_requests(PortableServer_POAManager obj,
					CORBA_boolean wait_for_completion,
					CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev,
					   ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return;
	}

	if(obj->state == PortableServer_POAManager_INACTIVE) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POAManager_AdapterInactive,
				    NULL);
		return;
	}

	obj->state = PortableServer_POAManager_HOLDING;
	if(!wait_for_completion)
		g_warning("hold_requests not finished - don't know how to kill outstanding request fulfillments");

	ev->_major = CORBA_NO_EXCEPTION;
}

void
PortableServer_POAManager_discard_requests(PortableServer_POAManager obj,
					   CORBA_boolean wait_for_completion,
					   CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev,
					   ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return;
	}

	if(obj->state == PortableServer_POAManager_INACTIVE) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POAManager_AdapterInactive,
				    NULL);
		return;
	}

	obj->state = PortableServer_POAManager_DISCARDING;
	if(!wait_for_completion)
		g_warning("discard_requests not finished - don't know how to kill outstanding request fulfillments");
	ev->_major = CORBA_NO_EXCEPTION;
}

void
PortableServer_POAManager_deactivate(PortableServer_POAManager obj,
				     CORBA_boolean etherealize_objects,
				     CORBA_boolean wait_for_completion,
				     CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev,
					   ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return;
	}

	if(obj->state == PortableServer_POAManager_INACTIVE) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POAManager_AdapterInactive,
				    NULL);
		return;
	}

	obj->state = PortableServer_POAManager_INACTIVE;

	if(etherealize_objects)
		g_slist_foreach(obj->poa_collection, (GFunc)ORBit_POA_etherealize_objects, ev);
	ev->_major = CORBA_NO_EXCEPTION;
}


CORBA_boolean
PortableServer_AdapterActivator_unknown_adapter(PortableServer_AdapterActivator obj,
						PortableServer_POA parent,
						CORBA_char *name,
						CORBA_Environment *ev)
{
	g_assert(!"Not yet implemented");
	return(CORBA_FALSE);
}


/**** PortableServer_ServantActivator_incarnate
 */
PortableServer_Servant

PortableServer_ServantActivator_incarnate
(PortableServer_ServantActivator obj,
 PortableServer_ObjectId *oid,
 PortableServer_POA adapter,
 CORBA_Environment *ev)
{
	g_assert(!"Not yet implemented");
	return(NULL);
}

void
PortableServer_ServantActivator_etherealize
(PortableServer_ServantActivator obj,
 PortableServer_ObjectId *oid, PortableServer_POA adapter,
 PortableServer_Servant serv,
 CORBA_boolean cleanup_in_progress,
 CORBA_boolean remaining_activations,
 CORBA_Environment *ev)
{
	g_assert(!"Not yet implemented");
	return;
}

PortableServer_POA
PortableServer_POA_create_POA
               (PortableServer_POA poa,
		CORBA_char *adapter_name,
		PortableServer_POAManager a_POAManager,
		CORBA_PolicyList* policies,
		CORBA_Environment *ev)
{
	PortableServer_POA new_poa = NULL;
	PortableServer_POA check_poa = NULL;
	
	/* Check for a child POA by the same name in parent */
	check_poa = PortableServer_POA_find_POA(poa,adapter_name,
						FALSE, ev);
	CORBA_exception_free (ev);

	if (!check_poa) {
	  new_poa = ORBit_POA_new(poa->orb,
				  adapter_name, a_POAManager, policies, ev);
	} else {
	  CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_PortableServer_POA_AdapterAlreadyExists,
			  NULL);
	  new_poa = NULL;
	}

	if(ev->_major == CORBA_NO_EXCEPTION) {
	  new_poa->parent_poa = poa;
	  ORBit_POA_add_child(poa, new_poa, ev);
	}

	return new_poa;
}

/**** PortableServer_POA_find_POA
      Inputs: 'obj' - a POA
              'activate_it' - whether to activate unknown POA's

      Outputs: 'child_poa'

      Description: Finds (and optionally activates) a child POA of 'obj'
                   with the specified names.

      TODO: Activate non-existent adapters if asked.

 */
PortableServer_POA
PortableServer_POA_find_POA(PortableServer_POA obj,
			    CORBA_char *adapter_name,
			    CORBA_boolean activate_it,
			    CORBA_Environment *ev)
{
	GSList *curitem;
	PortableServer_POA child_poa;

	for(curitem = obj->child_POAs; curitem;
	    curitem = g_slist_next(curitem)) {
		child_poa = (PortableServer_POA)curitem->data;
		if(!strcmp(child_poa->the_name, adapter_name)) {
			ev->_major = CORBA_NO_EXCEPTION;
			return child_poa;
		}
	}

	if(activate_it)
		g_warning("Don't yet know how to activate POA named \"%s\"",
			  adapter_name);

	CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			    ex_PortableServer_POA_AdapterNonExistent,
			    NULL);	

	return NULL;
}

/**** PortableServer_POA_destroy
      Inputs: 'obj' - the POA to be destroyed
              'etherealize_objects' - flag indicating whether any servant
	                              manager should be asked to etherealize
				      objects in the active object map
	      'wait_for_completion' - flag indicating whether to wait for
	                              requests currently being handled
 */
void
PortableServer_POA_destroy(PortableServer_POA obj,
			   CORBA_boolean etherealize_objects,
			   CORBA_boolean wait_for_completion,
			   CORBA_Environment *ev)
{
	if(etherealize_objects || !wait_for_completion)
		g_warning("PortableServer_POA_destroy not yet fully implemented; ignoring flags");

	if(ORBIT_ROOT_OBJECT(obj)->refs > 1)
		g_warning("POA has multiple refs [%d]",
			  ORBIT_ROOT_OBJECT(obj)->refs);

	CORBA_Object_release((CORBA_Object)obj, ev);
	ev->_major = CORBA_NO_EXCEPTION;
}

PortableServer_ThreadPolicy PortableServer_POA_create_thread_policy(PortableServer_POA obj, PortableServer_ThreadPolicyValue value, CORBA_Environment *ev)
{
	PortableServer_ThreadPolicy retval;

	retval = g_new(struct PortableServer_ThreadPolicy_type, 1);
	ORBit_policy_object_init((CORBA_Policy)retval,
				 PortableServer_THREAD_POLICY_ID, ev);

	retval->value = value;

	return (PortableServer_ThreadPolicy)CORBA_Object_duplicate((CORBA_Object)retval, ev);
}

PortableServer_LifespanPolicy PortableServer_POA_create_lifespan_policy(PortableServer_POA obj, PortableServer_LifespanPolicyValue value, CORBA_Environment *ev)
{
	PortableServer_LifespanPolicy retval;

	retval = g_new(struct PortableServer_LifespanPolicy_type, 1);
	ORBit_policy_object_init((CORBA_Policy)retval,
				 PortableServer_LIFESPAN_POLICY_ID, ev);

	retval->value = value;

	return (PortableServer_LifespanPolicy)CORBA_Object_duplicate((CORBA_Object)retval, ev);
}

PortableServer_IdUniquenessPolicy PortableServer_POA_create_id_uniqueness_policy(PortableServer_POA obj, PortableServer_IdUniquenessPolicyValue value, CORBA_Environment *ev)
{
	PortableServer_IdUniquenessPolicy retval;

	retval = g_new(struct PortableServer_IdUniquenessPolicy_type, 1);
	ORBit_policy_object_init((CORBA_Policy)retval,
				 PortableServer_ID_UNIQUENESS_POLICY_ID,
				 ev);

	retval->value = value;

	return (PortableServer_IdUniquenessPolicy)CORBA_Object_duplicate((CORBA_Object)retval, ev);
}

PortableServer_IdAssignmentPolicy PortableServer_POA_create_id_assignment_policy(PortableServer_POA obj, PortableServer_IdAssignmentPolicyValue value, CORBA_Environment *ev)
{
	PortableServer_IdAssignmentPolicy retval;

	retval = g_new(struct PortableServer_IdAssignmentPolicy_type, 1);
	ORBit_policy_object_init((CORBA_Policy)retval,
				 PortableServer_ID_ASSIGNMENT_POLICY_ID, ev);

	retval->value = value;

	return (PortableServer_IdAssignmentPolicy)CORBA_Object_duplicate((CORBA_Object)retval, ev);
}

PortableServer_ImplicitActivationPolicy PortableServer_POA_create_implicit_activation_policy(PortableServer_POA obj, PortableServer_ImplicitActivationPolicyValue value, CORBA_Environment *ev)
{
	PortableServer_ImplicitActivationPolicy retval;

	retval = g_new(struct PortableServer_ImplicitActivationPolicy_type, 1);
	ORBit_policy_object_init((CORBA_Policy)retval,
				 PortableServer_IMPLICIT_ACTIVATION_POLICY_ID, ev);

	retval->value = value;

	return (PortableServer_ImplicitActivationPolicy)CORBA_Object_duplicate((CORBA_Object)retval, ev);
}

PortableServer_ServantRetentionPolicy PortableServer_POA_create_servant_retention_policy(PortableServer_POA obj, PortableServer_ServantRetentionPolicyValue value, CORBA_Environment *ev)
{
	PortableServer_ServantRetentionPolicy retval;

	retval = g_new(struct PortableServer_ServantRetentionPolicy_type, 1);
	ORBit_policy_object_init((CORBA_Policy)retval,
				 PortableServer_SERVANT_RETENTION_POLICY_ID, ev);

	retval->value = value;

	return (PortableServer_ServantRetentionPolicy)CORBA_Object_duplicate((CORBA_Object)retval, ev);
}

PortableServer_RequestProcessingPolicy PortableServer_POA_create_request_processing_policy(PortableServer_POA obj, PortableServer_RequestProcessingPolicyValue value, CORBA_Environment *ev)
{
	PortableServer_RequestProcessingPolicy retval;

	retval = g_new(struct PortableServer_RequestProcessingPolicy_type, 1);
	ORBit_policy_object_init((CORBA_Policy)retval,
				 PortableServer_REQUEST_PROCESSING_POLICY_ID, ev);

	retval->value = value;

	return (PortableServer_RequestProcessingPolicy)CORBA_Object_duplicate((CORBA_Object)retval, ev);
}

CORBA_char *PortableServer_POA__get_the_name(PortableServer_POA obj, CORBA_Environment *ev)
{
	g_assert(obj);
	g_assert(obj->the_name);
	return obj->the_name;
}

PortableServer_POA
PortableServer_POA__get_the_parent(PortableServer_POA obj,
				   CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev,
					   ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return NULL;
	}

	return obj->parent_poa;
}

PortableServer_POAManager
PortableServer_POA__get_the_POAManager(PortableServer_POA obj,
				       CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev,
					   ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return NULL;
	}

	return obj->the_POAManager;
}

PortableServer_AdapterActivator PortableServer_POA__get_the_activator(PortableServer_POA obj, CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev,
					   ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return NULL;
	}

	return obj->the_activator;
}

void PortableServer_POA__set_the_activator(PortableServer_POA obj, PortableServer_AdapterActivator the_activator, CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev,
					   ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return;
	}

	obj->the_activator = the_activator;
}

PortableServer_ServantManager PortableServer_POA_get_servant_manager(PortableServer_POA obj, CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return NULL;
	}

	if(obj->request_processing != PortableServer_USE_SERVANT_MANAGER) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_WrongPolicy,
				    NULL);
		return NULL;
	}

	return obj->servant_manager;
}

void PortableServer_POA_set_servant_manager(PortableServer_POA obj, PortableServer_ServantManager imgr, CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return;
	}

	if(obj->request_processing != PortableServer_USE_SERVANT_MANAGER) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_WrongPolicy,
				    NULL);
		return;
	}

	obj->servant_manager = imgr;
}

PortableServer_Servant PortableServer_POA_get_servant(PortableServer_POA obj, CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return NULL;
	}

	if(obj->request_processing != PortableServer_USE_DEFAULT_SERVANT) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_WrongPolicy,
				    NULL);
		return NULL;
	}

	return obj->default_servant;
}

void PortableServer_POA_set_servant(PortableServer_POA obj, PortableServer_Servant p_servant, CORBA_Environment *ev)
{
	if(!obj) {
		CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return;
	}

	if(obj->request_processing != PortableServer_USE_DEFAULT_SERVANT) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_WrongPolicy,
				    NULL);
		return;
	}

	obj->default_servant = p_servant;
}

static CORBA_unsigned_long
get_objnum_for_obj(PortableServer_POA poa, ORBit_POAObject *obj)
{
	CORBA_unsigned_long retval;

	if(poa->first_free_id) {
		retval = poa->first_free_id;
		poa->first_free_id = GPOINTER_TO_UINT(g_ptr_array_index(poa->objnum_to_obj,
									retval));
		g_ptr_array_index(poa->objnum_to_obj, retval) = obj;
	} else {
		retval = poa->objnum_to_obj->len;
		g_ptr_array_add(poa->objnum_to_obj,
				obj);
	}

	return retval;
}

static CORBA_ORB
get_orb_for_poa(PortableServer_POA poa)
{
	if(poa->orb)
		return poa->orb;
	if(poa->parent_poa)
		return get_orb_for_poa(poa->parent_poa);

	return CORBA_OBJECT_NIL;
}

PortableServer_ObjectId *
PortableServer_POA_activate_object(PortableServer_POA obj,
				   PortableServer_Servant p_servant,
				   CORBA_Environment *ev)
{
	PortableServer_ServantBase *servant;
	PortableServer_ObjectId *new_objid;
	ORBit_POAObject *new_obj;

	servant = p_servant;

	if(obj->servant_retention != PortableServer_RETAIN
	   || obj->id_assignment != PortableServer_SYSTEM_ID) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_WrongPolicy,
				    NULL);
		return NULL;
	}
	
	/* Servant Already Active */
	if((obj->id_uniqueness==PortableServer_UNIQUE_ID) &&
	   (ORBIT_OBJECT_KEY(servant->_private)->object != 0)) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_ServantAlreadyActive,
				    NULL);
		return NULL;
	}
	

	new_obj = g_new0(ORBit_POAObject, 1);
	new_obj->object_id = (PortableServer_ObjectId*)CORBA_sequence_octet__alloc();

	new_objid =
		ORBit_POA_allocate_oid(obj,
					ORBIT_OBJECT_KEY(servant->_private)->class_info->class_name);
	
	new_obj->object_id->_buffer = CORBA_octet_allocbuf(new_objid->_length);
	new_obj->object_id->_length = new_objid->_length;
	memcpy(new_obj->object_id->_buffer, new_objid->_buffer,
		new_objid->_length);
	CORBA_sequence_set_release(new_obj->object_id, CORBA_TRUE);

	new_obj->servant = p_servant;
	ORBIT_OBJECT_KEY(servant->_private)->object = new_obj;
	new_obj->orb = get_orb_for_poa(obj);
	new_obj->poa = obj;
	new_obj->objnum = get_objnum_for_obj(obj, new_obj);
	orbit_genrand(new_obj->rand_data, ORBIT_RAND_KEY_LEN);

	g_hash_table_insert(obj->active_object_map,
			    new_obj->object_id,
			    new_obj);

	ev->_major = CORBA_NO_EXCEPTION;

	return new_objid;
}

void
PortableServer_POA_activate_object_with_id(PortableServer_POA obj,
					   PortableServer_ObjectId *id,
					   PortableServer_Servant p_servant,
					   CORBA_Environment *ev)
{
	PortableServer_ServantBase *servant = p_servant;
	ORBit_POAObject *newobj;

	if(!obj || !id || !p_servant) {
		CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return;
	}

	newobj = g_hash_table_lookup(obj->active_object_map,
				     id);

	if(newobj) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_ObjectAlreadyActive, NULL);
		return;
	}

	newobj = g_new0(ORBit_POAObject, 1);
	newobj->object_id = (PortableServer_ObjectId *)CORBA_sequence_octet__alloc();
	newobj->object_id->_length = id->_length;
	newobj->object_id->_buffer = CORBA_octet_allocbuf(id->_length);
	newobj->object_id->_release = CORBA_TRUE;
	memcpy(newobj->object_id->_buffer, id->_buffer, id->_length);
	newobj->poa = obj;
	newobj->orb = get_orb_for_poa(obj);
	newobj->objnum = get_objnum_for_obj(obj, newobj);
	orbit_genrand(newobj->rand_data, ORBIT_RAND_KEY_LEN);

	newobj->servant = p_servant;

	g_hash_table_insert(obj->active_object_map,
			    newobj->object_id,
			    newobj);

	ORBIT_OBJECT_KEY(servant->_private)->object = newobj;

	ev->_major = CORBA_NO_EXCEPTION;
}

void
PortableServer_POA_deactivate_object(PortableServer_POA obj,
				     PortableServer_ObjectId *oid,
				     CORBA_Environment *ev)
{
	ORBit_POAObject *oldobj;

	if(!obj || !oid) {
		CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM,
					   CORBA_COMPLETED_NO);
		return;
	}

	oldobj = g_hash_table_lookup(obj->active_object_map,
				     oid);

	if(!oldobj) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_ObjectNotActive,
				    NULL);
		return;
	}

	g_ptr_array_index(obj->objnum_to_obj, oldobj->objnum) = GUINT_TO_POINTER(obj->first_free_id);
	obj->first_free_id = oldobj->objnum;

	g_hash_table_remove(obj->active_object_map, oid);

	if(obj->request_processing == PortableServer_USE_SERVANT_MANAGER) {
		POA_PortableServer_ServantActivator__epv *epv;
		POA_PortableServer_ServantActivator *sm;

		sm = (POA_PortableServer_ServantActivator *)obj->servant_manager;
		epv = sm->vepv->PortableServer_ServantActivator_epv;
		epv->etherealize(sm, oldobj->object_id, obj,
				oldobj->servant,
				CORBA_FALSE,
				CORBA_FALSE,
				ev);
	} else {
	  PortableServer_ServantBase *serv = oldobj->servant;
	  serv->_private = NULL;
	}

	CORBA_free(oldobj->object_id);

	g_free(oldobj);
	ev->_major = CORBA_NO_EXCEPTION;
}

CORBA_Object
PortableServer_POA_create_reference(PortableServer_POA obj,
				    CORBA_RepositoryId intf,
				    CORBA_Environment *ev)
{
	g_assert(!"Not yet implemented");
	return(NULL);
}

PortableServer_ObjectId *PortableServer_POA_servant_to_id(PortableServer_POA obj, PortableServer_Servant p_servant, CORBA_Environment *ev)
{
	PortableServer_ObjectId *retval, *orig;
	PortableServer_ServantBase *serv = p_servant;
	g_return_val_if_fail(p_servant != NULL, NULL);

	orig = ORBIT_OBJECT_KEY(serv->_private)->object->object_id;
	retval = (PortableServer_ObjectId *)CORBA_sequence_octet__alloc();
	retval->_length = retval->_maximum = orig->_length;
	retval->_buffer = CORBA_octet_allocbuf(retval->_length);
	memcpy(retval->_buffer, orig->_buffer, retval->_length);
	CORBA_sequence_set_release(retval, CORBA_TRUE);

	return retval;
}

CORBA_Object
PortableServer_POA_servant_to_reference(PortableServer_POA obj, PortableServer_Servant p_servant, CORBA_Environment *ev)
{
	CORBA_Object retval;
	PortableServer_ObjectId *orig_id;
	PortableServer_ServantBase *servant = p_servant;
	ORBit_ObjectKey *obj_key =  ORBIT_OBJECT_KEY(servant->_private);
	
	int implicit = (obj->implicit_activation == PortableServer_IMPLICIT_ACTIVATION);
	int activate_able = (obj_key->object == 0) ||
		(obj->id_uniqueness==PortableServer_MULTIPLE_ID);
	/* ImplicitActivationPolicy */
	if( implicit && activate_able) {
		orig_id = PortableServer_POA_activate_object(obj, p_servant, ev);
	} else {
		orig_id = obj_key->object->object_id;
	}
	retval = PortableServer_POA_id_to_reference(obj,orig_id,ev);

	if( implicit && activate_able)
		CORBA_free (orig_id);

	return retval;
}

PortableServer_Servant
PortableServer_POA_reference_to_servant(PortableServer_POA obj, CORBA_Object reference, CORBA_Environment *ev)
{
	GSList *cur;

	g_assert(reference);

	if(obj->request_processing != PortableServer_USE_DEFAULT_SERVANT
	   && obj->servant_retention != PortableServer_RETAIN) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_WrongPolicy,
				    NULL);
		return NULL;
	}

	if(reference->servant)
		return reference->servant;

	for(cur = reference->profile_list; cur; cur = cur->next) {
		PortableServer_ObjectId *oid;
		ORBit_Object_info *curprof = cur->data;
		ORBit_POAObject *objinfo;

		objinfo = ORBit_POA_find_oid_for_object_key(obj, &(curprof->object_key), &oid);
		CORBA_free(oid);
		if(objinfo)
			return objinfo->servant;
	}

	CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			    ex_PortableServer_POA_ObjectNotActive,
			    NULL);
	return NULL;
}

PortableServer_ObjectId *PortableServer_POA_reference_to_id(PortableServer_POA obj, CORBA_Object reference, CORBA_Environment *ev)
{
	PortableServer_ObjectId *retval;
	ORBit_POAObject *objinfo;

	g_assert(reference);

	ORBit_object_get_connection(reference);

	g_assert(reference->active_profile);

	if(obj->request_processing != PortableServer_USE_DEFAULT_SERVANT
	   && obj->servant_retention != PortableServer_RETAIN) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_WrongPolicy,
				    NULL);
		return NULL;
	}

	objinfo = ORBit_POA_find_oid_for_object_key(obj, &(reference->active_profile->object_key), &retval);
	if(objinfo) {
		CORBA_free(retval);
		retval = (PortableServer_ObjectId *)CORBA_sequence_octet__alloc();
		retval->_length = retval->_maximum = objinfo->object_id->_length;
		retval->_buffer = CORBA_octet_allocbuf(retval->_length);
		memcpy(retval->_buffer, objinfo->object_id->_buffer, retval->_length);
		CORBA_sequence_set_release(retval, CORBA_TRUE);
		return retval;
	} else if(retval)
		return retval;

	CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			    ex_PortableServer_POA_ObjectNotActive,
			    NULL);
	return NULL;
}

PortableServer_Servant PortableServer_POA_id_to_servant(PortableServer_POA obj, PortableServer_ObjectId *oid, CORBA_Environment *ev)
{
	ORBit_POAObject *objinfo;

	if(obj->servant_retention != PortableServer_RETAIN) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_WrongPolicy,
				    NULL);
		return NULL;
	}

	objinfo = g_hash_table_lookup(obj->active_object_map,
				      oid);

	if(!objinfo) {
		CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
				    ex_PortableServer_POA_WrongPolicy,
				    NULL);
		return NULL;
	}

	return objinfo->servant;
}

static CORBA_Object
my_PortableServer_POA_id_to_reference(PortableServer_POA obj,
				      PortableServer_ObjectId *oid,
				      const char *type_id,
				      CORBA_Environment *ev)
{
	GSList *profiles=NULL;
	ORBit_Object_info *object_info;
	CORBA_Object retval;
	CORBA_ORB orb;
	ORBit_POAObject *pobj;
	ORBit_ObjectKey *objkey = NULL;

	orb = obj->the_POAManager->orb;

	g_assert(!oid->_buffer[oid->_length - 1]);

	pobj = g_hash_table_lookup(obj->active_object_map, oid);

	if(pobj) {
		objkey = ORBIT_OBJECT_KEY(((PortableServer_ServantBase *)pobj->servant)->_private);
		type_id= objkey->class_info->class_name;
	}

	/* Do the local connection first, so it will be attempted first by
	   the client parsing the IOR string
	 */
	if(orb->cnx.ipv6 || orb->cnx.usock) {
		object_info = g_new0(ORBit_Object_info, 1);

		object_info->profile_type=IOP_TAG_ORBIT_SPECIFIC;
		object_info->iiop_major = 1;
		object_info->iiop_minor = 0;

		ORBit_POA_find_object_key_for_oid(obj, pobj, oid, &object_info->object_key);

#ifdef HAVE_IPV6
		if(orb->cnx.ipv6) {
			object_info->tag.orbitinfo.ipv6_port =
				ntohs(IIOP_CONNECTION(orb->cnx.ipv6)->u.ipv6.location.sin_port);
		}
#endif
		if(orb->cnx.usock) {
			object_info->tag.orbitinfo.unix_sock_path =
				g_strdup(IIOP_CONNECTION(orb->cnx.usock)->u.usock.sun_path);
		}
		ORBit_set_object_key(object_info);
		profiles=g_slist_append(profiles, object_info);
	}

	if(orb->cnx.ipv4) {
		object_info=g_new0(ORBit_Object_info, 1);

		object_info->profile_type = IOP_TAG_INTERNET_IOP;
		object_info->iiop_major = 1;
		object_info->iiop_minor = 0;
		ORBit_POA_find_object_key_for_oid(obj, pobj, oid, &object_info->object_key);

		object_info->tag.iopinfo.host = g_strdup(IIOP_CONNECTION(orb->cnx.ipv4)->u.ipv4.hostname);
		object_info->tag.iopinfo.port = ntohs(IIOP_CONNECTION(orb->cnx.ipv4)->u.ipv4.location.sin_port);

		ORBit_set_object_key(object_info);
		profiles=g_slist_append(profiles, object_info);
	}

	retval = ORBit_create_object_with_info(profiles, type_id, orb, ev);

	if(retval != CORBA_OBJECT_NIL
	   && ev->_major == CORBA_NO_EXCEPTION
	   && objkey && objkey->class_info && objkey->class_info->init_local_objref) {
	  if(!retval->vepv)
	    {
	     /* XXX potential memleak if we get an already-valid objref */
		retval->vepv = g_new0(gpointer, ORBit_class_assignment_counter + 1);
		retval->vepv_size = ORBit_class_assignment_counter + 1;
		objkey->class_info->init_local_objref(retval, pobj->servant);
		retval->servant = pobj->servant;
	    }
	} else
		retval->vepv = retval->servant = NULL;

	return retval;
}

CORBA_Object PortableServer_POA_id_to_reference(PortableServer_POA obj,
						PortableServer_ObjectId *oid,
						CORBA_Environment *ev)
{
	return my_PortableServer_POA_id_to_reference(obj, oid, NULL, ev);
}

CORBA_Object
PortableServer_POA_create_reference_with_id(PortableServer_POA obj,
					    PortableServer_ObjectId *oid,
					    CORBA_RepositoryId intf,
					    CORBA_Environment *ev)
{
	return my_PortableServer_POA_id_to_reference(obj, oid, intf, ev);
}

PortableServer_POA PortableServer_Current_get_POA(PortableServer_Current obj, CORBA_Environment *ev)
{
	g_assert(!"Not yet implemented");
	return(NULL);
}

PortableServer_ObjectId *PortableServer_Current_get_object_id(PortableServer_Current obj, CORBA_Environment *ev)
{
	g_assert(!"Not yet implemented");
	return(NULL);
}


CORBA_char *PortableServer_ObjectId_to_string(PortableServer_ObjectId *id, CORBA_Environment *env)
{
	return CORBA_string_dup((CORBA_char *)id->_buffer);
}

CORBA_wchar *PortableServer_ObjectId_to_wstring(PortableServer_ObjectId *id, CORBA_Environment *env)
{
	g_assert(!"Not yet implemented");
	return(NULL);
}

PortableServer_ObjectId *PortableServer_string_to_ObjectId(CORBA_char *str, CORBA_Environment *env)
{
	PortableServer_ObjectId *retval;

	retval = (PortableServer_ObjectId *)CORBA_sequence_octet__alloc();

	retval->_length = strlen(str) + 1;
	retval->_buffer = CORBA_octet_allocbuf(retval->_length);

	memcpy(retval->_buffer, str, retval->_length);

	return retval;
}

PortableServer_ObjectId *PortableServer_wstring_to_ObjectId(CORBA_wchar *str, CORBA_Environment *env)
{
	g_assert(!"Not yet implemented");
	return(NULL);
}


PortableServer_POA PortableServer_ServantBase__default_POA(PortableServer_Servant servant, CORBA_Environment *ev)
{
	g_return_val_if_fail(servant, NULL);

	return ORBIT_OBJECT_KEY(((PortableServer_ServantBase *)servant)->_private)->object->poa;
}

void PortableServer_ServantLocator_preinvoke(PortableServer_ObjectId *oid, PortableServer_POA adapter, CORBA_Identifier op_name, PortableServer_ServantLocator_Cookie *cookie)
{
	g_assert(!"Not yet implemented");
	return;
}

void PortableServer_ServantLocator_postinvoke(PortableServer_ObjectId *oid, PortableServer_POA adapter, CORBA_Identifier op_name, PortableServer_ServantLocator_Cookie cookie, PortableServer_Servant servant)
{
	g_assert(!"Not yet implemented");
	return;
}

void PortableServer_ServantBase__init(PortableServer_Servant servant,
				      CORBA_Environment *ev)
{
	PortableServer_ServantBase *serv = servant;

	if(!serv->_private) /* If not already initialized, create the place to
			       stick our info */
		serv->_private = g_new0(ORBit_ObjectKey, 1);
}

void PortableServer_ServantBase__fini(PortableServer_Servant servant,
				      CORBA_Environment *ev)
{
	PortableServer_ServantBase *serv = servant;

	g_free(serv->_private);
	serv->_private = NULL;
}


/************************ ServerRequest stuff ********************/

CORBA_Identifier CORBA_ServerRequest_operation(CORBA_ServerRequest req, CORBA_Environment *env)
{
	return CORBA_string_dup(req->rbuf->message.u.request.operation);
}

CORBA_Context
CORBA_ServerRequest_ctx(CORBA_ServerRequest req, CORBA_Environment *env)
{
	if(!req->params || req->did_ctx) {
		CORBA_exception_set_system(env, ex_CORBA_BAD_INV_ORDER,
					   CORBA_COMPLETED_NO);
		return NULL;
	}

	return NULL;
}

void
CORBA_ServerRequest_arguments(CORBA_ServerRequest req,
			      CORBA_NVList *parameters,
			      CORBA_Environment *env)
{
	int i;

	if(req->params) {
		CORBA_exception_set_system(env, ex_CORBA_BAD_INV_ORDER,
					   CORBA_COMPLETED_NO);
		return;
	}

	req->params = parameters;

	for(i = 0; i < parameters->list->len; i++) {
		CORBA_NamedValue *cur;

		cur = &g_array_index(parameters->list, CORBA_NamedValue, i);

		if(cur->arg_modes & CORBA_ARG_OUT) continue;
		cur->argument._value = ORBit_demarshal_arg(req->rbuf,
							   cur->argument._type,
							   TRUE,
							   (CORBA_ORB)req->orb);
		CORBA_any_set_release(&cur->argument, TRUE);
	}
}

void
CORBA_ServerRequest_set_result(CORBA_ServerRequest req,
			       CORBA_any *value,
			       CORBA_Environment *env)
{
	if(req->sbuf) {
		CORBA_exception_set_system(env, ex_CORBA_BAD_INV_ORDER,
					   CORBA_COMPLETED_NO);
		return;
	}

	req->sbuf = giop_send_reply_buffer_use(GIOP_MESSAGE_BUFFER(req->rbuf)->connection,
					       NULL,
					       req->rbuf->message.u.request.request_id,
					       CORBA_NO_EXCEPTION);
	if(!req->sbuf) {
		CORBA_exception_set_system(env, ex_CORBA_COMM_FAILURE,
					   CORBA_COMPLETED_NO);
		return;
	}
		
	ORBit_marshal_arg(req->sbuf, value->_value, value->_type);
}

void
CORBA_ServerRequest_set_exception(CORBA_ServerRequest req,
				  CORBA_exception_type major,
				  CORBA_any *value,
				  CORBA_Environment *env)
{
	if(req->sbuf) {
		CORBA_exception_set_system(env, ex_CORBA_BAD_INV_ORDER,
					   CORBA_COMPLETED_NO);
		return;
	}

	req->sbuf = giop_send_reply_buffer_use(GIOP_MESSAGE_BUFFER(req->rbuf)->connection,
					       NULL,
					       req->rbuf->message.u.request.request_id,
					       major);
	if(!req->sbuf) {
		CORBA_exception_set_system(env, ex_CORBA_COMM_FAILURE,
					   CORBA_COMPLETED_NO);
		return;
	}

	req->did_exc = TRUE;

	/* XXX do we really need to copy the repo_id into the
	   send buffer? Or is there a way to assume that the CORBA_TypeCode
	   value->_type will be around until after we send the message? */
	{
		CORBA_unsigned_long slen;
		slen = strlen(value->_type->repo_id) + 1;
		giop_send_buffer_append_mem_indirect_a(req->sbuf, &slen,
						       sizeof(slen));
		giop_send_buffer_append_mem_indirect(req->sbuf,
						     value->_type->repo_id,
						     slen);
	}
		
	ORBit_marshal_arg(req->sbuf, value->_value, value->_type);
}

void
POA_PortableServer_ServantActivator__init(PortableServer_Servant servant,
					CORBA_Environment * ev)
{
	static const PortableServer_ClassInfo class_info =
	{NULL,
	"IDL:omg.org/PortableServer/ServantActivator:1.0",
	NULL};

	PortableServer_ServantBase__init(((PortableServer_ServantBase *) servant), ev);

	ORBIT_OBJECT_KEY(((PortableServer_ServantBase *)servant)->_private)->class_info = (gpointer)&class_info;
}

void
POA_PortableServer_ServantActivator__fini(PortableServer_Servant servant,
					CORBA_Environment * ev)
{
	PortableServer_ServantBase__fini(servant, ev);
}

void
POA_PortableServer_ServantLocator__init(PortableServer_Servant servant,
					CORBA_Environment * ev)
{
	static const PortableServer_ClassInfo class_info =
	{NULL,
	"IDL:omg.org/PortableServer/ServantLocator:1.0",
	NULL};

	PortableServer_ServantBase__init(((PortableServer_ServantBase *)servant), ev);

	ORBIT_OBJECT_KEY(((PortableServer_ServantBase *)servant)->_private)->class_info = (gpointer)&class_info;
}

void
POA_PortableServer_ServantLocator__fini(PortableServer_Servant servant,
					CORBA_Environment * ev)
{
	PortableServer_ServantBase__fini(servant, ev);
}

/* POA-related DSI stuff */
static void
dynamic_impl_skel(PortableServer_DynamicImpl *_ORBIT_servant,
		  GIOPRecvBuffer *_ORBIT_recv_buffer,
		  CORBA_Environment *ev,
		  PortableServer_DynamicImplRoutine invoke)
{
	/* here the magic occurs... */
	struct CORBA_ServerRequest_type sr;

	ORBit_pseudo_object_init(ORBIT_PSEUDO_OBJECT(&sr),
				 ORBIT_PSEUDO_SERVERREQUEST, ev);

	CORBA_Object_duplicate((CORBA_Object)&sr, ev); /* just to make
							  sure it doesn't die
							  elsewhere */

	sr.rbuf = _ORBIT_recv_buffer;
	sr.orb = GIOP_MESSAGE_BUFFER(_ORBIT_recv_buffer)->connection->orb_data;

	_ORBIT_servant->vepv->PortableServer_DynamicImpl_epv->invoke(_ORBIT_servant,
								     &sr);

	if(sr.sbuf) {
		int i;
		for(i = 0; i < sr.params->list->len; i++) {
			CORBA_NamedValue *cur;
			
			cur = &g_array_index(sr.params->list, CORBA_NamedValue, i);
			
			if(cur->arg_modes & CORBA_ARG_IN) continue;

			ORBit_marshal_arg(sr.sbuf, cur->argument._value,
					  cur->argument._type);
		}

		giop_send_buffer_write(sr.sbuf);
		giop_send_buffer_unuse(sr.sbuf);
	} else
		g_warning("Yo, your DSI code is messed up! You forgot to set_result|set_exception");

	CORBA_NVList_free(sr.params, ev);
}

static ORBitSkeleton 
dynamic_impl_get_skel(PortableServer_DynamicImpl * servant,
		      GIOPRecvBuffer * _ORBIT_recv_buffer,
		      gpointer * impl)
{
	*impl = (gpointer)servant->vepv->PortableServer_DynamicImpl_epv->invoke;

	return (ORBitSkeleton)dynamic_impl_skel;
}

void
PortableServer_DynamicImpl__init(PortableServer_Servant servant,
				 CORBA_Environment *ev)
{
	static const PortableServer_ClassInfo class_info =
	{(ORBitSkeleton (*)(PortableServer_ServantBase *, gpointer, gpointer *))
	 &dynamic_impl_get_skel, "IDL:CORBA/Object:1.0", NULL};
	
	PortableServer_ServantBase__init(servant, ev);
	
	ORBIT_OBJECT_KEY(((PortableServer_ServantBase *)servant)->_private)->class_info =
		(PortableServer_ClassInfo *) & class_info;
	
}

void PortableServer_DynamicImpl__fini(PortableServer_Servant servant,
				      CORBA_Environment *ev)
{
	PortableServer_ServantBase__fini(servant, ev);
}

