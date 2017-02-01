/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  ORBit: A CORBA v2.2 ORB
 *
 *  Copyright (C) 1998 Richard H. Porter, Red Hat Software
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Dick Porter <dick@cymru.net>
 *          Elliot Lee <sopwith@cuc.edu>
 *
 */

#include "orbit.h"
#include "orbit_typecode.h"
 
const struct CORBA_TypeCode_struct TC_null_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_null, "null", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_void_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_void, "void", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_short_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_short, "short", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_long_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_long, "long", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_longlong_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_longlong, "long long", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_ushort_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_ushort, "unsigned short", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_ulong_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_ulong, "unsigned long", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_ulonglong_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_ulonglong, "unsigned long long", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_float_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_float, "float", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_double_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_double, "double", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_longdouble_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_longdouble, "long double", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_boolean_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_boolean, "boolean", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_char_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_char, "char", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_wchar_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_wchar, "wide char", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_octet_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_octet, "octet", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_any_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_any, "any", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_TypeCode_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_TypeCode, "TypeCode", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_Principal_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_Principal, "Principal", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_Object_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_objref, "Object Reference", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_string_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_string, "string", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_wstring_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_wstring, "wide string", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};
const struct CORBA_TypeCode_struct TC_CORBA_NamedValue_struct=
	{{{(ORBit_RootObject_Interface *)&ORBit_TypeCode_epv, CORBA_FALSE, -1}, ORBIT_PSEUDO_TYPECODE}, CORBA_tk_struct, "CORBA NamedValue", "", 0, 0, NULL, NULL, NULL, NULL, -1, 0, 0, 0};

static const CORBA_TypeCode anon_subtypes_array7[] =
{(CORBA_TypeCode) & TC_CORBA_string_struct};

#if (TC_IMPL_TC_CORBA_Identifier_0 == '/')
const struct CORBA_TypeCode_struct TC_CORBA_Identifier_struct =
{
   {
      {(ORBit_RootObject_Interface *) & ORBit_TypeCode_epv, TRUE, -1}, ORBIT_PSEUDO_TYPECODE},
   CORBA_tk_alias, "Identifier", "IDL:omg.org/CORBA/Identifier:1.0",
   0, 1,
   NULL,
   (CORBA_TypeCode *) anon_subtypes_array7,
   NULL,
   CORBA_OBJECT_NIL, 0, -1, 0, 0
};
#endif

#if (TC_IMPL_TC_CORBA_RepositoryId_0 == '/')
const struct CORBA_TypeCode_struct TC_CORBA_RepositoryId_struct =
{
   {
      {(ORBit_RootObject_Interface *) & ORBit_TypeCode_epv, TRUE, -1}, ORBIT_PSEUDO_TYPECODE},
   CORBA_tk_alias, "RepositoryId", "IDL:omg.org/CORBA/RepositoryId:1.0",
   0, 1,
   NULL,
   (CORBA_TypeCode *) anon_subtypes_array7,
   NULL,
   CORBA_OBJECT_NIL, 0, -1, 0, 0
};
#endif
