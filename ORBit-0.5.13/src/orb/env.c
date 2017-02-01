/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  ORBit: A CORBA v2.2 ORB
 *
 *  Copyright (C) 1998 Richard H. Porter
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
 *
 */

/*
 * CORBA_Environment handling functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "orbit.h"
#include "interface_repository.h"

struct SysExInfo {
	const char *repo_id;
	const int exnum;
};

static const struct SysExInfo exception_table[] = {
	{NULL, 0},
	{"IDL:CORBA/UNKNOWN:1.0", 1},
	{"IDL:CORBA/BAD_PARAM:1.0", 2},
	{"IDL:CORBA/NO_MEMORY:1.0", 3},
	{"IDL:CORBA/IMP_LIMIT:1.0", 4},
	{"IDL:CORBA/COMM_FAILURE:1.0", 5},
	{"IDL:CORBA/INV_OBJREF:1.0", 6},
	{"IDL:CORBA/NO_PERMISSION:1.0", 7},
	{"IDL:CORBA/INTERNAL:1.0", 8},
	{"IDL:CORBA/MARSHAL:1.0", 9},
	{"IDL:CORBA/INITIALIZE:1.0", 10},
	{"IDL:CORBA/NO_IMPLEMENT:1.0", 11},
	{"IDL:CORBA/BAD_TYPECODE:1.0", 12},
	{"IDL:CORBA/BAD_OPERATION:1.0", 13},
	{"IDL:CORBA/NO_RESOURCES:1.0", 14},
	{"IDL:CORBA/NO_RESPONSE:1.0", 15},
	{"IDL:CORBA/PERSIST_STORE:1.0", 16},
	{"IDL:CORBA/BAD_INV_ORDER:1.0", 17},
	{"IDL:CORBA/TRANSIENT:1.0", 18},
	{"IDL:CORBA/FREE_MEM:1.0", 19},
	{"IDL:CORBA/INV_IDENT:1.0", 20},
	{"IDL:CORBA/INV_FLAG:1.0", 21},
	{"IDL:CORBA/INTF_REPOS:1.0", 22},
	{"IDL:CORBA/BAD_CONTEXT:1.0", 23},
	{"IDL:CORBA/OBJ_ADAPTER:1.0", 24},
	{"IDL:CORBA/DATA_CONVERSION:1.0", 25},
	{"IDL:CORBA/OBJECT_NOT_EXIST:1.0", 26},
	{"IDL:CORBA/TRANSACTION_REQUIRED:1.0", 27},
	{"IDL:CORBA/TRANSACTION_ROLLEDBACK:1.0", 28},
	{"IDL:CORBA/INVALID_TRANSACTION:1.0", 29},
	{NULL,0}
};

void CORBA_exception_free(CORBA_Environment *ev)
{
	g_assert(ev!=NULL);

	ev->_major=CORBA_NO_EXCEPTION;

	if(ev->_repo_id) {
		CORBA_free(ev->_repo_id);
		ev->_repo_id=NULL;
	}

	if(ev->_params) {
		CORBA_free(ev->_params);
		ev->_params=NULL;
	}

	if(ev->_any) {
		CORBA_free(ev->_any);
		ev->_any=NULL;
	}
}

void CORBA_exception_set(CORBA_Environment *ev, CORBA_exception_type major, 
			 const CORBA_char *except_repos_id, void *param)
{
	g_assert(ev!=NULL);

	if(ev->_major != CORBA_NO_EXCEPTION)
		CORBA_exception_free(ev);

	ev->_major=major;

	if(except_repos_id==NULL) {
		ev->_repo_id=NULL;
	} else {
		ev->_repo_id=CORBA_string_dup(except_repos_id);
	}

	ev->_params=param;
}

void CORBA_exception_set_system(CORBA_Environment *ev, CORBA_unsigned_long ex_value,
				CORBA_completion_status completed)
{
	CORBA_SystemException *new;

	new=ORBit_alloc(sizeof(CORBA_SystemException), NULL, NULL);
	if(new!=NULL) {
		new->minor=0;
		new->completed=completed;

		/* XXX what should the repo ID be? */
		CORBA_exception_set(ev, CORBA_SYSTEM_EXCEPTION,
				    exception_table[ex_value].repo_id,
				    new);
	}
}

void CORBA_exception_init(CORBA_Environment *ev)
{
	g_assert(ev!=NULL);

	ev->_major=CORBA_NO_EXCEPTION;
	ev->_repo_id=NULL;
	ev->_params=NULL;
	ev->_any=NULL;
}

CORBA_char *CORBA_exception_id(CORBA_Environment *ev)
{
	g_assert(ev!=NULL);

	if(ev->_major==CORBA_NO_EXCEPTION) {
		return(NULL);
	} else {
		return(ev->_repo_id);
	}
}

void *CORBA_exception_value(CORBA_Environment *ev)
{
	g_assert(ev!=NULL);

	if(ev->_major==CORBA_NO_EXCEPTION) {
		return(NULL);
	} else {
		return(ev->_params);
	}
}

CORBA_any *CORBA_exception_as_any(CORBA_Environment *ev)
{
	g_assert(ev!=NULL);

	if(ev->_major==CORBA_NO_EXCEPTION) {
		return(NULL);
	}

	if(ev->_any!=NULL) {
		return(ev->_any);
	}

	ev->_any=g_new(CORBA_any, 1);
	if(ev->_any!=NULL) {
		/* XXX is this the correct type? */
		ev->_any->_type = (CORBA_TypeCode)TC_CORBA_ExceptionDescription;
		ev->_any->_value = ev->_params;
		ev->_any->_release = 0;
	}

	return(ev->_any);
}

/**** ORBit_handle_exception
      Inputs: 'rb' - a receive buffer for which an exception condition has
                     been determined
	      'ev' - memory in which to store the exception information

	      'user_exceptions' -     list of user exceptions raisable
				      for this particular operation.
      Side-effects: reinitializes '*ev'

      Description:
           During demarshalling a reply, if reply_status != CORBA_NO_EXCEPTION,
	   we must find out what exception was raised and place that information
	   in '*ev'.  */

void ORBit_handle_exception(GIOPRecvBuffer *rb, CORBA_Environment *ev,
			    const ORBit_exception_demarshal_info *user_exceptions,
			    CORBA_ORB orb)
{
	CORBA_SystemException *new;
	CORBA_unsigned_long len, completion_status;
	CORBA_char *my_repoid;

	g_return_if_fail(GIOP_MESSAGE_BUFFER(rb)->message_header.message_type == GIOP_REPLY);

	CORBA_exception_free(ev);

	rb->cur = ALIGN_ADDRESS(rb->cur, sizeof(len));
	rb->decoder(&len, rb->cur, sizeof(len));
	/* (guchar *)rb->cur += sizeof(len); */
	rb->cur = ((guchar *)rb->cur) + sizeof(len);

	if(len) {
		my_repoid = rb->cur;
		rb->cur = ((guchar *)rb->cur) + len;
	} else
		my_repoid = NULL;

	if(rb->message.u.reply.reply_status == CORBA_SYSTEM_EXCEPTION) {
		CORBA_unsigned_long minor;

		ev->_major = CORBA_SYSTEM_EXCEPTION;

		rb->cur = ALIGN_ADDRESS(rb->cur, sizeof(minor));
		rb->decoder(&minor, rb->cur, sizeof(minor));
		rb->cur = ((guchar *)rb->cur) + sizeof(minor);

		rb->cur = ALIGN_ADDRESS(rb->cur, sizeof(completion_status));
		rb->decoder(&completion_status, rb->cur, sizeof(completion_status));
		rb->cur = ((guchar *)rb->cur) + sizeof(completion_status);

		new=ORBit_alloc(sizeof(CORBA_SystemException), NULL, NULL);

		if(new!=NULL) {
			new->minor=minor;
			new->completed=completion_status;
			
		/* XXX what should the repo ID be? */
		CORBA_exception_set(ev, CORBA_SYSTEM_EXCEPTION,
				    my_repoid,
				    new);
		}
	} else if(rb->message.u.reply.reply_status == CORBA_USER_EXCEPTION) {
		int i;

		if(!user_exceptions) {
			/* weirdness; they raised an exception that we don't
			   know about */
			CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
						   CORBA_COMPLETED_MAYBE);
		} else {
			for(i = 0; user_exceptions[i].tc != CORBA_OBJECT_NIL;
			    i++) {
				if(!strcmp(user_exceptions[i].tc->repo_id,
					   my_repoid))
					break;
			}

			if(user_exceptions[i].tc == CORBA_OBJECT_NIL) {
				/* weirdness; they raised an exception
				   that we don't know about */
				CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
							   CORBA_COMPLETED_MAYBE);
			} else {
				user_exceptions[i].demarshal(rb, ev);
			}
		}
	};

	/* ignore LOCATION_FORWARD here, that gets handled in the stub */
}

void
ORBit_send_system_exception(GIOPSendBuffer *send_buffer,
			    CORBA_Environment *ev)
{
	CORBA_unsigned_long minor;
	CORBA_unsigned_long completion_status;
	CORBA_SystemException *se = ev->_params;

	minor = se->minor;
	completion_status = se->completed;

	ENCODER_CALL(CORBA_char, ev->_repo_id);
	giop_send_buffer_append_mem_indirect_a(send_buffer, &minor,
					       sizeof(minor));
	giop_send_buffer_append_mem_indirect_a(send_buffer,
					       &completion_status,
					       sizeof(completion_status));
}

void
ORBit_send_user_exception(GIOPSendBuffer *send_buffer,
			  CORBA_Environment *ev,
			  const ORBit_exception_marshal_info *user_exceptions)
{
	int i;

	for(i = 0; user_exceptions[i].tc != CORBA_OBJECT_NIL; i++) {
		if(!strcmp(user_exceptions[i].tc->repo_id, ev->_repo_id))
			break;
	}

	if(user_exceptions[i].tc == CORBA_OBJECT_NIL) {
		CORBA_Environment fakeev;
		CORBA_exception_init(&fakeev);
		CORBA_exception_set_system(&fakeev, ex_CORBA_UNKNOWN,
					   CORBA_COMPLETED_MAYBE);
		ORBit_send_system_exception(send_buffer, &fakeev);
		CORBA_exception_free(&fakeev);
	} else {
		ENCODER_CALL(CORBA_char, ev->_repo_id);

		if(user_exceptions[i].marshal && ev->_params)
			user_exceptions[i].marshal(send_buffer, ev);
	}
}

void
ORBit_handle_system_exception(CORBA_Environment *ev,
			      CORBA_unsigned_long system_exception_minor,
			      CORBA_unsigned_long completion_status,
			      GIOPRecvBuffer *recv_buffer,
			      GIOPSendBuffer *send_buffer)
{
	CORBA_exception_set_system(ev, system_exception_minor, completion_status);

	if(send_buffer)
		giop_send_buffer_unuse(send_buffer);

	if(recv_buffer)
		giop_recv_buffer_unuse(recv_buffer);
}
