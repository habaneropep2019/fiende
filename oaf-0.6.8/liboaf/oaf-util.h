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

#ifndef OAF_UTIL_H
#define OAF_UTIL_H

#include <liboaf/oaf.h>

OAF_Property      *oaf_server_info_prop_find        (OAF_ServerInfo                      *server,
						     const char                          *prop_name);
const char        *oaf_server_info_prop_lookup      (OAF_ServerInfo                      *server,
						     const char                          *prop_name,
						     GSList                              *i18n_languages);
void               OAF_PropertyValue_copy           (OAF_PropertyValue                   *copy,
						     const OAF_PropertyValue             *original);
void               OAF_Property_copy                (OAF_Property                        *copy,
						     const OAF_Property                  *original);
void               CORBA_sequence_OAF_Property_copy (CORBA_sequence_OAF_Property         *copy,
						     const CORBA_sequence_OAF_Property   *original);
void               OAF_ServerInfo_copy              (OAF_ServerInfo                      *copy, 
						     const OAF_ServerInfo                *original);
OAF_ServerInfo    *OAF_ServerInfo_duplicate         (const OAF_ServerInfo                *original);

#endif /* OAF_UTIL_H */


