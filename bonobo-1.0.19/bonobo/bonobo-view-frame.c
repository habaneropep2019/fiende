/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-view-frame.c: view frame object.
 *
 * Authors:
 *   Nat Friedman    (nat@helixcode.com)
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtkplug.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-view.h>
#include <bonobo/bonobo-view-frame.h>
#include <gdk/gdkprivate.h>
#include <libgnomeui/gnome-canvas.h>
#include <gdk/gdkkeysyms.h>

enum {
	USER_ACTIVATE,
	USER_CONTEXT,
	LAST_SIGNAL
};

#define PARENT_TYPE BONOBO_CONTROL_FRAME_TYPE

static guint view_frame_signals [LAST_SIGNAL];

/* Parent object class in GTK hierarchy */
static GtkObjectClass *bonobo_view_frame_parent_class;

struct _BonoboViewFramePrivate {
	GtkWidget	  *wrapper; 
	BonoboClientSite  *client_site;
	BonoboUIContainer *ui_container;
	Bonobo_View        view;
};

static Bonobo_ClientSite
impl_Bonobo_ViewFrame_getClientSite (PortableServer_Servant servant,
				     CORBA_Environment *ev)
{
	BonoboViewFrame *view_frame = BONOBO_VIEW_FRAME (bonobo_object_from_servant (servant));

	return bonobo_object_dup_ref (
	        BONOBO_OBJREF (view_frame->priv->client_site), ev);
}

static gboolean
bonobo_view_frame_wrapper_button_press_cb (GtkWidget *wrapper,
					   GdkEventButton *event,
					   gpointer data)
{
	BonoboViewFrame *view_frame = BONOBO_VIEW_FRAME (data);

	bonobo_object_ref (BONOBO_OBJECT (view_frame));

	/* Check for double click. */
	if (event->type == GDK_2BUTTON_PRESS)
		gtk_signal_emit (GTK_OBJECT (view_frame), view_frame_signals [USER_ACTIVATE]);

	/* Check for right click. */
	else if (event->type == GDK_BUTTON_PRESS &&
		 event->button == 3)
		gtk_signal_emit (GTK_OBJECT (view_frame), view_frame_signals [USER_CONTEXT]);
		
	bonobo_object_unref (BONOBO_OBJECT (view_frame));

	return FALSE;
} 

static gboolean
bonobo_view_frame_key_press_cb (GtkWidget *wrapper,
			       GdkEventKey *event,
			       gpointer data)
{
	BonoboViewFrame *view_frame = BONOBO_VIEW_FRAME (data);

	bonobo_object_ref (BONOBO_OBJECT (view_frame));

	/* Hitting enter will activate the embedded component too. */
	if (event->keyval == GDK_Return)
		gtk_signal_emit (GTK_OBJECT (view_frame), view_frame_signals [USER_ACTIVATE]);

	bonobo_object_unref (BONOBO_OBJECT (view_frame));

	return FALSE;
}

/**
 * bonobo_view_frame_construct:
 * @view_frame: The BonoboViewFrame object to be initialized.
 * @wrapper: A BonoboWrapper widget which the new ViewFrame will use to cover its enclosed View.
 * @client_site: the client site to which the newly-created ViewFrame will belong.
 * @ui_container: 
 *
 * Initializes @view_frame with the parameters.
 *
 * Returns: the initialized BonoboViewFrame object @view_frame that implements the
 * Bonobo::ViewFrame CORBA service.
 */
BonoboViewFrame *
bonobo_view_frame_construct (BonoboViewFrame   *view_frame,
			     BonoboClientSite  *client_site,
			     Bonobo_UIContainer ui_container)
{
	GtkWidget *wrapper;

	g_return_val_if_fail (view_frame != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_VIEW_FRAME (view_frame), NULL);
	g_return_val_if_fail (client_site != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_CLIENT_SITE (client_site), NULL);

	bonobo_control_frame_construct (
		BONOBO_CONTROL_FRAME (view_frame), ui_container);

	view_frame->priv->client_site = client_site;
	
	/*
	 * Create the BonoboWrapper which will cover the remote
	 * BonoboView.
	 */
	wrapper = bonobo_wrapper_new ();
	if (wrapper == NULL) {
		bonobo_object_unref (BONOBO_OBJECT (view_frame));
		return NULL;
	}
	gtk_object_ref (GTK_OBJECT (wrapper));
	view_frame->priv->wrapper = wrapper;
	gtk_container_add (GTK_CONTAINER (wrapper),
			   bonobo_control_frame_get_widget (
				   BONOBO_CONTROL_FRAME (view_frame)));

	/*
	 * Connect signal handlers to catch activation events (double
	 * click and hitting Enter) on the wrapper.  These will cause
	 * the ViewFrame to emit the USER_ACTIVATE signal.
	 */
	gtk_signal_connect (GTK_OBJECT (wrapper), "button_press_event",
			    GTK_SIGNAL_FUNC (bonobo_view_frame_wrapper_button_press_cb),
			    view_frame);
	gtk_signal_connect (GTK_OBJECT (wrapper), "key_press_event",
			    GTK_SIGNAL_FUNC (bonobo_view_frame_key_press_cb),
			    view_frame);
	
	return view_frame;
}

/**
 * bonobo_view_frame_new:
 * @client_site: the client site to which the newly-created ViewFrame will belong.
 * @ui_container: A CORBA object for the container's UIContainer server. 
 *
 * Returns: BonoboViewFrame object that implements the
 * Bonobo::ViewFrame CORBA service.
 */
BonoboViewFrame *
bonobo_view_frame_new (BonoboClientSite  *client_site,
		       Bonobo_UIContainer ui_container)
{
	BonoboViewFrame *view_frame;
	
	g_return_val_if_fail (client_site != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_CLIENT_SITE (client_site), NULL);

	view_frame = gtk_type_new (BONOBO_VIEW_FRAME_TYPE);

	return bonobo_view_frame_construct (view_frame, client_site, ui_container);
}

static void
bonobo_view_frame_destroy (GtkObject *object)
{
	BonoboViewFrame *view_frame = BONOBO_VIEW_FRAME (object);

	if (view_frame->priv->view != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (view_frame->priv->view, NULL);
	
	bonobo_view_frame_parent_class->destroy (object);
}

static void
bonobo_view_frame_finalize (GtkObject *object)
{
	BonoboViewFrame *view_frame = BONOBO_VIEW_FRAME (object);

	gtk_object_unref (GTK_OBJECT (view_frame->priv->wrapper));
	g_free (view_frame->priv);
	
	bonobo_view_frame_parent_class->finalize (object);
}

static void
bonobo_view_frame_class_init (BonoboViewFrameClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	POA_Bonobo_ViewFrame__epv *epv = &klass->epv;

	bonobo_view_frame_parent_class = gtk_type_class (PARENT_TYPE);

	view_frame_signals [USER_ACTIVATE] =
		gtk_signal_new ("user_activate",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboViewFrameClass, user_activate),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);

	view_frame_signals [USER_CONTEXT] =
		gtk_signal_new ("user_context",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboViewFrameClass, user_context),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (
		object_class,
		view_frame_signals,
		LAST_SIGNAL);

	object_class->destroy  = bonobo_view_frame_destroy;
	object_class->finalize = bonobo_view_frame_finalize;

	epv->getClientSite = impl_Bonobo_ViewFrame_getClientSite;
}

static void
bonobo_view_frame_init (BonoboObject *object)
{
	BonoboViewFrame *view_frame = BONOBO_VIEW_FRAME (object);

	view_frame->priv = g_new0 (BonoboViewFramePrivate, 1);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboViewFrame, 
			   Bonobo_ViewFrame,
			   PARENT_TYPE,
			   bonobo_view_frame);

/**
 * bonobo_view_frame_bind_to_view:
 * @view_frame: A BonoboViewFrame object.
 * @view: The CORBA object for the BonoboView embedded
 * in this ViewFrame.
 *
 * Associates @view with this @view_frame.
 */
void
bonobo_view_frame_bind_to_view (BonoboViewFrame *view_frame, Bonobo_View view)
{
	g_return_if_fail (view_frame != NULL);
	g_return_if_fail (view != CORBA_OBJECT_NIL);
	g_return_if_fail (BONOBO_IS_VIEW_FRAME (view_frame));

	bonobo_control_frame_bind_to_control (
		BONOBO_CONTROL_FRAME (view_frame),
		(Bonobo_Control) view);
	
	view_frame->priv->view = bonobo_object_dup_ref (view, NULL);
}

/**
 * bonobo_view_frame_get_view:
 * @view_frame: A BonoboViewFrame object.
 * @view: The CORBA object for the BonoboView embedded
 * in this ViewFrame.
 *
 * Associates @view with this @view_frame.
 */
Bonobo_View
bonobo_view_frame_get_view (BonoboViewFrame *view_frame)
{
	g_return_val_if_fail (view_frame != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (BONOBO_IS_VIEW_FRAME (view_frame), CORBA_OBJECT_NIL);

	return view_frame->priv->view;
}

/**
 * bonobo_view_frame_set_covered:
 * @view_frame: A BonoboViewFrame object whose embedded View should be
 * either covered or uncovered.
 * @covered: %TRUE if the View should be covered.  %FALSE if it should
 * be uncovered.
 *
 * This function either covers or uncovers the View embedded in a
 * BonoboViewFrame.  If the View is covered, then the embedded widgets
 * will receive no Gtk events, such as mouse movements, keypresses,
 * and exposures.  When the View is uncovered, all events pass through
 * to the BonoboView's widgets normally.
 */
void
bonobo_view_frame_set_covered (BonoboViewFrame *view_frame, gboolean covered)
{
	GtkWidget *wrapper;

	g_return_if_fail (view_frame != NULL);
	g_return_if_fail (BONOBO_IS_VIEW_FRAME (view_frame));

	wrapper = bonobo_view_frame_get_wrapper (view_frame);
	bonobo_wrapper_set_covered (BONOBO_WRAPPER (wrapper), covered);
}


/**
 * bonobo_view_frame_get_ui_container:
 * @view_frame: A BonoboViewFrame object.
 *
 * Returns: The BonoboUIContainer associated with this ViewFrame.  See
 * also bonobo_view_frame_set_ui_container().
 */
Bonobo_UIContainer
bonobo_view_frame_get_ui_container (BonoboViewFrame *view_frame)
{
	g_return_val_if_fail (view_frame != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_VIEW_FRAME (view_frame), NULL);

	return bonobo_control_frame_get_ui_container (
		BONOBO_CONTROL_FRAME (view_frame));
}

/**
 * bonobo_view_frame_view_activate:
 * @view_frame: The BonoboViewFrame object whose view should be
 * activated.
 *
 * Activates the BonoboView embedded in @view_frame by calling the
 * activate() #Bonobo_Control interface method on it.
 */
void
bonobo_view_frame_view_activate (BonoboViewFrame *view_frame)
{
	g_return_if_fail (view_frame != NULL);
	g_return_if_fail (BONOBO_IS_VIEW_FRAME (view_frame));

	bonobo_control_frame_control_activate (
		BONOBO_CONTROL_FRAME (view_frame));
}


/**
 * bonobo_view_frame_view_deactivate:
 * @view_frame: The BonoboViewFrame object whose view should be
 * deactivated.
 *
 * Deactivates the BonoboView embedded in @view_frame by calling a the
 * activate() CORBA method on it with the parameter %FALSE.
 */
void
bonobo_view_frame_view_deactivate (BonoboViewFrame *view_frame)
{
	g_return_if_fail (view_frame != NULL);
	g_return_if_fail (BONOBO_IS_VIEW_FRAME (view_frame));

	bonobo_control_frame_control_deactivate (
		BONOBO_CONTROL_FRAME (view_frame));
}

/**
 * bonobo_view_frame_get_wrapper:
 * @view_frame: A BonoboViewFrame object.
 *
 * Returns: The BonoboWrapper widget associated with this ViewFrame.
 */
GtkWidget *
bonobo_view_frame_get_wrapper (BonoboViewFrame *view_frame)
{
	g_return_val_if_fail (view_frame != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_VIEW_FRAME (view_frame), NULL);

	return GTK_WIDGET (view_frame->priv->wrapper);
}

/**
 * bonobo_view_frame_set_zoom_factor:
 * @view_frame: A BonoboViewFrame object.
 * @zoom: a zoom factor.  1.0 means one-to-one mapping.
 *
 * Requests the associated view to change its zoom factor the the value in @zoom.
 */
void
bonobo_view_frame_set_zoom_factor (BonoboViewFrame *view_frame, double zoom)
{
	CORBA_Environment ev;

	g_return_if_fail (view_frame != NULL);
	g_return_if_fail (BONOBO_IS_VIEW_FRAME (view_frame));
	g_return_if_fail (zoom > 0.0);

	CORBA_exception_init (&ev);
	Bonobo_View_setZoomFactor (view_frame->priv->view, zoom, &ev);
	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (
			BONOBO_OBJECT (view_frame),
			(CORBA_Object) view_frame->priv->view, &ev);
	}
	CORBA_exception_free (&ev);
}

/**
 * bonobo_view_frame_get_client_site:
 * @view_frame: The view frame
 *
 * Returns the BonoboClientSite associated with this view frame
 */
BonoboClientSite *
bonobo_view_frame_get_client_site (BonoboViewFrame *view_frame)
{
	g_return_val_if_fail (view_frame != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_VIEW_FRAME (view_frame), NULL);

	return view_frame->priv->client_site;
}

