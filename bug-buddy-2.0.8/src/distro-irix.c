/* bug-buddy bug submitting program
 *
 * Copyright 2000 Ximian, Inc.
 *
 * Author:  David Kaelbling <drk@sgi.com>
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

static char *get_irix_version (Distribution *distro);
static void get_package_versions (GSList *packages);

Phylum irix_phy = { 
	get_irix_version,
	get_package_versions 
};

static char *
get_irix_version (Distribution *distro)
{
	g_return_val_if_fail (distro, NULL);
	g_return_val_if_fail (distro->version_file, NULL);

	return get_line_from_command ("/usr/bin/echo -n SGI `/sbin/uname -Rs | /bin/cut -d' ' -f1,3-`; "
				      "versions -b fw_common | awk 'NR >= 4 { print \", Freeware\",$3 }'");
}


static void
get_version_from_versions (gpointer data, gpointer udata)
{
	char *command;
	Package *package = data;

	if (package->version || !package->rpm)
		return;

	command = g_strdup_printf ("versions -b fw_%s | awk 'NR >= 4 { print $4 }'",
				   package->rpm);
	package->version = get_line_from_command (command);
	g_free (command);
}

static void
get_package_versions (GSList *packages)
{
	g_return_if_fail (packages);

	g_slist_foreach (packages, get_version_from_versions, NULL);
	append_packages ();
}
