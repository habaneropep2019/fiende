/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Author:
 *   Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright 2000 Maurer IT Systemlösungen (http://www.maurer-it.com)
 */

#ifndef _BONOBO_STORAGE_PLUGIN_H_
#define _BONOBO_STORAGE_PLUGIN_H_

#include <bonobo/bonobo-storage.h>
#include <gmodule.h>

#define BONOBO_STORAGE_VERSION "1.0"

BEGIN_GNOME_DECLS

typedef struct          _StoragePlugin         StoragePlugin;
typedef gint           (*StoragePluginInitFn) (StoragePlugin *plugin);
typedef BonoboStorage *(*BonoboStorageOpenFn) (const char *path, 
					       gint flags, 
					       gint mode, 
					       CORBA_Environment *ev);
typedef BonoboStream  *(*BonoboStreamOpenFn)  (const char *path, 
					       gint flags, 
					       gint mode,
					       CORBA_Environment *ev);

struct _StoragePlugin {
	/* public, read only */
	gchar               *filename;     
	gchar               *name;         /* efs, file */
	gchar               *description;
	gchar               *version;
	BonoboStorageOpenFn  storage_open;
	BonoboStreamOpenFn   stream_open;
	/* private */
	GModule             *handle;
};

extern GList *storage_plugin_list;

/* Each plugin must have this one function */
extern gint init_storage_plugin (StoragePlugin *plugin);

void           bonobo_storage_load_plugins (void);
StoragePlugin *bonobo_storage_plugin_find  (const gchar *name);

END_GNOME_DECLS

#endif /* _BONOBO_STORAGE_PLUGIN_H_ */
