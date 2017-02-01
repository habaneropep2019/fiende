/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-listener.h: Generic listener interface for callbacks.
 *
 * Authors:
 *	Alex Graveley (alex@helixcode.com)
 *	Mike Kestner  (mkestner@ameritech.net)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#ifndef _BONOBO_LISTENER_H_
#define _BONOBO_LISTENER_H_

#include <bonobo/bonobo-arg.h>
#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS

#define BONOBO_LISTENER_TYPE        (bonobo_listener_get_type ())
#define BONOBO_LISTENER(o)          (GTK_CHECK_CAST ((o), BONOBO_LISTENER_TYPE, BonoboListener))
#define BONOBO_LISTENER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_LISTENER_TYPE, BonoboListenerClass))
#define BONOBO_IS_LISTENER(o)       (GTK_CHECK_TYPE ((o), BONOBO_LISTENER_TYPE))
#define BONOBO_IS_LISTENER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_LISTENER_TYPE))

typedef struct _BonoboListenerPrivate BonoboListenerPrivate;

typedef struct {
        BonoboXObject          parent;

	BonoboListenerPrivate *priv;
} BonoboListener;

typedef struct {
	BonoboXObjectClass     parent_class;

	POA_Bonobo_Listener__epv epv;

	/* Signals */
	void (* event_notify) (BonoboListener    *listener, 
			       char              *event_name,
			       BonoboArg         *event_data, 
			       CORBA_Environment *ev);
} BonoboListenerClass;


typedef void (*BonoboListenerCallbackFn)  (BonoboListener    *listener,
					   char              *event_name, 
					   CORBA_any         *any,
					   CORBA_Environment *ev,
					   gpointer           user_data);

GtkType         bonobo_listener_get_type  (void);

BonoboListener *bonobo_listener_new       (BonoboListenerCallbackFn event_callback, 
					   gpointer                 user_data);

char           *bonobo_event_make_name    (const char *idl_path, 
					   const char *kind,
					   const char *subtype);

char           *bonobo_event_type         (const char *event_name);
char           *bonobo_event_subtype      (const char *event_name);
char           *bonobo_event_kind         (const char *event_name);
char           *bonobo_event_idl_path     (const char *event_name);

END_GNOME_DECLS

#endif /* _BONOBO_LISTENER_H_ */

