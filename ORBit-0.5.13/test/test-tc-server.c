#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <orb/orbit.h>
#include "test-tc.h"

TestTC client;

PortableServer_ServantBase__epv base_epv =
{
	NULL,
	NULL,
	NULL
};

static CORBA_TypeCode impl_TestTC__get_type(PortableServer_Servant servant, CORBA_Environment * ev);

POA_TestTC__epv TestTC_epv =
	{NULL, impl_TestTC__get_type};
POA_TestTC__vepv poa_TestTC_vepv =
	{&base_epv, &TestTC_epv};
POA_TestTC poa_TestTC_servant =
	{NULL, &poa_TestTC_vepv};

int main(int argc, char *argv[])
{
	PortableServer_ObjectId objid =
		{0, sizeof("myTestTC"), "myTestTC"};
	PortableServer_POA poa;

	CORBA_Environment ev;
	char *retval;
	CORBA_ORB orb;

	CORBA_exception_init(&ev);
	orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

	POA_TestTC__init(&poa_TestTC_servant, &ev);

	poa = (PortableServer_POA) CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
	PortableServer_POAManager_activate(PortableServer_POA__get_the_POAManager(poa, &ev), &ev);
	PortableServer_POA_activate_object_with_id(poa,
					     &objid, &poa_TestTC_servant,
						   &ev);

	client = PortableServer_POA_servant_to_reference(poa,
						     &poa_TestTC_servant,
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

static CORBA_TypeCode impl_TestTC__get_type(PortableServer_Servant servant, CORBA_Environment * ev)
{
	return (CORBA_TypeCode)(TC_TestTCStruct);
}
