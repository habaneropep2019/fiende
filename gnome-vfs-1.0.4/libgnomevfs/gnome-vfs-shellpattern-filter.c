/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-shellpattern-filter.c - fnmatch()-based filter for the
   GNOME Virtual File System.

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

   Author: Ettore Perazzoli <ettore@comm2000.it> */

#include <config.h>
#include "gnome-vfs-shellpattern-filter.h"

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

#ifdef _POSIX_SOURCE
#include <fnmatch.h>
#else
/* This enables the `FNM_CASEFOLD' #define on GNU libc. */
/* FIXME bugzilla.eazel.com 1195: OK to use a GNU extension? */
#define _POSIX_SOURCE
#include <fnmatch.h>
/* On Solaris 2.7 we need to undef this again.  */
#undef _POSIX_SOURCE
#endif


struct GnomeVFSShellpatternFilter {
	gchar *pattern;
	gint fnmatch_flags;
};


GnomeVFSShellpatternFilter *
gnome_vfs_shellpattern_filter_new (const gchar *pattern,
				   GnomeVFSDirectoryFilterOptions options)
{
	GnomeVFSShellpatternFilter *new;

	new = g_new (GnomeVFSShellpatternFilter, 1);

	new->fnmatch_flags = 0;
	/* FIXME bugzilla.eazel.com 1195: OK to use a GNU extension? */
	if (options & GNOME_VFS_DIRECTORY_FILTER_IGNORECASE)
		new->fnmatch_flags |= FNM_CASEFOLD;

	/* FIXME bugzilla.eazel.com 1196: What about `\' quoting and other fnmatch options?  */

	new->pattern = g_strdup (pattern);
	
	return new;
}

void
gnome_vfs_shellpattern_filter_destroy (GnomeVFSShellpatternFilter *filter)
{
	g_return_if_fail (filter != NULL);

	g_free (filter->pattern);
	g_free (filter);
}

gboolean
gnome_vfs_shellpattern_filter_apply (GnomeVFSShellpatternFilter *filter,
				     GnomeVFSFileInfo *info)
{
	g_return_val_if_fail (filter != NULL, FALSE);
	g_return_val_if_fail (info != NULL, FALSE);

	if (fnmatch (filter->pattern, info->name, filter->fnmatch_flags) != 0)
		return FALSE;
	else
		return TRUE;
}
