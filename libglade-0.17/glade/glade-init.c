/* -*- Mode: C; c-basic-offset: 8 -*-
 * libglade - a library for building interfaces from XML files at runtime
 * Copyright (C) 1998-2001  James Henstridge <james@daa.com.au>
 *
 * glade-init.c: initialisation functions for libglade
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>
#include <gmodule.h>

void glade_init_gtk_widgets (void);

/**
 * glade_init:
 * 
 * Should be called before creating any GladeXML objects.  Currently all it
 * does is register all the widget building routines, so they can be found
 * by widget name.  In the future it may do some more initialisation work.
 */
void
glade_init(void)
{
	static gboolean initialised = FALSE;

	if (initialised) return;
	initialised = TRUE;
	glade_init_gtk_widgets();

	/*probably should do something about auto-loading of widget sets here*/
}

/**
 * glade_load_module:
 * @module: the shared library.
 *
 * This routine loads the shared library @module, and calls the function
 * glade_init_module.  If the file doesn't exist, or it doesn't contain
 * a glade_init_module function, the loading fails.  The idea of this
 * function is to allow new widget building routines to be loaded into
 * libglade at runtime.
 *
 * This would be a lot more useful if special tags in the XML file could
 * trigger the loading of modules.
 */
void
glade_load_module (const char *module)
{
	GModule *mod;
	char *module_name;
	void (*init_func)(void) = NULL;

	if (!g_module_supported()) {
		g_warning("No gmodule support -- module '%s' not loaded", module);
		return;
	}

	if (module[0]=='/' || (module[0]=='l' && module[1]=='i' && module[2]=='b'))
		module_name = g_strdup(module);
	else
		module_name = g_strconcat("lib", module, ".so", NULL);
	mod = g_module_open(module_name, G_MODULE_BIND_LAZY);

	if (mod && g_module_symbol(mod, "glade_init_module", (gpointer*)&init_func)) {
		if (init_func) {
			g_module_make_resident(mod);
			init_func();
		} else
			g_module_close(mod);
	} else {
		g_warning("Failed to load module '%s': %s",
			  mod ? g_module_name(mod) : module_name, g_module_error());
		g_module_close(mod);
	}

	g_free(module_name);
}
