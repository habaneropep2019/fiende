/* -*- mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Bonobo control object
 *
 * Author:
 *   Nat Friedman      (nat@helixcode.com)
 *   Miguel de Icaza   (miguel@helixcode.com)
 *   Maciej Stachowiak (mjs@eazel.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 *                 2000 Eazel, Inc.
 */
#include <config.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-plug.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-exception.h>
#include <gdk/gdkprivate.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkmain.h>

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

enum {
	SET_FRAME,
	ACTIVATE,
	LAST_SIGNAL
};

static guint control_signals [LAST_SIGNAL];

/* Parent object class in GTK hierarchy */
static BonoboObjectClass *bonobo_control_parent_class;

struct _BonoboControlPrivate {
	GtkWidget                  *widget;
	Bonobo_ControlFrame         control_frame;
	gboolean                    active;

	GtkWidget                  *plug;
	GtkWidget                  *socket;
	gboolean                    is_local;
	gboolean                    xid_received;
	guint                       destroy_idle_id;
			
	BonoboUIComponent          *ui_component;
	gboolean                    automerge;
				   
	BonoboPropertyBag          *propbag;
};

/**
 * window_id_demangle:
 * @id: CORBA_char *
 * 
 * De-mangle a window id string,
 * fields are separated by ':' character,
 * currently only the first field is used.
 * 
 * Return value: the X11 window id.
 **/
inline static guint32
window_id_demangle (Bonobo_Control_windowId id)
{
	guint32 x11_id;
	char **elements;
	
/*	printf ("ID string '%s'\n", id);*/

	elements = g_strsplit (id, ":", -1);
	if (elements && elements [0])
		x11_id = strtol (elements [0], NULL, 10);
	else {
		g_warning ("Serious X id mangling error");
		x11_id = 0;
	}
	g_strfreev (elements);

/*	printf ("x11 : %d\n", x11_id);*/

	return x11_id;
}

/**
 * bonobo_control_windowid_from_x11:
 * @x11_id: the x11 window id.
 * 
 * This mangles the X11 name into the ':' delimited
 * string format "X-id: ..."
 * 
 * Return value: the string; free after use.
 **/
Bonobo_Control_windowId
bonobo_control_windowid_from_x11 (guint32 x11_id)
{
	CORBA_char *str;

	str = g_strdup_printf ("%d", x11_id);

/*	printf ("Mangled %d to '%s'\n", x11_id, str);*/
	return str;
}

/*
 * This callback is invoked when the plug is unexpectedly destroyed by
 * way of its associated X window dying.  This usually indicates that
 * the container application has died.  This callback is _not_ invoked
 * if the BonoboControl is destroyed normally, i.e. the user unrefs
 * the BonoboControl away.
 */
static gboolean
bonobo_control_plug_destroy_event_cb (GtkWidget   *plug,
				      GdkEventAny *event,
				      gpointer     closure)
{
	BonoboControl *control = BONOBO_CONTROL (closure);

	if (control->priv->plug == NULL)
		return FALSE;

	if (control->priv->plug != plug)
		g_warning ("Destroying incorrect plug!");

	/*
	 * Set the plug to NULL here so that we don't try to
	 * destroy it later.  It will get destroyed on its
	 * own.
	 */
	control->priv->plug            = NULL;

	/*
	 * Destroy this plug's BonoboControl.
	 */
	bonobo_object_unref (BONOBO_OBJECT (control));

	return FALSE;
}

/*
 * This callback is invoked when the plug is unexpectedly destroyed
 * through normal Gtk channels. FIXME FIXME FIXME 
 *
 */
static void
bonobo_control_plug_destroy_cb (GtkWidget *plug,
				gpointer   closure)
{
	BonoboControl *control = BONOBO_CONTROL (closure);

	if (control->priv->plug == NULL)
		return;

	if (control->priv->plug != plug)
		g_warning ("Destroying incorrect plug!");

	/*
	 * Set the plug to NULL here so that we don't try to
	 * destroy it later.  It will get destroyed on its
	 * own.
	 */
	control->priv->plug = NULL;
}


static void
bonobo_control_auto_merge (BonoboControl *control)
{
	Bonobo_UIContainer remote_container;

	if (control->priv->ui_component == NULL)
		return;

	remote_container = bonobo_control_get_remote_ui_container (control);
	if (remote_container == CORBA_OBJECT_NIL)
		return;

	bonobo_ui_component_set_container (
		control->priv->ui_component, remote_container);

	bonobo_object_release_unref (remote_container, NULL);
}


static void
bonobo_control_auto_unmerge (BonoboControl *control)
{
	if (control->priv->ui_component == NULL)
		return;
	
	bonobo_ui_component_unset_container (control->priv->ui_component);
}

static void
impl_Bonobo_Control_activate (PortableServer_Servant servant,
			      CORBA_boolean activated,
			      CORBA_Environment *ev)
{
	BonoboControl *control = BONOBO_CONTROL (bonobo_object_from_servant (servant));

	if (control->priv->automerge && control->priv->active != activated) {
		if (activated)
			bonobo_control_auto_merge (control);
		else
			bonobo_control_auto_unmerge (control);
	}

	if (control->priv->active != activated)
		gtk_signal_emit (GTK_OBJECT (control), control_signals [ACTIVATE], (gboolean) activated);

	control->priv->active = activated;
}

	
static void
impl_Bonobo_Control_setFrame (PortableServer_Servant servant,
			       Bonobo_ControlFrame frame,
			       CORBA_Environment *ev)
{
	BonoboControl *control = BONOBO_CONTROL (bonobo_object_from_servant (servant));

	bonobo_control_set_control_frame (control, frame);
}


static GtkWidget *
bonobo_gtk_widget_from_x11_id (guint32 xid)
{
	GdkWindow *window;
	gpointer data;

	window = gdk_window_lookup (xid);
	
	if (!window)
		return NULL;

	gdk_window_get_user_data(window, &data);

	if (!GTK_IS_WIDGET (data))
		return NULL;
	else
		return GTK_WIDGET (data);
}

static gint
idle_destroy_socket (gpointer data)
{
	BonoboControl *control = BONOBO_CONTROL (data);

	g_return_val_if_fail (control != NULL, FALSE);

	control->priv->destroy_idle_id = 0;

	gtk_widget_destroy (control->priv->socket);

	return FALSE;
}


static void
remove_destroy_idle (GtkWidget *socket,
		     BonoboControl *control)
{
	if (control->priv->destroy_idle_id != 0)
		gtk_idle_remove (control->priv->destroy_idle_id);

	control->priv->destroy_idle_id = 0;
}

static void
impl_Bonobo_Control_setWindowId (PortableServer_Servant  servant,
				 Bonobo_Control_windowId id,
				 CORBA_Environment      *ev)
{
	BonoboControl *control = BONOBO_CONTROL (bonobo_object_from_servant (servant));
	GtkWidget     *local_socket;
	guint32        x11_id;

	g_return_if_fail (control->priv->widget != NULL);

	x11_id = window_id_demangle (id);

	/*
	 * Check to see if this XID is local to the application.  In
	 * that case, we bypass the GtkPlug/GtkSocket mechanism and
	 * embed the control directly into the widget hierarchy.  This
	 * avoids a lot of the problems that Plug/Socket give us.
	 */
	local_socket = bonobo_gtk_widget_from_x11_id (x11_id);

	if (! local_socket) {
		GtkWidget *old_plug;

		old_plug            = control->priv->plug;

		/* Create the new plug */
		control->priv->plug = bonobo_plug_new (x11_id);

		gtk_signal_connect_while_alive (GTK_OBJECT (control->priv->plug), "destroy_event",
						GTK_SIGNAL_FUNC (bonobo_control_plug_destroy_event_cb),
						control, GTK_OBJECT (control));
		gtk_signal_connect_while_alive (GTK_OBJECT (control->priv->plug), "destroy",
						GTK_SIGNAL_FUNC (bonobo_control_plug_destroy_cb),
						control, GTK_OBJECT (control));

		/*
		 * Put the control widget inside the plug.  If we
		 * already have a plug, then reparent the control into
		 * the new plug.
		 */
		if (control->priv->xid_received) {

			if (old_plug != NULL) {
				gtk_object_unref (GTK_OBJECT (old_plug));
			}

			gtk_widget_reparent (control->priv->widget, control->priv->plug);
		} else {
 			gtk_container_add (GTK_CONTAINER (control->priv->plug), control->priv->widget);
		}

		gtk_widget_show (control->priv->plug);

		control->priv->is_local = FALSE;

	} else {
		GtkWidget *socket_parent;

		if (control->priv->xid_received)
			return;

		control->priv->is_local = TRUE;

		socket_parent = local_socket->parent;
		gtk_widget_hide (local_socket);

		control->priv->socket = local_socket;
		control->priv->destroy_idle_id = gtk_idle_add (
			idle_destroy_socket, control);

		gtk_signal_connect_while_alive (GTK_OBJECT (local_socket),
						"destroy",
						remove_destroy_idle,
						control, GTK_OBJECT (control));


		gtk_box_pack_end (GTK_BOX (socket_parent),
				  control->priv->widget,
				  TRUE, TRUE, 0);
	}

	control->priv->xid_received = TRUE;
}

static void
impl_Bonobo_Control_setSize (PortableServer_Servant  servant,
			     const CORBA_short       width,
			     const CORBA_short       height,
			     CORBA_Environment      *ev)
{
	/*
	 * Nothing.
	 *
	 * In the Gnome implementation of Bonobo, all size negotiation
	 * is handled by GtkPlug/GtkSocket for us, or GtkFrame in the
	 * local case.
	 */
}

static void
impl_Bonobo_Control_getDesiredSize (PortableServer_Servant  servant,
				    CORBA_short            *desired_width,
				    CORBA_short            *desired_height,
				    CORBA_Environment      *ev)
{
	BonoboControl *control = BONOBO_CONTROL (bonobo_object_from_servant (servant));
	GtkRequisition requisition;

	gtk_widget_size_request (control->priv->widget, &requisition);

	*desired_width = requisition.width;
	*desired_height = requisition.height;
}

static GtkStateType
bonobo_control_gtk_state_from_corba (const Bonobo_Control_State state)
{
	switch (state) {
	case Bonobo_Control_StateNormal:
		return GTK_STATE_NORMAL;

	case Bonobo_Control_StateActive:
		return GTK_STATE_ACTIVE;

	case Bonobo_Control_StatePrelight:
		return GTK_STATE_PRELIGHT;

	case Bonobo_Control_StateSelected:
		return GTK_STATE_SELECTED;

	case Bonobo_Control_StateInsensitive:
		return GTK_STATE_INSENSITIVE;

	default:
		g_warning ("bonobo_control_gtk_state_from_corba: Unknown state: %d", (gint) state);
		return GTK_STATE_NORMAL;
	}
}

static void
impl_Bonobo_Control_setState (PortableServer_Servant      servant,
			       const Bonobo_Control_State  state,
			       CORBA_Environment          *ev)
{
	BonoboControl *control = BONOBO_CONTROL (bonobo_object_from_servant (servant));
	GtkStateType gtk_state = bonobo_control_gtk_state_from_corba (state);

	g_return_if_fail (control->priv->widget != NULL);

	if (gtk_state == GTK_STATE_INSENSITIVE)
		gtk_widget_set_sensitive (control->priv->widget, FALSE);
	else {
		if (! GTK_WIDGET_SENSITIVE (control->priv->widget))
			gtk_widget_set_sensitive (control->priv->widget, TRUE);

		gtk_widget_set_state (control->priv->widget,
				      gtk_state);
	}
}

static Bonobo_PropertyBag
impl_Bonobo_Control_getProperties (PortableServer_Servant  servant,
				      CORBA_Environment      *ev)
{
	BonoboControl *control = BONOBO_CONTROL (bonobo_object_from_servant (servant));
	Bonobo_PropertyBag corba_propbag;

	if (control->priv->propbag == NULL)
		return CORBA_OBJECT_NIL;

	corba_propbag = BONOBO_OBJREF (control->priv->propbag);

	return bonobo_object_dup_ref (corba_propbag, ev);
}

static void
process_events (PortableServer_Servant servant)
{
	BonoboControl *control =
		BONOBO_CONTROL (bonobo_object_from_servant (servant));

	g_return_if_fail (control != NULL);
	g_return_if_fail (control->priv != NULL);

	if (!control->priv->is_local) {
		while (gtk_events_pending ())
			gtk_main_iteration ();
		gdk_flush ();
	}
}

static void
impl_Bonobo_Control_realize (PortableServer_Servant servant,
			     CORBA_Environment     *ev)
{
	process_events (servant);
}

static void
impl_Bonobo_Control_unrealize (PortableServer_Servant servant,
			       CORBA_Environment     *ev)
{
	process_events (servant);
}

static CORBA_boolean
impl_Bonobo_Control_focusChild (PortableServer_Servant servant,
				Bonobo_Control_FocusDirection corba_direction,
				CORBA_Environment *ev)
{
	BonoboControl *control;
	BonoboControlPrivate *priv;
	GtkDirectionType direction;

	control = BONOBO_CONTROL (bonobo_object_from_servant (servant));
	priv = control->priv;

	if (!priv->plug)
		return FALSE;

	switch (corba_direction) {
	case Bonobo_Control_TAB_FORWARD:
		direction = GTK_DIR_TAB_FORWARD;
		break;

	case Bonobo_Control_TAB_BACKWARD:
		direction = GTK_DIR_TAB_BACKWARD;
		break;

	case Bonobo_Control_UP:
		direction = GTK_DIR_UP;
		break;

	case Bonobo_Control_DOWN:
		direction = GTK_DIR_DOWN;
		break;

	case Bonobo_Control_LEFT:
		direction = GTK_DIR_LEFT;
		break;

	case Bonobo_Control_RIGHT:
		direction = GTK_DIR_RIGHT;
		break;

	default:
		/* Hmmm, we should throw an exception. */
		return FALSE;
	}

	bonobo_plug_clear_focus_chain (BONOBO_PLUG (priv->plug));
	return gtk_container_focus (GTK_CONTAINER (priv->plug), direction);
}

BonoboControl *
bonobo_control_construct (BonoboControl  *control,
			  GtkWidget      *widget)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	/*
	 * This sets up the X handler for Bonobo objects.  We basically will
	 * ignore X errors if our container dies (because X will kill the
	 * windows of the container and our container without telling us).
	 */
	bonobo_setup_x_error_handler ();

	control->priv->widget = GTK_WIDGET (widget);
	gtk_object_ref (GTK_OBJECT (widget));
	gtk_object_sink (GTK_OBJECT (widget));

	control->priv->ui_component = NULL;
	control->priv->propbag = NULL;

	return control;
}

/**
 * bonobo_control_new:
 * @widget: a GTK widget that contains the control and will be passed to the
 * container process.
 *
 * This function creates a new BonoboControl object for @widget.
 *
 * Returns: a BonoboControl object that implements the Bonobo::Control CORBA
 * service that will transfer the @widget to the container process.
 */
BonoboControl *
bonobo_control_new (GtkWidget *widget)
{
	BonoboControl *control;
	
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	control = gtk_type_new (bonobo_control_get_type ());
	
	return bonobo_control_construct (control, widget);
}

/**
 * bonobo_control_get_widget:
 * @control: a BonoboControl
 *
 * Returns the GtkWidget associated with a BonoboControl.
 *
 * Return value: the BonoboControl's widget
 **/
GtkWidget *
bonobo_control_get_widget (BonoboControl *control)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	return control->priv->widget;
}

/**
 * bonobo_control_set_automerge:
 * @control: A #BonoboControl.
 * @automerge: Whether or not menus and toolbars should be
 * automatically merged when the control is activated.
 *
 * Sets whether or not the control handles menu/toolbar merging
 * automatically.  If automerge is on, the control will automatically
 * register its BonoboUIComponent with the remote BonoboUIContainer
 * when it is activated.
 */
void
bonobo_control_set_automerge (BonoboControl *control,
			      gboolean       automerge)
{
	g_return_if_fail (BONOBO_IS_CONTROL (control));

	control->priv->automerge = automerge;

	if (automerge && !control->priv->ui_component)
		control->priv->ui_component = bonobo_ui_component_new_default ();
}

/**
 * bonobo_control_get_automerge:
 * @control: A #BonoboControl.
 *
 * Returns: Whether or not the control is set to automerge its
 * menus/toolbars.  See bonobo_control_set_automerge().
 */
gboolean
bonobo_control_get_automerge (BonoboControl *control)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), FALSE);

	return control->priv->automerge;
}

static void
bonobo_control_destroy (GtkObject *object)
{
	BonoboControl *control = BONOBO_CONTROL (object);
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	if (control->priv->destroy_idle_id != 0) {
		gtk_idle_remove (control->priv->destroy_idle_id);
	}
	control->priv->destroy_idle_id = 0;

	if (control->priv->propbag)
		bonobo_object_unref (BONOBO_OBJECT (control->priv->propbag));
	control->priv->propbag = NULL;

	if (control->priv->control_frame != CORBA_OBJECT_NIL) {
		if (control->priv->active)
			Bonobo_ControlFrame_activated (control->priv->control_frame,
						       FALSE, &ev);
		
		CORBA_Object_release (control->priv->control_frame, &ev);
	}

	CORBA_exception_free (&ev);

	/*
	 * If we have a UIComponent, destroy it.
	 */
	if (control->priv->ui_component != NULL) {
		bonobo_ui_component_unset_container (control->priv->ui_component);
		bonobo_object_unref (BONOBO_OBJECT (control->priv->ui_component));
	}

	GTK_OBJECT_CLASS (bonobo_control_parent_class)->destroy (object);
}

static void
bonobo_control_finalize (GtkObject *object)
{
	BonoboControl *control = BONOBO_CONTROL (object);

	/*
	 * Destroy the control's top-level widget.
	 */
	if (control->priv->widget)
		gtk_object_unref (GTK_OBJECT (control->priv->widget));

	/*
	 * If the plug still exists, destroy it.  The plug might not
	 * exist in the case where the container application died,
	 * taking the plug out with it, or the optimized local case
	 * where the plug/socket mechanism was bypassed.  In the
	 * formaer case, plug_destroy_cb() would have been invoked,
	 * and it would have triggered the destruction of the Control,
	 * which is why we're here now. In the latter case, it's not
	 * needed because there is no plug.  
	 */
	if (control->priv->plug) {
		gtk_object_destroy (GTK_OBJECT (control->priv->plug));
		control->priv->plug = NULL;
	}

	g_free (control->priv);

	GTK_OBJECT_CLASS (bonobo_control_parent_class)->finalize (object);
}

/**
 * bonobo_control_set_control_frame:
 * @control: A BonoboControl object.
 * @control_frame: A CORBA interface for the ControlFrame which contains this Controo.
 *
 * Sets the ControlFrame for @control to @control_frame.
 */
void
bonobo_control_set_control_frame (BonoboControl *control, Bonobo_ControlFrame control_frame)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_CONTROL (control));

	CORBA_exception_init (&ev);

	if (control->priv->control_frame != CORBA_OBJECT_NIL)
		CORBA_Object_release (control->priv->control_frame, &ev);
	
	if (control_frame == CORBA_OBJECT_NIL)
		control->priv->control_frame = CORBA_OBJECT_NIL;
	else
		control->priv->control_frame = CORBA_Object_duplicate (control_frame, &ev);
	
	CORBA_exception_free (&ev);

	gtk_signal_emit (GTK_OBJECT (control), control_signals [SET_FRAME]);
}

/**
 * bonobo_control_get_control_frame:
 * @control: A BonoboControl object whose Bonobo_ControlFrame CORBA interface is
 * being retrieved.
 *
 * Returns: The Bonobo_ControlFrame CORBA object associated with @control, this is
 * a CORBA_Object_duplicated object.  You need to CORBA_Object_release it when you are
 * done with it.
 */
Bonobo_ControlFrame
bonobo_control_get_control_frame (BonoboControl *control)
{
	Bonobo_ControlFrame control_frame;
	CORBA_Environment ev;
	
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);
	control_frame = CORBA_Object_duplicate (control->priv->control_frame, &ev);
	CORBA_exception_free (&ev);

	return control_frame;
}

/**
 * bonobo_control_get_ui_component:
 * @control: The control
 * 
 * Return value: the associated UI component
 **/
BonoboUIComponent *
bonobo_control_get_ui_component (BonoboControl *control)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	if (!control->priv->ui_component)
		control->priv->ui_component = bonobo_ui_component_new_default ();

	return control->priv->ui_component;
}

void
bonobo_control_set_ui_component (BonoboControl     *control,
				 BonoboUIComponent *component)
{
	g_return_if_fail (BONOBO_IS_CONTROL (control));
	g_return_if_fail (BONOBO_IS_UI_COMPONENT (component));

	if (control->priv->ui_component)
		bonobo_object_unref (BONOBO_OBJECT (control->priv->ui_component));
	
	control->priv->ui_component = component;
}

/**
 * bonobo_control_set_properties:
 * @control: A #BonoboControl object.
 * @pb: A #BonoboPropertyBag.
 *
 * Binds @pb to @control.  When a remote object queries @control
 * for its property bag, @pb will be used in the responses.
 */
void
bonobo_control_set_properties (BonoboControl *control, BonoboPropertyBag *pb)
{
	BonoboPropertyBag *old_bag;

	g_return_if_fail (BONOBO_IS_CONTROL (control));
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG (pb));

	old_bag = control->priv->propbag;
	control->priv->propbag = pb;

	if (pb)
		bonobo_object_ref (BONOBO_OBJECT (pb));

	if (old_bag)
		bonobo_object_unref (BONOBO_OBJECT (old_bag));
}

/**
 * bonobo_control_get_properties:
 * @control: A #BonoboControl whose PropertyBag has already been set.
 *
 * Returns: The #BonoboPropertyBag bound to @control.
 */
BonoboPropertyBag *
bonobo_control_get_properties (BonoboControl *control)
{
	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	return control->priv->propbag;
}

/**
 * bonobo_control_get_ambient_properties:
 * @control: A #BonoboControl which is bound to a remote
 * #BonoboControlFrame.
 * @ev: CORBA exception environment.
 *
 * Returns: A #Bonobo_PropertyBag bound to the bag of ambient
 * properties associated with this #Control's #ControlFrame.
 */
Bonobo_PropertyBag
bonobo_control_get_ambient_properties (BonoboControl     *control,
				       CORBA_Environment *ev)
{
	Bonobo_ControlFrame control_frame;
	Bonobo_PropertyBag pbag;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_val_if_fail (BONOBO_IS_CONTROL (control), NULL);

	control_frame = control->priv->control_frame;

	if (control_frame == CORBA_OBJECT_NIL)
		return NULL;

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	pbag = Bonobo_ControlFrame_getAmbientProperties (
		control_frame, real_ev);

	if (BONOBO_EX (real_ev)) {
		if (!ev)
			CORBA_exception_free (&tmp_ev);
		pbag = CORBA_OBJECT_NIL;
	}

	return pbag;
}

/**
 * bonobo_control_get_remote_ui_container:
 * @control: A BonoboControl object which is associated with a remote
 * ControlFrame.
 *
 * Returns: The Bonobo_UIContainer CORBA server for the remote BonoboControlFrame.
 */
Bonobo_UIContainer
bonobo_control_get_remote_ui_container (BonoboControl *control)
{
	CORBA_Environment  ev;
	Bonobo_UIContainer ui_container;

	g_return_val_if_fail (BONOBO_IS_CONTROL (control), CORBA_OBJECT_NIL);

	g_return_val_if_fail (control->priv->control_frame != CORBA_OBJECT_NIL,
			      CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);

	ui_container = Bonobo_ControlFrame_getUIHandler (control->priv->control_frame, &ev);

	bonobo_object_check_env (BONOBO_OBJECT (control), control->priv->control_frame, &ev);

	CORBA_exception_free (&ev);

	return ui_container;
}

/**
 * bonobo_control_activate_notify:
 * @control: A #BonoboControl object which is bound
 * to a remote ControlFrame.
 * @activated: Whether or not @control has been activated.
 *
 * Notifies the remote ControlFrame which is associated with
 * @control that @control has been activated/deactivated.
 */
void
bonobo_control_activate_notify (BonoboControl *control,
				gboolean      activated)
{
	CORBA_Environment ev;

	g_return_if_fail (BONOBO_IS_CONTROL (control));
	g_return_if_fail (control->priv->control_frame != CORBA_OBJECT_NIL);
	
	CORBA_exception_init (&ev);

	Bonobo_ControlFrame_activated (control->priv->control_frame, activated, &ev);

	bonobo_object_check_env (BONOBO_OBJECT (control), control->priv->control_frame, &ev);

	CORBA_exception_free (&ev);
}

static void
bonobo_control_class_init (BonoboControlClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *)klass;
	POA_Bonobo_Control__epv *epv;

	bonobo_control_parent_class = gtk_type_class (PARENT_TYPE);

	control_signals [SET_FRAME] =
                gtk_signal_new ("set_frame",
                                GTK_RUN_LAST,
                                object_class->type,
                                GTK_SIGNAL_OFFSET (BonoboControlClass, set_frame),
                                gtk_marshal_NONE__NONE,
                                GTK_TYPE_NONE, 0);

	control_signals [ACTIVATE] =
                gtk_signal_new ("activate",
                                GTK_RUN_LAST,
                                object_class->type,
                                GTK_SIGNAL_OFFSET (BonoboControlClass, activate),
                                gtk_marshal_NONE__BOOL,
                                GTK_TYPE_NONE, 1,
				GTK_TYPE_BOOL);

	gtk_object_class_add_signals (object_class, control_signals, LAST_SIGNAL);

	object_class->destroy  = bonobo_control_destroy;
	object_class->finalize = bonobo_control_finalize;

	epv = &klass->epv;

	epv->activate       = impl_Bonobo_Control_activate;
	epv->setSize        = impl_Bonobo_Control_setSize;
	epv->setWindowId    = impl_Bonobo_Control_setWindowId;
	epv->setState       = impl_Bonobo_Control_setState;
	epv->setFrame       = impl_Bonobo_Control_setFrame;
	epv->getDesiredSize = impl_Bonobo_Control_getDesiredSize;
	epv->getProperties  = impl_Bonobo_Control_getProperties;
	epv->realize        = impl_Bonobo_Control_realize;
	epv->unrealize      = impl_Bonobo_Control_unrealize;
	epv->focusChild     = impl_Bonobo_Control_focusChild;
}

static void
bonobo_control_init (BonoboControl *control)
{
	control->priv = g_new0 (BonoboControlPrivate, 1);

	control->priv->control_frame = CORBA_OBJECT_NIL;
}

BONOBO_X_TYPE_FUNC_FULL (BonoboControl, 
			 Bonobo_Control,
			 PARENT_TYPE,
			 bonobo_control);

void
bonobo_control_set_property (BonoboControl       *control,
			     const char          *first_prop,
			     ...)
{
	Bonobo_PropertyBag  bag;
	char               *err;
	CORBA_Environment   ev;
	va_list             args;

	g_return_if_fail (first_prop != NULL);
	g_return_if_fail (BONOBO_IS_CONTROL (control));

	va_start (args, first_prop);

	CORBA_exception_init (&ev);

	bag = BONOBO_OBJREF (control->priv->propbag);

	if ((err = bonobo_property_bag_client_setv (bag, &ev, first_prop, args)))
		g_warning ("Error '%s'", err);

	CORBA_exception_free (&ev);

	va_end (args);
}

void
bonobo_control_get_property (BonoboControl       *control,
			     const char          *first_prop,
			     ...)
{
	Bonobo_PropertyBag  bag;
	char               *err;
	CORBA_Environment   ev;
	va_list             args;

	g_return_if_fail (first_prop != NULL);
	g_return_if_fail (BONOBO_IS_CONTROL (control));

	va_start (args, first_prop);

	CORBA_exception_init (&ev);

	bag = BONOBO_OBJREF (control->priv->propbag);

	if ((err = bonobo_property_bag_client_getv (bag, &ev, first_prop, args)))
		g_warning ("Error '%s'", err);

	CORBA_exception_free (&ev);

	va_end (args);
}
