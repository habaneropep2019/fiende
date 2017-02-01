/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-ui-sync-toolbar.h: The Bonobo UI/XML sync engine for toolbars
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#ifndef _BONOBO_UI_SYNC_TOOLBAR_H_
#define _BONOBO_UI_SYNC_TOOLBAR_H_

#include <libgnomeui/gnome-dock.h>
#include <bonobo/bonobo-ui-toolbar.h>
#include <bonobo/bonobo-ui-sync.h>

BEGIN_GNOME_DECLS

#define BONOBO_TYPE_UI_SYNC_TOOLBAR            (bonobo_ui_sync_toolbar_get_type ())
#define BONOBO_UI_SYNC_TOOLBAR(obj)            (GTK_CHECK_CAST ((obj), BONOBO_TYPE_UI_SYNC_TOOLBAR, BonoboUISyncToolbar))
#define BONOBO_UI_SYNC_TOOLBAR_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_SYNC_TOOLBAR, BonoboUISyncToolbarClass))
#define BONOBO_IS_UI_SYNC_TOOLBAR(obj)         (GTK_CHECK_TYPE ((obj), BONOBO_TYPE_UI_SYNC_TOOLBAR))
#define BONOBO_IS_UI_SYNC_TOOLBAR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), BONOBO_TYPE_UI_SYNC_TOOLBAR))

typedef struct _BonoboUISyncToolbarPrivate BonoboUISyncToolbarPrivate;

typedef struct {
	BonoboUISync parent;

	GnomeDock       *dock;

	BonoboUISyncToolbarPrivate *priv;
} BonoboUISyncToolbar;

typedef struct {
	BonoboUISyncClass parent_class;
} BonoboUISyncToolbarClass;

BonoboUISync *bonobo_ui_sync_toolbar_new      (BonoboUIEngine *engine,
					       GnomeDock      *dock);
BonoboUIToolbarStyle bonobo_ui_sync_toolbar_get_look (BonoboUIEngine *engine,
						      BonoboUINode   *node);

END_GNOME_DECLS

#endif /* _BONOBO_UI_SYNC_TOOLBAR_H_ */
