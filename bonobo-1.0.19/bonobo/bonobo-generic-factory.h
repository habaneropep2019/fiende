/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-generic-factory.h: a GenericFactory object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#ifndef _BONOBO_GENERIC_FACTORY_H_
#define _BONOBO_GENERIC_FACTORY_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-object.h>
#include <gnome.h>
#include <liboaf/oaf.h>
#include <liboaf/liboaf.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_GENERIC_FACTORY_TYPE        (bonobo_generic_factory_get_type ())
#define BONOBO_GENERIC_FACTORY(o)          (GTK_CHECK_CAST ((o), BONOBO_GENERIC_FACTORY_TYPE, BonoboGenericFactory))
#define BONOBO_GENERIC_FACTORY_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_GENERIC_FACTORY_TYPE, BonoboGenericFactoryClass))
#define BONOBO_IS_GENERIC_FACTORY(o)       (GTK_CHECK_TYPE ((o), BONOBO_GENERIC_FACTORY_TYPE))
#define BONOBO_IS_GENERIC_FACTORY_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_GENERIC_FACTORY_TYPE))

typedef struct _BonoboGenericFactoryPrivate BonoboGenericFactoryPrivate;
typedef struct _BonoboGenericFactory        BonoboGenericFactory;

typedef BonoboObject * (*BonoboGenericFactoryFn)(BonoboGenericFactory *Factory, void *closure);
typedef BonoboObject * (*GnomeFactoryCallback)(BonoboGenericFactory *factory, const char *component_id, gpointer closure);
					
struct _BonoboGenericFactory {
	BonoboObject base;

	/* The function factory */
	BonoboGenericFactoryFn factory; /* compat reasons only */
	GnomeFactoryCallback   factory_cb;
	gpointer               factory_closure;

	/* The component_id for this generic factory */
	char *oaf_iid;
};

typedef struct {
	BonoboObjectClass parent_class;

	/* Virtual methods */
	BonoboObject *(*new_generic) (BonoboGenericFactory *c_factory,
				      const char           *component_id);
} BonoboGenericFactoryClass;

GtkType               bonobo_generic_factory_get_type  (void);

CORBA_Object          bonobo_generic_factory_corba_object_create (
	BonoboObject *object,
	gpointer      shlib_id);

BonoboGenericFactory *bonobo_generic_factory_new (
	const char            *oaf_iid,
	BonoboGenericFactoryFn factory,
	gpointer               user_data);

BonoboGenericFactory *bonobo_generic_factory_new_multi (
	const char            *oaf_iid,
	GnomeFactoryCallback   factory_cb,
	gpointer               data);

BonoboGenericFactory *bonobo_generic_factory_construct (
	const char            *oaf_iid,
	BonoboGenericFactory  *c_factory,
	CORBA_Object           corba_factory,
	BonoboGenericFactoryFn factory,
	GnomeFactoryCallback   factory_cb,
	gpointer               user_data);

BonoboGenericFactory *bonobo_generic_factory_construct_noregister (
	const char             *oaf_iid,
	BonoboGenericFactory   *c_factory,
	CORBA_Object            corba_factory,
	BonoboGenericFactoryFn  factory,
	GnomeFactoryCallback    factory_cb,
	gpointer                user_data);

void bonobo_generic_factory_set (
	BonoboGenericFactory  *c_factory,
	BonoboGenericFactoryFn factory,
	void                  *data);

POA_GNOME_ObjectFactory__epv *bonobo_generic_factory_get_epv (void);

#define BONOBO_OAF_FACTORY(oafiid, descr, version, fn, data)                  \
int main (int argc, char *argv [])                                            \
{                                                                             \
	BonoboGenericFactory *factory;                                        \
	CORBA_ORB orb;                                                        \
                                                                              \
	gnome_init_with_popt_table (descr, version, argc, argv,               \
				    oaf_popt_options, 0, NULL);               \
        orb = oaf_init (argc, argv);                                          \
	if (!bonobo_init (orb, CORBA_OBJECT_NIL, CORBA_OBJECT_NIL))           \
		g_error (_("Could not initialize Bonobo"));                   \
	factory = bonobo_generic_factory_new (oafiid, fn, data);              \
	bonobo_running_context_auto_exit_unref (BONOBO_OBJECT (factory));     \
	bonobo_main ();                                                       \
	return 0;                                                             \
}                                                                             

#define BONOBO_OAF_FACTORY_MULTI(oafiid, descr, version, fn, data)            \
int main (int argc, char *argv [])                                            \
{                                                                             \
	BonoboGenericFactory *factory;                                        \
	CORBA_ORB orb;                                                        \
                                                                              \
	gnome_init_with_popt_table (descr, version, argc, argv,               \
				    oaf_popt_options, 0, NULL);               \
        orb = oaf_init (argc, argv);                                          \
	if (!bonobo_init (orb, CORBA_OBJECT_NIL, CORBA_OBJECT_NIL))           \
		g_error (_("Could not initialize Bonobo"));                   \
	factory = bonobo_generic_factory_new_multi (oafiid, fn, data);        \
	bonobo_running_context_auto_exit_unref (BONOBO_OBJECT (factory));     \
	bonobo_main ();                                                       \
	return 0;                                                             \
}                                                                             

END_GNOME_DECLS

#endif
