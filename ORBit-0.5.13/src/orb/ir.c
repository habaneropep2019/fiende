/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  ORBit: A CORBA v2.2 ORB
 *
 *  Copyright (C) 1998 Richard H. Porter
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
 *
 */

#include <stdlib.h>
#include <assert.h>

#include "orbit.h"

/* FIXME: Right now this function doesn't record whether or not it has
   already visited a given TypeCode.  I'm not sure if every recursive
   type will have a tk_recursive node in it; if not, then this will
   need to be reworked a bit.  */
CORBA_boolean CORBA_TypeCode_equal(CORBA_TypeCode obj, CORBA_TypeCode tc, CORBA_Environment *ev)
{
	int i;

	g_return_val_if_fail(obj!=NULL, CORBA_FALSE);
	g_return_val_if_fail(tc!=NULL, CORBA_FALSE);

	if (obj->kind != tc->kind) {
		return CORBA_FALSE;
	}

	switch (obj->kind) {
	case CORBA_tk_wstring:
	case CORBA_tk_string:
		return obj->length == tc->length;
	case CORBA_tk_objref:
		return ! strcmp (obj->repo_id, tc->repo_id);
	case CORBA_tk_except:
	case CORBA_tk_struct:
		if (strcmp (obj->repo_id, tc->repo_id)
		    || obj->sub_parts != tc->sub_parts)
			return CORBA_FALSE;
		for (i = 0; i < obj->sub_parts; ++i)
			if (! CORBA_TypeCode_equal (obj->subtypes[i],
						    tc->subtypes[i], ev))
				return CORBA_FALSE;
		break;
	case CORBA_tk_union:
		if (strcmp (obj->repo_id, tc->repo_id)
		    || obj->sub_parts != tc->sub_parts
		    || ! CORBA_TypeCode_equal (obj->discriminator,
					       tc->discriminator, ev)
		    || obj->default_index != tc->default_index)
			return CORBA_FALSE;
		for (i = 0; i < obj->sub_parts; ++i)

			if (! CORBA_TypeCode_equal (obj->subtypes[i],
						    tc->subtypes[i], ev)
			    || ! ORBit_any_equivalent (&obj->sublabels[i],
						       &tc->sublabels[i], ev))
				return CORBA_FALSE;

		break;
	case CORBA_tk_enum:
		if (obj->sub_parts != tc->sub_parts
		    || strcmp (obj->repo_id, tc->repo_id))
			return CORBA_FALSE;
		for (i = 0; i < obj->sub_parts; ++i)
			if (strcmp (obj->subnames[i], tc->subnames[i]))
				return CORBA_FALSE;
		break;
	case CORBA_tk_sequence:
	case CORBA_tk_array:
		if (obj->length != tc->length)
			return CORBA_FALSE;
		g_assert (obj->sub_parts == 1);
		g_assert (tc->sub_parts == 1);
		return CORBA_TypeCode_equal (obj->subtypes[0], tc->subtypes[0],
					     ev);
	case CORBA_tk_alias:
		if (strcmp (obj->repo_id, tc->repo_id))
			return CORBA_FALSE;
		
		g_assert (obj->sub_parts == 1);
		g_assert (tc->sub_parts == 1);

		return CORBA_TypeCode_equal (obj->subtypes[0], tc->subtypes[0],
					       ev);
		break;
	case CORBA_tk_recursive:
		return obj->recurse_depth == tc->recurse_depth;
	case CORBA_tk_fixed:
		return obj->digits == tc->digits && obj->scale == tc->scale;

	default:
		/* Everything else is primitive.  */
		break;
	}

	return CORBA_TRUE;
}

CORBA_TCKind CORBA_TypeCode_kind(CORBA_TypeCode obj, CORBA_Environment *ev)
{
	return obj->kind;
}

static void bad_kind (CORBA_Environment *ev)
{
	CORBA_TypeCode_BadKind *err;
	err = g_new (CORBA_TypeCode_BadKind, 1);
	if (err == NULL) {
		CORBA_exception_set_system (ev, ex_CORBA_NO_MEMORY,
					    CORBA_COMPLETED_NO);
	} else {
		err->dummy = 23;
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     "IDL:omg.org/CORBA/TypeCode/BadKind/1.0",
				     err);
	}
}

CORBA_RepositoryId CORBA_TypeCode_id(CORBA_TypeCode obj, CORBA_Environment *ev)
{
	if (! (obj->kind == CORBA_tk_objref || obj->kind == CORBA_tk_struct
	       || obj->kind == CORBA_tk_enum || obj->kind == CORBA_tk_alias
	       || obj->kind == CORBA_tk_except)) {
		bad_kind (ev);
		return NULL;
	}
	return (CORBA_RepositoryId)obj->repo_id;
}

CORBA_Identifier CORBA_TypeCode_name(CORBA_TypeCode obj, CORBA_Environment *ev)
{
	if (! (obj->kind == CORBA_tk_objref || obj->kind == CORBA_tk_struct
	       || obj->kind == CORBA_tk_enum || obj->kind == CORBA_tk_alias
	       || obj->kind == CORBA_tk_except)) {
		bad_kind (ev);
		return NULL;
	}

	return (CORBA_Identifier)obj->name;
}

CORBA_unsigned_long CORBA_TypeCode_member_count(CORBA_TypeCode obj, CORBA_Environment *ev)
{
	if (! (obj->kind == CORBA_tk_struct || obj->kind == CORBA_tk_union
	       || obj->kind == CORBA_tk_enum)) {
		bad_kind (ev);
		return 0;
	}
	return obj->sub_parts;
}

static void bounds_error (CORBA_Environment *ev)
{
	CORBA_TypeCode_Bounds *err;
	err = g_new (CORBA_TypeCode_Bounds, 1);
	if (err == NULL) {
		CORBA_exception_set_system (ev, ex_CORBA_NO_MEMORY,
					    CORBA_COMPLETED_NO);
	} else {
		err->dummy = 23;
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     "IDL:omg.org/CORBA/TypeCode/Bounds/1.0",
				     err);
	}
}

CORBA_Identifier CORBA_TypeCode_member_name(CORBA_TypeCode obj, CORBA_unsigned_long index, CORBA_Environment *ev)
{
	if (! (obj->kind == CORBA_tk_struct || obj->kind == CORBA_tk_union
	       || obj->kind == CORBA_tk_enum)) {
		bad_kind (ev);
		return NULL;
	}
	if (index > obj->sub_parts) {
		bounds_error (ev);
		return NULL;
	}
	return (CORBA_Identifier)obj->subnames[index];
}

CORBA_TypeCode CORBA_TypeCode_member_type(CORBA_TypeCode obj, CORBA_unsigned_long index, CORBA_Environment *ev)
{
	if (! (obj->kind == CORBA_tk_struct || obj->kind == CORBA_tk_union
	       || obj->kind == CORBA_tk_enum)) {
		bad_kind (ev);
		return NULL;
	}
	if (index > obj->sub_parts) {
		bounds_error (ev);
		return NULL;
	}
	return obj->subtypes[index];
}

CORBA_any *CORBA_TypeCode_member_label(CORBA_TypeCode obj, CORBA_unsigned_long index, CORBA_Environment *ev)
{
	if (obj->kind != CORBA_tk_union) {
		bad_kind (ev);
		return NULL;
	}
	if (index > obj->sub_parts) {
		bounds_error (ev);
		return NULL;
	}
	return &obj->sublabels[index];
}

CORBA_TypeCode CORBA_TypeCode_discriminator_type(CORBA_TypeCode obj, CORBA_Environment *ev)
{
	if (obj->kind != CORBA_tk_union) {
		bad_kind (ev);
		return NULL;
	}
	return obj->discriminator;
}

CORBA_long CORBA_TypeCode_default_index(CORBA_TypeCode obj, CORBA_Environment *ev)
{
	if (obj->kind != CORBA_tk_union) {
		bad_kind (ev);
		return 0;
	}
	return obj->default_index;
}

CORBA_unsigned_long CORBA_TypeCode_length(CORBA_TypeCode obj, CORBA_Environment *ev)
{
	if (! (obj->kind == CORBA_tk_string || obj->kind == CORBA_tk_wstring
	       || obj->kind == CORBA_tk_array)) {
		bad_kind (ev);
		return 0;
	}
	return obj->length;
}

CORBA_TypeCode CORBA_TypeCode_content_type(CORBA_TypeCode obj, CORBA_Environment *ev)
{
	if (! (obj->kind == CORBA_tk_sequence || obj->kind == CORBA_tk_array
	       || obj->kind == CORBA_tk_alias)) {
		bad_kind (ev);
		return NULL;
	}
	g_assert (obj->sub_parts == 1);
	return obj->subtypes[0];
}

CORBA_unsigned_short CORBA_TypeCode_fixed_digits(CORBA_TypeCode obj, CORBA_Environment *ev)
{
	if (obj->kind != CORBA_tk_fixed) {
		bad_kind (ev);
		return 0;
	}
	return obj->digits;
}

CORBA_short CORBA_TypeCode_fixed_scale(CORBA_TypeCode obj, CORBA_Environment *ev)
{
	if (obj->kind != CORBA_tk_fixed) {
		bad_kind (ev);
		return 0;
	}
	return obj->scale;
}

CORBA_long CORBA_TypeCode_param_count(CORBA_TypeCode obj, CORBA_Environment *ev)
{
	g_assert(!"Deprecated");
	return(0);
}

CORBA_any *CORBA_TypeCode_parameter(CORBA_TypeCode obj, CORBA_long index, CORBA_Environment *ev)
{
	g_assert(!"Deprecated");
	return(NULL);
}
