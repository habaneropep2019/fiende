/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-stream.h: Stream manipulation, abstract class
 *
 * Author:
 *     Miguel de Icaza (miguel@gnu.org).
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_STREAM_H_
#define _BONOBO_STREAM_H_

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

/* Constants to make it easier to safely select drivers */
#define BONOBO_IO_DRIVER_FS  "fs"
#define BONOBO_IO_DRIVER_EFS "efs"
#define BONOBO_IO_DRIVER_VFS "vfs"

#define BONOBO_STREAM_TYPE        (bonobo_stream_get_type ())
#define BONOBO_STREAM(o)          (GTK_CHECK_CAST ((o), BONOBO_STREAM_TYPE, BonoboStream))
#define BONOBO_STREAM_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STREAM_TYPE, BonoboStreamClass))
#define BONOBO_IS_STREAM(o)       (GTK_CHECK_TYPE ((o), BONOBO_STREAM_TYPE))
#define BONOBO_IS_STREAM_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STREAM_TYPE))

typedef struct {
        BonoboObject object;
} BonoboStream;

typedef struct {
	BonoboObjectClass parent_class;

	/*
	 * virtual methods
	 */

	Bonobo_StorageInfo *(*get_info) (BonoboStream *stream,
					 const Bonobo_StorageInfoFields mask,
					 CORBA_Environment *ev);
	void          (*set_info)       (BonoboStream *stream,
					 const Bonobo_StorageInfo *info,
					 const Bonobo_StorageInfoFields mask,
					 CORBA_Environment *ev);
	void          (*write)          (BonoboStream *stream,
					 const Bonobo_Stream_iobuf *buffer,
					 CORBA_Environment *ev);
	void          (*read)           (BonoboStream *stream, 
					 CORBA_long count,
					 Bonobo_Stream_iobuf **buffer,
					 CORBA_Environment *ev);
        CORBA_long    (*seek)           (BonoboStream *stream,
					 CORBA_long offset, 
					 Bonobo_Stream_SeekType whence,
					 CORBA_Environment *ev);
        void          (*truncate)       (BonoboStream *stream,
					 const CORBA_long new_size, 
					 CORBA_Environment *ev);
	void          (*copy_to)        (BonoboStream *stream,
					 const CORBA_char * dest,
					 const CORBA_long bytes,
					 CORBA_long *read,
					 CORBA_long *written,
					 CORBA_Environment *ev);
        void          (*commit)         (BonoboStream *stream,
					 CORBA_Environment *ev);
        void          (*revert)         (BonoboStream *stream,
					 CORBA_Environment *ev);
} BonoboStreamClass;

GtkType                 bonobo_stream_get_type            (void);
POA_Bonobo_Stream__epv *bonobo_stream_get_epv             (void);
Bonobo_Stream           bonobo_stream_corba_object_create (BonoboObject *object);

BonoboStream           *bonobo_stream_open                (const char *driver,
							   const char *path,
							   gint flags,
							   gint mode);

BonoboStream           *bonobo_stream_open_full           (const char *driver,
							   const char *path,
							   gint flags,
							   gint mode,
							   CORBA_Environment *opt_ev);

END_GNOME_DECLS

#endif /* _BONOBO_STREAM_H_ */

