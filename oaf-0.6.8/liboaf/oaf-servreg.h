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

#ifndef OAF_SERVREG_H
#define OAF_SERVREG_H

#include <orb/orbit.h>
#include <liboaf/oaf.h>

OAF_RegistrationResult oaf_active_server_register (const char  *iid,
						   CORBA_Object obj);
void        oaf_active_server_unregister (const char  *iid, 
					  CORBA_Object obj);

char       *oaf_make_registration_id     (const char *iid, 
					  const char *display);


const char *oaf_activation_iid_get       (void);


#endif /* OAF_SERVREG_H */

