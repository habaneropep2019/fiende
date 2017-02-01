/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* search-method.c: Gnome-VFS interface to the medusa search service

   Copyright (C) 2000 Eazel

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

   Authors: Seth Nickell      (seth@eazel.com) 
            Maciej Stachowiak (mjs@eazel.com)  */


#include <config.h>

#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-context.h>
#include <libgnomevfs/gnome-vfs-cancellation.h>
#include <libgnomevfs/gnome-vfs-cancellable-ops.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-method.h>
#include <libmedusa/medusa-search-service.h>
#include <string.h>
#include <unistd.h>

/* Used for the search directory. */
typedef struct {
	MedusaSearchServiceConnection *connection;
	GnomeVFSFileInfoOptions options;
} SearchRealDirectoryHandle;

/* Used for directories inside the search directory. */
typedef struct {
	GnomeVFSDirectoryHandle *handle;
} SearchWrappedDirectoryHandle;

/* This is the kind of handle we return for directory calls. */
typedef struct {
	gboolean wrapped;
	union {
		SearchRealDirectoryHandle real;
		SearchWrappedDirectoryHandle wrapped;
	} u;
} SearchDirectoryHandle;

/* This is the kind of handle we return for file calls. */
typedef struct {
	GnomeVFSHandle *handle;
	char *name; /* used for get_file_info_from_handle only */
} SearchFileHandle;

typedef enum {
	SEARCH_URI_TYPE_TOP_LEVEL,
	SEARCH_URI_TYPE_IMMEDIATE_CHILD,
	SEARCH_URI_TYPE_DEEPER_CHILD
} SearchURIType;

#define READ_EXEC_PERMISSIONS \
	GNOME_VFS_PERM_USER_READ | \
	GNOME_VFS_PERM_USER_EXEC | \
	GNOME_VFS_PERM_GROUP_READ | \
	GNOME_VFS_PERM_GROUP_EXEC | \
	GNOME_VFS_PERM_OTHER_READ | \
	GNOME_VFS_PERM_OTHER_EXEC

/* Module entry points. */
GnomeVFSMethod *vfs_module_init     (const char     *method_name,
				     const char     *args);
void            vfs_module_shutdown (GnomeVFSMethod *method);

static SearchDirectoryHandle *
search_real_directory_handle_new (GnomeVFSFileInfoOptions options,
				  MedusaSearchServiceConnection *connection)
{
	SearchDirectoryHandle *result;

	result = g_new (SearchDirectoryHandle, 1);

	result->wrapped = FALSE;
	result->u.real.connection = connection;
	result->u.real.options = options;

	return result;
}

static SearchDirectoryHandle *
search_wrapped_directory_handle_new (GnomeVFSDirectoryHandle *handle)

{
	SearchDirectoryHandle *result;

	result = g_new (SearchDirectoryHandle, 1);

	result->wrapped = TRUE;
	result->u.wrapped.handle = handle;

	return result;
}

static void
search_directory_handle_destroy (SearchDirectoryHandle *handle)
{
	if (handle->wrapped) {
		gnome_vfs_directory_close (handle->u.wrapped.handle);
	} else {
		medusa_search_service_connection_destroy (handle->u.real.connection);
	}

	g_free (handle);
}

static SearchFileHandle *
search_file_handle_new (GnomeVFSHandle *handle,
			const char *unescaped_name)
{
	SearchFileHandle *result;

	result = g_new (SearchFileHandle, 1);

	result->handle = handle;
	result->name = unescaped_name == NULL ? NULL
		: gnome_vfs_escape_string (unescaped_name);

	return result;
}

static void
search_file_handle_destroy (SearchFileHandle *handle)
{
	gnome_vfs_handle_destroy (handle->handle);
	g_free (handle->name);
	g_free (handle);
}

/* Allocates a new char * representing the file URI portion of
 * a gnome-vfs search URI.
 */
static SearchURIType
parse_search_uri (const GnomeVFSURI *uri,
		  char **child_uri_text)
{
	const char *path;
	const char *first_slash, *second_slash, *endpoint;
	char *escaped, *unescaped;

	path = gnome_vfs_uri_get_path (uri);
	if (path == NULL) {
		*child_uri_text = NULL;
		return SEARCH_URI_TYPE_TOP_LEVEL;
	}

	/* Find the first slash and be sure it's not at the end. */
	first_slash = strchr (path, '/');
	if (first_slash == NULL || first_slash[1] == '\0') {
		*child_uri_text = NULL;
		return SEARCH_URI_TYPE_TOP_LEVEL;
	}

	/* Find the second slash if any. */
	second_slash = strchr (first_slash + 1, '/');
	if (second_slash == NULL) {
		endpoint = path + strlen (path);
	} else {
		endpoint = second_slash;
	}
	
	/* Extract the text between the first and second slash and
	 * unescape it.
	 */
	escaped = g_strndup (first_slash + 1, endpoint - (first_slash + 1));
	unescaped = gnome_vfs_unescape_string (escaped, NULL);
	g_free (escaped);

	/* Include the second slash and the text after it if there was any.
	 * If there was nothing after the second slash, then the escaped URI
	 * alone is the entire child URI.
	 */
	if (second_slash == NULL || second_slash[1] == '\0') {
		*child_uri_text = unescaped;
		return SEARCH_URI_TYPE_IMMEDIATE_CHILD;
	} else {
		*child_uri_text = g_strconcat (unescaped, second_slash, NULL);
		g_free (unescaped);
		return SEARCH_URI_TYPE_DEEPER_CHILD;
	}
}

static GnomeVFSResult
open_cancellable_cover (const char *uri_text,
			const char *name,
			GnomeVFSMethodHandle **method_handle,
			GnomeVFSOpenMode mode,
			GnomeVFSContext *context)
{
	GnomeVFSURI *vfs_uri;
	GnomeVFSResult result;
	GnomeVFSHandle *handle_to_wrap;
	
	vfs_uri = gnome_vfs_uri_new (uri_text);
	result = gnome_vfs_open_uri_cancellable
		(&handle_to_wrap, vfs_uri, mode, context);
	gnome_vfs_uri_unref (vfs_uri);
	if (result == GNOME_VFS_OK) {
		*method_handle = (GnomeVFSMethodHandle *) search_file_handle_new
			(handle_to_wrap, name);
	}
	return result;
}

static GnomeVFSResult
do_open (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI *uri,
	 GnomeVFSOpenMode mode,
	 GnomeVFSContext *context)
{
	GnomeVFSResult result;
	char *child_uri_text;

	switch (parse_search_uri (uri, &child_uri_text)) {

	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:
		result = GNOME_VFS_ERROR_NOT_SUPPORTED;
		break;
	case SEARCH_URI_TYPE_IMMEDIATE_CHILD:
		result = open_cancellable_cover
			(child_uri_text, child_uri_text,
			 method_handle, mode, context);
		break;
	case SEARCH_URI_TYPE_DEEPER_CHILD:
		/* Pass NULL for name so get_file_info_from_handle
		 * will use the real name of the file.
		 */
		result = open_cancellable_cover
			(child_uri_text, NULL,
			 method_handle, mode, context);
		break;
	}

	g_free (child_uri_text);
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
	GnomeVFSResult result;
	char *child_uri_text;
	GnomeVFSHandle *handle_to_wrap;
	GnomeVFSURI *vfs_uri;

	switch (parse_search_uri (uri, &child_uri_text)) {

	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:
		result = GNOME_VFS_ERROR_NOT_SUPPORTED;
		break;
	case SEARCH_URI_TYPE_IMMEDIATE_CHILD:
		result = GNOME_VFS_ERROR_NOT_PERMITTED;
		break;
	case SEARCH_URI_TYPE_DEEPER_CHILD:
		vfs_uri = gnome_vfs_uri_new (child_uri_text);
		result = gnome_vfs_create_uri_cancellable (&handle_to_wrap,
							   vfs_uri, mode, 
							   exclusive, perm, context);
		gnome_vfs_uri_unref (vfs_uri);
		if (result == GNOME_VFS_OK) {
			*method_handle = (GnomeVFSMethodHandle *) search_file_handle_new
				(handle_to_wrap, NULL);
		}
	}
	
	g_free (child_uri_text);
	return result;
}

static GnomeVFSResult
do_close (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext *context)
{
	SearchFileHandle *handle;
	GnomeVFSResult result;
	
	handle = (SearchFileHandle *) method_handle;
	
	result = gnome_vfs_close_cancellable
		(handle->handle, context);

	search_file_handle_destroy (handle);

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
	SearchFileHandle *handle;
	
	handle = (SearchFileHandle *) method_handle;
	
	return gnome_vfs_read_cancellable
		(handle->handle, buffer, num_bytes, bytes_read, context);
}

static GnomeVFSResult
do_write (GnomeVFSMethod *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer buffer,
	  GnomeVFSFileSize num_bytes,
	  GnomeVFSFileSize *bytes_written,
	  GnomeVFSContext *context)
{
	SearchFileHandle *handle;
	
	handle = (SearchFileHandle *) method_handle;
	
	return gnome_vfs_write_cancellable
		(handle->handle, buffer, num_bytes, bytes_written, context);
}

static GnomeVFSResult
do_seek (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSSeekPosition whence,
	 GnomeVFSFileOffset offset,
	 GnomeVFSContext *context)
{
	SearchFileHandle *handle;
	
	handle = (SearchFileHandle *) method_handle;
	
	return gnome_vfs_seek_cancellable
		(handle->handle, whence, offset, context);
}

static GnomeVFSResult
do_tell (GnomeVFSMethod *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSFileOffset *offset_return)
{
	SearchFileHandle *handle;
	
	handle = (SearchFileHandle *) method_handle;
	
	return gnome_vfs_tell
		(handle->handle, offset_return);
}


/* FIXME bugzilla.eazel.com 2615:
   This function works around a problem in gnome-vfs where you
 * can't get the original URI without added slashes. That problem
 * should be fixed and then this can be removed.
 */
static char *
uri_to_string_remove_extra_slashes (const GnomeVFSURI *uri)
{
	char *uri_text, *past_colon, *result;

	/* Remove the "//" after the ":" in this URI.
	 * It's safe to assume there there is a ":".
	 */
	uri_text = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	past_colon = strchr (uri_text, ':') + 1;
	if (strncmp (past_colon, "//", 2) == 0) {
		result = g_new (char, strlen (uri_text) - 2 + 1);
		memcpy (result, uri_text, past_colon - uri_text);
		strcpy (result + (past_colon - uri_text), past_colon + 2);
		g_free (uri_text);
	} else {
		result = uri_text;
	}

	return result;
}

static GnomeVFSResult
do_open_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle **method_handle,
		   GnomeVFSURI *uri,
		   GnomeVFSFileInfoOptions options,
		   const GnomeVFSDirectoryFilter *filter,
		   GnomeVFSContext *context)
{
	MedusaSearchServiceConnection *connection;
	char *uri_text;
	char *unescaped_uri_text;
	char *child_uri_text;
	GnomeVFSDirectoryHandle *handle;
	GnomeVFSResult result;


	switch (parse_search_uri (uri, &child_uri_text)) {

	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:

		
		/* FIXME bugzilla.eazel.com 2616:
		   Since medusa already knows about search
		 * URIs, the escaped character handling should be done
		 * there. We should just pass the URI text as-is and
		 * not unescape it ourselves.
		 */
		uri_text = uri_to_string_remove_extra_slashes (uri);
		unescaped_uri_text = gnome_vfs_unescape_string (uri_text, NULL);
		g_free (uri_text);
		connection = medusa_search_service_connection_new (unescaped_uri_text,
								   context,
								   &result);
		g_free (unescaped_uri_text);
		if (result != GNOME_VFS_OK) {

			return result;
		}
		result = medusa_search_service_connection_start_search (connection);



		if (result == GNOME_VFS_OK) {
			*method_handle = (GnomeVFSMethodHandle *) search_real_directory_handle_new
				(options, connection);
		}
		break;

	case SEARCH_URI_TYPE_IMMEDIATE_CHILD:
	case SEARCH_URI_TYPE_DEEPER_CHILD:
		result = gnome_vfs_directory_open
			(&handle, child_uri_text, options,
			 filter);
		if (result == GNOME_VFS_OK) {
			*method_handle = (GnomeVFSMethodHandle *) search_wrapped_directory_handle_new 
				(handle);
		}
		break;
	}

	g_free (child_uri_text);

	return result;
}

static GnomeVFSResult
do_close_directory (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext *context)
{
	SearchDirectoryHandle *wrapped_handle;

	wrapped_handle = (SearchDirectoryHandle *) method_handle;

	search_directory_handle_destroy (wrapped_handle);

	return GNOME_VFS_OK;
}

static void
get_file_info_for_virtual_directory (GnomeVFSFileInfo *file_info)
{
	g_assert (file_info->name == NULL);
	file_info->name = g_strdup ("/");
	file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
	file_info->permissions = READ_EXEC_PERMISSIONS;
	GNOME_VFS_FILE_INFO_SET_LOCAL (file_info, TRUE);
	file_info->uid = getuid();
	file_info->gid = getgid();
	file_info->mtime = time (NULL);

	/* FIXME bugzilla.eazel.com 2617:
	   We should only set the mime type here if 
	   the caller requests the mime type for
	   consistency
	 */
	g_assert (file_info->mime_type == NULL);
	file_info->mime_type = g_strdup ("x-directory/search");
	
	file_info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE
		| GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS
		| GNOME_VFS_FILE_INFO_FIELDS_FLAGS
		| GNOME_VFS_FILE_INFO_FIELDS_MTIME
		| GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

}

static void
get_file_info_for_virtual_link (GnomeVFSFileInfo *file_info, const char *target_uri_text)
{
	GnomeVFSURI *target_uri;

	target_uri = gnome_vfs_uri_new (target_uri_text);

	g_assert (file_info->name == NULL);
	file_info->name = gnome_vfs_escape_string (target_uri_text);
	file_info->type = GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK;
	file_info->permissions = READ_EXEC_PERMISSIONS;
	GNOME_VFS_FILE_INFO_SET_LOCAL (file_info, TRUE);
	file_info->uid = getuid();
	file_info->gid = getgid();
	/* FIXME bugzilla.eazel.com 2618:
	   The gnome-vfs symlink_name field holds a path only
	 * (not a URI). This means that the search results must all be
	 * on the local file system. Luckily, they are at the moment
	 * due to limitations in medusa, but this may have to be dealt
	 * with eventually.
	 */
	g_assert (file_info->symlink_name == NULL);
	file_info->symlink_name = g_strdup (gnome_vfs_uri_get_path (target_uri));
	/* FIXME bugzilla.eazel.com 2617:
	   We should check the mode before returning the
	   mime type for consistency
	 */
	g_assert (file_info->mime_type == NULL);
	file_info->mime_type = g_strdup ("x-special/symlink");
	
	file_info->valid_fields = GNOME_VFS_FILE_INFO_FIELDS_TYPE
		| GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS
		| GNOME_VFS_FILE_INFO_FIELDS_FLAGS
		| GNOME_VFS_FILE_INFO_FIELDS_SYMLINK_NAME
		| GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

	gnome_vfs_uri_unref (target_uri);
}

static GnomeVFSResult
do_read_directory (GnomeVFSMethod *method,
		   GnomeVFSMethodHandle *method_handle,
		   GnomeVFSFileInfo *file_info,
		   GnomeVFSContext *context)
{
	SearchDirectoryHandle *wrapped_handle;
	GnomeVFSResult result;
	char *result_uri;
	GnomeVFSURI *vfs_uri;

	wrapped_handle = (SearchDirectoryHandle *) method_handle;

	if (wrapped_handle->wrapped) {
		result = gnome_vfs_directory_read_next (wrapped_handle->u.wrapped.handle,
							file_info);
	} else {
		if (gnome_vfs_context_check_cancellation (context)) {
			return GNOME_VFS_ERROR_INTERRUPTED;
		}

		result = medusa_search_service_connection_read_search_result 
			(wrapped_handle->u.real.connection, context, &result_uri);

		if (result == GNOME_VFS_OK) {
			/* FIXME bugzilla.eazel.com 2619:
			   Add code to do filtering. Only the
			 * file method currently implements filtering
			 * so no one can really use it with an
			 * arbitrary directory handle anyway, so there
			 * is no big hurry to fix this method in
			 * particular, but when we decide what to do
			 * about this we should fix this method (and
			 * the others).
			 */

			if ((wrapped_handle->u.real.options & GNOME_VFS_FILE_INFO_FOLLOW_LINKS) == 0) {
				get_file_info_for_virtual_link (file_info, result_uri);
				result = GNOME_VFS_OK;
			} else {
				/* Get the file info. */
				vfs_uri = gnome_vfs_uri_new (result_uri);
				/* Ignore the return value here: we don't want to
				   interrupt the load because of errors relating to a single file */
				gnome_vfs_get_file_info_uri_cancellable
					(vfs_uri, file_info, 
					 wrapped_handle->u.real.options, 
					 context);
				gnome_vfs_uri_unref (vfs_uri);
				
				/* Substitute link name since file info
				 * for a symbolic link is supposed to
				 * have the name of the link, not the
				 * target file.
				 */
				g_free (file_info->name);
				file_info->name = gnome_vfs_escape_string (result_uri);
			}
			GNOME_VFS_FILE_INFO_SET_SYMLINK (file_info, TRUE);
			
			g_free (result_uri);
		}
	}

	return result;
}

static GnomeVFSResult
get_file_info_cancellable_cover (const char *uri_text,
				 GnomeVFSFileInfo *file_info,
				 GnomeVFSFileInfoOptions options,
				 GnomeVFSContext *context)
{
	GnomeVFSURI *vfs_uri;
	GnomeVFSResult result;

	vfs_uri = gnome_vfs_uri_new (uri_text);
	result = gnome_vfs_get_file_info_uri_cancellable
		(vfs_uri, file_info, options, 
		 context);
	gnome_vfs_uri_unref (vfs_uri);

	return result;
}

static GnomeVFSResult
do_get_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  GnomeVFSFileInfo *file_info,
		  GnomeVFSFileInfoOptions options,
		  GnomeVFSContext *context)
{
	GnomeVFSResult result;
	char *child_uri_text;

	switch (parse_search_uri (uri, &child_uri_text)) {
		
	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:
		get_file_info_for_virtual_directory (file_info);
		/* Need a check here to make sure the search service
		   is really running before returning any information */
		if (gnome_vfs_context_check_cancellation (context)) {
			return GNOME_VFS_ERROR_INTERRUPTED;
		}
		result = medusa_search_service_connection_is_available_for_uri (uri);
		break;
	case SEARCH_URI_TYPE_IMMEDIATE_CHILD:
		if ((options & GNOME_VFS_FILE_INFO_FOLLOW_LINKS) == 0) {
			/* If we are told to not follow links, we get the info
			 * about the virtual symbolic link that represents
			 * this particular search result.
			 */
			get_file_info_for_virtual_link (file_info, child_uri_text);
			result = GNOME_VFS_OK;
		} else {
			/* Call through to get the file info. */
			result = get_file_info_cancellable_cover (child_uri_text,
								  file_info, options, 
								  context);
			/* Substitute link name since file info
			 * through a symbolic link is supposed to have
			 * the name of the link, not the target file.
			 */
			g_free (file_info->name);
			file_info->name = gnome_vfs_escape_string (child_uri_text);
		}
		if (result == GNOME_VFS_OK) {
			g_assert ((file_info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_FLAGS) != 0);
			GNOME_VFS_FILE_INFO_SET_SYMLINK (file_info, TRUE);
		}
		break;
	case SEARCH_URI_TYPE_DEEPER_CHILD:
		result = get_file_info_cancellable_cover (child_uri_text,
							  file_info, options, 
							  context);
		/* Use the file name returned by get_file info, since
		 * this is not one of our virtual links.
		 */
		break;
	}

	g_free (child_uri_text);
	return result;
}

static GnomeVFSResult
do_get_file_info_from_handle (GnomeVFSMethod *method,
			      GnomeVFSMethodHandle *method_handle,
			      GnomeVFSFileInfo *file_info,
			      GnomeVFSFileInfoOptions options,
			      GnomeVFSContext *context)
{
	SearchFileHandle *handle;
	GnomeVFSResult result;

	handle = (SearchFileHandle *) method_handle;
	
	result = gnome_vfs_get_file_info_from_handle_cancellable 
		(handle->handle, file_info, options,
		 context);

	if (result == GNOME_VFS_OK && handle->name != NULL) {
		/* Substitute link name since file info
		 * through a symbolic link is supposed to have
		 * the name of the link, not the target file.
		 */
		g_free (file_info->name);
		file_info->name = g_strdup (handle->name);
	}

	return result;
}

static gboolean
do_is_local (GnomeVFSMethod *method,
	     const GnomeVFSURI *uri)
{
	gboolean result;
	char *child_uri_text;

	result = TRUE;

	switch (parse_search_uri (uri, &child_uri_text)) {

	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:
	case SEARCH_URI_TYPE_IMMEDIATE_CHILD:
		break;
	case SEARCH_URI_TYPE_DEEPER_CHILD:
		/* FIXME bugzilla.eazel.com 2620:
		   This should call through to the underlying
		 * file system, but there's no nice
		 * gnome_vfs_is_local_cancellable call to use.
		 */
		break;
	}

	g_free (child_uri_text);
	return result;
}

static GnomeVFSResult
do_make_directory (GnomeVFSMethod *method,
		   GnomeVFSURI *uri,
		   guint perm,
		   GnomeVFSContext *context)
{
	GnomeVFSResult result;
	char *child_uri_text;
	GnomeVFSURI *vfs_uri;

	switch (parse_search_uri (uri, &child_uri_text)) {

	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:
	case SEARCH_URI_TYPE_IMMEDIATE_CHILD:
		result = GNOME_VFS_ERROR_NOT_PERMITTED;
		break;
	case SEARCH_URI_TYPE_DEEPER_CHILD:
		vfs_uri = gnome_vfs_uri_new (child_uri_text);
		result = gnome_vfs_make_directory_for_uri_cancellable (vfs_uri, perm, context);
		gnome_vfs_uri_unref (vfs_uri);
		break;
	}

	g_free (child_uri_text);
	return result;
}

static GnomeVFSResult
do_remove_directory (GnomeVFSMethod *method,
		     GnomeVFSURI *uri,
		     GnomeVFSContext *context)
{
	GnomeVFSResult result;
	char *child_uri_text;
	GnomeVFSURI *vfs_uri;

	switch (parse_search_uri (uri, &child_uri_text)) {

	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:
		result = GNOME_VFS_ERROR_NOT_PERMITTED;
		break;
	case SEARCH_URI_TYPE_IMMEDIATE_CHILD:
		result = GNOME_VFS_ERROR_NOT_A_DIRECTORY;
		break;
	case SEARCH_URI_TYPE_DEEPER_CHILD:
		vfs_uri = gnome_vfs_uri_new (child_uri_text);
		result = gnome_vfs_remove_directory_from_uri_cancellable (vfs_uri, context);
		gnome_vfs_uri_unref (vfs_uri);
		break;
	}

	g_free (child_uri_text);
	return result;
}

static GnomeVFSResult
do_move (GnomeVFSMethod *method,
	 GnomeVFSURI *old_uri,
	 GnomeVFSURI *new_uri,
	 gboolean force_replace,
	 GnomeVFSContext *context)
{
	GnomeVFSResult result;
	SearchURIType old_type, new_type;
	char *old_child_text, *new_child_text;
	GnomeVFSURI *old_child, *new_child;

        old_type = parse_search_uri (old_uri, &old_child_text);
	new_type = parse_search_uri (new_uri, &new_child_text);

	if (old_type == SEARCH_URI_TYPE_DEEPER_CHILD && new_type == SEARCH_URI_TYPE_DEEPER_CHILD) {
		old_child = gnome_vfs_uri_new (old_child_text);
		new_child = gnome_vfs_uri_new (new_child_text);
		result = gnome_vfs_move_uri_cancellable (old_child, new_child, force_replace, context);
		gnome_vfs_uri_unref (old_child);
		gnome_vfs_uri_unref (new_child);
	} else {
		result = GNOME_VFS_ERROR_NOT_PERMITTED;
	}

	g_free (old_child_text);
	g_free (new_child_text);

	return result;
}

static GnomeVFSResult
do_unlink (GnomeVFSMethod *method,
	   GnomeVFSURI *uri,
	   GnomeVFSContext *context)
{
	GnomeVFSResult result;
	char *child_uri_text;
	GnomeVFSURI *vfs_uri;

	switch (parse_search_uri (uri, &child_uri_text)) {

	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:
	case SEARCH_URI_TYPE_IMMEDIATE_CHILD:
		result = GNOME_VFS_ERROR_NOT_PERMITTED;
		break;
	case SEARCH_URI_TYPE_DEEPER_CHILD:
		vfs_uri = gnome_vfs_uri_new (child_uri_text);
		result = gnome_vfs_unlink_from_uri_cancellable (vfs_uri, context);
		gnome_vfs_uri_unref (vfs_uri);
		break;
	}

	g_free (child_uri_text);
	return result;
}

static GnomeVFSResult
do_check_same_fs (GnomeVFSMethod *method,
		  GnomeVFSURI *a,
		  GnomeVFSURI *b,
		  gboolean *same_fs_return,
		  GnomeVFSContext *context)
{
	GnomeVFSResult result;
	SearchURIType a_type, b_type;
	char *a_child_text, *b_child_text;
	GnomeVFSURI *a_child, *b_child;

        a_type = parse_search_uri (a, &a_child_text);
	b_type = parse_search_uri (b, &b_child_text);

	if (a_type == SEARCH_URI_TYPE_DEEPER_CHILD && b_type == SEARCH_URI_TYPE_DEEPER_CHILD) {
		a_child = gnome_vfs_uri_new (a_child_text);
		b_child = gnome_vfs_uri_new (b_child_text);
		result = gnome_vfs_check_same_fs_uris_cancellable (a_child, b_child, same_fs_return, context);
		gnome_vfs_uri_unref (a_child);
		gnome_vfs_uri_unref (b_child);
	} else {
		*same_fs_return = TRUE;
		result = GNOME_VFS_OK;
	}

	g_free (a_child_text);
	g_free (b_child_text);

	return result;
}

static GnomeVFSResult
do_set_file_info (GnomeVFSMethod *method,
		  GnomeVFSURI *uri,
		  const GnomeVFSFileInfo *info,
		  GnomeVFSSetFileInfoMask mask,
		  GnomeVFSContext *context)
{	GnomeVFSResult result;
	char *child_uri_text;
	GnomeVFSURI *vfs_uri;

	switch (parse_search_uri (uri, &child_uri_text)) {
		
	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:
	case SEARCH_URI_TYPE_IMMEDIATE_CHILD:
		result = GNOME_VFS_ERROR_NOT_PERMITTED;
		break;
	case SEARCH_URI_TYPE_DEEPER_CHILD:
		vfs_uri = gnome_vfs_uri_new (child_uri_text);
		result = gnome_vfs_set_file_info_cancellable (vfs_uri, info, mask, context);
		gnome_vfs_uri_unref (vfs_uri);
		break;
	}

	g_free (child_uri_text);
	return result;
}

static GnomeVFSResult
do_truncate (GnomeVFSMethod *method,
	     GnomeVFSURI *uri,
	     GnomeVFSFileSize where,
	     GnomeVFSContext *context)
{
	GnomeVFSResult result;
	char *child_uri_text;
	GnomeVFSURI *vfs_uri;

	switch (parse_search_uri (uri, &child_uri_text)) {

	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:
		result = GNOME_VFS_ERROR_NOT_PERMITTED;
		break;
	case SEARCH_URI_TYPE_IMMEDIATE_CHILD:
	case SEARCH_URI_TYPE_DEEPER_CHILD:
		vfs_uri = gnome_vfs_uri_new (child_uri_text);
		result = gnome_vfs_truncate_uri_cancellable (vfs_uri, where, context);
		gnome_vfs_uri_unref (vfs_uri);
		break;
	}

	g_free (child_uri_text);
	return result;
}

static GnomeVFSResult
do_truncate_handle (GnomeVFSMethod *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSFileSize where,
		    GnomeVFSContext *context)
{
	SearchFileHandle *handle;
	
	handle = (SearchFileHandle *) method_handle;
	
	return gnome_vfs_truncate_handle_cancellable
		(handle->handle, where, context);
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
	GnomeVFSResult result;
	char *child_uri_text;
	GnomeVFSURI *vfs_uri;

	switch (parse_search_uri (near_uri, &child_uri_text)) {

	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:
	case SEARCH_URI_TYPE_IMMEDIATE_CHILD:
		result = GNOME_VFS_ERROR_NOT_SUPPORTED;
		break;
	case SEARCH_URI_TYPE_DEEPER_CHILD:		
		vfs_uri = gnome_vfs_uri_new (child_uri_text);
		result = gnome_vfs_find_directory_cancellable (vfs_uri, kind, result_uri, 
			create_if_needed, find_if_needed, permissions, context);
		gnome_vfs_uri_unref (vfs_uri);
		break;
	}

	g_free (child_uri_text);
	return result;
}

static GnomeVFSResult
do_create_symbolic_link (GnomeVFSMethod *method,
			 GnomeVFSURI *uri,
			 const char *target_reference,
			 GnomeVFSContext *context)
{
	GnomeVFSResult result;
	char *child_uri_text;
	GnomeVFSURI *vfs_uri;

	switch (parse_search_uri (uri, &child_uri_text)) {

	default:
	case SEARCH_URI_TYPE_TOP_LEVEL:
		result = GNOME_VFS_ERROR_NOT_PERMITTED;
		break;
	case SEARCH_URI_TYPE_DEEPER_CHILD:
		vfs_uri = gnome_vfs_uri_new (child_uri_text);
		result = gnome_vfs_create_symbolic_link_cancellable (vfs_uri, target_reference, context);
		gnome_vfs_uri_unref (vfs_uri);
		break;
	}

	g_free (child_uri_text);
	return result;
}

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


GnomeVFSMethod *
vfs_module_init (const char *method_name, const char *args)
{
	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
}
