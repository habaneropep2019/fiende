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
#include "constants.h"
#include <stdio.h>

static  
test_FixedLengthStruct
StructServer_opFixed(PortableServer_Servant _servant,
					 const test_FixedLengthStruct *inArg,
					 test_FixedLengthStruct *inoutArg,
					 test_FixedLengthStruct *outArg,
					 CORBA_Environment * ev){
  test_FixedLengthStruct retval;
  g_assert(inArg->a == constants_SHORT_IN);
  g_assert(inoutArg->a == constants_SHORT_INOUT_IN);
  
  inoutArg->a = constants_SHORT_INOUT_OUT;
  outArg->a = constants_SHORT_OUT;
  retval.a = constants_SHORT_RETN;
  return retval;
}



static  
test_VariableLengthStruct*
StructServer_opVariable(PortableServer_Servant _servant,
					 const test_VariableLengthStruct *inArg,
					 test_VariableLengthStruct *inoutArg,
					 test_VariableLengthStruct **outArg,
					 CORBA_Environment * ev){
  test_VariableLengthStruct *retval;
  g_assert(strcmp(inArg->a,constants_STRING_IN) == 0);
  g_assert(strcmp(inoutArg->a,constants_STRING_INOUT_IN)==0);
  
  *outArg = test_VariableLengthStruct__alloc();
  retval = test_VariableLengthStruct__alloc();
  
  CORBA_free(inoutArg->a);

  inoutArg->a = CORBA_string_dup(constants_STRING_INOUT_OUT);
  (*outArg)->a = CORBA_string_dup(constants_STRING_OUT);
  retval->a = CORBA_string_dup(constants_STRING_RETN);
  
  return retval;
}


static  
test_CompoundStruct*
StructServer_opCompound(PortableServer_Servant _servant,
					 const test_CompoundStruct *inArg,
					 test_CompoundStruct *inoutArg,
					 test_CompoundStruct **outArg,
					 CORBA_Environment * ev){
  test_CompoundStruct *retval;
  g_assert(strcmp(inArg->a.a,constants_STRING_IN) == 0);
  g_assert(strcmp(inoutArg->a.a,constants_STRING_INOUT_IN)==0);
  
  *outArg = test_CompoundStruct__alloc();
  retval = test_CompoundStruct__alloc();
  
  CORBA_free(inoutArg->a.a);

  inoutArg->a.a = CORBA_string_dup(constants_STRING_INOUT_OUT);
  (*outArg)->a.a = CORBA_string_dup(constants_STRING_OUT);
  retval->a.a = CORBA_string_dup(constants_STRING_RETN);
  
  return retval;
}


PortableServer_ServantBase__epv StructServer_base_epv = {NULL,NULL,NULL};

POA_test_StructServer__epv StructServer_epv = {
  NULL,
  StructServer_opFixed,
  StructServer_opVariable,
  StructServer_opCompound,
};

POA_test_StructServer__vepv StructServer_vepv = {&StructServer_base_epv,&StructServer_epv};

POA_test_StructServer StructServer_servant = {NULL,&StructServer_vepv};  /* Singleton */
