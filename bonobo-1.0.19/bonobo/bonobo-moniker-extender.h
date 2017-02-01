/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-moniker-extender: extending monikers
 *
 * Author:
 *	Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright 2000, Dietmar Maurer.
 */
#ifndef _BONOBO_MONIKER_EXTENDER_H_
#define _BONOBO_MONIKER_EXTENDER_H_

#include <bonobo/bonobo-moniker.h>

BEGIN_GNOME_DECLS

#define BONOBO_MONIKER_EXTENDER_TYPE        (bonobo_moniker_extender_get_type ())
#define BONOBO_MONIKER_EXTENDER(o)          (GTK_CHECK_CAST ((o), BONOBO_MONIKER_EXTENDER_TYPE, BonoboMonikerExtender))
#define BONOBO_MONIKER_EXTENDER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_MONIKER_EXTENDER_TYPE, BonoboMonikerExtenderClass))
#define BONOBO_IS_MONIKER_EXTENDER(o)       (GTK_CHECK_TYPE ((o), BONOBO_MONIKER_EXTENDER_TYPE))
#define BONOBO_IS_MONIKER_EXTENDER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_MONIKER_EXTENDER_TYPE))

typedef struct _BonoboMonikerExtender BonoboMonikerExtender;

typedef Bonobo_Unknown (*BonoboMonikerExtenderFn) (BonoboMonikerExtender       *extender,
						   const Bonobo_Moniker         parent,
						   const Bonobo_ResolveOptions *options,
						   const CORBA_char            *display_name,
						   const CORBA_char            *requested_interface,
						   CORBA_Environment           *ev);
struct _BonoboMonikerExtender {
        BonoboXObject           object;
	BonoboMonikerExtenderFn resolve;
	gpointer                data;
};

typedef struct {
	BonoboXObjectClass      parent_class;

	POA_Bonobo_MonikerExtender__epv epv;
	
	BonoboMonikerExtenderFn resolve;
} BonoboMonikerExtenderClass;

GtkType                bonobo_moniker_extender_get_type (void);
BonoboMonikerExtender *bonobo_moniker_extender_new      (BonoboMonikerExtenderFn      resolve,
							 gpointer                     data);
 
Bonobo_MonikerExtender bonobo_moniker_find_extender     (const gchar                 *name,
							 const gchar                 *interface,
							 CORBA_Environment           *ev);

Bonobo_Unknown         bonobo_moniker_use_extender      (const gchar                 *extender_oafiid,
							 BonoboMoniker               *moniker,
							 const Bonobo_ResolveOptions *options,
							 const CORBA_char            *requested_interface,
							 CORBA_Environment           *ev);

END_GNOME_DECLS

#endif /* _BONOBO_MONIKER_EXTENDER_H_ */
