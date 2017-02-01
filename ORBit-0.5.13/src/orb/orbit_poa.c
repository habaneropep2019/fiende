/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  ORBit: A CORBA v2.2 ORB
 *
 *  Copyright (C) 1998 Richard H. Porter and Red Hat Software
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
 *  Author: Phil Dawes <philipd@parallax.co.uk>
 *	    Elliot Lee <sopwith@redhat.com>
 *
 */

/*
 *   ORBit specific POA funcitons.
 *
 */

#include <string.h>
#include "orbit.h"
#include "orbit_poa_type.h"
#include "orbit_poa.h"
#include "genrand.h"

#define POA_KEY_LEN (sizeof(CORBA_unsigned_long) + ORBIT_RAND_KEY_LEN)
#define OBJ_KEY_LEN (sizeof(CORBA_unsigned_long) + ORBIT_RAND_KEY_LEN)

static void ORBit_POAManager_release(PortableServer_POAManager poa_mgr,
				     CORBA_Environment *ev);

static void ORBit_POA_release(PortableServer_POA poa,
			      CORBA_Environment *ev);

static PortableServer_Servant
ORBit_POA_ServantManager_use_servant(PortableServer_POA poa,
				     GIOPRecvBuffer *recv_buffer,
				     PortableServer_ServantLocator_Cookie *the_cookie,
				     PortableServer_ObjectId *oid,
				     ORBit_POAObject *fake_obj_impl,
				     CORBA_Environment *ev);
static void
ORBit_POA_ServantManager_unuse_servant(PortableServer_Servant servant,
				       PortableServer_POA poa,
				       GIOPRecvBuffer *recv_buffer,
				       PortableServer_ServantLocator_Cookie cookie,
				       PortableServer_ObjectId *oid,
				       ORBit_POAObject *fake_obj_impl,
				       CORBA_Environment *ev);

static const ORBit_RootObject_Interface CORBA_POAManager_epv =
{
	(void (*)(gpointer, CORBA_Environment *))ORBit_POAManager_release,
};

static const ORBit_RootObject_Interface CORBA_POA_epv =
{
	(void (*)(gpointer, CORBA_Environment *))ORBit_POA_release,
};

guint
g_sequence_octet_hash(CORBA_sequence_octet *so)
{
  const char *s = (char*)so->_buffer;
  const char *p, *e = ((char *)so->_buffer) + so->_length;
  guint h=0, g;

  for(p = s; p < e; p ++) {
    h = ( h << 4 ) + *p;
    if ( ( g = h & 0xf0000000 ) ) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }

  return h;
}

gint
g_sequence_octet_compare(CORBA_sequence_octet *s1, CORBA_sequence_octet *s2)
{
	if(s2->_length != s1->_length)
		return FALSE;

	return !memcmp(s1->_buffer, s2->_buffer, s1->_length);
}

PortableServer_POAManager 
ORBit_POAManager_new(CORBA_Environment *ev)
{
	PortableServer_POAManager poa_mgr;

	poa_mgr = g_new0(struct PortableServer_POAManager_type, 1);

	if(poa_mgr == NULL) {
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		goto error;
	}

	/* Initialise poa manager */

	ORBit_pseudo_object_init(ORBIT_PSEUDO_OBJECT(poa_mgr),
				 ORBIT_PSEUDO_POAMANAGER, ev);
	ORBIT_ROOT_OBJECT(poa_mgr)->refs = 0;
	ORBit_RootObject_set_interface(ORBIT_ROOT_OBJECT(poa_mgr),
				       (gpointer)&CORBA_POAManager_epv, ev);

	poa_mgr->poa_collection = NULL;
	poa_mgr->state = PortableServer_POAManager_HOLDING;

	return poa_mgr;

error:
	if(poa_mgr != NULL){
		ORBit_POAManager_release(poa_mgr, ev);
	}
	return NULL;
}

static void
ORBit_POAManager_release(PortableServer_POAManager poa_mgr,
			 CORBA_Environment *ev)
{

	if(--(ORBIT_ROOT_OBJECT(poa_mgr)->refs) > 0)
		return;

	if(poa_mgr != NULL) {
		if(poa_mgr->poa_collection != NULL)  {
			g_slist_free(poa_mgr->poa_collection);
			poa_mgr->poa_collection = NULL;
		}
		g_free(poa_mgr);
		poa_mgr = NULL;
	}
}

static void
ORBit_POAManager_register_poa(PortableServer_POAManager poa_mgr, 
			      PortableServer_POA poa,
			      CORBA_Environment *ev)
{
	poa_mgr->poa_collection = g_slist_remove(poa_mgr->poa_collection, poa);
	poa_mgr->poa_collection = 
		g_slist_append(poa_mgr->poa_collection, poa);
}

static void
ORBit_POAManager_unregister_poa(PortableServer_POAManager poa_mgr, 
				PortableServer_POA poa,
				CORBA_Environment *ev)
{
	poa_mgr->poa_collection = g_slist_remove(poa_mgr->poa_collection, poa);
}

static void
ORBit_POA_set_policy(PortableServer_POA poa,
		     CORBA_Policy policy,
		     CORBA_Environment *ev)
{
	switch(policy->policy_type) {
	case PortableServer_THREAD_POLICY_ID:
		poa->thread = ((PortableServer_ThreadPolicy)policy)->value;
		break;
	case PortableServer_LIFESPAN_POLICY_ID:
		poa->lifespan = ((PortableServer_LifespanPolicy)policy)->value;
		break;
	case PortableServer_ID_UNIQUENESS_POLICY_ID:
		poa->id_uniqueness = ((PortableServer_IdUniquenessPolicy)policy)->value;
		break;
	case PortableServer_ID_ASSIGNMENT_POLICY_ID:
		poa->id_assignment = ((PortableServer_IdAssignmentPolicy)policy)->value;
		break;
	case PortableServer_IMPLICIT_ACTIVATION_POLICY_ID:
	  poa->implicit_activation = ((PortableServer_ImplicitActivationPolicy)policy)->value;
		break;
	case PortableServer_SERVANT_RETENTION_POLICY_ID:
	  poa->servant_retention = ((PortableServer_ServantRetentionPolicy)policy)->value;
		break;
	case PortableServer_REQUEST_PROCESSING_POLICY_ID:
		poa->request_processing = ((PortableServer_ServantRetentionPolicy)policy)->value;
		break;
	default:
		g_warning("Unknown policy type, cannot set it on this POA");
	}
}


static void
ORBit_POA_check_policy_conflicts(PortableServer_POA poa,
		     CORBA_Environment *ev)
{

  /* Check for those policy combinations that aren't allowed */
  if ((poa->servant_retention == PortableServer_NON_RETAIN &&
       poa->request_processing == PortableServer_USE_ACTIVE_OBJECT_MAP_ONLY) ||
      
      (poa->request_processing == PortableServer_USE_DEFAULT_SERVANT &&
       poa->id_uniqueness == PortableServer_UNIQUE_ID) ||
      
      (poa->implicit_activation == PortableServer_IMPLICIT_ACTIVATION &&
       (poa->id_assignment == PortableServer_USER_ID ||
	poa->servant_retention == PortableServer_NON_RETAIN))
      )
    {
      CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_PortableServer_POA_InvalidPolicy,
			  NULL);
    }
}
      

static void
ORBit_POA_set_policylist(PortableServer_POA poa,
			 CORBA_PolicyList *policies,
			 CORBA_Environment *ev)
{
	CORBA_unsigned_long i;

	for(i = 0; i < policies->_length; i++) {
		if(ev->_major != CORBA_NO_EXCEPTION)
			break;
		ORBit_POA_set_policy(poa, policies->_buffer[i], ev);
	}
}

PortableServer_POA 
ORBit_POA_new(CORBA_ORB orb,
	      CORBA_char *adapter_name,
	      PortableServer_POAManager the_POAManager,
	      CORBA_PolicyList *policies,
	      CORBA_Environment *ev)
{
	PortableServer_POA poa;

	/* Create the object */
	poa = (PortableServer_POA) g_new0(struct PortableServer_POA_type, 1); 
	if(poa == NULL) {
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		goto error;
	}

	ORBit_pseudo_object_init(ORBIT_PSEUDO_OBJECT(poa), ORBIT_PSEUDO_POA, ev);

	ORBIT_ROOT_OBJECT(poa)->refs = 0;
	ORBit_RootObject_set_interface(ORBIT_ROOT_OBJECT(poa),
				       (gpointer)&CORBA_POA_epv, ev);

	if(ev->_major != CORBA_NO_EXCEPTION) goto error;

	/* If no POAManager was specified, create one */
	if(the_POAManager == NULL) {
	  the_POAManager = ORBit_POAManager_new(ev);
	}	  

	/* Register this poa with the poa manager */
	if(the_POAManager != NULL)
		ORBit_POAManager_register_poa(the_POAManager,poa,ev);
	if(ev->_major != CORBA_NO_EXCEPTION) goto error;

	/* Wire up the poa_manager */
	poa->the_POAManager = the_POAManager;

	/* Initialise the child poas table */
	poa->child_POAs = NULL;      /* initialise the slist */

	poa->held_requests = NULL;

	poa->poaID = orb->poas->len;
	g_ptr_array_set_size(orb->poas, orb->poas->len + 1);
	g_ptr_array_index(orb->poas, poa->poaID) = poa;

	poa->orb = orb;

	g_return_val_if_fail(ev->_major == CORBA_NO_EXCEPTION, NULL);

	/* Need to initialise poa policies etc.. here */
	poa->thread = PortableServer_ORB_CTRL_MODEL;
	poa->lifespan = PortableServer_TRANSIENT;
	poa->id_uniqueness = PortableServer_UNIQUE_ID;
	poa->id_assignment = PortableServer_SYSTEM_ID;
	poa->servant_retention = PortableServer_RETAIN;
	poa->request_processing = PortableServer_USE_ACTIVE_OBJECT_MAP_ONLY;
	poa->implicit_activation = PortableServer_NO_IMPLICIT_ACTIVATION;
	if (policies) {
	  ORBit_POA_set_policylist(poa, policies, ev);
	  ORBit_POA_check_policy_conflicts(poa, ev);
	  if(ev->_major != CORBA_NO_EXCEPTION) goto error;
	}

	/* copy the name  */
	poa->the_name = CORBA_string_dup(adapter_name);

	poa->active_object_map = g_hash_table_new((GHashFunc)g_sequence_octet_hash,
						  (GCompareFunc)g_sequence_octet_compare);
	poa->objnum_to_obj = g_ptr_array_new();
	g_ptr_array_set_size(poa->objnum_to_obj, 1);
	g_ptr_array_index(poa->objnum_to_obj, 0) = NULL;

	orbit_genrand(poa->rand_data, ORBIT_RAND_KEY_LEN);

	return poa;

error:
	if(poa && poa->the_name){
		CORBA_free(poa->the_name);
	}

	if(poa != NULL){
		ORBit_POA_release(poa, NULL);
	}
	return NULL;
}

static void
ORBit_POA_release(PortableServer_POA poa,
		  CORBA_Environment *ev)
{
	ORBIT_ROOT_OBJECT_UNREF(poa);

	if(ORBIT_ROOT_OBJECT(poa)->refs <= 0) {
		CORBA_free(poa->the_name);

		g_slist_foreach(poa->child_POAs, (GFunc)CORBA_Object_release,
				ev);

		if(poa->parent_poa)
			ORBit_POA_remove_child(poa->parent_poa, poa, ev);

		ORBit_POAManager_unregister_poa(poa->the_POAManager, 
						poa, ev);

		g_ptr_array_index(poa->orb->poas, poa->poaID) = NULL;

		g_free(poa);
	}
}

void
ORBit_POA_add_child(PortableServer_POA poa,
		    PortableServer_POA child_poa,
		    CORBA_Environment *ev)

{
	g_return_if_fail(poa != NULL);
	g_return_if_fail(child_poa != NULL);

	poa->child_POAs = g_slist_prepend(poa->child_POAs, child_poa);
}

void
ORBit_POA_remove_child(PortableServer_POA poa,
		       PortableServer_POA child_poa,
		       CORBA_Environment *ev)
{
	g_return_if_fail(poa != NULL);
	g_return_if_fail(child_poa != NULL);

	poa->child_POAs = g_slist_remove(poa->child_POAs, child_poa);
}

extern ORBit_request_validate ORBIT_request_validator;

gboolean
ORBit_POA_handle_request(GIOPRecvBuffer *recv_buffer,
			 PortableServer_POA poa)
{
	PortableServer_ServantBase *servant;
	PortableServer_ServantLocator_Cookie cookie;
	ORBit_POAObject *obj_impl = NULL, tmp_obj_impl;
	ORBitSkeleton skel;
	gpointer imp = NULL;
	CORBA_Environment ev;
	GIOPSendBuffer *send_buffer;
	guchar *opname;
	PortableServer_ObjectId *oid = NULL;

	CORBA_exception_init(&ev);

	switch(poa->the_POAManager->state) {
	case PortableServer_POAManager_HOLDING:
		poa->held_requests = g_slist_prepend(poa->held_requests,
						     recv_buffer);
		return FALSE;
		break;
	case PortableServer_POAManager_DISCARDING:
		send_buffer = giop_send_reply_buffer_use(GIOP_MESSAGE_BUFFER(recv_buffer)->connection,
							 NULL,
							 recv_buffer->message.u.request.request_id,
							 CORBA_SYSTEM_EXCEPTION);
		CORBA_exception_set_system(&ev,
					    ex_CORBA_TRANSIENT,
					    CORBA_COMPLETED_NO);
		ORBit_send_system_exception(send_buffer, &ev);
		giop_send_buffer_write(send_buffer);
		giop_recv_buffer_unuse(recv_buffer);
		giop_send_buffer_unuse(send_buffer);
		CORBA_exception_free(&ev);
		return TRUE;
		break;
	case PortableServer_POAManager_INACTIVE:
		send_buffer = giop_send_reply_buffer_use(GIOP_MESSAGE_BUFFER(recv_buffer)->connection,
							 NULL,
							 recv_buffer->message.u.request.request_id,
							 CORBA_SYSTEM_EXCEPTION);
		CORBA_exception_set_system(&ev,
					    ex_CORBA_OBJ_ADAPTER,
					    CORBA_COMPLETED_NO);
		ORBit_send_system_exception(send_buffer, &ev);
		giop_send_buffer_write(send_buffer);
		giop_recv_buffer_unuse(recv_buffer);
		giop_send_buffer_unuse(send_buffer);
		CORBA_exception_free(&ev);
		return TRUE;
		break;
	case PortableServer_POAManager_ACTIVE:
	default:
		break;
	}

	servant = NULL;

	if(recv_buffer->message.u.request.object_key._length
	   < (POA_KEY_LEN + sizeof(CORBA_unsigned_long))) {
		CORBA_exception_set_system(&ev,
					   ex_CORBA_OBJECT_NOT_EXIST,
					   CORBA_COMPLETED_NO);
		goto errout;
	}

	obj_impl = ORBit_POA_find_oid_for_object_key(poa, &(recv_buffer->message.u.request.object_key), &oid);

	if(poa->servant_retention == PortableServer_RETAIN
	   && obj_impl) {
		servant = obj_impl->servant;
		oid = obj_impl->object_id;
	}

	if(!servant) {
		switch(poa->request_processing) {
		case PortableServer_USE_ACTIVE_OBJECT_MAP_ONLY:
                  if(oid)
		    obj_impl = g_hash_table_lookup(poa->active_object_map, oid);
		  if(obj_impl)
		    servant = obj_impl->servant;
		  break;
		case PortableServer_USE_SERVANT_MANAGER:
			servant = ORBit_POA_ServantManager_use_servant(poa,
								       recv_buffer,
								       &cookie,
								       oid,
								       &tmp_obj_impl,
								       &ev);
			break;
		case PortableServer_USE_DEFAULT_SERVANT:
			servant = poa->default_servant;
			if(servant == NULL) {
				CORBA_exception_set_system(&ev,
							   ex_CORBA_OBJ_ADAPTER,
							   CORBA_COMPLETED_NO);
				goto errout;
			}
			break;
		default:
			break;
		}
	}

	if(!poa || !servant || !servant->_private) {
		CORBA_exception_set_system(&ev,
					   ex_CORBA_OBJECT_NOT_EXIST,
					   CORBA_COMPLETED_NO);
		goto errout;
	}

	opname = recv_buffer->message.u.request.operation;

	skel = ORBIT_OBJECT_KEY(servant->_private)->class_info->relay_call(servant, recv_buffer, &imp);
	
	if(!skel) {
		if (opname[0] == '_' && strcmp(opname + 1, "is_a") == 0) {
			skel = (gpointer)&ORBit_impl_CORBA_Object_is_a;
		}
		else {
			CORBA_exception_set_system(&ev, ex_CORBA_BAD_OPERATION,
						   CORBA_COMPLETED_NO);
			goto errout;
		}
	}
	else if (!imp) {
		CORBA_exception_set_system(&ev, ex_CORBA_NO_IMPLEMENT,
					   CORBA_COMPLETED_NO);
		goto errout;
	}

	/* If it got through the random keys, and nobody else had the opportunity to say otherwise, it must be auth'd */

	if(!ORBIT_request_validator)
		GIOP_MESSAGE_BUFFER(recv_buffer)->connection->is_auth = TRUE;

	skel(servant, recv_buffer, &ev, imp);

	if(poa->request_processing == PortableServer_USE_SERVANT_MANAGER) {
		ORBit_POA_ServantManager_unuse_servant(servant,
						       poa,
						       recv_buffer,
						       cookie, oid,
						       &tmp_obj_impl,
						       &ev);
	}

	if(!obj_impl)
		CORBA_free(oid);

	CORBA_exception_free(&ev);

	return TRUE;

 errout:
	if(ev._major == CORBA_SYSTEM_EXCEPTION) {
		GIOPSendBuffer *reply_buf;

		reply_buf =
			giop_send_reply_buffer_use(GIOP_MESSAGE_BUFFER(recv_buffer)->connection,
						   NULL,
						   recv_buffer->message.u.request.request_id,
						   CORBA_SYSTEM_EXCEPTION);

		 ORBit_send_system_exception(reply_buf, &ev);

		 giop_send_buffer_write(reply_buf);
		 giop_send_buffer_unuse(reply_buf);
	} else /* User exceptions are handled in the skels! */
		g_assert(ev._major == CORBA_NO_EXCEPTION);

	if(!obj_impl)
		CORBA_free(oid);

	CORBA_exception_free(&ev);

	return TRUE;
}

PortableServer_POA
ORBit_POA_find_POA_for_object_key(PortableServer_POA root_poa,
				  CORBA_sequence_octet *key)
{
	CORBA_unsigned_long pid;

	if(key->_length < (sizeof(CORBA_unsigned_long) + ORBIT_RAND_KEY_LEN))
		return NULL;

	pid = *((CORBA_unsigned_long *)key->_buffer);

	if(pid < root_poa->orb->poas->len) {
		PortableServer_POA poa;
		poa = g_ptr_array_index(root_poa->orb->poas, pid);
		if(!poa)
			return NULL;
		if(memcmp(poa->rand_data, key->_buffer + sizeof(CORBA_unsigned_long), ORBIT_RAND_KEY_LEN))
			return NULL;
		return poa;
	} else
		return NULL;
}

void
ORBit_POA_find_object_key_for_oid(PortableServer_POA poa,
				  ORBit_POAObject *obj,
				  PortableServer_ObjectId *oid,
				  CORBA_sequence_octet *retval)
{
	CORBA_long *vptr;

	g_return_if_fail(poa && (obj || oid));
	g_return_if_fail(retval);

	if(oid)
		g_assert(!oid->_buffer[oid->_length - 1]);

	if(obj)
		retval->_length = POA_KEY_LEN + OBJ_KEY_LEN;
	else
		retval->_length = POA_KEY_LEN  + sizeof(CORBA_long) + oid->_length;
	retval->_buffer = CORBA_octet_allocbuf(retval->_length);
	CORBA_sequence_set_release(retval, CORBA_TRUE);

	vptr = (CORBA_long *)retval->_buffer;
	*vptr = poa->poaID;
	memcpy(retval->_buffer + sizeof(CORBA_unsigned_long), poa->rand_data, ORBIT_RAND_KEY_LEN);

	vptr = (CORBA_long *)(retval->_buffer + POA_KEY_LEN);
	if(obj) {
		*vptr = obj->objnum;
		memcpy(retval->_buffer + POA_KEY_LEN + sizeof(CORBA_unsigned_long), obj->rand_data, ORBIT_RAND_KEY_LEN);
	} else {
		*vptr = -((CORBA_long)oid->_length);
		memcpy(retval->_buffer + POA_KEY_LEN + sizeof(CORBA_unsigned_long), oid->_buffer, oid->_length);
	}
}

ORBit_POAObject *
ORBit_POA_find_oid_for_object_key(PortableServer_POA poa,
				  CORBA_sequence_octet *object_key,
				  PortableServer_ObjectId **oid)
{
	CORBA_long onum;
	guchar *nptr;
	ORBit_POAObject *objinfo;

	*oid = NULL;
	nptr = object_key->_buffer + POA_KEY_LEN;

	if(object_key->_length < (POA_KEY_LEN + sizeof(CORBA_long))) {
		return NULL;
	}

	onum = *((CORBA_long *)nptr);

	if(onum < 0) {
		/* onum will be the -strlen(ObjectId) */
		if(object_key->_length < (POA_KEY_LEN + sizeof(CORBA_long) - onum))
			return NULL;

		*oid = (PortableServer_ObjectId *)CORBA_sequence_octet__alloc();
		(*oid)->_length = -onum;
		(*oid)->_buffer = CORBA_octet_allocbuf((*oid)->_length);
		memcpy((*oid)->_buffer, object_key->_buffer + POA_KEY_LEN + sizeof(CORBA_long), (*oid)->_length);

		return NULL;
	}

	if(onum >= poa->objnum_to_obj->len)
		return NULL;

	objinfo = g_ptr_array_index(poa->objnum_to_obj, onum);

	if(GPOINTER_TO_UINT(objinfo) <= poa->objnum_to_obj->len)
		return NULL;

	if(object_key->_length < (POA_KEY_LEN + OBJ_KEY_LEN))
		return NULL;

	if(memcmp(object_key->_buffer + POA_KEY_LEN + sizeof(CORBA_long), objinfo->rand_data, ORBIT_RAND_KEY_LEN))
		return NULL;

	return objinfo;
}

DEFINE_LOCK(id_assignment_counter);
static int id_assignment_counter = 0;

PortableServer_ObjectId *
ORBit_POA_allocate_oid(PortableServer_POA poa,
		       const char *basis)
{
	PortableServer_ObjectId *new_objid;
	char buf[512];
	int len;

	new_objid = (PortableServer_ObjectId *)CORBA_sequence_octet__alloc();

	GET_LOCK(id_assignment_counter);
	g_snprintf(buf, sizeof(buf), "%s%d", basis?basis:"Object",
		   id_assignment_counter);
	id_assignment_counter++;
	RELEASE_LOCK(id_assignment_counter);

	len = strlen(buf)+1;
	new_objid->_buffer = CORBA_octet_allocbuf(len);
	new_objid->_length = len;
	new_objid->_maximum = len;
	new_objid->_release = CORBA_TRUE;

	strcpy((CORBA_char *)new_objid->_buffer, buf);

	return new_objid;
}

static PortableServer_Servant
ORBit_POA_ServantManager_use_servant(PortableServer_POA poa,
				     GIOPRecvBuffer *recv_buffer,
				     PortableServer_ServantLocator_Cookie *the_cookie,
				     PortableServer_ObjectId *oid,
				     ORBit_POAObject *fake_obj_impl,
				     CORBA_Environment *ev)
{
	if(poa->servant_retention == PortableServer_RETAIN) {
		POA_PortableServer_ServantActivator *sm;
		POA_PortableServer_ServantActivator__epv *epv;
		
		sm = (POA_PortableServer_ServantActivator *)poa->servant_manager;
		epv = sm->vepv->PortableServer_ServantActivator_epv;
		return epv->incarnate(sm, oid, poa, ev);
	} else {
		POA_PortableServer_ServantLocator *sm;
		POA_PortableServer_ServantLocator__epv *epv;
		PortableServer_ServantBase *retval;

		sm = (POA_PortableServer_ServantLocator *)poa->servant_manager;
		epv = sm->vepv->PortableServer_ServantLocator_epv;
		retval = epv->preinvoke(sm, oid,
					poa, recv_buffer->message.u.request.operation,
					the_cookie,
					ev);
		if(!retval)
			return NULL;

		((ORBit_ObjectKey *)retval->_private)->object = fake_obj_impl;
		fake_obj_impl->object_id = oid;
		fake_obj_impl->poa = poa;
		fake_obj_impl->orb = poa->orb;
		fake_obj_impl->objnum = -1;
#ifdef NOT_BACKWARDS_COMPAT_0_4
		fake_obj_impl->use_count = NULL;
		fake_obj_impl->death_callback = NULL;
#endif

		return retval;
	}
}

static void
ORBit_POA_ServantManager_unuse_servant(PortableServer_Servant servant,
				       PortableServer_POA poa,
				       GIOPRecvBuffer *recv_buffer,
				       PortableServer_ServantLocator_Cookie cookie,
				       PortableServer_ObjectId *oid,
				       ORBit_POAObject *fake_obj_impl,
				       CORBA_Environment *ev)
{
	POA_PortableServer_ServantLocator *sm;
	POA_PortableServer_ServantLocator__epv *epv;

	if(poa->servant_retention != PortableServer_NON_RETAIN)
		return;

	sm = (POA_PortableServer_ServantLocator *)poa->servant_manager;
	epv = sm->vepv->PortableServer_ServantLocator_epv;
	
	((ORBit_ObjectKey *)((PortableServer_ServantBase *)servant)->_private)->object = NULL;
	epv->postinvoke(sm, oid,
			poa, recv_buffer->message.u.request.operation,
			cookie, servant, ev);

}

typedef struct {
	PortableServer_POA poa;
	CORBA_Environment *ev;
} EtherealizeInfo;

void
ORBit_POA_etherealize_object(PortableServer_ObjectId *oid,
			     ORBit_POAObject *obj_impl,
			     EtherealizeInfo *ei)
{
	POA_PortableServer_ServantActivator__epv *epv;
	POA_PortableServer_ServantActivator *sm;

	g_assert(ei->poa->servant_manager);

	g_hash_table_remove(ei->poa->active_object_map,
			    obj_impl->object_id);

	sm = (POA_PortableServer_ServantActivator *)ei->poa->servant_manager;
	epv = sm->vepv->PortableServer_ServantActivator_epv;
	epv->etherealize(sm, obj_impl->object_id, ei->poa,
			 obj_impl->servant,
			 CORBA_TRUE, CORBA_FALSE, ei->ev);
}

void
ORBit_POA_etherealize_objects(PortableServer_POA poa,
			      CORBA_Environment *ev)
{
	EtherealizeInfo ei;

	ei.poa = poa;
	ei.ev = ev;

	if(poa->servant_retention == PortableServer_RETAIN
	   && poa->request_processing == PortableServer_USE_SERVANT_MANAGER) {

		g_hash_table_foreach(poa->active_object_map,
				     (GHFunc)ORBit_POA_etherealize_object,
				     &ei);
	}
}

#ifdef NOT_BACKWARDS_COMPAT_0_4
void ORBit_servant_set_deathwatch(PortableServer_ServantBase *servant,
				  int *use_count,
                                  GFunc death_func,
                                  gpointer user_data)
{
	ORBit_POAObject *pobj;

	pobj = ORBIT_OBJECT_KEY(servant->_private)->object;

	pobj->use_count = use_count;
	pobj->death_callback = death_func;
	pobj->user_data = user_data;
}
#endif
