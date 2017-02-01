/**
 * Bonobo property object implementation.
 *
 * Author:
 *    Nat Friedman (nat@nat.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */

#include <config.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-bag.h>

typedef struct {
	POA_Bonobo_Property		 prop;
	BonoboPropertyBag		*pb;
	BonoboTransient                 *transient;
	char				*property_name;
} BonoboPropertyServant;

static CORBA_char *
impl_Bonobo_Property_getName (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	BonoboPropertyServant *ps = (BonoboPropertyServant *) servant;

	return CORBA_string_dup (ps->property_name);
}

static CORBA_TypeCode
impl_Bonobo_Property_getType (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	BonoboPropertyServant *ps = (BonoboPropertyServant *) servant;
	BonoboArgType          type;

	type = bonobo_property_bag_get_property_type (ps->pb, ps->property_name, ev);
	/* FIXME: we need to handle obscure cases like non existance of the property */

	return (CORBA_TypeCode) CORBA_Object_duplicate ((CORBA_Object) (type), ev);
}

static CORBA_any *
impl_Bonobo_Property_getValue (PortableServer_Servant servant,
				CORBA_Environment *ev)
{
	BonoboPropertyServant *ps = (BonoboPropertyServant *) servant;

	return bonobo_property_bag_get_value (ps->pb, ps->property_name, ev);
}

static void
impl_Bonobo_Property_setValue (PortableServer_Servant servant,
				const CORBA_any       *any,
				CORBA_Environment     *ev)
{
	BonoboPropertyServant *ps = (BonoboPropertyServant *) servant;

	bonobo_property_bag_set_value (ps->pb, ps->property_name, any, ev);
}

static CORBA_any *
impl_Bonobo_Property_getDefault (PortableServer_Servant servant,
				  CORBA_Environment *ev)
{
	BonoboPropertyServant *ps = (BonoboPropertyServant *) servant;

	return bonobo_property_bag_get_default (ps->pb, ps->property_name, ev);
}

static CORBA_char *
impl_Bonobo_Property_getDocString (PortableServer_Servant servant,
				     CORBA_Environment *ev)
{
	BonoboPropertyServant *ps = (BonoboPropertyServant *) servant;

	return CORBA_string_dup (bonobo_property_bag_get_docstring (ps->pb, 
		ps->property_name, ev));
}


static CORBA_long
impl_Bonobo_Property_getFlags (PortableServer_Servant servant,
				CORBA_Environment *ev)
{
	BonoboPropertyServant *ps = (BonoboPropertyServant *) servant;

	return bonobo_property_bag_get_flags (ps->pb, ps->property_name, ev);
}

static Bonobo_EventSource_ListenerId
impl_Bonobo_Property_addListener (PortableServer_Servant servant,
				  const Bonobo_Listener  l,
				  CORBA_Environment     *ev)
{
	BonoboPropertyServant *ps = (BonoboPropertyServant *) servant;
	Bonobo_Unknown corba_es;
	Bonobo_EventSource_ListenerId id = 0;
	char *mask;

	corba_es = BONOBO_OBJREF (ps->pb->es);

	mask = g_strdup_printf ("Bonobo/Property:change:%s", 
				ps->property_name);

	id = Bonobo_EventSource_addListenerWithMask (corba_es, l, mask, ev); 

	g_free (mask);

	return id;
}

static void
impl_Bonobo_Property_removeListener (PortableServer_Servant servant,
				     const Bonobo_EventSource_ListenerId id,
				     CORBA_Environment     *ev)
{
	BonoboPropertyServant *ps = (BonoboPropertyServant *) servant;
	Bonobo_Unknown corba_es;

	corba_es = BONOBO_OBJREF (ps->pb->es);

	Bonobo_EventSource_removeListener (corba_es, id, ev); 
}

static POA_Bonobo_Property__epv *
bonobo_property_get_epv (void)
{
	static POA_Bonobo_Property__epv *epv = NULL;

	if (epv != NULL)
		return epv;

	epv = g_new0 (POA_Bonobo_Property__epv, 1);

	epv->getName        = impl_Bonobo_Property_getName;
	epv->getType        = impl_Bonobo_Property_getType;
	epv->getValue       = impl_Bonobo_Property_getValue;
	epv->setValue       = impl_Bonobo_Property_setValue;
	epv->getDefault     = impl_Bonobo_Property_getDefault;
	epv->getDocString   = impl_Bonobo_Property_getDocString;
	epv->getFlags       = impl_Bonobo_Property_getFlags;
	epv->addListener    = impl_Bonobo_Property_addListener;
	epv->removeListener = impl_Bonobo_Property_removeListener;

	return epv;
}

static void
impl_Bonobo_Property_ref (PortableServer_Servant servant, 
			  CORBA_Environment *ev)
{
	/* nothing to do */
}

static void
impl_Bonobo_Property_unref (PortableServer_Servant servant, 
			    CORBA_Environment *ev)
{
	/* nothing to do */
}

static CORBA_Object
impl_Bonobo_Property_queryInterface (PortableServer_Servant  servant,
				     const CORBA_char       *repoid,
				     CORBA_Environment      *ev)
{
	BonoboPropertyServant *ps = (BonoboPropertyServant *) servant;

	if (!strcmp (repoid, "IDL:Bonobo/Property:1.0"))
		return bonobo_transient_create_objref (ps->transient,
			"IDL:Bonobo/Property:1.0", ps->property_name, 
			ev);
	else
		return CORBA_OBJECT_NIL;
}

static POA_Bonobo_Unknown__epv *
bonobo_property_get_unknown_epv (void)
{
	POA_Bonobo_Unknown__epv *epv;

	epv = g_new0 (POA_Bonobo_Unknown__epv, 1);

	epv->ref            = impl_Bonobo_Property_ref;
	epv->unref          = impl_Bonobo_Property_unref;
	epv->queryInterface = impl_Bonobo_Property_queryInterface;

	return epv;
}

static POA_Bonobo_Property__vepv *
bonobo_property_get_vepv (void)
{
	static POA_Bonobo_Property__vepv *vepv = NULL;

	if (vepv != NULL)
		return vepv;

	vepv = g_new0 (POA_Bonobo_Property__vepv, 1);

	vepv->Bonobo_Property_epv = bonobo_property_get_epv ();
	vepv->Bonobo_Unknown_epv = bonobo_property_get_unknown_epv ();
	
	return vepv;
}

PortableServer_Servant
bonobo_property_servant_new (PortableServer_POA     poa,
			     BonoboTransient        *bt,
			     char                   *property_name,
			     void                   *callback_data)
{
	BonoboPropertyServant	*servant;
	BonoboPropertyBag       *pb = (BonoboPropertyBag *)callback_data;
	CORBA_Environment        ev;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (bt != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (property_name != NULL, NULL);

	/*
	 * Verify that the specified property exists.
	 */
	if (! bonobo_property_bag_has_property (pb, property_name))
		return CORBA_OBJECT_NIL;

	CORBA_exception_init (&ev);

	/*
	 * Create a transient servant for the property.
	 */
	servant = g_new0 (BonoboPropertyServant, 1);

	servant->property_name = g_strdup (property_name);
	servant->transient = bt;
	servant->pb = pb;

	((POA_Bonobo_Property *) servant)->vepv = bonobo_property_get_vepv ();
	
	POA_Bonobo_Property__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("BonoboProperty: Could not initialize Property servant");
		g_free (servant->property_name);
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	return servant;
}

void
bonobo_property_servant_destroy (PortableServer_Servant  servant,
				 void                   *callback_data)
{
	CORBA_Environment ev;

	g_return_if_fail (servant != NULL);

	CORBA_exception_init (&ev);

	POA_Bonobo_Property__fini ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("BonoboProperty: Could not deconstruct Property servant");
		CORBA_exception_free (&ev);
		return;
	}

	CORBA_exception_free (&ev);

	g_free (((BonoboPropertyServant *) servant)->property_name);
	g_free (servant);
}
