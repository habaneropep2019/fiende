/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-stream-memory.c: Memory based stream
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#include <config.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <bonobo/bonobo-stream-memory.h>
#include <bonobo/bonobo-exception.h>
#include <errno.h>

static BonoboStreamClass *bonobo_stream_mem_parent_class;

static Bonobo_StorageInfo*
mem_get_info (BonoboStream                  *stream,
	      const Bonobo_StorageInfoFields mask,
	      CORBA_Environment             *ev)
{
	Bonobo_StorageInfo *si;
	BonoboStreamMem    *smem = BONOBO_STREAM_MEM (stream);

	si = Bonobo_StorageInfo__alloc ();

	si->name = CORBA_string_dup (smem->name);

	if (mask & Bonobo_FIELD_SIZE)
		si->size = smem->size;
	if (mask & Bonobo_FIELD_TYPE)
		si->type = Bonobo_STORAGE_TYPE_REGULAR;
	si->content_type = CORBA_string_dup (
		(mask & Bonobo_FIELD_CONTENT_TYPE)
		? smem->content_type
		: "");

	return si;
}

static void
mem_set_info (BonoboStream *stream,
	      const Bonobo_StorageInfo *info,
	      const Bonobo_StorageInfoFields mask,
	      CORBA_Environment *ev)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (stream);

	if (smem->read_only) {
		CORBA_exception_set (
			ev, CORBA_USER_EXCEPTION,
			ex_Bonobo_Stream_NoPermission, NULL);
		return;
	}

	if (mask & Bonobo_FIELD_SIZE)
		CORBA_exception_set (
			ev, CORBA_USER_EXCEPTION,
			ex_Bonobo_Stream_NotSupported, NULL);

	if ((mask & Bonobo_FIELD_TYPE) && 
	    (info->type != Bonobo_STORAGE_TYPE_REGULAR))
		CORBA_exception_set (
			ev, CORBA_USER_EXCEPTION,
			ex_Bonobo_Stream_NotSupported, NULL);

	if (mask & Bonobo_FIELD_CONTENT_TYPE) {
		bonobo_return_if_fail (info->content_type != NULL, ev);
		g_free (smem->content_type);
		smem->content_type = g_strdup (info->content_type);
	}

	if (strcmp (info->name, smem->name)) {
		bonobo_return_if_fail (info->name != NULL, ev);
		g_free (smem->name);
		smem->name = g_strdup (info->name);
	}
}

static void
mem_truncate (BonoboStream *stream,
	      const CORBA_long new_size, 
	      CORBA_Environment *ev)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (stream);
	void *newp;
	
	if (smem->read_only)
		return;

	newp = g_realloc (smem->buffer, new_size);
	if (!newp) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Stream_NoPermission, NULL);
		return;
	}

	smem->buffer = newp;
	smem->size = new_size;

	if (smem->pos > new_size)
		smem->pos = new_size;
}

static void
mem_write (BonoboStream *stream, const Bonobo_Stream_iobuf *buffer,
	   CORBA_Environment *ev)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (stream);
	long len = buffer->_length;

	if (smem->read_only){
		g_warning ("Should signal an exception here");
		return;
	}

	if (smem->pos + len > smem->size){
		if (smem->resizable){
			smem->size = smem->pos + len;
			smem->buffer = g_realloc (smem->buffer, smem->size);
		} else {
			mem_truncate (stream, smem->pos + len, ev);
			g_warning ("Should check for an exception here");
		}
	}

	if (smem->pos + len > smem->size)
		len = smem->size - smem->pos;
	
	memcpy (smem->buffer + smem->pos, buffer->_buffer, len);
	smem->pos += len;
		
	return;
}

static void
mem_read (BonoboStream *stream, CORBA_long count,
	  Bonobo_Stream_iobuf ** buffer,
	  CORBA_Environment *ev)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (stream);

	if (smem->pos + count > smem->size)
		count = smem->size - smem->pos;
	    
	*buffer = Bonobo_Stream_iobuf__alloc ();
	CORBA_sequence_set_release (*buffer, TRUE);
	(*buffer)->_buffer = CORBA_sequence_CORBA_octet_allocbuf (count);
	(*buffer)->_length = count;
	
	memcpy ((*buffer)->_buffer, smem->buffer + smem->pos, count);

	smem->pos += count;
}

static CORBA_long
mem_seek (BonoboStream *stream,
	  CORBA_long offset, Bonobo_Stream_SeekType whence,
	  CORBA_Environment *ev)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (stream);
	int pos = 0;
	
	switch (whence){
	case Bonobo_Stream_SEEK_SET:
		pos = offset;
		break;

	case Bonobo_Stream_SEEK_CUR:
		pos = smem->pos + offset;
		break;

	case Bonobo_Stream_SEEK_END:
		pos = smem->size + offset;
		break;

	default:
		g_warning ("Signal exception");
	}

	if (pos > smem->size){
		if (smem->resizable){
			smem->buffer = g_realloc (smem->buffer, pos);
			memset (smem->buffer + smem->size, 0,
				pos - smem->size);
			smem->size = pos;
		} else
			mem_truncate (stream, pos, ev);
	}
	smem->pos = pos;
	return pos;
}

static void
mem_copy_to  (BonoboStream *stream,
	      const CORBA_char *dest,
	      const CORBA_long bytes,
	      CORBA_long *read,
	      CORBA_long *written,
	      CORBA_Environment *ev)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (stream);
	gint fd_out;
	gint w;
	
	*read = smem->size - smem->pos;
	*written = 0;
	
	/* create the output file */
	fd_out = creat(dest, 0666);
	if (fd_out == -1) {
		g_warning ("unable to create output file");
		return;
	}
	
	/* write the memory stream buffer to the output file */
	do {
		w = write (fd_out, smem->buffer, *read);
	} while (w == -1 && errno == EINTR);
	
	if (w != -1)
		*written = w;
	else if (errno != EINTR) {
		/* should probably do something to signal an error here */
		g_warning ("ouput file write failed");
	}
	
	
	close(fd_out);

}

static void
mem_commit (BonoboStream *stream,
	    CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_Bonobo_Stream_NotSupported, NULL);
}

static void
mem_revert (BonoboStream *stream,
	    CORBA_Environment *ev)
{
	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_Bonobo_Stream_NotSupported, NULL);
}

static void
mem_destroy (GtkObject *object)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (object);
	
	if (smem->buffer)
		g_free (smem->buffer);
	g_free (smem->name);
	g_free (smem->content_type);
	
	GTK_OBJECT_CLASS (bonobo_stream_mem_parent_class)->destroy (object);
}

static char *
mem_get_buffer (BonoboStreamMem *stream_mem)
{
	g_return_val_if_fail (BONOBO_IS_STREAM_MEM (stream_mem), NULL);

	return stream_mem->buffer;
}

static size_t
mem_get_size (BonoboStreamMem *stream_mem)
{
	g_return_val_if_fail (BONOBO_IS_STREAM_MEM (stream_mem), 0);

	return stream_mem->size;
}

static void
bonobo_stream_mem_class_init (BonoboStreamMemClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	BonoboStreamClass *sclass = BONOBO_STREAM_CLASS (klass);
	
	bonobo_stream_mem_parent_class = gtk_type_class (bonobo_stream_get_type ());

	object_class->destroy = mem_destroy;
	
	sclass->get_info  = mem_get_info;
	sclass->set_info  = mem_set_info;
	sclass->write     = mem_write;
	sclass->read      = mem_read;
	sclass->seek      = mem_seek;
	sclass->truncate  = mem_truncate;
	sclass->copy_to   = mem_copy_to;
	sclass->commit    = mem_commit;
	sclass->revert    = mem_revert;

	klass->get_buffer = mem_get_buffer;
	klass->get_size   = mem_get_size;
}

/**
 * bonobo_stream_mem_get_type:
 *
 * Returns: the GtkType of the BonoboStreamMem class.
 */
GtkType
bonobo_stream_mem_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboStreamMem",
			sizeof (BonoboStreamMem),
			sizeof (BonoboStreamMemClass),
			(GtkClassInitFunc) bonobo_stream_mem_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_stream_get_type (), &info);
	}

	return type;
}

BonoboStreamMem *
bonobo_stream_mem_construct (BonoboStreamMem *stream_mem,
			     Bonobo_Stream    corba_stream,
			     const char      *buffer,
			     size_t           size,
			     gboolean         read_only,
			     gboolean         resizable)
{
	g_return_val_if_fail (corba_stream != CORBA_OBJECT_NIL, NULL);
	g_return_val_if_fail (BONOBO_IS_STREAM_MEM (stream_mem), NULL);

	if (buffer == NULL) {
		stream_mem->buffer = g_malloc (size);
		memset (stream_mem->buffer, 0, size);
	} else
		stream_mem->buffer = g_memdup (buffer, size);

	stream_mem->size = size;
	stream_mem->pos = 0;
	stream_mem->read_only = read_only;
	stream_mem->resizable = resizable;
	stream_mem->name = g_strdup ("");
	stream_mem->content_type = g_strdup ("application/octet-stream");

	return BONOBO_STREAM_MEM (
		bonobo_object_construct (BONOBO_OBJECT (stream_mem),
					 corba_stream));
}

/**
 * bonobo_stream_mem_create:
 * @buffer: The data for which a BonoboStreamMem object is to be created.
 * @size: The size in bytes of @buffer.
 * @read_only: Specifies whether or not the returned BonoboStreamMem
 * object should allow write() operations.
 * @resizable: Whether or not the buffer should be resized as needed.
 *
 * Creates a new BonoboStreamMem object.
 *
 * If @buffer is non-%NULL, @size bytes are copied from it into a new
 * buffer. If @buffer is %NULL, a new buffer of size @size is created
 * and filled with zero bytes.
 *
 * When data is read out of or (if @read_only is FALSE) written into
 * the returned BonoboStream object, the read() and write() operations
 * operate on the new buffer. If @resizable is TRUE, writing or seeking
 * past the end of the buffer will cause the buffer to be expanded (with
 * the new space zero-filled for a seek).
 *
 * Returns: the constructed BonoboStream object
 **/
BonoboStream *
bonobo_stream_mem_create (const char *buffer, size_t size,
			  gboolean read_only, gboolean resizable)
{
	BonoboStreamMem *stream_mem;
	Bonobo_Stream corba_stream;

	stream_mem = gtk_type_new (bonobo_stream_mem_get_type ());
	if (stream_mem == NULL)
		return NULL;


	corba_stream = bonobo_stream_corba_object_create (
		BONOBO_OBJECT (stream_mem));

	if (corba_stream == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (stream_mem));
		return NULL;
	}

	return BONOBO_STREAM (bonobo_stream_mem_construct (
		stream_mem, corba_stream, buffer, size,
		read_only, resizable));
}

/**
 * bonobo_stream_mem_get_buffer:
 * @stream_mem: a BonoboStreamMem
 *
 * Returns the buffer associated with a BonoboStreamMem. If the stream
 * is set to automatically resize itself, this buffer is only guaranteed
 * to stay valid until the next write operation on the stream.
 *
 * Return value: a buffer containing the data written to the stream (or
 * the data the stream was initialized with if nothing has been written).
 **/
const char *
bonobo_stream_mem_get_buffer (BonoboStreamMem *stream_mem)
{
	return BONOBO_STREAM_MEM_CLASS(
		GTK_OBJECT(stream_mem)->klass)->get_buffer (stream_mem);
}

/**
 * bonobo_stream_mem_get_size:
 * @stream_mem: a BonoboStreamMem
 *
 * Returns the size of the data associated with a BonoboStreamMem
 * see bonobo_stream_mem_get_buffer
 *
 * Return value: the size.
 **/
size_t
bonobo_stream_mem_get_size (BonoboStreamMem *stream_mem)
{
	return BONOBO_STREAM_MEM_CLASS(
		GTK_OBJECT(stream_mem)->klass)->get_size (stream_mem);
}
