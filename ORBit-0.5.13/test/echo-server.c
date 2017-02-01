#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <orb/orbit.h>
#include "echo.h"

Echo echo_client = CORBA_OBJECT_NIL;

static CORBA_Object
do_echoString(PortableServer_Servant servant,
	      const CORBA_char *astring,
	      CORBA_double *outnum,
	      CORBA_Environment *ev);

PortableServer_ServantBase__epv base_epv = {
  NULL,
  NULL,
  NULL
};
POA_Echo__epv echo_epv = { NULL, do_echoString };
POA_Echo__vepv poa_echo_vepv = { &base_epv, &echo_epv };
POA_Echo poa_echo_servant = { NULL, &poa_echo_vepv };

int
main (int argc, char *argv[])
{
    PortableServer_ObjectId objid = {0, sizeof("myEchoString"), "myEchoString"};
    PortableServer_POA poa;

    CORBA_Environment ev;
    char *retval;
    CORBA_ORB orb;

    signal(SIGINT, exit);
    signal(SIGTERM, exit);

    CORBA_exception_init(&ev);
    orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

    POA_Echo__init(&poa_echo_servant, &ev);

    poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
    PortableServer_POAManager_activate(PortableServer_POA__get_the_POAManager(poa, &ev), &ev);
    PortableServer_POA_activate_object_with_id(poa,
					       &objid, &poa_echo_servant, &ev);

    echo_client = PortableServer_POA_servant_to_reference(poa,
							  &poa_echo_servant,
							  &ev);
    if (!echo_client) {
	printf("Cannot get objref\n");
	return 1;
    }

    retval = CORBA_ORB_object_to_string(orb, echo_client, &ev);

    g_print("%s\n", retval); fflush(stdout);

    CORBA_free(retval);

    CORBA_ORB_run(orb, &ev);

    return 0;
}

static CORBA_Object
do_echoString(PortableServer_Servant servant,
	      const CORBA_char *astring,
	      CORBA_double *outnum,
	      CORBA_Environment *ev)
{
  *outnum = (double)(rand() % 100)/100.0;

  /* g_message("[server] %s -> %d", astring, *outnum); */

  return CORBA_Object_duplicate(echo_client, ev);
}
