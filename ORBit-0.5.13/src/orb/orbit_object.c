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
 *          Elliot Lee <sopwith@redhat.com>
 *
 */

/*
 *   ORBit specific CORBA_Object functions.
 *
 */

#undef PROFILE_DEBUG

#include <string.h>
#include "config.h"
#include "../IIOP/iiop-endianP.h"
#include "orbit_object_type.h"
#include "corba_object_type.h"
#include "allocators.h"
#include "iop.h"
#include <IIOP/IIOP.h>

static void ORBit_object_try_existing_connections(CORBA_Object obj);
static void CORBA_Object_release_fn(CORBA_Object obj, CORBA_Environment *ev);

static ORBit_RootObject_Interface CORBA_Object_epv =
{
	(void (*)(gpointer, CORBA_Environment *))CORBA_Object_release_fn,
};

void ORBit_pseudo_object_init(ORBit_PseudoObject obj,
			      ORBit_PseudoObject_type obj_type,
			      CORBA_Environment *ev)
{
	ORBIT_ROOT_OBJECT(obj)->is_pseudo_object = TRUE;
	ORBIT_ROOT_OBJECT(obj)->refs = 0;
	ORBIT_PSEUDO_OBJECT(obj)->pseudo_object_type = obj_type;
}

static void ORBit_Policy_release(CORBA_Policy policy,
				 CORBA_Environment *ev);

static const ORBit_RootObject_Interface CORBA_Policy__epv =
{
	(void (*)(gpointer, CORBA_Environment *))ORBit_Policy_release
};

void ORBit_policy_object_init(CORBA_Policy obj,
			      CORBA_PolicyType obj_type,
			      CORBA_Environment *ev)
{
	ORBit_pseudo_object_init(ORBIT_PSEUDO_OBJECT(obj),
				 ORBIT_PSEUDO_POLICY, ev);

	obj->policy_type = obj_type;

	ORBit_RootObject_set_interface(ORBIT_ROOT_OBJECT(obj),
				       (gpointer)&CORBA_Policy__epv,
				       ev);
}

/* Release method implementation for CORBA_Policy */
static void
ORBit_Policy_release(CORBA_Policy policy,
		     CORBA_Environment *ev)
{
	if(--(ORBIT_ROOT_OBJECT(policy)->refs) > 0)
		return;

	g_free(policy);
}

void ORBit_object_reference_init(CORBA_Object obj, CORBA_Environment *ev)
{
	/* set the interface up */
	ORBit_RootObject_set_interface(ORBIT_ROOT_OBJECT(obj),&CORBA_Object_epv,ev);
	/* initialise the reference count */
	ORBIT_ROOT_OBJECT(obj)->refs = 0; 
	ORBIT_ROOT_OBJECT(obj)->is_pseudo_object = FALSE;
}

CORBA_Object 
ORBit_CORBA_Object_new(CORBA_Environment *ev)
{
	CORBA_Object obj;
	/* Create the object */
	obj = ORBIT_CHUNK_ALLOC(CORBA_Object);
	memset(obj, '\0', sizeof(struct CORBA_Object_struct));

	ORBit_object_reference_init(obj, ev);

	return obj;

}

void
ORBit_set_object_key(ORBit_Object_info *info)
{
	g_assert(info);

	g_assert(info->object_key._length);

	info->object_key_data = g_malloc(sizeof(CORBA_unsigned_long) + info->object_key._length);
	info->object_key_data->_length = info->object_key._length;
	memcpy(info->object_key_data->_buffer, info->object_key._buffer, info->object_key._length);

	info->object_key_vec.iov_base =
		(gpointer)info->object_key_data;
	info->object_key_vec.iov_len = sizeof(CORBA_unsigned_long) + info->object_key._length;
}

static void ORBit_free_profile(gpointer item, gpointer data)
{
	ORBit_Object_info *info=(ORBit_Object_info *)item;

	g_assert(info);

	g_free(info->object_key_data);
	CORBA_free(info->object_key._buffer);

	if(info->profile_type == IOP_TAG_INTERNET_IOP) {
		g_free(info->tag.iopinfo.host);
	} else if(info->profile_type == IOP_TAG_ORBIT_SPECIFIC) {
		g_free(info->tag.orbitinfo.unix_sock_path);
	} else {
#ifdef PROFILE_DEBUG
		g_warning("ORBit_free_profile asked to free type %d", info->profile_type);
#endif
	}

	g_free(info);	/* Check its safe to free the item within a foreach func */
}

void ORBit_delete_profiles(GSList *profile_list)
{
	g_slist_foreach(profile_list, ORBit_free_profile, NULL);
	g_slist_free(profile_list);
}

/* this function is wired up to the RootObject interface */
void 
CORBA_Object_release_fn(CORBA_Object obj, CORBA_Environment *ev)
{

	g_assert(obj!=NULL);

	ORBIT_ROOT_OBJECT_UNREF(obj);

	if(ORBIT_ROOT_OBJECT(obj)->refs <= 0) {
		g_hash_table_remove(obj->orb->objrefs, obj);
		
		if(obj->connection)
			giop_connection_unref(obj->connection);

		g_free(obj->object_id);
		ORBit_delete_profiles(obj->profile_list);
		ORBit_delete_profiles(obj->forward_locations);

		g_free(obj->vepv);

		ORBIT_CHUNK_FREE(CORBA_Object, obj);
	}
}


/* Sets the interface member in the RootObject to the epv specified*/
void
ORBit_RootObject_set_interface(ORBit_RootObject obj,
			       ORBit_RootObject_Interface* epv,
			       CORBA_Environment *ev)
{
	g_assert(obj!=NULL);
	g_assert(epv!=NULL);

	obj->interface = epv;
}

#define GET_ATOM(x) G_STMT_START{ GIOP_RECV_BUFFER(recv_buffer)->decoder(&x, (GIOP_RECV_BUFFER(recv_buffer)->cur), sizeof(x)); \
GIOP_RECV_BUFFER(recv_buffer)->cur = ((guchar *)GIOP_RECV_BUFFER(recv_buffer)->cur) + sizeof(x); \
}G_STMT_END
#define ALIGNFOR(x) recv_buffer->cur = ALIGN_ADDRESS(recv_buffer->cur, sizeof(x))

CORBA_Object
ORBit_create_object_with_info(GSList *profiles,
			      const CORBA_char *type_id,
			      CORBA_ORB orb,
			      CORBA_Environment *ev)
{
	CORBA_Object new;
	struct CORBA_Object_struct refcheck;

	if(!type_id || !*type_id) {
		g_warning("Failing object creation because object has no type");
		CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
					   CORBA_COMPLETED_MAYBE);
		return CORBA_OBJECT_NIL;
	}

	if(g_slist_length(profiles) < 1) {
		g_warning("Failing object creation because object has no profiles");
		CORBA_exception_set_system(ev, ex_CORBA_MARSHAL,
					   CORBA_COMPLETED_MAYBE);
		return CORBA_OBJECT_NIL;
	}

	/* XXX badhack :) */
	refcheck.object_id = (char *)type_id;
	refcheck.profile_list = profiles;

	new = g_hash_table_lookup(orb->objrefs, &refcheck);
	if(new) {
		ORBit_delete_profiles(profiles);
		return CORBA_Object_duplicate(new, ev);
	}

	new = ORBit_CORBA_Object_new(ev);
	new->connection = NULL;
 	new->object_id = g_strdup(type_id);
	new->orb = (CORBA_ORB)CORBA_Object_duplicate((CORBA_Object)orb, ev);
	new->profile_list = profiles;
	new->active_profile = NULL;

	ORBit_object_try_existing_connections(new);

	g_hash_table_insert(orb->objrefs, new, new);
	
	return CORBA_Object_duplicate(new, ev);
}

static ORBit_Object_info *
ORBit_demarshal_profile(GIOPRecvBuffer *recv_buffer, IOP_ProfileId profile_id)
{
	ORBit_Object_info *object_info;
	CORBA_unsigned_long subpart_len;
	CORBA_octet o;
	CDR_Codec codec_d;
	CDR_Codec *codec=&codec_d;

	object_info = g_new0(ORBit_Object_info, 1);

	switch(profile_id) {
	case IOP_TAG_INTERNET_IOP:
		GET_ATOM(subpart_len); /* The length of the embedded sequence */
		CDR_codec_init_static(codec);
		codec->buffer = recv_buffer->cur;
		codec->release_buffer = CORBA_FALSE;
		recv_buffer->cur = ((guchar *)recv_buffer->cur) + subpart_len;

		codec->readonly = CORBA_TRUE;
		codec->host_endian = codec->data_endian = FLAG_ENDIANNESS;
		codec->buf_len = subpart_len;

		CDR_get_octet(codec, &o);
		codec->data_endian = o;

		object_info->profile_type = IOP_TAG_INTERNET_IOP;
		CDR_get_octet(codec, &object_info->iiop_major);

		if(object_info->iiop_major != 1)
			goto error_exit;
		/* XXX should we check for a specific minor version? */
		CDR_get_octet(codec, &object_info->iiop_minor);

		CDR_get_string(codec, &object_info->tag.iopinfo.host);

		CDR_get_ushort(codec, &object_info->tag.iopinfo.port);

		CDR_get_seq_begin(codec, &object_info->object_key._length);
		object_info->object_key._buffer =
			ORBit_alloc(object_info->object_key._length, NULL, NULL);
		CDR_buffer_gets(codec, object_info->object_key._buffer,
				object_info->object_key._length);
		object_info->object_key._maximum = object_info->object_key._release = 0;

		ORBit_set_object_key(object_info);

		return(object_info);
		break;

	default:
		g_warning("Unknown IOP profile");

	case IOP_TAG_GENERIC_IOP:
	case IOP_TAG_MULTIPLE_COMPONENTS:
		/* FIXME: IOP_TAG_MULTIPLE_COMPONENTS needs implementing */
		GET_ATOM(subpart_len);
		recv_buffer->cur = ((guchar *)recv_buffer->cur) + subpart_len;
		g_free (object_info);
		return NULL;
		break;

	case IOP_TAG_ORBIT_SPECIFIC:
		GET_ATOM(subpart_len);
		CDR_codec_init_static(codec);
		codec->buffer = recv_buffer->cur;
		codec->release_buffer = CORBA_FALSE;
		recv_buffer->cur = ((guchar *)recv_buffer->cur) + subpart_len;

		codec->readonly = CORBA_TRUE;
		codec->host_endian = codec->data_endian = FLAG_ENDIANNESS;
		codec->buf_len = subpart_len;

		CDR_get_octet(codec, &o);
		codec->data_endian = o;

		object_info->profile_type = IOP_TAG_ORBIT_SPECIFIC;
		CDR_get_octet(codec, &object_info->iiop_major);

		if(object_info->iiop_major != 1)
			goto error_exit;
		/* XXX should we check for a specific minor version? */
		CDR_get_octet(codec, &object_info->iiop_minor);

		CDR_get_string(codec, &object_info->tag.orbitinfo.unix_sock_path);
		CDR_get_ushort(codec, &object_info->tag.orbitinfo.ipv6_port);

		CDR_get_seq_begin(codec, &object_info->object_key._length);
		object_info->object_key._buffer =
			ORBit_alloc(object_info->object_key._length, NULL, NULL);
		CDR_buffer_gets(codec, object_info->object_key._buffer,
				object_info->object_key._length);
		object_info->object_key._maximum = object_info->object_key._release = 0;

		ORBit_set_object_key(object_info);

		return(object_info);
		break;
	}

error_exit:
	g_message("demarshal_profile(): IIOP major is %d",
		  object_info->iiop_major);
	g_free(object_info);

	return(NULL);
}

GSList *ORBit_demarshal_IOR(GIOPRecvBuffer *recv_buffer)
{
	GSList *profiles=NULL;
	ORBit_Object_info *object_info;
	CORBA_unsigned_long len, seq_len;
	IOP_ProfileId profile_id;
	int i;

	/* Get type_id */
	ALIGNFOR(CORBA_unsigned_long);
	GET_ATOM(len);

	if(len == 0)
		return(NULL);

	recv_buffer->cur = ((guchar *)recv_buffer->cur) + len;

	/* Decode the sequence<TaggedProfile> */
	ALIGNFOR(CORBA_unsigned_long);
	GET_ATOM(seq_len);
	for(i = 0; i < seq_len; i++) {
		ALIGNFOR(CORBA_unsigned_long);
		GET_ATOM(profile_id);
		object_info=ORBit_demarshal_profile(recv_buffer, profile_id);
		if (object_info)
			profiles=g_slist_append(profiles, object_info);
	}

	return(profiles);

error_exit:
	ORBit_delete_profiles(profiles);
	return(NULL);
}

CORBA_Object
ORBit_demarshal_object(GIOPRecvBuffer *recv_buffer, CORBA_ORB orb)
{
	GSList *profiles=NULL;
	ORBit_Object_info *object_info;
	CORBA_char *type_id;
	CORBA_unsigned_long len, seq_len;
	IOP_ProfileId profile_id;
	int i;
	CORBA_Environment ev;
	CORBA_Object retval;

	CORBA_exception_init(&ev);

	/* Get type_id */
	ALIGNFOR(CORBA_unsigned_long);
	GET_ATOM(len);

	type_id = recv_buffer->cur;
	recv_buffer->cur = ((guchar *)recv_buffer->cur) + len;

	/* Decode the sequence<TaggedProfile> */
	ALIGNFOR(CORBA_unsigned_long);
	GET_ATOM(seq_len);

	if(!seq_len)
		return CORBA_OBJECT_NIL;

	for(i = 0; i < seq_len; i++) {
		ALIGNFOR(CORBA_unsigned_long);
		GET_ATOM(profile_id);
		object_info=ORBit_demarshal_profile(recv_buffer, profile_id);
		if(object_info)
			profiles=g_slist_append(profiles, object_info);
	}

	if(!profiles)
		goto error_exit;

	retval = ORBit_create_object_with_info(profiles, type_id, orb, &ev);

	return retval;

 error_exit:
	ORBit_delete_profiles(profiles);

	CORBA_exception_set_system(&ev, ex_CORBA_MARSHAL,
				   CORBA_COMPLETED_MAYBE);
	return CORBA_OBJECT_NIL;
}

static void ORBit_marshal_profile(gpointer item, gpointer data)
{
	ORBit_Object_info *info = (ORBit_Object_info *)item;
	GIOPSendBuffer *send_buffer = (GIOPSendBuffer *)data;
	static const CORBA_unsigned_long ioptag = IOP_TAG_INTERNET_IOP,
		orbittag = IOP_TAG_ORBIT_SPECIFIC;
	CDR_Codec codec_d;
	CDR_Codec *codec = &codec_d;
	CORBA_unsigned_long len;
	CORBA_octet codecbuf[2048];
	static const CORBA_octet oc_endian = FLAG_ENDIANNESS;
	static const CORBA_octet iiopversion[] = {1,0};

	g_assert(info);
	g_assert(send_buffer);

	if (info->profile_type == IOP_TAG_INTERNET_IOP) {
		giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(send_buffer),
					       &ioptag, sizeof(ioptag));

		CDR_codec_init_static(codec);
		codec->buffer = codecbuf;
		codec->buf_len = 2048;
		codec->release_buffer = CORBA_FALSE;
		codec->readonly = CORBA_FALSE;
		codec->data_endian = codec->host_endian = FLAG_ENDIANNESS;
		CDR_put_octet(codec, oc_endian);
		CDR_put_octet(codec, iiopversion[0]);
		CDR_put_octet(codec, iiopversion[1]);
		CDR_put_string(codec, info->tag.iopinfo.host);
		CDR_put_ushort(codec, info->tag.iopinfo.port);
		CDR_put_ulong(codec, info->object_key._length);
		CDR_put_octets(codec, info->object_key._buffer,
			       info->object_key._length);
		len = codec->wptr;
		giop_send_buffer_append_mem_indirect_a(send_buffer,
						       &len, sizeof(len));
		giop_send_buffer_append_mem_indirect(send_buffer,
						     codec->buffer, codec->wptr);
	} else if (info->profile_type==IOP_TAG_ORBIT_SPECIFIC) {
		giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(send_buffer),
						 &orbittag, sizeof(orbittag));
		CDR_codec_init_static(codec);
		codec->buffer = codecbuf;
		codec->release_buffer = CORBA_FALSE;
		codec->buf_len = 2048;
		codec->readonly = CORBA_FALSE;
		codec->data_endian = codec->host_endian = FLAG_ENDIANNESS;
		CDR_put_octet(codec, oc_endian);
		CDR_put_octets(codec, (gpointer)iiopversion, sizeof(iiopversion));
		CDR_put_string(codec, info->tag.orbitinfo.unix_sock_path);
		CDR_put_ushort(codec, info->tag.orbitinfo.ipv6_port);
		CDR_put_ulong(codec, info->object_key._length);
		CDR_put_octets(codec, info->object_key._buffer,
			       info->object_key._length);
		len = codec->wptr;
		giop_send_buffer_append_mem_indirect_a(send_buffer,
						       &len, sizeof(len));
		giop_send_buffer_append_mem_indirect(send_buffer,
						     codec->buffer, codec->wptr);
	} else {
#ifdef PROFILE_DEBUG
		g_warning("ORBit_marshal_profile ask to marshal type %d\n", info->profile_type);
#endif
	}
}

void
ORBit_marshal_object(GIOPSendBuffer *send_buffer, CORBA_Object obj)
{
	CORBA_unsigned_long len;


	if(!obj) {
		static const CORBA_unsigned_long zero = 0, one = 1;
		/* zero-length typename */
		giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(send_buffer),
						 &one, sizeof(one));
		giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(send_buffer),
					       &zero, 1);

		/* zero profiles */
		giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(send_buffer),
						 &zero, sizeof(zero));
		return;
	}
	g_return_if_fail(ORBIT_ROOT_OBJECT(obj)->refs > 0);

	len = strlen(obj->object_id) + 1;
	giop_send_buffer_append_mem_indirect_a(send_buffer, &len,
					       sizeof(len));
	giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(send_buffer),
				       obj->object_id, len);

	len = g_slist_length(obj->profile_list);
	giop_send_buffer_append_mem_indirect_a(GIOP_SEND_BUFFER(send_buffer),
					       &len, sizeof(len));

	/* Marshal active first? */
	g_slist_foreach(obj->profile_list, ORBit_marshal_profile, send_buffer);
}

static void ORBit_test_profile(gpointer item, gpointer data)
{
	ORBit_Object_info *info = (ORBit_Object_info *)item;
	CORBA_Object obj = (CORBA_Object)data;

	if(obj->active_profile != NULL)
		return;		/* we already have a good profile */

	if(info->profile_type == IOP_TAG_ORBIT_SPECIFIC) {
		if(!ORBit_parse_unixsock(obj, info->tag.orbitinfo.unix_sock_path, TRUE)) {
			/* success */
			obj->active_profile=info;
		}
	} else if(info->profile_type == IOP_TAG_INTERNET_IOP) {
		if(!ORBit_parse_inet(obj, info->tag.iopinfo.host, info->tag.iopinfo.port, TRUE)) {
			/* success */
			obj->active_profile=info;
		}
	}
}

static void
ORBit_object_try_existing_connections(CORBA_Object obj)
{
	g_slist_foreach(obj->profile_list, ORBit_test_profile, obj);
}

static void ORBit_activate_profile(gpointer item, gpointer data)
{
	ORBit_Object_info *info = (ORBit_Object_info *)item;
	CORBA_Object obj = (CORBA_Object)data;

	if(obj->active_profile != NULL)
		return;		/* we already have a good profile */

	if(info->profile_type == IOP_TAG_ORBIT_SPECIFIC) {
		if(!ORBit_parse_unixsock(obj, info->tag.orbitinfo.unix_sock_path, FALSE)) {
			/* success */
			obj->active_profile=info;
		}
	} else if(info->profile_type == IOP_TAG_INTERNET_IOP) {
		if(!ORBit_parse_inet(obj, info->tag.iopinfo.host, info->tag.iopinfo.port, FALSE)) {
			/* success */
			obj->active_profile=info;
		}
	}
}

GIOPConnection *
_ORBit_object_get_connection(CORBA_Object obj)
{
	g_return_val_if_fail(obj, NULL);

	if (obj->connection) {
		giop_connection_unref(obj->connection);
		obj->connection = NULL;
		obj->active_profile = NULL;
	}

	g_slist_foreach(obj->profile_list, ORBit_activate_profile, obj);

	if(obj->active_profile == NULL || !obj->connection)
		return NULL;

	obj->connection->orb_data = obj->orb;

	return obj->connection;
}

GIOPConnection *
ORBit_object_get_forwarded_connection(CORBA_Object obj)
{
	g_return_val_if_fail(obj, NULL);

	if (obj->connection) {
		giop_connection_unref(obj->connection);
		obj->connection = NULL;
		obj->active_profile = NULL;
	}

	g_slist_foreach(obj->forward_locations, ORBit_activate_profile, obj);

	if(obj->active_profile == NULL || !obj->connection)
		return NULL;

	obj->connection->orb_data = obj->orb;

	return obj->connection;
}

/* This function is heavily based on the idl stubs output. Any changes there
   will probably have to be reflected here also. */

void ORBit_object_locate(CORBA_Object obj, CORBA_Environment *ev)
{
	GIOPConnection *cnx;
	GIOPSendBuffer *send_buffer;
	GIOPRecvBuffer *recv_buffer;
	GIOP_unsigned_long request_id;

	g_return_if_fail(obj!=NULL);
	g_return_if_fail(ev!=NULL);

	/* Send a LOCATE_REQUEST, wait for a LOCATE_REPLY. The reply will
	   either say "Object here", or will carry a new location. We set
	   obj->active_profile appropriately */

	cnx=ORBit_object_get_connection(obj);
	if((cnx==NULL) || (obj->active_profile==NULL)) {
		CORBA_exception_set_system(ev, ex_CORBA_COMM_FAILURE, CORBA_COMPLETED_NO);
		return;
	}
	request_id=giop_get_request_id();
	send_buffer=giop_send_locate_request_buffer_use(cnx, request_id, &(obj->active_profile->object_key_vec));
	if(!send_buffer) {
		CORBA_exception_set_system(ev, ex_CORBA_COMM_FAILURE, CORBA_COMPLETED_NO);
		return;
	}

	giop_send_buffer_write(send_buffer);
	giop_send_buffer_unuse(send_buffer);

	recv_buffer=giop_recv_locate_reply_buffer_use(request_id, TRUE);
	if(recv_buffer==NULL || recv_buffer->message_buffer.message_header.message_type!=GIOP_LOCATEREPLY) {
		CORBA_exception_set_system(ev, ex_CORBA_COMM_FAILURE, CORBA_COMPLETED_MAYBE);
		if(recv_buffer)
			giop_recv_buffer_unuse(recv_buffer);
		
		return;
	}

	ev->_major=CORBA_NO_EXCEPTION;
	switch(recv_buffer->message.u.locate_reply.locate_status) {
	case GIOP_UNKNOWN_OBJECT:
		CORBA_exception_set_system(ev, ex_CORBA_OBJECT_NOT_EXIST, CORBA_COMPLETED_NO);
		break;

	case GIOP_OBJECT_HERE:
		/* No further processing necessary */
		break;

	case GIOP_OBJECT_FORWARD:
		/* We've been forwarded onto somewhere else. The message body
		   contains the new IOR */
		if(obj->forward_locations != NULL) {
			ORBit_delete_profiles(obj->forward_locations);
		}
		obj->forward_locations=ORBit_demarshal_IOR(recv_buffer);

		/* This will adjust obj->active_profile */
		cnx=ORBit_object_get_forwarded_connection(obj);
		break;

	default:
		g_message("Bad Reply in ORBit_locate_object()\n");
		break;

	}

	giop_recv_buffer_unuse(recv_buffer);
}

GIOPConnection *
ORBit_handle_location_forward(GIOPRecvBuffer *rb, CORBA_Object obj)
{
	GIOPConnection *retval;

	if(obj->forward_locations)
		ORBit_delete_profiles(obj->forward_locations);
	obj->forward_locations = ORBit_demarshal_IOR(rb);

	retval = ORBit_object_get_forwarded_connection(obj);
	giop_recv_buffer_unuse(rb);

	return retval;
}
