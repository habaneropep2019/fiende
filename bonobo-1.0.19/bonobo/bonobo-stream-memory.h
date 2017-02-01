/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-stream-memory.h: Memory based stream
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_STREAM_MEM_H_
#define _BONOBO_STREAM_MEM_H_

#include <bonobo/bonobo-stream.h>

BEGIN_GNOME_DECLS

struct _BonoboStreamMem;
typedef struct _BonoboStreamMem BonoboStreamMem;
typedef struct _BonoboStreamMemPrivate BonoboStreamMemPrivate;

#ifndef _BONOBO_STORAGE_MEM_H_
struct _BonoboStorageMem;
typedef struct _BonoboStorageMem BonoboStorageMem;
#endif

#define BONOBO_STREAM_MEM_TYPE        (bonobo_stream_mem_get_type ())
#define BONOBO_STREAM_MEM(o)          (GTK_CHECK_CAST ((o), BONOBO_STREAM_MEM_TYPE, BonoboStreamMem))
#define BONOBO_STREAM_MEM_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STREAM_MEM_TYPE, BonoboStreamMemClass))
#define BONOBO_IS_STREAM_MEM(o)       (GTK_CHECK_TYPE ((o), BONOBO_STREAM_MEM_TYPE))
#define BONOBO_IS_STREAM_MEM_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STREAM_MEM_TYPE))

struct _BonoboStreamMem {
	BonoboStream  stream;

	char        *buffer;
	size_t       size;
	long         pos;
	gboolean     read_only;
	gboolean     resizable;
	char        *content_type;
	char        *name;

	BonoboStreamMemPrivate *priv;
};

typedef struct {
	BonoboStreamClass parent_class;
	char           *(*get_buffer) (BonoboStreamMem *stream_mem);
	size_t          (*get_size)   (BonoboStreamMem *stream_mem);
} BonoboStreamMemClass;

GtkType          bonobo_stream_mem_get_type   (void);
BonoboStreamMem *bonobo_stream_mem_construct  (BonoboStreamMem  *stream_mem,
					       Bonobo_Stream     corba_stream,
					       const char       *buffer,
					       size_t            size,
					       gboolean          read_only,
					       gboolean          resizable);

BonoboStream    *bonobo_stream_mem_create     (const char       *buffer,
					       size_t            size,
					       gboolean          read_only,
					       gboolean          resizable);

const char      *bonobo_stream_mem_get_buffer (BonoboStreamMem  *stream_mem);
size_t           bonobo_stream_mem_get_size   (BonoboStreamMem  *stream_mem);

END_GNOME_DECLS

#endif /* _BONOBO_STREAM_MEM_H_ */
