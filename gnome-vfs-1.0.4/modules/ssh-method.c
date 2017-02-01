/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* ssh-method.c - VFS Access to the GConf configuration database.

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

   Author: Ian McKellar <yakk@yakk.net> */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <string.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include "gnome-vfs-mime.h"

#include "gnome-vfs-cancellation.h"
#include "gnome-vfs-context.h"
#include "gnome-vfs-module.h"
#include "gnome-vfs-method.h"
#include "gnome-vfs-utils.h"
#include "gnome-vfs-module-shared.h"
#include "gnome-vfs-parse-ls.h"

#include "file-method.h"

typedef struct {
	GnomeVFSMethodHandle method_handle;
	GnomeVFSURI *uri;
	enum {
		SSH_FILE,
		SSH_LIST
	} type;
	GnomeVFSOpenMode open_mode;
	int read_fd;
	int write_fd;
	pid_t pid;
} SshHandle;

static GnomeVFSResult do_open           (GnomeVFSMethod *method,
				         GnomeVFSMethodHandle **method_handle,
				         GnomeVFSURI *uri,
				         GnomeVFSOpenMode mode,
				         GnomeVFSContext *context);
static GnomeVFSResult do_create         (GnomeVFSMethod *method,
				         GnomeVFSMethodHandle **method_handle,
				         GnomeVFSURI *uri,
				         GnomeVFSOpenMode mode,
				         gboolean exclusive,
				         guint perm,
				         GnomeVFSContext *context);
static GnomeVFSResult do_close          (GnomeVFSMethod *method,
				         GnomeVFSMethodHandle *method_handle,
				         GnomeVFSContext *context);
static GnomeVFSResult do_read		(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 gpointer buffer,
					 GnomeVFSFileSize num_bytes,
					 GnomeVFSFileSize *bytes_read,
					 GnomeVFSContext *context);
static GnomeVFSResult do_write          (GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
				         gconstpointer buffer,
				         GnomeVFSFileSize num_bytes,
				         GnomeVFSFileSize *bytes_written,
					 GnomeVFSContext *context);
static GnomeVFSResult do_open_directory (GnomeVFSMethod *method,
					 GnomeVFSMethodHandle **method_handle,
					 GnomeVFSURI *uri,
					 GnomeVFSFileInfoOptions options,
					 const GnomeVFSDirectoryFilter *filter,
					 GnomeVFSContext *context);
static GnomeVFSResult do_close_directory(GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSContext *context);
static GnomeVFSResult do_read_directory (GnomeVFSMethod *method,
					 GnomeVFSMethodHandle *method_handle,
					 GnomeVFSFileInfo *file_info,
					 GnomeVFSContext *context);
static GnomeVFSResult do_get_file_info  (GnomeVFSMethod *method,
					 GnomeVFSURI *uri,
					 GnomeVFSFileInfo *file_info,
					 GnomeVFSFileInfoOptions options,
					 GnomeVFSContext *context);
#if 0
static GnomeVFSResult do_get_file_info_from_handle
                                        (GnomeVFSMethodHandle *method_handle,
					 GnomeVFSFileInfo *file_info,
					 GnomeVFSFileInfoOptions options);
#endif
static gboolean       do_is_local       (GnomeVFSMethod *method,
					 const GnomeVFSURI *uri);

static GnomeVFSMethod method = {
	sizeof (GnomeVFSMethod),
        do_open,
        do_create, /* create */
        do_close,
        do_read, /* read */
        do_write, /* write */
        NULL, /* seek */
        NULL, /* tell */
        NULL, /* truncate */
        do_open_directory,
	do_close_directory,
        do_read_directory,
        do_get_file_info,
	NULL, /* get_file_info_from_handle */
        do_is_local,
	NULL, /* make directory */
        NULL, /* remove directory */
	NULL, /* unlink */
	NULL, /* check_same_fs */
	NULL, /* set_file_info */
	NULL, /* truncate */
	NULL, /* find_directory */
	NULL /* create_symbolic_link */
};

/* FIXME: does this like FDs? */
static GnomeVFSResult
ssh_connect (SshHandle **handle_return,
	     GnomeVFSURI *uri, const char *command)
{
	int in[2];
	int out[2];
	pid_t pid;
	SshHandle *handle;

	if ( pipe (in) == -1 || 
	     pipe (out) == -1 ) {
		/* bugger */
		return gnome_vfs_result_from_errno ();
	}

	pid = fork ();

	if (pid == 0) {
		/* child */

		if ( dup2 (in[0], 0) == -1 ||
		     dup2 (out[1], 1) == -1 ) {
			/* bugger */
			_exit (errno); /* can we get the error back to the parent? */
		}

		/* fixme: handle other ports */
		execlp ("ssh", "ssh", "-oBatchmode yes", "-x", "-l", 
				gnome_vfs_uri_get_user_name (uri),
				gnome_vfs_uri_get_host_name (uri),
				command, NULL);

		/* we shouldn't get here */

		_exit (errno); /* can we get the error back to the parent? */
	} else if (pid == -1) {
		/* bugger */
		return gnome_vfs_result_from_errno ();
	} else {
		/* parent */
		/*
		waitpid (pid, &status, 0);
		status = WEXITSTATUS (status);

		if (status != 0) {
			return gnome_vfs_result_from_errno ();
		}*/

	}

	handle = g_new0 (SshHandle, 1);
	handle->uri = uri;
	handle->read_fd = out[0];
	handle->write_fd = in[1];
	handle->pid = pid;

	gnome_vfs_uri_ref (handle->uri);

	*handle_return = handle;

	return GNOME_VFS_OK;
}

static GnomeVFSResult
ssh_destroy (SshHandle *handle)
{
	close (handle->read_fd);
	close (handle->write_fd);
	gnome_vfs_uri_unref (handle->uri);
	g_free (handle);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
ssh_read (SshHandle *handle,
	   gpointer buffer,
	   GnomeVFSFileSize num_bytes,
	   GnomeVFSFileSize *bytes_read)
{
	GnomeVFSFileSize my_read;
	my_read = (GnomeVFSFileSize) read (handle->read_fd, buffer, 
			(size_t) num_bytes);

	if (my_read == -1) {
		return gnome_vfs_result_from_errno ();
	}

	*bytes_read = my_read;

	return GNOME_VFS_OK;
}

static GnomeVFSResult
ssh_write (SshHandle *handle,
	   gconstpointer buffer,
	   GnomeVFSFileSize num_bytes,
	   GnomeVFSFileSize *bytes_written)
{
	GnomeVFSFileSize written;
	written = (GnomeVFSFileSize) write (handle->write_fd, buffer, 
			(size_t) num_bytes);

	if (written == -1) {
		return gnome_vfs_result_from_errno ();
	}

	*bytes_written = written;

	return GNOME_VFS_OK;
}

#if 0
static char *ssh_escape (const char *string)
{
	char *new_str;
	int i,j;
	
	new_str = g_malloc0 (strlen(string)*2+3);

	new_str[0]='\'';

	for (i=0,j=1; string[i] != '\0'; i++, j++) {
		if (string[i] == '\'') {
			new_str[j] = '\\';
			j++;
		}
		new_str[j] = string[i];
	}

	return new_str;
}

static GnomeVFSResult
ssh_send (SshHandle *handle,
	  const char *string)
{
	GnomeVFSFileSize len, written;
	GnomeVFSResult result = GNOME_VFS_OK;
	
	len = strlen (string);

	while (len > 0 && result == GNOME_VFS_OK) {
		result = ssh_write (handle, string, len, &written);
		len -= written;
		string += written;
	}

	return result;
}
#endif

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{
	GnomeVFSResult result = GNOME_VFS_OK;
	char *cmd;
	SshHandle *handle = NULL;

	if (mode == GNOME_VFS_OPEN_READ) {
		/* FIXME: escape for shell */
		cmd = g_strdup_printf ("cat '%s'\n", uri->text);
		result = ssh_connect (&handle, uri, cmd);
		g_free (cmd);

		if (result != GNOME_VFS_OK) {
			return result;
		}

	} else if (mode == GNOME_VFS_OPEN_WRITE) {
		return GNOME_VFS_ERROR_INVALID_OPEN_MODE;
	} else {
		return GNOME_VFS_ERROR_INVALID_OPEN_MODE;
	}
	
	handle->open_mode = mode;
	handle->type = SSH_FILE;
	*method_handle = (GnomeVFSMethodHandle *)handle;

	return result;
}

static GnomeVFSResult   
do_create (GnomeVFSMethod *method,
	   GnomeVFSMethodHandle **method_handle,
	   GnomeVFSURI *uri,
	   GnomeVFSOpenMode mode,
	   gboolean exclusive,
	   guint perm,
	   GnomeVFSContext *context)
{
	SshHandle *handle = NULL;
	char *cmd;
	GnomeVFSResult result;

	if (mode != GNOME_VFS_OPEN_WRITE) {
		return GNOME_VFS_ERROR_INVALID_OPEN_MODE;
	}


	/* FIXME: escape for shell */
	cmd = g_strdup_printf ("cat > '%s'\n", uri->text);
	result = ssh_connect (&handle, uri, cmd);
	g_free (cmd);

	if (result != GNOME_VFS_OK) {
		return result;
	}

	/* FIXME: set perm */

	handle->open_mode = mode;
	handle->type = SSH_FILE;
	*method_handle = (GnomeVFSMethodHandle *)handle;

	return result;
}

static GnomeVFSResult   
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)
{
	return ssh_destroy ((SshHandle *)method_handle);
}

static GnomeVFSResult
do_read (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read,
	 GnomeVFSContext *context)
{
	return ssh_read ((SshHandle *)method_handle, buffer, num_bytes,
			bytes_read);
}

/* alternative impl:
 * dd bs=1 conv=notrunc count=5 seek=60 of=/tmp/foo-test
 */
static GnomeVFSResult   
do_write (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_written,
	  GnomeVFSContext *context)
{
	return ssh_write ((SshHandle *)method_handle, buffer, num_bytes,
			bytes_written);
}

static GnomeVFSResult 
do_open_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle **method_handle,
                   GnomeVFSURI *uri,
                   GnomeVFSFileInfoOptions options,
                   const GnomeVFSDirectoryFilter *filter,
		   GnomeVFSContext *context)
{
	SshHandle *handle = NULL;
	char *cmd = NULL;
	GnomeVFSResult result;

	/* FIXME: escape for shell */
	cmd = g_strdup_printf ("ls -l '%s'", uri->text);
	result = ssh_connect (&handle, uri, cmd);
	g_free (cmd);

	if (result != GNOME_VFS_OK) {
		return result;
	}

	handle->open_mode = GNOME_VFS_OPEN_NONE;
	handle->type = SSH_LIST;
	*method_handle = (GnomeVFSMethodHandle *)handle;

	return result;
	return GNOME_VFS_OK;
}

static GnomeVFSResult 
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context)
{
	return ssh_destroy ((SshHandle *)method_handle);
}

#define LINE_LENGTH 4096 /* max line length we'll grok */

static GnomeVFSResult 
do_read_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle *method_handle,
                   GnomeVFSFileInfo *file_info,
		   GnomeVFSContext *context)
{
	GnomeVFSResult result = GNOME_VFS_OK;
	char line[LINE_LENGTH];
	char c;
	int i=0;
	GnomeVFSFileSize j;
	struct stat st;
	char *filename, *linkname;

	for (;;) {
		filename = NULL;
		linkname = NULL;
		i = 0;
		j = 0;
		while (i<LINE_LENGTH) {
			result = ssh_read ((SshHandle *)method_handle, &c, 1, &j);

			if (j == 0 || c == '\r' || c == '\n') {
				break;
			}

			if (result != GNOME_VFS_OK) {
				return result;
			}

			line[i] = c;
			i++;
		}

		line[i] = 0;

		if (i == 0) {
			return GNOME_VFS_ERROR_EOF;
		}

		if (!gnome_vfs_parse_ls_lga (line, &st, &filename, &linkname)) {
			continue; /* skip to next line */
		}

		gnome_vfs_stat_to_file_info (file_info, &st);
		file_info->name = filename;
		if (linkname) {
			file_info->symlink_name = linkname;
		}

		/* FIXME: support symlinks correctly */

		file_info->mime_type = g_strdup 
			(gnome_vfs_get_file_mime_type (filename, &st, FALSE));

		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		file_info->valid_fields &= 
			~GNOME_VFS_FILE_INFO_FIELDS_BLOCK_COUNT;
		file_info->valid_fields &= 
			~GNOME_VFS_FILE_INFO_FIELDS_IO_BLOCK_SIZE;

		break;
	}

	return result;
}

GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
                  GnomeVFSFileInfo *file_info,
                  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context)
{
	SshHandle *handle = NULL;
	char *cmd = NULL;
	GnomeVFSResult result;

	/* FIXME: escape for shell */
	cmd = g_strdup_printf ("ls -ld '%s'", uri->text);
	result = ssh_connect (&handle, uri, cmd);
	g_free (cmd);

	if (result != GNOME_VFS_OK) {
		return result;
	}

	handle->open_mode = GNOME_VFS_OPEN_NONE;
	handle->type = SSH_LIST;

	result = do_read_directory (method, (GnomeVFSMethodHandle *)handle,
			file_info, context);

	ssh_destroy (handle);

	return result;
}

#if 0
static GnomeVFSResult  
do_get_file_info_from_handle (GnomeVFSMethodHandle *method_handle,
			      GnomeVFSFileInfo *file_info,
			      GnomeVFSFileInfoOptions options)
{
	return GNOME_VFS_ERROR_WRONG_FORMAT;	
}
#endif

gboolean 
do_is_local (GnomeVFSMethod *method, const GnomeVFSURI *uri)
{
        return FALSE;
}

GnomeVFSMethod *
vfs_module_init (const char *method_name, const char *args)
{
        return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
}
