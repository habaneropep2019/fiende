/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-control-item.c: A special toolbar item for controls.
 *
 * Author:
 *	Jon K Hellan (hellan@acm.org)
 *
 * Copyright 2000 Jon K Hellan.
 */
#ifndef _BONOBO_UI_TOOLBAR_CONTROL_ITEM_H_
#define _BONOBO_UI_TOOLBAR_CONTROL_ITEM_H_

#include <libgnome/gnome-defs.h>
#include "bonobo-ui-toolbar-button-item.h"
#include "bonobo-widget.h"

BEGIN_GNOME_DECLS

#define BONOBO_TYPE_UI_TOOLBAR_CONTROL_ITEM            (bonobo_ui_toolbar_control_item_get_type ())
#define BONOBO_UI_TOOLBAR_CONTROL_ITEM(obj)            (GTK_CHECK_CAST ((obj), BONOBO_TYPE_UI_TOOLBAR_CONTROL_ITEM, BonoboUIToolbarControlItem))
#define BONOBO_UI_TOOLBAR_CONTROL_ITEM_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_TOOLBAR_CONTROL_ITEM, BonoboUIToolbarControlItemClass))
#define BONOBO_IS_UI_TOOLBAR_CONTROL_ITEM(obj)         (GTK_CHECK_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_CONTROL_ITEM))
#define BONOBO_IS_UI_TOOLBAR_CONTROL_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_CONTROL_ITEM))

typedef enum {
	BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL,
	BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_BUTTON,
	BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_NONE
} BonoboUIToolbarControlDisplay;

typedef struct _BonoboUIToolbarControlItemPrivate BonoboUIToolbarControlItemPrivate;

typedef struct {
	BonoboUIToolbarButtonItem parent;

	BonoboUIToolbarControlItemPrivate *priv;
} BonoboUIToolbarControlItem;

typedef struct {
	BonoboUIToolbarButtonItemClass parent_class;
} BonoboUIToolbarControlItemClass;

GtkType       bonobo_ui_toolbar_control_item_get_type    (void);
GtkWidget    *bonobo_ui_toolbar_control_item_new         (Bonobo_Control control_ref);
GtkWidget    *bonobo_ui_toolbar_control_item_construct   (BonoboUIToolbarControlItem *control_item,
							  Bonobo_Control              control_ref);
void          bonobo_ui_toolbar_control_item_set_display (BonoboUIToolbarControlItem    *item,
							  BonoboUIToolbarControlDisplay  hdisplay,
							  BonoboUIToolbarControlDisplay  vdisplay);

END_GNOME_DECLS

#endif /* _BONOBO_UI_TOOLBAR_CONTROL_ITEM_H_ */
