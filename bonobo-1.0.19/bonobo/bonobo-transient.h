/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-transient.h: a transient object implementation.
 *
 * This simplifies the creation of POA managers for transient objects.
 * Objects living in this POA are created on demand and destroyed after use.
 *
 * Authors:
 *   Nat Friedman    (nat@helixcode.com)
 *   Miguel de Icaza (miguel@helixcode.com)
 *
 * I just refactored the code from the original PropertyBag, all the smart hacks
 * are from Nat -mig.
 *
 * (C) 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_TRANSIENT_H_
#define _BONOBO_TRANSIENT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/Bonobo.h>

BEGIN_GNOME_DECLS

#define BONOBO_TRANSIENT_TYPE        (bonobo_transient_get_type ())
#define BONOBO_TRANSIENT(o)          (GTK_CHECK_CAST ((o), BONOBO_TRANSIENT_TYPE, BonoboTransient))
#define BONOBO_TRANSIENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_TRANSIENT_TYPE, BonoboTransientClass))
#define BONOBO_IS_TRANSIENT(o)       (GTK_CHECK_TYPE ((o), BONOBO_TRANSIENT_TYPE))
#define BONOBO_IS_TRANSIENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_TRANSIENT_TYPE))

typedef struct _BonoboTransientPriv BonoboTransientPriv;

typedef struct {
	GtkObject parent;

	BonoboTransientPriv *priv;
} BonoboTransient;

typedef struct {
	GtkObjectClass parent_class;
} BonoboTransientClass;

/*
 * Signature for incarnating servants in the BonoboTransient 
 */
typedef PortableServer_Servant (*BonoboTransientServantNew) (PortableServer_POA, BonoboTransient *, char *name, void *data);

/*
 * Signature for destroying servants created by BonoboTransientServantNew functions
 */
typedef void (*BonoboTransientServantDestroy) (PortableServer_Servant servant, void *data);

BonoboTransient *bonobo_transient_new           (PortableServer_POA poa,
				BonoboTransientServantNew     new_servant,
				BonoboTransientServantDestroy destroy_servant,
				void *data);
BonoboTransient *bonobo_transient_construct     (BonoboTransient *transient,
				PortableServer_POA        poa,
				BonoboTransientServantNew new_servant,
				BonoboTransientServantDestroy destroy_servant,
				gpointer                  data);

CORBA_Object bonobo_transient_create_objref (BonoboTransient   *transient,
				const char        *iface_name,
				const char        *name,
				CORBA_Environment *ev);

GtkType bonobo_transient_get_type (void);

END_GNOME_DECLS

#endif /* _BONOBO_TRANSIENT_H_ */



