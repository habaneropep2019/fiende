/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-item.h
 *
 * Author: Ettore Perazzoli
 *
 * Copyright (C) 2000 Helix Code, Inc.
 */

#ifndef _BONOBO_UI_TOOLBAR_ITEM_H_
#define _BONOBO_UI_TOOLBAR_ITEM_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtk.h>

BEGIN_GNOME_DECLS

#define BONOBO_TYPE_UI_TOOLBAR_ITEM			(bonobo_ui_toolbar_item_get_type ())
#define BONOBO_UI_TOOLBAR_ITEM(obj)			(GTK_CHECK_CAST ((obj), BONOBO_TYPE_UI_TOOLBAR_ITEM, BonoboUIToolbarItem))
#define BONOBO_UI_TOOLBAR_ITEM_CLASS(klass)		(GTK_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_TOOLBAR_ITEM, BonoboUIToolbarItemClass))
#define BONOBO_IS_UI_TOOLBAR_ITEM(obj)			(GTK_CHECK_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_ITEM))
#define BONOBO_IS_UI_TOOLBAR_ITEM_CLASS(klass)		(GTK_CHECK_CLASS_TYPE ((obj), BONOBO_TYPE_UI_TOOLBAR_ITEM))


typedef enum {
	BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_VERTICAL,
	BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_HORIZONTAL,
	BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_ONLY,
	BONOBO_UI_TOOLBAR_ITEM_STYLE_TEXT_ONLY
} BonoboUIToolbarItemStyle;


typedef struct _BonoboUIToolbarItemPrivate BonoboUIToolbarItemPrivate;

typedef struct {
	GtkBin parent;

	BonoboUIToolbarItemPrivate *priv;
} BonoboUIToolbarItem;

typedef struct {
	GtkBinClass parent_class;

	/* Virtual method */
	void (* set_state)       (BonoboUIToolbarItem     *item,
				  const char              *new_state);
	void (* set_tooltip)     (BonoboUIToolbarItem     *item,
				  GtkTooltips             *tooltips,
				  const char              *tooltip);

	/* Signals */
	void (* set_orientation) (BonoboUIToolbarItem     *item,
				  GtkOrientation           orientation);
	void (* set_style)       (BonoboUIToolbarItem     *item,
				  BonoboUIToolbarItemStyle style);
	void (* set_want_label)  (BonoboUIToolbarItem     *item,
				  gboolean                 want_label);
	void (* activate)        (BonoboUIToolbarItem     *item);
} BonoboUIToolbarItemClass;


GtkType                   bonobo_ui_toolbar_item_get_type         (void);
GtkWidget                *bonobo_ui_toolbar_item_new              (void);
void                      bonobo_ui_toolbar_item_set_tooltip      (BonoboUIToolbarItem      *item,
								   GtkTooltips              *tooltips,
								   const char               *tooltip);
void                      bonobo_ui_toolbar_item_set_state        (BonoboUIToolbarItem      *item,
								   const char               *new_state);
void                      bonobo_ui_toolbar_item_set_orientation  (BonoboUIToolbarItem      *item,
								   GtkOrientation            orientation);
GtkOrientation            bonobo_ui_toolbar_item_get_orientation  (BonoboUIToolbarItem      *item);
void                      bonobo_ui_toolbar_item_set_style        (BonoboUIToolbarItem      *item,
								   BonoboUIToolbarItemStyle  style);
BonoboUIToolbarItemStyle  bonobo_ui_toolbar_item_get_style        (BonoboUIToolbarItem      *item);

void			 bonobo_ui_toolbar_item_set_minimum_width(BonoboUIToolbarItem *item,
								  int minimum_width);

/* FIXME ugly names.  */
void                      bonobo_ui_toolbar_item_set_want_label   (BonoboUIToolbarItem      *button_item,
								   gboolean                  prefer_text);
gboolean                  bonobo_ui_toolbar_item_get_want_label   (BonoboUIToolbarItem      *button_item);

void                      bonobo_ui_toolbar_item_set_expandable   (BonoboUIToolbarItem      *button_item,
								   gboolean                  expandable);
gboolean                  bonobo_ui_toolbar_item_get_expandable   (BonoboUIToolbarItem      *button_item);

void                      bonobo_ui_toolbar_item_set_pack_end     (BonoboUIToolbarItem      *button_item,
								   gboolean                  expandable);
gboolean                  bonobo_ui_toolbar_item_get_pack_end     (BonoboUIToolbarItem      *button_item);

void                      bonobo_ui_toolbar_item_activate         (BonoboUIToolbarItem     *item);

END_GNOME_DECLS

#endif /* __BONOBO_UI_TOOLBAR_ITEM_H__ */
