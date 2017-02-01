/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-persist.c: a persistance interface
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/bonobo-persist.h>

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

/* Parent GTK object class */
static GtkObjectClass *bonobo_persist_parent_class;

#define CLASS(o) BONOBO_PERSIST_CLASS(GTK_OBJECT(o)->klass)

static inline BonoboPersist *
bonobo_persist_from_servant (PortableServer_Servant servant)
{
	return BONOBO_PERSIST (bonobo_object_from_servant (servant));
}

static Bonobo_Persist_ContentTypeList *
impl_Bonobo_Persist_getContentTypes (PortableServer_Servant servant,
				     CORBA_Environment     *ev)
{
	BonoboPersist *persist = bonobo_persist_from_servant (servant);

	return CLASS (persist)->get_content_types (persist, ev);
}

static void
bonobo_persist_destroy (GtkObject *object)
{
	bonobo_persist_parent_class->destroy (object);
}

static void
bonobo_persist_class_init (BonoboPersistClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	POA_Bonobo_Persist__epv *epv = &klass->epv;

	bonobo_persist_parent_class = gtk_type_class (PARENT_TYPE);

	/* Override and initialize methods */
	object_class->destroy = bonobo_persist_destroy;

	epv->getContentTypes = impl_Bonobo_Persist_getContentTypes;
}

static void
bonobo_persist_init (GtkObject *object)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC_FULL (BonoboPersist, 
			   Bonobo_Persist,
			   PARENT_TYPE,
			   bonobo_persist);

/**
 * bonobo_persist_generate_content_types:
 * @num: the number of content types specified
 * @...: the content types (as strings)
 *
 * Returns: a ContentTypeList containing the given ContentTypes
 **/
Bonobo_Persist_ContentTypeList *
bonobo_persist_generate_content_types (int num, ...)
{
	Bonobo_Persist_ContentTypeList *types;
	va_list ap;
	char *type;
	int i;

	types = Bonobo_Persist_ContentTypeList__alloc ();
	CORBA_sequence_set_release (types, TRUE);
	types->_length = types->_maximum = num;
	types->_buffer = CORBA_sequence_Bonobo_Persist_ContentType_allocbuf (num);

	va_start (ap, num);
	for (i = 0; i < num; i++) {
		type = va_arg (ap, char *);
		types->_buffer[i] = CORBA_string_alloc (strlen (type) + 1);
		strcpy (types->_buffer[i], type);
	}
	va_end (ap);

	return types;
}
