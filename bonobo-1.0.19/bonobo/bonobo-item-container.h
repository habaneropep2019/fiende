/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-item-container.h: a generic container for monikers.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */

#ifndef _BONOBO_ITEM_CONTAINER_H_
#define _BONOBO_ITEM_CONTAINER_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo-xobject.h>
#include <bonobo/bonobo-moniker.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_ITEM_CONTAINER_TYPE        (bonobo_item_container_get_type ())
#define BONOBO_ITEM_CONTAINER(o)          (GTK_CHECK_CAST ((o), BONOBO_ITEM_CONTAINER_TYPE, BonoboItemContainer))
#define BONOBO_ITEM_CONTAINER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_ITEM_CONTAINER_TYPE, BonoboItemContainerClass))
#define BONOBO_IS_ITEM_CONTAINER(o)       (GTK_CHECK_TYPE ((o), BONOBO_ITEM_CONTAINER_TYPE))
#define BONOBO_IS_ITEM_CONTAINER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_ITEM_CONTAINER_TYPE))

typedef GList BonoboClientSiteList;

typedef struct _BonoboItemContainerPrivate BonoboItemContainerPrivate;

typedef struct {
	BonoboXObject base;

	BonoboItemContainerPrivate *priv;
} BonoboItemContainer;

typedef struct {
	BonoboXObjectClass parent_class;

	POA_Bonobo_ItemContainer__epv epv;

	Bonobo_Unknown (*get_object) (BonoboItemContainer *item_container,
				      CORBA_char          *item_name,
				      CORBA_boolean        only_if_exists,
				      CORBA_Environment   *ev);
} BonoboItemContainerClass;

GtkType              bonobo_item_container_get_type       (void);
BonoboItemContainer *bonobo_item_container_new            (void);

void                 bonobo_item_container_add            (BonoboItemContainer *container,
							   const char          *name, 
							   BonoboObject        *object);

void                 bonobo_item_container_remove_by_name (BonoboItemContainer *container,
							   const char          *name);

END_GNOME_DECLS

#endif

