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

#include "config.h"
#include "../IIOP/iiop-endianP.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "orbit.h"

#define CDR_GROW_AMOUNT 128

static CORBA_boolean CDR_buffer_grow(CDR_Codec *codec, const unsigned int growth)
{
	unsigned int real_growth;
	div_t divvy;

	if(codec->release_buffer) {
		divvy=div(growth, CDR_GROW_AMOUNT);
		real_growth=CDR_GROW_AMOUNT * (divvy.quot+1);
		
		codec->buffer=(CORBA_octet *)g_realloc(codec->buffer,
						       codec->buf_len
						       +real_growth);
	}

	return CORBA_TRUE;
}

static void CDR_buffer_puts(CDR_Codec *codec, const void *data, const unsigned int len)
{
	if(codec->wptr+len > codec->buf_len) {
		CORBA_boolean res=CDR_buffer_grow(codec, len);

		if(res==CORBA_FALSE) {
			/* just bail out for now */
			g_assert(!"Malloc error");
		}
	}

	memcpy(&codec->buffer[codec->wptr], data, len);
	codec->wptr+=len;
}

CORBA_boolean CDR_buffer_gets(CDR_Codec *codec, void *dest, const unsigned int len)
{
	if(codec->rptr+len > codec->buf_len) {
		ORBit_Trace(TraceMod_CDR, TraceLevel_Debug, "CDR_buffer_gets: attempt to read past end of buffer\n");
		return(CORBA_FALSE);
	}

	memcpy(dest, &codec->buffer[codec->rptr], len);
	codec->rptr+=len;

	return(CORBA_TRUE);
}

static void CDR_buffer_put(CDR_Codec *codec, void *datum)
{
	if(codec->wptr+1 > codec->buf_len) {
		CORBA_boolean res=CDR_buffer_grow(codec, 1);

		if(res==CORBA_FALSE) {
			/* just bail out for now */
			g_assert(!"Malloc error");
		}
	}

	codec->buffer[codec->wptr++]=*(unsigned char *)datum;
}

static CORBA_boolean CDR_buffer_get(CDR_Codec *codec, void *dest)
{
	if(codec->rptr+1 > codec->buf_len) {
		ORBit_Trace(TraceMod_CDR, TraceLevel_Debug, "CDR_buffer_get: attempt to read past end of buffer\n");
		return(CORBA_FALSE);
	}

	*(CORBA_octet *)dest=codec->buffer[codec->rptr++];
	return(CORBA_TRUE);
}

#ifdef lame_slow_code
static void CDR_buffer_put2(CDR_Codec *codec, void *datum)
{
	unsigned long align;

	g_assert(codec!=NULL);
	g_assert(codec->readonly!=CORBA_TRUE);
	g_assert(codec->wptr<=codec->buf_len);

	align=((codec->wptr+1)&~1L);

	if(align+2 > codec->buf_len) {
		CORBA_boolean res=CDR_buffer_grow(codec, align+2-codec->wptr);

		if(res==CORBA_FALSE) {
			/* just bail out for now */
			g_assert(!"Malloc error");
		}
	}

	while(codec->wptr < align) {
		codec->buffer[codec->wptr++]='\0';
	}

	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[0];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[1];
}

static CORBA_boolean CDR_buffer_get2(CDR_Codec *codec, void *dest)
{
	unsigned long align;

	g_assert(codec!=NULL);
	g_assert(dest!=NULL);
	g_assert(codec->rptr<=codec->buf_len);

	align=((codec->rptr+1)&~1L);

	if(align+2 > codec->buf_len) {
		ORBit_Trace(TraceMod_CDR, TraceLevel_Debug, "CDR_buffer_get2: attempt to read past end of buffer\n");
		return(CORBA_FALSE);
	}

	codec->rptr=align;

	((CORBA_octet *)dest)[0]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[1]=codec->buffer[codec->rptr++];

	return(CORBA_TRUE);
}

static void CDR_buffer_put4(CDR_Codec *codec, void *datum)
{
	unsigned long align;

	g_assert(codec!=NULL);
	g_assert(codec->readonly!=CORBA_TRUE);
	g_assert(codec->wptr<=codec->buf_len);

	align=((codec->wptr+3)&~3L);

	if(align+4 > codec->buf_len) {
		CORBA_boolean res=CDR_buffer_grow(codec, align+4-codec->wptr);

		if(res==CORBA_FALSE) {
			/* just bail out for now */
			g_assert(!"Malloc error");
		}
	}

	while(codec->wptr < align) {
		codec->buffer[codec->wptr++]='\0';
	}

	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[0];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[1];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[2];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[3];
}

static CORBA_boolean CDR_buffer_get4(CDR_Codec *codec, void *dest)
{
	unsigned long align;

	g_assert(codec!=NULL);
	g_assert(dest!=NULL);
	g_assert(codec->rptr<=codec->buf_len);

	align=((codec->rptr+3)&~3L);

	if(align+4 > codec->buf_len) {
		ORBit_Trace(TraceMod_CDR, TraceLevel_Debug, "CDR_buffer_get4: attempt to read past end of buffer\n");
		return(CORBA_FALSE);
	}

	codec->rptr=align;

	((CORBA_octet *)dest)[0]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[1]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[2]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[3]=codec->buffer[codec->rptr++];

	return(CORBA_TRUE);
}

static void CDR_buffer_put8(CDR_Codec *codec, void *datum)
{
	unsigned long align;

	g_assert(codec!=NULL);
	g_assert(codec->readonly!=CORBA_TRUE);
	g_assert(codec->wptr<=codec->buf_len);

	align=((codec->wptr+7)&~7L);

	if(align+8 > codec->buf_len) {
		CORBA_boolean res=CDR_buffer_grow(codec, align+8-codec->wptr);

		if(res==CORBA_FALSE) {
			/* just bail out for now */
			g_assert(!"Malloc error");
		}
	}

	while(codec->wptr < align) {
		codec->buffer[codec->wptr++]='\0';
	}

	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[0];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[1];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[2];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[3];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[4];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[5];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[6];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[7];
}

#if 0
static CORBA_boolean CDR_buffer_get8(CDR_Codec *codec, void *dest)
{
	unsigned long align;

	g_assert(codec!=NULL);
	g_assert(dest!=NULL);
	g_assert(codec->rptr<=codec->buf_len);

	align=((codec->rptr+7)&~7L);

	if(align+8 > codec->buf_len) {
		ORBit_Trace(TraceMod_CDR, TraceLevel_Debug, "CDR_buffer_get8: attempt to read past end of buffer\n");
		return(CORBA_FALSE);
	}

	codec->rptr=align;

	((CORBA_octet *)dest)[0]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[1]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[2]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[3]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[4]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[5]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[6]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[7]=codec->buffer[codec->rptr++];

	return(CORBA_TRUE);
}
#endif

static void CDR_buffer_put16(CDR_Codec *codec, void *datum)
{
	unsigned long align;

	g_assert(codec!=NULL);
	g_assert(codec->readonly!=CORBA_TRUE);
	g_assert(codec->wptr<=codec->buf_len);

	align=((codec->wptr+15)&~15L);

	if(align+16 > codec->buf_len) {
		CORBA_boolean res=CDR_buffer_grow(codec, align+16-codec->wptr);

		if(res==CORBA_FALSE) {
			/* just bail out for now */
			g_assert(!"Malloc error");
		}
	}

	while(codec->wptr < align) {
		codec->buffer[codec->wptr++]='\0';
	}

	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[0];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[1];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[2];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[3];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[4];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[5];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[6];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[7];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[8];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[9];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[10];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[11];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[12];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[13];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[14];
	codec->buffer[codec->wptr++]=((CORBA_octet *)datum)[15];
}

#if 0
static CORBA_boolean CDR_buffer_get16(CDR_Codec *codec, void *dest)
{
	unsigned long align;

	g_assert(codec!=NULL);
	g_assert(dest!=NULL);
	g_assert(codec->rptr<=codec->buf_len);

	align=((codec->rptr+15)&~15L);

	if(align+16 > codec->buf_len) {
		ORBit_Trace(TraceMod_CDR, TraceLevel_Debug, "CDR_buffer_get16: attempt to read past end of buffer\n");
		return(CORBA_FALSE);
	}

	codec->rptr=align;

	((CORBA_octet *)dest)[0]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[1]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[2]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[3]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[4]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[5]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[6]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[7]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[8]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[9]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[10]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[11]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[12]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[13]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[14]=codec->buffer[codec->rptr++];
	((CORBA_octet *)dest)[15]=codec->buffer[codec->rptr++];

	return(CORBA_TRUE);
}
#endif
#endif /* lame_slow_code */

#define CDR_buffer_put2(codec, datum) CDR_buffer_putn(codec, datum, 2)
#define CDR_buffer_put4(codec, datum) CDR_buffer_putn(codec, datum, 4)
#define CDR_buffer_put8(codec, datum) CDR_buffer_putn(codec, datum, 8)
#define CDR_buffer_put16(codec, datum) CDR_buffer_putn(codec, datum, 16)
#define CDR_buffer_get2(codec, dest) CDR_buffer_getn(codec, dest, 2)
#define CDR_buffer_get4(codec, dest) CDR_buffer_getn(codec, dest, 4)
#define CDR_buffer_get8(codec, dest) CDR_buffer_getn(codec, dest, 8)
#define CDR_buffer_get16(codec, dest) CDR_buffer_getn(codec, dest, 16)

static CORBA_boolean
CDR_buffer_getn(CDR_Codec *codec, void *dest, int bsize)
{
	codec->rptr = (unsigned long)ALIGN_ADDRESS(codec->rptr, bsize);
	if(codec->host_endian==codec->data_endian)
		memcpy(dest, codec->buffer + codec->rptr, bsize);
	else
		iiop_byteswap(dest, codec->buffer + codec->rptr, bsize);
	codec->rptr += bsize;

	return CORBA_TRUE;
}

static CORBA_boolean
CDR_buffer_putn(CDR_Codec *codec, void *datum, int bsize)
{
#ifndef I_DONT_FEEL_LIKE_INITIALISING_MY_MEMORY
        gulong forward = (gulong)ALIGN_ADDRESS(codec->wptr, bsize),
	  i = codec->wptr;
        while(forward > i)
	        codec->buffer[i++] = '\0';
#endif

	codec->wptr = (unsigned long)ALIGN_ADDRESS(codec->wptr, bsize);
	if(codec->host_endian==codec->data_endian)
		memcpy(codec->buffer + codec->wptr, datum, bsize);
	else
		iiop_byteswap(codec->buffer + codec->wptr, datum, bsize);
	codec->wptr += bsize;

	return CORBA_TRUE;
}

#define CDR_swap2(d,s) iiop_byteswap((d), (s), 2)
#define CDR_swap4(d,s) iiop_byteswap((d), (s), 4)
#define CDR_swap8(d,s) iiop_byteswap((d), (s), 8)
#define CDR_swap16(d,s) iiop_byteswap((d), (s), 16)

#ifdef lame_slow_code
static void CDR_swap2(void *d, void *s)
{
	((CORBA_octet *)d)[0]=((CORBA_octet *)s)[1];
	((CORBA_octet *)d)[1]=((CORBA_octet *)s)[0];
}

static void CDR_swap4(void *d, void *s)
{
	((CORBA_octet *)d)[0]=((CORBA_octet *)s)[3];
	((CORBA_octet *)d)[1]=((CORBA_octet *)s)[2];
	((CORBA_octet *)d)[2]=((CORBA_octet *)s)[1];
	((CORBA_octet *)d)[3]=((CORBA_octet *)s)[0];
}

static void CDR_swap8(void *d, void *s)
{
	((CORBA_octet *)d)[0]=((CORBA_octet *)s)[7];
	((CORBA_octet *)d)[1]=((CORBA_octet *)s)[6];
	((CORBA_octet *)d)[2]=((CORBA_octet *)s)[5];
	((CORBA_octet *)d)[3]=((CORBA_octet *)s)[4];
	((CORBA_octet *)d)[4]=((CORBA_octet *)s)[3];
	((CORBA_octet *)d)[5]=((CORBA_octet *)s)[2];
	((CORBA_octet *)d)[6]=((CORBA_octet *)s)[1];
	((CORBA_octet *)d)[7]=((CORBA_octet *)s)[0];
}

static void CDR_swap16(void *d, void *s)
{
	((CORBA_octet *)d)[0]=((CORBA_octet *)s)[15];
	((CORBA_octet *)d)[1]=((CORBA_octet *)s)[14];
	((CORBA_octet *)d)[2]=((CORBA_octet *)s)[13];
	((CORBA_octet *)d)[3]=((CORBA_octet *)s)[12];
	((CORBA_octet *)d)[4]=((CORBA_octet *)s)[11];
	((CORBA_octet *)d)[5]=((CORBA_octet *)s)[10];
	((CORBA_octet *)d)[6]=((CORBA_octet *)s)[9];
	((CORBA_octet *)d)[7]=((CORBA_octet *)s)[8];
	((CORBA_octet *)d)[8]=((CORBA_octet *)s)[7];
	((CORBA_octet *)d)[9]=((CORBA_octet *)s)[6];
	((CORBA_octet *)d)[10]=((CORBA_octet *)s)[5];
	((CORBA_octet *)d)[11]=((CORBA_octet *)s)[4];
	((CORBA_octet *)d)[12]=((CORBA_octet *)s)[3];
	((CORBA_octet *)d)[13]=((CORBA_octet *)s)[2];
	((CORBA_octet *)d)[14]=((CORBA_octet *)s)[1];
	((CORBA_octet *)d)[15]=((CORBA_octet *)s)[0];
}
#endif


void CDR_put_short(CDR_Codec *codec, CORBA_short s)
{
	CDR_buffer_put2(codec, &s);
}

CORBA_boolean CDR_get_short(CDR_Codec *codec, CORBA_short *s)
{
	return CDR_buffer_get2(codec, s);
}

void CDR_put_ushort(CDR_Codec *codec, CORBA_unsigned_short us)
{
	CDR_buffer_put2(codec, &us);
}

CORBA_boolean CDR_get_ushort(CDR_Codec *codec, CORBA_unsigned_short *us)
{
	return CDR_buffer_get2(codec, us);
}

void CDR_put_long(CDR_Codec *codec, CORBA_long l)
{
	CDR_buffer_put4(codec, &l);
}

CORBA_boolean CDR_get_long(CDR_Codec *codec, CORBA_long *l)
{
	return CDR_buffer_get4(codec, l);
}

void CDR_put_ulong(CDR_Codec *codec, CORBA_unsigned_long ul)
{
	CDR_buffer_put4(codec, &ul);
}

CORBA_boolean CDR_get_ulong(CDR_Codec *codec, CORBA_unsigned_long *ul)
{
	return CDR_buffer_get4(codec, ul);
}

#ifdef HAVE_CORBA_LONG_LONG
CORBA_boolean CDR_get_long_long(CDR_Codec *codec, CORBA_long_long *ul)
{
	return CDR_buffer_get8(codec, ul);
}

void CDR_put_long_long(CDR_Codec *codec, CORBA_long_long ll)
{
	CDR_buffer_put8(codec, &ll);
}

void CDR_put_ulong_long(CDR_Codec *codec, CORBA_unsigned_long_long ll)
{
	CDR_buffer_put8(codec, &ll);
}

CORBA_boolean CDR_get_ulong_long(CDR_Codec *codec, CORBA_unsigned_long_long *ull)
{
	return CDR_buffer_get8(codec, ull);
}
#endif

void CDR_put_float(CDR_Codec *codec, CORBA_float f)
{
	CDR_buffer_put4(codec, &f);
}

void CDR_put_double(CDR_Codec *codec, CORBA_double d)
{
	CDR_buffer_put8(codec, &d);
}

void CDR_put_long_double(CDR_Codec *codec, CORBA_long_double ld)
{
	CDR_buffer_put16(codec, &ld);
}

void CDR_put_octet(CDR_Codec *codec, CORBA_octet datum)
{
	CDR_buffer_put(codec, &datum);
}

CORBA_boolean CDR_get_octet(CDR_Codec *codec, CORBA_octet *datum)
{
	return(CDR_buffer_get(codec, datum));
}

void CDR_put_octets(CDR_Codec *codec, void *data, unsigned long len)
{
	CDR_buffer_puts(codec, data, len);
}

void CDR_put_char(CDR_Codec *codec, CORBA_char c)
{
	CDR_buffer_put(codec, &c);
}

CORBA_boolean CDR_get_char(CDR_Codec *codec, CORBA_char *c)
{
	return CDR_buffer_get(codec, c);
}

void CDR_put_boolean(CDR_Codec *codec, CORBA_boolean datum)
{
	datum = datum&&1;
	CDR_buffer_put(codec, &datum);
}

CORBA_boolean CDR_get_boolean(CDR_Codec *codec, CORBA_boolean *b)
{
	return CDR_buffer_get(codec, b);
}

void CDR_put_string(CDR_Codec *codec, const char *str)
{
	unsigned int len;

	len=strlen(str)+1;

	CDR_put_ulong(codec, len);
	CDR_buffer_puts(codec, str, len);
}

CORBA_boolean CDR_get_string_static(CDR_Codec *codec,
				    CORBA_char **str)
{
	CORBA_unsigned_long len;

	if(CDR_get_ulong(codec, &len)==CORBA_FALSE)
		return CORBA_FALSE;

	if((codec->rptr + len) > codec->buf_len)
		return CORBA_FALSE;

	*str = ((CORBA_char *)codec->buffer) + codec->rptr;

	codec->rptr += len;

	return CORBA_TRUE;
}

CORBA_boolean CDR_get_string(CDR_Codec *codec, CORBA_char **str)
{
	CORBA_unsigned_long len;

	if(CDR_get_ulong(codec, &len)==CORBA_FALSE)
		return(CORBA_FALSE);

	if(len==0)
		return(CORBA_FALSE);

	*str=g_new(CORBA_char, len);

	if(CDR_buffer_gets(codec, *str, len)==CORBA_FALSE) {
		g_free(*str);
		return(CORBA_FALSE);
	}

	if((*str)[len-1]!='\0') {
		ORBit_Trace(TraceMod_CDR, TraceLevel_Notice, "CDR_get_string: string was not NULL-terminated, terminating it now\n");
		(*str)[len-1]='\0';
	}

	return(CORBA_TRUE);
}

CORBA_boolean CDR_get_seq_begin(CDR_Codec *codec, CORBA_unsigned_long *ul)
{
	return(CDR_get_ulong(codec, ul));
}

CDR_Codec *CDR_codec_init_static(CDR_Codec *codec)
{
	memset(codec, 0, sizeof(CDR_Codec));

	codec->host_endian = FLAG_ENDIANNESS;

	return codec;
}

CDR_Codec *CDR_codec_init(void)
{
	CDR_Codec *new;

	new=g_new0(CDR_Codec, 1);
	CDR_codec_init_static(new);
	new->release_buffer = CORBA_TRUE;

	return(new);
}

void CDR_codec_free(CDR_Codec *codec)
{
	if(codec->release_buffer)
		g_free(codec->buffer);

	g_free(codec);
}
