/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-ui-sync-keys.h: The Bonobo UI/XML sync engine for keys bindings
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#ifndef _BONOBO_UI_SYNC_KEYS_H_
#define _BONOBO_UI_SYNC_KEYS_H_

#include <gtk/gtkstatusbar.h>

#include <bonobo/bonobo-ui-sync.h>

BEGIN_GNOME_DECLS

#define BONOBO_TYPE_UI_SYNC_KEYS            (bonobo_ui_sync_keys_get_type ())
#define BONOBO_UI_SYNC_KEYS(obj)            (GTK_CHECK_CAST ((obj), BONOBO_TYPE_UI_SYNC_KEYS, BonoboUISyncKeys))
#define BONOBO_UI_SYNC_KEYS_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BONOBO_TYPE_UI_SYNC_KEYS, BonoboUISyncKeysClass))
#define BONOBO_IS_UI_SYNC_KEYS(obj)         (GTK_CHECK_TYPE ((obj), BONOBO_TYPE_UI_SYNC_KEYS))
#define BONOBO_IS_UI_SYNC_KEYS_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), BONOBO_TYPE_UI_SYNC_KEYS))

typedef struct _BonoboUISyncKeysPrivate BonoboUISyncKeysPrivate;

typedef struct {
	BonoboUISync parent;

	GHashTable   *keybindings;

	BonoboUISyncKeysPrivate *priv;
} BonoboUISyncKeys;

typedef struct {
	BonoboUISyncClass parent_class;
} BonoboUISyncKeysClass;

BonoboUISync *bonobo_ui_sync_keys_new            (BonoboUIEngine   *engine);

gint          bonobo_ui_sync_keys_binding_handle (GtkWidget        *widget,
						  GdkEventKey      *event,
						  BonoboUISyncKeys *msync);

END_GNOME_DECLS

#endif /* _BONOBO_UI_SYNC_KEYS_H_ */
