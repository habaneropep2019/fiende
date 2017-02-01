/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-running-context.c: An interface to track running objects
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#ifndef _BONOBO_RUNNING_CONTEXT_H_
#define _BONOBO_RUNNING_CONTEXT_H_

#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS

typedef struct _BonoboRunningContextPrivate BonoboRunningContextPrivate;

typedef struct {
	BonoboXObject parent;

	BonoboRunningContextPrivate *priv;
} BonoboRunningContext;

typedef struct {
	BonoboXObjectClass parent;

	POA_Bonobo_RunningContext__epv epv;

	void (*last_unref) (void);
} BonoboRunningContextClass;

GtkType       bonobo_running_context_get_type        (void);
BonoboObject *bonobo_running_context_new             (void);

/*
 *   This interface is private, and purely for speed
 * of impl. of the context.
 */
void        bonobo_running_context_add_object      (CORBA_Object object);
void        bonobo_running_context_remove_object   (CORBA_Object object);
void        bonobo_running_context_ignore_object   (CORBA_Object object);
void        bonobo_running_context_trace_objects   (CORBA_Object object,
						    const char  *fn,
						    int          line,
						    int          mode);
void        bonobo_running_context_at_exit_unref   (CORBA_Object object);

#ifdef BONOBO_OBJECT_DEBUG
#	define           bonobo_running_context_add_object(o)   G_STMT_START{bonobo_running_context_trace_objects((o),G_GNUC_PRETTY_FUNCTION,__LINE__,0);}G_STMT_END
#	define           bonobo_running_context_remove_object(o)   G_STMT_START{bonobo_running_context_trace_objects((o),G_GNUC_PRETTY_FUNCTION,__LINE__,1);}G_STMT_END
#	define           bonobo_running_context_ignore_object(o)   G_STMT_START{bonobo_running_context_trace_objects((o),G_GNUC_PRETTY_FUNCTION,__LINE__,2);}G_STMT_END
#endif

END_GNOME_DECLS

#endif /* _BONOBO_RUNNING_CONTEXT_H_ */

