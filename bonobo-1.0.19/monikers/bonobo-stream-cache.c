/*
 * bonobo-stream-cache.c: 
 *
 * A simple cache for streams (direct mapped, write back)
 *
 * Author:
 *	Dietmar Maurer (dietmar@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#include <config.h>
#include <string.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-stream-client.h>

#include "bonobo-stream-cache.h"

/* configurable values for cache size */

#define SC_PAGE_SIZE_BITS 14
#define SC_CACHE_SIZE_BITS 5

/* some handy macros */

#define SC_PAGE_SIZE         (1 << (SC_PAGE_SIZE_BITS - 1))
#define SC_CACHE_SIZE        (1 << (SC_CACHE_SIZE_BITS - 1))

#define SC_CACHE_TAG(pos)    (pos >> (SC_PAGE_SIZE_BITS - 1))
#define SC_TAG_POS(tag)      (tag << (SC_PAGE_SIZE_BITS - 1))
#define SC_CACHE_INDEX(pos)  (SC_CACHE_TAG(pos) & (SC_CACHE_SIZE - 1))
#define SC_BLOCK_OFFSET(pos) (pos & (SC_PAGE_SIZE - 1))


typedef struct {
	char     buf [SC_PAGE_SIZE];
	long     tag;
	gboolean valid;
	gboolean dirty;
} CacheEntry; 

struct _BonoboStreamCachePrivate {
	Bonobo_Stream cs;
	long          pos;
	long          size;
	CacheEntry    cache [SC_CACHE_SIZE];
};

static void
bonobo_stream_cache_invalidate (BonoboStreamCache *stream_cache, 
				long               pos)
{
	long i, tag = SC_CACHE_TAG (pos);

	for (i = 0; i < SC_CACHE_SIZE; i++) {
		if (stream_cache->priv->cache [i].valid &&
		    (stream_cache->priv->cache [i].tag >= tag))
			stream_cache->priv->cache [i].valid = FALSE;
	}
}

static void
bonobo_stream_cache_flush (BonoboStreamCache *stream, 
			   int                index,
			   CORBA_Environment *ev)
{
	long i, end, pos;
	
	end = index < 0 ? SC_CACHE_SIZE : index + 1;

	for (i = index < 0 ? 0 : index; i < end; i++) {
		if (((index < 0) || (index == i)) &&
		    stream->priv->cache [i].valid &&
		    stream->priv->cache [i].dirty) {
			pos = SC_TAG_POS (stream->priv->cache [i].tag);
			
			Bonobo_Stream_seek (stream->priv->cs, pos,
					    Bonobo_Stream_SEEK_SET, ev);
			if (BONOBO_EX (ev))
				continue;

			bonobo_stream_client_write (stream->priv->cs,
			        stream->priv->cache [i].buf,
						    SC_PAGE_SIZE, ev);
			if (!BONOBO_EX (ev))
				stream->priv->cache [i].dirty = FALSE;
		}
	}
}

static void
bonobo_stream_cache_load (BonoboStreamCache *stream, 
			  long               tag,
			  CORBA_Environment *ev)
{
	Bonobo_Stream_iobuf *iobuf;
	long pos, index;

	pos = SC_TAG_POS (tag);
	index = SC_CACHE_INDEX (pos);

	bonobo_stream_cache_flush (stream, index, ev);
	if (BONOBO_EX (ev))
		return;

	Bonobo_Stream_seek (stream->priv->cs, pos, Bonobo_Stream_SEEK_SET, ev);
	if (BONOBO_EX (ev))
		return;

	Bonobo_Stream_read (stream->priv->cs, SC_PAGE_SIZE, &iobuf, ev);
	if (BONOBO_EX (ev))
		return;
	
	if (iobuf->_length < SC_PAGE_SIZE) /* eof  - fill with zero */
		memset (stream->priv->cache [index].buf + iobuf->_length, 0,
			SC_PAGE_SIZE - iobuf->_length);
				
	if ((pos + iobuf->_length) > stream->priv->size)
		stream->priv->size = pos + iobuf->_length;

	memcpy (stream->priv->cache [index].buf, iobuf->_buffer, 
		iobuf->_length);
	
	stream->priv->cache [index].valid = TRUE;
	stream->priv->cache [index].dirty = FALSE;
	stream->priv->cache [index].tag = tag;

	CORBA_free (iobuf);
}

static long
bonobo_stream_cache_read (BonoboStreamCache *stream, 
			  long               count, 
			  char              *buffer,
			  CORBA_Environment *ev)
{
	long tag, bytes_read = 0;
	int index, offset, bc, d;

	while (bytes_read < count) {
		index = SC_CACHE_INDEX (stream->priv->pos);
		offset = SC_BLOCK_OFFSET (stream->priv->pos);
		tag = SC_CACHE_TAG (stream->priv->pos);

		if ((stream->priv->pos < stream->priv->size) &&
		    stream->priv->cache [index].valid &&
		    stream->priv->cache [index].tag == tag) {
			bc = SC_PAGE_SIZE - offset;

			if ((bytes_read + bc) > count)
				bc = count - bytes_read;

			if ((d = (stream->priv->pos + bc) - 
			     stream->priv->size) > 0)
				bc -= d;
			if (!bc)
				return bytes_read;

			memcpy (buffer + bytes_read, 
				stream->priv->cache [index].buf + offset, bc);
			bytes_read += bc;
			stream->priv->pos += bc;
		} else {
			bonobo_stream_cache_load (stream, tag, ev);
			if (BONOBO_EX (ev))
				break;
			if (stream->priv->pos >= stream->priv->size)
				break;
		}
	}

	return bytes_read;
}

static BonoboStream *
create_stream_cache_server (const BonoboStreamCache *stream_cache)
{
	Bonobo_Stream corba_stream;

	corba_stream = bonobo_stream_corba_object_create (
		BONOBO_OBJECT (stream_cache));

	return BONOBO_STREAM (
		bonobo_object_construct (BONOBO_OBJECT (stream_cache), 
					 corba_stream));
}

static void
bonobo_stream_cache_destroy (GtkObject *object)
{
	BonoboStreamCache *stream_cache = BONOBO_STREAM_CACHE (object);

	if (stream_cache->priv->cs)
		bonobo_object_release_unref (stream_cache->priv->cs, NULL);

	g_free (stream_cache->priv);
}

static Bonobo_StorageInfo*
impl_Bonobo_Stream_getInfo (BonoboStream                   *stream, 
			    const Bonobo_StorageInfoFields  mask,
			    CORBA_Environment              *ev)
{
	BonoboStreamCache *stream_cache = BONOBO_STREAM_CACHE (stream);
	
	return Bonobo_Stream_getInfo (stream_cache->priv->cs, mask, ev);
}

static void
impl_Bonobo_Stream_setInfo (BonoboStream                   *stream, 
			    const Bonobo_StorageInfo       *info,
			    const Bonobo_StorageInfoFields  mask, 
			    CORBA_Environment              *ev)
{
	BonoboStreamCache *stream_cache = BONOBO_STREAM_CACHE (stream);
	
	Bonobo_Stream_setInfo (stream_cache->priv->cs, info, mask, ev);
}

static void
impl_Bonobo_Stream_write (BonoboStream              *bonobo_stream, 
			  const Bonobo_Stream_iobuf *buffer,
			  CORBA_Environment         *ev)
{
	BonoboStreamCache *stream = BONOBO_STREAM_CACHE (bonobo_stream);
	long tag, bytes_written = 0;
	int index, offset, bc;
	
	while (bytes_written < buffer->_length) {
		index = SC_CACHE_INDEX (stream->priv->pos);
		offset = SC_BLOCK_OFFSET (stream->priv->pos);
		tag = SC_CACHE_TAG (stream->priv->pos);

		if (stream->priv->cache [index].valid &&
		    stream->priv->cache [index].tag == tag) {
			bc = SC_PAGE_SIZE - offset;
			if (bc > buffer->_length) 
				bc = buffer->_length;
			memcpy (stream->priv->cache [index].buf + offset,
				buffer->_buffer + bytes_written, bc);
			bytes_written += bc;
			stream->priv->pos += bc;
			stream->priv->cache [index].dirty = TRUE;
		} else {
			bonobo_stream_cache_load (stream, tag, ev);
			if (BONOBO_EX (ev))
				break;
		}
	}
}

static void
impl_Bonobo_Stream_read (BonoboStream         *stream, 
			 CORBA_long            count,
			 Bonobo_Stream_iobuf **buffer, 
			 CORBA_Environment    *ev)
{
	BonoboStreamCache *stream_cache = BONOBO_STREAM_CACHE (stream);
	CORBA_octet *data;

	if (count < 0) {
		bonobo_exception_set (ev, ex_Bonobo_Stream_IOError);
		return;
	}

	*buffer = Bonobo_Stream_iobuf__alloc ();
	CORBA_sequence_set_release (*buffer, TRUE);
	data = CORBA_sequence_CORBA_octet_allocbuf (count);
	(*buffer)->_buffer = data;
	(*buffer)->_length = bonobo_stream_cache_read (stream_cache, count, 
						       data, ev);
}

static CORBA_long
impl_Bonobo_Stream_seek (BonoboStream           *stream, 
			 CORBA_long              offset, 
			 Bonobo_Stream_SeekType  whence, 
			 CORBA_Environment      *ev)
{
	BonoboStreamCache *stream_cache = BONOBO_STREAM_CACHE (stream);
	
	stream_cache->priv->pos = Bonobo_Stream_seek (stream_cache->priv->cs, 
						      offset, whence, ev);

	return stream_cache->priv->pos;
}

static void
impl_Bonobo_Stream_truncate (BonoboStream      *stream, 
			     const CORBA_long   new_size, 
			     CORBA_Environment *ev)
{
	BonoboStreamCache *stream_cache = BONOBO_STREAM_CACHE (stream);
	
	bonobo_stream_cache_invalidate (stream_cache, new_size);
	
	stream_cache->priv->size = new_size;

	Bonobo_Stream_truncate (stream_cache->priv->cs, new_size, ev);
}

static void
impl_Bonobo_Stream_copyTo (BonoboStream      *stream, 
			   const CORBA_char  *dest,
			   const CORBA_long   bytes, 
			   CORBA_long        *read_bytes,
			   CORBA_long        *written_bytes, 
			   CORBA_Environment *ev)
{
	BonoboStreamCache *stream_cache = BONOBO_STREAM_CACHE (stream);

	Bonobo_Stream_copyTo (stream_cache->priv->cs, dest, bytes, read_bytes,
			      written_bytes, ev);
}

static void
impl_Bonobo_Stream_commit (BonoboStream      *stream, 
			   CORBA_Environment *ev)
{
	BonoboStreamCache *stream_cache = BONOBO_STREAM_CACHE (stream);

	bonobo_stream_cache_flush (stream_cache, -1, ev);
	 
	Bonobo_Stream_commit (stream_cache->priv->cs, ev);
}

static void
impl_Bonobo_Stream_revert (BonoboStream      *stream, 
			   CORBA_Environment *ev)
{
	BonoboStreamCache *stream_cache = BONOBO_STREAM_CACHE (stream);

	bonobo_stream_cache_invalidate (stream_cache, 0);

	Bonobo_Stream_revert (stream_cache->priv->cs, ev);
}

static void
bonobo_stream_cache_init (BonoboStreamCache *stream)
{
	stream->priv = g_new0 (BonoboStreamCachePrivate, 1);
}

static void
bonobo_stream_cache_class_init (BonoboStreamCacheClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	BonoboStreamClass *sclass = BONOBO_STREAM_CLASS (class);
	
	sclass->get_info = impl_Bonobo_Stream_getInfo;
	sclass->set_info = impl_Bonobo_Stream_setInfo;
	sclass->write    = impl_Bonobo_Stream_write;
	sclass->read     = impl_Bonobo_Stream_read;
	sclass->seek     = impl_Bonobo_Stream_seek;
	sclass->truncate = impl_Bonobo_Stream_truncate;
	sclass->copy_to  = impl_Bonobo_Stream_copyTo;
	sclass->commit   = impl_Bonobo_Stream_commit;
	sclass->revert   = impl_Bonobo_Stream_revert;

	object_class->destroy = bonobo_stream_cache_destroy;
}

GtkType
bonobo_stream_cache_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboStreamCache",
			sizeof (BonoboStreamCache),
			sizeof (BonoboStreamCacheClass),
			(GtkClassInitFunc) bonobo_stream_cache_class_init,
			(GtkObjectInitFunc) bonobo_stream_cache_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};
		
		type = gtk_type_unique (bonobo_stream_get_type (), &info);
	}
  
	return type;
}

/** 
 * bonobo_stream_cache_create:
 * @cs: a reference to the stream we want to cache
 * @opt_ev: an optional environment
 *
 * Returns a new BonoboStream object
 */
BonoboStream *
bonobo_stream_cache_create (Bonobo_Stream      cs,
			    CORBA_Environment *opt_ev)
{
	BonoboStreamCache *stream;
	CORBA_Environment  ev, *my_ev;

	bonobo_return_val_if_fail (cs != NULL, NULL, opt_ev);
	
	if (!(stream = gtk_type_new (bonobo_stream_cache_get_type ()))) {
		if (opt_ev)
			bonobo_exception_set (opt_ev, ex_Bonobo_Storage_IOError);
		return NULL;
	}

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	stream->priv->cs = bonobo_object_dup_ref (cs, my_ev);

	if (BONOBO_EX (my_ev)) {
		if (!opt_ev)
			CORBA_exception_free (&ev);
		bonobo_object_unref (BONOBO_OBJECT (stream));
		return NULL;
	}

	if (!opt_ev)
		CORBA_exception_free (&ev);

	if (!create_stream_cache_server (stream)) {
		bonobo_object_unref (BONOBO_OBJECT (stream));
		bonobo_exception_set (opt_ev, ex_Bonobo_Storage_IOError);
		return NULL;
	}

	return BONOBO_STREAM (stream);
}

