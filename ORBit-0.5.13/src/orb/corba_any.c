/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit: A CORBA v2.2 ORB
 *
 *  Copyright (C) 1998 Red Hat Software
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
 *  Author: Elliot Lee <sopwith@redhat.com>
 *
 */

#include <config.h>
#include <IIOP/IIOP.h>
#include "orbit.h"

#if 0
#define CORBA_Object_release(x, y) ({ g_message(__FILE__ ":%d Releasing object %#x from %d", __LINE__, \
x, ORBIT_ROOT_OBJECT(x)->refs); CORBA_Object_release(x, y); })
#define CORBA_Object_duplicate(x, y) ({ g_message(__FILE__ ":%d Duping object %#x from %d", __LINE__, \
x, ORBIT_ROOT_OBJECT(x)->refs); CORBA_Object_duplicate(x, y); })
#endif

gint
ORBit_find_alignment(CORBA_TypeCode tc)
{
    gint retval = 1;
    int i;

    switch(tc->kind) {
    case CORBA_tk_union:
	retval = MAX(retval, ORBit_find_alignment(tc->discriminator));
    case CORBA_tk_except:
    case CORBA_tk_struct:
#if ALIGNOF_CORBA_STRUCT > 1
	retval = MAX(retval, ALIGNOF_CORBA_STRUCT);
#endif
	for(i = 0; i < tc->sub_parts; i++)
	    retval = MAX(retval, ORBit_find_alignment(tc->subtypes[i]));
	return retval;
    case CORBA_tk_ulong:
    case CORBA_tk_long:
    case CORBA_tk_enum:
	return ALIGNOF_CORBA_LONG;
    case CORBA_tk_ushort:
    case CORBA_tk_short:
    case CORBA_tk_wchar:
	return ALIGNOF_CORBA_SHORT;
    case CORBA_tk_longlong:
    case CORBA_tk_ulonglong:
	return ALIGNOF_CORBA_LONG_LONG;
    case CORBA_tk_longdouble:
	return ALIGNOF_CORBA_LONG_DOUBLE;
    case CORBA_tk_float:
	return ALIGNOF_CORBA_FLOAT;
    case CORBA_tk_double:
	return ALIGNOF_CORBA_DOUBLE;
    case CORBA_tk_boolean:
    case CORBA_tk_char:
    case CORBA_tk_octet:
	return ALIGNOF_CORBA_CHAR;
    case CORBA_tk_string:
    case CORBA_tk_wstring:
    case CORBA_tk_TypeCode:
    case CORBA_tk_objref:
	return ALIGNOF_CORBA_POINTER;
    case CORBA_tk_sequence:
    case CORBA_tk_any:
	return MAX(MAX(ALIGNOF_CORBA_LONG, ALIGNOF_CORBA_STRUCT), ALIGNOF_CORBA_POINTER);
    case CORBA_tk_array:
    case CORBA_tk_alias:
	return ORBit_find_alignment(tc->subtypes[0]);
    case CORBA_tk_fixed:
	return MAX(ALIGNOF_CORBA_SHORT, ALIGNOF_CORBA_STRUCT);
    default:
	return 1;
    }
}

static void
ORBit_marshal_value(GIOPSendBuffer *buf,
		    gpointer *val,
		    CORBA_TypeCode tc,
		    ORBit_marshal_value_info *mi)
{
    CORBA_unsigned_long i, ulval;
    gpointer subval;
    ORBit_marshal_value_info submi;

#if 0
    g_message("Marshalling a %d value from %#x to offset %d",
	      tc->kind, (gulong)*val,
	      GIOP_MESSAGE_BUFFER(buf)->message_header.message_size);
#endif

    switch(tc->kind) {
    case CORBA_tk_wchar:
    case CORBA_tk_ushort:
    case CORBA_tk_short:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_SHORT);
	giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(buf), *val, sizeof(CORBA_short));
	*val = ((guchar *)*val) + sizeof(CORBA_short);
	break;
    case CORBA_tk_enum:
    case CORBA_tk_long:
    case CORBA_tk_ulong:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG);
	giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(buf), *val, sizeof(CORBA_long));
	*val = ((guchar *)*val) + sizeof(CORBA_long);
	break;
    case CORBA_tk_float:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_FLOAT);
	giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(buf), *val, sizeof(CORBA_float));
	*val = ((guchar *)*val) + sizeof(CORBA_float);
	break;
    case CORBA_tk_double:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_DOUBLE);
	giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(buf), *val, sizeof(CORBA_double));
	*val = ((guchar *)*val) + sizeof(CORBA_double);
	break;
    case CORBA_tk_boolean:
    case CORBA_tk_char:
    case CORBA_tk_octet:
	giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(buf), *val, sizeof(CORBA_octet));
	*val = ((guchar *)*val) + sizeof(CORBA_octet);
	break;
    case CORBA_tk_any:
	*val = ALIGN_ADDRESS(*val, MAX(ALIGNOF_CORBA_STRUCT, ALIGNOF_CORBA_POINTER));
	ORBit_marshal_any(buf, *val);
	*val = ((guchar *)*val) + sizeof(CORBA_any);
	break;
    case CORBA_tk_TypeCode:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	ORBit_encode_CORBA_TypeCode(*val, buf);
	*val = ((guchar *)*val) + sizeof(CORBA_TypeCode);
	break;
    case CORBA_tk_Principal:
	*val = ALIGN_ADDRESS(*val,
			     MAX(MAX(ALIGNOF_CORBA_LONG, ALIGNOF_CORBA_STRUCT),
				 ALIGNOF_CORBA_POINTER));

	ulval = *(CORBA_unsigned_long *)(*val);
	giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(buf), *val, sizeof(CORBA_unsigned_long));

	giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(buf),
				       *(char**)((char *)*val+sizeof(CORBA_unsigned_long)),
				       ulval);
	*val = ((guchar *)*val) + sizeof(CORBA_Principal);
	break;
    case CORBA_tk_objref:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	ORBit_marshal_object(buf, *(CORBA_Object *)*val);
	*val = ((guchar *)*val) + sizeof(CORBA_Object);
	break;
    case CORBA_tk_except:
    case CORBA_tk_struct:
	*val = ALIGN_ADDRESS(*val, ORBit_find_alignment(tc));
	for(i = 0; i < tc->sub_parts; i++) {
	    ORBit_marshal_value(buf, val, tc->subtypes[i], mi);
	}
	break;
    case CORBA_tk_union:
	/* Basic algorithm:
	   marshal the discriminator
	   find out which value we want to use */
	{
	    CORBA_TypeCode utc;
	    guint max_size = 0;
	    gpointer newval;

	    *val = newval = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_STRUCT);

	    utc = ORBit_get_union_tag(tc, val, FALSE);

	    ORBit_marshal_value(buf, val, tc->discriminator, mi);
/*	    *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_UNION); */

	    *val = ALIGN_ADDRESS(*val, ORBit_find_alignment(tc));
	    max_size = ORBit_gather_alloc_info(tc) - ((guchar *)*val - (guchar *)newval);

	    newval = ((char *)*val) + max_size;
	    ORBit_marshal_value(buf, val, utc, mi);
	    *val = newval;
	}
	break;
    case CORBA_tk_wstring:
	ulval = strlen(*(char **)*val) + 1;

	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	giop_send_buffer_append_mem_indirect_a(buf,
					       &ulval,
					       sizeof(CORBA_unsigned_long));
	giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(buf), *(char **)*val, ulval);
	
	*val = ((guchar *)*val) + sizeof(char *);
	break;
    case CORBA_tk_string:
	ulval = strlen(*(char **)*val) + 1;
	
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);

	giop_send_buffer_append_mem_indirect_a(buf,
					       &ulval,
					       sizeof(CORBA_unsigned_long));
	giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(buf), *(char **)*val, ulval);
	
	*val = ((guchar *)*val) + sizeof(char *);
	break;
    case CORBA_tk_sequence:
	{
	    CORBA_sequence_octet *sval = *val;

	    *val = ALIGN_ADDRESS(*val,
				 MAX(MAX(ALIGNOF_CORBA_LONG, ALIGNOF_CORBA_STRUCT), ALIGNOF_CORBA_POINTER));

	    giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(buf),
					     &sval->_length,
					     sizeof(sval->_length));

	    subval = sval->_buffer;

	    for(i = 0; i < sval->_length; i++)
		ORBit_marshal_value(buf, &subval, tc->subtypes[0], mi);
	    
	    *val = ((guchar *)*val) + sizeof(CORBA_sequence_octet);
	}
	break;
    case CORBA_tk_array:
	submi.alias_element_type = tc->subtypes[0];
	for(i = 0; i < tc->length; i++) {
	    ORBit_marshal_value(buf, val, submi.alias_element_type, &submi);
	    *val = ALIGN_ADDRESS(*val, ORBit_find_alignment(tc->subtypes[0]));
	}
	break;
    case CORBA_tk_alias:
	submi.alias_element_type = tc->subtypes[0];
	ORBit_marshal_value(buf, val, submi.alias_element_type, &submi);
	break;
    case CORBA_tk_longlong:
    case CORBA_tk_ulonglong:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_LONG);
	giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(buf), *val, sizeof(CORBA_long_long));
	return /* *val + sizeof(CORBA_long_long)*/;
	break;
    case CORBA_tk_longdouble:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_DOUBLE);
	giop_message_buffer_append_mem_a(GIOP_MESSAGE_BUFFER(buf), *val, sizeof(CORBA_long_double));
	return /* *val + sizeof(CORBA_long_double)*/;
	break;
    case CORBA_tk_fixed:
	/* XXX todo */
	g_error("CORBA_fixed NYI");
	
	break;
    case CORBA_tk_null:
    case CORBA_tk_void:
	break;
    default:
	g_error("Can't encode unknown type %d", tc->kind);
    }
}

static glong ORBit_get_union_switch(CORBA_TypeCode tc, gpointer *val, gboolean update)
{
    glong retval;

    switch(tc->kind) {
    case CORBA_tk_ulong:
    case CORBA_tk_long:
    case CORBA_tk_enum:
	retval = *(CORBA_long *)*val;
	if(update) *val = ((guchar *)val) + sizeof(CORBA_long);
	break;
    case CORBA_tk_ushort:
    case CORBA_tk_short:
	retval = *(CORBA_short *)*val;
	if(update) *val = ((guchar *)val) + sizeof(CORBA_short);
	break;
    case CORBA_tk_char:
    case CORBA_tk_boolean:
    case CORBA_tk_octet:
	retval = *(CORBA_octet *)*val;
	if(update) *val = ((guchar *)val) + sizeof(CORBA_char);
	break;
    case CORBA_tk_alias:
	return ORBit_get_union_switch(tc->subtypes[0], val, update);
	break;
    default:
	g_error("Wow, some nut has passed us a weird type[%d] as a union discriminator!", tc->kind);
    }

    return retval;
}

/* This function (and the one above it) exist for the
   sole purpose of finding out which CORBA_TypeCode a union discriminator value
   indicates.

   If {update} is TRUE, {*val} will be advanced by the native size
   of the descriminator type.

   Hairy stuff.
*/
CORBA_TypeCode
ORBit_get_union_tag(CORBA_TypeCode union_tc, gpointer *val, gboolean update)
{
    glong discrim_val, case_val;
    int i;
    CORBA_TypeCode retval = CORBA_OBJECT_NIL;

    discrim_val = ORBit_get_union_switch(union_tc->discriminator, val, update);

    for(i = 0; i < union_tc->sub_parts; i++) {
	if(i == union_tc->default_index)
	    continue;

	case_val = ORBit_get_union_switch(union_tc->sublabels[i]._type,
					  &union_tc->sublabels[i]._value, FALSE);
	if(case_val == discrim_val) {
	    retval = union_tc->subtypes[i];
	    break;
	}
    }

    if(retval)
	return retval;
    else if(union_tc->default_index >= 0)
	return union_tc->subtypes[union_tc->default_index];
    else {
	return TC_null;
    }
}

void
ORBit_marshal_arg(GIOPSendBuffer *buf,
		  gpointer val,
		  CORBA_TypeCode tc)
{
    ORBit_marshal_value_info mi;

    ORBit_marshal_value(buf, &val, tc, &mi);
}


void
ORBit_marshal_any(GIOPSendBuffer *buf, const CORBA_any *val)
{
    ORBit_marshal_value_info mi;

    gpointer mval = val->_value;

    ORBit_encode_CORBA_TypeCode(val->_type, buf);

    ORBit_marshal_value(buf, &mval, val->_type, &mi);
}

size_t
ORBit_gather_alloc_info(CORBA_TypeCode tc)
{
    int i, n, align=1, prevalign, sum, prev;
    size_t block_size;

    switch(tc->kind) {
    case CORBA_tk_long:
    case CORBA_tk_ulong:
    case CORBA_tk_enum:
	return sizeof(CORBA_long);
	break;
    case CORBA_tk_short:
    case CORBA_tk_ushort:
	return sizeof(CORBA_short);
	break;
    case CORBA_tk_float:
	return sizeof(CORBA_float);
	break;
    case CORBA_tk_double:
	return sizeof(CORBA_double);
	break;
    case CORBA_tk_boolean:
    case CORBA_tk_char:
    case CORBA_tk_octet:
	return sizeof(CORBA_octet);
	break;
    case CORBA_tk_any:
	return sizeof(CORBA_any);
	break;
    case CORBA_tk_TypeCode:
	return sizeof(CORBA_TypeCode);
	break;
    case CORBA_tk_Principal:
	return sizeof(CORBA_Principal);
	break;
    case CORBA_tk_objref:
	return sizeof(CORBA_Object);
	break;
    case CORBA_tk_except:
    case CORBA_tk_struct:
	sum = 0;
	for(i = 0; i < tc->sub_parts; i++) {
	    sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, ORBit_find_alignment(tc->subtypes[i])));
	    sum += ORBit_gather_alloc_info(tc->subtypes[i]);
	}
	sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, ORBit_find_alignment(tc)));
	return sum;
	break;
    case CORBA_tk_union:
	sum = ORBit_gather_alloc_info(tc->discriminator);
	n = -1;
	align = 1;
	for(prev = prevalign = i = 0; i < tc->sub_parts; i++) {
	    prevalign = align;
	    align = ORBit_find_alignment(tc->subtypes[i]);
	    if(align > prevalign)
		n = i;

	    prev = MAX(prev, ORBit_gather_alloc_info(tc->subtypes[i]));
	}
	if(n >= 0)
	    sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, ORBit_find_alignment(tc->subtypes[n])));
	sum += prev;
	sum = GPOINTER_TO_INT(ALIGN_ADDRESS(sum, ORBit_find_alignment(tc)));
	return sum;
 	break;
    case CORBA_tk_wstring:
    case CORBA_tk_string:
	return sizeof(char *);
	break;
    case CORBA_tk_sequence:
	return sizeof(CORBA_sequence_octet);
	break;
    case CORBA_tk_array:
	block_size = ORBit_gather_alloc_info(tc->subtypes[0]);
	return block_size * tc->length;
	break;
    case CORBA_tk_alias:
	return ORBit_gather_alloc_info(tc->subtypes[0]);
    case CORBA_tk_longlong:
    case CORBA_tk_ulonglong:
	return sizeof(CORBA_long_long);
    case CORBA_tk_longdouble:
	return sizeof(CORBA_long_double);
    case CORBA_tk_wchar:
	return sizeof(CORBA_wchar);
    case CORBA_tk_fixed:
	return sizeof(CORBA_fixed_d_s);
    default:
	return 0;
    }
}

/* to allocate a block, we need to know of any important data
   contained in it.
*/
static gpointer
ORBit_demarshal_allocate_mem(CORBA_TypeCode tc, gint nelements)
{
    size_t block_size;
    gpointer retval = NULL;

    if(!nelements) return retval;

    block_size = ORBit_gather_alloc_info(tc);

    if(block_size) {
	retval = ORBit_alloc_2(block_size * nelements,
			       (ORBit_free_childvals)ORBit_free_via_TypeCode,
			       GINT_TO_POINTER(nelements),
			       sizeof(CORBA_TypeCode));

	*(CORBA_TypeCode *)((char *)retval-sizeof(ORBit_mem_info)-sizeof(CORBA_TypeCode)) = (CORBA_TypeCode)CORBA_Object_duplicate((CORBA_Object)tc, NULL);
    }

    return retval;
}

#define DM_GET_ATOM(x, n) G_STMT_START{ GIOP_RECV_BUFFER(buf)->decoder(x, (GIOP_RECV_BUFFER(buf)->cur), n); GIOP_RECV_BUFFER(buf)->cur = ((guchar *)GIOP_RECV_BUFFER(buf)->cur) + n; }G_STMT_END

static void
ORBit_demarshal_value(GIOPRecvBuffer *buf,
		      gpointer *val,
		      CORBA_TypeCode tc,
		      gboolean dup_strings,
		      CORBA_ORB orb)
{
    CORBA_long i, n;

#if 0
    g_message("Demarshalling a %d value from offset %d into %#x",
	      tc->kind, buf->cur - buf->message_body, (gulong)*val);
#endif

    switch(tc->kind) {
    case CORBA_tk_short:
    case CORBA_tk_ushort:
    case CORBA_tk_wchar:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_SHORT);
	buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_short));
	DM_GET_ATOM(*val, sizeof(CORBA_short));
	*val = ((guchar *)*val) + sizeof(CORBA_short);
	break;
    case CORBA_tk_long:
    case CORBA_tk_ulong:
    case CORBA_tk_enum:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG);
	buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long));
	DM_GET_ATOM(*val, sizeof(CORBA_long));
	*val = ((guchar *)*val) + sizeof(CORBA_long);
	break;
    case CORBA_tk_longlong:
    case CORBA_tk_ulonglong:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_LONG);
	buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long_long));
	DM_GET_ATOM(*val, sizeof(CORBA_long_long));
	*val = ((guchar *)*val) + sizeof(CORBA_long_long);
	break;
    case CORBA_tk_longdouble:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_DOUBLE);
	buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long_double));
	DM_GET_ATOM(*val, sizeof(CORBA_long_double));
	*val = ((guchar *)*val) + sizeof(CORBA_long_double);
	break;
    case CORBA_tk_float:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_FLOAT);
	buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_float));
	DM_GET_ATOM(*val, sizeof(CORBA_float));
	*val = ((guchar *)*val) + sizeof(CORBA_float);
	break;
    case CORBA_tk_double:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_DOUBLE);
	buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_double));
	DM_GET_ATOM(*val, sizeof(CORBA_double));
	*val = ((guchar *)*val) + sizeof(CORBA_double);
	break;
    case CORBA_tk_boolean:
    case CORBA_tk_char:
    case CORBA_tk_octet:
	DM_GET_ATOM(*val, sizeof(CORBA_octet));
	*val = ((guchar *)*val) + sizeof(CORBA_octet);
	break;
    case CORBA_tk_any:
	{
	    CORBA_any *decoded;

	    *val = ALIGN_ADDRESS(*val,
				 MAX(ALIGNOF_CORBA_LONG,
				     MAX(ALIGNOF_CORBA_POINTER, ALIGNOF_CORBA_STRUCT)));
	    decoded = *val;
	    decoded->_release = CORBA_FALSE;
	    ORBit_demarshal_any(buf, decoded, dup_strings, orb);
	    *val = ((guchar *)*val) + sizeof(CORBA_any);
	}
	break;
    case CORBA_tk_TypeCode:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	ORBit_decode_CORBA_TypeCode(*val, buf);
	CORBA_Object_duplicate(*(CORBA_Object *)*val, NULL);
	*val = ((guchar *)*val) + sizeof(CORBA_TypeCode);
	break;
    case CORBA_tk_Principal:
	{
	    CORBA_Principal *p;

	    *val = ALIGN_ADDRESS(*val, MAX(ALIGNOF_CORBA_STRUCT,
					   MAX(ALIGNOF_CORBA_LONG, ALIGNOF_CORBA_POINTER)));

	    p = *val;
	    buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long));
	    CORBA_sequence_set_release(p, dup_strings);
	    DM_GET_ATOM(&p->_length, sizeof(CORBA_long));
	    p->_buffer = ORBit_alloc(p->_length, NULL, GINT_TO_POINTER(1));
	    memcpy(p->_buffer, buf->cur, p->_length);
	    buf->cur = ((guchar *)buf->cur) + p->_length;
	    *val = ((guchar *)*val) + sizeof(CORBA_sequence_octet);
	}
	break;
    case CORBA_tk_objref:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	*(CORBA_Object *)*val = ORBit_demarshal_object(buf, orb);
	*val = ((guchar *)*val) + sizeof(CORBA_Object);
	break;
    case CORBA_tk_except:
    case CORBA_tk_struct:
	*val = ALIGN_ADDRESS(*val, ORBit_find_alignment(tc));
	for(i = 0; i < tc->sub_parts; i++) {
	    ORBit_demarshal_value(buf, val, tc->subtypes[i], dup_strings, orb);
	}
	break;
    case CORBA_tk_union:
	{
	    gpointer discrimptr;

	    discrimptr = *val = ALIGN_ADDRESS(*val, ORBit_find_alignment(tc));
	    ORBit_demarshal_value(buf, val, tc->discriminator, dup_strings, orb);
	    n = 1;
	    for(i = 0; i < tc->sub_parts; i++) {
		n = MAX(n, ORBit_find_alignment(tc->subtypes[i]));
	    }
	    *val = ALIGN_ADDRESS(*val, n);
	    ORBit_demarshal_value(buf, val,
				  ORBit_get_union_tag(tc, &discrimptr, FALSE),
				  dup_strings, orb);
	}
	break;
    case CORBA_tk_string:
    case CORBA_tk_wstring:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long));
	DM_GET_ATOM(&i, sizeof(CORBA_long));
	if(dup_strings)
	    *(char **)*val = CORBA_string_dup(buf->cur);
	else
	    *(char **)*val = buf->cur;
	*val = ((guchar *)*val) + sizeof(CORBA_char *);
	buf->cur = (gpointer)((char *)buf->cur + i);
	break;
    case CORBA_tk_sequence:
	{
	    CORBA_sequence_octet *p;
	    gpointer subval;

	    *val = ALIGN_ADDRESS(*val, MAX(ALIGNOF_CORBA_STRUCT,
					   MAX(ALIGNOF_CORBA_LONG, ALIGNOF_CORBA_POINTER)));
	    p = *val;
	    buf->cur = ALIGN_ADDRESS(buf->cur, sizeof(CORBA_long));
	    DM_GET_ATOM(&p->_length, sizeof(CORBA_long));
	    if(tc->subtypes[0]->kind == CORBA_tk_octet
	       || tc->subtypes[0]->kind == CORBA_tk_boolean
	       || tc->subtypes[0]->kind == CORBA_tk_char) {
		/* This special-casing could be taken further to apply to
		   all atoms... */
		p->_buffer = ORBit_alloc(p->_length, NULL, GINT_TO_POINTER(1));
		memcpy(p->_buffer, buf->cur, p->_length);
		buf->cur = ((guchar *)buf->cur) + p->_length;
	    } else {
		p->_buffer = ORBit_demarshal_allocate_mem(tc->subtypes[0],
							  p->_length);
		subval = p->_buffer;

		for(i = 0; i < p->_length; i++)
		    ORBit_demarshal_value(buf, &subval,
					  tc->subtypes[0],
					  dup_strings,
					  orb);
	    }

	    *val = ((guchar *)*val) + sizeof(CORBA_sequence_octet);
	}
	break;
    case CORBA_tk_array:
	for(i = 0; i < tc->length; i++)
	    ORBit_demarshal_value(buf, val, tc->subtypes[0], dup_strings, orb);
	break;
    case CORBA_tk_alias:
	ORBit_demarshal_value(buf, val, tc->subtypes[0], dup_strings, orb);
	break;
    case CORBA_tk_fixed:
	g_error("CORBA_fixed NYI");
	break;
    default:
        break;
    }
}

gpointer
ORBit_demarshal_arg(GIOPRecvBuffer *buf,
		    CORBA_TypeCode tc,
		    gboolean dup_strings,
		    CORBA_ORB orb)
{
    gpointer retval, val;

    retval = val = ORBit_demarshal_allocate_mem(tc, 1);

    ORBit_demarshal_value(buf, &val, tc, dup_strings, orb);

    return retval;
}

void
ORBit_demarshal_any(GIOPRecvBuffer *buf, CORBA_any *retval,
		    gboolean dup_strings,
		    CORBA_ORB orb)
{
    gpointer val;

#if 0
    /* I wish I knew whether this was correct or not. It breaks things like 'any anop();' for sure,
       since we can't always initialize every single possible 'any' underneath _ORBIT_retval */
    if(retval->_release)
	CORBA_free(retval->_value);
#endif

    CORBA_any_set_release(retval, CORBA_TRUE);

    ORBit_decode_CORBA_TypeCode(&retval->_type, buf);

    val = retval->_value = ORBit_demarshal_allocate_mem(retval->_type, 1);
    ORBit_demarshal_value(buf, &val, retval->_type, dup_strings, orb);
}

void
_ORBit_copy_value(gpointer *val, gpointer *newval, CORBA_TypeCode tc)
{
    CORBA_long i;
    gpointer pval1, pval2;

    switch(tc->kind) {
    case CORBA_tk_wchar:
    case CORBA_tk_short:
    case CORBA_tk_ushort:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_SHORT);
	*newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_SHORT);
	*(CORBA_short *)*newval = *(CORBA_short *)*val;
	*val = ((guchar *)*val) + sizeof(CORBA_short);
	*newval = ((guchar *)*newval) + sizeof(CORBA_short);
	break;
    case CORBA_tk_enum:
    case CORBA_tk_long:
    case CORBA_tk_ulong:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG);
	*newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_LONG);
	*(CORBA_long *)*newval = *(CORBA_long *)*val;
	*val = ((guchar *)*val) + sizeof(CORBA_long);
	*newval = ((guchar *)*newval) + sizeof(CORBA_long);
	break;
    case CORBA_tk_longlong:
    case CORBA_tk_ulonglong:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_LONG);
	*newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_LONG_LONG);
	*(CORBA_long_long *)*newval = *(CORBA_long_long *)*val;
	*val = ((guchar *)*val) + sizeof(CORBA_long_long);
	*newval = ((guchar *)*newval) + sizeof(CORBA_long_long);
	break;
    case CORBA_tk_longdouble:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_LONG_DOUBLE);
	*newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_LONG_DOUBLE);
	*(CORBA_long_double *)*newval = *(CORBA_long_double *)*val;
	*val = ((guchar *)*val) + sizeof(CORBA_long_double);
	*newval = ((guchar *)*newval) + sizeof(CORBA_long_double);
	break;
    case CORBA_tk_float:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_FLOAT);
	*newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_FLOAT);
	*(CORBA_long *)*newval = *(CORBA_long *)*val;
	*val = ((guchar *)*val) + sizeof(CORBA_float);
	*newval = ((guchar *)*newval) + sizeof(CORBA_float);
	break;
    case CORBA_tk_double:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_DOUBLE);
	*newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_DOUBLE);
	*(CORBA_double *)*newval = *(CORBA_double *)*val;
	*val = ((guchar *)*val) + sizeof(CORBA_double);
	*newval = ((guchar *)*newval) + sizeof(CORBA_double);
	break;
    case CORBA_tk_boolean:
    case CORBA_tk_char:
    case CORBA_tk_octet:
	*(CORBA_octet *)*newval = *(CORBA_octet *)*val;
	*val = ((guchar *)*val) + sizeof(CORBA_octet);
	*newval = ((guchar *)*newval) + sizeof(CORBA_octet);
	break;
    case CORBA_tk_any:
	{
	    CORBA_any *oldany, *newany;
	    *val = ALIGN_ADDRESS(*val, MAX(ALIGNOF_CORBA_STRUCT, ALIGNOF_CORBA_POINTER));
	    *newval = ALIGN_ADDRESS(*newval, MAX(ALIGNOF_CORBA_STRUCT, ALIGNOF_CORBA_POINTER));
	    oldany = *val;
	    newany = *newval;
	    newany->_type = (CORBA_TypeCode)CORBA_Object_duplicate((CORBA_Object)oldany->_type, NULL);
	    /* XXX are we supposed to do this even if oldany->_release
               == FALSE? */
	    newany->_value = ORBit_copy_value(oldany->_value, oldany->_type);
	    newany->_release = CORBA_TRUE;
	    *val = ((guchar *)*val) + sizeof(CORBA_any);
	    *newval = ((guchar *)*newval) + sizeof(CORBA_any);
	}
	break;
    case CORBA_tk_Principal:
	*val = ALIGN_ADDRESS(*val,
			     MAX(MAX(ALIGNOF_CORBA_LONG,
				     ALIGNOF_CORBA_STRUCT),
				 ALIGNOF_CORBA_POINTER));
	*newval = ALIGN_ADDRESS(*newval,
				MAX(MAX(ALIGNOF_CORBA_LONG,
					ALIGNOF_CORBA_STRUCT),
				    ALIGNOF_CORBA_POINTER));
	*(CORBA_Principal *)*newval = *(CORBA_Principal *)*val;
	((CORBA_Principal *)*newval)->_buffer =
	    CORBA_octet_allocbuf(((CORBA_Principal *)*newval)->_length);
	memcpy(((CORBA_Principal *)*newval)->_buffer,
	       ((CORBA_Principal *)*val)->_buffer,
	       ((CORBA_Principal *)*val)->_length);
	*val = ((guchar *)*val) + sizeof(CORBA_Principal);
	*newval = ((guchar *)*newval) + sizeof(CORBA_Principal);
	break;
    case CORBA_tk_TypeCode:
    case CORBA_tk_objref:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	*newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_POINTER);
	*(CORBA_Object *)*newval = CORBA_Object_duplicate(*(CORBA_Object *)*val,
							  NULL);
	*val = ((guchar *)*val) + sizeof(CORBA_Object);
	*newval = ((guchar *)*newval) + sizeof(CORBA_Object);
	break;
    case CORBA_tk_struct:
    case CORBA_tk_except:
	*val = ALIGN_ADDRESS(*val, ORBit_find_alignment(tc));
	*newval = ALIGN_ADDRESS(*newval, ORBit_find_alignment(tc));
	for(i = 0; i < tc->sub_parts; i++) {
	    _ORBit_copy_value(val, newval, tc->subtypes[i]);
	}
	break;
    case CORBA_tk_union:
	{
	    CORBA_TypeCode utc = ORBit_get_union_tag(tc, val, FALSE);
	    gint	union_align = ORBit_find_alignment(tc);
	    size_t	union_size = ORBit_gather_alloc_info(tc);

	    /* need to advance val,newval by size of union, not just
	     * current tagged field within it */
	    pval1 = *val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_STRUCT);
	    pval2 = *newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_STRUCT);
	    _ORBit_copy_value(&pval1, &pval2, tc->discriminator);
	    pval1 = ALIGN_ADDRESS(pval1, union_align);
	    pval2 = ALIGN_ADDRESS(pval2, union_align);
	    _ORBit_copy_value(&pval1, &pval2, utc);
	    *val = ((guchar *)*val) + union_size;
	    *newval = ((guchar *)*newval) + union_size;
	}
	break;
    case CORBA_tk_wstring:
    case CORBA_tk_string:
	*val = ALIGN_ADDRESS(*val, ALIGNOF_CORBA_POINTER);
	*newval = ALIGN_ADDRESS(*newval, ALIGNOF_CORBA_POINTER);
	
	*(CORBA_char **)*newval = CORBA_string_dup(*(CORBA_char **)*val);
	*val = ((guchar *)*val) + sizeof(CORBA_char *);
	*newval = ((guchar *)*newval) + sizeof(CORBA_char *);
	break;
    case CORBA_tk_sequence:
	*val = ALIGN_ADDRESS(*val,
			     MAX(MAX(ALIGNOF_CORBA_LONG,
				     ALIGNOF_CORBA_STRUCT),
				 ALIGNOF_CORBA_POINTER));
	*newval = ALIGN_ADDRESS(*newval,
				MAX(MAX(ALIGNOF_CORBA_LONG,
					ALIGNOF_CORBA_STRUCT),
				    ALIGNOF_CORBA_POINTER));
	((CORBA_Principal *)*newval)->_release = CORBA_TRUE;
	((CORBA_Principal *)*newval)->_length =
	     ((CORBA_Principal *)*newval)->_maximum =
					 ((CORBA_Principal *)*val)->_length;
	((CORBA_Principal *)*newval)->_buffer = pval2 =
	    ORBit_demarshal_allocate_mem(tc->subtypes[0],
					 ((CORBA_Principal *)*val)->_length);
	pval1 = ((CORBA_Principal *)*val)->_buffer;
	
	for(i = 0; i < ((CORBA_Principal *)*newval)->_length; i++) {
	    _ORBit_copy_value(&pval1, &pval2, tc->subtypes[0]);
	}
	*val = ((guchar *)*val) + sizeof(CORBA_sequence_octet);
	*newval = ((guchar *)*newval) + sizeof(CORBA_sequence_octet);
	break;
    case CORBA_tk_array:
	for(i = 0; i < tc->length; i++) {
	    _ORBit_copy_value(val, newval, tc->subtypes[0]);
	}
	break;
    case CORBA_tk_alias:
	_ORBit_copy_value(val, newval, tc->subtypes[0]);
	break;
    case CORBA_tk_fixed:
	g_error("CORBA_fixed NYI!");
	break;
    case CORBA_tk_void:
    case CORBA_tk_null:
	*val = NULL;
	break;
    default:
	g_error("Can't handle copy of value kind %d", tc->kind);
    }
}

gpointer
ORBit_copy_value(gpointer value, CORBA_TypeCode tc)
{
    gpointer retval, newval;

    retval = newval = ORBit_demarshal_allocate_mem(tc, 1);
    _ORBit_copy_value(&value, &newval, tc);

    return retval;
}

void
CORBA_any__copy(CORBA_any *out, CORBA_any *in)
{
    out->_type = (CORBA_TypeCode)CORBA_Object_duplicate((CORBA_Object)in->_type,
							NULL);
    out->_value = ORBit_copy_value(in->_value, in->_type);
    out->_release = CORBA_TRUE;
}
