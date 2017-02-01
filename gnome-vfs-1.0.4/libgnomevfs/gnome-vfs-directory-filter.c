/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-directory-filter.c - Directory filter for the GNOME
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
#include "gnome-vfs-directory-filter.h"

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"
#include <string.h>


struct GnomeVFSDirectoryFilter {
	/* Filter type.  */
	GnomeVFSDirectoryFilterType type;

	/* Filter options.  */
	GnomeVFSDirectoryFilterOptions options;

	/* The pattern.  */
	gchar *pattern;

	/* Filter function (only with
           `gnome_vfs_directory_filter_new_custom()'.  If it returns
           `FALSE', the item must be discarded, otherwise it must be
           accepted.  */
	GnomeVFSDirectoryFilterFunc func;

	/* Data to pass to the filter function.  */
	gpointer data;

	/* Bit array specifying what kind of information the filter
           needs to select files.  */
	GnomeVFSDirectoryFilterNeeds needs;
};


static gboolean
common_filter (const GnomeVFSDirectoryFilter *filter,
	       GnomeVFSFileInfo *info)
{
	GnomeVFSDirectoryFilterOptions options;

	options = filter->options;

	if (info->type != GNOME_VFS_FILE_TYPE_DIRECTORY) {
		if (options & GNOME_VFS_DIRECTORY_FILTER_DIRSONLY)
			return FALSE;
	} else {
		if (options & GNOME_VFS_DIRECTORY_FILTER_NODIRS)
			return FALSE;
	}

	if (info->name[0] == '.') {
		if (options & GNOME_VFS_DIRECTORY_FILTER_NODOTFILES) {
			return FALSE;
		}

	    if ((options & GNOME_VFS_DIRECTORY_FILTER_NOSELFDIR)
		&& info->name[1] == 0)
		    return FALSE;

	    if ((options & GNOME_VFS_DIRECTORY_FILTER_NOPARENTDIR)
		&& info->name[1] == '.' && info->name[2] == 0)
		    return FALSE;
	}

	if (info->name[strlen (info->name) - 1] == '~') {
		if (options & GNOME_VFS_DIRECTORY_FILTER_NOBACKUPFILES) {
			return FALSE;
		}
	}
	return TRUE;
}


/**
 * gnome_vfs_directory_filter_new:
 * @type: Filter type
 * @options: Options for the filter
 * @pattern: Pattern for name-based selection
 * 
 * Create a new directory filter.
 * 
 * Return value: A pointer to the newly created filter.
 **/
GnomeVFSDirectoryFilter *
gnome_vfs_directory_filter_new (GnomeVFSDirectoryFilterType type,
				GnomeVFSDirectoryFilterOptions options,
				const gchar *pattern)
{
	GnomeVFSDirectoryFilter *new;

	if (type == GNOME_VFS_DIRECTORY_FILTER_NONE && options == 0)
		return NULL;

	new = g_new (GnomeVFSDirectoryFilter, 1);

	new->type = type;
	new->options = options;

	if (pattern != NULL)
		new->pattern = g_strdup (pattern);
	else
		new->pattern = NULL;

	new->func = NULL;

	switch (type) {
	case GNOME_VFS_DIRECTORY_FILTER_REGEXP:
		new->data = gnome_vfs_regexp_filter_new (pattern,
							 options);
		break;
	case GNOME_VFS_DIRECTORY_FILTER_SHELLPATTERN:
		new->data = gnome_vfs_shellpattern_filter_new (pattern,
							       options);
		break;
	default:
		break;
	}

	new->needs = GNOME_VFS_DIRECTORY_FILTER_NEEDS_NAME;
	if (new->options & GNOME_VFS_DIRECTORY_FILTER_NODIRS)
		new->needs |= GNOME_VFS_DIRECTORY_FILTER_NEEDS_TYPE;

	return new;
}

/**
 * gnome_vfs_directory_filter_new_custom:
 * @func: Function to call to evaluate whether a file must pass the filter or
 * not
 * @needs: Bitmask representing the information that @func needs to decide
 * if a file passes through the filter or not.
 * @func_data: Additional data to be passed to @func
 * 
 * Create a new custom directory filter.  Whenever the filter is applied, @func
 * will be called with info about the file to be filtered and @func_data as
 * parameters.  If @func returns %TRUE, the file passes the filter; if it
 * returns %FALSE, it does not.
 * 
 * Return value: A pointer to the newly created filter.
 **/
GnomeVFSDirectoryFilter *
gnome_vfs_directory_filter_new_custom (GnomeVFSDirectoryFilterFunc func,
				       GnomeVFSDirectoryFilterNeeds needs,
				       gpointer func_data)
{
	GnomeVFSDirectoryFilter *new;

	new = g_new (GnomeVFSDirectoryFilter, 1);

	new->type = GNOME_VFS_DIRECTORY_FILTER_NONE;
	new->options = GNOME_VFS_DIRECTORY_FILTER_DEFAULT;
	new->pattern = NULL;

	new->func = func;
	new->data = func_data;
	new->needs = needs;

	return new;
}

/**
 * gnome_vfs_directory_filter_destroy:
 * @filter: A directory filter
 * 
 * Destroy @filter.
 **/
void
gnome_vfs_directory_filter_destroy (GnomeVFSDirectoryFilter *filter)
{
	if (filter == NULL)
		return;

	switch (filter->type) {
	case GNOME_VFS_DIRECTORY_FILTER_REGEXP:
		gnome_vfs_regexp_filter_destroy
			((GnomeVFSRegexpFilter *) filter->data);
		break;
	case GNOME_VFS_DIRECTORY_FILTER_SHELLPATTERN:
		gnome_vfs_shellpattern_filter_destroy
			((GnomeVFSShellpatternFilter *) filter->data);
		break;
	default:
		break;
	}

	g_free (filter->pattern);
	g_free (filter);
}

/**
 * gnome_vfs_directory_filter_apply:
 * @filter: A directory filter
 * @info: Information about the file to be filtered
 * 
 * Apply @filter to the file whose info is @info.
 * 
 * Return value: TRUE if the file passes through the filter, FALSE otherwise.
 **/
gboolean
gnome_vfs_directory_filter_apply (const GnomeVFSDirectoryFilter *filter,
				  GnomeVFSFileInfo *info)
{
	g_return_val_if_fail (info != NULL, FALSE);

	if (filter == NULL)
		return TRUE;

	if (filter->func != NULL)
		return filter->func (info, filter->data);

	if (! common_filter (filter, info))
		return FALSE;

	switch (filter->type) {
	case GNOME_VFS_DIRECTORY_FILTER_REGEXP:
		return gnome_vfs_regexp_filter_apply
			((GnomeVFSRegexpFilter *) filter->data, info);
	case GNOME_VFS_DIRECTORY_FILTER_SHELLPATTERN:
		return gnome_vfs_shellpattern_filter_apply
			((GnomeVFSShellpatternFilter *) filter->data, info);
	default:
		break;
	}

	return TRUE;
}

/**
 * gnome_vfs_directory_filter_get_needs:
 * @filter: A directory filter
 *
 * Check what kind of information the filter needs to select files.
 * 
 * Return value: A bitmask representing the information needed by the filter to
 * perform selection.
 **/
GnomeVFSDirectoryFilterNeeds
gnome_vfs_directory_filter_get_needs (const GnomeVFSDirectoryFilter *filter)
{
	g_return_val_if_fail (filter != NULL,
			      GNOME_VFS_DIRECTORY_FILTER_NEEDS_NOTHING);

	return filter->needs;
}
