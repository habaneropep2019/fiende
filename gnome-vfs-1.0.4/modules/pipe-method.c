/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* pipe-method.c - pipe access method for GNOME Virtual File System

   Copyright (C) 1999, 2000 Red Hat Inc.

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

   Author: Elliot Lee <sopwith@redhat.com> */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "gnome-vfs-mime.h"

#include "gnome-vfs-cancellation.h"
#include "gnome-vfs-context.h"
#include "gnome-vfs-module.h"
#include "gnome-vfs-method.h"
#include "gnome-vfs-utils.h"
#include "gnome-vfs-module-shared.h"

#include "pipe-method.h"

struct _FileHandle {
	GnomeVFSURI *uri;
	FILE *fh;
};
typedef struct _FileHandle FileHandle;

static FileHandle *
file_handle_new (GnomeVFSURI *uri,
		 FILE *fh)
{
	FileHandle *new;

	new = g_new (FileHandle, 1);

	new->uri = gnome_vfs_uri_ref (uri);
	new->fh = fh;

	return new;
}

static void
file_handle_destroy (FileHandle *handle)
{
	pclose(handle->fh);
	gnome_vfs_uri_unref (handle->uri);
	g_free (handle);
}

static char *
str_without_suffix (const char *str)
{
	const char *semicolon;

	semicolon = strchr (str, ';');
	
	return g_strndup (str, (semicolon == NULL) ? strlen (str) : semicolon - str);
}

static char *
mime_from_uri (const gchar *uri)
{
	const char *mime;
	char *result;
	
	mime = uri == NULL ? NULL : strstr (uri, ";mime-type=");
	if (mime != NULL) {
		result = str_without_suffix (mime + 11);
		if (result[0] == '\0') {
			/* No mime-type specified */
			g_free (result);
			result = g_strdup (GNOME_VFS_MIME_TYPE_UNKNOWN);
		}
	} else {
		result = g_strdup (GNOME_VFS_MIME_TYPE_UNKNOWN);
	}
	return result;
}


static void
set_default_file_info (GnomeVFSFileInfo *file_info,
		       GnomeVFSURI *uri)
{
        file_info->name = g_strdup (uri->text);
	file_info->flags = GNOME_VFS_FILE_FLAGS_NONE;
	file_info->type = GNOME_VFS_FILE_TYPE_REGULAR;
	file_info->permissions = (GNOME_VFS_PERM_USER_READ
				  | GNOME_VFS_PERM_GROUP_READ
				  | GNOME_VFS_PERM_OTHER_READ);

	file_info->valid_fields = (GNOME_VFS_FILE_INFO_FIELDS_FLAGS
				   | GNOME_VFS_FILE_INFO_FIELDS_TYPE 
				   | GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS);
}

#define SAFE_POPEN_CHARACTERS "?'/. +:-_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"


static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{
	FileHandle *file_handle;
	FILE *fh;
	char *real_uri;

	_GNOME_VFS_METHOD_PARAM_CHECK (method_handle != NULL);
	_GNOME_VFS_METHOD_PARAM_CHECK (uri != NULL);

	if (!(mode & GNOME_VFS_OPEN_READ))
		return GNOME_VFS_ERROR_INVALID_OPEN_MODE;

	real_uri = str_without_suffix (gnome_vfs_unescape_string (uri->text, ""));

	/* Check that all the characters being passed to popen are "safe" */
	/* If we allow just any characters through, it's easy to make URIs 
	 * that make dangerous command lines here. If we prevent any 
	 * interesting characters, then it's just simple parameters passed 
	 * to the command that's the first thing passed in, which is much 
	 * safer.
	 * Without this "man:ls&shutdown -h now" would have undesired effects.
	 */
	if (strspn (real_uri, SAFE_POPEN_CHARACTERS) != strlen (real_uri)) {
		g_message ("real_uri is %s, has illegal chars, failing", real_uri);
		g_free (real_uri);
		return GNOME_VFS_ERROR_NOT_PERMITTED;
	}

	fh = popen (real_uri, "r");
	g_free (real_uri);

	if (!fh)
		return gnome_vfs_result_from_errno ();

	file_handle = file_handle_new (uri, fh);
	
	*method_handle = (GnomeVFSMethodHandle *) file_handle;

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)
{
	FileHandle *file_handle;

	g_return_val_if_fail (method_handle != NULL, GNOME_VFS_ERROR_INTERNAL);

	file_handle = (FileHandle *) method_handle;

	file_handle_destroy (file_handle);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_read (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read,
	 GnomeVFSContext *context)
{
	FileHandle *file_handle;
	gint read_val;

	g_return_val_if_fail (method_handle != NULL, GNOME_VFS_ERROR_INTERNAL);

	file_handle = (FileHandle *) method_handle;

	read_val = fread (buffer, 1, num_bytes, file_handle->fh);

	if (read_val <= 0) {
		*bytes_read = 0;
		return GNOME_VFS_ERROR_EOF;
	} else {
		*bytes_read = read_val;
		return GNOME_VFS_OK;
	}
}

static GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  GnomeVFSFileInfo *file_info,
		  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context)
{
        set_default_file_info (file_info, uri);

	file_info->mime_type = mime_from_uri (gnome_vfs_unescape_string (uri->text, ""));
	file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

        return GNOME_VFS_OK;
}

static GnomeVFSResult
do_get_file_info_from_handle (GnomeVFSMethod *method,
			      GnomeVFSMethodHandle *method_handle,
			      GnomeVFSFileInfo *file_info,
			      GnomeVFSFileInfoOptions options,
			      GnomeVFSContext *context)
{
        FileHandle *handle;

	handle = (FileHandle *) method_handle;

        set_default_file_info (file_info, handle->uri);

	file_info->mime_type = mime_from_uri (gnome_vfs_unescape_string (handle->uri->text, ""));
	file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

	return GNOME_VFS_OK;
}

static gboolean
is_local(GnomeVFSMethod *method,
	 const GnomeVFSURI *uri)
{
	return TRUE;
}

static GnomeVFSMethod method = {
	sizeof (GnomeVFSMethod),
	do_open,
	NULL, /* create */
	do_close,
	do_read,
	NULL, /* write */
	NULL, /* seek */
	NULL, /* tell */
	NULL, /* truncate_handle */
	NULL, /* open_directory */
	NULL, /* close_directory */
	NULL, /* read_directory */
	do_get_file_info,
	do_get_file_info_from_handle,
	is_local,
	NULL, /* make_directory */
	NULL, /* remove_directory */
	NULL, /* move */
	NULL, /* unlink */
	NULL, /* check_same_fs */
	NULL, /* set_file_info */
	NULL, /* truncate */
	NULL, /* find_directory */
	NULL  /* create_symbolic_link */
};

GnomeVFSMethod *
vfs_module_init (const char *method_name, const char *args)
{
	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
}
