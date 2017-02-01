/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-method.c - Handling of access methods in the GNOME
   Virtual File System.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@gnu.org> */

#include <config.h>
#include "gnome-vfs-method.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <glib.h>
#include <gmodule.h>

#include <libgnomevfs/gnome-vfs-transform.h>
#include <libgnomevfs/gnome-vfs-module.h>
#include <libgnomevfs/gnome-vfs-private.h>


struct _ModuleElement {
	char *name;
	const char *args;
	GnomeVFSMethod *method;
	GnomeVFSTransform *transform;
	int nusers;
};
typedef struct _ModuleElement ModuleElement;

static gboolean method_already_initialized = FALSE;
G_LOCK_DEFINE_STATIC (method_already_initialized);

static GHashTable *module_hash = NULL;
G_LOCK_DEFINE_STATIC (module_hash);

static GList *module_path_list = NULL;
G_LOCK_DEFINE_STATIC (module_path_list);


static gboolean
init_hash_table (void)
{
	G_LOCK (module_hash);
	module_hash = g_hash_table_new (g_str_hash, g_str_equal);
	G_UNLOCK (module_hash);

	return TRUE;
}

static gboolean
install_path_list (const gchar *user_path_list)
{
	const gchar *p, *oldp;

	/* Notice that this assumes the list has already been locked.  */

	oldp = user_path_list;
	while (1) {
		gchar *elem;

		p = strchr (oldp, ':');

		if (p == NULL) {
			if (*oldp != '\0') {
				elem = g_strdup (oldp);
				module_path_list = g_list_append
						       (module_path_list, elem);
			}
			break;
		} else if (p != oldp) {
			elem = g_strndup (oldp, p - oldp);
			module_path_list = g_list_append (module_path_list,
							  elem);
		} else {
			elem = NULL;
		}

		oldp = p + 1;
	}

	return TRUE;
}

static gboolean
init_path_list (void)
{
	const gchar *user_path_list;
	gboolean retval;

	retval = TRUE;

	G_LOCK (module_path_list);

	if (module_path_list != NULL) {
		retval = TRUE;
		goto end;
	}

	/* User-supplied path.  */

	user_path_list = getenv ("GNOME_VFS_MODULE_PATH");
	if (user_path_list != NULL) {
		if (! install_path_list (user_path_list)) {
			retval = FALSE;
			goto end;
		}
	}

	/* Default path.  It comes last so that users can override it.  */

	module_path_list = g_list_append (module_path_list,
					  g_strdup (GNOME_VFS_MODULE_DIR));

 end:
	G_UNLOCK (module_path_list);
	return retval;
}

gboolean
gnome_vfs_method_init (void)
{
	G_LOCK (method_already_initialized);

	if (method_already_initialized) {
		G_UNLOCK (method_already_initialized);
		return TRUE;
	}

	if (! init_hash_table ())
		return FALSE;
	if (! init_path_list ())
		return FALSE;

	method_already_initialized = TRUE;
	G_UNLOCK (method_already_initialized);

	return TRUE;
}

static void
load_module (const gchar *module_name, const char *method_name, const char *args,
	     GnomeVFSMethod **method, GnomeVFSTransform **transform)
{
	GModule *module;
	GnomeVFSMethod *temp_method = NULL;
	GnomeVFSTransform *temp_transform = NULL;
	
	GnomeVFSMethodInitFunc init_function = NULL;
	GnomeVFSTransformInitFunc transform_function = NULL;
	GnomeVFSMethodShutdownFunc shutdown_function = NULL;

	*method = NULL;
	*transform = NULL;

	module = g_module_open (module_name, G_MODULE_BIND_LAZY);
	if (module == NULL) {
		g_warning ("Cannot load module `%s' (%s)", module_name, g_module_error ());
		return;
	}

        g_module_symbol (module, GNOME_VFS_MODULE_INIT,
			 (gpointer *) &init_function);
	g_module_symbol (module, GNOME_VFS_MODULE_TRANSFORM,
			 (gpointer *) &transform_function);
	g_module_symbol (module, GNOME_VFS_MODULE_SHUTDOWN,
			 (gpointer *) &shutdown_function);
	
	if ((init_function == NULL || shutdown_function == NULL) &&
	    (transform_function == NULL)) {
		g_warning ("module '%s' has no init function; may be an out-of-date module", module_name);
		return;
	}

	if (init_function)
		temp_method = (* init_function) (method_name, args);

	if (temp_method == NULL && init_function) {
		g_warning ("module '%s' returned a NULL handle", module_name);
		return;
	}

	if (temp_method != NULL) {
		/* Some basic checks */
		if (temp_method->method_table_size == 0) {
			g_warning ("module '%s' has 0 table size", module_name);
			return;
		} else if (temp_method->method_table_size > (0x100 * sizeof (GnomeVFSMethod))) {
			g_warning ("module '%s' has unreasonable table size, perhaps it is using the old GnomeVFSMethod struct?", module_name);
			return;
		} else if (!VFS_METHOD_HAS_FUNC(temp_method, open)) {
			g_warning ("module '%s' has no open fn", module_name);
			return;
#if 0
		} else if (!VFS_METHOD_HAS_FUNC(temp_method, create)) {
			g_warning ("module '%s' has no create fn", module_name);
			return;
#endif
		} else if (!VFS_METHOD_HAS_FUNC(temp_method, is_local)) {
			g_warning ("module '%s' has no is-local fn", module_name);
			return;
#if 0
		} else if (!VFS_METHOD_HAS_FUNC(temp_method, get_file_info)) {
			g_warning ("module '%s' has no get-file-info fn", module_name);
			return;
#endif
		}

		/* More advanced assumptions.  */
		if (VFS_METHOD_HAS_FUNC(temp_method, tell) && !VFS_METHOD_HAS_FUNC(temp_method, seek)) {
			g_warning ("module '%s' has tell and no seek", module_name);
			return;
		}

		if (VFS_METHOD_HAS_FUNC(temp_method, seek) && !VFS_METHOD_HAS_FUNC(temp_method, tell)) {
			g_warning ("module '%s' has seek and no tell", module_name);
			return;
		}
	}

	if (transform_function)
		temp_transform = (* transform_function) (method_name, args);
	if (temp_transform) {
		if (temp_transform->transform == NULL) {
			g_warning ("module '%s' has no transform method", module_name);
			return;
		}
	}

	*method = temp_method;
	*transform = temp_transform;
}

static void
load_module_in_path_list (const gchar *base_name, const char *method_name, const char *args,
			  GnomeVFSMethod **method, GnomeVFSTransform **transform)
{
	GList *p;

	*method = NULL;
	*transform = NULL;
	
	for (p = module_path_list; p != NULL; p = p->next) {
		const gchar *path;
		gchar *name;

		path = p->data;
		name = g_module_build_path (path, base_name);

		load_module (name, method_name, args, method, transform);
		g_free (name);

		if (*method != NULL || *transform != NULL)
			return;
	}
}

static gboolean
gnome_vfs_add_module_to_hash_table (const gchar *name)
{
	GnomeVFSMethod *method = NULL;
	GnomeVFSTransform *transform = NULL;
	ModuleElement *module_element;
	const char *module_name;
	pid_t saved_uid;
	gid_t saved_gid;
	const char *args;

	G_LOCK (module_hash);
	module_element = g_hash_table_lookup (module_hash, name);
	G_UNLOCK (module_hash);

	if (module_element != NULL)
		return TRUE;

	module_name = gnome_vfs_configuration_get_module_path (name, &args);
	if (module_name == NULL)
		return FALSE;

	/* Set the effective UID/GID to the user UID/GID to prevent attacks to
           setuid/setgid executables.  */

	saved_uid = geteuid ();
	saved_gid = getegid ();
	seteuid (getuid ());
	setegid (getgid ());

	if (g_path_is_absolute (module_name))
		load_module (module_name, name, args, &method, &transform);
	else
		load_module_in_path_list (module_name, name, args, &method, &transform);

	seteuid (saved_uid);
	setegid (saved_gid);

	if (method == NULL && transform == NULL)
		return FALSE;

	module_element = g_new (ModuleElement, 1);
	module_element->name = g_strdup (name);
	module_element->method = method;
	module_element->transform = transform;

	G_LOCK (module_hash);
	g_hash_table_insert (module_hash, module_element->name, module_element);
	G_UNLOCK (module_hash);

	return TRUE;
}

GnomeVFSMethod *
gnome_vfs_method_get (const gchar *name)
{
	ModuleElement *module_element;

	g_return_val_if_fail (name != NULL, NULL);

	G_LOCK (module_hash);
	module_element = g_hash_table_lookup (module_hash, name);
	G_UNLOCK (module_hash);

	if (module_element != NULL)
		return module_element->method;

	if (gnome_vfs_add_module_to_hash_table (name)) {
		G_LOCK (module_hash);
		module_element = g_hash_table_lookup (module_hash, name);
		G_UNLOCK (module_hash);
		
		if (module_element != NULL)
			return module_element->method;
	}

	return NULL;
}

GnomeVFSTransform *
gnome_vfs_transform_get (const gchar *name)
{
	ModuleElement *module_element;

	g_return_val_if_fail (name != NULL, NULL);

	G_LOCK (module_hash);
	module_element = g_hash_table_lookup (module_hash, name);
	G_UNLOCK (module_hash);

	if (module_element != NULL)
		return module_element->transform;

	if (gnome_vfs_add_module_to_hash_table (name)) {
		G_LOCK (module_hash);
		module_element = g_hash_table_lookup (module_hash, name);
		G_UNLOCK (module_hash);
		
		if (module_element != NULL)
			return module_element->transform;
	}

	return NULL;
}
