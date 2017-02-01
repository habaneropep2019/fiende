/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 * Test code for the POA
 */

#include <stdio.h>
#include <stdlib.h>
#include "orbit.h"

/* orb internals */
typedef struct _ORBit_ServantPrivate {
	CORBA_long nvtables;
}  ORBit_ServantPrivate;

typedef struct _ORBit_EpvPrvivate {
	CORBA_TypeCode type;
} ORBit_EpvPrivate;


/* What the IDL compiler spits out */

typedef struct POA_Counter__epv { 
	void* _private; 
	CORBA_long (*add)(PortableServer_Servant servant, CORBA_long val, CORBA_Environment* env); 
} POA_Counter__epv;

typedef struct POA_Counter__vepv { 
	PortableServer_ServantBase__epv* _base_epv; 
	POA_Counter__epv* Counter_epv; 
} POA_Counter__vepv;

typedef struct POA_Counter { 
	void* _private; 
	POA_Counter__vepv* vepv; 
} POA_Counter;

void 
POA_Counter__init (POA_Counter* servant, CORBA_Environment* env)
{
	ORBit_ServantPrivate *servant_private = g_new (ORBit_ServantPrivate, 1);
	ORBit_EpvPrivate *epv1_private = g_new (ORBit_EpvPrivate, 1);
	ORBit_EpvPrivate *epv2_private = g_new (ORBit_EpvPrivate, 1);
	
	servant_private->nvtables = 2;
	servant->_private = servant_private;
	
	
	
	epv1_private->type = NULL;
	servant->vepv->_base_epv->_private = epv1_private;
	
	
	/* Initialise the typecode */
	epv2_private->type = g_new(struct CORBA_TypeCode_struct,1);
	servant->vepv->Counter_epv->_private = epv2_private;
}


void
POA_Counter__fini(POA_Counter* servant, CORBA_Environment* env)
{
	ORBit_EpvPrivate* epv2_private;
	epv2_private = servant->vepv->Counter_epv->_private;

	if(epv2_private->type != NULL)
		{
			g_free(epv2_private->type);
			epv2_private->type=NULL;
		}
}


/* Our application's implementation */

typedef struct AppServant { 
	POA_Counter base; 
	CORBA_long value; 
} AppServant;

CORBA_long app_servant_add(PortableServer_Servant _servant, 
			   CORBA_long val, 
			   CORBA_Environment* _env) 
{
	AppServant* self = (AppServant*)_servant; 
	self->value += val; 
	return self->value; 
}

PortableServer_ServantBase__epv base_epv = { 
	NULL, /* ignore ORB private data */ 
	NULL, /* no servant-specific finalize function needed */ 
	NULL, /* use  base default_POA function */ 
}; 

POA_Counter__epv counter_epv = { 
	NULL, /* ignore ORB private data */ 
	app_servant_add /* point to our add function */ 
}; 

/* Vector of EPVs */ 
POA_Counter__vepv counter_vepv = { &base_epv, &counter_epv };




int main(int argc, char **argv)
{
	CORBA_ORB orb;
	CORBA_ORB_ObjectIdList *ref_list;
	CORBA_Environment ev;
	CORBA_Object obj, dummy;
	PortableServer_POA root_poa;
	int i;

	orb=CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);
	ref_list=CORBA_ORB_list_initial_services(orb, &ev);

	for(i=0;i<ref_list->_length; i++) {
		printf("Ref: %s\n", ref_list->_buffer[i]);
	}

	if(CORBA_sequence_get_release(ref_list->_buffer) == CORBA_TRUE)
	  {
		CORBA_free(ref_list->_buffer);
	  }

	CORBA_free(ref_list);


	/*Get a ref to the root poa*/
	obj = CORBA_ORB_resolve_initial_references(orb,"RootPOA",&ev);
	/* Is there a better way of doing this cast? (e.g. like _narrow in C++) */
	root_poa = (PortableServer_POA) obj;

	if(ev._major != CORBA_NO_EXCEPTION){
		printf("Failed to get root poa - exception thrown with id %s\n",CORBA_exception_id(&ev));
		CORBA_exception_free(&ev);
		exit(-1);
	}

	{
		CORBA_PolicyList policies;
		PortableServer_POAManager poa_mgr=NULL;
		PortableServer_POA my_poa=NULL;
		

		policies._maximum=0;
		policies._length=0;
		policies._buffer=NULL; /* default policies */
		CORBA_sequence_set_release(&policies,CORBA_TRUE); 
		
		poa_mgr = PortableServer_POA__get_the_POAManager(root_poa,&ev);

		my_poa = PortableServer_POA_create_POA(root_poa,
						       "Phils_poa",
						       poa_mgr, 
						       &policies,
						       &ev);
	  
	}

	/* Test the exception throwing mechanism */
	dummy = CORBA_ORB_resolve_initial_references(orb,"dummy",&ev);

	if(ev._major != CORBA_NO_EXCEPTION){
		printf("Failed to get dummy - exception thrown with id %s\n",CORBA_exception_id(&ev));
		CORBA_exception_free(&ev);
		exit(-1);
	}


	return(0);
}


