/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-property-control.c: Bonobo PropertyControl implementation
 *
 * Author:
 *      Iain Holmes  <iain@helixcode.com>
 *
 * Copyright 2000 Helix Code, Inc.
 */
#include <config.h>
#include <stdio.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtktypeutils.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-property-control.h>
#include <bonobo/bonobo-event-source.h>
#include "Bonobo.h"

struct _BonoboPropertyControlPrivate {
	BonoboPropertyControlGetControlFn get_fn;
	BonoboEventSource *event_source;

	void *closure;
	int page_count;
};

enum {
	ACTION,
	LAST_SIGNAL
};

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

static GtkObjectClass *parent_class;

static guint32 signals[LAST_SIGNAL] = { 0 };

static CORBA_long
impl_Bonobo_PropertyControl__get_pageCount (PortableServer_Servant servant,
					    CORBA_Environment *ev)
{
	BonoboObject *bonobo_object;
	BonoboPropertyControl *property_control;
	BonoboPropertyControlPrivate *priv;

	bonobo_object = bonobo_object_from_servant (servant);
	property_control = BONOBO_PROPERTY_CONTROL (bonobo_object);
	priv = property_control->priv;

	return priv->page_count;
}
       
static Bonobo_Control
impl_Bonobo_PropertyControl_getControl (PortableServer_Servant servant,
					CORBA_long pagenumber,
					CORBA_Environment *ev)
{
	BonoboObject *bonobo_object;
	BonoboPropertyControl *property_control;
	BonoboPropertyControlPrivate *priv;
	BonoboControl *control;

	bonobo_object = bonobo_object_from_servant (servant);
	property_control = BONOBO_PROPERTY_CONTROL (bonobo_object);
	priv = property_control->priv;

	if (pagenumber < 0 || pagenumber >= priv->page_count) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_PropertyControl_NoPage, NULL);
		return CORBA_OBJECT_NIL;
	}

	control = priv->get_fn (property_control, 
			        pagenumber,
		                priv->closure);

	if (control == NULL)
		return CORBA_OBJECT_NIL;

	return (Bonobo_Control) CORBA_Object_duplicate 
		(BONOBO_OBJREF (control), ev);
}

static void
impl_Bonobo_PropertyControl_notifyAction (PortableServer_Servant servant,
					  CORBA_long pagenumber,
					  Bonobo_PropertyControl_Action action,
					  CORBA_Environment *ev)
{
	BonoboObject *bonobo_object;
	BonoboPropertyControl *property_control;
	BonoboPropertyControlPrivate *priv;

	bonobo_object = bonobo_object_from_servant (servant);
	property_control = BONOBO_PROPERTY_CONTROL (bonobo_object);
	priv = property_control->priv;

	if (pagenumber < 0 || pagenumber >= priv->page_count) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_PropertyControl_NoPage, NULL);
		return;
	}
	
	gtk_signal_emit (GTK_OBJECT (bonobo_object), signals [ACTION], pagenumber, action);
}

static void
bonobo_property_control_destroy (GtkObject *object)
{
	BonoboPropertyControl *property_control;
	
	property_control = BONOBO_PROPERTY_CONTROL (object);
	if (property_control->priv == NULL)
		return;

	g_free (property_control->priv);
	property_control->priv = NULL;

	parent_class->destroy (object);
}

static void
bonobo_property_control_class_init (BonoboPropertyControlClass *klass)
{
	GtkObjectClass *object_class;
	POA_Bonobo_PropertyControl__epv *epv = &klass->epv;

	object_class = GTK_OBJECT_CLASS (klass);
	object_class->destroy = bonobo_property_control_destroy;

	signals [ACTION] = gtk_signal_new ("action",
					 GTK_RUN_FIRST, object_class->type,
					 GTK_SIGNAL_OFFSET (BonoboPropertyControlClass, action),
					 gtk_marshal_NONE__INT_INT, GTK_TYPE_NONE,
					 2, GTK_TYPE_INT, GTK_TYPE_ENUM);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
				     
	parent_class = gtk_type_class (PARENT_TYPE);

	epv->_get_pageCount = impl_Bonobo_PropertyControl__get_pageCount;
	epv->getControl     = impl_Bonobo_PropertyControl_getControl;
	epv->notifyAction   = impl_Bonobo_PropertyControl_notifyAction;
}

static void
bonobo_property_control_init (BonoboPropertyControl *property_control)
{
	BonoboPropertyControlPrivate *priv;

	priv = g_new (BonoboPropertyControlPrivate, 1);
	priv->get_fn = NULL;
	priv->closure = NULL;
	priv->page_count = 0;

	property_control->priv = priv;
}

BONOBO_X_TYPE_FUNC_FULL (BonoboPropertyControl, 
			   Bonobo_PropertyControl,
			   PARENT_TYPE,
			   bonobo_property_control);

/**
 * bonobo_property_control_construct:
 * @property_control: A BonoboPropertyControl object.
 * @event_source: A BonoboEventSource object that will be aggregated onto the
 * property control.
 * @get_fn: Creation routine.
 * @closure: Data passed to closure routine.
 *
 * Initialises the BonoboPropertyControl object.
 *
 * Returns: The newly constructed BonoboPropertyControl.
 */
BonoboPropertyControl *
bonobo_property_control_construct (BonoboPropertyControl            *property_control,
				   BonoboEventSource                *event_source,
				   BonoboPropertyControlGetControlFn get_fn,
				   int                               num_pages,
				   void                             *closure)
{
	BonoboPropertyControlPrivate *priv;

	g_return_val_if_fail (BONOBO_IS_EVENT_SOURCE (event_source), NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_CONTROL (property_control), NULL);

	priv = property_control->priv;
	priv->get_fn = get_fn;
	priv->page_count = num_pages;
	priv->closure = closure;

	priv->event_source = event_source;
	bonobo_object_add_interface (BONOBO_OBJECT (property_control),
				     BONOBO_OBJECT (priv->event_source));

	return property_control;
}

/**
 * bonobo_property_control_new_full:
 * @get_fn: The function to be called when the getControl method is called.
 * @num_pages: The number of pages this property control has.
 * @event_source: The event source to use to emit events on.
 * @closure: The data to be passed into the @get_fn routine.
 *
 * Creates a BonoboPropertyControl object.
 *
 * Returns: A pointer to a newly created BonoboPropertyControl object.
 */
BonoboPropertyControl *
bonobo_property_control_new_full (BonoboPropertyControlGetControlFn get_fn,
				  int                               num_pages,
				  BonoboEventSource                *event_source,
				  void                             *closure)
{
	BonoboPropertyControl *property_control;

	g_return_val_if_fail (num_pages > 0, NULL);
	g_return_val_if_fail (BONOBO_IS_EVENT_SOURCE (event_source), NULL);

	property_control = gtk_type_new (bonobo_property_control_get_type ());
					
	return bonobo_property_control_construct (
		property_control, event_source, get_fn, num_pages, closure);
}

/**
 * bonobo_property_control_new:
 * @get_fn: The function to be called when the getControl method is called.
 * @num_pages: The number of pages this property control has.
 * @closure: The data to be passed into the @get_fn routine
 *
 * Creates a BonoboPropertyControl object.
 *
 * Returns: A pointer to a newly created BonoboPropertyControl object.
 */
BonoboPropertyControl *
bonobo_property_control_new (BonoboPropertyControlGetControlFn get_fn,
			     int   num_pages,
			     void *closure)
{
	BonoboEventSource *event_source;

	g_return_val_if_fail (num_pages > 0, NULL);

	event_source = bonobo_event_source_new ();

	return bonobo_property_control_new_full (
		get_fn, num_pages, event_source, closure);
}

/**
 * bonobo_property_control_changed:
 * @property_control: The BonoboPropertyControl that has changed.
 * @opt_ev: An optional CORBA_Environment for exception handling. 
 *
 * Tells the server that a value in the property control has been changed,
 * and that it should indicate this somehow.
 */
void
bonobo_property_control_changed (BonoboPropertyControl *property_control,
				 CORBA_Environment     *opt_ev)
{
	BonoboPropertyControlPrivate *priv;
	CORBA_Environment ev;
	CORBA_any any;
	CORBA_short s;

	g_return_if_fail (property_control != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_CONTROL (property_control));

	priv = property_control->priv;

	if (opt_ev == NULL)
		CORBA_exception_init (&ev);
	else
		ev = *opt_ev;

	s = 0;
	any._type = (CORBA_TypeCode) TC_short;
	any._value = &s;

	bonobo_event_source_notify_listeners (priv->event_source,
					      BONOBO_PROPERTY_CONTROL_CHANGED,
					      &any, &ev);
	if (opt_ev == NULL && BONOBO_EX (&ev)) {
		g_warning ("ERROR: %s", CORBA_exception_id (&ev));
	}

	if (opt_ev == NULL)
		CORBA_exception_free (&ev);
}

/**
 * bonobo_property_control_get_event_source:
 * @property_control: The BonoboPropertyControl.
 * 
 * Returns the BonoboEventSource that @property_control uses.
 * Returns: A BonoboEventSource.
 */
BonoboEventSource *
bonobo_property_control_get_event_source (BonoboPropertyControl *property_control)
{
	g_return_val_if_fail (property_control != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_CONTROL (property_control), NULL);

	return property_control->priv->event_source;
}
