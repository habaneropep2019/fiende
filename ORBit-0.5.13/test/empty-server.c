#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <orb/orbit.h>
#include "empty.h"

Empty empty_client = CORBA_OBJECT_NIL;

static void do_Nothing(PortableServer_Servant servant, CORBA_Environment *ev);

PortableServer_ServantBase__epv base_epv = {
  NULL,
  NULL,
  NULL
};
POA_Empty__epv empty_epv = { NULL, do_Nothing };
POA_Empty__vepv poa_empty_vepv = { &base_epv, &empty_epv };
POA_Empty poa_empty_servant = { NULL, &poa_empty_vepv };

static void do_exit(int arg)
{
	exit(2);
}

int
main (int argc, char *argv[])
{
    PortableServer_ObjectId objid = {0, sizeof("myFoo"), "myFoo"};
    PortableServer_POA poa;

    CORBA_Environment ev;
    char *retval;
    CORBA_ORB orb;

    signal(SIGINT, do_exit);
    signal(SIGTERM, do_exit);

    CORBA_exception_init(&ev);
    orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

    POA_Empty__init(&poa_empty_servant, &ev);

    poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
    PortableServer_POAManager_activate(PortableServer_POA__get_the_POAManager(poa, &ev), &ev);
    PortableServer_POA_activate_object_with_id(poa,
					       &objid, &poa_empty_servant, &ev);

    empty_client = PortableServer_POA_servant_to_reference(poa,
							&poa_empty_servant,
							  &ev);
    if (!empty_client) {
	printf("Cannot get objref\n");
	return 1;
    }

    retval = CORBA_ORB_object_to_string(orb, empty_client, &ev);

    g_print("%s\n", retval); fflush(stdout);

    CORBA_free(retval);

    CORBA_ORB_run(orb, &ev);

    PortableServer_POA_deactivate_object(poa, &objid, &ev);

    return 0;
}

static void
do_Nothing(PortableServer_Servant servant,
	      CORBA_Environment *ev)
{
}
