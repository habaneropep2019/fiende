/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  oafd: OAF CORBA dameon.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 1999, 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors: Elliot Lee <sopwith@redhat.com>,
 *
 */

#include <config.h>

#include "oaf-i18n.h"
#include "oafd.h"
#include "ac-query-expr.h"
#include "od-utils.h"
#include "liboaf/liboaf.h"

#include <ORBitservices/CosNaming.h>
#include <ORBitservices/CosNaming_impl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <popt.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>


#ifdef OAF_DEBUG
static void debug_queries (void);
#endif

/* Option values */
static char *od_source_dir = NULL;
#ifdef OAF_DEBUG
static char *ac_evaluate = NULL;
#endif
static char *od_domain = "session";
static int server_ac = 0, ior_fd = -1, server_ns = 1;

static struct poptOption options[] = {

	{"od-source-dir", '\0', POPT_ARG_STRING, &od_source_dir, 0,
	 N_("Directory to read .oaf files from"), N_("DIRECTORY")},
	{"od-domain", '\0', POPT_ARG_STRING, &od_domain, 0,
	 N_("Domain of ObjectDirectory"), N_("DOMAIN")},

	{"ac-activate", '\0', POPT_ARG_NONE, &server_ac, 0,
	 N_("Serve as an ActivationContext (default is as an ObjectDirectory only)"),
	 NULL},


	{"ior-output-fd", '\0', POPT_ARG_INT, &ior_fd, 0,
	 N_("File descriptor to write IOR to"), N_("FD")},

#ifdef OAF_DEBUG

	{"evaluate", '\0', POPT_ARG_STRING, &ac_evaluate, 0,
	 N_("Query expression to evaluate"), N_("EXPRESSION")},
#endif

	POPT_AUTOHELP {NULL}
};

int
main (int argc, char *argv[])
{
	GMainLoop *ml;
	CORBA_ORB orb;
	PortableServer_POA root_poa;
	CORBA_Environment ev;
	OAF_ObjectDirectory od;
	OAF_ActivationContext ac;
	CORBA_Object primary_server;
	poptContext ctx;
	char *ior;
	FILE *fh;
	struct sigaction sa;
        char *oaf_debug_output;
        int dev_null_fd;
        
	if (chdir ("/")) {
		g_print ("Couldn't chdir() to '/' (why ?!!). Exiting.\n");
		exit (EXIT_FAILURE);
	}

        /* Become process group leader, detach from controlling terminal,
         * etc.
         */
        setsid ();
        
        /* This is needed because otherwise, if oafd persists across X
         * sessions, spawned processes will inherit an invalid value of
         * SESSION_MANAGER and be very very slow while attempting to 
         * connect to it.
         */
#if HAVE_UNSETENV
        unsetenv ("SESSION_MANAGER");
#else
        putenv ("SESSION_MANAGER=");
#endif

	setlocale(LC_ALL, "");

	/* internationalization. */
        bindtextdomain (PACKAGE, OAF_LOCALEDIR);
        textdomain (PACKAGE);

	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = SIG_IGN;
	sigaction (SIGPIPE, &sa, NULL);

	CORBA_exception_init (&ev);

	ctx = poptGetContext ("oafd", argc, (const char **)argv, options, 0);
	while (poptGetNextOpt (ctx) >= 0) {
        }

	poptFreeContext (ctx);

        LIBXML_TEST_VERSION

        oaf_debug_output = g_getenv ("OAF_DEBUG_OUTPUT");

        if (oaf_debug_output == NULL || strlen (oaf_debug_output) == 0) {
                dev_null_fd = open ("/dev/null", O_RDWR);
                dup2 (dev_null_fd, 0);
                dup2 (dev_null_fd, 1);
                dup2 (dev_null_fd, 2);
                close (dev_null_fd);
        }

	ml = g_main_new (FALSE);

	orb = oaf_init (argc, argv);
	root_poa = (PortableServer_POA)
		CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);
	{
		char *env_od_source_dir;
		char *gnome_env_od_source_dir;
                char *config_file_od_source_dir;
		GString *gnome_od_source_dir;
                char **gnome_dirs;
		GString *real_od_source_dir;
                int i;

		real_od_source_dir = g_string_new (OAFINFODIR);
		env_od_source_dir = g_getenv ("OAF_INFO_PATH");
		gnome_env_od_source_dir = g_getenv ("GNOME_PATH");
                config_file_od_source_dir = od_utils_load_config_file ();

		if (od_source_dir) {
			g_string_append_c (real_od_source_dir, ':');
			g_string_append (real_od_source_dir, od_source_dir);
		}
		if (env_od_source_dir) {
			g_string_append_c (real_od_source_dir, ':');
			g_string_append (real_od_source_dir,
					 env_od_source_dir);
		}
                if (config_file_od_source_dir) {
			g_string_append_c (real_od_source_dir, ':');
			g_string_append (real_od_source_dir,
					 config_file_od_source_dir);
			g_free (config_file_od_source_dir);
                }
		if (gnome_env_od_source_dir) {
                        gnome_dirs = g_strsplit (gnome_env_od_source_dir, ":", -1);
                        gnome_od_source_dir = g_string_new("");
                        for (i=0; gnome_dirs[i]; i++) {
                                g_string_append (gnome_od_source_dir,
                                                 gnome_dirs[i]);
                                g_string_append (gnome_od_source_dir,
                                                 "/share/oaf:");
                        }
                        g_strfreev (gnome_dirs);
			g_string_append_c (real_od_source_dir, ':');
			g_string_append (real_od_source_dir,
					 gnome_od_source_dir->str);
		}

		od = OAF_ObjectDirectory_create (root_poa, od_domain,
                                                 real_od_source_dir->str,
                                                 &ev);
		if (server_ns) {
			CORBA_Object naming_service;

			naming_service =
				impl_CosNaming_NamingContext__create
				(root_poa, &ev);
			OAF_ObjectDirectory_register_new 
                                (od,
                                 "OAFIID:oaf_naming_service:7e2b90ef-eaf0-4239-bb7c-812606fcd80d",
                                 naming_service,
                                 &ev);
		}

		g_string_free (real_od_source_dir, TRUE);
	}
	if (server_ac) {
		primary_server = ac =
			OAF_ActivationContext_create (root_poa, &ev);
		OAF_ActivationContext_add_directory (ac, od, &ev);
	} else
		primary_server = od;

	ior = CORBA_ORB_object_to_string (orb, primary_server, &ev);

	fh = NULL;
	if (ior_fd >= 0)
		fh = fdopen (ior_fd, "w");
	if (fh) {
		fprintf (fh, "%s\n", ior);
		fclose (fh);
	} else {
		fprintf (stdout, "%s\n", ior);
		fflush (stdout);
	}
	CORBA_free (ior);

#ifdef OAF_DEBUG
	debug_queries ();
#endif

	PortableServer_POAManager_activate
		(PortableServer_POA__get_the_POAManager (root_poa, &ev), &ev);
	g_main_run (ml);

	return 0;
}

#ifdef OAF_DEBUG
static void
debug_queries (void)
{
	if (ac_evaluate) {
		QueryExpr *exp;
		const char *err;
		QueryContext tmpctx = { NULL, 0, CORBA_OBJECT_NIL };

		err = qexp_parse (ac_evaluate, &exp);
		if (err) {
			g_print ("Parse error: %s\n", err);
		} else {
			QueryExprConst res;

			qexp_dump (exp);
			g_print ("\n");
			g_print ("Evaluation with no server record is: ");
			res = qexp_evaluate (NULL, exp, &tmpctx);
			qexp_constant_dump (&res);
			g_print ("\n");
		}
	}
}
#endif
