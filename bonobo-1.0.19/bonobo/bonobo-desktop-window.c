/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Bonobo Desktop Window Control implementation.
 * Enables applications to export their geometry control through CORBA.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtkplug.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-desktop-window.h>
#include <gdk/gdkprivate.h>
#include <gdk/gdkx.h>

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

/**
 * bonobo_desktop_window_construct:
 * @desk_win: The BonoboDesktopWindow object to be initialized.
 * @toplevel: Window we will have control over.
 *
 * Returns: the intialized BonoboDesktopWindow object.
 */
BonoboDesktopWindow *
bonobo_desktop_window_construct (BonoboDesktopWindow *desk_win,
				 GtkWindow           *toplevel)
{
	g_return_val_if_fail (GTK_IS_WINDOW (toplevel), NULL);
	g_return_val_if_fail (BONOBO_IS_DESKTOP_WINDOW (desk_win), NULL);
	
	desk_win->window = toplevel;

	return desk_win;
}

/**
 * bonobo_desktop_window_new:
 * @toplevel: The toplevel Gtk window to control
 * container process.
 *
 * Returns: a new BonoboDesktopWindow
 */
BonoboDesktopWindow *
bonobo_desktop_window_new (GtkWindow *toplevel)
{
	BonoboDesktopWindow *desktop_window;
	
	g_return_val_if_fail (GTK_IS_WINDOW (toplevel), NULL);

	desktop_window = gtk_type_new (bonobo_desktop_window_get_type ());
	
	return bonobo_desktop_window_construct (desktop_window, toplevel);
}

static CORBA_char *
impl_Desktop_Window_get_title (PortableServer_Servant servant, CORBA_Environment * ev)
{
	BonoboDesktopWindow *desk_win = BONOBO_DESKTOP_WINDOW (bonobo_object_from_servant (servant));

	return CORBA_string_dup (desk_win->window->title);
}

static void
impl_Desktop_Window_set_title (PortableServer_Servant servant,
			       const CORBA_char *value,
			       CORBA_Environment * ev)
{
	BonoboDesktopWindow *desk_win = BONOBO_DESKTOP_WINDOW (bonobo_object_from_servant (servant));

	gtk_window_set_title (desk_win->window, value);
}

static Bonobo_Desktop_Window_Geometry
impl_Desktop_Window_getGeometry (PortableServer_Servant servant,
				 CORBA_Environment * ev)
{
	BonoboDesktopWindow *desk_win = BONOBO_DESKTOP_WINDOW (bonobo_object_from_servant (servant));
	Bonobo_Desktop_Window_Geometry geo;
	gint x, y;
	
	gdk_window_get_origin (GTK_WIDGET (desk_win->window)->window, &x, &y);
	geo.x = x;
	geo.y = y;
	geo.width = GTK_WIDGET (desk_win->window)->allocation.width;
	geo.height = GTK_WIDGET (desk_win->window)->allocation.height;

	return geo;
}

static void
impl_Desktop_Window_setGeometry (PortableServer_Servant               servant,
				 const Bonobo_Desktop_Window_Geometry *geo,
				 CORBA_Environment                   *ev)
{
	BonoboDesktopWindow *desk_win = BONOBO_DESKTOP_WINDOW (bonobo_object_from_servant (servant));

	gtk_widget_set_uposition (GTK_WIDGET (desk_win->window), geo->x, geo->y);
	gtk_widget_set_usize (GTK_WIDGET (desk_win->window), geo->width, geo->height);
}

static CORBA_unsigned_long
impl_Desktop_Window_getWindowId (PortableServer_Servant servant, CORBA_Environment * ev)
{
	BonoboDesktopWindow *desk_win = BONOBO_DESKTOP_WINDOW (bonobo_object_from_servant (servant));

	return GDK_WINDOW_XWINDOW (GTK_WIDGET (desk_win->window)->window);
}

static void
bonobo_desktop_window_class_init (BonoboDesktopWindowClass *klass)
{
	POA_Bonobo_Desktop_Window__epv *epv = &klass->epv;

	epv->_get_title = impl_Desktop_Window_get_title;
	epv->_set_title = impl_Desktop_Window_set_title;
	epv->getGeometry = impl_Desktop_Window_getGeometry;
	epv->setGeometry = impl_Desktop_Window_setGeometry;
	epv->getWindowId = impl_Desktop_Window_getWindowId;
}

static void
bonobo_desktop_window_init (GtkObject *object)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC_FULL (BonoboDesktopWindow, 
			   Bonobo_Desktop_Window,
			   PARENT_TYPE,
			   bonobo_desktop_window);

/**
 * bonobo_desktop_window_control:
 * @object: Object to be aggregated.
 * @win: Window to be controled.
 *
 * Attaches a Bonobo::Desktop::Window corba handler to a Bonobo
 * object controlling the window @win.  
 */
void
bonobo_desktop_window_control (BonoboObject *object, GtkWindow *win)
{
	BonoboObject *win_control_object;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (win != NULL);
	g_return_if_fail (BONOBO_IS_OBJECT (object));
	g_return_if_fail (GTK_IS_WINDOW (win));

	win_control_object = BONOBO_OBJECT (bonobo_desktop_window_new (win));
	bonobo_object_add_interface (object, win_control_object);
}
