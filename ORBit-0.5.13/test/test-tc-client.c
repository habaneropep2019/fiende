#include <stdio.h>
#include <orb/orbit.h>
#include "test-tc.h"

int main(int argc, char *argv[])
{
	CORBA_Environment ev;
	CORBA_TypeCode rettc;
	CORBA_ORB orb;
	int i;

	TestTC obj;

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
	rettc = TestTC__get_type(obj, &ev);

	printf("Returned typecode: %s (%s)\n", CORBA_TypeCode_name(rettc, &ev), CORBA_TypeCode_id(rettc, &ev));
	for (i = 0; i < CORBA_TypeCode_member_count(rettc, &ev); i++) {
		printf("Element %d: %s\n", i, CORBA_TypeCode_member_name(rettc, i, &ev));
	}

	if (ev._major != CORBA_NO_EXCEPTION) {
		printf("we got exception %d from TestAny_print!\n", ev._major);
		return 1;
	}
	CORBA_Object_release(obj, &ev);
	CORBA_Object_release((CORBA_Object) orb, &ev);

	return 0;
}
