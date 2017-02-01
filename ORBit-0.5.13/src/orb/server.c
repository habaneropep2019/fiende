/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* Elliot's stuff */
/* This is somewhat a mess, because I tried to make it easy to add
   select() support, and as a result #ifdef's litter the land. */

#include "orbit.h"
#include "orbit_poa.h"
#include "orbit_poa_type.h"
#include <IIOP/IIOP-private.h>
#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>

/* We need:
   a way to find out what FD's need to be selected on
   a dummy main loop to implement the CORBA_ORB_run() routine;
*/

gboolean orb_server_keep_running = FALSE;

ORBit_request_validate ORBIT_request_validator = NULL;

/* function protos */
static PortableServer_POA ORBit_find_POA_for_request(PortableServer_POA poa,
						     GIOPRecvBuffer *recv_buffer);
static PortableServer_POA ORBit_find_POA_for_locate_request(PortableServer_POA poa,
						     GIOPRecvBuffer *recv_buffer);

static void ORBit_handle_incoming_message(GIOPRecvBuffer *recv_buffer);

void
ORBit_custom_run_setup(CORBA_ORB orb, CORBA_Environment *ev)
{
	IIOPIncomingMessageHandler = ORBit_handle_incoming_message;
}

void
CORBA_ORB_run(CORBA_ORB orb, CORBA_Environment *ev)
{
	ORBit_custom_run_setup(orb, ev);

	orb_server_keep_running = TRUE;

	giop_main();
}

static void
ORBit_handle_incoming_request(GIOPRecvBuffer *recv_buffer)
{
	CORBA_ORB orb;
	PortableServer_POA poa;
	GIOPConnection *connection;
	ORBit_MessageValidationResult mvr;
	gboolean do_unuse = TRUE;

	g_assert(recv_buffer);

	connection = GIOP_MESSAGE_BUFFER(recv_buffer)->connection;
	g_return_if_fail(connection != NULL);

	orb = connection->orb_data;

	g_return_if_fail(orb != NULL);

	ORBit_Trace(TraceMod_ORB, TraceLevel_Debug,
		    "Received request %s, id %d, on %s",
		    recv_buffer->message.u.request.operation,
		    recv_buffer->message.u.request.request_id,
		    recv_buffer->message.u.request.object_key._buffer);

	if(ORBIT_request_validator)
		mvr = ORBIT_request_validator(recv_buffer->message.u.request.request_id,
					      &recv_buffer->message.u.request.requesting_principal,
					      recv_buffer->message.u.request.operation);
	else
		mvr = ORBIT_MESSAGE_ALLOW;

	if(mvr == ORBIT_MESSAGE_ALLOW_ALL)
		connection->is_auth = TRUE;

	if(mvr != ORBIT_MESSAGE_BAD) {
		/* Find the POA for this incoming request */
		poa = ORBit_find_POA_for_request((PortableServer_POA)orb->root_poa,
						 recv_buffer);
			
		if(poa)
			do_unuse = ORBit_POA_handle_request(recv_buffer, poa);
		else
			g_warning("No POA found for operation %s [%d]",
				  recv_buffer->message.u.request.operation,
				  recv_buffer->message.u.request.request_id);
	} else {
		g_warning("Request %s, ID %d was rejected by the authentication mechanism!",
			  recv_buffer->message.u.request.operation,
			  recv_buffer->message.u.request.request_id);
	}

	if(do_unuse)
		giop_recv_buffer_unuse(recv_buffer);
}

static void
ORBit_handle_incoming_locate_request(GIOPRecvBuffer *recv_buffer)
{
	CORBA_ORB orb;
	PortableServer_POA poa;
	GIOPConnection *connection;
	GIOPSendBuffer *send_buffer;

	g_assert(recv_buffer!=NULL);

	connection = GIOP_MESSAGE_BUFFER(recv_buffer)->connection;
	g_return_if_fail(connection != NULL);

	orb = connection->orb_data;

	g_return_if_fail(orb != NULL);

	ORBit_Trace(TraceMod_ORB, TraceLevel_Debug,
		    "Received locate request id %d, on %s",
		    recv_buffer->message.u.locate_request.request_id,
		    recv_buffer->message.u.locate_request.object_key._buffer);
	/* Find the POA for this incoming request */
	poa = ORBit_find_POA_for_locate_request((PortableServer_POA)orb->root_poa, recv_buffer);
		
	if(poa) {
		/* Object found, reply with "Object Here" */
		send_buffer = giop_send_locate_reply_buffer_use(connection,
			 recv_buffer->message.u.locate_request.request_id,
			 GIOP_OBJECT_HERE);
		giop_send_buffer_write(send_buffer);
		giop_send_buffer_unuse(send_buffer);
	} else {
		/* Object not found, reply with "Unknown Object" */
		send_buffer = giop_send_locate_reply_buffer_use(connection,
			 recv_buffer->message.u.locate_request.request_id,
			 GIOP_UNKNOWN_OBJECT);
		giop_send_buffer_write(send_buffer);
		giop_send_buffer_unuse(send_buffer);
	}

	giop_recv_buffer_unuse(recv_buffer);
}

static void
ORBit_handle_incoming_message(GIOPRecvBuffer *recv_buffer)
{
	GIOPConnection *connection;

	g_assert(recv_buffer);

	connection = GIOP_MESSAGE_BUFFER(recv_buffer)->connection;
	g_return_if_fail(connection != NULL);

	switch(GIOP_MESSAGE_BUFFER(recv_buffer)->message_header.message_type) {
	case GIOP_REQUEST:
		ORBit_handle_incoming_request(recv_buffer);
		break;
	case GIOP_LOCATEREQUEST:
		ORBit_handle_incoming_locate_request(recv_buffer);
		break;
	case GIOP_CLOSECONNECTION:
		/* Lame hack - need to do this in a manner that isn't
                   IIOP-specific */
		giop_recv_buffer_unuse(recv_buffer);
		giop_main_handle_connection_exception(connection);
		break;
	case GIOP_REPLY:
		/* the above comment probably applies here also... */
		giop_received_list_push(recv_buffer);
		break;
	default:
		g_warning("discarding message type %d (id possibly %d)",
			  GIOP_MESSAGE_BUFFER(recv_buffer)->message_header.message_type,
			  GIOP_MESSAGE_BUFFER(recv_buffer)->message_header.message_type?recv_buffer->message.u.reply.request_id:recv_buffer->message.u.request.request_id);
		break;
	}
}

static PortableServer_POA
ORBit_find_POA_for_request(PortableServer_POA poa,
			   GIOPRecvBuffer *recv_buffer)
{
	return ORBit_POA_find_POA_for_object_key(poa,
						 &recv_buffer->message.u.request.object_key);
}

static PortableServer_POA
ORBit_find_POA_for_locate_request(PortableServer_POA poa,
			   GIOPRecvBuffer *recv_buffer)
{
	return ORBit_POA_find_POA_for_object_key(poa,
						 &recv_buffer->message.u.locate_request.object_key);
}

void
ORBit_set_request_validation_handler(ORBit_request_validate validator)
{
	ORBIT_request_validator = validator;
}
