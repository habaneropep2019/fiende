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
 *          Elliot Lee <sopwith@redhat.com>
 *
 */

#include <assert.h>

#include "config.h"
#include "orbit.h"
#include "sequences.h"

#define o_return_val_if_fail(expr, val) if(!(expr)) { CORBA_exception_set_system (ev, ex_CORBA_BAD_PARAM, CORBA_COMPLETED_NO); return (val); }
#define o_return_if_fail(expr)          if(!(expr)) { CORBA_exception_set_system (ev, ex_CORBA_BAD_PARAM, CORBA_COMPLETED_NO); return; }
#define b_return_val_if_fail(expr, val) if(!(expr)) { CORBA_exception_set_system (ev, ex_CORBA_OBJECT_NOT_EXIST, CORBA_COMPLETED_NO); return (val); }
#define b_return_if_fail(expr)          if(!(expr)) { CORBA_exception_set_system (ev, ex_CORBA_OBJECT_NOT_EXIST, CORBA_COMPLETED_NO); return; }


static void DynamicAny_DynAny_release_fn(DynamicAny_DynAny obj, CORBA_Environment *ev);

static const ORBit_RootObject_Interface DynamicAny_DynAny__epv = {
	(void (*)(gpointer, CORBA_Environment *)) DynamicAny_DynAny_release_fn
};

/*	     CORBA_tk_null:
	case CORBA_tk_void:
	case CORBA_tk_short:
	case CORBA_tk_long:
	case CORBA_tk_ushort:
	case CORBA_tk_ulong:
	case CORBA_tk_float:
	case CORBA_tk_double:
	case CORBA_tk_boolean:
	case CORBA_tk_char:
	case CORBA_tk_octet:
	case CORBA_tk_any:
	case CORBA_tk_TypeCode:
	case CORBA_tk_Principal:
	case CORBA_tk_objref:
 	case CORBA_tk_struct:
	case CORBA_tk_union:
	case CORBA_tk_enum:
	case CORBA_tk_string:
	case CORBA_tk_sequence:
	case CORBA_tk_array:
	case CORBA_tk_alias:
	case CORBA_tk_except:
	case CORBA_tk_longlong:
	case CORBA_tk_ulonglong:
	case CORBA_tk_longdouble:
	case CORBA_tk_wchar:
	case CORBA_tk_wstring:
	case CORBA_tk_fixed:
	case CORBA_tk_recursive: */


#define DYNANY_TYPE_SINGLE \
	     CORBA_tk_null: \
	case CORBA_tk_void: \
	case CORBA_tk_short: \
	case CORBA_tk_long: \
	case CORBA_tk_ushort: \
	case CORBA_tk_ulong: \
	case CORBA_tk_float: \
	case CORBA_tk_double: \
	case CORBA_tk_boolean: \
	case CORBA_tk_char: \
	case CORBA_tk_octet: \
	case CORBA_tk_string: \
	case CORBA_tk_longlong: \
	case CORBA_tk_ulonglong: \
	case CORBA_tk_longdouble: \
	case CORBA_tk_wchar: \
	case CORBA_tk_wstring: \
	case CORBA_tk_objref: \
	case CORBA_tk_any: \
	case CORBA_tk_TypeCode: \
	case CORBA_tk_Principal

#define DYNANY_TYPE_RELEASED \
 	     CORBA_tk_struct: \
	case CORBA_tk_union: \
	case CORBA_tk_sequence: \
	case CORBA_tk_array: \
	case CORBA_tk_alias: \
	case CORBA_tk_objref: \
	case CORBA_tk_any: \
	case CORBA_tk_TypeCode: \
	case CORBA_tk_Principal



typedef struct _DynAny DynAny;

struct _DynAny {
	/*
	 *  This stores the DynAny's typecode, which is
	 * always valid unless OBJECT_NOT_EXIST should
	 * be fired. any->_value points to the value;
	 * the validity of this is enforced.
	 */
	CORBA_any   *any;
	int          idx;

	/*
	 *  These are validated when something bad
	 * happens such as changing a union's discriminator
	 * or shrinking a sequence.
	 */
	GSList      *children;		/* DynAny *'s */
	int          parent_idx;

	/* To allow de-registration */
	DynAny      *parent;
};

/*
 *  All clever validation happens parent -> child
 * children use their parent simply to deregister themselfs;
 */
static void
dynany_invalidate (DynAny *d,
		   gboolean invalidate_cur,
		   CORBA_Environment *ev)
{
	if (invalidate_cur) {
		if (d->parent) {
			d->parent->children = g_slist_remove (
				d->parent->children, d);
			d->parent = NULL;
			g_assert (!d->any->_release);
		}

		if (d->any->_release) 
			CORBA_free (d->any);
		d->any = NULL;
	}

	while (d->children)
		dynany_invalidate (d->children->data, TRUE, ev);
}

#define GET_DYNANY(obj) ((DynAny *)((struct DynamicAny_DynAny_type *)(obj))->data)

static void
DynamicAny_DynAny_release_fn (DynamicAny_DynAny obj, CORBA_Environment *ev)
{
	DynAny *dynany;

	g_return_if_fail (obj != NULL);

	dynany = GET_DYNANY (obj);

	dynany_invalidate (dynany, FALSE, ev);

	if (dynany->any)
		CORBA_free (dynany->any);

	g_slist_free (dynany->children);

	g_free (dynany);
	g_free (obj);
}

static CORBA_TypeCode
dynany_get_cur_type (DynAny *dynany)
{
	CORBA_TypeCode tc = dynany->any->_type;

 find_type:
	switch (tc->kind) {
	case DYNANY_TYPE_SINGLE:
	case CORBA_tk_fixed:
	case CORBA_tk_enum:
		return tc;
		
	case CORBA_tk_sequence:
	case CORBA_tk_array:
		return tc->subtypes [0];
		
	case CORBA_tk_struct:
	case CORBA_tk_except:
		if (dynany->idx >= 0 && dynany->idx < tc->sub_parts)
			return tc->subtypes [dynany->idx];
		else
			return CORBA_OBJECT_NIL;

	case CORBA_tk_union:
		if (dynany->idx == 0)
			return tc->discriminator;
		else
			g_warning ("Union body type checking unimplemented");
		break;

	case CORBA_tk_alias:
		tc = tc->subtypes [0];
		goto find_type;
		
	default:
		g_warning ("Unknown kind '%d'", tc->kind);
		break;
	}

	return CORBA_OBJECT_NIL;
}

static gboolean
dynany_type_mismatch (DynAny        *dynany,
		      CORBA_TypeCode assign,
		      CORBA_Environment *ev)
{
	gboolean       equal;
	CORBA_TypeCode tc = dynany_get_cur_type (dynany);

	if (tc != CORBA_OBJECT_NIL) {
		equal = CORBA_TypeCode_equal (tc, assign, ev);

		if (ev->_major != CORBA_NO_EXCEPTION)
			return TRUE;
	} else
		equal = FALSE;

	if (!equal)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_DynamicAny_DynAny_TypeMismatch, NULL);
	return !equal;
}

static gboolean
dynany_kind_mismatch (DynAny *dynany,
		      CORBA_TCKind kind,
		      CORBA_Environment *ev)
{
	if (dynany->any->_type->kind != kind) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_DynamicAny_DynAny_TypeMismatch, NULL);
		return TRUE;
	}

	return FALSE;
}

/* ORBit_demarshal_allocate_mem cut & paste: should globalize this */
static gpointer
ORBit_demarshal_allocate_mem (CORBA_TypeCode tc, gint nelements)
{
	size_t block_size;
	gpointer retval = NULL;

	if (!nelements)
		return retval;

	block_size = ORBit_gather_alloc_info(tc);

	if (block_size) {
		retval = ORBit_alloc_2 (
			block_size *nelements,
			(ORBit_free_childvals) ORBit_free_via_TypeCode,
			GINT_TO_POINTER (nelements),
			sizeof (CORBA_TypeCode));

		*(CORBA_TypeCode *)((char *) retval - sizeof (ORBit_mem_info) -
				    sizeof (CORBA_TypeCode)) =
			(CORBA_TypeCode) CORBA_Object_duplicate ((CORBA_Object) tc, NULL);
	}

	return retval;
}

static void
dynany_init_default (gpointer *val, const CORBA_TypeCode tc)
{
	int    i;
	size_t size;

	size = ORBit_gather_alloc_info (tc);

	*val = ALIGN_ADDRESS (*val, ORBit_find_alignment (tc));

	switch (tc->kind) {
	case CORBA_tk_null:
		break;
	case CORBA_tk_long:
	case CORBA_tk_ulong:
	case CORBA_tk_enum:
	case CORBA_tk_short:
	case CORBA_tk_ushort:
	case CORBA_tk_wchar:
	case CORBA_tk_boolean:
	case CORBA_tk_char:
	case CORBA_tk_octet:
	case CORBA_tk_longlong:
	case CORBA_tk_ulonglong:
		memset (*val, 0, size);
		*val = ((guchar *)*val) + size;
		break;

	case CORBA_tk_wstring:
	case CORBA_tk_string:
		*(CORBA_char **) *val = CORBA_string_dup ("");
		*val = ((guchar *)*val) + size;
		break;

	case CORBA_tk_float:
		*(CORBA_float *) *val = 0.0;
		*val = ((guchar *)*val) + size;
		break;

	case CORBA_tk_double:
		*(CORBA_double *) *val = 0.0;
		*val = ((guchar *)*val) + size;
		break;

	case CORBA_tk_longdouble:
		*(CORBA_long_double *) *val = 0.0;
		*val = ((guchar *)*val) + size;
		break;

	case CORBA_tk_objref:
		*(CORBA_Object *) *val = CORBA_OBJECT_NIL;
		*val = ((guchar *)*val) + size;
		break;

	case CORBA_tk_any: {
		CORBA_any *any = (CORBA_any *) *val;

		any->_type = (CORBA_TypeCode) CORBA_Object_duplicate (
			(CORBA_Object) TC_null, NULL);
		any->_value = NULL;

		CORBA_any_set_release (any, CORBA_TRUE);
		*val = ((guchar *)*val) + size;
		break;
	}

	case CORBA_tk_TypeCode:
		*(CORBA_Object *) *val = CORBA_Object_duplicate (
			(CORBA_Object) TC_null, NULL);
		*val = ((guchar *)*val) + size;
		break;

 	case CORBA_tk_sequence: {
		CORBA_sequence_octet *s;

		s = (CORBA_sequence_octet *) *val;

		s->_maximum = tc->length;
		s->_length = 0;
		s->_buffer = NULL;
		s->_release = CORBA_TRUE;

		*val = ((guchar *)*val) + sizeof (CORBA_sequence_octet);
		break;
	}

	case CORBA_tk_array:
		for (i = 0; i < tc->length; i++) 
			dynany_init_default (val, tc->subtypes [0]);
		break;
		
	case CORBA_tk_except:
	case CORBA_tk_struct:
		for (i = 0; i < tc->sub_parts; i++)
			dynany_init_default (val, tc->subtypes [i]);
		break;

	case CORBA_tk_union: {
		gpointer oldval = *val;

		dynany_init_default (val, tc->discriminator);

		dynany_init_default (val, tc->subtypes [0]);

		*val = ((guchar *)oldval) + size;
		break;
	}

	case CORBA_tk_alias:
		dynany_init_default (val, tc->subtypes [0]);
		break;
		
	case CORBA_tk_fixed:
	default:
		g_warning ("Unhandled typecode");
		break;
	}
}

static gpointer
dynany_any_new_default (const CORBA_TypeCode tc)
{
	gpointer value;
	gpointer p;

	p = value = ORBit_demarshal_allocate_mem (tc, 1);
	dynany_init_default (&p, tc);

	return value;
}

static gboolean
dynany_sequence_realloc_to (CORBA_sequence_octet *s,
			    CORBA_TypeCode        sequence_tc,
			    long                  len,
			    CORBA_Environment    *ev)
{
	CORBA_TypeCode tc = sequence_tc->subtypes [0];
	gpointer buf, old_buf, a, b;
	CORBA_unsigned_long old_len;

	buf = ORBit_demarshal_allocate_mem (tc, len);

	if (!buf)
		return FALSE;

	old_buf = s->_buffer;
	old_len = s->_length;
	s->_buffer = buf;
	s->_length = len;

	if (old_buf) {
		int i;

		a = old_buf;
		b = buf;
		
		for (i = 0; i < old_len; i++)
			_ORBit_copy_value (&a, &b, tc);
		
		ORBit_free (old_buf, TRUE);
	}

	return TRUE;
}

#define DYNANY_GET_OFFSET(p,i,tc) \
	((gpointer) (((guchar *) (p)) + ORBit_gather_alloc_info (tc) * (i)))

/**
 * dynany_get_value:
 * @dynany: dynany
 * @ev: exception environment
 * 
 *   Calculates the address of the current component's
 * value data; ie. for a _tk_long we return a CORBA_long *
 * 
 * Return value: as above, or NULL on exception.
 **/
static gpointer
dynany_get_value (DynAny *dynany, CORBA_Environment *ev)
{
	gpointer  value;
	CORBA_any *any = dynany->any;
	CORBA_TypeCode tc = any->_type;

 get_from_typecode:
	switch (tc->kind) {
	case DYNANY_TYPE_SINGLE:
	case CORBA_tk_fixed:
	case CORBA_tk_enum:
		value = any->_value;
		break;
	default:
		if (dynany->idx < 0) {
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_DynamicAny_DynAny_InvalidValue, NULL);
			return NULL;
		}

		switch (tc->kind) {
		case CORBA_tk_sequence: {
			CORBA_sequence_octet *s;
			
			s = (CORBA_sequence_octet *) any->_value;
			if (!s || dynany->idx >= s->_length) {
				value = NULL;
				g_warning ("Serious internal sequence related "
					   "error %p %d >= %d", s,
					   s ? s->_length: -1, dynany->idx);
			} else
				value = DYNANY_GET_OFFSET (s->_buffer, dynany->idx,
							   tc->subtypes [0]);
			break;
		}

		case CORBA_tk_array:
			value = DYNANY_GET_OFFSET (any->_value, dynany->idx,
						   tc->subtypes [0]);
			break;

		case CORBA_tk_struct:
		case CORBA_tk_except: {
			int i;
			/* FIXME: could cache for efficiency */
			value = any->_value;
			for (i = 0; i < dynany->idx; i++)
				value = DYNANY_GET_OFFSET (value, 1, tc->subtypes [i]);
			break;
		}

		case CORBA_tk_alias:
			tc = tc->subtypes [0];
			goto get_from_typecode;

		case CORBA_tk_union:
			g_warning ("Can't get some complex types yet");
			value = NULL;

		default:
			g_warning ("Unknown kind '%d'", any->_type->kind);
			value = NULL;
			break;
		}
	}

	if (!value)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_DynamicAny_DynAny_InvalidValue, NULL);
	
	return value;
}

static void
dynany_get (DynAny *dynany,
	    gpointer retval,
	    CORBA_TypeCode value_type,
	    CORBA_Environment *ev)
{
	gpointer dval;

	dval = dynany_get_value (dynany, ev);
	if (!dval)
		return;

	_ORBit_copy_value (&dval, &retval, value_type);
}

static void
dynany_insert (DynAny        *dynany,
	       CORBA_TypeCode value_type,
	       gpointer       value,
	       CORBA_Environment *ev)
{
	gpointer dval;

	dval = dynany_get_value (dynany, ev);
	if (!dval)
		return;

	_ORBit_copy_value (&value, &dval, value_type);
}

/**
 * dynany_create:
 * @type: the type of the any
 * @value: a pointer to a value
 * @parent: flags whether we are a slave object or a master
 * @ev: exception environment
 * 
 *   If we have a parent, it is reffed and we store a pointer
 * into the parent's data structure. -REALLOC-!
 * 
 * Return value: a new DynAny object
 **/
static DynamicAny_DynAny
dynany_create (const CORBA_TypeCode type,
	       gpointer             value,
	       DynAny              *parent,
	       CORBA_Environment   *ev)
{
	DynamicAny_DynAny object;
	DynAny      *dynany;

	o_return_val_if_fail (type, CORBA_OBJECT_NIL);

	object = g_new0 (struct DynamicAny_DynAny_type, 1);
	if (!object)
		goto nomemory;

	dynany = g_new (DynAny, 1);
	if (!dynany) {
		g_free (object);
		goto nomemory;
	}

	dynany->idx = 0;
	dynany->parent = NULL;
	dynany->children = NULL;
	dynany->parent_idx = 0;

	ORBit_RootObject_set_interface (
		ORBIT_ROOT_OBJECT (object),
		(ORBit_RootObject_Interface *) &DynamicAny_DynAny__epv, ev);
	ORBIT_ROOT_OBJECT (object)->refs = 0;

	dynany->any = CORBA_any_alloc ();
	dynany->any->_type = (CORBA_TypeCode)
		CORBA_Object_duplicate ((CORBA_Object) type, ev);

	if (parent) {
		dynany->parent = parent;
		dynany->parent_idx = parent->idx;
		parent->children = g_slist_prepend (
			parent->children, dynany);

		g_assert (value);
		dynany->any->_release = CORBA_FALSE;
		dynany->any->_value = value;
	} else {
		dynany->any->_release = CORBA_TRUE;

		if (value)
			dynany->any->_value = ORBit_copy_value (value, type);
		else
			dynany->any->_value = dynany_any_new_default (type);
	}

	object->data = dynany;

	return object;

 nomemory:
	CORBA_exception_set_system (
		ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);

	return CORBA_OBJECT_NIL;
}


/* Section 7.2.2 */
DynamicAny_DynAny
CORBA_ORB_create_dyn_any(CORBA_ORB obj,
			 CORBA_any *value,
			 CORBA_Environment *ev)
{
	o_return_val_if_fail (value, CORBA_OBJECT_NIL);

	return dynany_create (value->_type, value->_value, CORBA_OBJECT_NIL, ev);
}

/* Section 7.2.2
 *
 * raises: InconsistentTypeCode
 */
DynamicAny_DynAny
CORBA_ORB_create_basic_dyn_any(CORBA_ORB obj,
			       const CORBA_TypeCode type,
			       CORBA_Environment *ev)
{
	o_return_val_if_fail (type, CORBA_OBJECT_NIL);

	return dynany_create (type, NULL, CORBA_OBJECT_NIL, ev);
}

/* Section 7.2.2
 *
 * raises: InconsistentTypeCode
 */
DynamicAny_DynStruct
CORBA_ORB_create_dyn_struct (CORBA_ORB obj,
			     const CORBA_TypeCode type,
			     CORBA_Environment *ev)
{
	o_return_val_if_fail (type, CORBA_OBJECT_NIL);

	return (DynamicAny_DynStruct) dynany_create (type, NULL, CORBA_OBJECT_NIL, ev);
}

/* Section 7.2.2
 *
 * raises: InconsistentTypeCode
 */
DynamicAny_DynSequence
CORBA_ORB_create_dyn_sequence (CORBA_ORB obj,
			       const CORBA_TypeCode type,
			       CORBA_Environment *ev)
{
	o_return_val_if_fail (type, CORBA_OBJECT_NIL);

	return (DynamicAny_DynSequence) dynany_create (type, NULL, CORBA_OBJECT_NIL, ev);
}

/* Section 7.2.2
 *
 * raises: InconsistentTypeCode
 */
DynamicAny_DynArray
CORBA_ORB_create_dyn_array (CORBA_ORB obj,
			    const CORBA_TypeCode type,
			    CORBA_Environment *ev)
{
	o_return_val_if_fail (type, CORBA_OBJECT_NIL);

	return (DynamicAny_DynArray) dynany_create (type, NULL, CORBA_OBJECT_NIL, ev);
}

/* Section 7.2.2
 *
 * raises: InconsistentTypeCode
 */
DynamicAny_DynUnion
CORBA_ORB_create_dyn_union (CORBA_ORB obj,
			    const CORBA_TypeCode type,
			    CORBA_Environment *ev)
{
	o_return_val_if_fail (type, CORBA_OBJECT_NIL);

	return (DynamicAny_DynUnion) dynany_create (type, NULL, CORBA_OBJECT_NIL, ev);
}

/* Section 7.2.2
 *
 * raises: InconsistentTypeCode
 */
DynamicAny_DynEnum
CORBA_ORB_create_dyn_enum (CORBA_ORB obj,
			   const CORBA_TypeCode type,
			   CORBA_Environment *ev)
{
	o_return_val_if_fail (type, CORBA_OBJECT_NIL);

	return (DynamicAny_DynEnum) dynany_create (type, NULL, CORBA_OBJECT_NIL, ev);
}

/* 9.2.3.1 */

CORBA_TypeCode
DynamicAny_DynAny_type (DynamicAny_DynAny obj, CORBA_Environment *ev)
{
	DynAny *dynany;

	o_return_val_if_fail (obj != NULL, CORBA_OBJECT_NIL);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL,
			      CORBA_OBJECT_NIL);

	return (CORBA_TypeCode) CORBA_Object_duplicate (
		(CORBA_Object) dynany->any->_type, ev);
}

/* 9.2.3.2 */

void
DynamicAny_DynAny_assign (DynamicAny_DynAny dest,
		     DynamicAny_DynAny src,
		     CORBA_Environment *ev)
{
	DynAny *dynany_src;

	o_return_if_fail (dest && src);

	dynany_src = GET_DYNANY (src);
	o_return_if_fail (dynany_src != NULL &&
			  dynany_src->any != NULL &&
			  dynany_src->any->_type != NULL);

	DynamicAny_DynAny_from_any (dest, dynany_src->any, ev);
}

/* 9.2.3.3 */

void
DynamicAny_DynAny_from_any (DynamicAny_DynAny obj,
		       CORBA_any   *value,
		       CORBA_Environment *ev)
{
	DynAny  *dynany;
	gboolean equal;

	o_return_if_fail (value != NULL && value->_type);

	dynany = GET_DYNANY (obj);
	b_return_if_fail (dynany != NULL &&
			  dynany->any != NULL &&
			  dynany->any->_type != NULL);

	equal = CORBA_TypeCode_equal (dynany->any->_type, value->_type, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return;

	if (!equal) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_DynamicAny_DynAny_TypeMismatch, NULL);
		return;
	}

	dynany_invalidate (dynany, TRUE, ev);
	
	CORBA_free (dynany->any);

	dynany->any = CORBA_any_alloc ();
	CORBA_any__copy (dynany->any, value);
}

/* 9.2.3.4 */

CORBA_any *
DynamicAny_DynAny_to_any (DynamicAny_DynAny obj,
			  CORBA_Environment *ev)
{
	DynAny    *dynany;
	CORBA_any *any;

	o_return_val_if_fail (obj != NULL, NULL);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL,
			      NULL);

	any = CORBA_any_alloc ();

	CORBA_any__copy (any, dynany->any);

	return any;
}

/* 9.2.3.5 */

CORBA_boolean
DynamicAny_DynAny_equal (DynamicAny_DynAny obj_a,
			 DynamicAny_DynAny obj_b,
			 CORBA_Environment *ev)
{
	DynAny *dynanya;
	DynAny *dynanyb;

	o_return_val_if_fail (obj_a != NULL && obj_b != NULL, FALSE);

	dynanya = GET_DYNANY (obj_a);
	dynanyb = GET_DYNANY (obj_b);
	b_return_val_if_fail (dynanya != NULL &&
			      dynanya->any != NULL,
			      FALSE);

	b_return_val_if_fail (dynanyb != NULL &&
			      dynanyb->any != NULL,
			      FALSE);

	return ORBit_any_equivalent (dynanya->any, dynanyb->any, ev);
}

/* 9.2.3.6 */

void
DynamicAny_DynAny_destroy(DynamicAny_DynAny obj,
		     CORBA_Environment *ev)
{
	/*
	 * This method is almost totaly pointless
	 * In theory we should destroy the Any and return
	 * OBJECT_NOT_EXIST if people try to access it.
	 */
}

/* 9.2.3.7 */

DynamicAny_DynAny
DynamicAny_DynAny_copy (DynamicAny_DynAny obj,
		   CORBA_Environment *ev)
{
	DynAny *dynany;

	o_return_val_if_fail (obj != NULL, CORBA_OBJECT_NIL);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL,
			      CORBA_OBJECT_NIL);

	return dynany_create (dynany->any->_type, dynany->any->_value, dynany->parent, ev);
}

/* 9.2.3.8 */

void
DynamicAny_DynAny_insert_string (DynamicAny_DynAny       obj,
				 const CORBA_char  *value,
				 CORBA_Environment *ev)
{
	DynAny *dynany;

	o_return_if_fail (obj != NULL);

	dynany = GET_DYNANY (obj);
	b_return_if_fail (dynany != NULL &&
			  dynany->any != NULL);

	if (dynany_type_mismatch (dynany, TC_string, ev))
		return;

	dynany_insert (dynany, TC_string, (gpointer)&value, ev);

	return;
}

#define MAKE_DYNANY_INSERT(ctype,typecode,apiname)				\
										\
void										\
DynamicAny_DynAny_insert_##apiname (DynamicAny_DynAny       obj,				\
			       CORBA_##ctype      value,			\
			       CORBA_Environment *ev)				\
{										\
	DynAny  *dynany;							\
										\
	o_return_if_fail (obj != NULL);						\
										\
	dynany = GET_DYNANY (obj);						\
	b_return_if_fail (dynany != NULL &&					\
			  dynany->any != NULL);					\
										\
	if (dynany_type_mismatch (dynany, typecode, ev))			\
		return;								\
										\
	dynany_insert (dynany, typecode, &value, ev);				\
										\
	return;									\
}

#if 0
MAKE_DYNANY_INSERT (char *,              TC_string,             string);
#endif
MAKE_DYNANY_INSERT (short,               TC_short,              short);
MAKE_DYNANY_INSERT (long,                TC_long,               long);
MAKE_DYNANY_INSERT (unsigned_short,      TC_ushort,             ushort);
MAKE_DYNANY_INSERT (unsigned_long,       TC_ulong,              ulong);
MAKE_DYNANY_INSERT (float,               TC_float,              float);
MAKE_DYNANY_INSERT (double,              TC_double,             double);
MAKE_DYNANY_INSERT (long_double,         TC_longdouble,         longdouble);
MAKE_DYNANY_INSERT (boolean,             TC_boolean,            boolean);
MAKE_DYNANY_INSERT (char,                TC_char,               char);
MAKE_DYNANY_INSERT (wchar,               TC_wchar,              wchar);
MAKE_DYNANY_INSERT (octet,               TC_octet,              octet);
MAKE_DYNANY_INSERT (any *,               TC_any,                any);
MAKE_DYNANY_INSERT (TypeCode,            TC_TypeCode,           typecode);
MAKE_DYNANY_INSERT (Object,              TC_Object,             reference);
MAKE_DYNANY_INSERT (wchar *,             TC_wstring,            wstring);
#ifdef HAVE_CORBA_LONG_LONG
MAKE_DYNANY_INSERT (long_long,           TC_longlong,           longlong); 
MAKE_DYNANY_INSERT (unsigned_long_long,  TC_ulonglong,          ulonglong);
#endif

#define MAKE_DYNANY_GET(ctype,typecode,apiname)					\
										\
CORBA_##ctype									\
DynamicAny_DynAny_get_##apiname (DynamicAny_DynAny       obj,				\
			    CORBA_Environment *ev)				\
{										\
	DynAny       *dynany;							\
	CORBA_##ctype value;							\
										\
	o_return_val_if_fail (obj != NULL, 0);				       	\
										\
	dynany = GET_DYNANY (obj);						\
										\
	/* FIXME: assumes NULL == CORBA_OBJECT_NIL for simplicity */		\
	b_return_val_if_fail (dynany != NULL &&					\
			      dynany->any != NULL, 0);				\
										\
	if (dynany_type_mismatch (dynany, typecode, ev))			\
		return 0;	       						\
										\
	dynany_get (dynany, &value, typecode, ev);				\
										\
	return value;								\
}

MAKE_DYNANY_GET (short,               TC_short,              short);
MAKE_DYNANY_GET (long,                TC_long,               long);
MAKE_DYNANY_GET (unsigned_short,      TC_ushort,             ushort);
MAKE_DYNANY_GET (unsigned_long,       TC_ulong,              ulong);
MAKE_DYNANY_GET (float,               TC_float,              float);
MAKE_DYNANY_GET (double,              TC_double,             double);
MAKE_DYNANY_GET (long_double,         TC_longdouble,         longdouble);
MAKE_DYNANY_GET (boolean,             TC_boolean,            boolean);
MAKE_DYNANY_GET (char,                TC_char,               char);
MAKE_DYNANY_GET (wchar,               TC_wchar,              wchar);
MAKE_DYNANY_GET (octet,               TC_octet,              octet);
MAKE_DYNANY_GET (any *,               TC_any,                any);
MAKE_DYNANY_GET (TypeCode,            TC_TypeCode,           typecode);
MAKE_DYNANY_GET (Object,              TC_Object,             reference);
MAKE_DYNANY_GET (char *,              TC_string,             string);
MAKE_DYNANY_GET (wchar *,             TC_wstring,            wstring);
#ifdef HAVE_CORBA_LONG_LONG
MAKE_DYNANY_GET (long_long,           TC_longlong,           longlong); 
MAKE_DYNANY_GET (unsigned_long_long,  TC_ulonglong,          ulonglong);
#endif

/* 9.2.3.9 */

CORBA_boolean
DynamicAny_DynAny_seek (DynamicAny_DynAny obj, CORBA_long index, CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_any *any;
	CORBA_TypeCode tc;

	o_return_val_if_fail (obj != NULL, FALSE);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL,
			      FALSE);
	any = dynany->any;
	tc = any->_type;

 validate_seek:
	switch (tc->kind) {
	case DYNANY_TYPE_SINGLE:
	case CORBA_tk_enum:
	case CORBA_tk_fixed:
		dynany->idx = -1;
		if (index == -1)
			return TRUE;
		else
			return FALSE;

	case CORBA_tk_array:
		if (index < tc->length && index >= 0) {
			dynany->idx = index;
			return TRUE;
		} else {
			dynany->idx = -1;
			return FALSE;
		}
		break;

 	case CORBA_tk_struct:
 	case CORBA_tk_except:
		if (index < tc->sub_parts && index >= 0) {
			dynany->idx = index;
			return TRUE;
		}
		break;

	case CORBA_tk_sequence: {
		CORBA_sequence_octet *s;

		s = dynany->any->_value;
		if (s && index < s->_length) {
			dynany->idx = index;
			return TRUE;
		} else {
			dynany->idx = -1;
			return FALSE;
		}
	}

	case CORBA_tk_union:
		if (index < 0 || index > 1) {
			dynany->idx = -1;
			return FALSE;
		}
		dynany->idx = index;
		return TRUE;

	case CORBA_tk_alias:
		tc = tc->subtypes [0];
		goto validate_seek;

	default:
		g_error ("Unknown kind '%d'", tc->kind);
	}

	dynany->idx = -1;

	return FALSE;
}

CORBA_unsigned_long
DynamicAny_DynAny_component_count (DynamicAny_DynAny obj,
			      CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_any *any;
	CORBA_TypeCode tc;

	o_return_val_if_fail (obj != NULL, 0);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL, 0);

	any = dynany->any;
	tc = any->_type;

 count_type:
	switch (tc->kind) {
	case DYNANY_TYPE_SINGLE:
	case CORBA_tk_fixed:
	case CORBA_tk_enum:
		return 0;

	case CORBA_tk_sequence: {
		CORBA_sequence_octet *s = any->_value;
		if (!s) {
			g_warning ("Wierd");
			return 0;
		}
		
		return s->_length;
	}

	case CORBA_tk_array:
		return tc->length;

 	case CORBA_tk_struct:
 	case CORBA_tk_except:
		return tc->sub_parts;

	case CORBA_tk_union:
		g_warning ("Can't count complex types yet");
		break;

	case CORBA_tk_alias:
		tc = tc->subtypes [0];
		goto count_type;

	default:
		g_error ("Unknown kind '%d'", tc->kind);
		break;
	}

	return 0;
}

DynamicAny_DynAny
DynamicAny_DynAny_current_component (DynamicAny_DynAny obj,
				CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_any *any;
	CORBA_TypeCode tc;

	o_return_val_if_fail (obj != NULL, CORBA_OBJECT_NIL);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL,
			      CORBA_OBJECT_NIL);
	if (dynany->idx < 0)
		return CORBA_OBJECT_NIL;

	any = dynany->any;
	tc = any->_type;
 find_type:	
	switch (tc->kind) {

	case CORBA_tk_alias:
		tc = tc->subtypes [0];
		goto find_type;

	case CORBA_tk_enum:
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_DynamicAny_DynAny_TypeMismatch, NULL);
		return CORBA_OBJECT_NIL;

	case CORBA_tk_except:
		if (tc->sub_parts == 0) {
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_DynamicAny_DynAny_TypeMismatch, NULL);
			return CORBA_OBJECT_NIL;
		} /* else drop through */

	case CORBA_tk_fixed:
	case CORBA_tk_union:
 	case CORBA_tk_struct:
	case CORBA_tk_sequence:
	case CORBA_tk_array:
	case DYNANY_TYPE_SINGLE:
		return dynany_create (
			dynany_get_cur_type (dynany),
			dynany_get_value (dynany, ev), dynany, ev);

	default:
		g_error ("Unknown kind '%d'", any->_type->kind);
	}

	return CORBA_OBJECT_NIL;
}

CORBA_boolean
DynamicAny_DynAny_next (DynamicAny_DynAny obj, CORBA_Environment *ev)
{
	DynAny *dynany;

	o_return_val_if_fail (obj != NULL, FALSE);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL,
			      FALSE);

	return DynamicAny_DynAny_seek (obj, dynany->idx + 1, ev);
}

void
DynamicAny_DynAny_rewind (DynamicAny_DynAny obj, CORBA_Environment *ev)
{
	DynamicAny_DynAny_seek (obj, 0, ev);
}

/* 9.2.5 */

CORBA_char *
DynamicAny_DynEnum_get_as_string (DynamicAny_DynEnum obj,
			     CORBA_Environment *ev)
{
	CORBA_unsigned_long *i;
	DynAny *dynany;
	CORBA_TypeCode tc;

	o_return_val_if_fail (obj != NULL, NULL);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL,
			      NULL);

	if (dynany_kind_mismatch (dynany, CORBA_tk_enum, ev))
		return NULL;

	i = dynany_get_value (dynany, ev);
	if (!i)
		return NULL;

	tc = dynany->any->_type;

	g_assert (*i < tc->sub_parts);

	return CORBA_string_dup (tc->subnames [*i]);
}

void
DynamicAny_DynEnum_set_as_string (DynamicAny_DynEnum obj,
			     const CORBA_char *value_as_string,
			     CORBA_Environment *ev)
{
	CORBA_unsigned_long i;
	DynAny *dynany;
	CORBA_TypeCode tc;

	o_return_if_fail (obj != NULL);

	dynany = GET_DYNANY (obj);
	b_return_if_fail (dynany != NULL &&
			  dynany->any != NULL &&
			  dynany->any->_type != NULL);

	if (dynany_kind_mismatch (dynany, CORBA_tk_enum, ev))
		return;

	tc = dynany->any->_type;
	for (i = 0; i < tc->sub_parts; i++) {
		if (!strcmp (tc->subnames [i], value_as_string)) {
			CORBA_unsigned_long *e;

			e = dynany_get_value (dynany, ev);
			if (!e)
				return;

			*e = i;
			return;
		}
	}

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_DynamicAny_DynAny_InvalidValue, NULL);
}

CORBA_unsigned_long
DynamicAny_DynEnum_get_as_ulong (DynamicAny_DynEnum obj,
			    CORBA_Environment *ev)
{
	CORBA_unsigned_long *i;
	DynAny *dynany;

	o_return_val_if_fail (obj != NULL, 0);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL,
			      0);

	if (dynany_kind_mismatch (dynany, CORBA_tk_enum, ev))
		return 0;

	i = dynany_get_value (dynany, ev);
	if (!i)
		return 0;
	
	return *i;
}

void
DynamicAny_DynEnum_set_as_ulong (DynamicAny_DynEnum obj,
			    CORBA_unsigned_long value_as_ulong,
			    CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_TypeCode tc;

	o_return_if_fail (obj != NULL);

	dynany = GET_DYNANY (obj);
	b_return_if_fail (dynany != NULL &&
			  dynany->any != NULL &&
			  dynany->any->_type != NULL);

	if (dynany_kind_mismatch (dynany, CORBA_tk_enum, ev))
		return;

	tc = dynany->any->_type;
	if (value_as_ulong < tc->sub_parts) {
		CORBA_unsigned_long *e;

		e = dynany_get_value (dynany, ev);
		if (!e)
			return;
		
		*e = value_as_ulong;
	} else
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_DynamicAny_DynAny_InvalidValue, NULL);
}

/* 9.2.6 */

CORBA_FieldName
DynamicAny_DynStruct_current_member_name (DynamicAny_DynStruct obj,
					  CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_TypeCode tc;

	o_return_val_if_fail (obj != NULL, NULL);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL, NULL);

	if (dynany_kind_mismatch (dynany, CORBA_tk_struct, ev))
		return NULL;

	tc = dynany->any->_type;

	if (dynany->idx >= 0 &&
	    dynany->idx < tc->sub_parts) {

		if (tc->subnames [dynany->idx])
			return CORBA_string_dup (
				tc->subnames [dynany->idx]);
		else
			return CORBA_string_dup ("");
	}
	
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_DynamicAny_DynAny_InvalidValue, NULL);
	return NULL;
}

CORBA_TCKind
DynamicAny_DynStruct_current_member_kind (DynamicAny_DynStruct obj,
					  CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_TypeCode tc;
	
	o_return_val_if_fail (obj != NULL, 0);
	
	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL, 0);
	
	if (dynany_kind_mismatch (dynany, CORBA_tk_struct, ev))
		return 0;

	tc = dynany->any->_type;

	if (dynany->idx >= 0 &&
	    dynany->idx < tc->sub_parts &&
	    tc->subtypes [dynany->idx])

		return tc->subtypes [dynany->idx]->kind;
	
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_DynamicAny_DynAny_InvalidValue, NULL);
	return 0;
}

CORBA_NameValuePairSeq *
DynamicAny_DynStruct_get_members (DynamicAny_DynStruct obj,
			     CORBA_Environment *ev)
{
	g_assert (!"Not yet implemented");
	return NULL;
}

void
DynamicAny_DynStruct_set_members (DynamicAny_DynStruct obj,
			     CORBA_NameValuePairSeq *value,
			     CORBA_Environment *ev)
{
	g_assert (!"Not yet implemented");
}

/* 9.2.7 */

DynamicAny_DynAny
DynamicAny_DynUnion_get_discriminator (DynamicAny_DynUnion obj,
				  CORBA_Environment *ev)
{
	g_assert (!"Not yet implemented");
	return NULL;
}

void
DynamicAny_DynUnion_set_discriminator (DynamicAny_DynUnion obj,
				  DynamicAny_DynAny   d,
				  CORBA_Environment *ev)
{
	g_assert (!"Not yet implemented");
}

void
DynamicAny_DynUnion_set_to_default_member (DynamicAny_DynUnion obj,
				      CORBA_Environment *ev)
{
	g_assert (!"Not yet implemented");
}

void
DynamicAny_DynUnion_set_to_no_active_member (DynamicAny_DynUnion obj,
					CORBA_Environment *ev)
{
	g_assert (!"Not yet implemented");
}

CORBA_boolean
DynamicAny_DynUnion_has_no_active_member (DynamicAny_DynUnion obj,
				     CORBA_Environment *ev)
{
	DynAny *dynany;
	
	o_return_val_if_fail (obj != NULL, 0);
	
	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL, 0);

	if (dynany->idx == 0)
		return TRUE;
	else
		return FALSE;
}

CORBA_TCKind
DynamicAny_DynUnion_discriminator_kind (DynamicAny_DynUnion obj,
				   CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_TypeCode tc;
	
	o_return_val_if_fail (obj != NULL, 0);
	
	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL, 0);
	
	if (dynany_kind_mismatch (dynany, CORBA_tk_union, ev))
		return 0;

	tc = dynany->any->_type;

	if (tc->discriminator)
		return tc->discriminator->kind;

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_DynamicAny_DynAny_InvalidValue, NULL);
	return 0;
}

DynamicAny_DynAny
DynamicAny_DynUnion_member (DynamicAny_DynUnion obj,
		       CORBA_Environment *ev)
{
	g_assert (!"Not yet implemented");
	return NULL;
}

CORBA_FieldName
DynamicAny_DynUnion_member_name (DynamicAny_DynUnion obj,
			    CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_TypeCode tc;
	
	o_return_val_if_fail (obj != NULL, 0);
	
	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL, 0);

	tc = dynany->any->_type;

	if (dynany->idx >= 0 &&
	    dynany->idx < tc->sub_parts) {

		if (tc->subnames [dynany->idx])
			return CORBA_string_dup (
				tc->subnames [dynany->idx]);
		else
			return CORBA_string_dup ("");
	}
	
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_DynamicAny_DynAny_InvalidValue, NULL);
	return NULL;
}

CORBA_TCKind
DynamicAny_DynUnion_member_kind (DynamicAny_DynUnion obj,
			    CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_TypeCode tc;
	
	o_return_val_if_fail (obj != NULL, 0);
	
	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL, 0);
	
	if (dynany_kind_mismatch (dynany, CORBA_tk_union, ev))
		return 0;

	tc = dynany->any->_type;

	if (dynany->idx >= 0 &&
	    dynany->idx < tc->sub_parts &&
	    tc->subtypes [dynany->idx])

		return tc->subtypes [dynany->idx]->kind;

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_DynamicAny_DynAny_InvalidValue, NULL);
	return 0;
}

/* 9.2.8 */

DynamicAny_DynAny_AnySeq *
DynamicAny_DynSequence_get_elements (DynamicAny_DynSequence obj,
				CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_sequence_octet *s;
	DynamicAny_DynAny_AnySeq *retval;
	int i;
	gpointer src;
	CORBA_TypeCode subtc;

	o_return_val_if_fail (obj != NULL, NULL);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL,
			      NULL);

	if (dynany_kind_mismatch (dynany, CORBA_tk_sequence, ev))
		return NULL;

	s = dynany->any->_value;
	if (!s)
		return NULL;
	
	src = s->_buffer;
	retval = CORBA_sequence_DynamicAny_DynAny_AnySeq__alloc ();
	retval->_buffer = CORBA_sequence_DynamicAny_DynAny_AnySeq_allocbuf (
		s->_length);
	retval->_length = s->_length;
	subtc = dynany->any->_type->subtypes [0];

	for (i = 0; i < s->_length; i++) {
		CORBA_any *any = CORBA_any__alloc ();
		gpointer   to;

		retval->_buffer [i] = any;

		any->_type = (CORBA_TypeCode) CORBA_Object_duplicate (
			(CORBA_Object) subtc, ev);
		to = any->_value = ORBit_demarshal_allocate_mem (subtc, 1);
		
		_ORBit_copy_value (&src, &to, subtc);
	}

	return retval;
}

void
DynamicAny_DynSequence_set_elements (DynamicAny_DynSequence obj,
				DynamicAny_DynAny_AnySeq *value,
				CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_sequence_octet *s;
	int i;
	gpointer dest;
	CORBA_TypeCode subtc;

	o_return_if_fail (obj != NULL);
	o_return_if_fail (value != NULL);

	dynany = GET_DYNANY (obj);
	b_return_if_fail (dynany != NULL &&
			  dynany->any != NULL &&
			  dynany->any->_type != NULL);

	if (dynany_kind_mismatch (dynany, CORBA_tk_sequence, ev))
		return;

	s = dynany->any->_value;
	if (!s)
		return;
	
	subtc = dynany->any->_type->subtypes [0];

	for (i = 0; i < value->_length && i < s->_length; i++) {
		CORBA_any *a = value->_buffer [i];
		if (!a || !a->_type ||
		    !CORBA_TypeCode_equal (subtc, a->_type, ev)) {
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_DynamicAny_DynAny_InvalidValue, NULL);
			return;
		}
	}

	dynany_invalidate (dynany, FALSE, ev);

	dest = s->_buffer;
	for (i = 0; i < value->_length; i++) {
		CORBA_any *a = value->_buffer [i];
		gpointer src = a->_value;

		_ORBit_copy_value (&src, &dest, subtc);
	}
}

CORBA_unsigned_long
DynamicAny_DynSequence_get_length (DynamicAny_DynSequence  obj,
			      CORBA_Environment *ev)
{
	DynAny *dynany;
	CORBA_sequence_octet *s;

	o_return_val_if_fail (obj != NULL, 0);

	dynany = GET_DYNANY (obj);
	b_return_val_if_fail (dynany != NULL &&
			      dynany->any != NULL &&
			      dynany->any->_type != NULL,
			      0);

	if (dynany_kind_mismatch (dynany, CORBA_tk_sequence, ev))
		return -1;

	s = dynany->any->_value;
	if (!s)
		return -1;

	return s->_length;
}

void
DynamicAny_DynSequence_set_length (DynamicAny_DynSequence   obj,
			      CORBA_unsigned_long length,
			      CORBA_Environment  *ev)
{
	DynAny *dynany;
	CORBA_sequence_octet *s;
	CORBA_long old_length;

	o_return_if_fail (obj != NULL);

	dynany = GET_DYNANY (obj);
	b_return_if_fail (dynany != NULL &&
			  dynany->any != NULL &&
			  dynany->any->_type != NULL);

	if (dynany_kind_mismatch (dynany, CORBA_tk_sequence, ev))
		return;

	s = dynany->any->_value;
	if (!s)
		return;

	if (length == s->_length)
		return;

	if (s->_maximum && length > s->_maximum) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_DynamicAny_DynAny_InvalidValue, NULL);
		return;
	}

	old_length = s->_length;
	if (!dynany_sequence_realloc_to (s, dynany->any->_type, length, ev))
		return;

	if (length > old_length) {
		if (dynany->idx == -1)
			dynany->idx = old_length;

	} else {
		GSList *l;

		for (l = dynany->children; l; l = l->next) {
			DynAny *child = l->data;

			if (child->parent_idx >= length)
				dynany_invalidate (child, TRUE, ev);
		}

		if (length == 0 ||
		    dynany->idx >= length)
			dynany->idx = -1;
	}
}

/* 9.2.9 */

DynamicAny_DynAny_AnySeq *
DynamicAny_DynArray_get_elements (DynamicAny_DynArray obj,
			     CORBA_Environment *ev)
{
	g_assert (!"Not yet implemented");
	return NULL;
}

void
DynamicAny_DynArray_set_elements (DynamicAny_DynArray obj,
			     DynamicAny_DynAny_AnySeq *value,
			     CORBA_Environment *ev)
{
	g_assert (!"Not yet implemented");
}

static const CORBA_TypeCode
DynamicAny_DynAny_AnySeq_subtypes_array [] = {
	TC_any
};

const struct CORBA_TypeCode_struct
TC_CORBA_sequence_DynamicAny_DynAny_AnySeq_struct = {
	{{(ORBit_RootObject_Interface *) & ORBit_TypeCode_epv, TRUE, -1},
	 ORBIT_PSEUDO_TYPECODE},

	CORBA_tk_sequence, NULL, NULL,
	0, 1,
	NULL,
	(CORBA_TypeCode *) DynamicAny_DynAny_AnySeq_subtypes_array,
	NULL,
	CORBA_OBJECT_NIL, 0, -1, 0, 0
};

CORBA_sequence_DynamicAny_DynAny_AnySeq *
CORBA_sequence_DynamicAny_DynAny_AnySeq__alloc (void)
{
	CORBA_sequence_DynamicAny_DynAny_AnySeq *retval;

	retval = ORBit_demarshal_allocate_mem (
		TC_CORBA_sequence_DynamicAny_DynAny_AnySeq, 1);

	retval->_maximum = 0;
	retval->_length = 0;
	retval->_buffer = NULL;
	retval->_release = CORBA_FALSE;

	return retval;
}

CORBA_any **
CORBA_sequence_DynamicAny_DynAny_AnySeq_allocbuf (CORBA_unsigned_long len)
{
	CORBA_any **retval;

	retval = ORBit_demarshal_allocate_mem (
		TC_any, len);

	memset (retval, '\0', sizeof (CORBA_any *) * len);

	return retval;
}

static const CORBA_TypeCode
DynamicAny_DynAny_DynAnySeq_subtypes_array [] = {
	TC_Object
};

const struct CORBA_TypeCode_struct
TC_CORBA_sequence_DynamicAny_DynAny_DynAnySeq_struct = {
	{{(ORBit_RootObject_Interface *) & ORBit_TypeCode_epv, TRUE, -1},
	 ORBIT_PSEUDO_TYPECODE},

	CORBA_tk_sequence, NULL, NULL,
	0, 1,
	NULL,
	(CORBA_TypeCode *) DynamicAny_DynAny_DynAnySeq_subtypes_array,
	NULL,
	CORBA_OBJECT_NIL, 0, -1, 0, 0
};

CORBA_sequence_DynamicAny_DynAny_DynAnySeq *
CORBA_sequence_DynamicAny_DynAny_DynAnySeq__alloc (void)
{
	CORBA_sequence_DynamicAny_DynAny_DynAnySeq *retval;

	retval = ORBit_demarshal_allocate_mem (
		TC_CORBA_sequence_DynamicAny_DynAny_DynAnySeq, 1);

	retval->_maximum = 0;
	retval->_length = 0;
	retval->_buffer = NULL;
	retval->_release = CORBA_FALSE;

	return retval;
}

DynamicAny_DynAny *
CORBA_sequence_DynamicAny_DynAny_DynAnySeq_allocbuf (CORBA_unsigned_long len)
{
	DynamicAny_DynAny *retval;

	retval = ORBit_demarshal_allocate_mem (
		TC_Object, len);

	memset (retval, '\0', sizeof (DynamicAny_DynAny) * len);

	return retval;
}
