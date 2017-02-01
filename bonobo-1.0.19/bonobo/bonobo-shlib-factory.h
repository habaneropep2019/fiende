/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-shlib-factory.h: a ShlibFactory object.
 *
 * Author:
 *    Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_SHLIB_FACTORY_H_
#define _BONOBO_SHLIB_FACTORY_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-object.h>
#include <liboaf/oaf.h>
#include <bonobo/bonobo-generic-factory.h>
#include <bonobo/bonobo-exception.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_SHLIB_FACTORY_TYPE        (bonobo_shlib_factory_get_type ())
#define BONOBO_SHLIB_FACTORY(o)          (GTK_CHECK_CAST ((o), BONOBO_SHLIB_FACTORY_TYPE, BonoboShlibFactory))
#define BONOBO_SHLIB_FACTORY_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_SHLIB_FACTORY_TYPE, BonoboShlibFactoryClass))
#define BONOBO_IS_SHLIB_FACTORY(o)       (GTK_CHECK_TYPE ((o), BONOBO_SHLIB_FACTORY_TYPE))
#define BONOBO_IS_SHLIB_FACTORY_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_SHLIB_FACTORY_TYPE))

#define BONOBO_OAF_SHLIB_FACTORY_MULTI(oafiid, descr, fn, data)               \
static CORBA_Object                                                           \
make_factory (PortableServer_POA poa, const char *iid, gpointer impl_ptr,     \
	      CORBA_Environment *ev)                                          \
{                                                                             \
	BonoboShlibFactory *f;                                                \
        CORBA_Object object_ref;                                              \
	f = bonobo_shlib_factory_new_multi (oafiid, poa, impl_ptr, fn, data); \
        object_ref = bonobo_object_corba_objref (BONOBO_OBJECT (f));          \
        if (BONOBO_EX (ev) || !object_ref) {                                  \
		g_warning ("cannot get objref: '%s'",                         \
			   bonobo_exception_get_text (ev));                   \
                return CORBA_OBJECT_NIL;                                      \
        }                                                                     \
        return CORBA_Object_duplicate (object_ref, ev);                       \
}                                                                             \
static OAFPluginObject plugin_list[] = {{oafiid, make_factory}, { NULL } };   \
const OAFPlugin OAF_Plugin_info = { plugin_list, descr };

#define BONOBO_OAF_SHLIB_FACTORY(oafiid, descr, fn, data)                     \
static CORBA_Object                                                           \
make_factory (PortableServer_POA poa, const char *iid, gpointer impl_ptr,     \
	      CORBA_Environment *ev)                                          \
{                                                                             \
	BonoboShlibFactory *f;                                                \
        CORBA_Object object_ref;                                              \
	f = bonobo_shlib_factory_new (oafiid, poa, impl_ptr, fn, data);       \
        object_ref = bonobo_object_corba_objref (BONOBO_OBJECT (f));          \
        if (BONOBO_EX (ev) || !object_ref) {                                  \
		g_warning ("cannot get objref: '%s'",                         \
			   bonobo_exception_get_text (ev));                   \
                return CORBA_OBJECT_NIL;                                      \
        }                                                                     \
        return CORBA_Object_duplicate (object_ref, ev);                       \
}                                                                             \
static OAFPluginObject plugin_list[] = {{oafiid, make_factory}, { NULL } };   \
const OAFPlugin OAF_Plugin_info = { plugin_list, descr };

typedef struct _BonoboShlibFactoryPrivate BonoboShlibFactoryPrivate;
					
typedef struct {
	BonoboGenericFactory base;

	int                  live_objects;
	gpointer             oaf_impl_ptr;

	BonoboShlibFactoryPrivate *priv;
} BonoboShlibFactory;

typedef struct {
	BonoboGenericFactoryClass parent_class;
} BonoboShlibFactoryClass;

GtkType             bonobo_shlib_factory_get_type  (void);

BonoboShlibFactory *bonobo_shlib_factory_construct (
	BonoboShlibFactory    *c_factory,
	const char            *component_id,
	PortableServer_POA     poa,
	gpointer               oaf_impl_ptr,
	CORBA_Object           corba_factory,
	BonoboGenericFactoryFn factory,
	GnomeFactoryCallback   factory_cb,
	void                  *data);

BonoboShlibFactory *bonobo_shlib_factory_new (
	const char            *component_id,
	PortableServer_POA     poa,
	gpointer               oaf_impl_ptr,
	BonoboGenericFactoryFn factory,
	gpointer               user_data);

BonoboShlibFactory *bonobo_shlib_factory_new_multi (
	const char            *component_id,
	PortableServer_POA     poa,
	gpointer               oaf_impl_ptr,
	GnomeFactoryCallback   factory_cb,
	gpointer               user_data);

void                bonobo_shlib_factory_track_object (
	BonoboShlibFactory    *factory,
	BonoboObject          *object);

void                bonobo_shlib_factory_inc_live     (
	BonoboShlibFactory    *factory);

void                bonobo_shlib_factory_dec_live     (
	BonoboShlibFactory    *factory);

END_GNOME_DECLS

#endif
