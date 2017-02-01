/* -*- Mode: C; tab-width: 8; indent-tabs-mode: 8; c-basic-offset: 8 -*- */

/* desktop-method.c

   Copyright (C) 2001 Red Hat, Inc.

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
*/

/* URI scheme for remapping directories under magic URIs, used
 * for the magic desktop file directories such as start-here.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <libgnome/libgnome.h>

#include <libgnomevfs/gnome-vfs-mime.h>

#include <libgnomevfs/gnome-vfs-module.h>
#include <libgnomevfs/gnome-vfs-method.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libgnomevfs/gnome-vfs-module-shared.h>

/* FIXME Maybe when chaining to file:, we should call the gnome-vfs wrapper
 * functions, instead of the file: methods directly.
 */

#define N_ELEMENTS(arr)		(sizeof (arr) / sizeof ((arr)[0]))

static GnomeVFSURI* desktop_uri_to_file_uri (GnomeVFSURI *desktop_uri);

static GnomeVFSResult open_merged_directory (GnomeVFSMethod *method,
					     GnomeVFSMethodHandle **method_handle,
					     GnomeVFSURI *uri,
					     GnomeVFSFileInfoOptions options,
					     const GnomeVFSDirectoryFilter *filter,
					     GnomeVFSContext *context);

static char*         create_file_uri_in_dir (const char  *dir,
					     const char  *filename);

static GnomeVFSMethod *parent_method = NULL;

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{
	GnomeVFSURI *file_uri;
	GnomeVFSResult result;

	file_uri = desktop_uri_to_file_uri (uri);
	result = (* parent_method->open) (parent_method,
					  method_handle,
					  file_uri,
					  mode,
					  context);
	gnome_vfs_uri_unref (file_uri);

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
	GnomeVFSURI *file_uri;
	GnomeVFSResult result;

	file_uri = desktop_uri_to_file_uri (uri);
	result = (* parent_method->create) (parent_method,
					    method_handle,
					    file_uri,
					    mode,
					    exclusive,
					    perm,
					    context);
	gnome_vfs_uri_unref (file_uri);

	return result;
}

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)
{
	GnomeVFSResult result;
	
	result = (* parent_method->close) (parent_method,
					   method_handle,
					   context);

	return result;
}

static GnomeVFSResult
do_read (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer buffer,
	 GnomeVFSFileSize num_bytes,
	 GnomeVFSFileSize *bytes_read,
	 GnomeVFSContext *context)
{
	GnomeVFSResult result;
	
	result = (* parent_method->read) (parent_method,
					  method_handle,
					  buffer, num_bytes,
					  bytes_read,
					  context);

	return result;
}

static GnomeVFSResult
do_write (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_written,
	  GnomeVFSContext *context)
{
	GnomeVFSResult result;
	
	result = (* parent_method->write) (parent_method,
					   method_handle,
					   buffer, num_bytes,
					   bytes_written,
					   context);

	return result;
}


static GnomeVFSResult
do_seek (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSSeekPosition whence,
	 GnomeVFSFileOffset offset,
	 GnomeVFSContext *context)
{
	GnomeVFSResult result;
	
	result = (* parent_method->seek) (parent_method,
					  method_handle,
					  whence, offset,
					  context);

	return result;
}

static GnomeVFSResult
do_tell (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSFileOffset *offset_return)
{
	GnomeVFSResult result;
	
	result = (* parent_method->tell) (parent_method,
					  method_handle,
					  offset_return);

	return result;
}


static GnomeVFSResult
do_truncate_handle (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSFileSize where,
		    GnomeVFSContext *context)
{
	GnomeVFSResult result;
	
	result = (* parent_method->truncate_handle) (parent_method,
						     method_handle,
						     where,
						     context);

	return result;
}

static GnomeVFSResult
do_truncate (GnomeVFSMethod *method,
	     GnomeVFSURI *uri,
	     GnomeVFSFileSize where,
	     GnomeVFSContext *context)
{
	GnomeVFSURI *file_uri;
	GnomeVFSResult result;

	file_uri = desktop_uri_to_file_uri (uri);
	result = (* parent_method->truncate) (parent_method,
					      file_uri,
					      where,
					      context);

	gnome_vfs_uri_unref (file_uri);

	return result;
}

typedef struct _DirHandle DirHandle;

struct _DirHandle
{
	GSList *next;
	GSList *handles;
};

static GnomeVFSResult
do_open_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle **method_handle,
		   GnomeVFSURI *uri,
		   GnomeVFSFileInfoOptions options,
		   const GnomeVFSDirectoryFilter *filter,
		   GnomeVFSContext *context)
{
	return open_merged_directory (method, method_handle,
				      uri, options, filter, context);
}

static GnomeVFSResult
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context)
{
	GnomeVFSResult result;
	GSList *tmp;
	DirHandle *dh;

	dh = (DirHandle*) method_handle;

	result = GNOME_VFS_OK;
	tmp = dh->handles;
	while (tmp != NULL) {
		GnomeVFSResult this_result;
		
		this_result = (* parent_method->close_directory) (parent_method,
								  tmp->data,
								  context);

		if (this_result != GNOME_VFS_OK)
			result = this_result;
		
		tmp = tmp->next;
	}

	g_slist_free (dh->handles);
	g_free (dh);
	
	return result;
}

static GnomeVFSResult
do_read_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle *method_handle,
		   GnomeVFSFileInfo *file_info,
		   GnomeVFSContext *context)
{
	GnomeVFSResult result;
	GnomeVFSMethodHandle *parent_handle;
	DirHandle *dh;

	dh = (DirHandle*) method_handle;

	if (dh->next == NULL) {
		return GNOME_VFS_ERROR_EOF;
	}

 next:
	parent_handle = dh->next->data;
	
	result = (* parent_method->read_directory) (parent_method,
						    parent_handle,
						    file_info,
						    context);

	if (result != GNOME_VFS_OK) {
		dh->next = dh->next->next;
		if (dh->next)
			goto next;
		else
			return result;
	} else {
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
	GnomeVFSURI *file_uri;
	GnomeVFSResult result;

	file_uri = desktop_uri_to_file_uri (uri);
	result = (* parent_method->get_file_info) (parent_method,
						   file_uri,
						   file_info,
						   options,
						   context);
	
	gnome_vfs_uri_unref (file_uri);

	return result;
}

static GnomeVFSResult
do_get_file_info_from_handle (GnomeVFSMethod *method,
			      GnomeVFSMethodHandle *method_handle,
			      GnomeVFSFileInfo *file_info,
			      GnomeVFSFileInfoOptions options,
			      GnomeVFSContext *context)
{
	GnomeVFSResult result;

	result = (* parent_method->get_file_info_from_handle) (parent_method,
							       method_handle,
							       file_info,
							       options,
							       context);

	return result;
}


static gboolean
do_is_local (GnomeVFSMethod *method,
	     const GnomeVFSURI *uri)
{
	return TRUE;
}


static GnomeVFSResult
do_make_directory (GnomeVFSMethod *method,
		   GnomeVFSURI *uri,
		   guint perm,
		   GnomeVFSContext *context)
{
	GnomeVFSURI *file_uri;
	GnomeVFSResult result;

	file_uri = desktop_uri_to_file_uri (uri);
	result = (* parent_method->make_directory) (parent_method,
						    file_uri,
						    perm,
						    context);
	
	gnome_vfs_uri_unref (file_uri);

	return result;
}

static GnomeVFSResult
do_remove_directory (GnomeVFSMethod *method,
		     GnomeVFSURI *uri,
		     GnomeVFSContext *context)
{
	GnomeVFSURI *file_uri;
	GnomeVFSResult result;

	file_uri = desktop_uri_to_file_uri (uri);
	result = (* parent_method->remove_directory) (parent_method,
						      file_uri,
						      context);
	
	gnome_vfs_uri_unref (file_uri);

	return result;
}

static GnomeVFSResult
do_find_directory (GnomeVFSMethod *method,
		   GnomeVFSURI *near_uri,
		   GnomeVFSFindDirectoryKind kind,
		   GnomeVFSURI **result_uri,
		   gboolean create_if_needed,
		   gboolean find_if_needed,
		   guint permissions,
		   GnomeVFSContext *context)
{
	GnomeVFSURI *file_uri;
	GnomeVFSURI *file_result_uri;
	GnomeVFSResult result;

	file_result_uri = NULL;
	file_uri = desktop_uri_to_file_uri (near_uri);
	result = (* parent_method->find_directory) (parent_method,
						    file_uri,
						    kind,
						    &file_result_uri,
						    create_if_needed,
						    find_if_needed,
						    permissions,
						    context);
	
	gnome_vfs_uri_unref (file_uri);

	if (result_uri)
		*result_uri = file_result_uri;
	
	if (file_result_uri == NULL)
		result = GNOME_VFS_ERROR_NOT_FOUND;
		
	return result;
}

static GnomeVFSResult
do_move (GnomeVFSMethod *method,
	 GnomeVFSURI *old_uri,
	 GnomeVFSURI *new_uri,
	 gboolean force_replace,
	 GnomeVFSContext *context)
{
	GnomeVFSURI *old_file_uri;
	GnomeVFSURI *new_file_uri;
	GnomeVFSResult result;

	old_file_uri = desktop_uri_to_file_uri (old_uri);
	new_file_uri = desktop_uri_to_file_uri (new_uri);

	result = (* parent_method->move) (parent_method,
					  old_file_uri,
					  new_file_uri,
					  force_replace,
					  context);
	gnome_vfs_uri_unref (old_file_uri);
	gnome_vfs_uri_unref (new_file_uri);

	return result;
}

static GnomeVFSResult
do_unlink (GnomeVFSMethod *method,
	   GnomeVFSURI *uri,
	   GnomeVFSContext *context)
{
	GnomeVFSURI *file_uri;
	GnomeVFSResult result;

	file_uri = desktop_uri_to_file_uri (uri);
	result = (* parent_method->unlink) (parent_method,
					    file_uri,
					    context);
	
	gnome_vfs_uri_unref (file_uri);

	return result;	
}

static GnomeVFSResult
do_create_symbolic_link (GnomeVFSMethod *method,
			 GnomeVFSURI *uri,
			 const char *target_reference,
			 GnomeVFSContext *context)
{
	GnomeVFSURI *file_uri;
	GnomeVFSResult result;

	file_uri = desktop_uri_to_file_uri (uri);
	result = (* parent_method->create_symbolic_link) (parent_method,
							  file_uri,
							  target_reference,
							  context);
	
	gnome_vfs_uri_unref (file_uri);

	return result;	
}

static GnomeVFSResult
do_check_same_fs (GnomeVFSMethod *method,
		  GnomeVFSURI *source_uri,
		  GnomeVFSURI *target_uri,
		  gboolean *same_fs_return,
		  GnomeVFSContext *context)
{
	GnomeVFSURI *source_file_uri;
	GnomeVFSURI *target_file_uri;
	GnomeVFSResult result;

	source_file_uri = desktop_uri_to_file_uri (source_uri);
	target_file_uri = desktop_uri_to_file_uri (target_uri);

	result = (* parent_method->check_same_fs) (parent_method,
						   source_file_uri,
						   target_file_uri,
						   same_fs_return,
						   context);
	gnome_vfs_uri_unref (source_file_uri);
	gnome_vfs_uri_unref (target_file_uri);

	return result;	
}

static GnomeVFSResult
do_set_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  const GnomeVFSFileInfo *info,
		  GnomeVFSSetFileInfoMask mask,
		  GnomeVFSContext *context)
{
	GnomeVFSURI *file_uri;
	GnomeVFSResult result;

	file_uri = desktop_uri_to_file_uri (uri);
	result = (* parent_method->set_file_info) (parent_method,
						   file_uri,
						   info,
						   mask,
						   context);
	
	gnome_vfs_uri_unref (file_uri);

	return result;	
}


/* gnome-vfs bureaucracy */

static GnomeVFSMethod method = {
	sizeof (GnomeVFSMethod),
	do_open,
	do_create,
	do_close,
	do_read,
	do_write,
	do_seek,
	do_tell,
	do_truncate_handle,
	do_open_directory,
	do_close_directory,
	do_read_directory,
	do_get_file_info,
	do_get_file_info_from_handle,
	do_is_local,
	do_make_directory,
	do_remove_directory,
	do_move,
	do_unlink,
	do_check_same_fs,
	do_set_file_info,
	do_truncate,
	do_find_directory,
	do_create_symbolic_link
};


typedef enum
{
	SCHEME_FAVORITES,
	SCHEME_PREFERENCES,
	SCHEME_START_HERE,
	SCHEME_SYSTEM_SETTINGS,
	SCHEME_SERVER_SETTINGS,
	SCHEME_PROGRAMS
} SchemeID;

#define MAX_DIRECTORIES 3
#define DIRECTORIES_INITIALIZER { NULL, NULL, NULL }

typedef struct _SchemeDescription SchemeDescription;

struct _SchemeDescription
{
	SchemeID id;
	
	const char *scheme;

	char *directories[MAX_DIRECTORIES];
};

static SchemeDescription schemes[] = 
{
	{ SCHEME_FAVORITES, "favorites",
	  DIRECTORIES_INITIALIZER },
	{ SCHEME_PREFERENCES, "preferences",
	  DIRECTORIES_INITIALIZER },
	{ SCHEME_START_HERE, "start-here",
	  DIRECTORIES_INITIALIZER },
	{ SCHEME_SYSTEM_SETTINGS, "system-settings",
	  DIRECTORIES_INITIALIZER },
	{ SCHEME_SERVER_SETTINGS, "server-settings",
	  DIRECTORIES_INITIALIZER },
	{ SCHEME_PROGRAMS, "programs",
	  DIRECTORIES_INITIALIZER }
};

GnomeVFSMethod *
vfs_module_init (const char *method_name, 
		 const char *args)
{
	int i;
	
	parent_method = gnome_vfs_method_get ("file");

	if (parent_method == NULL) {
		g_error ("Could not find 'file' method for gnome-vfs");
		return NULL;
	}

	i = 0;
	while (i < N_ELEMENTS (schemes)) {
		switch (schemes[i].id) {
		case SCHEME_FAVORITES:
			schemes[i].directories[0] =
				g_strconcat (g_get_home_dir (),
					     "/.gnome/apps",
					     NULL);
			break;
		case SCHEME_PREFERENCES:
			schemes[i].directories[0] =
				gnome_unconditional_datadir_file ("control-center");
			break;
		case SCHEME_START_HERE:
			schemes[i].directories[0] = g_strconcat (SYSCONFDIR,
								 "/X11/starthere",
								 NULL);
			break;
		case SCHEME_SYSTEM_SETTINGS:
			schemes[i].directories[0] =
				g_strconcat (SYSCONFDIR, "/X11/sysconfig", NULL);
			break;
		case SCHEME_SERVER_SETTINGS:
			schemes[i].directories[0] =
				g_strconcat (SYSCONFDIR, "/X11/serverconfig", NULL);
			break;
		case SCHEME_PROGRAMS:
			schemes[i].directories[0] = g_strconcat (SYSCONFDIR,
								 "/X11/applnk",
								 NULL);
			schemes[i].directories[1] =
				gnome_unconditional_datadir_file ("gnome/apps");
			break;
		default:
			g_assert_not_reached ();
			break;
		}

		++i;
	}
	
	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
	int i;
	
	i = 0;
	while (i < N_ELEMENTS (schemes)) {
		int j;

		j = 0;
		while (j < MAX_DIRECTORIES) {
			g_free (schemes[i].directories[j]);
			schemes[i].directories[j] = NULL;
			++j;
		}
		
		++i;
	}
}



static const SchemeDescription*
get_desc_for_uri (GnomeVFSURI *desktop_uri)
{
	const SchemeDescription *desc;
	int i;
	const char *scheme;
	
	scheme = gnome_vfs_uri_get_scheme (desktop_uri);

	desc = NULL;
	i = 0;
	while (i < N_ELEMENTS (schemes)) {
		if (strcmp (schemes[i].scheme, scheme) == 0) {
			desc = &schemes[i];
			break;
		}
		
		++i;
	}

	return desc;
}

static GnomeVFSURI*
desktop_uri_to_file_uri (GnomeVFSURI *desktop_uri)
{
	const SchemeDescription *desc;
	GnomeVFSURI *new_uri;
	const char *path;
	int i;

	desc = get_desc_for_uri (desktop_uri);

	if (desc == NULL) {
		gnome_vfs_uri_ref (desktop_uri);
		return desktop_uri;
	}

	/* Prepend the base for the desktop URI.
	 * If the SchemeDescription contains > 1 directory, we use the directory
	 * after the first if the given file actually exists there.
	 */
	new_uri = NULL;
	path = gnome_vfs_uri_get_path (desktop_uri);
	i = 0;
	while (desc->directories[i])
		++i;
	do {
		char *s;

		--i;
		
		s = create_file_uri_in_dir (desc->directories[i], path);

		new_uri = gnome_vfs_uri_new (s);

		g_free (s);
		
		if (i == 0 ||
		    gnome_vfs_uri_exists (new_uri)) {
			return new_uri;
		} else {
			gnome_vfs_uri_unref (new_uri);
			new_uri = NULL;
		}
	} while (i > 0);


	g_assert_not_reached ();

	return NULL;
}


static GnomeVFSResult
open_merged_directory (GnomeVFSMethod *method,
		       GnomeVFSMethodHandle **method_handle,
		       GnomeVFSURI *desktop_uri,
		       GnomeVFSFileInfoOptions options,
		       const GnomeVFSDirectoryFilter *filter,
		       GnomeVFSContext *context)
{
	GnomeVFSResult result;
	DirHandle *dh;
	const SchemeDescription *desc;
	int i;
	gboolean found;
	const char *path;
	
	desc = get_desc_for_uri (desktop_uri);
	
	if (desc == NULL) {
		return GNOME_VFS_ERROR_NOT_FOUND;
	}

	dh = g_new0 (DirHandle, 1);
	
	/* Prepend the base for the desktop URI.
	 * If the SchemeDescription contains > 1 directory, we use the directory
	 * after the first if the given file actually exists there.
	 */
	found = FALSE;
	path = gnome_vfs_uri_get_path (desktop_uri);
	i = 0;
	while (desc->directories[i]) {
		char *s;
		GnomeVFSURI *file_uri;
		GnomeVFSMethodHandle *parent_handle = NULL;
		
		s = create_file_uri_in_dir (desc->directories[i], path);

		file_uri = gnome_vfs_uri_new (s);

		g_free (s);

		result = (* parent_method->open_directory) (parent_method,
							    &parent_handle,
							    file_uri,
							    options,
							    filter,
							    context);

		if (result == GNOME_VFS_OK) {
			found = TRUE;
			dh->handles = g_slist_prepend (dh->handles, parent_handle);
		}

		gnome_vfs_uri_unref (file_uri);

		++i;
	}

	dh->next = dh->handles;

	*method_handle = (GnomeVFSMethodHandle*) dh;
	
	return found ? GNOME_VFS_OK : GNOME_VFS_ERROR_NOT_FOUND;
}


static char*
create_file_uri_in_dir (const char  *dir,
			const char  *filename)
{
	char *dir_uri;
	char *retval;
	
	dir_uri = gnome_vfs_get_uri_from_local_path (dir);

	retval = g_strconcat (dir_uri, "/", filename, NULL);

	g_free (dir_uri);
	
	return retval;
}





