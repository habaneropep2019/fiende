/*
 * main.c: Startup code for the Echo Bonobo Component.
 *
 * Author:
 *   Miguel de Icaza (miguel@helixcode.com)
 *
 * (C) 1999, 2000 Helix Code, Inc.  http://www.helixcode.com
 */
#include <config.h>
#include <bonobo.h>

#include "Bonobo_Sample_Echo.h"
#include "echo.h"

static BonoboObject *
echo_factory (BonoboGenericFactory *this_factory, void *data)
{
	Echo *echo;

	echo = echo_new ();
	
	if (echo == NULL) 
		return NULL;
   
	return BONOBO_OBJECT (echo);
}

int
main (int argc, char *argv [])
{
	BonoboGenericFactory *factory;
	CORBA_Environment ev;
	CORBA_ORB orb;
	CORBA_exception_init (&ev);

	gnome_init_with_popt_table ("echo", "0.37", argc, argv,
				    oaf_popt_options, 0, CORBA_OBJECT_NIL);
	orb = oaf_init (argc, argv);
	if (!bonobo_init (orb, CORBA_OBJECT_NIL, CORBA_OBJECT_NIL))
		g_error (_("Could not initialize Bonobo" ));

	factory = bonobo_generic_factory_new ("OAFIID:Bonobo_Sample_Echo_Factory",
					      echo_factory, NULL);

	/* NB. normaly we would want the server to quit here when we ran out
	 * of live objects, in this case we do not */

	bonobo_main ();

	CORBA_exception_free (&ev);

	return 0;
}
