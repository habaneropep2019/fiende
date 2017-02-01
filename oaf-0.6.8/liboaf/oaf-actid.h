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

#ifndef OAF_ACTID_H
#define OAF_ACTID_H

/* If you wish to manipulate the internals of this structure, please
   use g_malloc/g_free to allocate memory. */
typedef struct
{
	char *iid;		/* Implementation ID */
	char *user;		/* user name */
	char *host;		/* DNS name or IP address */
	char *domain;		/* This is not a DNS domain, but an activation domain */
}
OAFActivationInfo;


OAF_ActivationID    oaf_actinfo_stringify    (const OAFActivationInfo   *actinfo);

OAFActivationInfo  *oaf_servinfo_to_actinfo  (const OAF_ServerInfo      *servinfo);

OAFActivationInfo  *oaf_actid_parse          (const OAF_ActivationID     actid);

OAFActivationInfo  *oaf_actinfo_new          (void);

void                oaf_actinfo_free         (OAFActivationInfo          *actinfo);


#endif /* OAF_ACTID_H */
