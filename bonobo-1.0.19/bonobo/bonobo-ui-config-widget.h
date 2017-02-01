/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-ui-config-widget.h: Bonobo Component UIConfig
 *
 * Author:
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright  2000 Helix Code, Inc.
 */
#ifndef BONOBO_UI_CONFIG_WIDGET_H
#define BONOBO_UI_CONFIG_WIDGET_H

#include <gtk/gtk.h>
#include <bonobo/bonobo-ui-engine.h>

BEGIN_GNOME_DECLS

#define BONOBO_UI_CONFIG_WIDGET(obj)		GTK_CHECK_CAST(obj,  bonobo_ui_config_widget_get_type (), BonoboUIConfigWidget)
#define BONOBO_UI_CONFIG_WIDGET_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, bonobo_ui_config_widget_get_type (), BonoboUIConfigWidgetClass)
#define BONOBO_IS_UI_CONFIG_WIDGET(obj)		GTK_CHECK_TYPE (obj, bonobo_ui_config_widget_get_type ())

typedef struct _BonoboUIConfigWidgetPrivate BonoboUIConfigWidgetPrivate;

typedef struct {
	GtkVBox parent;

	BonoboUIEngine *engine;

	BonoboUIConfigWidgetPrivate *priv;
} BonoboUIConfigWidget;

typedef struct {
	GtkVBoxClass parent_class;
} BonoboUIConfigWidgetClass;

GtkType	   bonobo_ui_config_widget_get_type  (void);

GtkWidget *bonobo_ui_config_widget_construct (BonoboUIConfigWidget *config,
					      BonoboUIEngine       *engine,
					      GtkAccelGroup        *accel_group);

GtkWidget *bonobo_ui_config_widget_new       (BonoboUIEngine       *engine,
					      GtkAccelGroup        *accel_group);

END_GNOME_DECLS

#endif /* BONOBO_UI_CONFIG_WIDGET_H */
