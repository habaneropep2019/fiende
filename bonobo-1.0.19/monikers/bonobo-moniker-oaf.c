/*
 * gnome-moniker-oaf.c: Sample oaf-system based Moniker implementation
 *
 * This is the oaf-activation based Moniker implementation.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 */
#include <config.h>

#include <liboaf/liboaf.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-exception.h>

#include "bonobo-moniker-std.h"

Bonobo_Unknown
bonobo_moniker_oaf_resolve (BonoboMoniker               *moniker,
			    const Bonobo_ResolveOptions *options,
			    const CORBA_char            *requested_interface,
			    CORBA_Environment           *ev)
{
	Bonobo_Moniker       parent;
	Bonobo_Unknown       object;
	
	parent = bonobo_moniker_get_parent (moniker, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;
	
	if (parent != CORBA_OBJECT_NIL) {
		bonobo_object_release_unref (parent, ev);

		g_warning ("wierd; oafid moniker with a parent; strange");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		return CORBA_OBJECT_NIL;
	}

	object = oaf_activate_from_id (
		(char *) bonobo_moniker_get_name_full (moniker), 0, NULL, ev);


	if (BONOBO_USER_EX (ev, "IDL:OAF/GeneralError:1.0")) {
		OAF_GeneralError *error;

		error = ev->_params;
		g_message ("OAF error activating component: %s", error->description);
		return CORBA_OBJECT_NIL;
	}
/*	g_warning ("Activate from ID '%s' = '%p'", bonobo_moniker_get_name_full (moniker)); */

	return bonobo_moniker_util_qi_return (object, requested_interface, ev);
}
