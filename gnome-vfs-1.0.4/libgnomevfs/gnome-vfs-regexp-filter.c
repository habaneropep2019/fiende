/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-regexp-filter.c - Regexp-based filter for the GNOME
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

   Author: Ettore Perazzoli <ettore@comm2000.it> */

#include <config.h>
#include "gnome-vfs-regexp-filter.h"

#include "gnome-vfs-private.h"
#include "gnome-vfs.h"
#include <regex.h>
#include <sys/types.h>

struct GnomeVFSRegexpFilter {
	regex_t regex;
};

GnomeVFSRegexpFilter *
gnome_vfs_regexp_filter_new (const gchar *regexp,
			     GnomeVFSDirectoryFilterOptions options)
{
	GnomeVFSRegexpFilter *new;
	gint regflags;

	new = g_new (GnomeVFSRegexpFilter, 1);

	regflags = REG_NOSUB;
	if (options & GNOME_VFS_DIRECTORY_FILTER_IGNORECASE)
		regflags |= REG_ICASE;
	if (options & GNOME_VFS_DIRECTORY_FILTER_EXTENDEDREGEXP)
		regflags |= REG_EXTENDED;

	if (regcomp (&new->regex, regexp, regflags) != 0) {
		g_free (new);
		return NULL;
	}

	return new;
}

void
gnome_vfs_regexp_filter_destroy (GnomeVFSRegexpFilter *filter)
{
	g_return_if_fail (filter != NULL);

	regfree (&filter->regex);
	g_free (filter);
}

gboolean
gnome_vfs_regexp_filter_apply (GnomeVFSRegexpFilter *filter,
			       GnomeVFSFileInfo *info)
{
	gint result;

	result = regexec (&filter->regex, info->name, 0, NULL, 0);

	if (result == 0)
		return TRUE;
	else
		return FALSE;
}
