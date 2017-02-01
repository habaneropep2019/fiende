/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar.h
 *
 * Author:
 *    Ettore Perazzoli
 *
 * Copyright (C) 2000 Helix Code, Inc.
 */

#ifndef _BONOBO_UI_TOOLBAR_H_
#define _BONOBO_UI_TOOLBAR_H_

#include <gtk/gtkcontainer.h>
#include <libgnome/gnome-defs.h>
#include "bonobo-ui-toolbar-item.h"

BEGIN_GNOME_DECLS

#define BONOBO_TYPE_UI_TOOLBAR            (bonobo_ui_toolbar_get_type ())
#define BONOBO_UI_TOOLBAR(obj)            (GTK_CHECK_CAST ((obj), BONOBO_TYPE_UI_TOOLBAR, BonoboUIToolbar))
#define BONOBO_UI_TOOLBAR_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_TOOLBAR, BonoboUIToolbarClass))
#define BONOBO_IS_UI_TOOLBAR(obj)         (GTK_CHECK_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR))
#define BONOBO_IS_UI_TOOLBAR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR))


enum _BonoboUIToolbarStyle {
	BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT,
	BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT,
	BONOBO_UI_TOOLBAR_STYLE_ICONS_ONLY
};
typedef enum _BonoboUIToolbarStyle BonoboUIToolbarStyle;

typedef struct _BonoboUIToolbarPrivate BonoboUIToolbarPrivate;

typedef struct {
	GtkContainer parent;

	BonoboUIToolbarPrivate *priv;
} BonoboUIToolbar;

typedef struct {
	GtkContainerClass parent_class;

	void (* set_orientation) (BonoboUIToolbar *toolbar,
				  GtkOrientation orientation);
	void (* style_changed)   (BonoboUIToolbar *toolbar);
} BonoboUIToolbarClass;


GtkType               bonobo_ui_toolbar_get_type         (void);
void                  bonobo_ui_toolbar_construct        (BonoboUIToolbar      *toolbar);
GtkWidget            *bonobo_ui_toolbar_new              (void);

void                  bonobo_ui_toolbar_set_orientation  (BonoboUIToolbar      *toolbar,
							  GtkOrientation        orientation);
GtkOrientation        bonobo_ui_toolbar_get_orientation  (BonoboUIToolbar      *toolbar);

BonoboUIToolbarStyle  bonobo_ui_toolbar_get_style        (BonoboUIToolbar      *toolbar);

void                  bonobo_ui_toolbar_set_hv_styles    (BonoboUIToolbar      *toolbar,
							  BonoboUIToolbarStyle  hstyle,
							  BonoboUIToolbarStyle  vstyle);

void                  bonobo_ui_toolbar_insert           (BonoboUIToolbar      *toolbar,
							  BonoboUIToolbarItem  *item,
							  int                   position);

GtkTooltips          *bonobo_ui_toolbar_get_tooltips     (BonoboUIToolbar      *toolbar);
void                  bonobo_ui_toolbar_show_tooltips    (BonoboUIToolbar      *toolbar,
							  gboolean              show_tips);


GList                *bonobo_ui_toolbar_get_children     (BonoboUIToolbar      *toolbar);

END_GNOME_DECLS

#endif /* _BONOBO_UI_TOOLBAR_H_ */

