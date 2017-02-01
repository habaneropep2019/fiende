/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-event-source.c: Generic event emitter.
 *
 * Author:
 *	Alex Graveley (alex@helixcode.com)
 *	Iain Holmes   (iain@helixcode.com)
 *      docs, Miguel de Icaza (miguel@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <bonobo/bonobo-listener.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-event-source.h>
#include <bonobo/bonobo-running-context.h>
#include <time.h>

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

static GtkObjectClass *bonobo_event_source_parent_class;

struct _BonoboEventSourcePrivate {
	GSList  *listeners;  /* CONTAINS: ListenerDesc* */
	gboolean ignore;
	gint     counter;    /* to create unique listener Ids */
};

typedef struct {
	Bonobo_Listener listener;
	Bonobo_EventSource_ListenerId id;
	gchar **event_masks; /* send all events if NULL */
} ListenerDesc;

/*
 * tries to make a unique connection Id. Adding the time make an
 * accidental remove of a listener more unlikely.
 */
static Bonobo_EventSource_ListenerId
create_listener_id (BonoboEventSource *source)
{
	guint32 id;

	source->priv->counter = source->priv->counter++ & 0x0000ffff;

	if (!source->priv->counter) source->priv->counter++;

	id = source->priv->counter | (time(NULL) << 16);

	return id;
}

static inline BonoboEventSource * 
bonobo_event_source_from_servant (PortableServer_Servant servant)
{
	return BONOBO_EVENT_SOURCE (bonobo_object_from_servant (servant));
}

static void
desc_free (ListenerDesc *desc, CORBA_Environment *ev)
{
	if (desc) {
		g_strfreev (desc->event_masks);
		bonobo_object_release_unref (desc->listener, ev);
		g_free (desc);
	}
}

static Bonobo_EventSource_ListenerId
impl_Bonobo_EventSource_addListenerWithMask (PortableServer_Servant servant,
					     const Bonobo_Listener  l,
					     const CORBA_char      *event_mask,
					     CORBA_Environment     *ev)
{
	BonoboEventSource *event_source;
	ListenerDesc      *desc;

	g_return_val_if_fail (!CORBA_Object_is_nil (l, ev), 0);

	event_source = bonobo_event_source_from_servant (servant);

	if (event_source->priv->ignore) /* Hook for running context */
		bonobo_running_context_ignore_object (l);

	desc = g_new0 (ListenerDesc, 1);
	desc->listener = bonobo_object_dup_ref (l, ev);
	desc->id = create_listener_id (event_source);

	if (event_mask)
		desc->event_masks = g_strsplit (event_mask, ",", 0);

	event_source->priv->listeners = g_slist_prepend (
		event_source->priv->listeners, desc);

	return desc->id;
}

static Bonobo_EventSource_ListenerId
impl_Bonobo_EventSource_addListener (PortableServer_Servant servant,
				     const Bonobo_Listener  l,
				     CORBA_Environment     *ev)
{
	return impl_Bonobo_EventSource_addListenerWithMask (
		servant, l, NULL, ev);
}

static void
impl_Bonobo_EventSource_removeListener (PortableServer_Servant servant,
					const Bonobo_EventSource_ListenerId id,
					CORBA_Environment     *ev)
{
	GSList                   *l, *next;
	BonoboEventSourcePrivate *priv;

	priv = bonobo_event_source_from_servant (servant)->priv;

	for (l = priv->listeners; l; l = next) {
		ListenerDesc *desc = l->data;

		next = l->next;

		if (desc->id == id) {
			priv->listeners = g_slist_remove_link (
				priv->listeners, l);
			g_slist_free_1 (l);
			desc_free (desc, ev);
			return;
		}
	}

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_Bonobo_EventSource_UnknownListener, 
			     NULL);
}

/*
 * if the mask starts with a '=', we do exact compares - else we only check 
 * if the mask is a prefix of name.
 */
static gboolean
event_match (const char *name, gchar **event_masks)
{
	int i = 0, j = 0;

	while (event_masks[j]) {
		char *mask = event_masks[j];
		
		if (mask [0] == '=')
			if (!strcmp (name, mask + 1))
				return TRUE;

		while (name [i] && mask [i] && name [i] == mask [i])
			i++;
		
		if (mask [i] == '\0')
			return TRUE;

		j++;
	}
	
	return FALSE;
} 

/**
 * bonobo_event_source_notify_listeners:
 * @event_source: the Event Source that will emit the event.
 * @event_name: Name of the event being emitted
 * @value: A CORBA_any value that contains the data that is passed to interested clients
 * @opt_ev: A CORBA_Environment where a failure code can be returned, can be NULL.
 *
 * This will notify all clients that have registered with this EventSource
 * (through the addListener or addListenerWithMask methods) of the availability
 * of the event named @event_name.  The @value CORBA::any value is passed to
 * all listeners.
 *
 * @event_name can not contain comma separators, as commas are used to
 * separate the various event names. 
 */
void
bonobo_event_source_notify_listeners (BonoboEventSource *event_source,
				      const char        *event_name,
				      const CORBA_any   *value,
				      CORBA_Environment *opt_ev)
{
	GSList *l, *notify;
	CORBA_Environment ev, *my_ev;

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	notify = NULL;

	for (l = event_source->priv->listeners; l; l = l->next) {
		ListenerDesc *desc = (ListenerDesc *) l->data;

		if (desc->event_masks == NULL ||
		    event_match (event_name, desc->event_masks)) {
			CORBA_Object_duplicate (desc->listener, my_ev);
			notify = g_slist_prepend (notify, desc->listener);
		}
	}

	bonobo_object_ref (BONOBO_OBJECT (event_source));

	for (l = notify; l; l = l->next) {
		Bonobo_Listener_event (l->data, event_name, value, my_ev);
		CORBA_Object_release (l->data, my_ev);
	}

	bonobo_object_unref (BONOBO_OBJECT (event_source));

	g_slist_free (notify);

	if (!opt_ev)
		CORBA_exception_free (&ev);
}

void
bonobo_event_source_notify_listeners_full (BonoboEventSource *event_source,
					   const char        *path,
					   const char        *type,
					   const char        *subtype,
					   const CORBA_any   *value,                          
					   CORBA_Environment *opt_ev)
{
	char *event_name;

	event_name = bonobo_event_make_name (path, type, subtype);

	bonobo_event_source_notify_listeners (event_source, event_name,
					      value, opt_ev);

	g_free (event_name);
}

static void
bonobo_event_source_destroy (GtkObject *object)
{
	CORBA_Environment         ev;
	BonoboEventSourcePrivate *priv;
	
	priv = BONOBO_EVENT_SOURCE (object)->priv;

	CORBA_exception_init (&ev);
	
	while (priv->listeners) {
		ListenerDesc *d = priv->listeners->data;

		priv->listeners = g_slist_remove (
			priv->listeners, d);

		desc_free (d, &ev);
	}

	CORBA_exception_free (&ev);

	g_free (priv);

	bonobo_event_source_parent_class->destroy (object);
}

static void
bonobo_event_source_class_init (BonoboEventSourceClass *klass)
{
	GtkObjectClass *oclass = (GtkObjectClass *) klass;
	POA_Bonobo_EventSource__epv *epv = &klass->epv;

	bonobo_event_source_parent_class = gtk_type_class (PARENT_TYPE);

	oclass->destroy = bonobo_event_source_destroy;

	epv->addListener         = impl_Bonobo_EventSource_addListener;
	epv->addListenerWithMask = impl_Bonobo_EventSource_addListenerWithMask;
	epv->removeListener      = impl_Bonobo_EventSource_removeListener;
}

static void
bonobo_event_source_init (GtkObject *object)
{
	BonoboEventSource *event_source;

	event_source = BONOBO_EVENT_SOURCE (object);
	event_source->priv = g_new0 (BonoboEventSourcePrivate, 1);
	event_source->priv->listeners = NULL;
}

BONOBO_X_TYPE_FUNC_FULL (BonoboEventSource, 
			   Bonobo_EventSource,
			   PARENT_TYPE,
			   bonobo_event_source);

/**
 * bonobo_event_source_new:
 *
 * Creates a new BonoboEventSource object.  Typically this
 * object will be exposed to clients through CORBA and they
 * will register and unregister functions to be notified
 * of events that this EventSource generates.
 * 
 * To notify clients of an event, use the bonobo_event_source_notify_listeners()
 * function.
 *
 * Returns: A new #BonoboEventSource server object.
 */
BonoboEventSource *
bonobo_event_source_new (void)
{
	return gtk_type_new (BONOBO_EVENT_SOURCE_TYPE);
}

/**
 * bonobo_event_source_ignore_listeners:
 * @event_source: 
 * 
 *  Instructs the event source to de-register any listeners
 * that are added from the global running context.
 **/
void
bonobo_event_source_ignore_listeners (BonoboEventSource *event_source)
{
	g_return_if_fail (BONOBO_IS_EVENT_SOURCE (event_source));

	event_source->priv->ignore = TRUE;
}

void
bonobo_event_source_client_remove_listener (Bonobo_Unknown  object,
					    Bonobo_EventSource_ListenerId id,
					    CORBA_Environment *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	Bonobo_Unknown es;

	g_return_if_fail (object != CORBA_OBJECT_NIL);
	g_return_if_fail (id != 0);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	if (CORBA_Object_is_a (object, "IDL:Bonobo/Property:1.0", my_ev)) {

		Bonobo_Property_removeListener (object, id, my_ev);

	} else { 

		es = Bonobo_Unknown_queryInterface (object, 
		       "IDL:Bonobo/EventSource:1.0", my_ev);

		if (BONOBO_EX(my_ev) || !es)
			goto remove_listener_end;

		Bonobo_EventSource_removeListener (es, id, my_ev);

		Bonobo_Unknown_unref (es, my_ev);
	}

 remove_listener_end:

	if (!opt_ev) {
		if (BONOBO_EX (my_ev))
			g_warning ("remove_listener failed '%s'",
				   bonobo_exception_get_text (my_ev));
		CORBA_exception_free (&ev);
	}
}

Bonobo_EventSource_ListenerId
bonobo_event_source_client_add_listener (Bonobo_Unknown object,
					 BonoboListenerCallbackFn event_callback,
					 const char *opt_mask,
					 CORBA_Environment *opt_ev,
					 gpointer user_data)
{
	CORBA_Environment ev, *my_ev;
	BonoboListener *listener = NULL;
	Bonobo_Listener corba_listener;
	Bonobo_EventSource_ListenerId id = 0;
	Bonobo_Unknown es;

	g_return_val_if_fail (object != CORBA_OBJECT_NIL, 0);
	g_return_val_if_fail (event_callback != NULL, 0);
	
	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	if (CORBA_Object_is_a (object, "IDL:Bonobo/Property:1.0", my_ev)) {
		if (!(listener = bonobo_listener_new (event_callback, 
						      user_data)))
			goto add_listener_end;
	      
		corba_listener = BONOBO_OBJREF (listener);

		id = Bonobo_Property_addListener (object, corba_listener, 
						  my_ev);

		bonobo_object_unref (BONOBO_OBJECT (listener));

	} else {
		es = Bonobo_Unknown_queryInterface (object, 
		        "IDL:Bonobo/EventSource:1.0", my_ev);

		if (BONOBO_EX(my_ev) || !es)
			goto add_listener_end;

		if (!(listener = bonobo_listener_new (event_callback, 
						      user_data)))
			goto add_listener_end;

		corba_listener = BONOBO_OBJREF (listener);
	
		if (opt_mask)
			id = Bonobo_EventSource_addListenerWithMask (es, 
			        corba_listener, opt_mask, my_ev);
		else 
			id = Bonobo_EventSource_addListener (es, 
				corba_listener, my_ev);

		bonobo_object_unref (BONOBO_OBJECT (listener));
		Bonobo_Unknown_unref (es, my_ev);
	}

 add_listener_end:

	if (!opt_ev) {
		if (BONOBO_EX (my_ev))
			g_warning ("add_listener failed '%s'",
				   bonobo_exception_get_text (my_ev));
		CORBA_exception_free (&ev);
	}

	return id;
} 

