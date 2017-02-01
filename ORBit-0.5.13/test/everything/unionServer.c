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
test_FixedLengthUnion
UnionServer_opFixed(PortableServer_Servant _servant,
					 const test_FixedLengthUnion *inArg,
					test_FixedLengthUnion *inoutArg,
					 test_FixedLengthUnion *outArg,
					 CORBA_Environment * ev){
  test_FixedLengthUnion retval;
  g_assert(inArg->_d == 'a');
  g_assert(inArg->_u.x == constants_LONG_IN);
  
  g_assert(inoutArg->_d == 'b');
  g_assert(inoutArg->_u.y == 't');

  inoutArg->_u.z = TRUE;
  inoutArg->_d = 'c';

  outArg->_u.x = constants_LONG_OUT;
  outArg->_d = 'a';

  retval._u.z = FALSE;
  retval._d = 'd';
  return retval;
}

static
test_VariableLengthUnion *
UnionServer_opVariable(PortableServer_Servant _servant,
					   const test_VariableLengthUnion * inArg,
					   test_VariableLengthUnion * inoutArg,
					   test_VariableLengthUnion ** outArg,
					   CORBA_Environment * ev){
  test_VariableLengthUnion *retval;
  
  g_assert(inArg->_d == 1);
  g_assert(inArg->_u.x == constants_LONG_IN);
  
  g_assert(inoutArg->_d == 2);
  g_assert(strcmp(inoutArg->_u.y,constants_STRING_INOUT_IN)==0);

  CORBA_free(inoutArg->_u.y);
  inoutArg->_u.z = TRUE;
  inoutArg->_d = 3;
  
  *outArg = test_VariableLengthUnion__alloc();
  (*outArg)->_u.x = constants_LONG_OUT;
  (*outArg)->_d = 1;

  retval = test_VariableLengthUnion__alloc();
  retval->_u.z = FALSE;
  retval->_d = 4;
  return retval;
}



PortableServer_ServantBase__epv UnionServer_base_epv = {NULL,NULL,NULL};

POA_test_UnionServer__epv UnionServer_epv = {
  NULL,
  UnionServer_opFixed,
  UnionServer_opVariable,
};

POA_test_UnionServer__vepv UnionServer_vepv = {&UnionServer_base_epv,&UnionServer_epv};

POA_test_UnionServer UnionServer_servant = {NULL,&UnionServer_vepv};  /* Singleton */
