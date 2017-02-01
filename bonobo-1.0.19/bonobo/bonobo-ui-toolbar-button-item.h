/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-button-item.h: a toolbar button
 *
 * Author: Ettore Perazzoli
 *
 * Copyright (C) 2000 Helix Code, Inc.
 */

#ifndef _BONOBO_UI_TOOLBAR_BUTTON_ITEM_H_
#define _BONOBO_UI_TOOLBAR_BUTTON_ITEM_H_

#include <libgnome/gnome-defs.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "bonobo-ui-toolbar-item.h"

BEGIN_GNOME_DECLS

#define BONOBO_TYPE_UI_TOOLBAR_BUTTON_ITEM		(bonobo_ui_toolbar_button_item_get_type ())
#define BONOBO_UI_TOOLBAR_BUTTON_ITEM(obj)		(GTK_CHECK_CAST ((obj), BONOBO_TYPE_UI_TOOLBAR_BUTTON_ITEM, BonoboUIToolbarButtonItem))
#define BONOBO_UI_TOOLBAR_BUTTON_ITEM_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_TOOLBAR_BUTTON_ITEM, BonoboUIToolbarButtonItemClass))
#define BONOBO_IS_UI_TOOLBAR_BUTTON_ITEM(obj)		(GTK_CHECK_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_BUTTON_ITEM))
#define BONOBO_IS_UI_TOOLBAR_BUTTON_ITEM_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_BUTTON_ITEM))

typedef struct _BonoboUIToolbarButtonItemPrivate BonoboUIToolbarButtonItemPrivate;

typedef struct {
	BonoboUIToolbarItem parent;

	BonoboUIToolbarButtonItemPrivate *priv;
} BonoboUIToolbarButtonItem;

typedef struct {
	BonoboUIToolbarItemClass parent_class;

	/* Virtual methods */
	void (* set_icon)       (BonoboUIToolbarButtonItem *button_item,
				 GdkPixbuf                 *icon);
	void (* set_label)      (BonoboUIToolbarButtonItem *button_item,
				 const char                *label);

	/* Signals.  */
	void (* clicked)	(BonoboUIToolbarButtonItem *toolbar_button_item);
	void (* set_want_label) (BonoboUIToolbarButtonItem *toolbar_button_item);
} BonoboUIToolbarButtonItemClass;

GtkType    bonobo_ui_toolbar_button_item_get_type           (void);
void       bonobo_ui_toolbar_button_item_construct          (BonoboUIToolbarButtonItem *item,
							     GtkButton                 *button_widget,
							     GdkPixbuf                 *icon,
							     const char                *label);
GtkWidget *bonobo_ui_toolbar_button_item_new                (GdkPixbuf                 *icon,
							     const char                *label);

void       bonobo_ui_toolbar_button_item_set_icon           (BonoboUIToolbarButtonItem *button_item,
							     GdkPixbuf                 *icon);
void       bonobo_ui_toolbar_button_item_set_label          (BonoboUIToolbarButtonItem *button_item,
							     const char                *label);

GtkButton *bonobo_ui_toolbar_button_item_get_button_widget  (BonoboUIToolbarButtonItem *button_item);

END_GNOME_DECLS

#endif /* _BONOBO_UI_TOOLBAR_BUTTON_ITEM_H_ */
