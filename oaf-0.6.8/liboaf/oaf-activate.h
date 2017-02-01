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

#ifndef OAF_ACTIVATE_H
#define OAF_ACTIVATE_H

#include <orb/orbit.h>
#include <liboaf/oaf.h>

CORBA_Object oaf_name_service_get (CORBA_Environment * ev);


OAF_ServerInfoList *oaf_query (const char *requirements,
			       char *const *selection_order,
			       CORBA_Environment * ev);
CORBA_Object oaf_activate (const char *requirements,
			   char *const *selection_order,
			   OAF_ActivationFlags flags,
			   OAF_ActivationID * ret_aid,
			   CORBA_Environment * ev);
CORBA_Object oaf_activate_from_id (const OAF_ActivationID aid,
				   OAF_ActivationFlags flags,
				   OAF_ActivationID * ret_aid,
				   CORBA_Environment * ev);




/* debugging functions. */
void         oaf_set_test_components_enabled (gboolean val);
gboolean     oaf_get_test_components_enabled (void);



#endif /* OAF_ACTIVATE_H */














