/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-separator-item.h
 *
 * Author:
 *    Ettore Perazzoli
 *
 * Copyright (C) 2000 Helix Code, Inc.
 */

#ifndef _BONOBO_UI_TOOLBAR_SEPARATOR_ITEM_H_
#define _BONOBO_UI_TOOLBAR_SEPARATOR_ITEM_H_

#include <libgnome/gnome-defs.h>
#include "bonobo-ui-toolbar-item.h"

BEGIN_GNOME_DECLS

#define BONOBO_TYPE_UI_TOOLBAR_SEPARATOR_ITEM            (bonobo_ui_toolbar_separator_item_get_type ())
#define BONOBO_UI_TOOLBAR_SEPARATOR_ITEM(obj)            (GTK_CHECK_CAST ((obj), BONOBO_TYPE_UI_TOOLBAR_SEPARATOR_ITEM, BonoboUIToolbarSeparatorItem))
#define BONOBO_UI_TOOLBAR_SEPARATOR_ITEM_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_TOOLBAR_SEPARATOR_ITEM, BonoboUIToolbarSeparatorItemClass))
#define BONOBO_IS_UI_TOOLBAR_SEPARATOR_ITEM(obj)         (GTK_CHECK_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_SEPARATOR_ITEM))
#define BONOBO_IS_UI_TOOLBAR_SEPARATOR_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_SEPARATOR_ITEM))


typedef struct _BonoboUIToolbarSeparatorItemPrivate BonoboUIToolbarSeparatorItemPrivate;

typedef struct {
	BonoboUIToolbarItem parent;

	BonoboUIToolbarSeparatorItemPrivate *priv;
} BonoboUIToolbarSeparatorItem;

typedef struct {
	BonoboUIToolbarItemClass parent_class;
} BonoboUIToolbarSeparatorItemClass;


GtkType    bonobo_ui_toolbar_separator_item_get_type (void);
GtkWidget *bonobo_ui_toolbar_separator_item_new      (void);

END_GNOME_DECLS

#endif /* _BONOBO_UI_TOOLBAR_SEPARATOR_ITEM_H_ */
