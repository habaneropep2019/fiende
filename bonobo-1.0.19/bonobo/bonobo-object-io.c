/**
 * gnome-object-io.c: Helper routines for loading and saving of objects
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999, Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-object-io.h>
#include <bonobo/bonobo-object-client.h>
#include <bonobo/bonobo-stream-client.h>

/** 
 * bonobo_persist_stream_save_object_iid:
 * @target: A Bonobo_Stream object where the @object_iid will be written
 * @object_iid: the OBJECT ID to write to the @target stream
 * @ev: Error values are returned here
 *
 * This routine saves the @object_iid in the @target stream.
 */
void
bonobo_persist_stream_save_object_iid (Bonobo_Stream target,
				   const CORBA_char *object_iid,
				   CORBA_Environment *ev)
{
	char *copy;
	int len, slen;
	
	g_return_if_fail (target != CORBA_OBJECT_NIL);
	g_return_if_fail (object_iid != NULL);

	slen = strlen (object_iid) + 1;
	len = sizeof (gint32) + slen;
	copy = g_malloc (len);
	((gint32 *) copy) = slen;
	strcpy (copy + sizeof (gint32), object_iid);
		
	bonobo_stream_client_write (target, copy, len, ev);

	if (BONOBO_EX (ev)){
		CORBA_exception_free (ev);
		return;
	}
}

/**
 * bonobo_persist_stream_load_object_iid:
 * @source: Stream to load the OBJECT ID from.
 *
 * Loads a OBJECT ID from the @source Bonobo_Stream CORBA object reference.
 *
 * Returns: a pointer to the OBJECT ID retrieved from the @source Bonobo_Stream
 * object, or %NULL if an error happens.
 */
char *
bonobo_persist_stream_load_object_iid (Bonobo_Stream source)
{
	CORBA_Environment ev;
	Bonobo_Stream_iobuf *buf;
	CORBA_long n;
	char      *rval;
	
	g_return_val_if_fail (source != CORBA_OBJECT_NIL, NULL);

	CORBA_exception_init (&ev);
	Bonobo_Stream_read (source, sizeof (gint32), &buf, &ev);
	if (BONOBO_EX (&ev) ||
	    buf->_length != sizeof (gint32)){
		CORBA_exception_free (&ev);
		return NULL;
	}

	n = *((gint32 *) buf->_buffer);
	CORBA_free (buf);
	
	Bonobo_Stream_read (source, n, &buf, &ev);
	if (BONOBO_EX (&ev) ||
	    buf->_length != n) {
		CORBA_exception_free (&ev);
		return NULL;
	}
	
	/*
	 * Sanity check: the object-id should be NULL terminated
	 */
	if (buf->_buffer [n - 1] != 0) {
		CORBA_free (buf);
		return NULL;
	}

	rval = g_strdup (buf->_buffer);
	CORBA_free (buf);
	CORBA_exception_free (&ev);

	return rval;
}

/**
 * bonobo_persiststream_save_to_stream:
 * @pstream: A Bonobo_PersistStream CORBA reference.
 * @stream: A Bonobo_Stream CORBA reference to save object on
 *
 * Queries the object_iid for the @pstream object, and saves this on  @object in the
 * @stream and then the object in @pstream is saved.
 *
 * Returns: The IO status for the operation.  Might return %GNOME_IOERR_PERSIST_NOT_SUPPORTED
 * if @object does not support the IDL:Bonobo/PersistStream:1.0 interface
 */
GnomeIOStatus
bonobo_persiststream_save_to_stream (Bonobo_PersistStream pstream, Bonobo_Stream target,
				    const char *object_iid)
{
	CORBA_Environment ev;
	
	g_return_val_if_fail (pstream != CORBA_OBJECT_NIL, GNOME_IOERR_GENERAL);
	g_return_val_if_fail (target != CORBA_OBJECT_NIL, GNOME_IOERR_GENERAL);
	
	CORBA_exception_init (&ev);

	bonobo_persist_stream_save_object_iid (target, object_iid, &ev);

	Bonobo_PersistStream_save (pstream, target, "", &ev);
	if (BONOBO_EX (&ev)){
		CORBA_exception_free (&ev);
		return GNOME_IOERR_GENERAL;
	}

	return GNOME_IO_OK;
}

/**
 * bonobo_object_save_to_stream:
 * @object: A BonoboObject
 * @stream: A Bonobo_Stream CORBA reference to save object on
 *
 * Saves the BonoboObject @object in the @stream.
 *
 * Returns: The IO status for the operation.  Might return %GNOME_IOERR_PERSIST_NOT_SUPPORTED
 * if @object does not support the IDL:Bonobo/PersistStream:1.0 interface
 */
GnomeIOStatus
bonobo_object_save_to_stream (BonoboObject *object, Bonobo_Stream stream,
			     const char *object_iid)
{
	Bonobo_PersistStream pstream;
	
	g_return_val_if_fail (object != NULL, GNOME_IOERR_GENERAL);
	g_return_val_if_fail (BONOBO_IS_OBJECT (object), GNOME_IOERR_GENERAL);
	g_return_val_if_fail (stream != CORBA_OBJECT_NIL, GNOME_IOERR_GENERAL);

	pstream = bonobo_object_query_interface (
		BONOBO_OBJECT (object), "IDL:Bonobo/PersistStream:1.0");
	
	if (pstream != CORBA_OBJECT_NIL)
		return GNOME_IOERR_PERSIST_NOT_SUPPORTED;

	return bonobo_persiststream_save_to_stream (pstream, stream, object_iid);
	
	return GNOME_IO_OK;
}
