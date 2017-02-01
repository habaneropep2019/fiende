#include <stdio.h>
#include <stdlib.h>
#include <orb/orbit.h>

#include "echo.h"

Echo echo_client, bec;

#if 0
char theblah[4096];
#endif

int
main (int argc, char *argv[])
{
    CORBA_Environment ev;
    CORBA_ORB orb;
    CORBA_double rv;
    char buf[30];
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

    echo_client = CORBA_ORB_string_to_object(orb, argv[1], &ev);
    if (!echo_client) {
	printf("Cannot bind to %s\n", argv[1]);
	return 1;
    }
#ifdef LOCATE_TEST
    ORBit_object_locate(echo_client, &ev);
    if(ev._major != CORBA_NO_EXCEPTION) {
    	printf("Couldn't locate that object! (Exception %d)\n", ev._major);
	return(1);
    }
#endif
    for(i = 0; i < niters; i++) {
	    g_snprintf(buf, sizeof(buf), "Hello, world [%d]", i);
	    bec = Echo_echoString(echo_client, buf, &rv, &ev);
	    if(ev._major != CORBA_NO_EXCEPTION) {
	      printf("we got exception %d from echoString!\n", ev._major);
	      return 1;
	    }

	    /* g_assert(echo_client == bec); */

	    CORBA_Object_release(echo_client, &ev);
	    if(ev._major != CORBA_NO_EXCEPTION) {
	      printf("we got exception %d from release!\n", ev._major);
	      return 1;
	    }
#if 0
	     else
	      g_message("[client] %g", rv);
#endif

	    echo_client = bec; bec = CORBA_OBJECT_NIL;
    }

    CORBA_Object_release(echo_client, &ev);
    CORBA_Object_release((CORBA_Object)orb, &ev);

    return 0;
}
