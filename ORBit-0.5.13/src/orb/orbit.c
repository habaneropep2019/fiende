/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/*
 *  ORBit: A CORBA v2.2 ORB
 *
 *  Copyright (C) 1998 Richard H. Porter and Red Hat Software
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

/*
 * This file is a repository for random functions that don't fit anywhere
 * else, and for ORBit-specific stuff.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <math.h>

#include <config.h>
#include "orbit.h"

const guint orbit_major_version = ORBIT_MAJOR_VERSION,
	orbit_minor_version = ORBIT_MINOR_VERSION,
	orbit_micro_version = ORBIT_MICRO_VERSION;
const char orbit_version[] = ORBIT_VERSION;

typedef struct ORBitClassInfo ORBitClassInfo;

typedef void (*ORBitObjectInitFunc)(CORBA_Object _handle_to_be, gpointer class_data);

struct ORBitClassInfo {
  char *name;
  gulong id;
  gpointer method_stubs, method_skels;
  ORBitObjectInitFunc class_vtable_init_func;
  ORBitClassInfo **parent_classes;
};

GHashTable *orbit_class_list = NULL, *orbit_class_byid;
glong class_id_counter = -1;

void CORBA_any_set_release(CORBA_any *any, CORBA_boolean flag)
{
	g_assert(any!=NULL);

	if(flag==CORBA_TRUE) {
		any->_release |= CORBA_ANYFLAGS_RELEASE;
	} else {
		any->_release &= ~CORBA_ANYFLAGS_RELEASE;
	}

}

CORBA_boolean CORBA_any_get_release(CORBA_any *any)
{
	g_assert(any!=NULL);

	if(any->_release & CORBA_ANYFLAGS_RELEASE)
		return(CORBA_TRUE);
	else
		return(CORBA_FALSE);
}

void CORBA_sequence_set_release(void *seq, CORBA_boolean flag)
{
	struct CORBA_Sequence_type *sequence;

	g_assert(seq!=NULL);

	sequence=(struct CORBA_Sequence_type *)seq;

	if(flag==CORBA_TRUE) {
		sequence->_release |= CORBA_ANYFLAGS_RELEASE;
	} else {
		sequence->_release &= ~CORBA_ANYFLAGS_RELEASE;
	}
}

CORBA_boolean CORBA_sequence_get_release(void *seq)
{
	struct CORBA_Sequence_type *sequence;

	g_assert(seq!=NULL);

	sequence=(struct CORBA_Sequence_type *)seq;

	if(sequence->_release & CORBA_ANYFLAGS_RELEASE)
		return(CORBA_TRUE);
	else
		return(CORBA_FALSE);
}

/*
 * As far as I understand, values returned by CORBA_*_alloc() are supposed to be
 * freeable by CORBA_free(), so we can't use memory chunks here in any reasonable
 * fashion.
 */
gpointer
CORBA_any__free(gpointer mem, gpointer func_data, CORBA_boolean ignore)
{
	CORBA_any *aval = mem;

	if(aval->_release)
		ORBit_free(aval->_value, CORBA_TRUE);
	CORBA_Object_release((CORBA_Object)aval->_type, NULL);

	return aval + 1;
}

CORBA_any *CORBA_any_alloc(void)
{
	CORBA_any *retval = ORBit_alloc(sizeof(CORBA_any), &CORBA_any__free,
					GINT_TO_POINTER(1));

	memset(retval, 0, sizeof(CORBA_any)); /* Make things easier on stubs */

	return retval;
}

#define ALIGN_COMPARE(a,b,tk,type,align)	\
	case CORBA_tk_##tk:			\
		*a = ALIGN_ADDRESS (*a, align);	\
		*b = ALIGN_ADDRESS (*b, align);	\
		ret = *(CORBA_##type *) *a == *(CORBA_##type *) *b;	\
		*a = ((guchar *) *a) + sizeof (CORBA_##type);		\
		*b = ((guchar *) *b) + sizeof (CORBA_##type);		\
		return ret

#define ALIGNOF_CORBA_UNION	(MAX (MAX (ALIGNOF_CORBA_LONG,		\
					   ALIGNOF_CORBA_STRUCT),	\
				      ALIGNOF_CORBA_POINTER))


CORBA_boolean
ORBit_value_equivalent (gpointer *a, gpointer *b,
			CORBA_TypeCode tc,
			CORBA_Environment *ev)
{
	gboolean ret;
	int i;

	switch (tc->kind) {
	case CORBA_tk_null:
	case CORBA_tk_void:
		return TRUE;

		ALIGN_COMPARE(a, b, short,  short, ALIGNOF_CORBA_SHORT);
		ALIGN_COMPARE(a, b, ushort, short, ALIGNOF_CORBA_SHORT);
		ALIGN_COMPARE(a, b, wchar,  short, ALIGNOF_CORBA_SHORT);

		ALIGN_COMPARE(a, b, enum, long,  ALIGNOF_CORBA_LONG);
		ALIGN_COMPARE(a, b, long, long,  ALIGNOF_CORBA_LONG);
		ALIGN_COMPARE(a, b, ulong, long, ALIGNOF_CORBA_LONG);

		ALIGN_COMPARE(a, b, longlong, long_long,  ALIGNOF_CORBA_LONG_LONG);
		ALIGN_COMPARE(a, b, ulonglong, long_long, ALIGNOF_CORBA_LONG_LONG);

		ALIGN_COMPARE(a, b, longdouble, long_double, ALIGNOF_CORBA_LONG_DOUBLE);

		ALIGN_COMPARE(a, b, float, float,   ALIGNOF_CORBA_FLOAT);
		ALIGN_COMPARE(a, b, double, double, ALIGNOF_CORBA_DOUBLE);

		ALIGN_COMPARE(a, b, boolean, octet, ALIGNOF_CORBA_OCTET);
		ALIGN_COMPARE(a, b, char,    octet, ALIGNOF_CORBA_OCTET);
		ALIGN_COMPARE(a, b, octet,   octet, ALIGNOF_CORBA_OCTET);

	case CORBA_tk_string:
		*a = ALIGN_ADDRESS (*a, ALIGNOF_CORBA_POINTER);
		*b = ALIGN_ADDRESS (*b, ALIGNOF_CORBA_POINTER);
		ret = !strcmp (*(char **)*a, *(char **)*b);
		*a = ((guchar *) *a) + sizeof (CORBA_char *);
		*b = ((guchar *) *b) + sizeof (CORBA_char *);
		return ret;

	case CORBA_tk_wstring:
		g_warning ("wstring totaly broken");
  		return FALSE;

	case CORBA_tk_TypeCode:
	case CORBA_tk_objref:
		*a = ALIGN_ADDRESS (*a, ALIGNOF_CORBA_POINTER);
		*b = ALIGN_ADDRESS (*b, ALIGNOF_CORBA_POINTER);
		ret = CORBA_Object_is_equivalent (*a, *b, ev);
		*a = ((guchar *) *a) + sizeof (CORBA_Object);
		*b = ((guchar *) *b) + sizeof (CORBA_Object);
		return ret;

	case CORBA_tk_any: {
		CORBA_any *any_a, *any_b;

		*a = ALIGN_ADDRESS (*a, ALIGNOF_CORBA_POINTER);
		*b = ALIGN_ADDRESS (*b, ALIGNOF_CORBA_POINTER);

		any_a = *((CORBA_any **) *a);
		any_b = *((CORBA_any **) *b);

		ret = ORBit_any_equivalent (any_a, any_b, ev);

		*a = ((guchar *) *a) + sizeof (CORBA_any *);
		*b = ((guchar *) *b) + sizeof (CORBA_any *);

		return ret;
	}

	case CORBA_tk_struct:
	case CORBA_tk_except: {
		int i;

		*a = ALIGN_ADDRESS (*a, ORBit_find_alignment (tc));
		*b = ALIGN_ADDRESS (*b, ORBit_find_alignment (tc));

		for (i = 0; i < tc->sub_parts; i++)
			if (!ORBit_value_equivalent (a, b, tc->subtypes [i], ev))
				return FALSE;

		return TRUE;
	}

	case CORBA_tk_sequence: {
		CORBA_Principal *ap, *bp;
		gpointer a_val, b_val;

		*a = ALIGN_ADDRESS (*a, ALIGNOF_CORBA_UNION);
		*b = ALIGN_ADDRESS (*b, ALIGNOF_CORBA_UNION);
			
		ap = (CORBA_Principal *) *a;
		bp = (CORBA_Principal *) *b;

		if (ap->_length != bp->_length)
			return FALSE;

		a_val = ap->_buffer;
		b_val = bp->_buffer;

		for (i = 0; i < ap->_length; i++) {
			if (!ORBit_value_equivalent (&a_val, &b_val, tc->subtypes [0], ev))
				return FALSE;
		}
		*a = ((guchar *) *a) + sizeof (CORBA_sequence_octet);
		*b = ((guchar *) *b) + sizeof (CORBA_sequence_octet);
		return TRUE;
	}

	case CORBA_tk_union: {
		gint   union_align = ORBit_find_alignment (tc);
		size_t union_size = ORBit_gather_alloc_info (tc);

		CORBA_TypeCode utc_a = ORBit_get_union_tag (tc, a, FALSE);
		CORBA_TypeCode utc_b = ORBit_get_union_tag (tc, b, FALSE);

		if (!CORBA_TypeCode_equal (utc_a, utc_b, ev))
			return FALSE;

		*a = ALIGN_ADDRESS (*a, ALIGNOF_CORBA_STRUCT);
		*b = ALIGN_ADDRESS (*b, ALIGNOF_CORBA_STRUCT);

		if (!ORBit_value_equivalent (a, b, tc->discriminator, ev))
			return FALSE;

		*a = ALIGN_ADDRESS (*a, union_align);
		*b = ALIGN_ADDRESS (*b, union_align);

		if (!ORBit_value_equivalent (a, b, utc_a, ev))
			return FALSE;

		*a = ((guchar *) *a) + union_size;
		*b = ((guchar *) *b) + union_size;
		return TRUE;
	}

	case CORBA_tk_array:
		for (i = 0; i < tc->length; i++) {
			if (!ORBit_value_equivalent (a, b, tc->subtypes [0], ev))
				return FALSE;
		}
		return TRUE;

	case CORBA_tk_alias:
		return ORBit_value_equivalent (a, b, tc->subtypes [0], ev);

	default:
		g_warning ("ORBit_value_equivalent unimplemented");
		return FALSE;
	};
}

/*
 * Compares the typecodes of each any
 */
CORBA_boolean
ORBit_any_equivalent(CORBA_any *obj, CORBA_any *any, CORBA_Environment *ev)
{
	gpointer a, b;

	/* Is this correct ? */
	if (obj == NULL &&
	    any == NULL)
		return TRUE;

	if (!obj->_type || !any->_type) {
		CORBA_exception_set_system (
			ev, ex_CORBA_BAD_PARAM, CORBA_COMPLETED_NO);
		return FALSE;
	}

	if (!CORBA_TypeCode_equal (obj->_type, any->_type, ev))
		return FALSE;

	if (ev->_major != CORBA_NO_EXCEPTION)
		return FALSE;
	
	a = obj->_value;
	b = any->_value;

	return ORBit_value_equivalent (&a, &b, any->_type, ev);
}

/* This is needed by skels, that generate a __free function when they see
   the TypeCode interface */
gpointer
CORBA_TypeCode__free(gpointer mem, gpointer func_data, CORBA_boolean ignore)
{
	CORBA_Object_release(*(CORBA_Object *)mem, NULL);
	return ((guchar *)mem) + sizeof(CORBA_TypeCode);
}

CORBA_char *CORBA_string_dup(const CORBA_char *string)
{
	if(!string)
		return NULL;

	return strcpy(ORBit_alloc(strlen(string)+1, NULL, NULL), string);
}

CORBA_char *CORBA_string_alloc(CORBA_unsigned_long len)
{
	return ORBit_alloc(len + 1, NULL, NULL);
}

CORBA_wchar *CORBA_wstring_alloc(CORBA_unsigned_long len)
{
	return ORBit_alloc(len + 1, NULL, NULL);
}

gpointer
CORBA_string__free(gpointer str, gpointer dat, CORBA_boolean ignore)
{
		CORBA_free(*((gpointer *)str));
	return (gpointer)((guchar *)str + sizeof(CORBA_char *));
}

gpointer CORBA_Object__free(gpointer str, gpointer dat, CORBA_boolean ignore)
{
	CORBA_Environment ev;
	CORBA_exception_init(&ev);
	CORBA_Object_release(*((gpointer *)str), &ev);
	CORBA_exception_free(&ev);
	return (gpointer)((guchar *)str + sizeof(CORBA_Object));
}

/* 19.14 */

/* The big picture for fixeds.
   We have to represent a number in memory.

   1 2 3 . 4 5 6 7

   There are three pieces of information in a fixed:

   - Number of significant digits. (_digits)

   - The scale. The number of places the decimal point is to the right
   of the first significant digit. (_scale)

   - The digits themselves (_value)

 */
CORBA_long CORBA_fixed_integer_part(const void *fp)
{
	CORBA_long retval = 0;
	int i, power_of_ten, digit;
	const CORBA_fixed_d_s *val = fp;

	g_return_val_if_fail(fp != NULL, INT_MIN);

	for(i = 0; i < (val->_digits - val->_scale); i++) {
		power_of_ten = val->_digits - i - val->_scale - 1;
		digit = val->_value[i];
		retval += digit * ((int)pow(10, power_of_ten));
	}

	return retval;
}

CORBA_long CORBA_fixed_fraction_part(const void *fp)
{
	CORBA_long retval = 0;
	int i, power_of_ten, digit;
	const CORBA_fixed_d_s *val = fp;

	g_return_val_if_fail(fp != NULL, INT_MIN);

	for(i = val->_digits - val->_scale; i < val->_digits; i++){
		power_of_ten = val->_digits - i - 1;
		digit = val->_value[i];
		retval += digit * ((int)pow(10, power_of_ten));
	}

	return retval;
}

static inline
CORBA_long do_div (CORBA_long *n)
{
  int __res;

  __res = (*n) % (unsigned) 10;
  *n = (*n) / (unsigned) 10;

  return __res;
}

void CORBA_fixed_set(void *rp, CORBA_long i, CORBA_long f)
{
	CORBA_fixed_d_s *val = rp;
	CORBA_long left_to_eat, cur;
	signed char sign = 1;

	g_return_if_fail(rp != NULL);

	memset(val->_value, 0, val->_digits);

	if(i) sign = i/abs(i);
	val->_sign = sign;
	i = abs(i);
	f = abs(f);

	for(cur = 0, left_to_eat = i;
	    left_to_eat != 0 && cur < val->_digits; cur++) {
		val->_value[cur] = do_div(&left_to_eat) * sign;
		sign = 1;
	}

	val->_scale = cur - 1;

	for(left_to_eat = f;
	    left_to_eat != 0 && cur < val->_digits; cur++) {
		val->_value[cur] = do_div(&left_to_eat);
	}
}

void CORBA_fixed_add(void *rp, const void *f1p, const void *f2p)
{
	g_assert(!"Not yet implemented");
}

void CORBA_fixed_sub(void *rp, const void *f1p, const void *f2p)
{
	g_assert(!"Not yet implemented");
}

void CORBA_fixed_mul(void *rp, const void *f1p, const void *f2p)
{
	g_assert(!"Not yet implemented");
}

void CORBA_fixed_div(void *rp, const void *f1p, const void *f2p)
{
	g_assert(!"Not yet implemented");
}

CORBA_fixed_d_s *CORBA_fixed_alloc(CORBA_unsigned_short d)
{
	return (CORBA_fixed_d_s *)
		g_malloc(sizeof(CORBA_fixed_d_s) + d + 1);
}

void CORBA_free(void *storage)
{
	ORBit_free(storage, CORBA_TRUE);
}

int ORBit_parse_unixsock(CORBA_Object obj,
			 char *sockpath,
			 gboolean existing_only)
{
	if(!sockpath || !*sockpath)
		return -1;
	
	obj->connection =
		GIOP_CONNECTION(iiop_connection_unix_get(sockpath,
							 existing_only));

	if(!obj->connection)
		return -1;

	giop_connection_ref(obj->connection);
	return 0;
}

int ORBit_parse_inet(CORBA_Object obj, char *hostname, unsigned short port,
			 gboolean existing_only)
{
	obj->connection = GIOP_CONNECTION(iiop_connection_get(hostname, port, existing_only));

	if(!obj->connection)
		return -1;
	giop_connection_ref(obj->connection);
	return 0;
}

/* public */ const CORBA_unsigned_long ORBit_zero_int = 0;
struct iovec ORBit_default_principal_iovec = {(gpointer)&ORBit_zero_int, sizeof(ORBit_zero_int)};

void ORBit_set_default_principal(CORBA_Principal *principal)
{
	gpointer t;

	if((gpointer)ORBit_default_principal_iovec.iov_base != (gpointer)&ORBit_zero_int)
		g_free(ORBit_default_principal_iovec.iov_base);

	ORBit_default_principal_iovec.iov_len = principal->_length
		+ sizeof(CORBA_unsigned_long);

	t = ORBit_default_principal_iovec.iov_base =
		g_malloc(ORBit_default_principal_iovec.iov_len);

	memcpy(t, &principal->_length, sizeof(principal->_length));

	t = ((guchar *)t) + sizeof(principal->_length);
	memcpy(t, principal->_buffer, principal->_length);
}

CORBA_unsigned_long ORBit_class_assignment_counter = 0;
GHashTable *ORBit_class_assignments = NULL;

/* XXX not thread-safe */
CORBA_unsigned_long
ORBit_register_class(const PortableServer_ClassInfo *class_info)
{
	CORBA_unsigned_long retval;

	if(!ORBit_class_assignments)
		ORBit_class_assignments = g_hash_table_new(g_str_hash, g_str_equal);

	/* This needs to be pre-increment - we don't want to give out
	   classid 0, because (a) that is reserved for the base Object class
	   (b) all the routines allocate a new id if the variable
	   storing their ID == 0 */
	retval = ++ORBit_class_assignment_counter;

	g_hash_table_insert(ORBit_class_assignments, (gpointer)class_info->class_name,
			    GINT_TO_POINTER(retval));

	return retval;
}
