/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-configuration.c - Handling of the GNOME Virtual File System
   configuration.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@gnu.org> */

#include <config.h>
#include "gnome-vfs-configuration.h"

#include "gnome-vfs-private.h"
#include "gnome-vfs.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct _Configuration Configuration;
struct _Configuration {
	GHashTable *method_to_module_path;
	time_t last_checked;
	GList *directories;
};

typedef struct _ModulePathElement ModulePathElement;
struct _ModulePathElement {
	char *method_name;
	char *path;
	char *args;
};

typedef struct _VfsDirSource VfsDirSource;
struct _VfsDirSource {
	char *dirname;
	struct stat s;
	unsigned int valid : 1;
};

/* Global variable */
static Configuration *configuration = NULL;

G_LOCK_DEFINE_STATIC (configuration);
#define MAX_CFG_FILES 128



static ModulePathElement *
module_path_element_new (const char *method_name,
			 const char *path,
			 const char *args)
{
	ModulePathElement *new;

	new = g_new (ModulePathElement, 1);
	new->method_name = g_strdup (method_name);
	new->path = g_strdup (path);
	new->args = g_strdup (args);

	return new;
}

static void
module_path_element_free (ModulePathElement *module_path)
{
	g_free (module_path->method_name);
	g_free (module_path->path);
	g_free (module_path->args);
	g_free (module_path);
}

static VfsDirSource *
vfs_dir_source_new (const char *dirname)
{
	VfsDirSource *new;

	new = g_new (VfsDirSource, 1);
	new->dirname = g_strdup (dirname);

	return new;
}

static void
vfs_dir_source_free (VfsDirSource *vfs_source)
{
	g_free (vfs_source->dirname);
	g_free (vfs_source);
}



static void
hash_free_module_path (gpointer key,
		       gpointer value,
		       gpointer user_data)
{
	ModulePathElement *module_path;

	module_path = (ModulePathElement *) value;
	module_path_element_free (module_path);
}

/* Destroy configuration information.  */
static void
configuration_destroy (Configuration *configuration)
{
	g_return_if_fail (configuration != NULL);

	g_hash_table_foreach (configuration->method_to_module_path,
			      hash_free_module_path, NULL);
	g_hash_table_destroy (configuration->method_to_module_path);
	g_list_foreach (configuration->directories, (GFunc) vfs_dir_source_free, NULL);
	g_list_free (configuration->directories);
	g_free (configuration);
}



/* This reads a line and handles backslashes at the end of the line to join
   lines.  */
static gint
read_line (FILE *stream,
	   gchar **line_return,
	   guint *n,
	   guint *lines_read)
{
#define START_BUFFER_SIZE 1024
	gboolean backslash;
	gint pos;

	if (feof (stream))
		return -1;

	pos = 0;
	backslash = FALSE;
	*lines_read = 0;
	while (1) {
		int c;

		if (pos == *n) {
			if (*n == 0)
				*n = START_BUFFER_SIZE;
			else
				*n *= 2;
			*line_return = g_realloc (*line_return, *n);
		}

		c = fgetc (stream);
		if (c == '\n')
			(*lines_read)++;
		if (c == EOF || (c == '\n' && ! backslash)) {
			(*line_return)[pos] = 0;
			return pos;
		}

		if (c == '\\' && ! backslash) {
			backslash = TRUE;
		} else if (c != '\n') {
			if (backslash)
				(*line_return)[pos++] = '\\';
			(*line_return)[pos] = c;
			pos++;
			backslash = FALSE;
		}
	}
#undef START_BUFFER_SIZE
}

static void
remove_comment (gchar *buf)
{
	gchar *p;

	p = strchr (buf, '#');
	if (p != NULL)
		*p = '\0';
}

static gboolean
parse_line (Configuration *configuration,
	    gchar *line_buffer,
	    guint line_len,
	    const gchar *file_name,

	    guint line_number)
{
	guint string_len;
	gboolean retval;
	gchar *p;
	gchar *method_start;
	char *module_name;
	char *args = NULL;
	GList *method_list;
	GList *lp;

	string_len = strlen (line_buffer);
	if (string_len != line_len) {
		g_warning (_("%s:%d contains NUL characters."),
			   file_name, line_number);
		return FALSE;
	}

	remove_comment (line_buffer);
	line_buffer = g_strstrip (line_buffer);

	method_list = NULL;
	p = line_buffer;
	method_start = line_buffer;
	retval = TRUE;
	while (*p != '\0') {
		if (*p == ' ' || *p == '\t' || *p == ':') {
			gchar *method_name;

			if (p == method_start) {
				g_warning (_("%s:%d contains no method name."),
					   file_name, line_number);
				retval = FALSE;
				goto cleanup;
			}

			method_name = g_strndup (method_start,
						 p  - method_start);
			method_list = g_list_prepend (method_list, method_name);

			while (*p == ' ' || *p == '\t')
				p++;

			if (*p == ':') {
				p++;
				break;
			}

			method_start = p;
		}

		p++;
	}

	while (*p && isspace ((guchar) *p))
		p++;

	if (*p == '\0') {
		if (method_list != NULL) {
			g_warning (_("%s:%d contains no module name."),
				   file_name, line_number);
			retval = FALSE;
		} else {
			/* Empty line.  */
			retval = TRUE;
		}
		goto cleanup;
	}

	module_name = p;
	while(*p && !isspace ((guchar) *p)) p++;

	if(*p) {
		*p = '\0';
		p++;
		while(*p && isspace ((guchar) *p)) p++;
		if(*p)
			args = p;
	}

	for (lp = method_list; lp != NULL; lp = lp->next) {
		ModulePathElement *element;
		gchar *method_name;

		method_name = lp->data;
		element = module_path_element_new (method_name, module_name, args);
		g_hash_table_insert (configuration->method_to_module_path,
				     method_name, element);
	}

	retval = TRUE;

 cleanup:
	if (method_list != NULL)
		g_list_free (method_list);
	return retval;
}

/* FIXME bugzilla.eazel.com 1139:
   maybe we should return FALSE if any errors during parsing happen so
   that we abort immediately, but this sounds a bit too overkill.  */
static gboolean
parse_file (Configuration *configuration,
	    const gchar *file_name)
{
	FILE *f;
	gchar *line_buffer;
	guint line_buffer_size;
	guint line_number;

	f = fopen (file_name, "r");
	if (f == NULL) {
		g_warning (_("Configuration file `%s' was not found: %s"),
			   file_name, strerror (errno));
		return FALSE;
	}

	line_buffer = NULL;
	line_buffer_size = 0;
	line_number = 0;
	while (1) {
		guint lines_read;
		gint line_len;

		line_len = read_line (f, &line_buffer, &line_buffer_size,
				      &lines_read);
		if (line_len == -1)
			break;	/* EOF */
		parse_line (configuration, line_buffer, line_len, file_name,
			    line_number);
		line_number += lines_read;
	}

	g_free (line_buffer);

	fclose (f);

	return TRUE;
}

static void
configuration_load (void)
{
	gchar *file_names[MAX_CFG_FILES + 1];
	GList *list;
	int i = 0;
	DIR *dirh;

	configuration->method_to_module_path = g_hash_table_new (g_str_hash, g_str_equal);

	/* Go through the list of configuration directories and build up a list of config files */
	for (list = configuration->directories; list && i < MAX_CFG_FILES; list = list->next) {
		VfsDirSource *dir_source = (VfsDirSource *)list->data;
		struct dirent *dent;

		if (stat (dir_source->dirname, &dir_source->s) == -1)
			continue;

		dirh = opendir (dir_source->dirname);
		if(!dirh)
			continue;

		while ((dent = readdir(dirh)) && i < MAX_CFG_FILES) {
			char *ctmp;
			ctmp = strstr(dent->d_name, ".conf");
			if(!ctmp || strcmp(ctmp, ".conf"))
				continue;
			file_names[i] = g_strdup_printf ("%s/%s", dir_source->dirname, dent->d_name);
			i++;
		}
		closedir(dirh);
	}
	file_names[i] = NULL;

	/* Now read these cfg files */
	for(i = 0; file_names[i]; i++) {
		/* FIXME: should we try to catch errors? */
		parse_file (configuration, file_names[i]);
		g_free (file_names[i]);
	}
}


static void
add_directory_internal (const char *dir)
{
	VfsDirSource *dir_source = vfs_dir_source_new (dir);

	configuration->directories = g_list_prepend (configuration->directories, dir_source);
}

void
gnome_vfs_configuration_add_directory (const char *dir)
{
	G_LOCK (configuration);
	if (configuration == NULL) {
		g_warning ("gnome_vfs_configuration_init must be called prior to adding a directory.");
		G_UNLOCK (configuration);
		return;
	}

	add_directory_internal (dir);

	G_UNLOCK (configuration);
}


static void
install_path_list (const gchar *environment_path)
{
	const char *p, *oldp;

	oldp = environment_path;
	while (1) {
		char *elem;

		p = strchr (oldp, ':');

		if (p == NULL) {
			if (*oldp != '\0') {
				add_directory_internal (oldp);
			}
			break;
		} else {
			elem = g_strndup (oldp, p - oldp);
			add_directory_internal (elem);
			g_free (elem);
		} 

		oldp = p + 1;
	}
}


gboolean
gnome_vfs_configuration_init (void)
{
	char *home_config;
	char *environment_path;

	G_LOCK (configuration);
	if (configuration != NULL) {
		G_UNLOCK (configuration);
		return FALSE;
	}

	configuration = g_new0 (Configuration, 1);

	home_config = g_strdup_printf ("%s%c%s",
				       g_get_home_dir (),
				       G_DIR_SEPARATOR,
				       ".gnome/vfs/modules");
	add_directory_internal (GNOME_VFS_MODULE_CFGDIR);
	environment_path = getenv ("GNOME_VFS_MODULE_CONFIG_PATH");
	if (environment_path != NULL) {
		install_path_list (environment_path);
	}
	add_directory_internal (home_config);
	g_free (home_config);

	configuration_load ();

	G_UNLOCK (configuration);

	if (configuration == NULL)
		return FALSE;
	else
		return TRUE;
}

void
gnome_vfs_configuration_uninit (void)
{
	G_LOCK (configuration);
	if (configuration == NULL) {
		G_UNLOCK (configuration);
		return;
	}

	configuration_destroy (configuration);
	configuration = NULL;
	G_UNLOCK (configuration);
}

static void
maybe_reload (void)
{
	time_t now = time (NULL);
	GList *list;
	gboolean need_reload = FALSE;
	struct stat s;

	/* only check every 5 seconds minimum */
	if (configuration->last_checked + 5 >= now)
		return;

	for (list = configuration->directories; list; list = list->next) {
		VfsDirSource *dir_source = (VfsDirSource *) list->data;
		if (stat (dir_source->dirname, &s) == -1)
			continue;
		if (s.st_mtime != dir_source->s.st_mtime) {
			need_reload = TRUE;
			break;
		}
	}

	configuration->last_checked = now;

	if (!need_reload)
		return;

	configuration->last_checked = time (NULL);

	g_hash_table_foreach (configuration->method_to_module_path,
			      hash_free_module_path, NULL);
	g_hash_table_destroy (configuration->method_to_module_path);
	configuration_load ();
}

const gchar *
gnome_vfs_configuration_get_module_path (const gchar *method_name, const char ** args)
{
	ModulePathElement *element;

	g_return_val_if_fail (method_name != NULL, NULL);

	G_LOCK (configuration);

	maybe_reload ();

	if (configuration != NULL) {
		element = g_hash_table_lookup
			(configuration->method_to_module_path, method_name);
	} else {
		/* This should never happen.  */
		g_warning ("Internal error: the configuration system was not initialized. Did you call gnome_vfs_configuration_init?");
		element = NULL;
	}
	G_UNLOCK (configuration);

	if (element == NULL)
		return NULL;
	else {
		if(args)
			*args = element->args;
		return element->path;
	}
}
