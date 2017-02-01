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

#ifndef OAF_MAINLOOP_H
#define OAF_MAINLOOP_H

#include <popt.h>

CORBA_ORB      oaf_orb_init   (int   *argc, 
			       char **argv);
CORBA_ORB      oaf_orb_get    (void);

gboolean       oaf_is_initialized   (void);
CORBA_ORB      oaf_init       (int      argc, 
			       char   **argv);
void           oaf_preinit    (gpointer app, 
			       gpointer mod_info);
void           oaf_postinit   (gpointer app, 
			       gpointer mod_info);

CORBA_Context  oaf_context_get      (void);

const char    *oaf_hostname_get     (void);
const char    *oaf_session_name_get (void);
const char    *oaf_domain_get       (void);
#define oaf_username_get() g_get_user_name()

char          *oaf_get_popt_table_name (void);

extern struct poptOption oaf_popt_options[];

#endif /* OAF_MAINLOOP_H */

