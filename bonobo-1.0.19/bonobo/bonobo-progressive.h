/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo::ProgressiveDataSink
 *
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#ifndef _BONOBO_PROGRESSIVE_DATA_SINK_H_
#define _BONOBO_PROGRESSIVE_DATA_SINK_H_

#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS

#define BONOBO_PROGRESSIVE_DATA_SINK_TYPE        (bonobo_progressive_data_sink_get_type ())
#define BONOBO_PROGRESSIVE_DATA_SINK(o)          (GTK_CHECK_CAST ((o), BONOBO_PROGRESSIVE_DATA_SINK_TYPE, BonoboProgressiveDataSink))
#define BONOBO_PROGRESSIVE_DATA_SINK_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PROGRESSIVE_DATA_SINK_TYPE, BonoboProgressiveDataSinkClass))
#define BONOBO_IS_PROGRESSIVE_DATA_SINK(o)       (GTK_CHECK_TYPE ((o), BONOBO_PROGRESSIVE_DATA_SINK_TYPE))
#define BONOBO_IS_PROGRESSIVE_DATA_SINK_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PROGRESSIVE_DATA_SINK_TYPE))

typedef struct _BonoboProgressiveDataSink BonoboProgressiveDataSink;
typedef struct _BonoboProgressiveDataSinkPrivate BonoboProgressiveDataSinkPrivate;

/* Callback typedefs. */
typedef int (*BonoboProgressiveDataSinkStartFn)   (BonoboProgressiveDataSink *psink,
						  void *closure);

typedef int (*BonoboProgressiveDataSinkEndFn)     (BonoboProgressiveDataSink *psink, void *closure);

typedef int (*BonoboProgressiveDataSinkAddDataFn) (BonoboProgressiveDataSink *psink,
						  const Bonobo_ProgressiveDataSink_iobuf *buffer,
						  void *closure);

typedef int (*BonoboProgressiveDataSinkSetSizeFn) (BonoboProgressiveDataSink *psink,
						  const CORBA_long count, void *closure);

struct _BonoboProgressiveDataSink {
	BonoboXObject object;

	/*
	 * These are the callbacks the user can set.  If we use the
	 * default class methods, then these are NULL.
	 */
	BonoboProgressiveDataSinkStartFn start_fn;
	BonoboProgressiveDataSinkEndFn end_fn;
	BonoboProgressiveDataSinkAddDataFn add_data_fn;
	BonoboProgressiveDataSinkSetSizeFn set_size_fn;

	void *closure;

	BonoboProgressiveDataSinkPrivate *priv;
};

typedef struct {
	BonoboXObjectClass parent_class;

	POA_Bonobo_ProgressiveDataSink__epv epv;

	/* Methods. */
	int (*start_fn)    (BonoboProgressiveDataSink *psink);
	int (*end_fn)      (BonoboProgressiveDataSink *psink);
	int (*add_data_fn) (BonoboProgressiveDataSink *psink,
			    const Bonobo_ProgressiveDataSink_iobuf *buffer);
	int (*set_size_fn) (BonoboProgressiveDataSink *psink,
			    const CORBA_long count);
			 
} BonoboProgressiveDataSinkClass;

GtkType		           bonobo_progressive_data_sink_get_type        (void);

BonoboProgressiveDataSink *bonobo_progressive_data_sink_new		(BonoboProgressiveDataSinkStartFn   start_fn,
									 BonoboProgressiveDataSinkEndFn     end_fn,
									 BonoboProgressiveDataSinkAddDataFn add_data_fn,
									 BonoboProgressiveDataSinkSetSizeFn set_size_fn,
									 void                              *closure);

BonoboProgressiveDataSink *bonobo_progressive_data_sink_construct       (BonoboProgressiveDataSink         *psink,
									 BonoboProgressiveDataSinkStartFn   start_fn,
									 BonoboProgressiveDataSinkEndFn     end_fn,
									 BonoboProgressiveDataSinkAddDataFn add_data_fn,
									 BonoboProgressiveDataSinkSetSizeFn set_size_fn,
									 void                              *closure);
							       
END_GNOME_DECLS

#endif /* _BONOBO_PROGRESSIVE_DATA_SINK_H_ */

