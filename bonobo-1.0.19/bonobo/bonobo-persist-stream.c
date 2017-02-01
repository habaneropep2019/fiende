/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-persist-stream.c: PersistStream implementation.  Can be used as a
 * base class, or directly for implementing objects that use PersistStream.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-persist-stream.h>

#define PARENT_TYPE BONOBO_PERSIST_TYPE

/* Parent GTK object class */
static BonoboPersistClass *bonobo_persist_stream_parent_class;

static CORBA_boolean
impl_is_dirty (PortableServer_Servant servant, CORBA_Environment * ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistStream *pstream = BONOBO_PERSIST_STREAM (object);

	return pstream->is_dirty;
}

static void
impl_load (PortableServer_Servant servant,
	   Bonobo_Stream stream,
	   Bonobo_Persist_ContentType type,
	   CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistStream *ps = BONOBO_PERSIST_STREAM (object);
	
	if (ps->load_fn != NULL)
		(*ps->load_fn)(ps, stream, type, ps->closure, ev);
	else {
		GtkObjectClass *oc = GTK_OBJECT (ps)->klass;
		BonoboPersistStreamClass *class = BONOBO_PERSIST_STREAM_CLASS (oc);

		if (class->load)
			(*class->load)(ps, stream, type, ev);
		else
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_NotSupported, NULL);
	}
}

static void
impl_save (PortableServer_Servant servant,
	   Bonobo_Stream stream,
	   Bonobo_Persist_ContentType type,
	   CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistStream *ps = BONOBO_PERSIST_STREAM (object);
	
	if (ps->save_fn != NULL)
		(*ps->save_fn)(ps, stream, type, ps->closure, ev);
	else {
		GtkObjectClass *oc = GTK_OBJECT (ps)->klass;
		BonoboPersistStreamClass *class = BONOBO_PERSIST_STREAM_CLASS (oc);

		if (class->save)
			(*class->save)(ps, stream, type, ev);
		else
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_NotSupported, NULL);
	}

	ps->is_dirty = FALSE;
}

static CORBA_long
impl_get_size_max (PortableServer_Servant servant, CORBA_Environment * ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistStream *ps = BONOBO_PERSIST_STREAM (object);
	GtkObjectClass *oc = GTK_OBJECT (object)->klass;
	BonoboPersistStreamClass *class = BONOBO_PERSIST_STREAM_CLASS (oc);


	if (ps->max_fn != NULL)
		return (*ps->max_fn)(ps, ps->closure, ev);

	return (*class->get_size_max)(BONOBO_PERSIST_STREAM (object), ev);
}

static CORBA_long
bonobo_persist_stream_size_unknown (BonoboPersistStream *ps,
				    CORBA_Environment *ev)
{
	return -1;
}

static Bonobo_Persist_ContentTypeList *
get_content_types (BonoboPersist *persist, CORBA_Environment *ev)
{
	BonoboPersistStream *ps = BONOBO_PERSIST_STREAM (persist);

	if (ps->types_fn)
		return ps->types_fn (ps, ps->closure, ev);
	else
		return bonobo_persist_generate_content_types (1, "");
}

static void
bonobo_persist_stream_class_init (BonoboPersistStreamClass *klass)
{
	BonoboPersistClass *persist_class = BONOBO_PERSIST_CLASS (klass);
	POA_Bonobo_PersistStream__epv *epv = &klass->epv;

	bonobo_persist_stream_parent_class = gtk_type_class (PARENT_TYPE);

	/* Override and initialize methods */
	klass->save = NULL;
	klass->load = NULL;
	klass->get_size_max = bonobo_persist_stream_size_unknown;

	persist_class->get_content_types = get_content_types;

	epv->load       = impl_load;
	epv->save       = impl_save;
	epv->getMaxSize = impl_get_size_max;
	epv->isDirty    = impl_is_dirty;
}

static void
bonobo_persist_stream_init (BonoboPersistStream *ps)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC_FULL (BonoboPersistStream,
			   Bonobo_PersistStream,
			   PARENT_TYPE,
			   bonobo_persist_stream);

/**
 * bonobo_persist_stream_construct:
 * @ps: A BonoboPersistStream object
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Initializes the BonoboPersistStream object.  The load and save
 * operations for the object are performed by the provided @load_fn
 * and @save_fn callback functions, which are passed @closure when
 * they are invoked.  If either @load_fn or @save_fn is %NULL, the
 * corresponding operation is performed by the class load and save
 * routines.
 *
 * Returns: The initialized BonoboPersistStream object.
 */
BonoboPersistStream *
bonobo_persist_stream_construct (BonoboPersistStream       *ps,
				 BonoboPersistStreamIOFn    load_fn,
				 BonoboPersistStreamIOFn    save_fn,
				 BonoboPersistStreamMaxFn   max_fn,
				 BonoboPersistStreamTypesFn types_fn,
				 void                      *closure)
{
	g_return_val_if_fail (ps != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PERSIST_STREAM (ps), NULL);

	ps->load_fn = load_fn;
	ps->save_fn = save_fn;
	ps->max_fn = max_fn;
	ps->types_fn = types_fn;
	ps->closure = closure;
	
	return ps;
}

/**
 * bonobo_persist_stream_new:
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @max_fn: get_max_size routine
 * @types_fn: get_content_types routine
 * @closure: Data passed to IO routines.
 *
 * Creates a new BonoboPersistStream object. The various operations
 * for the object are performed by the provided callback functions,
 * which are passed @closure when they are invoked. If any callback is
 * %NULL, the corresponding operation is performed by the class load
 * and save routines.
 *
 * Returns: the newly-created BonoboPersistStream object.
 */
BonoboPersistStream *
bonobo_persist_stream_new (BonoboPersistStreamIOFn    load_fn,
			   BonoboPersistStreamIOFn    save_fn,
			   BonoboPersistStreamMaxFn   max_fn,
			   BonoboPersistStreamTypesFn types_fn,
			   void                      *closure)
{
	BonoboPersistStream *ps;

	ps = gtk_type_new (bonobo_persist_stream_get_type ());

	bonobo_persist_stream_construct (ps, load_fn, save_fn,
					 max_fn, types_fn, closure);

	return ps;
}

/**
 * bonobo_persist_stream_set_dirty:
 * @ps: A BonoboPersistStream object
 * @dirty: A boolean value representing whether the object is dirty or not
 *
 * This routine sets the dirty bit for the PersistStream object.
 */
void
bonobo_persist_stream_set_dirty (BonoboPersistStream *pstream, gboolean dirty)
{
	g_return_if_fail (pstream != NULL);
	g_return_if_fail (BONOBO_IS_PERSIST_STREAM (pstream));

	pstream->is_dirty = dirty;
}
