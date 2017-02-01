/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-shlib-factory.c: a ShlibFactory object.
 *
 * The BonoboShlibFactory object is used to instantiate new
 * Bonobo::ShlibFactory objects.  It acts as a wrapper for the
 * Bonobo::ShlibFactory CORBA interface, and dispatches to
 * a user-specified factory function whenever its create_object()
 * method is invoked.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-shlib-factory.h>
#include <bonobo/bonobo-running-context.h>
#include <liboaf/liboaf.h>

POA_GNOME_ObjectFactory__vepv bonobo_shlib_factory_vepv;

static BonoboObjectClass *bonobo_shlib_factory_parent_class = NULL;

/**
 * bonobo_shlib_factory_construct:
 * @c_factory: The object to be initialized.
 * @oaf_iid: The GOAD id that the new factory will implement.
 * @oaf_impl_ptr: Oaf shlib handle
 * @corba_factory: The CORBA object which supports the
 * Bonobo::ShlibFactory interface and which will be used to
 * construct this BonoboShlibFactory Gtk object.
 * @factory: A callback which is used to create new GnomeShlib object instances.
 * @data: The closure data to be passed to the @factory callback routine.
 *
 * Initializes @c_factory with the command-line arguments and registers
 * the new factory in the name server.
 *
 * Returns: The initialized BonoboShlibFactory object.
 */
BonoboShlibFactory *
bonobo_shlib_factory_construct (BonoboShlibFactory    *c_factory,
				const char            *oaf_iid,
				PortableServer_POA     poa,
				gpointer               oaf_impl_ptr,
				CORBA_Object           corba_factory,
				BonoboGenericFactoryFn factory,
				GnomeFactoryCallback   factory_cb,
				gpointer               user_data)
{
	BonoboGenericFactory *r_factory;

	g_return_val_if_fail (c_factory != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_SHLIB_FACTORY (c_factory), NULL);
	g_return_val_if_fail (corba_factory != CORBA_OBJECT_NIL, NULL);

	c_factory->live_objects = 0;
	c_factory->oaf_impl_ptr = oaf_impl_ptr;

        oaf_plugin_use (poa, oaf_impl_ptr);

	r_factory = bonobo_generic_factory_construct_noregister (
		oaf_iid, BONOBO_GENERIC_FACTORY (c_factory),
		corba_factory, factory, factory_cb, user_data);

	return (BonoboShlibFactory *) r_factory;
}

/**
 * bonobo_shlib_factory_new:
 * @oaf_iid: The GOAD id that this factory implements
 * @poa: the poa.
 * @oaf_impl_ptr: Oaf shlib handle
 * @factory: A callback which is used to create new BonoboObject instances.
 * @user_data: The closure data to be passed to the @factory callback routine.
 *
 * This is a helper routine that simplifies the creation of factory
 * objects for GNOME objects.  The @factory function will be
 * invoked by the CORBA server when a request arrives to create a new
 * instance of an object supporting the Bonobo::Shlib interface.
 * The factory callback routine is passed the @data pointer to provide
 * the creation function with some state information.
 *
 * Returns: A BonoboShlibFactory object that has an activated
 * Bonobo::ShlibFactory object that has registered with the GNOME
 * name server.
 */
BonoboShlibFactory *
bonobo_shlib_factory_new (const char            *oaf_iid,
			  PortableServer_POA     poa,
			  gpointer               oaf_impl_ptr,
			  BonoboGenericFactoryFn factory,
			  gpointer               user_data)
{
	BonoboShlibFactory *c_factory;
	GNOME_ObjectFactory corba_factory;

	g_return_val_if_fail (factory != NULL, NULL);
	
	c_factory = gtk_type_new (bonobo_shlib_factory_get_type ());

	corba_factory = bonobo_generic_factory_corba_object_create (
		BONOBO_OBJECT (c_factory), factory);

	if (corba_factory == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (c_factory));
		return NULL;
	}
	
	return bonobo_shlib_factory_construct (
		c_factory, oaf_iid, poa, oaf_impl_ptr,
		corba_factory, factory, NULL, user_data);
}

/**
 * bonobo_shlib_factory_new_multi:
 * @oaf_iid: The GOAD id that this factory implements
 * @poa: the poa.
 * @oaf_impl_ptr: Oaf shlib handle
 * @factory_cb: A callback which is used to create new BonoboObject instances.
 * @user_data: The closure data to be passed to the @factory callback routine.
 *
 * This is a helper routine that simplifies the creation of factory
 * objects for GNOME objects.  The @factory function will be
 * invoked by the CORBA server when a request arrives to create a new
 * instance of an object supporting the Bonobo::Shlib interface.
 * The factory callback routine is passed the @data pointer to provide
 * the creation function with some state information.
 *
 * Returns: A BonoboShlibFactory object that has an activated
 * Bonobo::ShlibFactory object that has registered with the GNOME
 * name server.
 */
BonoboShlibFactory *bonobo_shlib_factory_new_multi (
	const char          *oaf_iid,
	PortableServer_POA   poa,
	gpointer             oaf_impl_ptr,
	GnomeFactoryCallback factory_cb,
	gpointer             user_data)
{
	BonoboShlibFactory *c_factory;
	GNOME_ObjectFactory corba_factory;

	g_return_val_if_fail (factory_cb != NULL, NULL);
	g_return_val_if_fail (oaf_iid != NULL, NULL);
	
	c_factory = gtk_type_new (bonobo_shlib_factory_get_type ());

	corba_factory = bonobo_generic_factory_corba_object_create (
		BONOBO_OBJECT (c_factory), factory_cb);

	if (corba_factory == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (c_factory));
		return NULL;
	}
	
	return bonobo_shlib_factory_construct (
		c_factory, oaf_iid, poa, oaf_impl_ptr,
		corba_factory, NULL, factory_cb, user_data);
}

static void
bonobo_shlib_factory_finalize (GtkObject *object)
{
/*	BonoboShlibFactory *c_factory = BONOBO_SHLIB_FACTORY (object);*/

	/*
	 * We pray this happens only when we have released our
	 * last ref and no one is holding pointers into the soon
	 * to be unloaded shlib, particularly the stack.
	 *
	 * This is achieved by an idle unref handler.
	 */

	/* we dont unload it because of a problem with the gtk type system */
	/* oaf_plugin_unuse (c_factory->oaf_impl_ptr); */

	GTK_OBJECT_CLASS (bonobo_shlib_factory_parent_class)->finalize (object);
}

static BonoboObject *
bonobo_shlib_factory_new_generic (BonoboGenericFactory *factory,
				  const char           *oaf_iid)
{
	BonoboObject *retval;

	retval = BONOBO_GENERIC_FACTORY_CLASS (
		bonobo_shlib_factory_parent_class)->new_generic (factory, oaf_iid);

	return retval;
}

static void
bonobo_shlib_factory_class_init (BonoboGenericFactoryClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	bonobo_shlib_factory_parent_class = gtk_type_class (
		bonobo_generic_factory_get_type ());

	klass->new_generic = bonobo_shlib_factory_new_generic;

	object_class->finalize = bonobo_shlib_factory_finalize;
}

/**
 * bonobo_shlib_factory_get_type:
 *
 * Returns: The GtkType of the BonoboShlibFactory class.
 */
GtkType
bonobo_shlib_factory_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboShlibFactory",
			sizeof (BonoboShlibFactory),
			sizeof (BonoboShlibFactoryClass),
			(GtkClassInitFunc)  bonobo_shlib_factory_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (
			bonobo_generic_factory_get_type (), &info);
	}

	return type;
}

void
bonobo_shlib_factory_inc_live (BonoboShlibFactory *factory)
{
	g_return_if_fail (BONOBO_IS_SHLIB_FACTORY (factory));

	factory->live_objects++;
}


static gboolean
bonobo_shlib_factory_dec_live_cb (BonoboShlibFactory *factory)
{
	factory->live_objects--;

	if (factory->live_objects <= 0)
		bonobo_object_unref (BONOBO_OBJECT (factory));

	return FALSE;
}

void
bonobo_shlib_factory_dec_live (BonoboShlibFactory *factory)
{
	g_return_if_fail (BONOBO_IS_SHLIB_FACTORY (factory));

	g_idle_add ((GSourceFunc) bonobo_shlib_factory_dec_live_cb, factory);
}

static void
destroy_handler (GtkObject *object, BonoboShlibFactory *factory)
{
	bonobo_shlib_factory_dec_live (factory);
}

void
bonobo_shlib_factory_track_object (BonoboShlibFactory *factory,
				   BonoboObject       *object)
{
	g_return_if_fail (BONOBO_IS_OBJECT (object));
	g_return_if_fail (BONOBO_IS_SHLIB_FACTORY (factory));

	bonobo_shlib_factory_inc_live (factory);

	gtk_signal_connect (GTK_OBJECT (object), "destroy",
			    destroy_handler, factory);
}
