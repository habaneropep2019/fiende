/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-view.c: a view object of an embeddable
 *
 * Authors:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Nat Friedman    (nat@nat.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtkplug.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-view.h>
#include <gdk/gdkprivate.h>

#define PARENT_TYPE bonobo_control_get_type ()

/* Parent object class in GTK hierarchy */
static BonoboControlClass *bonobo_view_parent_class;

enum {
	VIEW_UNDO_LAST_OPERATION,
	SET_ZOOM_FACTOR,
	LAST_SIGNAL
};

static guint view_signals [LAST_SIGNAL];

typedef void (*GnomeSignal_NONE__DOUBLE) (GtkObject *object, double arg1, gpointer user_data);

struct _BonoboViewPrivate {
	int placeholder;
};

static void
impl_Bonobo_View_setZoomFactor (PortableServer_Servant servant,
				const CORBA_double zoom,
				CORBA_Environment *ev)
{
	BonoboView *view = BONOBO_VIEW (bonobo_object_from_servant (servant));

	gtk_signal_emit (GTK_OBJECT (view),
			 view_signals [SET_ZOOM_FACTOR], zoom);
}

#if 0
/**
 * bonobo_view_activate:
 * @view: 
 * @activate: 
 * 
 *   A basic default handler so we can at least
 * get focus in a development component; 95% of these
 * will override this to merge menus for them.
 **/
static void
bonobo_view_activate (BonoboView *view, gboolean activate, gpointer user_data)
{
	bonobo_view_activate_notify (view, activate);
}
#endif

/**
 * bonobo_view_construct:
 * @view: The BonoboView object to be initialized.
 * @corba_view: The CORBA Bonobo_View interface for the new BonoboView object.
 * @widget: A GtkWidget contains the view and * will be passed to the container
 * process's ViewFrame object.
 * 
 * @item_creator might be NULL for widget-based views.
 *
 * Returns: the intialized BonoboView object.
 */
BonoboView *
bonobo_view_construct (BonoboView *view, GtkWidget *widget)
{
	g_return_val_if_fail (BONOBO_IS_VIEW (view), NULL);
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
	
	bonobo_control_construct (BONOBO_CONTROL (view), widget);

/*	gtk_signal_connect (GTK_OBJECT (view), "view_activate",
			    GTK_SIGNAL_FUNC (bonobo_view_activate),
			    NULL);*/
	return view;
}

/**
 * bonobo_view_new:
 * @widget: a GTK widget that contains the view and will be passed to the
 * container process.
 *
 * This function creates a new BonoboView object for @widget
 *
 * Returns: a BonoboView object that implements the Bonobo::View CORBA
 * service that will transfer the @widget to the container process.
 */
BonoboView *
bonobo_view_new (GtkWidget *widget)
{
	BonoboView *view;
	
	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	view = gtk_type_new (bonobo_view_get_type ());
	
	return bonobo_view_construct (view, widget);
}

static void
bonobo_view_destroy (GtkObject *object)
{
	BonoboView *view;

	g_return_if_fail (object != NULL);
	g_return_if_fail (BONOBO_IS_VIEW (object));
	
	view = BONOBO_VIEW (object);

	g_free (view->priv);
	
	bonobo_object_unref (BONOBO_OBJECT (view->embeddable));

	GTK_OBJECT_CLASS (bonobo_view_parent_class)->destroy (object);
}

static void 
gnome_marshal_NONE__DOUBLE (GtkObject * object,
			    GtkSignalFunc func,
			    gpointer func_data,
			    GtkArg * args)
{
	GnomeSignal_NONE__DOUBLE rfunc;
	rfunc = (GnomeSignal_NONE__DOUBLE) func;
	(*rfunc) (object,
		  GTK_VALUE_DOUBLE (args[0]),
		  func_data);
}

static void
bonobo_view_class_init (BonoboViewClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	POA_Bonobo_View__epv *epv = &klass->epv;

	bonobo_view_parent_class = gtk_type_class (PARENT_TYPE);

	view_signals [SET_ZOOM_FACTOR] =
                gtk_signal_new ("set_zoom_factor",
                                GTK_RUN_LAST,
                                object_class->type,
                                GTK_SIGNAL_OFFSET (BonoboViewClass, set_zoom_factor), 
                                gnome_marshal_NONE__DOUBLE,
                                GTK_TYPE_NONE, 1,
				GTK_TYPE_DOUBLE);

	gtk_object_class_add_signals (object_class, view_signals, LAST_SIGNAL);

	object_class->destroy = bonobo_view_destroy;

	epv->setZoomFactor = impl_Bonobo_View_setZoomFactor;
}

static void
bonobo_view_init (BonoboView *view)
{
	view->priv = g_new0 (BonoboViewPrivate, 1);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboView, 
			   Bonobo_View,
			   PARENT_TYPE,
			   bonobo_view)

/**
 * bonobo_view_set_embeddable:
 * @view: A BonoboView object.
 * @embeddable: The BonoboEmbeddable object for which @view is a view.
 *
 * This function associates @view with the specified GnomeEmbeddabe
 * object, @embeddable.
 */
void
bonobo_view_set_embeddable (BonoboView *view, BonoboEmbeddable *embeddable)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (BONOBO_IS_VIEW (view));
	g_return_if_fail (embeddable != NULL);
	g_return_if_fail (BONOBO_IS_EMBEDDABLE (embeddable));

	if (view->embeddable != NULL)
		bonobo_object_unref (BONOBO_OBJECT (view->embeddable));

	view->embeddable = embeddable;
	bonobo_object_ref (BONOBO_OBJECT (view->embeddable));
}

/**
 * bonobo_view_get_embeddable:
 * @view: A BonoboView object.
 *
 * Returns: The BonoboEmbeddable object for which @view is a BonoboView.
 */
BonoboEmbeddable *
bonobo_view_get_embeddable (BonoboView *view)
{
	g_return_val_if_fail (view != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_VIEW (view), NULL);

	return view->embeddable;
}

/**
 * bonobo_view_set_view_frame:
 * @view: A BonoboView object.
 * @view_frame: A CORBA interface for the ViewFrame which contains this View.
 *
 * Sets the ViewFrame for @view to @view_frame.
 */
void
bonobo_view_set_view_frame (BonoboView *view, Bonobo_ViewFrame view_frame)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (BONOBO_IS_VIEW (view));
	
	bonobo_control_set_control_frame (BONOBO_CONTROL (view), (Bonobo_ControlFrame) view_frame);

	view->view_frame = view_frame;
}

/**
 * bonobo_view_get_view_frame:
 * @view: A BonoboView object whose Bonobo_ViewFrame CORBA interface is
 * being retrieved.
 *
 * Returns: The Bonobo_ViewFrame CORBA object associated with @view, this is
 * a CORBA_object_duplicated object.  You need to CORBA_free it when you are
 * done with it.
 */
Bonobo_ViewFrame
bonobo_view_get_view_frame (BonoboView *view)

{
	g_return_val_if_fail (view != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (BONOBO_IS_VIEW (view), CORBA_OBJECT_NIL);
	
	return view->view_frame;
}

/**
 * bonobo_view_get_ui_component:
 * @view: A BonoboView object for which a BonoboUIComponent has been created and set.
 *
 * Returns: The BonoboUIComponent which was associated with @view when it was created.
 */
BonoboUIComponent *
bonobo_view_get_ui_component (BonoboView *view)
{
	g_return_val_if_fail (view != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_VIEW (view), NULL);

	return bonobo_control_get_ui_component (BONOBO_CONTROL (view));
}

/**
 * bonobo_view_get_remote_ui_container
 * @view: A BonoboView object which is bound to a remote BonoboViewFrame.
 *
 * Returns: The Bonobo_UIContainer CORBA server for the remote BonoboViewFrame.
 */
Bonobo_UIContainer
bonobo_view_get_remote_ui_container (BonoboView *view)
{
	g_return_val_if_fail (view != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (BONOBO_IS_VIEW (view), CORBA_OBJECT_NIL);

	return bonobo_control_get_remote_ui_container (BONOBO_CONTROL (view));
}

/**
 * bonobo_view_activate_notify:
 * @view: A BonoboView object which is bound to a remote BonoboViewFrame..
 * @activate: %TRUE if the view is activated, %FALSE otherwise.
 *
 * This function notifies @view's remote ViewFrame that the activation
 * state of @view has changed.
 */
void
bonobo_view_activate_notify (BonoboView *view, gboolean activated)
{
	g_return_if_fail (view != NULL);
	g_return_if_fail (BONOBO_IS_VIEW (view));

	bonobo_control_activate_notify (BONOBO_CONTROL (view), activated);
}
