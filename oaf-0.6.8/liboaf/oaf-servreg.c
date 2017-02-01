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

#include "liboaf/liboaf-private.h"
#include <stdio.h>
#include <unistd.h>

static gboolean check_registration = TRUE;
static gboolean need_ior_printout  = TRUE;

void
oaf_timeout_reg_check_set (gboolean on)
{
        check_registration = on;
}

gboolean
oaf_timeout_reg_check (gpointer data)
{
        if (!check_registration)
                return FALSE;

        if (need_ior_printout) {
                g_error ("This process has not registered the required OafIID "
                         "your source code should register '%s'. If your code is "
                         "performing delayed registration and this message is trapped "
                         "in error, see oaf_idle_reg_check_set.",
                         oaf_activation_iid_get ());
        }

        return FALSE;
}


/**
 * oaf_active_server_register:
 * @iid: IID of the server to register.
 * @obj: CORBA::Object to register.
 *
 * Registers @obj with @iid in the local OAF daemon.
 *
 * Return value: status of the registration.
 */
OAF_RegistrationResult
oaf_active_server_register (const char *registration_id, 
                            CORBA_Object obj)
{
	OAF_ObjectDirectory od;
	CORBA_Environment ev;
	OAF_RegistrationResult retval;
	const char *actid;
        const char *iid;

        iid = strrchr (registration_id, ',');

        if (iid == NULL) {
                iid = registration_id;
        } else {
                iid++;
        }

	CORBA_exception_init (&ev);

	actid = oaf_activation_iid_get ();

        if (actid && strcmp (actid, iid) == 0 && oaf_private) {
                retval = OAF_REG_SUCCESS;
        } else {
                od = oaf_object_directory_get (oaf_username_get (),
                                               oaf_hostname_get (),
                                               NULL);
                
                if (CORBA_Object_is_nil (od, &ev)) {
                        return OAF_REG_ERROR;
                }
                
                retval = OAF_ObjectDirectory_register_new (od, 
                                                           (char *) registration_id, 
                                                           obj, &ev);
        }

	if (actid && strcmp (actid, iid) == 0 && need_ior_printout) {
		char *iorstr;
		FILE *fh;
		int iorfd = oaf_ior_fd_get ();

		need_ior_printout = FALSE;

		if (iorfd == 1)
			fh = stdout;
		else {
			fh = fdopen (iorfd, "w");
			if (!fh)
				fh = stdout;
		}

		iorstr =
			CORBA_ORB_object_to_string (oaf_orb_get (), obj, &ev);

		if (ev._major == CORBA_NO_EXCEPTION) {
			fprintf (fh, "%s\n", iorstr);
			CORBA_free (iorstr);
		}

		if (fh != stdout) {
			fclose (fh);
		} else if (iorfd > 2) {
			close (iorfd);
                }
	}
#ifdef OAF_DEBUG
        else if (actid && need_ior_printout) {
                g_message ("Unusual '%s' was activated, but "
                           "'%s' is needed", iid, actid);
        }
#endif

	CORBA_exception_free (&ev);

#ifdef OAF_DEBUG
        g_message ("Successfully registered `%s'", registration_id);
#endif

	return retval;
}


/**
 * oaf_active_server_unregister:
 * @iid: IID of the server to unregister.
 * @obj: CORBA::Object to unregister.
 *
 * Unregisters @obj with @iid in the local OAF daemon.
 */
void
oaf_active_server_unregister (const char *iid, CORBA_Object obj)
{
	OAF_ObjectDirectory od;
	CORBA_Environment ev;
	const char *actid;

	actid = oaf_activation_iid_get ();
	if(actid && strcmp (actid, iid) == 0 && oaf_private) {
		return;
        }

	od = oaf_object_directory_get (oaf_username_get (), 
                                       oaf_hostname_get (),
                                       NULL);

	CORBA_exception_init (&ev);
	if (CORBA_Object_is_nil (od, &ev)) {
		return;
        }

	OAF_ObjectDirectory_unregister (od, (char *) iid, obj,
					OAF_ObjectDirectory_UNREGISTER_NORMAL, &ev);
	CORBA_exception_free (&ev);
}


char *
oaf_make_registration_id (const char *iid, const char *display)
{
        if (display == NULL) {
                return g_strdup (iid);
        } else {
                return g_strconcat (display, ",", iid, NULL);
        }
}
