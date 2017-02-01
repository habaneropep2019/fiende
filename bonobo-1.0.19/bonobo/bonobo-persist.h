/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-persist.h: a persistance interface
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#ifndef _BONOBO_PERSIST_H_
#define _BONOBO_PERSIST_H_

#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS

#define BONOBO_PERSIST_TYPE        (bonobo_persist_get_type ())
#define BONOBO_PERSIST(o)          (GTK_CHECK_CAST ((o), BONOBO_PERSIST_TYPE, BonoboPersist))
#define BONOBO_PERSIST_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PERSIST_TYPE, BonoboPersistClass))
#define BONOBO_IS_PERSIST(o)       (GTK_CHECK_TYPE ((o), BONOBO_PERSIST_TYPE))
#define BONOBO_IS_PERSIST_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PERSIST_TYPE))

typedef struct _BonoboPersistPrivate BonoboPersistPrivate;

typedef struct {
	BonoboXObject object;

	BonoboPersistPrivate *priv;
} BonoboPersist;

typedef struct {
	BonoboXObjectClass      parent_class;

	POA_Bonobo_Persist__epv epv;

	Bonobo_Persist_ContentTypeList *
	                      (*get_content_types) (BonoboPersist     *persist,
						    CORBA_Environment *ev);
} BonoboPersistClass;

GtkType                         bonobo_persist_get_type               (void);

Bonobo_Persist_ContentTypeList *bonobo_persist_generate_content_types (int num,
								       ...);

END_GNOME_DECLS

#endif /* _BONOBO_PERSIST_H_ */
