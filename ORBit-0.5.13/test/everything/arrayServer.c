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

test_LongArray_slice *
ArrayServer_opLongArray(PortableServer_Servant _servant,
						const test_LongArray inArg,
						test_LongArray inoutArg,
						test_LongArray outArg,
						CORBA_Environment * ev) {
  int i;
  test_LongArray_slice *retn;
  for(i=0;i<constants_SEQLEN;i++)
	g_assert(inArg[i]==constants_SEQ_LONG_IN[i]);
  for(i=0;i<constants_SEQLEN;i++)
	g_assert(inoutArg[i]==constants_SEQ_LONG_INOUT_IN[i]);

  for(i=0;i<constants_SEQLEN;i++)
	inoutArg[i] = constants_SEQ_LONG_INOUT_OUT[i];
  
  for(i=0;i<constants_SEQLEN;i++)
	outArg[i] = constants_SEQ_LONG_OUT[i];
	
  retn = test_LongArray__alloc();

  for(i=0;i<constants_SEQLEN;i++)
	retn[i] = constants_SEQ_LONG_RETN[i];
      
  return retn; 
}


static
test_StrArray_slice *
ArrayServer_opStrArray(PortableServer_Servant _servant,
					   const test_StrArray inArg,
					   test_StrArray inoutArg,
					   test_StrArray_slice ** outArg,
					   CORBA_Environment * ev){
  int i;
  test_StrArray_slice *retn;
  for(i=0;i<constants_SEQLEN;i++)
	g_assert(strcmp(inArg[i],constants_SEQ_STRING_IN[i])==0);

  for(i=0;i<constants_SEQLEN;i++)
	g_assert(strcmp(inoutArg[i],constants_SEQ_STRING_INOUT_IN[i])==0);

  for(i=0;i<constants_SEQLEN;i++){
	CORBA_free(inoutArg[i]);
	inoutArg[i] = CORBA_string_dup(constants_SEQ_STRING_INOUT_OUT[i]);
  }

  *outArg = test_StrArray__alloc();
  for(i=0;i<constants_SEQLEN;i++)
	(*outArg)[i] = CORBA_string_dup(constants_SEQ_STRING_OUT[i]);
  
  retn = test_StrArray__alloc();
  for(i=0;i<constants_SEQLEN;i++)
	retn[i] = CORBA_string_dup(constants_SEQ_STRING_RETN[i]);
  
  return retn;  
}


PortableServer_ServantBase__epv ArrayServer_base_epv = {NULL,NULL,NULL};

POA_test_ArrayServer__epv ArrayServer_epv = {
  NULL,
  ArrayServer_opLongArray,
  ArrayServer_opStrArray,
};

POA_test_ArrayServer__vepv ArrayServer_vepv = {&ArrayServer_base_epv,&ArrayServer_epv};

POA_test_ArrayServer ArrayServer_servant = {NULL,&ArrayServer_vepv};  /* Singleton */
