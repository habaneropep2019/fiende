/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-main.h: Bonobo Main
 *
 * Author:
 *    Miguel de Icaza  (miguel@gnu.org)
 *    Nat Friedman     (nat@nat.org)
 *    Peter Wainwright (prw@wainpr.demo.co.uk)
 *
 * Copyright 1999 Helix Code, Inc.
 */

#ifndef __GNOME_MAIN_H__
#define __GNOME_MAIN_H__ 1

#include <gtk/gtkobject.h>
#include <bonobo/Bonobo.h>

gboolean		    bonobo_init			 (CORBA_ORB orb,
							  PortableServer_POA poa,
							  PortableServer_POAManager manager);
void			    bonobo_main			 (void);
gboolean		    bonobo_activate		 (void);
void			    bonobo_setup_x_error_handler (void);

CORBA_ORB		    bonobo_orb			 (void);
PortableServer_POA	    bonobo_poa			 (void);
PortableServer_POAManager   bonobo_poa_manager		 (void);

#endif /* __GNOME_MAIN_H__ */
