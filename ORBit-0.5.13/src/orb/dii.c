/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit: A CORBA v2.2 ORB
 *
 *  Copyright (C) 1998 Red Hat Software, Richard H. Porter
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
 *  Author: Elliot Lee <sopwith@redhat.com>
 *          Dick Porter <dick@cymru.net>
 *
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "orbit.h"
#include "interface_repository.h"

struct CORBA_Request_type {
	struct ORBit_PseudoObject_struct parent;

	CORBA_Object obj;
	CORBA_Context ctx;

	CORBA_Flags req_flags;
	CORBA_Identifier operation;

	CORBA_NamedValue *result;
	CORBA_NVList *arg_list;

	CORBA_unsigned_long request_id;
	GIOPSendBuffer *request_buffer;
	GIOPRecvBuffer *reply_buffer;
};

static const ORBit_RootObject_Interface interface_CORBA_Request = {
	(void (*)(gpointer,CORBA_Environment *))CORBA_Request_delete
};

/* Section 5.2.1 */
CORBA_Status
CORBA_Object_create_request(CORBA_Object obj,
							CORBA_Context ctx,
							CORBA_Identifier operation,
							CORBA_NVList *arg_list,
							CORBA_NamedValue *result,
							CORBA_Request *request,
							CORBA_Flags req_flags,
							CORBA_Environment *ev)
{
	CORBA_Request new;

	new=g_new0(struct CORBA_Request_type, 1);
	ORBit_pseudo_object_init((ORBit_PseudoObject)new, ORBIT_PSEUDO_REQUEST, ev);
	ORBit_RootObject_set_interface((ORBit_RootObject)new,
								   (ORBit_RootObject_Interface *)&interface_CORBA_Request, ev);

	if(new==NULL) {
		CORBA_exception_set_system(ev,
								   ex_CORBA_NO_MEMORY,
								   CORBA_COMPLETED_NO);
		return;
	}

	new->obj=CORBA_Object_duplicate(obj, ev);
	new->ctx=(CORBA_Context)CORBA_Object_duplicate((CORBA_Object)ctx, ev);
	new->operation=CORBA_string_dup(operation);

	new->result=result;

	new->req_flags=req_flags;
	new->request_id = giop_get_request_id();
	new->arg_list = arg_list;

	*request=(CORBA_Request)CORBA_Object_duplicate((CORBA_Object)new, ev);
}

/* Section 5.2, 5.2.2 */
CORBA_Status
CORBA_Request_add_arg(CORBA_Request req,
					  CORBA_Identifier name,
					  CORBA_TypeCode arg_type,
					  void *value,
					  CORBA_long len,
					  CORBA_Flags arg_flags,
					  CORBA_Environment *ev)
{
	gpointer new_value;

	g_assert(req!=NULL);

	if((arg_flags & CORBA_IN_COPY_VALUE) && (arg_flags & CORBA_ARG_IN)) {
		new_value = ORBit_copy_value(value, arg_type);
		if(new_value==NULL) {
			CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY,
									   CORBA_COMPLETED_NO);
			return;
		}
	} else
		new_value=value;

	CORBA_NVList_add_item(req->arg_list, name, arg_type,
						  new_value, len, arg_flags | req->req_flags, ev);
}

/* Section 5.2, 5.2.3 */
CORBA_Status
CORBA_Request_invoke(CORBA_Request req,
					 CORBA_Flags invoke_flags,
					 CORBA_Environment *ev)
{
	CORBA_Request_send(req, invoke_flags, ev);
	if(ev->_major == CORBA_NO_EXCEPTION)
		CORBA_Request_get_response(req, invoke_flags, ev);
}

/* Section 5.2, 5.2.4 */
CORBA_Status CORBA_Request_delete(CORBA_Request req, CORBA_Environment *ev)
{
	CORBA_Object_release(req->obj, ev);
	CORBA_Object_release((CORBA_Object)req->ctx, ev);

	if(req->operation != NULL)
		CORBA_free(req->operation);

	if(req->arg_list != NULL) {
		if(req->req_flags & CORBA_OUT_LIST_MEMORY)
			CORBA_NVList_free(req->arg_list, ev);
		else {
			int i;
			for(i = 0; i < req->arg_list->list->len; i++)
				ORBit_NamedValue_free(&g_array_index(req->arg_list->list,
													 CORBA_NamedValue, i));
			g_array_free(req->arg_list->list, TRUE);
			
			g_free(req->arg_list);
		}
	}

	if(req->result!=NULL)
		ORBit_NamedValue_free(req->result);

	if(req->request_buffer)
		giop_send_buffer_unuse(req->request_buffer);

	if(req->reply_buffer)
		giop_recv_buffer_unuse(req->reply_buffer);

	g_free(req);
}

/* Section 5.2, 5.3.1 */
CORBA_Status
CORBA_Request_send(CORBA_Request req,
				   CORBA_Flags invoke_flags,
				   CORBA_Environment *ev)
{
	int i;
	GIOPConnection *cnx;

	struct { CORBA_unsigned_long opstrlen; char opname[1]; } *opnameinfo;
	struct iovec opvec = { NULL, 0 };

	opvec.iov_len = strlen(req->operation)+1+sizeof(CORBA_unsigned_long);

	opnameinfo = g_malloc(strlen(req->operation)+1+sizeof(CORBA_unsigned_long));
	opvec.iov_base = (gpointer)opnameinfo;
	opnameinfo->opstrlen = strlen(req->operation) + 1;
	strcpy(opnameinfo->opname, req->operation);

	cnx = ORBit_object_get_connection(req->obj);

	g_assert(req->obj->active_profile);
	req->request_buffer =
		giop_send_request_buffer_use(req->obj->connection,
									 NULL,
									 req->request_id,
									 req->result?TRUE:FALSE,
									 &(req->obj->active_profile->object_key_vec),
									 &opvec,
									 &ORBit_default_principal_iovec
									 );

	if(!req->request_buffer) {
		CORBA_exception_set_system(ev,
								   ex_CORBA_COMM_FAILURE,
								   CORBA_COMPLETED_NO);
		return;
	}

	for(i = 0; i < req->arg_list->list->len; i++) {
		CORBA_NamedValue *nv;

		nv = &g_array_index(req->arg_list->list, CORBA_NamedValue, i); 

		if((nv->arg_modes & CORBA_ARG_IN)
		   || (nv->arg_modes & CORBA_ARG_INOUT))
			ORBit_marshal_arg(req->request_buffer, 
							  nv->argument._value,
							  nv->argument._type);
	}
				      
	giop_send_buffer_write(req->request_buffer);

	giop_send_buffer_unuse(req->request_buffer);
	req->request_buffer = 0;

	g_free(opnameinfo);
}

/* Section 5.3.2 */
CORBA_Status
CORBA_send_multiple_requests(CORBA_Request *reqs,
							 CORBA_Environment *env,
							 CORBA_long count,
							 CORBA_Flags invoke_flags)
{
	int i;

	for(i = 0; i < count; i++)
		CORBA_Request_send(reqs[i], invoke_flags, env);	
}

void
ORBit_handle_dii_reply(CORBA_Request req, CORBA_Environment *ev)
{
	int i;

	/* XXX TODO - handle exceptions, location forwards(?), all that */
	req->result->argument._value =
		ORBit_demarshal_arg(req->reply_buffer, req->result->argument._type,
							TRUE, req->obj->orb);
	req->result->argument._release = CORBA_TRUE;
	
	for(i = 0; i < req->arg_list->list->len; i++) {
		CORBA_NamedValue *nv;

		nv = &g_array_index(req->arg_list->list, CORBA_NamedValue, i); 

		if(nv->arg_modes & CORBA_ARG_INOUT) {
			CORBA_Object_duplicate((CORBA_Object)nv->argument._type, NULL);
			CORBA_any__free(&nv->argument, NULL, TRUE);
		}

		if((nv->arg_modes & CORBA_ARG_OUT)
		   || (nv->arg_modes & CORBA_ARG_INOUT))
			nv->argument._value = ORBit_demarshal_arg(req->reply_buffer,
													  nv->argument._type,
													  TRUE, req->obj->orb);
	}

	giop_recv_buffer_unuse(req->reply_buffer);
	req->reply_buffer = 0;
}

/* Section 5.2, 5.3.3
 *
 * Raises: WrongTransaction
 */
CORBA_Status
CORBA_Request_get_response(CORBA_Request req,
						   CORBA_Flags response_flags,
						   CORBA_Environment *ev)
{
	req->reply_buffer = giop_recv_reply_buffer_use(req->request_id,
												   !(response_flags & CORBA_RESP_NO_WAIT));

	if(!req->reply_buffer) {
		CORBA_exception_set_system(ev,
								   ex_CORBA_COMM_FAILURE,
								   CORBA_COMPLETED_NO);
		return;
	}

	ORBit_handle_dii_reply(req, ev);
}

/* Section 5.3.4
 *
 * Raises: WrongTransaction
 */
CORBA_Status
CORBA_Request_get_next_response(CORBA_Environment *env,
								CORBA_Flags response_flags,
								CORBA_Request *req)
{
	int i;
	GIOPRecvBuffer *rb;
	GArray *reqids = g_array_new(FALSE, FALSE,
								 sizeof(CORBA_unsigned_long));

	for(i = 0; req[i]; i++) {
		g_array_append_val(reqids, req[i]->request_id);
	}

	rb = giop_recv_reply_buffer_use_multiple(reqids,
											 !(response_flags & CORBA_RESP_NO_WAIT));

	if(rb) {
		for(i = 0; i < reqids->len; i++) {
			if(g_array_index(reqids, CORBA_unsigned_long, i)
			   == rb->message.u.reply.request_id) {
				req[i]->reply_buffer = rb;
				break;
			}
		}

		if(i < reqids->len)
			ORBit_handle_dii_reply(req[i], env);
	}

	g_array_free(reqids, TRUE);
}


/* Section 5.4.1 */
CORBA_Status
CORBA_ORB_create_list(CORBA_ORB orb,
					  CORBA_long count,
					  CORBA_NVList **new_list,
					  CORBA_Environment *ev)
{
	CORBA_NVList *new;

	new = g_new0(CORBA_NVList, 1);
	if(new==NULL) goto new_alloc_failed;

	new->list = g_array_new(FALSE, TRUE, sizeof(CORBA_NamedValue));

	*new_list = new;

	return;

 new_alloc_failed:
	CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
}

/* Section 5.4.6 */
CORBA_Status
CORBA_ORB_create_operation_list(CORBA_ORB orb,
								CORBA_OperationDef oper,
								CORBA_NVList **new_list,
								CORBA_Environment *ev)
{
	if(!new_list) {
		CORBA_exception_set_system(ev,
								   ex_CORBA_BAD_PARAM,
								   CORBA_COMPLETED_NO);
		return;
	}

	g_warning("CORBA_ORB_create_operation_list NYI");

	CORBA_exception_set_system(ev,
							   ex_CORBA_IMP_LIMIT,
							   CORBA_COMPLETED_NO);
}

/* Section 5.4.2 */
CORBA_Status
CORBA_NVList_add_item(CORBA_NVList *list,
					  CORBA_Identifier item_name,
					  CORBA_TypeCode item_type,
					  void *value,
					  CORBA_long value_len,
					  CORBA_Flags item_flags,
					  CORBA_Environment *ev)
{
	CORBA_NamedValue newval;

	g_assert(list!=NULL);

	newval.name = CORBA_string_dup(item_name);
	newval.argument._type = (CORBA_TypeCode)CORBA_Object_duplicate((CORBA_Object)item_type, ev);
	if(item_flags & CORBA_IN_COPY_VALUE) {
		newval.argument._value = ORBit_copy_value(value, item_type);
		newval.argument._release = CORBA_TRUE;
	} else {
		newval.argument._value = value;
		newval.argument._release = CORBA_FALSE;
	}

	newval.len = value_len; /* Is this even useful? *sigh* */
	newval.arg_modes = item_flags;

	g_array_append_val(list->list, newval);
}

void ORBit_NamedValue_free(CORBA_NamedValue *nv)
{
	CORBA_free(nv->name);
}

/* Section 5.4.3 */
CORBA_Status
CORBA_NVList_free(CORBA_NVList *list,
				  CORBA_Environment *ev)
{
	int i;

	CORBA_NVList_free_memory(list, ev);

	for(i = 0; i < list->list->len; i++)
		ORBit_NamedValue_free(&g_array_index(list->list, CORBA_NamedValue, i));

	g_array_free(list->list, TRUE);

	g_free(list);
}

/* Section 5.4.4 */
CORBA_Status
CORBA_NVList_free_memory(CORBA_NVList *list,
						 CORBA_Environment *ev)
{
	int i;

	for(i = 0; i < list->list->len; i++) {
		CORBA_free(g_array_index(list->list, CORBA_NamedValue, i).argument._value);
		g_array_index(list->list, CORBA_NamedValue, i).argument._value = NULL;
		CORBA_Object_release((CORBA_Object)g_array_index(list->list, CORBA_NamedValue, i).argument._type, ev);
		g_array_index(list->list, CORBA_NamedValue, i).argument._release = CORBA_FALSE;
	}
}


/* Section 5.4.5 */
CORBA_Status
CORBA_NVList_get_count(CORBA_NVList *list,
					   CORBA_long *count,
					   CORBA_Environment *ev)
{
	*count = list->list->len;
}

