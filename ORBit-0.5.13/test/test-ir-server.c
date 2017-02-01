#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <orb/orbit.h>
#include "test-ir.h"

PortableServer_ServantBase__epv base_epv =
{
	NULL,
	NULL,
	NULL
};

static CORBA_long impl_Test_Inter__get_a1(PortableServer_Servant servant, CORBA_Environment *ev);
static void impl_Test_Inter__set_a1(PortableServer_Servant servant, CORBA_long value, CORBA_Environment *ev);
static CORBA_long impl_Test_Inter__get_a2(PortableServer_Servant servant, CORBA_Environment *ev);
static CORBA_long impl_Test_Inter_method1(PortableServer_Servant servant, CORBA_long arg1, CORBA_long *arg2, CORBA_Environment * ev);

POA_Test_Inter__epv Test_Inter_epv =
	{NULL,
	impl_Test_Inter__get_a1,
	impl_Test_Inter__set_a1,
	impl_Test_Inter__get_a2,
	impl_Test_Inter_method1};
POA_Test_Inter__vepv poa_Test_Inter_vepv =
	{&base_epv, &Test_Inter_epv};
POA_Test_Inter poa_Test_Inter_servant =
	{NULL, &poa_Test_Inter_vepv};

int main(int argc, char *argv[])
{
	PortableServer_ObjectId objid =
		{0, sizeof("myTest_Inter"), "myTest_Inter"};
	PortableServer_POA poa;

	CORBA_Environment ev;
	char *retval;
	CORBA_ORB orb;
	Test_Inter client;

	CORBA_exception_init(&ev);
	orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

	POA_Test_Inter__init(&poa_Test_Inter_servant, &ev);

	poa = (PortableServer_POA) CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
	PortableServer_POAManager_activate(PortableServer_POA__get_the_POAManager(poa, &ev), &ev);
	PortableServer_POA_activate_object_with_id(poa,
					     &objid, &poa_Test_Inter_servant,
						   &ev);

	client = PortableServer_POA_servant_to_reference(poa,
						     &poa_Test_Inter_servant,
							 &ev);
	if (!client) {
		printf("Cannot get objref\n");
		return 1;
	}
	retval = CORBA_ORB_object_to_string(orb, client, &ev);

	g_print("%s\n", retval);
	fflush(stdout);

	CORBA_free(retval);

	CORBA_ORB_run(orb, &ev);

	return 0;
}

static CORBA_long impl_Test_Inter_method1(PortableServer_Servant servant, CORBA_long arg1, CORBA_long *arg2, CORBA_Environment * ev)
{
	return (1);
}

static CORBA_long impl_Test_Inter__get_a1(PortableServer_Servant servant, CORBA_Environment *ev)
{
	return(2);
}

static void impl_Test_Inter__set_a1(PortableServer_Servant servant, CORBA_long value, CORBA_Environment *ev)
{
}

static CORBA_long impl_Test_Inter__get_a2(PortableServer_Servant servant, CORBA_Environment *ev)
{
	return(3);
}
