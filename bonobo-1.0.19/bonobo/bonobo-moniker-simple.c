/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-moniker-simple: Simplified object naming abstraction
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, Helix Code, Inc.
 */
#include <config.h>

#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-simple.h>

static Bonobo_Unknown
simple_resolve (BonoboMoniker               *moniker,
		const Bonobo_ResolveOptions *options,
		const CORBA_char            *requested_interface,
		CORBA_Environment           *ev)
{
	BonoboMonikerSimple *simple;

	g_return_val_if_fail (BONOBO_IS_MONIKER_SIMPLE (moniker),
			      CORBA_OBJECT_NIL);

	simple = BONOBO_MONIKER_SIMPLE (moniker);

	return simple->resolve_fn (moniker, options, requested_interface, ev);
}

static void
bonobo_moniker_simple_class_init (BonoboMonikerClass *klass)
{
	klass->resolve = simple_resolve;
}

static void 
bonobo_moniker_simple_init (GtkObject *object)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC (BonoboMonikerSimple, 
		      bonobo_moniker_get_type (),
		      bonobo_moniker_simple);

/**
 * bonobo_moniker_simple_construct:
 * @moniker: the moniker to construct
 * @name: the name of the moniker eg. 'file:'
 * @resolve_fn: the function used to resolve the moniker
 * 
 * Constructs a simple moniker
 * 
 * Return value: the constructed moniker or NULL on failure.
 **/
BonoboMoniker *
bonobo_moniker_simple_construct (BonoboMonikerSimple         *moniker,
				 const char                  *name,
				 BonoboMonikerSimpleResolveFn resolve_fn)
{
	g_return_val_if_fail (resolve_fn != NULL, NULL);

	moniker->resolve_fn = resolve_fn;

	return bonobo_moniker_construct (
		BONOBO_MONIKER (moniker), name);
}

/**
 * bonobo_moniker_simple_new:
 * @name: the display name for the moniker
 * @resolve_fn: a resolve function for the moniker
 * 
 * Create a new instance of a simplified moniker.
 * 
 * Return value: the moniker object
 **/
BonoboMoniker *
bonobo_moniker_simple_new (const char                  *name,
			   BonoboMonikerSimpleResolveFn resolve_fn)
{
	BonoboMoniker *moniker;

	moniker = gtk_type_new (bonobo_moniker_simple_get_type ());

	return bonobo_moniker_simple_construct (
		BONOBO_MONIKER_SIMPLE (moniker),
		name, resolve_fn);
}

