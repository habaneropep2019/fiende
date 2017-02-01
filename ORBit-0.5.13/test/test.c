/*
 * Simple test for the ORB.
 *
 * Run as eg: 
 * test -ORBImplRepoIOR=IOR:0100000010000000646174613a3a70686f6e65626f6f6b0001000000000000003a0000000101000023000000686167626172642e6170617468657469632e646973636f726469612e6f72672e756b0000d72b000006000000776962626c65 -ORBDebugModules 3 -ORBDebugLevel 8
 */

#include <stdio.h>
#include "orbit.h"

int main(int argc, char **argv)
{
	CORBA_ORB orb;
	CORBA_ORB_ObjectIdList *ref_list;
	CORBA_Environment ev;
	int i;

	orb=CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);
	ref_list=CORBA_ORB_list_initial_services(orb, &ev);

	for(i=0;i<ref_list->_length; i++) {
		printf("Ref: %s\n", ref_list->_buffer[i]);
	}

	return(0);
}
