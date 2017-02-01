/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-moniker: Object naming abstraction
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, Helix Code, Inc.
 */
#ifndef _BONOBO_MONIKER_H_
#define _BONOBO_MONIKER_H_

#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS

typedef struct _BonoboMonikerPrivate BonoboMonikerPrivate;

#define BONOBO_MONIKER_TYPE        (bonobo_moniker_get_type ())
#define BONOBO_MONIKER(o)          (GTK_CHECK_CAST ((o), BONOBO_MONIKER_TYPE, BonoboMoniker))
#define BONOBO_MONIKER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_MONIKER_TYPE, BonoboMonikerClass))
#define BONOBO_IS_MONIKER(o)       (GTK_CHECK_TYPE ((o), BONOBO_MONIKER_TYPE))
#define BONOBO_IS_MONIKER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_MONIKER_TYPE))

typedef struct {
        BonoboXObject         object;
	
	BonoboMonikerPrivate *priv;
} BonoboMoniker;

typedef struct {
	BonoboXObjectClass      parent_class;

	POA_Bonobo_Moniker__epv epv;

	/*
	 * virtual methods
	 */
	Bonobo_Moniker (*get_parent)         (BonoboMoniker               *moniker,
					      CORBA_Environment           *ev);
	void           (*set_parent)         (BonoboMoniker               *moniker,
					      const Bonobo_Moniker         parent,
					      CORBA_Environment           *ev);
	CORBA_char    *(*get_display_name)   (BonoboMoniker               *moniker,
					      CORBA_Environment           *ev);
	Bonobo_Moniker (*parse_display_name) (BonoboMoniker               *moniker,
					      Bonobo_Moniker               parent,
					      const CORBA_char            *name,
					      CORBA_Environment           *ev);
	Bonobo_Unknown (*resolve)            (BonoboMoniker               *moniker,
					      const Bonobo_ResolveOptions *options,
					      const CORBA_char            *requested_interface,
					      CORBA_Environment           *ev);
	CORBA_long     (*equal)              (BonoboMoniker               *moniker,
					      const CORBA_char            *display_name,
					      CORBA_Environment           *ev);

	void           (*set_name)           (BonoboMoniker               *moniker,
					      const char                  *unescaped_name);
	const char    *(*get_name)           (BonoboMoniker               *moniker);

	gpointer        dummy;
} BonoboMonikerClass;

GtkType                  bonobo_moniker_get_type            (void);

BonoboMoniker           *bonobo_moniker_construct           (BonoboMoniker     *moniker,
							     const char        *prefix);

Bonobo_Moniker           bonobo_moniker_get_parent          (BonoboMoniker     *moniker,
							     CORBA_Environment *ev);
void                     bonobo_moniker_set_parent          (BonoboMoniker     *moniker,
							     Bonobo_Moniker     parent,
							     CORBA_Environment *ev);

const char              *bonobo_moniker_get_name            (BonoboMoniker     *moniker);

const char              *bonobo_moniker_get_name_full       (BonoboMoniker     *moniker);
char                    *bonobo_moniker_get_name_escaped    (BonoboMoniker     *moniker);

void                     bonobo_moniker_set_name            (BonoboMoniker     *moniker,
							     const char        *unescaped_name,
							     int                num_chars);

const char              *bonobo_moniker_get_prefix          (BonoboMoniker     *moniker);

void                     bonobo_moniker_set_case_sensitive  (BonoboMoniker     *moniker,
							     gboolean           sensitive);
gboolean                 bonobo_moniker_get_case_sensitive  (BonoboMoniker     *moniker);

END_GNOME_DECLS

#endif /* _BONOBO_MONIKER_H_ */
