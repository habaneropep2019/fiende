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

#include "config.h"

#include "liboaf/liboaf-private.h"
#include "oafd.h"
#include <stdio.h>
#include <popt.h>

static char *acior = NULL, *specs = NULL;
static int do_query;

static struct poptOption options[] = {

	{"ac-ior", '\0', POPT_ARG_STRING, &acior, 0,
	 "IOR of ActivationContext to use", "IOR"},
	{"do-query", 'q', POPT_ARG_NONE, &do_query, 0,
	 "Run a query instead of activating", "QUERY"},
	{"spec", 's', POPT_ARG_STRING, &specs, 0,
	 "Specification string for object to activate", "SPEC"},
	POPT_AUTOHELP {NULL}
};

static void
od_dump_list (OAF_ServerInfoList * list)
{
	int i, j, k;

	for (i = 0; i < list->_length; i++) {
		g_print ("IID %s, type %s, location %s\n",
			 list->_buffer[i].iid,
			 list->_buffer[i].server_type,
			 list->_buffer[i].location_info);
		for (j = 0; j < list->_buffer[i].props._length; j++) {
			OAF_Property *prop =
				&(list->_buffer[i].props._buffer[j]);
			g_print ("    %s = ", prop->name);
			switch (prop->v._d) {
			case OAF_P_STRING:
				g_print ("\"%s\"\n", prop->v._u.value_string);
				break;
			case OAF_P_NUMBER:
				g_print ("%f\n", prop->v._u.value_number);
				break;
			case OAF_P_BOOLEAN:
				g_print ("%s\n",
					 prop->v.
					 _u.value_boolean ? "TRUE" : "FALSE");
				break;
			case OAF_P_STRINGV:
				g_print ("[");
				for (k = 0;
				     k < prop->v._u.value_stringv._length;
				     k++) {
					g_print ("\"%s\"",
						 prop->v._u.
						 value_stringv._buffer[k]);
					if (k <
					    (prop->v._u.
					     value_stringv._length - 1))
						g_print (", ");
				}
				g_print ("]\n");
				break;
			}
		}
	}
}

int
main (int argc, char *argv[])
{
	CORBA_Environment ev;
	OAF_ActivationContext ac;
	poptContext ctx;
	gboolean do_usage_exit = FALSE;
	OAF_ServerInfoList *slist;
	CORBA_ORB orb;
	GNOME_stringlist reqs = { 0 };

	CORBA_exception_init (&ev);

	ctx = poptGetContext ("oaf-client", argc, (const char **)argv, options, 0);
	while (poptGetNextOpt (ctx) >= 0)
		/**/;

	orb = oaf_init (argc, argv);

	if (!specs) {
		g_print ("You must specify an operation to perform.\n");
		do_usage_exit = TRUE;
	}

	if (do_usage_exit) {
		poptPrintUsage (ctx, stdout, 0);
		return 1;
	}

	if (acior) {
		ac = CORBA_ORB_string_to_object (orb, acior, &ev);
		if (ev._major != CORBA_NO_EXCEPTION) {
			g_print ("Error doing string_to_object(%s)\n", acior);
			do_usage_exit = TRUE;
		}
	} else
		ac = oaf_activation_context_get ();

	poptFreeContext (ctx);

	g_print ("Query spec is \"%s\"\n", specs);

	if (do_query) {

		slist =
			OAF_ActivationContext_query (ac, specs, &reqs,
						     oaf_context_get (), &ev);
		switch (ev._major) {
		case CORBA_NO_EXCEPTION:
			od_dump_list (slist);
			CORBA_free (slist);
			break;
		case CORBA_USER_EXCEPTION:
			{
				char *id;
				id = CORBA_exception_id (&ev);
				g_print
					("User exception \"%s\" resulted from query\n",
					 id);
				if (!strcmp
				    (id,
				     "IDL:OAF/ActivationContext/ParseFailed:1.0"))
				{
					OAF_ActivationContext_ParseFailed
						* exdata =
						CORBA_exception_value (&ev);

					if (exdata)
						g_print ("Description: %s\n",
							 exdata->description);
				}
			}
			break;
		case CORBA_SYSTEM_EXCEPTION:
			{
				char *id;
				id = CORBA_exception_id (&ev);
				g_print
					("System exception \"%s\" resulted from query\n",
					 id);
			}
			break;
		}
	} else {
		OAF_ActivationResult *res;

		res =
			OAF_ActivationContext_activate (ac, specs, &reqs, 0,
							oaf_context_get (),
							&ev);
		switch (ev._major) {
		case CORBA_NO_EXCEPTION:
			g_print ("Activation ID \"%s\" ", res->aid);
			switch (res->res._d) {
			case OAF_RESULT_OBJECT:
				g_print ("RESULT_OBJECT\n");
				acior =
					CORBA_ORB_object_to_string (orb,
								    res->
								    res._u.res_object,
								    &ev);
				g_print ("%s\n", acior);
				break;
			case OAF_RESULT_SHLIB:
				g_print ("RESULT_SHLIB\n");
				break;
			case OAF_RESULT_NONE:
				g_print ("RESULT_NONE\n");
				break;
			}
			break;
		case CORBA_USER_EXCEPTION:
			{
				char *id;
				id = CORBA_exception_id (&ev);
				g_print
					("User exception \"%s\" resulted from query\n",
					 id);
				if (strcmp (id,
                                            "IDL:OAF/ActivationContext/ParseFailed:1.0") == 0) {
					OAF_ActivationContext_ParseFailed
						* exdata =
						CORBA_exception_value (&ev);

					if (exdata)
						g_print ("Description: %s\n",
							 exdata->description);
				}
				if (strcmp (id,
                                            "IDL:OAF/GeneralError:1.0") == 0) {
					OAF_GeneralError
						* exdata =
						CORBA_exception_value (&ev);

					if (exdata)
						g_print ("Description: %s\n",
							 exdata->description);
				}
			}
			break;
		case CORBA_SYSTEM_EXCEPTION:
			{
				char *id;
				id = CORBA_exception_id (&ev);
				g_print
					("System exception \"%s\" resulted from query\n",
					 id);
			}
			break;
		}
	}

	CORBA_exception_free (&ev);

	return 0;
}
