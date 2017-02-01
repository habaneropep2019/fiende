/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-async.h: Code to make hand written
 * async CORBA wrappers easy to write.
 *
 * Author:
 *     Miguel de Icaza (miguel@gnu.org).
 *
 * Copyright 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_ASYNC_H_
#define _BONOBO_ASYNC_H_

#include <glib.h>
#include <orb/orbit.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

typedef enum {
	BONOBO_ASYNC_IN  = 0x1,
	BONOBO_ASYNC_OUT = 0x2
} BonoboAsyncArgFlag;

typedef struct {
	const CORBA_TypeCode  type;
	BonoboAsyncArgFlag    flag;
} BonoboAsyncArg;

typedef struct {
	const char           *name;
	const CORBA_TypeCode  ret_type;
	const BonoboAsyncArg *arguments;  /* NULL-terminated */
	const CORBA_TypeCode *exceptions; /* NULL-terminated */
} BonoboAsyncMethod;

typedef struct _BonoboAsyncReply BonoboAsyncReply;

typedef void (*BonoboAsyncCallback) (BonoboAsyncReply  *reply,
				     CORBA_Environment *ev,
				     gpointer           user_data);

void bonobo_async_demarshal (BonoboAsyncReply        *reply,
			     gpointer                 retval,
			     gpointer                *out_args);

void bonobo_async_invoke    (const BonoboAsyncMethod *method,
			     BonoboAsyncCallback      cb,
			     gpointer                 user_data,
			     guint                    timeout_msec,
			     CORBA_Object             object,
			     gpointer                *args,
			     CORBA_Environment       *ev);

GIOPRecvBuffer *bonobo_async_handle_get_recv (BonoboAsyncReply *reply);

END_GNOME_DECLS

#endif /* _BONOBO_ASYNC_H_ */

