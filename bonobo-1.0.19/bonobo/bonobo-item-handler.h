/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-item-handler.h: a generic ItemContainer handler for monikers.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 * Copyright 1999, 2000 Miguel de Icaza
 */

#ifndef _BONOBO_ITEM_HANDLER_H_
#define _BONOBO_ITEM_HANDLER_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_ITEM_HANDLER_TYPE        (bonobo_item_handler_get_type ())
#define BONOBO_ITEM_HANDLER(o)          (GTK_CHECK_CAST ((o), BONOBO_ITEM_HANDLER_TYPE, BonoboItemHandler))
#define BONOBO_ITEM_HANDLER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_ITEM_HANDLER_TYPE, BonoboItemHandlerClass))
#define BONOBO_IS_ITEM_HANDLER(o)       (GTK_CHECK_TYPE ((o), BONOBO_ITEM_HANDLER_TYPE))
#define BONOBO_IS_ITEM_HANDLER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_ITEM_HANDLER_TYPE))

typedef struct _BonoboItemHandlerPrivate BonoboItemHandlerPrivate;
typedef struct _BonoboItemHandler        BonoboItemHandler;

typedef Bonobo_ItemContainer_ObjectNames *(*BonoboItemHandlerEnumObjectsFn)
	(BonoboItemHandler *h, gpointer data, CORBA_Environment *);

typedef Bonobo_Unknown (*BonoboItemHandlerGetObjectFn)
	(BonoboItemHandler *h, const char *item_name, gboolean only_if_exists,
	 gpointer data, CORBA_Environment *ev);

struct _BonoboItemHandler {
	BonoboXObject base;

	POA_Bonobo_ItemContainer__epv epv;

	BonoboItemHandlerEnumObjectsFn enum_objects;
	BonoboItemHandlerGetObjectFn   get_object;
	gpointer                       user_data;

	BonoboItemHandlerPrivate      *priv;
};

typedef struct {
	BonoboXObjectClass parent_class;

	POA_Bonobo_ItemContainer__epv epv;
} BonoboItemHandlerClass;

GtkType              bonobo_item_handler_get_type    (void);
BonoboItemHandler   *bonobo_item_handler_new         (BonoboItemHandlerEnumObjectsFn enum_objects,
						      BonoboItemHandlerGetObjectFn   get_object,
						      gpointer                       user_data);

BonoboItemHandler   *bonobo_item_handler_construct   (BonoboItemHandler             *handler,
						      BonoboItemHandlerEnumObjectsFn enum_objects,
						      BonoboItemHandlerGetObjectFn   get_object,
						      gpointer                       user_data);

/* Utility functions that can be used by getObject routines */
typedef struct {
	char *key;
	char *value;
} BonoboItemOption;

GSList *bonobo_item_option_parse (const char *option_string);
void    bonobo_item_options_free (GSList *options);

END_GNOME_DECLS

#endif

