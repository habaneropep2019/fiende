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

#include "oafd.h"
#include "liboaf/liboaf-private.h"
#include "oaf-i18n.h"

#include <signal.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#include "oafd-corba-extensions.h"

CORBA_Object od_server_activate_factory (OAF_ServerInfo * si,
					 ODActivationInfo * actinfo,
					 CORBA_Environment * ev);
CORBA_Object od_server_activate_exe (OAF_ServerInfo * si,
				     ODActivationInfo * actinfo,
				     CORBA_Object od_obj,
				     CORBA_Environment * ev);

CORBA_Object
od_server_activate (OAF_ServerInfo * si, ODActivationInfo * actinfo,
		    CORBA_Object od_obj, CORBA_Environment * ev)
{
	if (!strcmp (si->server_type, "exe"))
		return od_server_activate_exe (si, actinfo, od_obj, ev);

	else if (!strcmp (si->server_type, "factory"))
		return od_server_activate_factory (si, actinfo, ev);

	else if (!strcmp (si->server_type, "shlib"))
		g_warning
			(_("We don't handle activating shlib objects in a remote process yet"));

	return CORBA_OBJECT_NIL;
}

CORBA_Object
od_server_activate_factory (OAF_ServerInfo * si, ODActivationInfo * actinfo,
			    CORBA_Environment * ev)
{
	CORBA_Object retval = CORBA_OBJECT_NIL, factory = CORBA_OBJECT_NIL;
	OAF_ActivationResult *res;
	GNOME_stringlist params = { 0 };

	res =
		OAF_ActivationContext_activate_from_id (actinfo->ac,
							si->location_info,
							((actinfo->flags |
							  OAF_FLAG_NO_LOCAL) &
							 (~OAF_FLAG_PRIVATE)),
							actinfo->ctx,
							ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		goto out;

	switch (res->res._d) {
	case OAF_RESULT_NONE:
		CORBA_free (res);
		goto out;
		break;
	case OAF_RESULT_OBJECT:
		factory = res->res._u.res_object;
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	retval =
		GNOME_ObjectFactory_create_object (factory, si->iid, &params,
						    ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		retval = CORBA_OBJECT_NIL;

	CORBA_free (res);

      out:
	return retval;
}


/* Copied largely from goad.c, goad_server_activate_exe() */
CORBA_Object
od_server_activate_exe (OAF_ServerInfo * si, ODActivationInfo * actinfo,
			CORBA_Object od_obj, CORBA_Environment * ev)
{
	char **args;
	char *extra_arg, *ctmp, *ctmp2;
        int fd_arg;
	int i;
        char *iorstr;
        char *display;
        CORBA_Object retval;

	/* Munge the args */
	args = oaf_alloca (36 * sizeof (char *));
	for (i = 0, ctmp = ctmp2 = si->location_info; i < 32; i++) {
		while (*ctmp2 && !isspace ((guchar) *ctmp2))
			ctmp2++;
		if (!*ctmp2)
			break;

		args[i] = oaf_alloca (ctmp2 - ctmp + 1);
		strncpy (args[i], ctmp, ctmp2 - ctmp);
		args[i][ctmp2 - ctmp] = '\0';

		ctmp = ctmp2;
		while (*ctmp2 && isspace ((guchar) *ctmp2))
			ctmp2++;
		if (!*ctmp2)
			break;
		ctmp = ctmp2;
	}
	if (!isspace ((guchar) *ctmp) && i < 32)
		args[i++] = ctmp;

	extra_arg =
		oaf_alloca (strlen (si->iid) +
			    sizeof ("--oaf-activate-iid="));
	args[i++] = extra_arg;
	sprintf (extra_arg, "--oaf-activate-iid=%s", si->iid);

        fd_arg = i;
	extra_arg = oaf_alloca (sizeof ("--oaf-ior-fd=") + 10);
	args[i++] = "--oaf-ior-fd=%d";


        iorstr = CORBA_ORB_object_to_string (oaf_orb_get (), od_obj,
                                             ev);

        if (ev->_major != CORBA_NO_EXCEPTION)
	  iorstr = NULL;

        if(actinfo->flags & OAF_FLAG_PRIVATE) {
                extra_arg =
                        oaf_alloca (sizeof ("--oaf-private"));
                args[i++] = extra_arg;
                g_snprintf (extra_arg, sizeof ("--oaf-private"),
                            "--oaf-private");
        }

	args[i] = NULL;

        display = oafd_CORBA_Context_get_value (actinfo->ctx, "display", NULL, ev);
        
        /* We set the process group of activated servers to our process group;
         * this allows people to destroy all OAF servers along with oafd
         * if necessary
         */
	retval = oaf_internal_server_by_forking_extended ((const char **) args,
                                                          TRUE,
                                                          fd_arg, display,
                                                          iorstr, ev);
        
        g_free (display);
	CORBA_free (iorstr);

        return retval;
}
