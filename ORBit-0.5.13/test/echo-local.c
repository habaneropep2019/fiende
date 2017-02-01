#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    Echo bec;
    char buf[30];
    CORBA_double rv;
    int i;


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

#if 0
    CORBA_ORB_run(orb, &ev);
#else
    for(i = 0; i < 100; i++) {
	    g_snprintf(buf, sizeof(buf), "Hello, world [%d]", i);
	    bec = Echo_echoString(echo_client, buf, &rv, &ev);
	    if(ev._major != CORBA_NO_EXCEPTION) {
	      printf("we got exception %d from echoString!\n", ev._major);
	      return 1;
	    }

	    g_assert(echo_client == bec);

	    CORBA_Object_release(echo_client, &ev);
	    if(ev._major != CORBA_NO_EXCEPTION) {
	      printf("we got exception %d from release!\n", ev._major);
	      return 1;
	    } else
	      g_message("[client] %f", rv);

	    echo_client = bec; bec = CORBA_OBJECT_NIL;
    }

#endif

    CORBA_Object_release(echo_client, &ev);
    CORBA_Object_release((CORBA_Object)orb, &ev);

    return 0;
}

static CORBA_Object
do_echoString(PortableServer_Servant servant,
	      const CORBA_char *astring,
	      CORBA_double *outnum,
	      CORBA_Environment *ev)
{
  g_message("[server] %s", astring);

  *outnum = rand() % 100;

  return CORBA_Object_duplicate(echo_client, ev);
}
