/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-activation-context.c: A global activation interface
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-moniker-extender.h>
#include <bonobo/bonobo-activation-context.h>

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

static Bonobo_Moniker
impl_Bonobo_ActivationContext_createWithParent (PortableServer_Servant servant,
						Bonobo_Moniker         optParent,
						const CORBA_char      *name,
						CORBA_Environment     *ev)
{
	return bonobo_moniker_util_new_from_name_full (
		optParent, name, ev);
}

static Bonobo_Moniker
impl_Bonobo_ActivationContext_createFromName (PortableServer_Servant servant,
					      const CORBA_char      *name,
					      CORBA_Environment     *ev)
{
	return impl_Bonobo_ActivationContext_createWithParent (
		servant, CORBA_OBJECT_NIL, name, ev);
}

static Bonobo_Unknown
impl_Bonobo_ActivationContext_getObject (PortableServer_Servant servant,
					 const CORBA_char      *name,
					 const CORBA_char      *repo_id,
					 CORBA_Environment     *ev)
{
	return bonobo_get_object (name, repo_id, ev);
}

static Bonobo_MonikerExtender
impl_Bonobo_ActivationContext_getExtender (PortableServer_Servant servant,
					   const CORBA_char      *monikerPrefix,
					   const CORBA_char      *interfaceId,
					   CORBA_Environment     *ev)
{
	return bonobo_moniker_find_extender (monikerPrefix, interfaceId, ev);
}

static void
bonobo_activation_context_class_init (BonoboActivationContextClass *klass)
{
	POA_Bonobo_ActivationContext__epv *epv = &klass->epv;

	epv->getObject        = impl_Bonobo_ActivationContext_getObject;
	epv->createFromName   = impl_Bonobo_ActivationContext_createFromName;
	epv->createWithParent = impl_Bonobo_ActivationContext_createWithParent;
	epv->getExtender      = impl_Bonobo_ActivationContext_getExtender;
}

static void 
bonobo_activation_context_init (GtkObject *object)
{
	/* nothing to do */
}

static
BONOBO_X_TYPE_FUNC_FULL (BonoboActivationContext,
			   Bonobo_ActivationContext,
			   PARENT_TYPE,
			   bonobo_activation_context);

BonoboObject *
bonobo_activation_context_new (void)
{
        return BONOBO_OBJECT (gtk_type_new (
		bonobo_activation_context_get_type ()));
}
