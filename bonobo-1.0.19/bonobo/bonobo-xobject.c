/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-xobject.c: B Unknown interface base implementation
 *
 * Authors:
 *   Michael Meeks (michael@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */

#include <config.h>
#include <stdio.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtktypeutils.h>
#include <bonobo/bonobo-xobject.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include "bonobo-running-context.h"
#include "bonobo-object-directory.h"

/* #define BONOBO_X_OBJECT_DEBUG */

/* Shared stuff with BonoboObject */
extern int  bonobo_object_get_refs (BonoboObject            *object);
extern void bonobo_object_epv_init (POA_Bonobo_Unknown__epv *epv);

/* Need to examine how ORBit sets up ORBit_RootObject_struct's
	ORBit_RootObject_Interface* interface; 
	guchar is_pseudo_object;
	gint refs;
*/
#define BONOBO_X_SERVANT_FLAG_PATTERN 0x7133

static GtkObjectClass *x_object_parent_class;

/* FIXME: cut and paste from orbit_object.c: CORBA_Object_release_fn */
static void
b_corba_object_free (BonoboXObject *object)
{
	CORBA_Object obj = BONOBO_X_OBJECT_GET_CORBA (object);

	ORBIT_ROOT_OBJECT_UNREF (obj);

	g_assert (g_hash_table_lookup (obj->orb->objrefs, obj));
	g_hash_table_remove (obj->orb->objrefs, obj);
	g_assert (!g_hash_table_lookup (obj->orb->objrefs, obj));
		
	if (obj->connection)
		giop_connection_unref (obj->connection);
	obj->connection = NULL;
	
	g_free (obj->object_id);
	obj->object_id = NULL;

	ORBit_delete_profiles (obj->profile_list);
	obj->profile_list = NULL;

	ORBit_delete_profiles (obj->forward_locations);
	obj->forward_locations = NULL;
	
	g_free (obj->vepv); /* FIXME: what ? */
	obj->vepv = NULL;

#ifdef BONOBO_X_OBJECT_DEBUG
	g_warning ("CORBA Object free '%p'", object);
#endif
}


static void
corba_cleanup (BonoboXObject *x_object)
{
	void                    *servant;
	CORBA_Environment        ev;
	BonoboXObjectClass      *klass;
	PortableServer_ObjectId *oid;

	klass = (BonoboXObjectClass *) GTK_OBJECT (x_object)->klass;
	servant = BONOBO_X_OBJECT_GET_SERVANT (x_object);

	b_corba_object_free (x_object);

	CORBA_exception_init (&ev);
			
	oid = PortableServer_POA_servant_to_id (bonobo_poa(), servant, &ev);
	PortableServer_POA_deactivate_object (bonobo_poa (), oid, &ev);
			
	if (klass->poa_fini_fn)
		klass->poa_fini_fn (servant, &ev);
	else /* Actually quicker and nicer */
		PortableServer_ServantBase__fini (servant, &ev);
			
	CORBA_free (oid);

	CORBA_exception_free (&ev);
}

static void
try_finalize (BonoboXObject *object)
{
	CORBA_Object obj = BONOBO_X_OBJECT_GET_CORBA (object);

#ifdef BONOBO_X_OBJECT_DEBUG
	g_warning ("Try finalize '%p' : %d %d on", object,
		   ORBIT_ROOT_OBJECT (obj)->refs,
		   bonobo_object_get_refs ((BonoboObject *) object));
#endif

	if (ORBIT_ROOT_OBJECT (obj)->refs <= 0) {
		if (bonobo_object_get_refs ((BonoboObject *) object) <= 0) {

			corba_cleanup (object);

			object->base.servant = NULL;
			object->base.corba_objref = CORBA_OBJECT_NIL;

			x_object_parent_class->finalize ((GtkObject *) object);
#ifdef BONOBO_X_OBJECT_DEBUG
			g_warning ("Finalized '%p'", object);
#endif
		} else
			g_warning ("Reference counting error: "
				   "Attempts to release CORBA_Object associated with "
				   "'%s' which still has a reference count of %d",
				   gtk_type_name (GTK_OBJECT (object)->klass->type),
				   bonobo_object_get_refs (BONOBO_OBJECT (object)));
	}
}

static void
bonobo_x_object_finalize_real (GtkObject *object)
{
	BonoboXObject *x_object;
	CORBA_Object   obj;
	
	x_object = BONOBO_X_OBJECT (object);
	obj = BONOBO_X_OBJECT_GET_CORBA (x_object);

	/* Setup the BonoboObject so nothing nasty happens when we chain later */
	x_object->base.servant      = NULL;
	x_object->base.corba_objref = CORBA_OBJECT_NIL;

	bonobo_running_context_remove_object (obj);

	ORBIT_ROOT_OBJECT_UNREF (obj);

	try_finalize (x_object);
}

/* cf. orbit_object.c (CORBA_Object_release_fn) */
static void
corba_release (gpointer obj, CORBA_Environment *ev)
{
	BonoboXObject *object = BONOBO_X_CORBA_GET_OBJECT (obj);

	ORBIT_ROOT_OBJECT_UNREF (obj);

	if (ORBIT_ROOT_OBJECT (obj)->refs <= 0)
		try_finalize (object);
}

static void
do_corba_hacks (BonoboXObject      *object,
		BonoboXObjectClass *klass)
{
	CORBA_Object obj;
	CORBA_Environment ev;
	BonoboXObjectClass *xklass;
	static ORBit_RootObject_Interface ri = { corba_release };

	CORBA_exception_init (&ev);

	/* Setup the servant structure */
	object->servant.servant_placeholder._private = NULL;
	object->servant.servant_placeholder.vepv     = klass->vepv;
	object->servant.bonobo_object                = (BonoboObject *) object;
	object->flags                                = BONOBO_X_SERVANT_FLAG_PATTERN;

	/* Initialize the servant structure with our POA__init fn */
	for (xklass = klass; xklass && !xklass->poa_init_fn;)
		xklass = gtk_type_class (gtk_type_parent (
			((GtkObjectClass *)xklass)->type));

	if (!xklass || !xklass->epv_struct_offset) {
		g_warning ("It looks like you used gtk_type_unique "
			   "instead of b_type_unique on type '%s'",
			   gtk_type_name (((GtkObjectClass *)klass)->type));
		return;
	}

	xklass->poa_init_fn ((PortableServer_Servant) &object->servant, &ev);

	if (BONOBO_EX (&ev)) {
		g_warning ("Exception initializing servant '%s'",
			   bonobo_exception_get_text (&ev));
		return;
	}

	/* Wierdness from bonobo-object */
	CORBA_free (PortableServer_POA_activate_object (
		bonobo_poa (), &object->servant, &ev));

	/* Instantiate a CORBA_Object reference for the servant */
	obj = PortableServer_POA_servant_to_reference (
		bonobo_poa (), &object->servant, &ev);

	/* FIXME: Grab ownership of the CORBA_Object reference by brute force */
	{
		memcpy (&object->object, obj, sizeof (object->object));

		if (ORBIT_ROOT_OBJECT (obj)->refs > 1)
			g_error ("Impossible object instantiation collision");

		/* De-register the old object */
		g_assert (g_hash_table_lookup (obj->orb->objrefs, obj));
		g_hash_table_remove (obj->orb->objrefs, obj);
		g_assert (!g_hash_table_lookup (obj->orb->objrefs, obj));

		/* Re-register the new object */
		g_hash_table_insert (obj->orb->objrefs,
				     &object->object, &object->object);

		ORBIT_CHUNK_FREE (CORBA_Object, obj);

		/* Override the CORBA_Object's release fn */
		object->object.parent.interface = &ri;

	}

	bonobo_running_context_add_object (BONOBO_X_OBJECT_GET_CORBA (object));

	CORBA_exception_free (&ev);
}

static void
bonobo_x_object_instance_init (GtkObject    *gtk_object,
			       GtkTypeClass *klass)
{
	BonoboXObject *object = BONOBO_X_OBJECT (gtk_object);

	GTK_OBJECT_UNSET_FLAGS (GTK_OBJECT (object), GTK_FLOATING);

	do_corba_hacks (object, BONOBO_X_OBJECT_CLASS (klass));

	object->base.corba_objref = BONOBO_X_OBJECT_GET_CORBA   (object);
	object->base.servant      = BONOBO_X_OBJECT_GET_SERVANT (object);

#ifdef BONOBO_X_OBJECT_DEBUG
	g_warning ("bonobo_x_object_instance init '%s' '%s' -> %p (%d, %d)",
		   gtk_type_name (gtk_object->klass->type),
		   gtk_type_name (klass->type), object,
		   ORBIT_ROOT_OBJECT (object)->refs,
		   bonobo_object_get_refs ((BonoboObject *) object));
#endif
}

static void
bonobo_x_object_class_init (BonoboXObjectClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	x_object_parent_class = gtk_type_class (bonobo_object_get_type ());

	object_class->finalize = bonobo_x_object_finalize_real;
}

/**
 * bonobo_x_object_get_type:
 *
 * Returns: the GtkType associated with the base BonoboXObject class type.
 */
GtkType
bonobo_x_object_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboXObject",
			sizeof (BonoboXObject),
			sizeof (BonoboXObjectClass),
			(GtkClassInitFunc) bonobo_x_object_class_init,
			(GtkObjectInitFunc) bonobo_x_object_instance_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (
			bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_x_type_setup:
 * @type: The type to initialize
 * @init_fn: the POA_init function for the CORBA interface or NULL
 * @fini_fn: NULL or a custom POA free fn.
 * @epv_struct_offset: the offset in the class structure where the epv is or 0
 * 
 *   This function initializes a type derived from BonoboXObject, such that
 * when you instantiate a new object of this type with gtk_type_new the
 * CORBA object will be correctly created and embedded.
 * 
 * Return value: TRUE on success, FALSE on error.
 **/
gboolean
bonobo_x_type_setup (GtkType            type,
		     BonoboXObjectPOAFn init_fn,
		     BonoboXObjectPOAFn fini_fn,
		     int                epv_struct_offset)
{
	GtkType       p, b_type;
	int           depth;
	BonoboXObjectClass *klass;
	gpointer     *vepv;

	/* Setup our class data */
	klass = gtk_type_class (type);
	klass->epv_struct_offset = epv_struct_offset;
	klass->poa_init_fn       = init_fn;
	klass->poa_fini_fn       = fini_fn;

	/* Calculate how far down the tree we are in epvs */
	b_type = bonobo_x_object_get_type ();
	for (depth = 0, p = type; p && p != b_type;
	     p = gtk_type_parent (p)) {
		BonoboXObjectClass *xklass;

		xklass = gtk_type_class (p);

		if (xklass->epv_struct_offset)
			depth++;
	}
	if (!p) {
		g_warning ("Trying to inherit '%s' from a BonoboXObject, but "
			   "no BonoboXObject in the ancestory",
			   gtk_type_name (type));
		return FALSE;
	}

#ifdef BONOBO_X_OBJECT_DEBUG
	g_warning ("We are at depth %d with type '%s'",
		   depth, gtk_type_name (type));
#endif

	/* Setup the Unknown epv */
	bonobo_object_epv_init (&klass->epv);
	klass->epv._private = NULL;
	
	vepv = g_new0 (gpointer, depth + 2);
	klass->vepv = (POA_Bonobo_Unknown__vepv *) vepv;
	klass->vepv->_base_epv = NULL;
	klass->vepv->Bonobo_Unknown_epv = &klass->epv;

	/* Build our EPV */
	if (depth > 0) {
		int i;

		for (p = type, i = depth; i > 0;) {
			BonoboXObjectClass *xklass;

			xklass = gtk_type_class (p);

			if (xklass->epv_struct_offset) {
				vepv [i + 1] = ((guchar *)klass) +
					xklass->epv_struct_offset;
				i--;
			}

			p = gtk_type_parent (p);
		}
	}

	return TRUE;
}

/**
 * bonobo_x_type_unique:
 * @parent_type: the parent Gtk Type
 * @init_fn: a POA initialization function
 * @fini_fn: a POA finialization function or NULL
 * @epv_struct_offset: the offset into the struct that the epv
 * commences at, or 0 if we are inheriting a plain Gtk Object
 * from a BonoboXObject, adding no new CORBA interfaces
 * @info: the standard GtkTypeInfo.
 * 
 * This function is the main entry point for deriving bonobo
 * server interfaces.
 * 
 * Return value: the constructed Gtk Type.
 **/
GtkType
bonobo_x_type_unique (GtkType            parent_type,
		      BonoboXObjectPOAFn init_fn,
		      BonoboXObjectPOAFn fini_fn,
		      int                epv_struct_offset,
		      const GtkTypeInfo *info)
{
	GtkType       type;

	/*
	 *  Since we call gtk_type_class after the gtk_type_unique
	 * and before we can return the type to the get_type fn.
	 * it is possible we can re-enter here through eg. a
	 * type check macro, hence we need this guard.
	 */
	if ((type = gtk_type_from_name (info->type_name)))
		return type;

	type = gtk_type_unique (parent_type, info);
	if (!type)
		return 0;

	if (bonobo_x_type_setup (type, init_fn, fini_fn,
				 epv_struct_offset))
		return type;
	else
		return 0;
}

/**
 * bonobo_x_object:
 * @p: a pointer to something
 * 
 * This function can be passed a BonoboXObject * or a
 * PortableServer_Servant, and it will return a BonoboXObject *.
 * 
 * Return value: a BonoboXObject or NULL.
 **/
BonoboXObject *
bonobo_x_object (gpointer p)
{
	BonoboXObject *xobj;

	if (!p)
		return NULL;

	xobj = p;

	if (xobj->base.corba_objref ==
	    BONOBO_X_OBJECT_GET_CORBA (xobj) &&
	    xobj->base.servant ==
	    BONOBO_X_OBJECT_GET_SERVANT (xobj))
		return xobj;

	else {
		BonoboObjectServant *s = p;

		if ((xobj = s->bonobo_object) ==
		    BONOBO_X_SERVANT_GET_OBJECT (s) &&
		    &xobj->servant == p)
			return xobj;
		else
			g_warning ("Serious error, unidentifiable "
				   "pointer type");
	}

	return NULL;
}
