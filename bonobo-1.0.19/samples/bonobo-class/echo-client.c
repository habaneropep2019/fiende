/*
 * Sample user for the Echo Bonobo component
 *
 * Author:
 *   Miguel de Icaza  (miguel@helixcode.com)
 *
 */


#include <config.h>
#include <gnome.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>
#include "Bonobo_Sample_Echo.h"

static void
init_bonobo (int argc, char *argv [])
{
	CORBA_ORB orb;

        gnome_init_with_popt_table (
		"echo-client", "1.0",
		argc, argv,
		oaf_popt_options, 0, NULL); 

	orb = oaf_init (argc, argv);

	if (!bonobo_init (orb, CORBA_OBJECT_NIL,
			  CORBA_OBJECT_NIL))
		g_error (_("I could not initialize Bonobo"));

	/*
	 * Enable CORBA/Bonobo to start processing requests
	 */
	bonobo_activate ();
}

static void
usage (void)
{
	fprintf (stderr, "To use this program run oaf-slay, then type\n"
		 "bonobo-echo & # to register the echoing server\n"
		 "then run echo-client to see the echo\n");
}

int 
main (int argc, char *argv [])
{
	Bonobo_Sample_Echo  echo_server;
	CORBA_Environment   ev;
	char               *obj_id;

	init_bonobo (argc, argv);

	obj_id = "OAFIID:Bonobo_Sample_Echo";

	CORBA_exception_init (&ev);

	echo_server = bonobo_get_object (obj_id, "IDL:Bonobo/Sample/Echo:1.0", &ev);

	if (echo_server == CORBA_OBJECT_NIL) {
		printf ("Could not create an instance of the %s component", obj_id);
		return 1;
	}

	/* Send a message */
	Bonobo_Sample_Echo_echo (echo_server, "This is the message from the client\n", &ev);
	if (BONOBO_EX (&ev))
		usage ();

	CORBA_exception_free (&ev);

	bonobo_object_release_unref (echo_server, NULL);
	
	return 0;
}
