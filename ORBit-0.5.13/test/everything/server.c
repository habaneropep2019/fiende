/*
 * CORBA C language mapping tests
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Phil Dawes <philipd@users.sourceforge.net>
 */

#include "everything.h"
#include <stdio.h>


/* Singleton accessor for the test factory */
test_TestFactory getFactoryInstance(CORBA_Environment *ev);


#include "basicServer.c"
#include "structServer.c"
#include "sequenceServer.c"
#include "unionServer.c"
#include "arrayServer.c"
#include "anyServer.c"


/* Servant class */

typedef struct {
  POA_test_TestFactory baseServant;
  test_BasicServer basicServerRef;
  test_StructServer structServerRef;
  test_SequenceServer sequenceServerRef;
  test_UnionServer unionServerRef;
  test_ArrayServer arrayServerRef;
  test_AnyServer anyServerRef;
} test_TestFactory_Servant;


/* Methods */

static
test_BasicServer
TestFactory_getBasicServer(PortableServer_Servant servant,
						   CORBA_Environment *ev) {
  test_TestFactory_Servant *this = (test_TestFactory_Servant*)servant;
  return CORBA_Object_duplicate(this->basicServerRef,ev);
}

static
test_StructServer
TestFactory_getStructServer(PortableServer_Servant servant,
						   CORBA_Environment *ev) {
  test_TestFactory_Servant *this = (test_TestFactory_Servant*)servant;
  return CORBA_Object_duplicate(this->structServerRef,ev);
}

static
test_SequenceServer
TestFactory_getSequenceServer(PortableServer_Servant servant,
						   CORBA_Environment *ev) {
  test_TestFactory_Servant *this = (test_TestFactory_Servant*)servant;
  return CORBA_Object_duplicate(this->sequenceServerRef,ev);
}

static
test_UnionServer
TestFactory_getUnionServer(PortableServer_Servant servant,
						   CORBA_Environment *ev) {
  test_TestFactory_Servant *this = (test_TestFactory_Servant*)servant;
  return CORBA_Object_duplicate(this->unionServerRef,ev);
}

static
test_ArrayServer
TestFactory_getArrayServer(PortableServer_Servant servant,
						   CORBA_Environment *ev) {
  test_TestFactory_Servant *this = (test_TestFactory_Servant*)servant;
  return CORBA_Object_duplicate(this->arrayServerRef,ev);
}

static
test_AnyServer
TestFactory_getAnyServer(PortableServer_Servant servant,
						   CORBA_Environment *ev) {
  test_TestFactory_Servant *this = (test_TestFactory_Servant*)servant;
  return CORBA_Object_duplicate(this->anyServerRef,ev);
}


static
void test_TestFactory__fini(PortableServer_Servant servant, CORBA_Environment *ev) {
  test_TestFactory_Servant *this = (test_TestFactory_Servant*)servant;
  CORBA_Object_release(this->basicServerRef,ev);
  CORBA_Object_release(this->structServerRef,ev);
  CORBA_Object_release(this->sequenceServerRef,ev);
  CORBA_Object_release(this->unionServerRef,ev);
  CORBA_Object_release(this->arrayServerRef,ev);
  CORBA_Object_release(this->anyServerRef,ev);
}


/* vtable */
PortableServer_ServantBase__epv TestFactory_base_epv = {NULL,test_TestFactory__fini,NULL};

POA_test_TestFactory__epv TestFactory_epv = {
  NULL,
  TestFactory_getBasicServer,
  TestFactory_getStructServer,
  TestFactory_getSequenceServer,
  TestFactory_getUnionServer,
  TestFactory_getArrayServer,
  TestFactory_getAnyServer,
};

POA_test_TestFactory__vepv TestFactory_vepv = {&TestFactory_base_epv,&TestFactory_epv};


/* constructor */
void test_TestFactory__init(PortableServer_Servant servant, 
							PortableServer_POA poa, 
							CORBA_Environment *ev) {
  PortableServer_ObjectId *oid;
  test_TestFactory_Servant *this = (test_TestFactory_Servant*)servant;  
  this->baseServant._private = NULL;
  this->baseServant.vepv = &TestFactory_vepv;

  POA_test_TestFactory__init((PortableServer_ServantBase*)servant,ev);
  
  POA_test_BasicServer__init(&BasicServer_servant,ev);
  this->basicServerRef = PortableServer_POA_servant_to_reference(poa,&BasicServer_servant,ev);  

  POA_test_StructServer__init(&StructServer_servant,ev);
  this->structServerRef = PortableServer_POA_servant_to_reference(poa,&StructServer_servant,ev);  

  POA_test_SequenceServer__init(&SequenceServer_servant,ev);
  this->sequenceServerRef = PortableServer_POA_servant_to_reference(poa,&SequenceServer_servant,ev);  

  POA_test_UnionServer__init(&UnionServer_servant,ev);
  this->unionServerRef = PortableServer_POA_servant_to_reference(poa,&UnionServer_servant,ev);  

  POA_test_ArrayServer__init(&ArrayServer_servant,ev);
  this->arrayServerRef = PortableServer_POA_servant_to_reference(poa,&ArrayServer_servant,ev);  

  POA_test_AnyServer__init(&AnyServer_servant,ev);
  this->anyServerRef = PortableServer_POA_servant_to_reference(poa,&AnyServer_servant,ev);  
}


static test_TestFactory factory;

test_TestFactory getFactoryInstance(CORBA_Environment *ev) {
  return CORBA_Object_duplicate(factory,ev);
}


int
main(int argc, char *argv[]){
    PortableServer_POA poa;
    CORBA_Environment ev;
    CORBA_ORB orb;
	test_TestFactory_Servant servant;

    CORBA_exception_init(&ev);
    orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);

    poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
    PortableServer_POAManager_activate(PortableServer_POA__get_the_POAManager(poa, &ev), &ev);

	test_TestFactory__init(&servant,poa,&ev);
	factory = PortableServer_POA_servant_to_reference(poa,&servant,&ev);

	{
	  FILE *outfile;
	  CORBA_char *ior;

	  POA_test_TestFactory__init(&servant,&ev);

	  outfile = fopen("iorfile","wb");
	  ior = CORBA_ORB_object_to_string(orb, factory, &ev);
	  fwrite(ior,strlen(ior),1,outfile);
	  fclose(outfile);
	  CORBA_free(ior);
	}

    CORBA_ORB_run(orb, &ev);
	return 0;
}
