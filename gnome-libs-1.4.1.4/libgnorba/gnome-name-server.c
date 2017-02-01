/*
 * gnome-name-server
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org)
 *   Elliot Lee (sopwith@cuc.edu)
 *
 * This is just a version of the name service that registers its IOR
 * with the X server.  That way GNOME applications running against that
 * display will be able to find the naming service. 
 *
 * If the name server got killed for any reason, it is importnat to have
 * a way to reliably find this situation, we use the following trick to
 * achieve this:
 *
 * The GNOME_NAME_SERVICE property on the X root window is set to point
 * to an invisible window (the window id).  On this window, we store also
 * a GNOME_NAME_SERVICE property that points to itself.  Additionally,
 * a GNOME_NAME_SERVICE_IOR property is kept there with the IOR to this
 * name server.
 *
 * In the event of the name-server being killed, we have a reliable way
 * of finding this out.
 *
 * Code and ideas are based on Owen Taylor's and Federico Mena's proxy window
 * for the Midnight Commander.
 *
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 * Warning: internal ORBit & gnome API's get used here. You probably
 * don't want to use this program as an example of the API's that normal
 * programs should use. 
 */
#include <config.h>

#include <sys/types.h>

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <ORBitservices/CosNaming.h>
#include <ORBitservices/CosNaming_impl.h>
#include "gnorba.h"

/* The widget that points to the proxy window that keeps the IOR alive */
Window proxy_window;
Display *display;

int x_error_code = 0;
GMainLoop *ml;

CORBA_ORB orb;
CORBA_Environment ev;

static int
x_error_handler (Display *disp, XErrorEvent *ev)
{
	x_error_code = ev->error_code;

	return 0;
}

static gboolean
handle_x_connection (GIOChannel *source, GIOCondition cond, Display *disp)
{
	XEvent ev;
	
	if (cond & (G_IO_ERR|G_IO_HUP|G_IO_NVAL)){
		syslog (LOG_INFO, "input condition is: %#x, exiting", cond);
		XCloseDisplay (disp);
		g_main_quit (ml);
		return TRUE;
	}
	
	/* ignore everything */
	while (XPending (disp))
		XNextEvent (disp, &ev);
	
	return TRUE;
}

static gboolean
setup_atomically_name_server_ior (CORBA_char *ior)
{
	Atom name_server_atom, name_server_ior_atom, window_atom, string_atom;
	Atom type;
	int format;
	Window *proxy_data;
	Window proxy, proxy_xid;
	unsigned long nitems, after;
	Window rootwin;
	
	name_server_atom = XInternAtom (display, "GNOME_NAME_SERVER", FALSE);
	name_server_ior_atom = XInternAtom (display, "GNOME_NAME_SERVER_IOR", FALSE);
	window_atom = XInternAtom (display, "WINDOW", FALSE);
	string_atom = XInternAtom (display, "STRING", FALSE);

	rootwin = DefaultRootWindow(display);
	proxy_window = proxy_xid = XCreateSimpleWindow(display,
						       rootwin,
						       -10, -10, 1, 1, 1, 
						       0, 0);
	XGrabServer (display);

	type = None;
	proxy = None;

	x_error_code = 0;

	XGetWindowProperty (display,
			    rootwin, name_server_atom, 0, 1, False,
			    AnyPropertyType,
			    &type, &format, &nitems, &after,
			    (guchar **) &proxy_data);

	if (type != None){
		if ((format == 32) && (nitems == 1))
			proxy = *proxy_data;

		XFree (proxy_data);
	}

	/*
	 * If the property was set, check if the window it poitns to exists and has
	 * a "GNOME_NAME_SERVER" property pointing to itself
	 */
	if (proxy){
		XGetWindowProperty (display, proxy,
				    name_server_atom, 0,
				    1, False, AnyPropertyType,
				    &type, &format, &nitems, &after,
				    (guchar **) &proxy_data);
		
		if (!x_error_code && type != None){
			if ((format == 32) && (nitems == 1))
				if (*proxy_data != proxy)
					proxy = None;
			if (proxy != None) {
				CosNaming_NameComponent nc[1] = {{"GNOME", "subcontext"}};
				CosNaming_Name          nom;
				CORBA_Object tmp, name_service;
				
				nom._length = 1; nom._buffer = nc;
				
				name_service = CORBA_ORB_string_to_object(orb, ior, &ev);
				if (!CORBA_Object_is_nil (name_service, &ev)) {
					tmp = CosNaming_NamingContext_resolve(name_service, &nom, &ev);
					
					if(ev._major == CORBA_NO_EXCEPTION)
						CORBA_Object_release(tmp, &ev);
					else
						proxy = None;
				} else
					proxy = None;
			}
			XFree (proxy_data);
		} else
			proxy = None;

		if (!proxy)
			syslog (LOG_INFO, "Stale reference to the GNOME name server");
	}

	/*
	 * If the window was invalid, set the property to point to us
	 */
	if (!proxy){
		XChangeProperty (
			display, rootwin,
			name_server_atom, window_atom,
			32, PropModeReplace, (guchar *) &proxy_xid, 1);
		XChangeProperty (
			display, proxy_xid,
			name_server_atom, window_atom,
			32, PropModeReplace, (guchar *) &proxy_xid, 1);
		XChangeProperty (
			display, proxy_xid,
			name_server_ior_atom, string_atom,
			8, PropModeReplace, (guchar *) ior, strlen (ior)+1);
	}
	XUngrabServer (display);
	XSync(display, True);

	if (proxy)
		return FALSE;
	else
		return TRUE;
}

static void
signal_handler (int signo)
{
	syslog (LOG_ERR, "Receveived signal %d\nshutting down.", signo);
	
	switch(signo) {
	case SIGSEGV:
	abort();
	
	default:
		exit (1);
	}
}


static gboolean
setup_name_server (CORBA_Object name_service, CORBA_Environment *ev)
{
	CORBA_Object gnome_context  = CORBA_OBJECT_NIL;
	CORBA_Object server_context = CORBA_OBJECT_NIL;
	CosNaming_NameComponent nc [2] =
	{ {"GNOME","subcontext"},
	  {"Servers", "subcontext"} };
	CosNaming_Name context_name;

	context_name._maximum = 2;
	context_name._length = 2;
	context_name._buffer = (CosNaming_NameComponent *)&nc;
	context_name._release = FALSE;

	context_name._length = 1;
	/*
	 * Create the default context "/GNOME/Servers"
	 */
	gnome_context = CosNaming_NamingContext_bind_new_context(name_service, &context_name, ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
		g_warning(_("Creating '/GNOME' context %s %d"), __FILE__, __LINE__);
		switch( ev->_major ) {
		case CORBA_SYSTEM_EXCEPTION:
			g_warning("sysex: %s.\n", CORBA_exception_id(ev));
			break;
		case CORBA_USER_EXCEPTION:
			g_warning("usrex: %s.\n", CORBA_exception_id( ev ) );
		default:
			break;
		}
		return FALSE;
	}
	CORBA_Object_release(gnome_context, ev);
	
	context_name._length = 2;
	server_context = CosNaming_NamingContext_bind_new_context(name_service, &context_name, ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
		g_warning(_("Creating '/GNOME/Servers' context %s %d"), __FILE__, __LINE__);
		switch( ev->_major ) {
		case CORBA_SYSTEM_EXCEPTION:
			g_warning("	sysex: %s.\n", CORBA_exception_id(ev));
			break;
		case CORBA_USER_EXCEPTION:
			g_warning("usr	ex: %s.\n", CORBA_exception_id( ev ) );
		default:
			break;
		}
		return FALSE;
	}

	CORBA_Object_release(server_context, ev);

	return TRUE;
}

extern void _gnome_gnorba_cookie_setup(Display *disp, Window rootwin);

int
main (int argc, char *argv [])
{
	CORBA_Object name_server;
	PortableServer_POA root_poa;
	PortableServer_POAManager pm;
	struct sigaction act;
	sigset_t empty_mask;
	CORBA_char *ior;
	gboolean v;
	GIOChannel *channel;
	
	if (getenv("GNOME_NAME_SERVER_DEBUG")){
		volatile int spinme = 1;
		while (spinme)
			;
	}

	/* Logs */
	openlog ("gnome-name-server", LOG_NDELAY | LOG_PID, LOG_DAEMON);
	syslog (LOG_INFO, "starting");

	/* Session setup */
	sigemptyset (&empty_mask);
	act.sa_handler = signal_handler;
	act.sa_mask    = empty_mask;
	act.sa_flags   = 0;
	sigaction (SIGINT,  &act, 0);
	sigaction (SIGHUP,  &act, 0);
	sigaction (SIGSEGV, &act, 0);
	sigaction (SIGABRT, &act, 0);

	act.sa_handler = SIG_IGN;
	sigaction (SIGINT, &act, 0);
	
	sigemptyset (&empty_mask);

	chdir ("/");

	CORBA_exception_init (&ev);
	display = XOpenDisplay (getenv ("DISPLAY"));
	g_return_val_if_fail (display, 1);
	XSetErrorHandler (x_error_handler);

	channel = g_io_channel_unix_new(ConnectionNumber(display));
	g_io_add_watch_full(channel, G_PRIORITY_DEFAULT,
			    G_IO_IN|G_IO_ERR|G_IO_HUP|G_IO_NVAL, 
			    (GIOFunc)handle_x_connection,
			    display, NULL);
	g_io_channel_unref (channel);

	ml = g_main_new(FALSE);

	orb = gnorba_CORBA_init (&argc, argv, GNORBA_INIT_DISABLE_COOKIES, &ev);
	_gnome_gnorba_cookie_setup(display, DefaultRootWindow(display));
	root_poa = (PortableServer_POA)
		CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);

	name_server = impl_CosNaming_NamingContext__create (root_poa, &ev);
	if (!setup_name_server (name_server, &ev)){
		syslog (LOG_INFO, "Could not setup the name server\n");
		CORBA_Object_release (name_server, &ev);
		return 1;
	}

	ior = CORBA_ORB_object_to_string (orb, name_server, &ev);

	v = setup_atomically_name_server_ior (ior);

	if (!v){
		syslog (LOG_INFO, "name server was running on display, exiting");
		CORBA_Object_release (name_server, &ev);
		return 1;
	} else
		syslog (LOG_INFO, "name server starting");

	printf ("%s\n", ior);
	fflush (stdout);
	CORBA_free (ior);

	pm = PortableServer_POA__get_the_POAManager (root_poa, &ev);
	PortableServer_POAManager_activate (pm, &ev);
	
	g_main_run (ml);

	syslog (LOG_INFO, "exiting");

	return 0;
}
