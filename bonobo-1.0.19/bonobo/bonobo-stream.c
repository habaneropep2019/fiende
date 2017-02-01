/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-stream.c: Stream manipulation, abstract class
 *
 * Author:
 *     Miguel de Icaza (miguel@gnu.org).
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#include <config.h>

#include <bonobo/bonobo-stream.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-storage-plugin.h>

static BonoboObjectClass *bonobo_stream_parent_class;

POA_Bonobo_Stream__vepv bonobo_stream_vepv;

#define CLASS(o) BONOBO_STREAM_CLASS(GTK_OBJECT(o)->klass)

static inline BonoboStream *
bonobo_stream_from_servant (PortableServer_Servant servant)
{
	return BONOBO_STREAM (bonobo_object_from_servant (servant));
}

static Bonobo_StorageInfo*
impl_Bonobo_Stream_getInfo (PortableServer_Servant servant,
			    const Bonobo_StorageInfoFields mask,
			    CORBA_Environment *ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	return CLASS (stream)->get_info (stream, mask, ev);
}

static void          
impl_Bonobo_Stream_setInfo (PortableServer_Servant servant,
			    const Bonobo_StorageInfo *info,
			    const Bonobo_StorageInfoFields mask,
			    CORBA_Environment *ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	CLASS (stream)->set_info (stream, info, mask, ev);
}

static void
impl_Bonobo_Stream_read (PortableServer_Servant servant,
			 CORBA_long             count,
			 Bonobo_Stream_iobuf  **buffer,
			 CORBA_Environment     *ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	CLASS (stream)->read (stream, count, buffer, ev);
}

static void
impl_Bonobo_Stream_write (PortableServer_Servant servant,
			  const Bonobo_Stream_iobuf *buffer,
			  CORBA_Environment *ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	CLASS (stream)->write (stream, buffer, ev);
}

static CORBA_long
impl_Bonobo_Stream_seek (PortableServer_Servant servant,
			 CORBA_long offset,
			 Bonobo_Stream_SeekType whence,
			 CORBA_Environment *ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	return CLASS (stream)->seek (stream, offset, whence, ev);
}

static void
impl_Bonobo_Stream_truncate (PortableServer_Servant servant,
			     CORBA_long length,
			     CORBA_Environment *ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	CLASS (stream)->truncate (stream, length, ev);
}

static void
impl_Bonobo_Stream_copyTo (PortableServer_Servant servant,
			   const CORBA_char *dest,
			   CORBA_long bytes,
			   CORBA_long *read,
			   CORBA_long *written,
			   CORBA_Environment *ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	CLASS (stream)->copy_to (stream, dest, bytes, read, written, ev);
}

static void
impl_Bonobo_Stream_commit (PortableServer_Servant servant,
			   CORBA_Environment * ev)
{
       BonoboStream *stream = bonobo_stream_from_servant (servant);

       CLASS (stream)->commit (stream, ev);
}

static void
impl_Bonobo_Stream_revert (PortableServer_Servant servant,
			   CORBA_Environment * ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	CLASS (stream)->revert (stream, ev);
}

/**
 * bonobo_stream_get_epv:
 */
POA_Bonobo_Stream__epv *
bonobo_stream_get_epv (void)
{
	POA_Bonobo_Stream__epv *epv;

	epv = g_new0 (POA_Bonobo_Stream__epv, 1);

	epv->getInfo  = impl_Bonobo_Stream_getInfo;
	epv->setInfo  = impl_Bonobo_Stream_setInfo;
	epv->read     = impl_Bonobo_Stream_read;
	epv->write    = impl_Bonobo_Stream_write;
	epv->seek     = impl_Bonobo_Stream_seek;
	epv->truncate = impl_Bonobo_Stream_truncate;
	epv->copyTo   = impl_Bonobo_Stream_copyTo;
	epv->commit   = impl_Bonobo_Stream_commit;
	epv->revert   = impl_Bonobo_Stream_revert;

	return epv;
}

static void
init_stream_corba_class (void)
{
	/* The VEPV */
	bonobo_stream_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_stream_vepv.Bonobo_Stream_epv = bonobo_stream_get_epv ();
}

static void
bonobo_stream_class_init (BonoboStreamClass *klass)
{
	bonobo_stream_parent_class = gtk_type_class (bonobo_object_get_type ());

	init_stream_corba_class ();
}

/**
 * bonobo_stream_get_type:
 *
 * Returns: the GtkType for a BonoboStream.
 */
GtkType
bonobo_stream_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboStream",
			sizeof (BonoboStream),
			sizeof (BonoboStreamClass),
			(GtkClassInitFunc) bonobo_stream_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_stream_open:
 * @driver: driver to use for opening.
 * @path: path where the base file resides
 * @flags: Bonobo Storage OpenMode
 * @mode: Unix open(2) mode
 *
 * Opens or creates the file named at @path with the stream driver @driver.
 *
 * @driver is one of: "fs" or "vfs" for now.
 *
 * Returns: a created BonoboStream object.
 */
BonoboStream *
bonobo_stream_open_full (const char *driver, const char *path, gint flags, 
			 gint mode, CORBA_Environment *opt_ev)
{
	BonoboStream  *stream = NULL;
	StoragePlugin *p;
	CORBA_Environment ev, *my_ev;
	
	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	if (!driver || !path)
		CORBA_exception_set (my_ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);
	else if (!(p = bonobo_storage_plugin_find (driver)) ||
		 !p->stream_open)
		CORBA_exception_set (my_ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
	else 
		stream = p->stream_open (path, flags, mode, my_ev);

	if (!opt_ev) {
		if (BONOBO_EX (my_ev))
			g_warning ("bonobo_stream_open failed '%s'",
				   bonobo_exception_get_text (my_ev));
		CORBA_exception_free (&ev);
	}

	return stream;
}

BonoboStream *
bonobo_stream_open (const char *driver, const char *path,
		    gint flags, gint mode)
{
	return bonobo_stream_open_full (driver, path, flags, mode, NULL);
}

/**
 * bonobo_stream_corba_object_create:
 * @object: the GtkObject that will wrap the CORBA object
 *
 * Creates and activates the CORBA object that is wrapped by the
 * @object BonoboObject.
 *
 * Returns: An activated object reference to the created object
 * or %CORBA_OBJECT_NIL in case of failure.
 */
Bonobo_Stream
bonobo_stream_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_Stream *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_Stream *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_stream_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_Stream__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);

	return bonobo_object_activate_servant (object, servant);
}

/**
 * bonobo_internal_get_major_mime_type:
 *
 * This is not the function you are looking for. In fact, you've never
 * even heard of this function.
 * But when you wake up, you will remember to g_free the result.
 */
gchar *
bonobo_internal_get_major_mime_type (const char *mime_type)
{
	char *major_end;
	char *major;
	int   major_length;

	major_end = strchr (mime_type, '/');
	if (!major_end)
		return g_strdup (mime_type);

	major_length = major_end - mime_type;
	major = g_strndup (mime_type, major_length);
	
	return major;
}
