/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  liboaf: A library for accessing oafd in a nice way.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Elliot Lee <sopwith@redhat.com>
 *
 */

#include "config.h"
#include "oaf-i18n.h"
#include "liboaf/liboaf-private.h"

#include "liboaf/oaf-mainloop.h"
#include "liboaf/oaf-registration.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <popt.h>

/****************** ORBit-specific stuff ****************/

#ifdef HAVE_ORB_ORBIT_H

#include <orb/orbit.h>

static int oaf_corba_prio = G_PRIORITY_LOW;

#ifndef ORBIT_USES_GLIB_MAIN_LOOP

static gboolean
orb_handle_connection (GIOChannel * source, GIOCondition cond,
		       GIOPConnection * cnx)
{
	/* The best way to know about an fd exception is if select()/poll()
	 * tells you about it, so we just relay that information on to ORBit
	 * if possible
	 */

	if (cond & (G_IO_HUP | G_IO_NVAL | G_IO_ERR))
		giop_main_handle_connection_exception (cnx);
	else
		giop_main_handle_connection (cnx);

	return TRUE;
}

static void
orb_add_connection (GIOPConnection * cnx)
{
	int tag;
	GIOChannel *channel;

	channel = g_io_channel_unix_new (GIOP_CONNECTION_GET_FD (cnx));
	tag = g_io_add_watch_full (channel, oaf_corba_prio,
				   G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
				   (GIOFunc) orb_handle_connection,
				   cnx, NULL);
	g_io_channel_unref (channel);

	cnx->user_data = GUINT_TO_POINTER (tag);
}

static void
orb_remove_connection (GIOPConnection * cnx)
{
	g_source_remove (GPOINTER_TO_UINT (cnx->user_data));
	cnx->user_data = GINT_TO_POINTER (-1);
}

#endif /* !ORBIT_USES_GLIB_MAIN_LOOP */


static CORBA_ORB oaf_orb = CORBA_OBJECT_NIL;
static CORBA_Context oaf_context;
static gboolean is_initialized = FALSE;

/* prevent registering with OAF when oaf_active_server_register() */
gboolean oaf_private = FALSE;


/**
 * oaf_orb_get:
 *
 * Returns the ORB used by OAF.
 *
 * Return value: the ORB used by OAF.
 */
CORBA_ORB
oaf_orb_get (void)
{
	return oaf_orb;
}

const char *
oaf_hostname_get (void)
{
	static char *hostname = NULL;
	char hn_tmp[65], ha_tmp[4];
	struct hostent *hent;

	if (!hostname) {
		gethostname (hn_tmp, sizeof (hn_tmp) - 1);

		hent = gethostbyname (hn_tmp);
		if (hent) {
			memcpy (ha_tmp, hent->h_addr, 4);
			hent = gethostbyaddr (ha_tmp, 4, AF_INET);
			if (hent)
				hostname = g_strdup (hent->h_name);
			else
				hostname =
					g_strdup (inet_ntoa
						  (*
						   ((struct in_addr *)
						    ha_tmp)));
		} else
			hostname = g_strdup (hn_tmp);
	}

	return hostname;
}


CORBA_Context
oaf_context_get (void)
{
	return oaf_context;
}

const char *
oaf_session_name_get (void)
{
	const char *dumbptr = "local";

	return dumbptr;
}

const char *
oaf_domain_get (void)
{
	return "session";
}

CORBA_Object
oaf_internal_activation_context_get_extended (gboolean           existing_only,
                                               CORBA_Environment *ev)
{
	OAFBaseService base_service = {};

	base_service.name = "IDL:OAF/ActivationContext:1.0";
	base_service.session_name = oaf_session_name_get ();
	base_service.domain = "session";

	return oaf_internal_service_get_extended (&base_service, existing_only,
                                                   ev);
}

CORBA_Object
oaf_activation_context_get (void)
{
	OAFBaseService base_service = {};

	base_service.name = "IDL:OAF/ActivationContext:1.0";
	base_service.session_name = oaf_session_name_get ();
	base_service.domain = "session";

	return oaf_service_get (&base_service);
}

CORBA_Object
oaf_object_directory_get (const char *username,
                          const char *hostname,
                          const char *domain)
{
        OAFBaseService base_service = {};

        base_service.name = "IDL:OAF/ObjectDirectory:1.0";
        base_service.session_name = oaf_session_name_get ();
        base_service.username = username;
        base_service.hostname = hostname;
        base_service.domain = domain;
        
        return oaf_service_get (&base_service);
}

static char *oaf_od_ior = NULL;
static int oaf_ior_fd = 1;
static char *oaf_activate_iid = NULL;

struct poptOption oaf_popt_options[] = {
  {NULL, '\0', POPT_ARG_INTL_DOMAIN, PACKAGE, 0, NULL, NULL},
  {"oaf-od-ior", '\0', POPT_ARG_STRING, &oaf_od_ior, 0,
   N_("Object directory to use when registering servers"), "IOR"},
  {"oaf-ior-fd", '\0', POPT_ARG_INT, &oaf_ior_fd, 0,
   N_("File descriptor to print IOR on"), N_("FD")},
  {"oaf-activate-iid", '\0', POPT_ARG_STRING, &oaf_activate_iid, 0,
   N_("IID to activate"), "IID"},
  {"oaf-private", '\0', POPT_ARG_NONE, &oaf_private, 0,
   N_("Prevent registering of server with OAF"), NULL},
  {NULL}
};

/**
 * oaf_activation_iid_get:
 *
 * If this process was launched to activate an exe server, this
 * function gives the IID of the server requested, otherwise it
 * returns NULL.
 * 
 * Return value: The IID of the activated server or NULL.
 */

const char *
oaf_activation_iid_get (void)
{
	return oaf_activate_iid;
}

int
oaf_ior_fd_get (void)
{
	return oaf_ior_fd;
}

/* If it is specified on the command line, it overrides everything else */
static char *
cmdline_check (const OAFBaseServiceRegistry *registry,
	       const OAFBaseService *base_service,
               int *distance,
	       gpointer user_data)
{
	if (!strcmp (base_service->name, "IDL:OAF/ObjectDirectory:1.0")) {
		*distance = 0;
		return g_strdup (oaf_od_ior?oaf_od_ior:getenv("OAF_OD_IOR"));
	}

	return NULL;
}

static OAFBaseServiceRegistry cmdline_registry = {
	NULL,
	NULL,
	cmdline_check,
	NULL,
	NULL
};

/* If it is specified on the command line, it overrides everything else */
static char *
ac_check (const OAFBaseServiceRegistry *registry,
	  const OAFBaseService *base_service, 
          int *ret_distance,
	  gpointer user_data)
{
	if (!strcmp (base_service->name, "IDL:OAF/ObjectDirectory:1.0")) {
		OAF_ActivationContext ac;
		OAF_ObjectDirectoryList *od;
		CORBA_Environment ev;
		char *retval, *str_ior;

		ac = oaf_activation_context_get ();

		CORBA_exception_init (&ev);
		if (CORBA_Object_is_nil (ac, &ev))
			return NULL;

		od = OAF_ActivationContext__get_directories (ac, &ev);
		if (ev._major != CORBA_NO_EXCEPTION) {
			CORBA_exception_free (&ev);
			return NULL;
		}

		if (od->_length < 1) {
			CORBA_free (od);
			CORBA_exception_free (&ev);
			return NULL;
		}

		str_ior =
			CORBA_ORB_object_to_string (oaf_orb_get (),
						    od->_buffer[0], &ev);
		if (ev._major != CORBA_NO_EXCEPTION) {
			CORBA_free (od);
			CORBA_exception_free (&ev);
			return NULL;
		}
		retval = g_strdup (str_ior);
		CORBA_free (str_ior);

		*ret_distance = 1;

		CORBA_free (od);

		return retval;
	}

	return NULL;
}

static OAFBaseServiceRegistry ac_registry = {
	NULL,
	NULL,
	ac_check,
	NULL,
	NULL
};

#define STRMATCH(x, y) ((!x && !y) || (x && y && !strcmp(x, y)))

static CORBA_Object
local_activator (const OAFBaseService *base_service,
                 const char **cmd,
		 int fd_arg, 
                 CORBA_Environment *ev)
{
	if (
	    (!base_service->username
	     || STRMATCH (base_service->username, g_get_user_name ()))
	    && (!base_service->hostname
		|| STRMATCH (base_service->hostname, oaf_hostname_get ()))
	    && (!base_service->domain
		|| STRMATCH (base_service->domain, oaf_domain_get ()))) {
		return oaf_server_by_forking (cmd, fd_arg, NULL, NULL, ev);
	}

	return CORBA_OBJECT_NIL;
}

void
oaf_preinit (gpointer app, gpointer mod_info)
{
}

void
oaf_postinit (gpointer app, gpointer mod_info)
{
	oaf_registration_activator_add (local_activator, 0);

	oaf_registration_location_add (&ac_registry, -500, NULL);

	oaf_rloc_file_register ();

	if (oaf_ior_fd > 2)
		fcntl (oaf_ior_fd, F_SETFD, FD_CLOEXEC);

	if (oaf_od_ior)
		oaf_registration_location_add (&cmdline_registry, -1000, NULL);

        if (oaf_activate_iid)
                g_timeout_add_full (G_PRIORITY_LOW,
                                    OAF_FACTORY_TIMEOUT,
                                    oaf_timeout_reg_check,
                                    NULL, NULL);
        else
                oaf_timeout_reg_check_set (FALSE);

	is_initialized = TRUE;
}

#ifdef OAF_DEBUG
static void
do_barrier (int signum)
{
	volatile int barrier = 1;

	while (barrier);
}
#endif

/**
 * oaf_is_initialized:
 *
 * Tells you whether or not OAF is initialized.
 *
 * Return value: whether OAF is initialized or not.
 */
gboolean
oaf_is_initialized (void)
{
	return is_initialized;
}


/**
 * oaf_init:
 *
 * Get the table name to use for the oaf popt options table when
 * registering with libgnome
 * 
 * Return value: A localized copy of the string "OAF options"
 */

char *
oaf_get_popt_table_name ()
{
        bindtextdomain (PACKAGE, OAF_LOCALEDIR);
        return _("OAF options");
}


/**
 * oaf_init:
 * @argc: number of command-line arguments passed to the program.
 * @argv: array of strings containing the command-line 
 *        arguments of the program.
 *
 * Initializes liboaf. Should be called before any other call to 
 * the library.
 *
 * Return value: the ORB used by OAF.
 */
CORBA_ORB
oaf_init (int argc, char **argv)
{
	CORBA_ORB retval;
	int i;

	g_return_val_if_fail (is_initialized == FALSE, oaf_orb);

        bindtextdomain (PACKAGE, OAF_LOCALEDIR);

	oaf_preinit (NULL, NULL);

	retval = oaf_orb_init (&argc, argv);

	/* Handle non-popt case */
	for (i = 1; i < argc; i++) {
		if (!strncmp
		    ("--oaf-od-ior=", argv[i], strlen ("--oaf-od-ior="))) {
			oaf_od_ior = argv[i] + strlen ("--oaf-od-ior=");
		} else if (!strncmp
                           ("--oaf-ior-fd=", argv[i],
                            strlen ("--oaf-ior-fd="))) {
                        oaf_ior_fd =
                                atoi (argv[i] + strlen ("--oaf-ior-fd="));
                        if (!oaf_ior_fd)
                                oaf_ior_fd = 1;
                } else if (!strncmp
                           ("--oaf-activate-iid=", argv[i],
                            strlen ("--oaf-activate-iid="))) {
			oaf_activate_iid =
				argv[i] + strlen ("--oaf-activate-iid=");
                } else if (!strcmp
                           ("--oaf-private", argv[i])) {
                        oaf_private = TRUE;
                }     
	}

	oaf_postinit (NULL, NULL);

	return retval;
}

CORBA_ORB
oaf_orb_init (int *argc, char **argv)
{
	CORBA_Environment ev;
	const char *hostname;
        char *display;

#ifndef ORBIT_USES_GLIB_MAIN_LOOP
	IIOPAddConnectionHandler = orb_add_connection;
	IIOPRemoveConnectionHandler = orb_remove_connection;
#endif /* !ORBIT_USES_GLIB_MAIN_LOOP */

	CORBA_exception_init (&ev);

	oaf_orb = CORBA_ORB_init (argc, argv, "orbit-local-orb", &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	/* Set values in default context */
	CORBA_ORB_get_default_context (oaf_orb, &oaf_context, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	hostname = oaf_hostname_get ();
	CORBA_Context_set_one_value (oaf_context, "hostname",
				     (char *) hostname, &ev);
	CORBA_Context_set_one_value (oaf_context, "domain", "user", &ev);
	CORBA_Context_set_one_value (oaf_context, "username",
				     g_get_user_name (), &ev);
        
        
        display =  g_getenv ("DISPLAY");
        
        if (display != NULL) {
                CORBA_Context_set_one_value (oaf_context, "display",
                                             display, &ev);
        }

	CORBA_exception_free (&ev);

#ifdef OAF_DEBUG
	if (getenv ("OAF_TRAP_SEGV")) {
		struct sigaction sa;
		sa.sa_handler = do_barrier;
		sigaction (SIGSEGV, &sa, NULL);
		sigaction (SIGPIPE, &sa, NULL);
	}
	if (getenv ("OAF_BARRIER_INIT")) {
		volatile int barrier = 1;
		while (barrier);
	}
#endif

	return oaf_orb;
}

#else

#error "You need to use a supported ORB for liboaf"

#endif

const char liboaf_version[] = VERSION;
const guint liboaf_major_version = OAF_MAJOR_VERSION,
	liboaf_minor_version = OAF_MINOR_VERSION,
	liboaf_micro_version = OAF_MICRO_VERSION;
