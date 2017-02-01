#include <stdio.h>
#include <orb/orbit.h>
#include "test-ir.h"

int main(int argc, char *argv[])
{
	CORBA_Environment ev;
	Test_Inter obj;
	CORBA_ORB orb;
	CORBA_InterfaceDef interface_def;
	int i;

	CORBA_exception_init(&ev);
	orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

	if (argc < 2) {
		printf("Need a binding ID thing as argv[1]\n");
		return 1;
	}
	obj = CORBA_ORB_string_to_object(orb, argv[1], &ev);

	if (!obj) {
		printf("Cannot bind to %s\n", argv[1]);
		return 1;
	}

	interface_def = CORBA_Object_get_interface(obj, &ev);
	if(interface_def == CORBA_OBJECT_NIL) {
		printf("Didn't get an InterfaceDef\n");
	} else {
		CORBA_InterfaceDef_FullInterfaceDescription *desc=CORBA_InterfaceDef_describe_interface(interface_def, &ev);
		printf("Name           : %s\n", desc->name);
		printf("ID             : %s\n", desc->id);
		printf("Defined in     : %s\n", desc->defined_in);
		printf("Version        : %s\n", desc->version);
		printf("Operations     : \n");
		for(i=0;i<desc->operations._length;i++) {
			printf("\t\t%d) %s\n", i, desc->operations._buffer[i].name);
		}
		printf("Attributes     : \n");
		for(i=0;i<desc->attributes._length;i++) {
			printf("\t\t%d) %s\n", i, desc->attributes._buffer[i].name);
		}
		printf("Base interfaces: \n");
		for(i=0;i<desc->base_interfaces._length;i++) {
			printf("\t\t%d) %s\n", i, desc->base_interfaces._buffer[i]);
		}
	}

	if (ev._major != CORBA_NO_EXCEPTION) {
		printf("we got exception %d from TestAny_print!\n", ev._major);
		return 1;
	}
	CORBA_Object_release(interface_def, &ev);
	CORBA_Object_release(obj, &ev);
	CORBA_Object_release((CORBA_Object) orb, &ev);

	return 0;
}
