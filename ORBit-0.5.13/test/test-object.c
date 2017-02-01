/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Test code for the POA
 *
 */

#include "orbit.h"
#include "orbit_object.h"

int main(int argc, char **argv)
{
	CORBA_Object obj;
	CORBA_Environment env;
       

	obj = ORBit_CORBA_Object_new(&env);	

	CORBA_Object_release(obj,&env);

	return(0);
}



