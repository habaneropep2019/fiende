#include <stdio.h>
#include <stdlib.h>
#include <orb/orbit.h>

#include "empty.h"

Empty empty_client;

int
main (int argc, char *argv[])
{
    CORBA_Environment ev;
    CORBA_ORB orb;
    int i;

    int niters = 100;

    CORBA_exception_init(&ev);
    orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

#if 0
    for(i = 0; i < (sizeof(theblah) - 1); i++)
	theblah[i] = 'a';
    theblah[sizeof(theblah) - 1] = '\0';
#endif

    if(argc < 2)
      {
	printf("Need a binding ID thing as argv[1]\n");
	return 1;
      }

    if(argc == 3)
	niters = atoi(argv[2]);

    empty_client = CORBA_ORB_string_to_object(orb, argv[1], &ev);
    if (!empty_client) {
	printf("Cannot bind to %s\n", argv[1]);
	return 1;
    }

    printf("corba = %d, empty = %d, foobar = %d\n",
	CORBA_Object_is_a(empty_client, "IDL:CORBA/Object:1.0", &ev),
	CORBA_Object_is_a(empty_client, "IDL:Empty:1.0", &ev),
	CORBA_Object_is_a(empty_client, "IDL:Foo/Bar:1.0", &ev));

    for(i = 0; i < niters; i++) {
	    Empty_doNothing(empty_client, &ev);
	    if(ev._major != CORBA_NO_EXCEPTION) {
	      printf("we got exception %d from doNothing!\n", ev._major);
	      return 1;
	    }
    }

    CORBA_Object_release(empty_client, &ev);
    CORBA_Object_release((CORBA_Object)orb, &ev);

    return 0;
}
