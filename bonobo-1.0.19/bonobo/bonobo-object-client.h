/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-object-client.c:
 *   This handles the client-view of a remote Bonobo object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#ifndef _BONOBO_OBJECT_CLIENT_H_
#define _BONOBO_OBJECT_CLIENT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-object.h>

#define BONOBO_OBJECT_CLIENT_TYPE        (bonobo_object_client_get_type ())
#define BONOBO_OBJECT_CLIENT(o)          (GTK_CHECK_CAST ((o), BONOBO_OBJECT_CLIENT_TYPE, BonoboObjectClient))
#define BONOBO_OBJECT_CLIENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_OBJECT_CLIENT_TYPE, BonoboObjectClientClass))
#define BONOBO_IS_OBJECT_CLIENT(o)       (GTK_CHECK_TYPE ((o), BONOBO_OBJECT_CLIENT_TYPE))
#define BONOBO_IS_OBJECT_CLIENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_OBJECT_CLIENT_TYPE))

typedef struct {
	BonoboObject parent;
} BonoboObjectClient;

typedef struct {
	BonoboObjectClass parent_class;
} BonoboObjectClientClass;

typedef void      (*BonoboObjectClientAsyncCallback)     (BonoboObjectClient             *o,
							  const char                     *error,
							  gpointer                        user_data);
GtkType             bonobo_object_client_get_type        (void);
BonoboObjectClient *bonobo_object_client_from_corba      (Bonobo_Unknown                  o);
BonoboObjectClient *bonobo_object_client_construct       (BonoboObjectClient             *object_client,
							  CORBA_Object                    corba_object);
BonoboObjectClient *bonobo_object_activate               (const char                     *iid,
							  gint                            oaf_flags);
void                bonobo_object_activate_async         (const char                     *iid,
							  gint                            oaf_flags,
							  BonoboObjectClientAsyncCallback callback,
							  gpointer                        user_data);
/* Convenience Bonobo_Unknown wrappers */
gboolean            bonobo_object_client_has_interface   (BonoboObjectClient             *object,
							  const char                     *interface_desc,
							  CORBA_Environment              *opt_ev);
Bonobo_Unknown      bonobo_object_client_query_interface (BonoboObjectClient             *object,
							  const char                     *interface_desc,
							  CORBA_Environment              *opt_ev);
void                bonobo_object_client_ref             (BonoboObjectClient             *object_client,
							  BonoboObject                   *opt_exception_obj);
void                bonobo_object_client_unref           (BonoboObjectClient             *object_client,
							  BonoboObject                   *opt_exception_obj);

#endif /* _BONOBO_OBJECT_CLIENT_H_ */

