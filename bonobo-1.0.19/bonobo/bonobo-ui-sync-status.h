/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-ui-sync-status.h: The Bonobo UI/XML sync engine for statuss
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#ifndef _BONOBO_UI_SYNC_STATUS_H_
#define _BONOBO_UI_SYNC_STATUS_H_

#include <gtk/gtkstatusbar.h>

#include <bonobo/bonobo-ui-sync.h>

BEGIN_GNOME_DECLS

#define BONOBO_TYPE_UI_SYNC_STATUS            (bonobo_ui_sync_status_get_type ())
#define BONOBO_UI_SYNC_STATUS(obj)            (GTK_CHECK_CAST ((obj), BONOBO_TYPE_UI_SYNC_STATUS, BonoboUISyncStatus))
#define BONOBO_UI_SYNC_STATUS_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_SYNC_STATUS, BonoboUISyncStatusClass))
#define BONOBO_IS_UI_SYNC_STATUS(obj)         (GTK_CHECK_TYPE ((obj), BONOBO_TYPE_UI_SYNC_STATUS))
#define BONOBO_IS_UI_SYNC_STATUS_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), BONOBO_TYPE_UI_SYNC_STATUS))

typedef struct _BonoboUISyncStatusPrivate BonoboUISyncStatusPrivate;

typedef struct {
	BonoboUISync parent;

	GtkBox       *status;
	GtkStatusbar *main_status;

	BonoboUISyncStatusPrivate *priv;
} BonoboUISyncStatus;

typedef struct {
	BonoboUISyncClass parent_class;
} BonoboUISyncStatusClass;

BonoboUISync *bonobo_ui_sync_status_new      (BonoboUIEngine *engine,
					      GtkBox         *status);

END_GNOME_DECLS

#endif /* _BONOBO_UI_SYNC_STATUS_H_ */
