/**
 * gnome-object-io.c: Helper routines for loading and saving of objects
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999, Helix Code, Inc.
 */
#ifndef _BONOBO_OBJECT_IO_H_
#define _BONOBO_OBJECT_IO_H_

typedef enum {
	GNOME_IO_OK,
	
	/* Generic error */
	GNOME_IOERR_GENERAL,

	/* PersistStorage interface not supported by object */
	GNOME_IOERR_PERSIST_NOT_SUPPORTED
	
} GnomeIOStatus;

void            bonobo_persist_stream_save_object_iid  (Bonobo_Stream target,
							const CORBA_char *object_iid,
							CORBA_Environment *ev);
char           *bonobo_persist_stream_load_object_iid  (Bonobo_Stream source);
GnomeIOStatus   bonobo_persiststream_save_to_stream    (Bonobo_PersistStream pstream,
							Bonobo_Stream target,
							const char *object_iid);
GnomeIOStatus   bonobo_object_save_to_stream           (BonoboObject *object,
							Bonobo_Stream stream,
							const char *object_iid);

#endif /* _BONOBO_OBJECT_IO_H_ */
