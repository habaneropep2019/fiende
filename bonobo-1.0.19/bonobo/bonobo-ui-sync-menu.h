/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-ui-sync-menu.h: The Bonobo UI/XML sync engine for menus
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#ifndef _BONOBO_UI_SYNC_MENU_H_
#define _BONOBO_UI_SYNC_MENU_H_

#include <gtk/gtkmenu.h>
#include <gtk/gtkaccelgroup.h>

#include <bonobo/bonobo-ui-sync.h>

BEGIN_GNOME_DECLS

#define BONOBO_TYPE_UI_SYNC_MENU            (bonobo_ui_sync_menu_get_type ())
#define BONOBO_UI_SYNC_MENU(obj)            (GTK_CHECK_CAST ((obj), BONOBO_TYPE_UI_SYNC_MENU, BonoboUISyncMenu))
#define BONOBO_UI_SYNC_MENU_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_SYNC_MENU, BonoboUISyncMenuClass))
#define BONOBO_IS_UI_SYNC_MENU(obj)         (GTK_CHECK_TYPE ((obj), BONOBO_TYPE_UI_SYNC_MENU))
#define BONOBO_IS_UI_SYNC_MENU_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), BONOBO_TYPE_UI_SYNC_MENU))

typedef struct _BonoboUISyncMenuPrivate BonoboUISyncMenuPrivate;

typedef struct {
	BonoboUISync parent;

	GtkMenuBar    *menu;
	GtkWidget     *menu_dock_item;
	GtkAccelGroup *accel_group;
	GHashTable    *radio_groups;
	GSList        *popups;

	BonoboUISyncMenuPrivate *priv;
} BonoboUISyncMenu;

typedef struct {
	BonoboUISyncClass parent_class;
} BonoboUISyncMenuClass;

BonoboUISync *bonobo_ui_sync_menu_new          (BonoboUIEngine *engine,
						GtkMenuBar     *menu,
						GtkWidget      *menu_dock_item,
						GtkAccelGroup  *group);

void          bonobo_ui_sync_menu_remove_popup (BonoboUISyncMenu *sync,
						const char       *path);

void          bonobo_ui_sync_menu_add_popup    (BonoboUISyncMenu *sync,
						GtkMenu          *menu,
						const char       *path);

END_GNOME_DECLS

#endif /* _BONOBO_UI_SYNC_MENU_H_ */
