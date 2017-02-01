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

#include "liboaf-private.h"


/**
 * oaf_actinfo_new:
 *
 * This function allocates a %OAFActicationInfo structure and returns it.
 * Should NOT be called from outside of this code.
 *
 * Return value: a newly allocated non-initialized %OAFActicationInfo structure.
 */
OAFActivationInfo *
oaf_actinfo_new (void)
{
        return g_new0 (OAFActivationInfo, 1);
}

/**
 * oaf_servinfo_to_actinfo:
 * @servinfo: An array of %OAF_ServerInfo structures.
 *
 * This function converts a %OAF_ServerInfo structure to a
 * %OAFActivationInfo structure. The returned structure should
 * be freed with oaf_actinfo_free.
 *
 * Return value: a newly allocated initialized %OAFActivationInfo structure.
 */

OAFActivationInfo *
oaf_servinfo_to_actinfo (const OAF_ServerInfo * servinfo)
{
	OAFActivationInfo *retval = oaf_actinfo_new ();

	retval->iid = g_strdup (servinfo->iid);
	retval->user = g_strdup (servinfo->username);
	retval->host = g_strdup (servinfo->hostname);
	retval->domain = g_strdup (servinfo->domain);

	return retval;
}

/**
 * oaf_actinfo_free:
 * @actinfo: the %OAFActivationInfo structure to free.
 *
 * Frees @actinfo.
 *
 */

void
oaf_actinfo_free (OAFActivationInfo * actinfo)
{
	g_free (actinfo->iid);
	g_free (actinfo->user);
	g_free (actinfo->host);
	g_free (actinfo->domain);
	g_free (actinfo);
}


/**
 * oaf_actid_parse:
 * @actid: the activation id structure.
 *
 * Returns a pointer to a newly allocated %OAFActivationInfo
 * structure (to be freed with oaf_actinfo_free) initialized 
 * with the data of @actid.
 *
 * Return value: the %OAFActivationInfo corresponding to @actid.
 */

OAFActivationInfo *
oaf_actid_parse (const OAF_ActivationID actid)
{
	OAFActivationInfo *retval;
	char *splitme, *ctmp, *ctmp2;
	char **parts[4];
	const int nparts = sizeof (parts) / sizeof (parts[0]);
	int bracket_count, partnum;

	g_return_val_if_fail (actid, NULL);
	if (strncmp (actid, "OAFAID:", sizeof ("OAFAID:") - 1))
		return NULL;

	ctmp = (char *) (actid + sizeof ("OAFAID:") - 1);
	if (*ctmp != '[')
		return NULL;

	retval = oaf_actinfo_new ();

	splitme = oaf_alloca (strlen (ctmp) + 1);
	strcpy (splitme, ctmp);

	parts[0] = &(retval->iid);
	parts[1] = &(retval->user);
	parts[2] = &(retval->host);
	parts[3] = &(retval->domain);
	for (partnum = bracket_count = 0, ctmp = ctmp2 = splitme;
	     bracket_count >= 0 && *ctmp && partnum < nparts; ctmp++) {

		switch (*ctmp) {
		case '[':
			if (bracket_count <= 0)
				ctmp2 = ctmp + 1;
			bracket_count++;
			break;
		case ']':
			bracket_count--;
			if (bracket_count <= 0) {
				*ctmp = '\0';
				if (*ctmp2)
					*parts[partnum++] = g_strdup (ctmp2);
			}
			break;
		case ',':
			if (bracket_count == 1) {
				*ctmp = '\0';
				if (*ctmp2)
					*parts[partnum++] = g_strdup (ctmp2);
				ctmp2 = ctmp + 1;
			}
			break;
		default:
			break;
		}
	}

	return retval;
}


/**
 * oaf_actinfo_stringify:
 * @actinfo: the %OAFActivationInfo to flatten.
 *
 * Serializes @actinfo into a char *. Should be freed with g_free ().
 *
 * Return value: the serialized version of @actinfo.
 */
char *
oaf_actinfo_stringify (const OAFActivationInfo * actinfo)
{
	g_return_val_if_fail (actinfo, NULL);

	return g_strconcat ("OAFAID:[",
			    actinfo->iid ? actinfo->iid : "",
			    ",",
			    actinfo->user ? actinfo->user : "",
			    ",",
			    actinfo->host ? actinfo->host : "",
			    ",",
			    actinfo->domain ? actinfo->domain : "",
			    "]", NULL);
}



