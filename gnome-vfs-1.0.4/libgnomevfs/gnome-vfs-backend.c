/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-backend.c - Handling of asynchronocity backends in the GNOME
                         Virtual File System.

   Copyright (C) 2000 Red Hat, Inc.
   Copyright (C) 2000 Eazel, Inc.
   All rights reserved.

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

   Author: Elliot Lee <sopwith@redhat.com>
*/

#include <config.h>
#include "gnome-vfs-backend.h"

#include "gnome-vfs-backend-private.h"
#include "gnome-vfs-module-callback.h"
#include "gnome-vfs.h"
#include <gmodule.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static GModule *gmod = NULL;
static gboolean (* gnome_vfs_backend_module_init)(gboolean deps_init);

static char *backend_lower;

const char *
gnome_vfs_backend_name (void)
{
	return (*backend_lower) ? backend_lower : NULL;
}

void
gnome_vfs_backend_loadinit (gpointer app, gpointer modinfo)
{
	const char *backend;
	char *short_name, *backend_filename, *init_func;

	/* Decide which backend module to load, based on
	   (a) environment variable
	   (b) default
	*/
	if (gmod != NULL) {
		return;
	}

	backend = getenv ("GNOME_VFS_BACKEND");
	if (backend == NULL) {
		backend = "pthread";
	}

	backend_lower = g_strdup (backend);
	g_strdown (backend_lower);

	short_name = g_strdup_printf ("gnomevfs-%s", backend);
	backend_filename = g_module_build_path (NULL, short_name);

	gmod = g_module_open (backend_filename, G_MODULE_BIND_LAZY);
	if (gmod == NULL) {
		g_error("Could not open %s: %s", backend_filename, g_module_error());
	}
	g_free (backend_filename);

	init_func = g_strdup_printf ("gnome_vfs_%s_init", backend_lower);
	if (!g_module_symbol (gmod, init_func, 
		(gpointer *)&gnome_vfs_backend_module_init)) {
		g_module_close (gmod); 
		gmod = NULL;
		g_error("Could not locate module initialization function: %s", g_module_error());
	}
	g_free (init_func);

}

gboolean
gnome_vfs_backend_init (gboolean deps_init)
{
	g_assert (gmod);
	g_assert (gnome_vfs_backend_init);

	gnome_vfs_backend_module_init (deps_init);

	return TRUE;
}

void
gnome_vfs_backend_shutdown (void)
{
	/* find and call the backend shutdown function */
	void (* thread_backend_shutdown_call) (void);
	
	g_assert (gmod);
	if (g_module_symbol (gmod, "gnome_vfs_thread_backend_shutdown", 
			      (gpointer)&thread_backend_shutdown_call)) {
		g_assert (thread_backend_shutdown_call);
		(* thread_backend_shutdown_call) ();
	}
}

typedef GnomeVFSResult (*GnomeVFSAsyncFunction) ();

static GnomeVFSAsyncFunction
func_lookup(const char *func_name)
{
	char *name;
	gpointer function;

	name = g_strdup_printf ("%s_%s", backend_lower, func_name);

	g_assert (gmod);
	if (!g_module_symbol (gmod, name, &function)) {
		function = NULL;
	}

	g_free (name);

	return (GnomeVFSAsyncFunction) function;
}

#define CALL_BACKEND(name, parameters) \
G_STMT_START { \
	if (real_##name == NULL) { \
		real_##name = (void (*)()) func_lookup (#name); \
	} \
	if (real_##name == NULL) { \
		g_warning ("can't find " #name " in the back end"); \
	} else { \
		real_##name parameters; \
	} \
} G_STMT_END

#define CALL_BACKEND_RETURN(name, parameters) \
G_STMT_START { \
	if (real_##name == NULL) { \
		real_##name = func_lookup (#name); \
	} \
	if (real_##name == NULL) { \
		g_warning ("can't find " #name " in the back end"); \
		return GNOME_VFS_ERROR_INTERNAL; \
	} \
	return real_##name parameters; \
} G_STMT_END

void
gnome_vfs_async_open (GnomeVFSAsyncHandle **handle_return,
		      const gchar *text_uri,
		      GnomeVFSOpenMode open_mode,
		      GnomeVFSAsyncOpenCallback callback,
		      gpointer callback_data)
{
	static void	 
		(*real_gnome_vfs_async_open) (GnomeVFSAsyncHandle **handle_return,
					      const gchar *text_uri,
					      GnomeVFSOpenMode open_mode,
					      GnomeVFSAsyncOpenCallback callback,
					      gpointer callback_data) = NULL;

	CALL_BACKEND (gnome_vfs_async_open,
		      (handle_return, text_uri, open_mode, callback, callback_data));
}

void
gnome_vfs_async_open_uri (GnomeVFSAsyncHandle **handle_return,
			  GnomeVFSURI *uri,
			  GnomeVFSOpenMode open_mode,
			  GnomeVFSAsyncOpenCallback callback,
			  gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_open_uri) (GnomeVFSAsyncHandle **handle_return,
						  GnomeVFSURI *uri,
						  GnomeVFSOpenMode open_mode,
						  GnomeVFSAsyncOpenCallback callback,
						  gpointer callback_data) = NULL;

	CALL_BACKEND (gnome_vfs_async_open_uri,
		      (handle_return, uri, open_mode, callback, callback_data));
}

void
gnome_vfs_async_open_as_channel (GnomeVFSAsyncHandle **handle_return,
				 const gchar *text_uri,
				 GnomeVFSOpenMode open_mode,
				 guint advised_block_size,
				 GnomeVFSAsyncOpenAsChannelCallback callback,
				 gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_open_as_channel) (GnomeVFSAsyncHandle **handle_return,
							 const gchar *text_uri,
							 GnomeVFSOpenMode open_mode,
							 guint advised_block_size,
							 GnomeVFSAsyncOpenAsChannelCallback callback,
							 gpointer callback_data) = NULL;

	CALL_BACKEND (gnome_vfs_async_open_as_channel,
		      (handle_return, text_uri, open_mode, advised_block_size,
		       callback, callback_data));
}

void	 
gnome_vfs_async_create (GnomeVFSAsyncHandle **handle_return,
			const gchar *text_uri,
			GnomeVFSOpenMode open_mode,
			gboolean exclusive,
			guint perm,
			GnomeVFSAsyncOpenCallback callback,
			gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_create) (GnomeVFSAsyncHandle **handle_return,
						const gchar *text_uri,
						GnomeVFSOpenMode open_mode,
						gboolean exclusive,
						guint perm,
						GnomeVFSAsyncOpenCallback callback,
						gpointer callback_data) = NULL;

	CALL_BACKEND (gnome_vfs_async_create,
		      (handle_return, text_uri, open_mode, exclusive, perm,
		       callback, callback_data));
}

void
gnome_vfs_async_create_as_channel (GnomeVFSAsyncHandle **handle_return,
				   const gchar *text_uri,
				   GnomeVFSOpenMode open_mode,
				   gboolean exclusive,
				   guint perm,
				   GnomeVFSAsyncOpenAsChannelCallback callback,
				   gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_create_as_channel) (GnomeVFSAsyncHandle **handle_return,
							   const gchar *text_uri,
							   GnomeVFSOpenMode open_mode,
							   gboolean exclusive,
							   guint perm,
							   GnomeVFSAsyncOpenAsChannelCallback callback,
							   gpointer callback_data) = NULL;
	
	CALL_BACKEND (gnome_vfs_async_create_as_channel,
		      (handle_return, text_uri, open_mode, exclusive, perm,
		       callback, callback_data));
}

void
gnome_vfs_async_create_uri (GnomeVFSAsyncHandle **handle_return,
			    GnomeVFSURI *text_uri,
			    GnomeVFSOpenMode open_mode,
			    gboolean exclusive,
			    guint perm,
			    GnomeVFSAsyncOpenCallback callback,
			    gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_create_uri) (GnomeVFSAsyncHandle **handle_return,
						    GnomeVFSURI *uri,
						    GnomeVFSOpenMode open_mode,
						    gboolean exclusive,
						    guint perm,
						    GnomeVFSAsyncOpenCallback callback,
						    gpointer callback_data);

	CALL_BACKEND (gnome_vfs_async_create_uri,
		      (handle_return, text_uri, open_mode, exclusive, perm,
		       callback, callback_data));
}


void
gnome_vfs_async_create_symbolic_link (GnomeVFSAsyncHandle **handle_return,
				      GnomeVFSURI *uri,
				      const gchar *uri_reference,
				      GnomeVFSAsyncOpenCallback callback,
				      gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_create_symbolic_link) (GnomeVFSAsyncHandle **handle_return,
							      GnomeVFSURI *uri,
							      const gchar *uri_reference,
							      GnomeVFSAsyncOpenCallback callback,
							      gpointer callback_data);

	CALL_BACKEND (gnome_vfs_async_create_symbolic_link,
		      (handle_return, uri, uri_reference, callback, callback_data));
}

void	 
gnome_vfs_async_close (GnomeVFSAsyncHandle *handle,
		       GnomeVFSAsyncCloseCallback callback,
		       gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_close) (GnomeVFSAsyncHandle *handle,
					       GnomeVFSAsyncCloseCallback callback,
					       gpointer callback_data);

	CALL_BACKEND (gnome_vfs_async_close,
		      (handle, callback, callback_data));
}

void	 
gnome_vfs_async_read (GnomeVFSAsyncHandle *handle,
		      gpointer buffer,
		      guint bytes,
		      GnomeVFSAsyncReadCallback callback,
		      gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_read) (GnomeVFSAsyncHandle *handle,
					      gpointer buffer,
					      guint bytes,
					      GnomeVFSAsyncReadCallback callback,
					      gpointer callback_data);

	CALL_BACKEND (gnome_vfs_async_read,
		      (handle, buffer, bytes,
		       callback, callback_data));
}

void	 
gnome_vfs_async_write (GnomeVFSAsyncHandle *handle,
		       gconstpointer buffer,
		       guint bytes,
		       GnomeVFSAsyncWriteCallback callback,
		       gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_write) (GnomeVFSAsyncHandle *handle,
					       gconstpointer buffer,
					       guint bytes,
					       GnomeVFSAsyncWriteCallback callback,
					       gpointer callback_data);

	CALL_BACKEND (gnome_vfs_async_write,
		      (handle, buffer, bytes,
		       callback, callback_data));
}

void
gnome_vfs_async_get_file_info  (GnomeVFSAsyncHandle **handle_return,
				GList *uris,
				GnomeVFSFileInfoOptions options,
				GnomeVFSAsyncGetFileInfoCallback callback,
				gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_get_file_info) (GnomeVFSAsyncHandle **handle_return,
						       GList *uris,
						       GnomeVFSFileInfoOptions options,
						       GnomeVFSAsyncGetFileInfoCallback callback,
						       gpointer callback_data);

	CALL_BACKEND (gnome_vfs_async_get_file_info,
		      (handle_return, uris, options,
		       callback, callback_data));
}

void
gnome_vfs_async_find_directory (GnomeVFSAsyncHandle **handle_return,
				GList *near_uri_list,
				GnomeVFSFindDirectoryKind kind,
				gboolean create_if_needed,
		   		gboolean find_if_needed,
				guint permissions,
				GnomeVFSAsyncFindDirectoryCallback callback,
				gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_find_directory) (GnomeVFSAsyncHandle **handle_return,
						        GList *near_uri_list,
						        GnomeVFSFindDirectoryKind kind,
						        gboolean create_if_needed,
						        gboolean find_if_needed,
						        guint permissions,
						        GnomeVFSAsyncFindDirectoryCallback callback,
						        gpointer callback_data);
	
	CALL_BACKEND (gnome_vfs_async_find_directory,
		      (handle_return, near_uri_list, kind, create_if_needed, find_if_needed,
		       permissions, callback, callback_data));
}

void
gnome_vfs_async_set_file_info  (GnomeVFSAsyncHandle **handle_return,
				GnomeVFSURI *uri,
				GnomeVFSFileInfo *info,
				GnomeVFSSetFileInfoMask mask,
			        GnomeVFSFileInfoOptions options,
				GnomeVFSAsyncSetFileInfoCallback callback,
				gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_set_file_info) (GnomeVFSAsyncHandle **handle_return,
						       GnomeVFSURI *uri,
						       GnomeVFSFileInfo *info,
						       GnomeVFSSetFileInfoMask mask,
						       GnomeVFSFileInfoOptions options,
						       GnomeVFSAsyncSetFileInfoCallback callback,
						       gpointer callback_data);

	CALL_BACKEND (gnome_vfs_async_set_file_info,
		      (handle_return, uri, info, mask, options,
		       callback, callback_data));
}

void
gnome_vfs_async_load_directory_uri (GnomeVFSAsyncHandle **handle_return,
				    GnomeVFSURI *uri,
				    GnomeVFSFileInfoOptions options,
				    GnomeVFSDirectoryFilterType filter_type,
				    GnomeVFSDirectoryFilterOptions filter_options,
				    const gchar *filter_pattern,
				    guint items_per_notification,
				    GnomeVFSAsyncDirectoryLoadCallback callback,
				    gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_load_directory_uri) (GnomeVFSAsyncHandle **handle_return,
							    GnomeVFSURI *uri,
							    GnomeVFSFileInfoOptions options,
							    GnomeVFSDirectoryFilterType filter_type,
							    GnomeVFSDirectoryFilterOptions filter_options,
							    const gchar *filter_pattern,
							    guint items_per_notification,
							    GnomeVFSAsyncDirectoryLoadCallback callback,
							    gpointer callback_data);
	
	CALL_BACKEND (gnome_vfs_async_load_directory_uri,
		      (handle_return, uri, options,
		       filter_type, filter_options,
		       filter_pattern, items_per_notification,
		       callback, callback_data));
}

void
gnome_vfs_async_load_directory (GnomeVFSAsyncHandle **handle_return,
				const gchar *uri,
				GnomeVFSFileInfoOptions options,
				GnomeVFSDirectoryFilterType filter_type,
				GnomeVFSDirectoryFilterOptions filter_options,
				const gchar *filter_pattern,
				guint items_per_notification,
				GnomeVFSAsyncDirectoryLoadCallback callback,
				gpointer callback_data)
{
	static void
		(*real_gnome_vfs_async_load_directory) (GnomeVFSAsyncHandle **handle_return,
							const gchar *uri,
							GnomeVFSFileInfoOptions options,
							GnomeVFSDirectoryFilterType filter_type,
							GnomeVFSDirectoryFilterOptions filter_options,
							const gchar *filter_pattern,
							guint items_per_notification,
							GnomeVFSAsyncDirectoryLoadCallback callback,
							gpointer callback_data);

	CALL_BACKEND (gnome_vfs_async_load_directory,
		      (handle_return, uri, options,
		       filter_type, filter_options, filter_pattern, items_per_notification,
		       callback, callback_data));
}

GnomeVFSResult
gnome_vfs_async_xfer (GnomeVFSAsyncHandle **handle_return,
		      const GList *source_uri_list,
		      const GList *target_uri_list,
		      GnomeVFSXferOptions xfer_options,
		      GnomeVFSXferErrorMode error_mode,
		      GnomeVFSXferOverwriteMode overwrite_mode,
		      GnomeVFSAsyncXferProgressCallback progress_update_callback,
		      gpointer update_callback_data,
		      GnomeVFSXferProgressCallback progress_sync_callback,
		      gpointer sync_callback_data)
{
	static GnomeVFSResult
		(*real_gnome_vfs_async_xfer) (GnomeVFSAsyncHandle **handle_return,
					      const GList *source_uri_list,
					      const GList *target_uri_list,
					      GnomeVFSXferOptions xfer_options,
					      GnomeVFSXferErrorMode error_mode,
					      GnomeVFSXferOverwriteMode overwrite_mode,
					      GnomeVFSAsyncXferProgressCallback progress_update_callback,
					      gpointer update_callback_data,
					      GnomeVFSXferProgressCallback progress_sync_callback,
					      gpointer sync_callback_data);

	CALL_BACKEND_RETURN (gnome_vfs_async_xfer,
			     (handle_return,
			      source_uri_list, target_uri_list,
			      xfer_options, error_mode, overwrite_mode,
			      progress_update_callback, update_callback_data,
			      progress_sync_callback, sync_callback_data));
}

void
gnome_vfs_async_cancel (GnomeVFSAsyncHandle *handle)
{
	static void
		(*real_gnome_vfs_async_cancel)(GnomeVFSAsyncHandle *handle);

	CALL_BACKEND (gnome_vfs_async_cancel, (handle));
}

void
gnome_vfs_backend_get_current_context (/* OUT */ GnomeVFSContext **context)
{
	static void
		(*real_gnome_vfs_get_current_context) (GnomeVFSContext **context);

	CALL_BACKEND (gnome_vfs_get_current_context, (context));

}

void
gnome_vfs_backend_dispatch_module_callback (GnomeVFSAsyncModuleCallback callback,
					    gconstpointer in,
					    gsize in_size,
					    gpointer out,
					    gsize out_size,
					    gpointer user_data,
					    GnomeVFSModuleCallbackResponse response,
					    gpointer response_data)
{
	static void
		(*real_gnome_vfs_dispatch_module_callback) (GnomeVFSAsyncModuleCallback callback,
							   gpointer user_data,
							   gconstpointer in, gsize in_size,
							   gpointer out, gsize out_size,
							   GnomeVFSModuleCallbackResponse response,
							   gpointer response_data);

	CALL_BACKEND (gnome_vfs_dispatch_module_callback, (callback,
							   user_data,
							   in, in_size,
							   out, out_size,
							   response, response_data));
}



int
gnome_vfs_backend_get_job_count (void)
{
	/* find and call the backend function */

	int (* get_count) (void);
	
	g_assert (gmod != NULL);
	if (g_module_symbol (gmod,
			     "gnome_vfs_job_get_count",
			     (gpointer) &get_count)) {
		return (* get_count) ();
	}

	return -1;
}
