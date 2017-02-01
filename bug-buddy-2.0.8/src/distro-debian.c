/* bug-buddy bug submitting program
 *
 * Copyright (C) Jacob Berkman
 *
 * Author:  Jacob Berkman  <jberkman@andrew.cmu.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <gnome.h>
#include "bug-buddy.h"
#include "distro.h"
#include "util.h"

#define d(x)

static char *get_debian_version (Distribution *distro);
static void get_package_versions (GSList *packages);

Phylum debian_phy = { 
	get_debian_version,
	get_package_versions 
};

static char *
get_debian_version (Distribution *distro)
{
	char *retval, *version;

	g_return_val_if_fail (distro, NULL);
	g_return_val_if_fail (distro->version_file, NULL);
	g_return_val_if_fail (distro->name, NULL);

	version = get_line_from_file (distro->version_file);
	if (!version) {
		d(g_warning ("Could not get distro version"));
		return NULL;
	}
	
	retval = g_strdup_printf ("%s %s", distro->name, version);
	g_free (version);

	return retval;
}

static void
ioc_destroy (gpointer data)
{
	GHashTable *table = data;
	g_hash_table_destroy (table);
	druid_set_sensitive (TRUE, TRUE, TRUE);
	append_packages ();
}


static gboolean
handle_input (GIOChannel *ioc, GIOCondition condition, gpointer data)
{
	GHashTable *table = data;
	char **argv;
	char *line;

	Package *package;

	if (condition == G_IO_HUP)
		return FALSE;

	line = get_line_from_ioc (ioc);
	if (!line) 
		return FALSE;

	argv = g_strsplit (line, " ", 2);
	if (!argv[0] || !argv[1])
		goto end_while;
	package = g_hash_table_lookup (table, argv[0]);
	if (!package)
		goto end_while;
	package->version = g_strdup_printf ("%s %s", package->name, argv[1]);

 end_while:
	g_strfreev (argv);
	
	return TRUE;
}

static void
get_package_versions (GSList *packages)
{
	pid_t pid;
	int argc, fd;
	char **argv, *command, *line;
	Package *package;
	GHashTable *table;
	GSList *list;
	GIOChannel *ioc;

	g_return_if_fail (packages);
	
	argc = 0;
	for (list = packages; list; list = g_slist_next (list)) {
		package = list->data;
		if (!package->version &&
		    package->deb)
			argc++;
	}

	if (argc == 0)
		return;
	
	argv = g_new (char *, argc+1);

	table = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_freeze (table);
		
	argc = 0;
	for (list = packages; list; list = g_slist_next (list)) {
		package = list->data;
		if (package->version ||
		    !package->deb)
			continue;
		argv[argc] = package->deb;
		g_hash_table_insert (table, 
				     package->deb,
				     package);
		argc++;
	}
	
	g_hash_table_thaw (table);
	argv[argc] = NULL;
	line = g_strjoinv (" ", argv);
	g_free (argv);
	command = g_strdup_printf ("dpkg -l %s | tail +6 | "
				   "awk '{ print $2\" \"$3 }'",
				   line);
	g_free (line);
	pid = start_command (command, &fd);
	g_free (command);

	ioc = g_io_channel_unix_new (fd);
	g_io_add_watch_full (ioc, 0, G_IO_IN | G_IO_HUP, handle_input,
			     table, ioc_destroy);
	g_io_channel_unref (ioc);
	druid_set_sensitive (FALSE, FALSE, TRUE);
		
}
