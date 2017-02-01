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


#ifndef LIBOAF_PRIVATE_H
#define LIBOAF_PRIVATE_H 1

#include "config.h"

#include "liboaf.h"

#ifdef g_alloca
#define oaf_alloca g_alloca
#else
#define oaf_alloca alloca
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#endif

#include <string.h>

#define OAF_FACTORY_TIMEOUT 1000

void         oaf_timeout_reg_check_set  (gboolean on);
gboolean     oaf_timeout_reg_check      (gpointer data);
CORBA_Object oaf_server_by_forking      (const char **cmd, 
                                         int fd_Arg,
                                         const char *display,
					 const char *od_iorstr,
                                         CORBA_Environment * ev);
void         oaf_rloc_file_register     (void);
int          oaf_ior_fd_get             (void);
CORBA_Object oaf_activation_context_get (void);
CORBA_Object oaf_object_directory_get   (const char *username,
                                         const char *hostname,
                                         const char *domain);

extern gboolean oaf_private;

#define OAF_STR_NULL_OR_MATCH(x, y) ((x == NULL) || (x != NULL && y != NULL && strcmp (x, y) != 0))

CORBA_Object oaf_internal_service_get_extended  (const OAFBaseService         *base_service,
                                                 gboolean                     existing_only,
                                                 CORBA_Environment          *ev);
CORBA_Object oaf_internal_activation_context_get_extended (gboolean           existing_only,
                                                           CORBA_Environment *ev);


CORBA_Object oaf_internal_server_by_forking_extended (const char **cmd,
                                                      gboolean set_process_group,
                                                      int fd_arg, 
                                                      const char *display,
                                                      const char *od_iorstr,
                                                      CORBA_Environment * ev);

#endif

