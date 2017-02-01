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
 *          Elliot Lee <sopwith@redhat.com>
 *
 */

#include "config.h"
#include <IIOP/IIOP.h>
#include "orbit_types.h"
#include "corba_object.h"
#include "corba_object_type.h"
#include "env.h"
#include "orb.h"
#include "interface_repository.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Section 4.2.1 */
CORBA_InterfaceDef CORBA_Object_get_interface(CORBA_Object obj, CORBA_Environment *ev)
{
	CORBA_Repository repo;
	CORBA_InterfaceDef interface;

	if(obj==CORBA_OBJECT_NIL)
		return(CORBA_OBJECT_NIL); /* no exception defined in spec */

	repo=CORBA_ORB_resolve_initial_references(obj->orb, "InterfaceRepository", ev);
	if(repo==CORBA_OBJECT_NIL)
		return(CORBA_OBJECT_NIL);

	interface=CORBA_Repository_lookup_id(repo, obj->object_id, ev);
	CORBA_Object_release(repo, ev);

	return(interface);
}

/* Section 4.2.3 */
CORBA_boolean CORBA_Object_is_nil(CORBA_Object obj, CORBA_Environment *ev)
{
	if(obj==CORBA_OBJECT_NIL) {
		return(CORBA_TRUE);
	} else {
		return(CORBA_FALSE);
	}
}

/* Section 4.2.2 */
/* XXXX Big warning: lots of places inside ORBit expect this to
                     always return 'obj'. Do not change this, upon pain
		     of death... */
CORBA_Object CORBA_Object_duplicate(CORBA_Object obj, CORBA_Environment *ev)
{
	if(obj == CORBA_OBJECT_NIL)
		return CORBA_OBJECT_NIL;

	if(ORBIT_ROOT_OBJECT(obj)->refs >= 0)
		ORBIT_ROOT_OBJECT_REF(obj);

	return(obj);
}


/* Section 4.2.2 */
void CORBA_Object_release(CORBA_Object obj, CORBA_Environment *ev)
{
	if(obj != CORBA_OBJECT_NIL)
		ORBIT_ROOT_OBJECT_release(obj,ev);
}

extern GHashTable *ORBit_class_assignments;

void ORBit_impl_CORBA_Object_is_a(gpointer servant,
				  GIOPRecvBuffer * _ORBIT_recv_buffer,
				  CORBA_Environment *ev,
				  gpointer dummy)
{
	GIOPSendBuffer *_ORBIT_send_buffer;
	struct CORBA_Object_struct objdummy; /* XXX badhack to save backwards compat */
	CORBA_boolean retval;
	char *repo_id;
	CORBA_unsigned_long slen;
	guchar *curptr;
	ORBit_ObjectKey *objkey;
	gpointer *tmp_vepv;
	guint sz;
	CORBA_unsigned_long clsid;
	PortableServer_ServantBase *_ORBIT_servant;

	_ORBIT_servant = servant;

	/* XXX security implications */
	curptr = _ORBIT_recv_buffer->cur;
	curptr = ALIGN_ADDRESS(curptr, 4);
	if(giop_msg_conversion_needed(GIOP_MESSAGE_BUFFER(_ORBIT_recv_buffer)))
		iiop_byteswap((guchar *)&slen, curptr, sizeof(CORBA_unsigned_long));
	else
		slen = *((CORBA_unsigned_long *)curptr);
	curptr += 4;
	repo_id = curptr;

	repo_id[slen] = '\0';

	objkey = ORBIT_OBJECT_KEY(_ORBIT_servant->_private);

	sz = sizeof(gpointer) * (ORBit_class_assignment_counter + 1);
	tmp_vepv = alloca(sz);
	memset(tmp_vepv, '\0', sz);

	objdummy.vepv = tmp_vepv;
	objkey->class_info->init_local_objref(&objdummy, servant);

	clsid = GPOINTER_TO_UINT(g_hash_table_lookup(ORBit_class_assignments, repo_id));
	retval = (clsid && tmp_vepv[clsid]);

	_ORBIT_send_buffer = giop_send_reply_buffer_use(GIOP_MESSAGE_BUFFER(_ORBIT_recv_buffer)->connection, NULL,
							_ORBIT_recv_buffer->message.u.request.request_id, ev->_major);
	giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(_ORBIT_send_buffer), &retval, sizeof(retval));
	giop_send_buffer_write(_ORBIT_send_buffer);
	giop_send_buffer_unuse(_ORBIT_send_buffer);
}

/* Section 4.2.4 */
CORBA_boolean CORBA_Object_is_a(CORBA_Object obj, CORBA_char *logical_type_id, CORBA_Environment *ev)
{
	if(obj == CORBA_OBJECT_NIL)
		return CORBA_FALSE;

	if (obj->servant && obj->vepv) {
		CORBA_unsigned_long clsid;

		clsid = GPOINTER_TO_UINT(g_hash_table_lookup(ORBit_class_assignments, logical_type_id));

		return (clsid && (clsid < obj->vepv_size) && obj->vepv[clsid]);
	} else if(!strcmp(obj->object_id, logical_type_id)
		  || !strcmp("IDL:CORBA/Object:1.0", logical_type_id)) {
		return CORBA_TRUE;
	} else {
		/* Cut and paste from orbit-idl output */
		/* XXX security implications */
		register GIOP_unsigned_long _ORBIT_request_id,
			_ORBIT_system_exception_minor;
		register CORBA_completion_status _ORBIT_completion_status;
		register GIOPSendBuffer *_ORBIT_send_buffer;
		register GIOPRecvBuffer *_ORBIT_recv_buffer;
		register GIOPConnection *_cnx;

		_cnx = ORBit_object_get_connection(obj);

	_ORBIT_retry_request:
		_ORBIT_send_buffer = NULL;
		_ORBIT_recv_buffer = NULL;
		_ORBIT_completion_status = CORBA_COMPLETED_NO;
		_ORBIT_request_id = GPOINTER_TO_UINT(alloca(0));
		{				/* marshalling */
			static const struct {
				CORBA_unsigned_long len;
				char opname[6];
			} _ORBIT_operation_name_data = {
				6, "_is_a"
			};
			static const struct iovec _ORBIT_operation_vec =
			{(gpointer) & _ORBIT_operation_name_data, 10};
			register CORBA_unsigned_long _ORBIT_tmpvar_0;
			CORBA_unsigned_long _ORBIT_tmpvar_1;

			_ORBIT_send_buffer =
				giop_send_request_buffer_use(_cnx, NULL, _ORBIT_request_id, CORBA_TRUE,
							     &(obj->active_profile->object_key_vec), &_ORBIT_operation_vec, &ORBit_default_principal_iovec);

			_ORBIT_system_exception_minor = ex_CORBA_COMM_FAILURE;
			if (!_ORBIT_send_buffer)
				goto _ORBIT_system_exception;
			_ORBIT_tmpvar_1 = strlen(logical_type_id) + 1;
			giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(_ORBIT_send_buffer), 4);
			giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(_ORBIT_send_buffer), &(_ORBIT_tmpvar_1), sizeof(_ORBIT_tmpvar_1));
			giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(_ORBIT_send_buffer), (logical_type_id), sizeof(logical_type_id[_ORBIT_tmpvar_0]) * _ORBIT_tmpvar_1);
			giop_send_buffer_write(_ORBIT_send_buffer);
			_ORBIT_completion_status = CORBA_COMPLETED_MAYBE;
			giop_send_buffer_unuse(_ORBIT_send_buffer);
			_ORBIT_send_buffer = NULL;
		}
		{				/* demarshalling */
			register guchar *_ORBIT_curptr;
			CORBA_boolean _ORBIT_retval;

			_ORBIT_recv_buffer = giop_recv_reply_buffer_use_2(_cnx, _ORBIT_request_id, TRUE);
			if (!_ORBIT_recv_buffer)
				goto _ORBIT_system_exception;
			_ORBIT_completion_status = CORBA_COMPLETED_YES;
			if (_ORBIT_recv_buffer->message.u.reply.reply_status != GIOP_NO_EXCEPTION)
				goto _ORBIT_msg_exception;
			if (giop_msg_conversion_needed(GIOP_MESSAGE_BUFFER(_ORBIT_recv_buffer))) {
				_ORBIT_curptr = GIOP_RECV_BUFFER(_ORBIT_recv_buffer)->cur;
				_ORBIT_retval = *((CORBA_boolean *) _ORBIT_curptr);
			} else {
				_ORBIT_curptr = GIOP_RECV_BUFFER(_ORBIT_recv_buffer)->cur;
				_ORBIT_retval = *((CORBA_boolean *) _ORBIT_curptr);
			}
			giop_recv_buffer_unuse(_ORBIT_recv_buffer);
			return _ORBIT_retval;
		_ORBIT_system_exception:
			CORBA_exception_set_system(ev, _ORBIT_system_exception_minor, _ORBIT_completion_status);
			giop_recv_buffer_unuse(_ORBIT_recv_buffer);
			giop_send_buffer_unuse(_ORBIT_send_buffer);
			return _ORBIT_retval;
		_ORBIT_msg_exception:
			if (_ORBIT_recv_buffer->message.u.reply.reply_status == GIOP_LOCATION_FORWARD) {
				if (obj->forward_locations != NULL)
					ORBit_delete_profiles(obj->forward_locations);
				obj->forward_locations = ORBit_demarshal_IOR(_ORBIT_recv_buffer);
				_cnx = ORBit_object_get_forwarded_connection(obj);
				giop_recv_buffer_unuse(_ORBIT_recv_buffer);

				goto _ORBIT_retry_request;
			} else {
				ORBit_handle_exception(_ORBIT_recv_buffer, ev, NULL, obj->orb);
				giop_recv_buffer_unuse(_ORBIT_recv_buffer);
				return _ORBIT_retval;
			}
		}

	}
}

/* Section 4.2.5 */
static void do_exit(int signum) {
  _exit(5);
}

/* Lovely hack to try and figure out without hanging whether an object exists or not. */
CORBA_boolean CORBA_Object_non_existent(CORBA_Object obj, CORBA_Environment *ev)
{
  int childpid, exitstatus, itmp;
  sigset_t mask, omask;

  ev->_major = CORBA_NO_EXCEPTION;

  if(obj == CORBA_OBJECT_NIL)
	  return TRUE;

  if(obj->servant)
	  return FALSE;

  if(obj->connection && obj->connection->is_valid)
	  return FALSE;

  /* Block SIGCHLD so no one else can wait() on the child before we do. */
  sigemptyset(&mask);
  sigaddset(&mask, SIGCHLD);
  sigprocmask(SIG_BLOCK, &mask, &omask);

  childpid = fork();

  if(!childpid) {
	  GIOPConnection* cnx = NULL;
	  struct sigaction sa;

	  memset(&sa, 0, sizeof(sa));
	  sa.sa_handler = do_exit;
	  sigaction(SIGALRM, &sa, NULL);
	  alarm(2);
	  cnx = _ORBit_object_get_connection(obj);

	  /* XXX todo - try invoking a strange operation on the object, and see what type of exception we get. */ 

	  _exit((cnx == NULL)?1:0);
  }

  while ((itmp = waitpid(childpid, &exitstatus, 0)) == -1 && errno == EINTR)
	  ;
  sigprocmask (SIG_SETMASK, &omask, NULL);

  if(itmp < 0) return TRUE;
  return WEXITSTATUS(exitstatus) && TRUE;
}

gboolean
g_CORBA_Object_equal(CORBA_Object obj1, CORBA_Object obj2)
{
	gboolean retval;
	CORBA_Environment ev;

	CORBA_exception_init(&ev);

	retval = (gboolean)CORBA_Object_is_equivalent(obj1, obj2, &ev);

	CORBA_exception_free(&ev);

	return retval;
}

/* Section 4.2.6 */
CORBA_boolean CORBA_Object_is_equivalent(CORBA_Object obj, CORBA_Object other_object, CORBA_Environment *ev)
{
	ORBit_Object_info *obj_profile, *other_object_profile;
	int i,j, obj_profile_count, other_object_profile_count;

	if(obj == CORBA_OBJECT_NIL
	   && other_object == CORBA_OBJECT_NIL)
		return CORBA_TRUE;

	if(obj == CORBA_OBJECT_NIL
	   || other_object == CORBA_OBJECT_NIL)
		goto ret_false;

	/*
	 * If one profile in "obj" matches one in "other_object", then these
	 * objects are equivalent.
	 *
	 * This is O(n*m) at worst case :-( Hopefully though most objects will
	 * only have 1 or 2 profiles.
	 *
	 * The profile list could be indexed as a hash table (the linked list
	 * is still needed, as the profile order is significant)
	 */

	obj_profile_count = g_slist_length(obj->profile_list);
	other_object_profile_count = g_slist_length(other_object->profile_list);

	for(i=0;i<obj_profile_count;i++) {
		obj_profile=(ORBit_Object_info *)g_slist_nth_data(obj->profile_list, i);

		for(j=0;j<other_object_profile_count;j++) {
			other_object_profile=(ORBit_Object_info *)g_slist_nth_data(other_object->profile_list, j);

			if(obj_profile->profile_type != other_object_profile->profile_type)
				continue;
			
			if(obj_profile->object_key._length != other_object_profile->object_key._length)
				continue;

			if(memcmp(obj_profile->object_key._buffer, other_object_profile->object_key._buffer, obj_profile->object_key._length))
				continue;

			if(obj_profile->profile_type == IOP_TAG_INTERNET_IOP) {
				TAG_INTERNET_IOP_info *ii1, *ii2;

				ii1 = &obj_profile->tag.iopinfo;
				ii2 = &other_object_profile->tag.iopinfo;

				if(ii1->port != ii2->port)
					continue;
				if(strcmp(ii1->host, ii2->host))
					continue;

				return(CORBA_TRUE);
			} else if(obj_profile->profile_type == IOP_TAG_ORBIT_SPECIFIC) {
				TAG_ORBIT_SPECIFIC_info *oi1, *oi2;

				oi1 = &obj_profile->tag.orbitinfo;
				oi2 = &other_object_profile->tag.orbitinfo;

				if(strcmp(oi1->unix_sock_path, oi2->unix_sock_path))
					continue;
				if(oi1->ipv6_port != oi2->ipv6_port)
					continue;

				return(CORBA_TRUE);
			}
		}
	}

 ret_false:
	return CORBA_FALSE;
}

guint
g_CORBA_Object_hash(CORBA_Object obj)
{
	guint retval;
	CORBA_Environment ev;

	CORBA_exception_init(&ev);

	retval = (guint)CORBA_Object_hash(obj, UINT_MAX, &ev);

	CORBA_exception_free(&ev);

	return retval;
}

static void profile_hash(gpointer item, gpointer data)
{
	ORBit_Object_info *info = (ORBit_Object_info *)item;
	CORBA_unsigned_long *retval = (CORBA_unsigned_long *)data;

	g_assert(info);
	g_assert(retval);

	*retval ^= info->object_key._length;

	if(info->profile_type == IOP_TAG_INTERNET_IOP) {
		*retval ^= !info->tag.iopinfo.port;
	} else if(info->profile_type == IOP_TAG_ORBIT_SPECIFIC) {
		*retval ^= g_str_hash(info->tag.orbitinfo.unix_sock_path);
		*retval ^= !info->tag.orbitinfo.ipv6_port;
	}
}

/* Section 4.2.6 */
CORBA_unsigned_long CORBA_Object_hash(CORBA_Object obj,
				      CORBA_unsigned_long maximum,
				      CORBA_Environment *ev)
{
	CORBA_unsigned_long retval = 0;
	char *tptr;

	g_assert(obj);

	tptr = obj->object_id;
	while(*tptr) {
		retval = (retval << 8) ^ *tptr;
		tptr++;
	}

	if(g_slist_length(obj->profile_list)>0) {
		g_slist_foreach(obj->profile_list, profile_hash, &retval);
	} else {
		g_warning("Object of type %s doesn't seem to have any connection info!", obj->object_id);
	}

	return (retval % maximum);
}

/* Section 4.2.7 */
CORBA_Policy CORBA_Object_get_policy(CORBA_Object obj, CORBA_PolicyType policy_type, CORBA_Environment *ev)
{
	g_assert(!"Not yet implemented");
	return(NULL);
}
