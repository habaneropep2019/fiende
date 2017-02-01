/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* By Owen Taylor <otaylor@gtk.org>              98/4/4 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#undef SOCKET_DEBUG

#include <stdio.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkdnd.h>
#include <bonobo/bonobo-socket.h>
#include <bonobo/bonobo-control-frame.h>



struct _BonoboSocketPrivate {
	/* The control on the other side which we use to gdk_flush() */
	BonoboControlFrame *frame;

	guint16 request_width;
	guint16 request_height;
	guint16 current_width;
	guint16 current_height;

	GdkWindow *plug_window;
	guint same_app : 1;
	guint have_size : 1;
	guint need_map : 1;

	/* Whether we gave the focus to the child through our own ::focus() */
	guint gave_focus : 1;
};

/* Last timestamp we got from the X server, for use with XSetInputFocus() */
static guint32 last_x_time_stamp = CurrentTime;

/* Forward declararations */

static void bonobo_socket_class_init               (BonoboSocketClass    *klass);
static void bonobo_socket_init                     (BonoboSocket         *socket);
static void bonobo_socket_finalize                 (GtkObject            *object);
static void bonobo_socket_realize                  (GtkWidget        *widget);
static void bonobo_socket_unrealize                (GtkWidget        *widget);
static void bonobo_socket_size_request             (GtkWidget      *widget,
						    GtkRequisition *requisition);
static void bonobo_socket_size_allocate            (GtkWidget     *widget,
						    GtkAllocation *allocation);
static gint bonobo_socket_focus_in_event           (GtkWidget *widget,
						    GdkEventFocus *event);
static gint bonobo_socket_focus_out_event          (GtkWidget *widget,
						    GdkEventFocus *event);
static gint bonobo_socket_focus                    (GtkContainer *container,
						    GtkDirectionType direction);

static GdkFilterReturn bonobo_socket_filter_func   (GdkXEvent *gdk_xevent,
						    GdkEvent *event,
						    gpointer data);

/* From Tk */
#define EMBEDDED_APP_WANTS_FOCUS NotifyNormal+20

/* Local data */

static GtkContainerClass *parent_class = NULL;



guint
bonobo_socket_get_type ()
{
	static guint socket_type = 0;

	if (!socket_type)
	{
		static const GtkTypeInfo socket_info =
		{
			"BonoboSocket",
			sizeof (BonoboSocket),
			sizeof (BonoboSocketClass),
			(GtkClassInitFunc) bonobo_socket_class_init,
			(GtkObjectInitFunc) bonobo_socket_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		socket_type = gtk_type_unique (gtk_container_get_type (), &socket_info);
	}

	return socket_type;
}

static void
bonobo_socket_class_init (BonoboSocketClass *class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;

	object_class = (GtkObjectClass*) class;
	widget_class = (GtkWidgetClass*) class;
	container_class = (GtkContainerClass*) class;

	parent_class = gtk_type_class (GTK_TYPE_CONTAINER);

	object_class->finalize = bonobo_socket_finalize;

	widget_class->realize = bonobo_socket_realize;
	widget_class->unrealize = bonobo_socket_unrealize;
	widget_class->size_request = bonobo_socket_size_request;
	widget_class->size_allocate = bonobo_socket_size_allocate;
	widget_class->focus_in_event = bonobo_socket_focus_in_event;
	widget_class->focus_out_event = bonobo_socket_focus_out_event;

	container_class->focus = bonobo_socket_focus;
}

static void
bonobo_socket_init (BonoboSocket *socket)
{
	BonoboSocketPrivate *priv;

	priv = g_new (BonoboSocketPrivate, 1);
	socket->priv = priv;

	priv->frame = NULL;

	priv->request_width = 0;
	priv->request_height = 0;
	priv->current_width = 0;
	priv->current_height = 0;

	priv->plug_window = NULL;
	priv->same_app = FALSE;
	priv->have_size = FALSE;
	priv->need_map = FALSE;
	priv->gave_focus = FALSE;
}

/* Destroy handler for the socket */
static void
bonobo_socket_finalize (GtkObject *object)
{
	BonoboSocket *socket;
	BonoboSocketPrivate *priv;

	g_return_if_fail (object != NULL);
	g_return_if_fail (BONOBO_IS_SOCKET (object));

	socket = BONOBO_SOCKET (object);
	priv = socket->priv;

	g_free (priv);
	socket->priv = NULL;

	if (GTK_OBJECT_CLASS (parent_class)->finalize)
		(* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}

/**
 * bonobo_socket_new:
 *
 * Create a new empty #BonoboSocket.
 * Returns: A new #BonoboSocket.
 */
GtkWidget*
bonobo_socket_new (void)
{
	BonoboSocket *socket;

	socket = gtk_type_new (bonobo_socket_get_type ());

	return GTK_WIDGET (socket);
}

/**
 * bonobo_socket_steal:
 * @socket: the #BonoboSocket.
 * @id: the XID of an existing toplevel window.
 *
 * Reparents a pre-existing toplevel window into a
 * #BonoboSocket.
 */
void
bonobo_socket_steal (BonoboSocket *socket, guint32 id)
{
	BonoboSocketPrivate *priv;
	GtkWidget *widget;

	g_return_if_fail (socket != NULL);
	g_return_if_fail (BONOBO_IS_SOCKET (socket));

	priv = socket->priv;

	widget = GTK_WIDGET (socket);

	priv->plug_window = gdk_window_lookup (id);

	gdk_error_trap_push ();

	if (priv->plug_window && priv->plug_window->user_data) {
		/*
		  GtkWidget *child_widget;

		  child_widget = GTK_WIDGET (priv->plug_window->user_data);
		*/

		g_warning("Stealing from same app not yet implemented");

		priv->same_app = TRUE;
	} else {
		priv->plug_window = gdk_window_foreign_new (id);
		if (!priv->plug_window) {
			/* was deleted before we could get it */
			gdk_error_trap_pop ();
			return;
		}

		priv->same_app = FALSE;
		priv->have_size = FALSE;

		XSelectInput (GDK_DISPLAY (),
			      GDK_WINDOW_XWINDOW (priv->plug_window),
			      StructureNotifyMask | PropertyChangeMask);

		gtk_widget_queue_resize (widget);
	}

	gdk_window_hide (priv->plug_window);
	gdk_window_reparent (priv->plug_window, widget->window, 0, 0);

	gdk_flush ();
	gdk_error_trap_pop ();

	priv->need_map = TRUE;
}

static void
bonobo_socket_realize (GtkWidget *widget)
{
	BonoboSocket *socket;
	GdkWindowAttr attributes;
	gint attributes_mask;
	XWindowAttributes xattrs;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_SOCKET (widget));

	socket = BONOBO_SOCKET (widget);
	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	attributes.event_mask = GDK_FOCUS_CHANGE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
					 &attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, socket);

	widget->style = gtk_style_attach (widget->style, widget->window);
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);

	XGetWindowAttributes (GDK_DISPLAY (),
			      GDK_WINDOW_XWINDOW (widget->window),
			      &xattrs);

	XSelectInput (GDK_DISPLAY (),
		      GDK_WINDOW_XWINDOW(widget->window),
		      xattrs.your_event_mask |
		      SubstructureNotifyMask | SubstructureRedirectMask);

	gdk_window_add_filter (widget->window, bonobo_socket_filter_func, widget);

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

	bonobo_control_frame_sync_realize (socket->priv->frame);
}

static void
bonobo_socket_unrealize (GtkWidget *widget)
{
	BonoboSocket *socket;
	BonoboSocketPrivate *priv;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_SOCKET (widget));

	socket = BONOBO_SOCKET (widget);
	priv = socket->priv;

	if (priv->plug_window) {
		GtkWidget *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (socket));

		if (toplevel && GTK_IS_WINDOW (toplevel))
			gtk_window_remove_embedded_xid (
				GTK_WINDOW (toplevel),
				GDK_WINDOW_XWINDOW (priv->plug_window));
	}

	if (GTK_WIDGET_CLASS (parent_class)->unrealize)
		(* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);

	bonobo_control_frame_sync_unrealize (priv->frame);
}

void
bonobo_socket_set_control_frame (BonoboSocket       *socket,
				 BonoboControlFrame *frame)
{
	g_return_if_fail (BONOBO_IS_SOCKET (socket));

	if (socket->priv)
		socket->priv->frame = frame;
}

static void
bonobo_socket_size_request (GtkWidget      *widget,
			    GtkRequisition *requisition)
{
	BonoboSocket *socket;
	BonoboSocketPrivate *priv;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_SOCKET (widget));
	g_return_if_fail (requisition != NULL);

	socket = BONOBO_SOCKET (widget);
	priv = socket->priv;

	if (!priv->have_size && priv->plug_window) {
		XSizeHints hints;
		long supplied;

		gdk_error_trap_push ();

		if (XGetWMNormalHints (GDK_DISPLAY (),
				       GDK_WINDOW_XWINDOW (priv->plug_window),
				       &hints, &supplied)) {
			/*
			 * This is obsolete, according the X docs, but many
			 * programs still use it.
			 */
			if (hints.flags & (PSize | USSize)) {
				priv->request_width = hints.width;
				priv->request_height = hints.height;
			} else if (hints.flags & PMinSize) {
				priv->request_width = hints.min_width;
				priv->request_height = hints.min_height;
			} else if (hints.flags & PBaseSize) {
				priv->request_width = hints.base_width;
				priv->request_height = hints.base_height;
			}
		}

		priv->have_size = TRUE;	/* don't check again? */
		gdk_error_trap_pop ();
	}

	requisition->width = MAX (priv->request_width, 1);
	requisition->height = MAX (priv->request_height, 1);
}

static void
send_configure_event (BonoboSocket *socket)
{
	BonoboSocketPrivate *priv;
	XEvent event;

	priv = socket->priv;

	g_return_if_fail (priv->plug_window != NULL);

	event.xconfigure.type = ConfigureNotify;
	event.xconfigure.display = gdk_display;

	event.xconfigure.event = GDK_WINDOW_XWINDOW (priv->plug_window);
	event.xconfigure.window = GDK_WINDOW_XWINDOW (priv->plug_window);

	event.xconfigure.x = 0;
	event.xconfigure.y = 0;
	event.xconfigure.width = GTK_WIDGET (socket)->allocation.width;
	event.xconfigure.height = GTK_WIDGET (socket)->allocation.height;

	event.xconfigure.border_width = 0;
	event.xconfigure.above = None;
	event.xconfigure.override_redirect = False;

	gdk_error_trap_push ();
	XSendEvent (gdk_display,
		    GDK_WINDOW_XWINDOW (priv->plug_window),
		    False, NoEventMask, &event);
	gdk_flush ();
	gdk_error_trap_pop ();
}

static void
bonobo_socket_size_allocate (GtkWidget     *widget,
			     GtkAllocation *allocation)
{
	BonoboSocket *socket;
	BonoboSocketPrivate *priv;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_SOCKET (widget));
	g_return_if_fail (allocation != NULL);

	socket = BONOBO_SOCKET (widget);
	priv = socket->priv;

	widget->allocation = *allocation;
	if (GTK_WIDGET_REALIZED (widget)) {
		gdk_window_move_resize (widget->window,
					allocation->x, allocation->y,
					allocation->width, allocation->height);

		if (priv->plug_window) {
			gdk_error_trap_push ();

			if (!priv->need_map &&
			    (allocation->width == priv->current_width) &&
			    (allocation->height == priv->current_height)) {
				send_configure_event (socket);
				GTK_NOTE(PLUGSOCKET,
					 g_message ("BonoboSocket - allocated no change: %d %d",
						    allocation->width, allocation->height));
			} else {
				gdk_window_move_resize (priv->plug_window,
							0, 0,
							allocation->width, allocation->height);
				GTK_NOTE (PLUGSOCKET,
					  g_message ("BonoboSocket - allocated: %d %d",
						     allocation->width, allocation->height));
				priv->current_width = allocation->width;
				priv->current_height = allocation->height;
			}

			if (priv->need_map) {
				gdk_window_show (priv->plug_window);
				priv->need_map = FALSE;
			}

			gdk_flush ();
			gdk_error_trap_pop ();
		}
	}
}

/*
 * Focus_in_event handler for the socket widget.  If we had previously given the
 * focus to the child window, we give it the focus again.  This is so that
 * unfocusing the toplevel window and focusing it back again will restore the
 * focus to our embedded window.
 */
static gint
bonobo_socket_focus_in_event (GtkWidget *widget, GdkEventFocus *event)
{
	BonoboSocket *socket;
	BonoboSocketPrivate *priv;

	g_return_val_if_fail (BONOBO_IS_SOCKET (widget), TRUE);

	socket = BONOBO_SOCKET (widget);
	g_return_val_if_fail (socket->priv != NULL, TRUE);

	priv = socket->priv;

	if (priv->gave_focus && priv->plug_window) {
		gdk_error_trap_push ();
		XSetInputFocus (GDK_DISPLAY (),
				GDK_WINDOW_XWINDOW (priv->plug_window),
				RevertToParent, CurrentTime);
		gdk_flush();
		gdk_error_trap_pop ();
	}

	return TRUE;
}

/* Focus_out_event handler for the socket widget */
static gint
bonobo_socket_focus_out_event (GtkWidget *widget, GdkEventFocus *event)
{
	BonoboSocket *socket;
	BonoboSocketPrivate *priv;
	GtkWidget *toplevel;

	socket = BONOBO_SOCKET (widget);
	priv = socket->priv;

	toplevel = gtk_widget_get_ancestor (widget, GTK_TYPE_WINDOW);

	if (toplevel && GTK_WIDGET_MAPPED (toplevel)) {
		/*
		 * XSetInputFocus() will BadMatch if the window is not viewable;
		 * that is why we put the test for MAPPED above.  Still, any of
		 * its ancestors, namely the window manager frame, may not be
		 * mapped, so we push an error trap anyways.
		 */
		gdk_error_trap_push ();
		XSetInputFocus (GDK_DISPLAY (),
				GDK_WINDOW_XWINDOW (toplevel->window),
				RevertToParent, CurrentTime);
		gdk_flush();
		gdk_error_trap_pop ();
	}

	priv->gave_focus = FALSE;

	return TRUE;
}

static void
claim_focus (BonoboSocket *socket)
{
	BonoboSocketPrivate *priv;

	priv = socket->priv;

	priv->gave_focus = TRUE;

	/* Oh, the trickery... */

	GTK_WIDGET_SET_FLAGS (socket, GTK_CAN_FOCUS);
	gtk_widget_grab_focus (GTK_WIDGET (socket));
	GTK_WIDGET_UNSET_FLAGS (socket, GTK_CAN_FOCUS);

	/*
	 * FIXME: we might grab the focus even if we don't have it as an
	 * app... (and see _focus_in())
	 */
	if (priv->plug_window) {
		gdk_error_trap_push ();
		XSetInputFocus (GDK_DISPLAY (),
				GDK_WINDOW_XWINDOW (priv->plug_window),
				RevertToParent, CurrentTime);
		gdk_flush ();
		gdk_error_trap_pop ();
	}
}

/*
 * Focus handler for the socket widget.  We proxy the focus request to the child
 * Bonobo control and then give it the actual input focus or reclaim it as
 * appropriate.
 */
static gint
bonobo_socket_focus (GtkContainer *container, GtkDirectionType direction)
{
	BonoboSocket *socket;
	BonoboSocketPrivate *priv;

	socket = BONOBO_SOCKET (container);
	priv = socket->priv;

	if (!priv->gave_focus && priv->plug_window) {
		claim_focus (socket);
		return bonobo_control_frame_focus_child (priv->frame, direction);
	} else
		return FALSE;
}

static void
bonobo_socket_add_window (BonoboSocket *socket, guint32 xid)
{
	BonoboSocketPrivate *priv;

	priv = socket->priv;

	priv->plug_window = gdk_window_lookup (xid);
	priv->same_app = TRUE;

	if (!priv->plug_window) {
		GtkWidget *toplevel;
		GdkDragProtocol protocol;

		priv->plug_window = gdk_window_foreign_new (xid);
		if (!priv->plug_window) /* Already gone */
			return;

		priv->same_app = FALSE;

		gdk_error_trap_push ();
		XSelectInput (GDK_DISPLAY (),
			      GDK_WINDOW_XWINDOW (priv->plug_window),
			      StructureNotifyMask | PropertyChangeMask);

		if (gdk_drag_get_protocol (xid, &protocol))
			gtk_drag_dest_set_proxy (GTK_WIDGET (socket), priv->plug_window,
						 protocol, TRUE);
		gdk_flush ();
		gdk_error_trap_pop ();

		gdk_window_add_filter (priv->plug_window,
				       bonobo_socket_filter_func, socket);

		/* Add a pointer to the socket on our toplevel window */

		toplevel = gtk_widget_get_toplevel (GTK_WIDGET (socket));
		if (toplevel && GTK_IS_WINDOW (toplevel))
			gtk_window_add_embedded_xid (GTK_WINDOW (toplevel), xid);
	}
}

static GdkFilterReturn
bonobo_socket_filter_func (GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data)
{
	BonoboSocket *socket;
	BonoboSocketPrivate *priv;
	GtkWidget *widget;
	XEvent *xevent;
	GdkFilterReturn return_val;

	socket = BONOBO_SOCKET (data);
	priv = socket->priv;

	widget = GTK_WIDGET (socket);
	xevent = (XEvent *) gdk_xevent;

	return_val = GDK_FILTER_CONTINUE;

	switch (xevent->type) {
	case KeyPress:
	case KeyRelease:
		last_x_time_stamp = xevent->xkey.time;
		break;

	case ButtonPress:
	case ButtonRelease:
		last_x_time_stamp = xevent->xbutton.time;
		break;

	case MotionNotify:
		last_x_time_stamp = xevent->xmotion.time;
		break;

	case EnterNotify:
	case LeaveNotify:
		last_x_time_stamp = xevent->xcrossing.time;
		break;

	case PropertyNotify:
		last_x_time_stamp = xevent->xproperty.time;
		break;

	default:
		break;
	}

	switch (xevent->type) {
	case CreateNotify: {
		XCreateWindowEvent *xcwe = &xevent->xcreatewindow;

		if (!priv->plug_window) {
			bonobo_socket_add_window (socket, xcwe->window);
			if (!priv->plug_window)
				break;

			gdk_error_trap_push ();
			gdk_window_move_resize (priv->plug_window,
						0, 0,
						widget->allocation.width,
						widget->allocation.height);
			gdk_flush ();
			gdk_error_trap_pop ();

			priv->request_width = xcwe->width;
			priv->request_height = xcwe->height;
			priv->have_size = TRUE;

			GTK_NOTE (PLUGSOCKET,
				  g_message ("BonoboSocket - window created with size: %d %d",
					     priv->request_width,
					     priv->request_height));

			gtk_widget_queue_resize (widget);
		}

		return_val = GDK_FILTER_REMOVE;
		break;
	}

	case ConfigureRequest: {
		XConfigureRequestEvent *xcre = &xevent->xconfigurerequest;

		if (!priv->plug_window)
			bonobo_socket_add_window (socket, xcre->window);

		if (!priv->plug_window)
			break;

		if (xcre->window == GDK_WINDOW_XWINDOW (priv->plug_window)) {
			if (xcre->value_mask & (CWWidth | CWHeight)) {
				priv->request_width = xcre->width;
				priv->request_height = xcre->height;
				priv->have_size = TRUE;

				GTK_NOTE (PLUGSOCKET,
					  g_message ("BonoboSocket - configure request: %d %d",
						     priv->request_width,
						     priv->request_height));

				gtk_widget_queue_resize (widget);
			} else if (xcre->value_mask & (CWX | CWY))
				send_configure_event (socket);

			/* Ignore stacking requests. */
			return_val = GDK_FILTER_REMOVE;
		}

		break;
	}

	case DestroyNotify: {
		XDestroyWindowEvent *xdwe = &xevent->xdestroywindow;

		if (priv->plug_window && (xdwe->window == GDK_WINDOW_XWINDOW (priv->plug_window))) {
			GtkWidget *toplevel;

			GTK_NOTE (PLUGSOCKET, g_message ("BonoboSocket - destroy notify"));

			toplevel = gtk_widget_get_toplevel (GTK_WIDGET (socket));
			if (toplevel && GTK_IS_WINDOW (toplevel))
				gtk_window_remove_embedded_xid (GTK_WINDOW (toplevel), xdwe->window);

			gdk_window_destroy_notify (priv->plug_window);
			gtk_widget_destroy (widget);

			priv->plug_window = NULL;

			return_val = GDK_FILTER_REMOVE;
		}
		break;
	}

	case FocusIn:
		if (xevent->xfocus.mode == EMBEDDED_APP_WANTS_FOCUS)
			claim_focus (socket);
		else if (xevent->xfocus.detail == NotifyInferior) {
#if 0
			GtkWidget *toplevel;
			toplevel = gtk_widget_get_ancestor (widget, gtk_window_get_type());

			if (toplevel)
				XSetInputFocus (GDK_DISPLAY (),
						GDK_WINDOW_XWINDOW (toplevel->window),
						RevertToParent, CurrentTime);
#endif
		}

		return_val = GDK_FILTER_REMOVE;
		break;

	case FocusOut:
		return_val = GDK_FILTER_REMOVE;
		break;

	case MapRequest:
		if (!priv->plug_window)
			bonobo_socket_add_window (socket, xevent->xmaprequest.window);

		if (!priv->plug_window)
			break;

		if (xevent->xmaprequest.window == GDK_WINDOW_XWINDOW (priv->plug_window)) {
			GTK_NOTE (PLUGSOCKET, g_message ("BonoboSocket - Map Request"));

			gdk_error_trap_push ();
			gdk_window_show (priv->plug_window);
			gdk_flush ();
			gdk_error_trap_pop ();

			return_val = GDK_FILTER_REMOVE;
		}
		break;

	case PropertyNotify:
		if (!priv->plug_window)
			break;

		if (xevent->xproperty.window == GDK_WINDOW_XWINDOW (priv->plug_window)) {
			GdkDragProtocol protocol;

			if ((xevent->xproperty.atom == gdk_atom_intern ("XdndAware", FALSE)) ||
			    (xevent->xproperty.atom == gdk_atom_intern ("_MOTIF_DRAG_RECEIVER_INFO",
									FALSE))) {
				gdk_error_trap_push ();
				if (gdk_drag_get_protocol (xevent->xproperty.window, &protocol))
					gtk_drag_dest_set_proxy (GTK_WIDGET (socket),
								 priv->plug_window,
								 protocol, TRUE);
				gdk_flush ();
				gdk_error_trap_pop ();
			}
			return_val = GDK_FILTER_REMOVE;
		}
	}

	return return_val;
}
