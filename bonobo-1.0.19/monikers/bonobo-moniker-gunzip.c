/*
 * bonobo-moniker-gunzip.c: gunzip based Moniker
 *
 * Author:
 *   Joe Shaw (joe@helixcode.com)
 *
 * Copyright (c) 2000 Helix Code, Inc.
 */
#include <config.h>
#include <zlib.h>
#include <gnome.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>
#include <bonobo/bonobo-stream-memory.h>
#include <bonobo/bonobo-moniker-extender.h>

#include "bonobo-moniker-gunzip.h"

/* Count number of bytes to skip at start of buf */
static int gz_magic [2] = {0x1f, 0x8b};
/* gzip flag byte */
#define GZ_ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define GZ_HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define GZ_EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define GZ_ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define GZ_COMMENT      0x10 /* bit 4 set: file comment present */
#define GZ_RESERVED     0xE0 /* bits 5..7: reserved */

static int
count_gzip_header (guint8 *buf, guint32 input_length)
{
	int method, flags;
	guint8 *s = buf;
	guint32 left_len = input_length;
	
	if (left_len < 4)
		return -1;

	if (*s++ != gz_magic [0] || *s++ != gz_magic[1])
		return -2;
	
	method = *s++;
	flags = *s++;
	left_len -= 4;
	
	if (method != Z_DEFLATED || (flags & GZ_RESERVED) != 0) {
		/* If it's not deflated, or the reserved isn't 0 */
		return -3;
	}
	
	/* Skip time, xflags, OS code */
	if (left_len < 6)
		return -4;
	s += 6;
	left_len -= 6;
	
	if (flags & GZ_EXTRA_FIELD) {
		unsigned int len;
		if (left_len < 2)
			return -5;
		len = (unsigned int) (*s++);
		len += ((unsigned int) (*s++)) << 8;
		if (left_len < len)
			return -6;
		s += len;
		left_len -= len;
	}
	
	/* Skip filename */
	if (flags & GZ_ORIG_NAME) {
		while (--left_len != 0 && *s++ != '\0') ;
		if (left_len == 0)
			return -7;
	}
	/* Skip comment */
	if (flags & GZ_COMMENT) {
		while (--left_len != 0 && *s++ != '\0') ;
		if (left_len == 0)
			return -7;
	}
	/* Skip CRC */
	if (flags & GZ_HEAD_CRC) {
		if (left_len < 2)
			return -7;
		s += 2;
		left_len -= 2;
	}
	
	return input_length - left_len;
}

static char *
uncompress_memory (char *input_buffer, int size, int *out_size)
{
	z_stream zs;
	gchar *outbuf = NULL;
	GByteArray *data;
	int data_len = 0;
	int zret;
	int gzip_hdr;

	g_return_val_if_fail(input_buffer, NULL);

	data = g_byte_array_new ();

	gzip_hdr = count_gzip_header (input_buffer, size);
	if (gzip_hdr < 0) {
		/* This is not a gzipped stream. Just return the stuff */
		g_byte_array_free(data, FALSE);
		outbuf = g_memdup(input_buffer, size);
		if (out_size)
			*out_size = size;
		return outbuf;
	}

	zs.next_in = input_buffer + gzip_hdr;
	zs.avail_in = size - gzip_hdr;
	zs.zalloc = NULL;
	zs.zfree = NULL;
	zs.opaque = NULL;
	
	outbuf = g_malloc (10000);
	zs.next_out = outbuf;
	zs.avail_out = 10000;

	/* Negative inflateinit is magic to tell zlib that there is no
	 * zlib header */
	inflateInit2 (&zs, -MAX_WBITS);

	while (1) {
		zret = inflate (&zs, Z_SYNC_FLUSH);
		if (zret != Z_OK && zret != Z_STREAM_END)
			break;

		g_byte_array_append(data, outbuf, 10000 - zs.avail_out);
		data_len += 10000 - zs.avail_out;
		zs.next_out = outbuf;
		zs.avail_out = 10000;

		if (zret == Z_STREAM_END)
			break;
	}
	
	if (zret != Z_STREAM_END)
		g_warning ("libz inflate failed! (%d)", zret);
	
	inflateEnd (&zs);

	g_free (outbuf);

	outbuf = data->data;
	if (out_size)
		*out_size = data->len;
	g_byte_array_free(data, FALSE);

	return outbuf;
}

static Bonobo_Stream
gunzip_resolve_stream (BonoboMoniker               *moniker,
		       const Bonobo_ResolveOptions *options,
		       CORBA_Environment           *ev)
{
	Bonobo_Moniker parent;
	Bonobo_Stream in_stream;
	BonoboStream *memstream;
	Bonobo_Stream out_stream;
	int len;
	Bonobo_Stream_iobuf *buffer;
	gboolean cont;
	char *ob;
	GByteArray *data;

	parent = bonobo_moniker_get_parent (moniker, ev);

	if (BONOBO_EX (ev) || parent == CORBA_OBJECT_NIL)
		return CORBA_OBJECT_NIL;

	in_stream = Bonobo_Moniker_resolve (
		parent, options, "IDL:Bonobo/Stream:1.0", ev);
		
	if (BONOBO_EX (ev))
		goto return_unref_parent;
		
	if (in_stream == CORBA_OBJECT_NIL) {
		g_warning ("Failed to obtain a stream from the parent");
		goto return_unref_parent;
	}
		
	/* This is necessary because while there is a 
	   bonobo_stream_client_read_string, there isn't any
	   bonobo_stream_client_read, and this compressed data isn't a
	   string. */
	data = g_byte_array_new ();
	do {
		Bonobo_Stream_read (in_stream, 1, &buffer, ev);
			
		if (BONOBO_EX (ev)) {
			g_byte_array_free (data, TRUE);
			goto return_unref_stream;
		}
		cont = buffer->_length ? TRUE : FALSE;
		if (buffer->_length) {
			data = g_byte_array_append (
				data, buffer->_buffer, buffer->_length);
		}
		CORBA_free (buffer);
	} while (cont);
		
	/* This function will return a memdup of the data we pass to it if it
	   can't decode the header. */
	ob = uncompress_memory (data->data, data->len, &len);
	g_byte_array_free (data, TRUE);
		
	memstream = bonobo_stream_mem_create (ob, len, TRUE, FALSE);
	g_free (ob);
	out_stream = BONOBO_OBJREF (memstream);

	bonobo_object_release_unref (in_stream, ev);
	bonobo_object_release_unref (parent, ev);
		
	return out_stream;
		
 return_unref_stream:
	bonobo_object_release_unref (in_stream, ev);

 return_unref_parent:
	bonobo_object_release_unref (parent, ev);
	return CORBA_OBJECT_NIL;
}

static Bonobo_Unknown
gunzip_resolve (BonoboMoniker *moniker,
		const Bonobo_ResolveOptions *options,
		const CORBA_char *requested_interface,
		CORBA_Environment *ev)
{
	g_warning ("Going to resolve the gunzip now");

	if (!strcmp (requested_interface, "IDL:Bonobo/Stream:1.0"))
		return gunzip_resolve_stream (moniker, options, ev);
	
	return bonobo_moniker_use_extender (
		"OAFIID:Bonobo_MonikerExtender_stream",
		moniker, options, requested_interface, ev);
}

static BonoboObject *
bonobo_moniker_gunzip_factory (BonoboGenericFactory *this, void *closure)
{
	return BONOBO_OBJECT (bonobo_moniker_simple_new ("gunzip:", 
							 gunzip_resolve));
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Moniker_gzip_Factory",
		    "gunzip-moniker", VERSION,
		    bonobo_moniker_gunzip_factory,
		    NULL)

