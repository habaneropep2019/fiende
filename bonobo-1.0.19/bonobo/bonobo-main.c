/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-main.c: Bonobo Main
 *
 * Author:
 *    Miguel de Icaza  (miguel@gnu.org)
 *    Nat Friedman     (nat@nat.org)
 *    Peter Wainwright (prw@wainpr.demo.co.uk)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-context.h>

#include <signal.h>
#include <gtk/gtkmain.h>
#include <liboaf/liboaf.h>

#include <X11/Xlib.h>
#include <libintl.h>

CORBA_ORB                 __bonobo_orb;
PortableServer_POA        __bonobo_poa;
PortableServer_POAManager __bonobo_poa_manager = NULL;

/**
 * bonobo_orb:
 *
 * fetches the bonobo orb
 *
 * Returns: The ORB used for this Bonobo application.  The ORB
 * is created in bonobo_init().
 */
CORBA_ORB
bonobo_orb (void)
{
	return __bonobo_orb;
}

/**
 * bonobo_poa:
 *
 * fetches the bonobo poa
 *
 * Returns: The POA used for this Bonobo application.  The POA
 * is created when bonobo_init() is called.
 */
PortableServer_POA
bonobo_poa (void)
{
	return __bonobo_poa;
}

/**
 * bonobo_poa_manager:
 *
 * fetches the bonobo poa manager.
 *
 * Returns: The POA Manager used for this Bonobo application.  The POA
 * Manager is created when bonobo_init() is called, but it is not
 * activated until bonobo_main() is called.
 */
PortableServer_POAManager
bonobo_poa_manager (void)
{
	return __bonobo_poa_manager;
}

static int (*gdk_x_error) (Display *, XErrorEvent *);

static int
bonobo_x_error_handler (Display *display, XErrorEvent *error)
{
	if (!error->error_code)
		return 0;

	/*
	 * If we got a BadDrawable or a BadWindow, we ignore it for
	 * now.  FIXME: We need to somehow distinguish real errors
	 * from X-server-induced errors.  Keeping a list of windows
	 * for which we will ignore BadDrawables would be a good idea.
	 */
	if (error->error_code == BadDrawable ||
	    error->error_code == BadWindow)
		return 0;

	/*
	 * Otherwise, let gdk deal.
	 */

	return gdk_x_error (display, error);
}

/**
 * bonobo_setup_x_error_handler:
 *
 * To do graphical embedding in the X window system, Bonobo
 * uses the classic foreign-window-reparenting trick.  The
 * GtkPlug/GtkSocket widgets are used for this purpose.  However,
 * serious robustness problems arise if the GtkSocket end of the
 * connection unexpectedly dies.  The X server sends out DestroyNotify
 * events for the descendants of the GtkPlug (i.e., your embedded
 * component's windows) in effectively random order.  Furthermore, if
 * you happened to be drawing on any of those windows when the
 * GtkSocket was destroyed (a common state of affairs), an X error
 * will kill your application.
 *
 * To solve this latter problem, Bonobo sets up its own X error
 * handler which ignores certain X errors that might have been
 * caused by such a scenario.  Other X errors get passed to gdk_x_error
 * normally.
 */
void
bonobo_setup_x_error_handler (void)
{
	static gboolean error_handler_setup = FALSE;

	if (error_handler_setup)
		return;

	error_handler_setup = TRUE;

	gdk_x_error = XSetErrorHandler (bonobo_x_error_handler);
}

/**
 * bonobo_init:
 * @orb: the ORB in which we run
 * @poa: optional, a POA.
 * @manager: optional, a POA Manager
 *
 * Initializes the bonobo document model.  It requires at least
 * the value for @orb.  If @poa is CORBA_OBJECT_NIL, then the
 * RootPOA will be used, in this case @manager should be CORBA_OBJECT_NIL.
 *
 * Returns %TRUE on success, or %FALSE on failure.
 */
gboolean
bonobo_init (CORBA_ORB orb, PortableServer_POA poa, PortableServer_POAManager manager)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	/*
	 * In Bonobo, components and containers must not crash if the
	 * remote end crashes.  If a remote server crashes and then we
	 * try to make a CORBA call on it, we may get a SIGPIPE.  So,
	 * for lack of a better solution, we ignore SIGPIPE here.  This
	 * is open for reconsideration in the future.
	 *
	 * When SIGPIPE is ignored, write() calls which would
	 * ordinarily trigger a signal will instead return -1 and set
	 * errno to EPIPE.  So ORBit will be able to catch these
	 * errors instead of letting them kill the component.
	 *
	 * Possibilities are the MSG_PEEK trick, where you test if the
	 * connection is dead right before doing the writev().  That
	 * approach has two problems:
	 *
	 *   1. There is the possibility of a race condition, where
	 *      the remote end calls right after the test, and right
	 *      before the writev().
	 * 
	 *   2. An extra system call per write might be regarded by
	 *      some as a performance hit.
	 *
	 * Another possibility is to surround the call to writev() in
	 * ORBit (giop-msg-buffer.c:197 or so) with something like
	 * this:
	 *
	 *		orbit_ignore_sigpipe = 1;
	 *
	 *		result = writev ( ... );
	 *
	 *		orbit_ignore_sigpipe = 0;
	 *
	 * The SIGPIPE signal handler will check the global
	 * orbit_ignore_sigpipe variable and ignore the signal if it
	 * is 1.  If it is 0, it can proxy to the user's original
	 * signal handler.  This is a real possibility.
	 */
	signal (SIGPIPE, SIG_IGN);

	/*
	 * Create the POA.
	 */
	if (orb == CORBA_OBJECT_NIL) {
		orb = oaf_orb_get ();
		if (orb == CORBA_OBJECT_NIL) {
			g_warning ("Can not resolve initial reference to ORB");
			CORBA_exception_free (&ev);
			return FALSE;
		}
	}
	
	if (CORBA_Object_is_nil ((CORBA_Object) poa, &ev)) {
		poa = (PortableServer_POA) CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);
		if (BONOBO_EX (&ev)) {
			g_warning ("Can not resolve initial reference to RootPOA");
			CORBA_exception_free (&ev);
			return FALSE;
		}
		
	}

	/*
	 * Create the POA Manager.
	 */
	if (CORBA_Object_is_nil ((CORBA_Object)manager, &ev)){
		manager = PortableServer_POA__get_the_POAManager (poa, &ev);
		if (BONOBO_EX (&ev)){
			g_warning ("Can not get the POA manager");
			CORBA_exception_free (&ev);
			return FALSE;
		}
	}

	/*
	 * Store global copies of these which can be retrieved with
	 * bonobo_orb()/bonobo_poa()/bonobo_poa_manager().
	 */
	__bonobo_orb = orb;
	__bonobo_poa = poa;
	__bonobo_poa_manager = manager;

	CORBA_exception_free (&ev);

	bonobo_object_init ();

	bonobo_context_init ();

	bindtextdomain (PACKAGE, BONOBO_LOCALEDIR);

	return TRUE;
}

/**
 * bonobo_activate:
 *
 * Activates the Bonobo POA manager registered by bonobo_init.
 * This should be called at the end of application initialization.
 * You do not need to call this function if you use bonobo_main().
 * 
 * Returns %TRUE on success, or %FALSE on failure.
 */
gboolean
bonobo_activate (void)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	if (!__bonobo_poa_manager) {
		g_warning ("Tried to activate Bonobo before initializing");
		CORBA_exception_free (&ev);
		return FALSE;
	}
	PortableServer_POAManager_activate (__bonobo_poa_manager, &ev);
	if (BONOBO_EX (&ev)){
		g_warning ("Failed to activate the Bonobo POA manager");
		CORBA_exception_free (&ev);
		return FALSE;
	}

	CORBA_exception_free (&ev);
	
	return TRUE;
}

/**
 * bonobo_main:
 * 
 * Activates the Bonobo POA Manager and enters the main event loop.
 */
void
bonobo_main (void)
{
	bonobo_activate ();
	gtk_main ();
}
