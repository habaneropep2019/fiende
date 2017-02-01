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

#include <glib.h>
#include <stdio.h>
#include "everything.h"
#include "constants.h"

#undef TIMING_RUN

#ifndef TIMING_RUN
#define d_print(a) g_print(a)
#else
#define d_print(a)
#endif

void testConst() {
  d_print("Testing constants...\n");
  g_assert(test_CONST_CHAR=='t');
  g_assert(test_CONST_LONG==0x12345678);
  g_assert(test_CONST_LONGLONG==0x12345678);
  g_assert(strcmp(test_CONST_STRING,"ConstString")==0);
  /* I can never get constant floats to compare properly. */
  /* g_assert(test_CONST_FLOAT==1234.56); */
  g_assert(test_CONST_DOUBLE==1234.5678);
  g_assert(test_FAVORITE_COLOUR==test_red);

}


void testAttribute(test_TestFactory factory, 
				CORBA_Environment *ev) {  
  test_BasicServer objref;
  CORBA_char *val;
  CORBA_long lval;
  d_print("Testing attributes...\n");
  objref = test_TestFactory_getBasicServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  val = test_BasicServer__get_foo(objref,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);  
  g_assert(strcmp(val,constants_STRING_RETN)==0);	
  CORBA_free(val);
  test_BasicServer__set_foo(objref,constants_STRING_IN,ev);

  lval = test_BasicServer__get_bah(objref,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);  
  g_assert(lval == constants_LONG_RETN);

  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testString(test_TestFactory factory, 
				CORBA_Environment *ev) {
  test_BasicServer objref;
  const CORBA_char *in;
  CORBA_char *inout, *out, *retn; 
  d_print("Testing strings...\n");
  objref = test_TestFactory_getBasicServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  in = constants_STRING_IN;
  inout = CORBA_string_dup(constants_STRING_INOUT_IN);
  retn = test_BasicServer_opString(objref,in,&inout,&out,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  
  g_assert(strcmp(in,constants_STRING_IN)==0);
  g_assert(strcmp(inout,constants_STRING_INOUT_OUT)==0);
  g_assert(strcmp(out,constants_STRING_OUT)==0);
  g_assert(strcmp(retn,constants_STRING_RETN)==0);	
  
  CORBA_free(inout);
  CORBA_free(out);
  CORBA_free(retn);
  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}


void testLong(test_TestFactory factory, 
			  CORBA_Environment *ev) {
  test_BasicServer objref;
  CORBA_long inArg,inoutArg,outArg,retn;
  d_print("Testing longs...\n");
  objref = test_TestFactory_getBasicServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  inArg = constants_LONG_IN;
  inoutArg = constants_LONG_INOUT_IN;
  retn = test_BasicServer_opLong(objref,inArg,&inoutArg,&outArg,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  g_assert(inArg == constants_LONG_IN);
  g_assert(inoutArg == constants_LONG_INOUT_OUT);
  g_assert(outArg == constants_LONG_OUT);
  g_assert(retn == constants_LONG_RETN);
  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testEnum(test_TestFactory factory, 
			  CORBA_Environment *ev) {
  test_BasicServer objref;
  test_AnEnum inArg,inoutArg,outArg,retn;
  d_print("Testing enums...\n");
  objref = test_TestFactory_getBasicServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  inArg = test_ENUM_IN;
  inoutArg = test_ENUM_INOUT_IN;
  retn = test_BasicServer_opEnum(objref,inArg,&inoutArg,&outArg,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  g_assert(inArg == test_ENUM_IN);
  g_assert(inoutArg == test_ENUM_INOUT_OUT);
  g_assert(outArg == test_ENUM_OUT);
  g_assert(retn == test_ENUM_RETN);
  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testException(test_TestFactory factory, 
				   CORBA_Environment *ev) {
  test_BasicServer objref;
  test_TestException *ex;
  d_print("Testing exceptions...\n");
  objref = test_TestFactory_getBasicServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  test_BasicServer_opException(objref,ev);
  
  g_assert(ev->_major == CORBA_USER_EXCEPTION);
  g_assert(strcmp(CORBA_exception_id(ev),ex_test_TestException) == 0);
  ex = CORBA_exception_value(ev);
  g_assert(strcmp(ex->reason,constants_STRING_IN) == 0);
  g_assert(ex->number == constants_LONG_IN);
  g_assert(ex->aseq._length == 1);
  g_assert(ex->aseq._buffer[0] == constants_LONG_IN);

  CORBA_exception_free(ev);
  CORBA_Object_release(objref, ev);  
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testFixedLengthStruct(test_TestFactory factory, 
						   CORBA_Environment *ev) {
  test_StructServer objref;
  test_FixedLengthStruct inArg,inoutArg,outArg,retn;
  d_print("Testing fixed length structs...\n");
  objref = test_TestFactory_getStructServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  inArg.a = constants_SHORT_IN;
  inoutArg.a = constants_SHORT_INOUT_IN;
  
  retn = test_StructServer_opFixed(objref,&inArg,&inoutArg,&outArg,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  
  g_assert(inArg.a == constants_SHORT_IN);
  g_assert(inoutArg.a == constants_SHORT_INOUT_OUT);
  g_assert(outArg.a == constants_SHORT_OUT);
  g_assert(retn.a == constants_SHORT_RETN);
  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testVariableLengthStruct(test_TestFactory factory, 
						   CORBA_Environment *ev) {
  test_StructServer objref;
  test_VariableLengthStruct inArg,inoutArg, *outArg, *retn;
  d_print("Testing variable length structs...\n");
  objref = test_TestFactory_getStructServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  inArg.a = (CORBA_char*)constants_STRING_IN;  /* const cast */
  inoutArg.a = CORBA_string_dup(constants_STRING_INOUT_IN);
  
  retn = test_StructServer_opVariable(objref,&inArg,&inoutArg,&outArg,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  
  g_assert(strcmp(inArg.a,constants_STRING_IN)==0);
  g_assert(strcmp(inoutArg.a,constants_STRING_INOUT_OUT)==0);
  g_assert(strcmp(outArg->a,constants_STRING_OUT)==0);
  g_assert(strcmp(retn->a,constants_STRING_RETN)==0);	

  CORBA_free(inoutArg.a);
  CORBA_free(outArg);
  CORBA_free(retn);
  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}


void testCompoundStruct(test_TestFactory factory, 
						   CORBA_Environment *ev) {
  test_StructServer objref;
  test_CompoundStruct inArg,inoutArg, *outArg, *retn;
  d_print("Testing compound structs...\n");
  objref = test_TestFactory_getStructServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  inArg.a.a = CORBA_string_dup(constants_STRING_IN);
  inoutArg.a.a = CORBA_string_dup(constants_STRING_INOUT_IN);
  
  retn = test_StructServer_opCompound(objref,&inArg,&inoutArg,&outArg,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  
  g_assert(strcmp(inArg.a.a,constants_STRING_IN)==0);
  g_assert(strcmp(inoutArg.a.a,constants_STRING_INOUT_OUT)==0);
  g_assert(strcmp(outArg->a.a,constants_STRING_OUT)==0);
  g_assert(strcmp(retn->a.a,constants_STRING_RETN)==0);	
  
  CORBA_free(inArg.a.a);
  CORBA_free(inoutArg.a.a);
  CORBA_free(outArg);
  CORBA_free(retn);  
  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testUnboundedSequence(test_TestFactory factory, 
						   CORBA_Environment *ev) {
  test_SequenceServer objref;
  test_StrSeq inArg, inoutArg, *outArg, *retn;
  guint i;
  d_print("Testing unbounded sequences...\n");
  objref = test_TestFactory_getSequenceServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  
  inArg._buffer = CORBA_sequence_CORBA_string_allocbuf(2);
  inArg._length = 2;

  for (i=0;i<inArg._length;i++){
	inArg._buffer[i] = CORBA_string_dup(constants_SEQ_STRING_IN[i]);
  }

  inoutArg._buffer = CORBA_sequence_CORBA_string_allocbuf(2);
  inoutArg._length = 2;

  for (i=0;i<inoutArg._length;i++){
	inoutArg._buffer[i] = CORBA_string_dup(constants_SEQ_STRING_INOUT_IN[i]);
  }

  retn = test_SequenceServer_opStrSeq(objref,&inArg,&inoutArg,&outArg,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  
  for (i=0;i<inArg._length;i++){
	g_assert(strcmp(inArg._buffer[i],constants_SEQ_STRING_IN[i]) == 0);
  }

  for (i=0;i<inoutArg._length;i++){
	g_assert(strcmp(inoutArg._buffer[i],constants_SEQ_STRING_INOUT_OUT[i]) == 0);
  }

  for (i=0;i<outArg->_length;i++){
	g_assert(strcmp(outArg->_buffer[i],constants_SEQ_STRING_OUT[i]) == 0);
  }

  for (i=0;i<retn->_length;i++){
	g_assert(strcmp(retn->_buffer[i],constants_SEQ_STRING_RETN[i]) == 0);
  }

  CORBA_free(inArg._buffer);
  CORBA_free(inoutArg._buffer);
  CORBA_free(outArg);
  CORBA_free(retn);
  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testBoundedSequence(test_TestFactory factory, 
						   CORBA_Environment *ev) {
  test_SequenceServer objref;
  test_BoundedStructSeq inArg, inoutArg, *outArg, *retn;
  guint i;
  d_print("Testing bounded sequences...\n");
  objref = test_TestFactory_getSequenceServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  
  inArg._buffer = CORBA_sequence_test_CompoundStruct_allocbuf(2);
  inArg._length = 2;
  inArg._maximum = 2;

  for (i=0;i<inArg._length;i++){
	inArg._buffer[i].a.a = CORBA_string_dup(constants_SEQ_STRING_IN[i]);
  }

  inoutArg._buffer = CORBA_sequence_test_CompoundStruct_allocbuf(2);
  inoutArg._length = 2;
  inoutArg._maximum = 2;

  for (i=0;i<inoutArg._length;i++){
	inoutArg._buffer[i].a.a = CORBA_string_dup(constants_SEQ_STRING_INOUT_IN[i]);
  }

  retn = test_SequenceServer_opBoundedStructSeq(objref,&inArg,&inoutArg,&outArg,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
  

  for (i=0;i<inArg._length;i++){
	g_assert(strcmp(inArg._buffer[i].a.a,constants_SEQ_STRING_IN[i]) == 0);
  }

  for (i=0;i<inoutArg._length;i++){
	g_assert(strcmp(inoutArg._buffer[i].a.a,constants_SEQ_STRING_INOUT_OUT[i]) == 0);
  }

  for (i=0;i<outArg->_length;i++){
	g_assert(strcmp(outArg->_buffer[i].a.a,constants_SEQ_STRING_OUT[i]) == 0);
  }

  for (i=0;i<retn->_length;i++){
	g_assert(strcmp(retn->_buffer[i].a.a,constants_SEQ_STRING_RETN[i]) == 0);
  }

  CORBA_free(inArg._buffer);
  CORBA_free(inoutArg._buffer);
  CORBA_free(outArg);
  CORBA_free(retn);
  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testFixedLengthUnion(test_TestFactory factory, 
						  CORBA_Environment *ev) {
  test_UnionServer objref;
  test_FixedLengthUnion inArg, inoutArg, outArg, retn;
  d_print("Testing fixed length unions...\n");
  objref = test_TestFactory_getUnionServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  inArg._u.x = constants_LONG_IN;
  inArg._d = 'a';

  inoutArg._u.y = 't';
  inoutArg._d = 'b';
  
  retn = test_UnionServer_opFixed(objref,&inArg,&inoutArg,&outArg,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  g_assert(inArg._d == 'a');
  g_assert(inArg._u.x == constants_LONG_IN);
  g_assert(inoutArg._d == 'c');
  g_assert(inoutArg._u.z == TRUE);
  g_assert(outArg._d == 'a');
  g_assert(outArg._u.x == constants_LONG_OUT);
  g_assert(retn._d == 'd');
  g_assert(retn._u.z == FALSE);


  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testVariableLengthUnion(test_TestFactory factory, 
						  CORBA_Environment *ev) {
  test_UnionServer objref;
  test_VariableLengthUnion inArg, inoutArg, *outArg, *retn;
  d_print("Testing variable length unions...\n");
  objref = test_TestFactory_getUnionServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  inArg._u.x = constants_LONG_IN;
  inArg._d = 1;

  inoutArg._u.y = CORBA_string_dup(constants_STRING_INOUT_IN);
  inoutArg._d = 2;
  
  retn = test_UnionServer_opVariable(objref,&inArg,&inoutArg,&outArg,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  g_assert(inArg._d == 1);
  g_assert(inArg._u.x == constants_LONG_IN);
  g_assert(inoutArg._d == 3);
  g_assert(inoutArg._u.z == TRUE);
  g_assert(outArg->_d == 1);
  g_assert(outArg->_u.x == constants_LONG_OUT);
  g_assert(retn->_d == 4);
  g_assert(retn->_u.z == FALSE);

  CORBA_free(outArg);
  CORBA_free(retn);

  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testFixedLengthArray(test_TestFactory factory, 
							 CORBA_Environment *ev) {
  
  test_ArrayServer objref;
  test_LongArray inArg, inoutArg, outArg;
  test_LongArray_slice *retn;
  int i;
  d_print("Testing arrays with fixed length members...\n");
  objref = test_TestFactory_getArrayServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  for(i=0;i<constants_SEQLEN;i++)
	inArg[i] = constants_SEQ_LONG_IN[i];

  for(i=0;i<constants_SEQLEN;i++)
	inoutArg[i] = constants_SEQ_LONG_INOUT_IN[i];

  retn = test_ArrayServer_opLongArray(objref,inArg,inoutArg,outArg,ev);

  for(i=0;i<constants_SEQLEN;i++)
	g_assert(inArg[i]==constants_SEQ_LONG_IN[i]);
  for(i=0;i<constants_SEQLEN;i++)
	g_assert(inoutArg[i]==constants_SEQ_LONG_INOUT_OUT[i]);
  for(i=0;i<constants_SEQLEN;i++)
	g_assert(outArg[i]==constants_SEQ_LONG_OUT[i]);
  for(i=0;i<constants_SEQLEN;i++)
	g_assert(retn[i]==constants_SEQ_LONG_RETN[i]);

  CORBA_free(retn);
  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testVariableLengthArray(test_TestFactory factory, 
							 CORBA_Environment *ev) {
  test_ArrayServer objref;
  test_StrArray inArg, inoutArg;
  test_StrArray_slice *outArg, *retn;
  test_StrArrayMultiDimensional_slice *multidim;
  int i,n0,n1,n2;
  
  d_print("Testing arrays with variable length members...\n");
  objref = test_TestFactory_getArrayServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  for(i=0;i<constants_SEQLEN;i++)
	inArg[i] = CORBA_string_dup(constants_SEQ_STRING_IN[i]);

  for(i=0;i<constants_SEQLEN;i++)
	inoutArg[i] = CORBA_string_dup(constants_SEQ_STRING_INOUT_IN[i]);

  retn = test_ArrayServer_opStrArray(objref,inArg,inoutArg,&outArg,ev);

  for(i=0;i<constants_SEQLEN;i++)
	CORBA_free (inArg[i]); 

  for(i=0;i<constants_SEQLEN;i++)
        CORBA_free (inoutArg[i]);

  CORBA_free(outArg);
  CORBA_free(retn);
  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  
  multidim = test_StrArrayMultiDimensional__alloc();
  for (n0 = 0; n0 < 2; n0++) {
	for (n1 = 0; n1 < 3; n1++) {
	  for (n2 = 0; n2 < 5; n2++) {
		multidim[n0][n1][n2] = CORBA_string_dup(constants_SEQ_STRING_INOUT_IN[0]);
	  }
	}
  }
  CORBA_free(multidim);

}


void testAnyLong(test_TestFactory factory, 
				 CORBA_Environment *ev) {
  test_AnyServer objref;
  CORBA_any inArg, inoutArg, *outArg, *retn;
  CORBA_long tmp, tmp1;

  d_print("Testing any with longs...\n");
  objref = test_TestFactory_getAnyServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  tmp = constants_LONG_IN;
  inArg._type = (CORBA_TypeCode)TC_long;
  inArg._value = &tmp;
  CORBA_any_set_release(&inArg, CORBA_FALSE);

  inoutArg._type = (CORBA_TypeCode)TC_long;
  tmp1 = constants_LONG_INOUT_IN;
  inoutArg._value = &tmp1;
  CORBA_any_set_release(&inoutArg, CORBA_FALSE);
    
  retn = test_AnyServer_opAnyLong(objref,&inArg,&inoutArg,&outArg,ev);

  g_assert( CORBA_TypeCode_equal(inArg._type,TC_long,ev) );
  g_assert(*(CORBA_long*)inArg._value == constants_LONG_IN);

  g_assert( CORBA_TypeCode_equal(inoutArg._type,TC_long,ev) );
  g_assert(*(CORBA_long*)inoutArg._value == constants_LONG_INOUT_OUT);

  g_assert( CORBA_TypeCode_equal(outArg->_type,TC_long,ev) );
  g_assert(*(CORBA_long*)outArg->_value == constants_LONG_OUT);

  g_assert( CORBA_TypeCode_equal(retn->_type,TC_long,ev) );
  g_assert(*(CORBA_long*)retn->_value == constants_LONG_RETN);

  if(CORBA_any_get_release(&inArg)){
	CORBA_free(inArg._value);
	CORBA_Object_release((CORBA_Object)inArg._type, ev);
  }

  if(CORBA_any_get_release(&inoutArg)){
	CORBA_free(inoutArg._value);
	CORBA_Object_release((CORBA_Object)inoutArg._type, ev);
  }

  CORBA_free(outArg);
  CORBA_free(retn);

  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void testAnyString(test_TestFactory factory, 
				   CORBA_Environment *ev) {
  test_AnyServer objref;
  CORBA_any inArg, inoutArg, *outArg, *retn;

  d_print("Testing any with strings...\n");
  objref = test_TestFactory_getAnyServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  inArg._type = (CORBA_TypeCode)TC_string;
  inArg._value = &constants_STRING_IN;
  CORBA_any_set_release(&inArg, CORBA_FALSE);

  inoutArg._type = (CORBA_TypeCode)TC_string;
  inoutArg._value = &constants_STRING_INOUT_IN;
  CORBA_any_set_release(&inoutArg, CORBA_FALSE);
  
  retn = test_AnyServer_opAnyString(objref,&inArg,&inoutArg,&outArg,ev);

  g_assert(CORBA_TypeCode_equal(inArg._type,TC_string,ev));
  g_assert(strcmp(*(CORBA_char **)inArg._value,constants_STRING_IN) == 0);

  g_assert(CORBA_TypeCode_equal(inoutArg._type,TC_string,ev) );
  g_assert(strcmp(*(CORBA_char **)inoutArg._value,constants_STRING_INOUT_OUT) == 0);

  g_assert(CORBA_TypeCode_equal(outArg->_type,TC_string,ev) );
  g_assert(strcmp(*(CORBA_char **)outArg->_value,constants_STRING_OUT) == 0);

  g_assert(CORBA_TypeCode_equal(retn->_type,TC_string,ev) );
  g_assert(strcmp(*(CORBA_char **)retn->_value,constants_STRING_RETN) == 0);

  if(CORBA_any_get_release(&inArg)){
	CORBA_free(inArg._value);
	CORBA_Object_release((CORBA_Object)inArg._type, ev);
  }

  if(CORBA_any_get_release(&inoutArg)){
	CORBA_free(inoutArg._value);
	CORBA_Object_release((CORBA_Object)inoutArg._type, ev);
  }

  CORBA_free(outArg);
  CORBA_free(retn);

  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

void
testAnyStrSeq (test_TestFactory   factory, 
	       CORBA_Environment *ev)
{
	test_AnyServer objref;
	CORBA_any     *retn;

	d_print ("Testing any with string sequences ...\n");

	objref = test_TestFactory_getAnyServer(factory,ev);
	g_assert(ev->_major == CORBA_NO_EXCEPTION);

	retn = test_AnyServer_opAnyStrSeq (objref, ev);
	g_assert(ev->_major == CORBA_NO_EXCEPTION);

	CORBA_free (retn);

	CORBA_Object_release (objref, ev);
}

void testAnyStruct(test_TestFactory factory, 
				   CORBA_Environment *ev) {
  test_AnyServer objref;
  CORBA_any inArg, inoutArg, *outArg, *retn;
  test_VariableLengthStruct inArgStruct; 
  test_VariableLengthStruct * inoutArgStruct;

  d_print("Testing any with structs...\n");

  objref = test_TestFactory_getAnyServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  inoutArgStruct = test_VariableLengthStruct__alloc();
  inArgStruct.a=(CORBA_char*)constants_STRING_IN; /* const cast */
  inArg._type = (CORBA_TypeCode)TC_test_VariableLengthStruct;
  inArg._value = &inArgStruct;
  CORBA_any_set_release(&inArg, CORBA_FALSE);


  inoutArgStruct->a = CORBA_string_dup(constants_STRING_INOUT_IN);
  inoutArg._type = (CORBA_TypeCode)TC_test_VariableLengthStruct;
  inoutArg._value = inoutArgStruct;
  CORBA_any_set_release(&inoutArg, CORBA_TRUE);

  retn = test_AnyServer_opAnyStruct(objref,&inArg,&inoutArg,&outArg,ev);

  
  g_assert(CORBA_TypeCode_equal(inArg._type,TC_test_VariableLengthStruct,ev));
  g_assert(strcmp((*(test_VariableLengthStruct*)inArg._value).a,constants_STRING_IN) == 0);

  g_assert(CORBA_TypeCode_equal(inoutArg._type,TC_test_VariableLengthStruct,ev) );
  g_assert(strcmp((*(test_VariableLengthStruct*)inoutArg._value).a,constants_STRING_INOUT_OUT) == 0);

  g_assert(CORBA_TypeCode_equal(outArg->_type,TC_test_VariableLengthStruct,ev) );
  g_assert(strcmp((*(test_VariableLengthStruct*)outArg->_value).a,constants_STRING_OUT) == 0);

  g_assert(CORBA_TypeCode_equal(retn->_type,TC_test_VariableLengthStruct,ev) );
  g_assert(strcmp((*(test_VariableLengthStruct*)retn->_value).a,constants_STRING_RETN) == 0);


  if(CORBA_any_get_release(&inArg)){
	/* This shouldn't be called */
	CORBA_free(inArg._value);
	CORBA_Object_release((CORBA_Object)inArg._type, ev);
  }

  if(CORBA_any_get_release(&inoutArg)){
	CORBA_free(inoutArg._value);
	CORBA_Object_release((CORBA_Object)inoutArg._type, ev);
  }

  CORBA_free(outArg);
  CORBA_free(retn);

  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

}

void testSequenceOfAny(test_TestFactory factory, 
					   CORBA_Environment *ev) {
  /* this test just checks the memory management for seq of any */

  test_AnySeq anyseq;
  int i;

  d_print("Testing Sequence of Any...\n");

  anyseq._buffer= CORBA_sequence_CORBA_any_allocbuf(2);
  anyseq._length= 2;
  
  for (i=0;i<anyseq._length;i++){
	anyseq._buffer[i]._type = (CORBA_TypeCode)TC_string;
	anyseq._buffer[i]._value = &constants_STRING_IN;
	CORBA_any_set_release(&anyseq._buffer[i], CORBA_FALSE);
  }
  
  CORBA_free(anyseq._buffer);
}

/**
   This test tests memory management of exceptions in anys
 */
void
testAnyException (test_TestFactory   factory, 
		  CORBA_Environment *ev)
{
	CORBA_any          *inArg; 
	test_TestException *testex;

	inArg = CORBA_any__alloc ();
	testex = test_TestException__alloc ();  
	inArg->_type = (CORBA_TypeCode) TC_test_TestException;
	inArg->_value = testex;
	CORBA_any_set_release (inArg, CORBA_TRUE);

	CORBA_free (inArg);
}

void testTypeCode(test_TestFactory factory, 
				  CORBA_Environment *ev) {
  test_AnyServer objref;
  CORBA_TypeCode inArg, inoutArg, outArg, retn;

  d_print("Testing TypeCodes...\n");
  objref = test_TestFactory_getAnyServer(factory,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  inArg = TC_test_ArrayUnion;
  inoutArg = TC_test_AnyServer;
  
  retn = test_AnyServer_opTypeCode(objref,inArg,&inoutArg,&outArg,ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);

  g_assert(CORBA_TypeCode_equal(inArg,TC_test_ArrayUnion,ev) );
  g_assert(CORBA_TypeCode_equal(inoutArg,TC_test_TestException,ev) );  
  g_assert(CORBA_TypeCode_equal(outArg,TC_test_AnEnum,ev) );  
  g_assert(CORBA_TypeCode_equal(retn,TC_test_VariableLengthStruct,ev) );

  CORBA_Object_release((CORBA_Object)inArg, ev);
  CORBA_Object_release((CORBA_Object)inoutArg, ev);
  CORBA_Object_release((CORBA_Object)outArg, ev);
  CORBA_Object_release((CORBA_Object)retn, ev);

  CORBA_Object_release(objref, ev);
  g_assert(ev->_major == CORBA_NO_EXCEPTION);
}

int main(int argc, char *argv[]) {
  CORBA_Environment ev;
  CORBA_ORB orb;
  test_TestFactory factory;
  int i;
  CORBA_exception_init(&ev);
  orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);
  g_assert(ev._major == CORBA_NO_EXCEPTION);


  { /* read the ior from iorfile, and swizzle to an objref*/
	int size;
	char ior[1024];
	FILE *infile = fopen("iorfile","rb");
	size = fread(ior,1,1024,infile);
	fclose(infile);
	ior[size] = '\0';   /* insure that string is terminated correctly */
	factory = CORBA_ORB_string_to_object(orb, ior, &ev);
	g_assert(ev._major == CORBA_NO_EXCEPTION);
  }

#ifdef TIMING_RUN
  for(i=0;i<1000;i++){
#else
  for(i=0;i<1;i++){
#endif
  testConst();
  testAttribute(factory,&ev);  
  testString(factory,&ev);
  testLong(factory,&ev);
  testEnum(factory,&ev);
  testException(factory,&ev);
  testFixedLengthStruct(factory,&ev);
  testVariableLengthStruct(factory,&ev);
  testCompoundStruct(factory,&ev);
  testUnboundedSequence(factory,&ev);
  testBoundedSequence(factory,&ev);
  testFixedLengthUnion(factory,&ev);
  testVariableLengthUnion(factory,&ev);
  testFixedLengthArray(factory,&ev);
  testVariableLengthArray(factory,&ev);
  testAnyLong(factory,&ev);
  testAnyString(factory,&ev);
  testAnyStruct(factory,&ev);
  testAnyStrSeq(factory,&ev);
  testAnyException(factory,&ev);
  testSequenceOfAny(factory,&ev);
  testTypeCode(factory,&ev);
  }
  CORBA_Object_release(factory, &ev);
  CORBA_Object_release((CORBA_Object)orb, &ev);
  g_assert(ev._major == CORBA_NO_EXCEPTION);

#ifdef TIMING_RUN
  g_warning ("Did '%d' iterations", i);
#endif

  return 0;
}
