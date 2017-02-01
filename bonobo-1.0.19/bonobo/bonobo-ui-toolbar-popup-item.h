/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-popup-item.h
 *
 * Author:
 *    Ettore Perazzoli
 *
 * Copyright (C) 2000 Helix Code, Inc.
 */

#ifndef _BONOBO_UI_TOOLBAR_POPUP_ITEM_H_
#define _BONOBO_UI_TOOLBAR_POPUP_ITEM_H_

#include <libgnome/gnome-defs.h>
#include "bonobo-ui-toolbar-toggle-button-item.h"

BEGIN_GNOME_DECLS

#define BONOBO_TYPE_UI_TOOLBAR_POPUP_ITEM            (bonobo_ui_toolbar_popup_item_get_type ())
#define BONOBO_UI_TOOLBAR_POPUP_ITEM(obj)            (GTK_CHECK_CAST ((obj), BONOBO_TYPE_UI_TOOLBAR_POPUP_ITEM, BonoboUIToolbarPopupItem))
#define BONOBO_UI_TOOLBAR_POPUP_ITEM_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_TOOLBAR_POPUP_ITEM, BonoboUIToolbarPopupItemClass))
#define BONOBO_IS_UI_TOOLBAR_POPUP_ITEM(obj)	     (GTK_CHECK_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_POPUP_ITEM))
#define BONOBO_IS_UI_TOOLBAR_POPUP_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_POPUP_ITEM))


typedef struct _BonoboUIToolbarPopupItemPrivate BonoboUIToolbarPopupItemPrivate;

typedef struct {
	BonoboUIToolbarToggleButtonItem parent;
} BonoboUIToolbarPopupItem;

typedef struct {
	BonoboUIToolbarToggleButtonItemClass parent_class;
} BonoboUIToolbarPopupItemClass;


GtkType    bonobo_ui_toolbar_popup_item_get_type  (void);
GtkWidget *bonobo_ui_toolbar_popup_item_new       (void);
void       bonobo_ui_toolbar_popup_item_construct (BonoboUIToolbarPopupItem *);

END_GNOME_DECLS

#endif /* _BONOBO_UI_TOOLBAR_POPUP_ITEM_H_ */
