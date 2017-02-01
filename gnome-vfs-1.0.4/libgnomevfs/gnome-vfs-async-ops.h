/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-async-ops.h - Asynchronous operations in the GNOME Virtual File
   System.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@comm2000.it> */

#ifndef GNOME_VFS_ASYNC_OPS_H
#define GNOME_VFS_ASYNC_OPS_H

#include <libgnomevfs/gnome-vfs-directory-filter.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-find-directory.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-xfer.h>

typedef struct GnomeVFSAsyncHandle GnomeVFSAsyncHandle;

typedef void	(* GnomeVFSAsyncCallback)	(GnomeVFSAsyncHandle *handle,
						 GnomeVFSResult result,
						 gpointer callback_data);

typedef GnomeVFSAsyncCallback GnomeVFSAsyncOpenCallback;
typedef GnomeVFSAsyncCallback GnomeVFSAsyncCreateCallback;

typedef void	(* GnomeVFSAsyncOpenAsChannelCallback)
						(GnomeVFSAsyncHandle *handle,
						 GIOChannel *channel,
						 GnomeVFSResult result,
						 gpointer callback_data);

typedef GnomeVFSAsyncOpenAsChannelCallback GnomeVFSAsyncCreateAsChannelCallback;

#define GnomeVFSAsyncCloseCallback	GnomeVFSAsyncCallback

typedef void	(* GnomeVFSAsyncReadCallback)	(GnomeVFSAsyncHandle *handle,
						 GnomeVFSResult result,
						 gpointer buffer,
						 GnomeVFSFileSize bytes_requested,
						 GnomeVFSFileSize bytes_read,
						 gpointer callback_data);

typedef void	(* GnomeVFSAsyncWriteCallback)	(GnomeVFSAsyncHandle *handle,
						 GnomeVFSResult result,
						 gconstpointer buffer,
						 GnomeVFSFileSize bytes_requested,
						 GnomeVFSFileSize bytes_written,
						 gpointer callback_data);


typedef void    (* GnomeVFSAsyncGetFileInfoCallback)
                                                (GnomeVFSAsyncHandle *handle,
						 GList *results, /* GnomeVFSGetFileInfoResult* items */
						 gpointer callback_data);

typedef void	(* GnomeVFSAsyncSetFileInfoCallback)	
						(GnomeVFSAsyncHandle *handle,
						 GnomeVFSResult result,
						 GnomeVFSFileInfo *file_info,
						 gpointer callback_data);


typedef void	(* GnomeVFSAsyncDirectoryLoadCallback)
						(GnomeVFSAsyncHandle *handle,
						 GnomeVFSResult result,
						 GList *list,
						 guint entries_read,
						 gpointer callback_data);

typedef gint    (* GnomeVFSAsyncXferProgressCallback)
						(GnomeVFSAsyncHandle *handle,
						 GnomeVFSXferProgressInfo *info,
						 gpointer data);

typedef struct {
	GnomeVFSURI *uri;
	GnomeVFSResult result;
} GnomeVFSFindDirectoryResult;

typedef void    (* GnomeVFSAsyncFindDirectoryCallback)
						(GnomeVFSAsyncHandle *handle,
						 GList *results /* GnomeVFSFindDirectoryResult */,
						 gpointer data);


void           gnome_vfs_async_cancel                 (GnomeVFSAsyncHandle                   *handle);

void           gnome_vfs_async_open                   (GnomeVFSAsyncHandle                  **handle_return,
						       const gchar                           *text_uri,
						       GnomeVFSOpenMode                       open_mode,
						       GnomeVFSAsyncOpenCallback              callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_open_uri               (GnomeVFSAsyncHandle                  **handle_return,
						       GnomeVFSURI                           *uri,
						       GnomeVFSOpenMode                       open_mode,
						       GnomeVFSAsyncOpenCallback              callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_open_as_channel        (GnomeVFSAsyncHandle                  **handle_return,
						       const gchar                           *text_uri,
						       GnomeVFSOpenMode                       open_mode,
						       guint                                  advised_block_size,
						       GnomeVFSAsyncOpenAsChannelCallback     callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_open_uri_as_channel    (GnomeVFSAsyncHandle                  **handle_return,
						       GnomeVFSURI                           *uri,
						       GnomeVFSOpenMode                       open_mode,
						       guint                                  advised_block_size,
						       GnomeVFSAsyncOpenAsChannelCallback     callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_create                 (GnomeVFSAsyncHandle                  **handle_return,
						       const gchar                           *text_uri,
						       GnomeVFSOpenMode                       open_mode,
						       gboolean                               exclusive,
						       guint                                  perm,
						       GnomeVFSAsyncOpenCallback              callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_create_uri             (GnomeVFSAsyncHandle                  **handle_return,
						       GnomeVFSURI                           *uri,
						       GnomeVFSOpenMode                       open_mode,
						       gboolean                               exclusive,
						       guint                                  perm,
						       GnomeVFSAsyncOpenCallback              callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_create_symbolic_link   (GnomeVFSAsyncHandle                  **handle_return,
						       GnomeVFSURI                           *uri,
						       const gchar                           *uri_reference,
						       GnomeVFSAsyncOpenCallback              callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_create_as_channel      (GnomeVFSAsyncHandle                  **handle_return,
						       const gchar                           *text_uri,
						       GnomeVFSOpenMode                       open_mode,
						       gboolean                               exclusive,
						       guint                                  perm,
						       GnomeVFSAsyncCreateAsChannelCallback   callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_create_uri_as_channel  (GnomeVFSAsyncHandle                  **handle_return,
						       GnomeVFSURI                           *uri,
						       GnomeVFSOpenMode                       open_mode,
						       gboolean                               exclusive,
						       guint                                  perm,
						       GnomeVFSAsyncCreateAsChannelCallback   callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_close                  (GnomeVFSAsyncHandle                   *handle,
						       GnomeVFSAsyncCloseCallback             callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_read                   (GnomeVFSAsyncHandle                   *handle,
						       gpointer                               buffer,
						       guint                                  bytes,
						       GnomeVFSAsyncReadCallback              callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_write                  (GnomeVFSAsyncHandle                   *handle,
						       gconstpointer                          buffer,
						       guint                                  bytes,
						       GnomeVFSAsyncWriteCallback             callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_get_file_info          (GnomeVFSAsyncHandle                  **handle_return,
						       GList                                 *uri_list,
						       GnomeVFSFileInfoOptions                options,
						       GnomeVFSAsyncGetFileInfoCallback       callback,
						       gpointer                               callback_data);

/* Setting the file info sometimes changes more info than the
 * caller specified; for example, if the name changes the MIME type might
 * change, and if the owner changes the SUID & SGID bits might change. 
 * Therefore the callback returns the new file info for the caller's
 * convenience. The GnomeVFSFileInfoOptions passed here are those used 
 * for the returned file info; they are not used when setting.
 */
void           gnome_vfs_async_set_file_info          (GnomeVFSAsyncHandle                  **handle_return,
						       GnomeVFSURI                           *uri,
						       GnomeVFSFileInfo                      *info,
						       GnomeVFSSetFileInfoMask                mask,
						       GnomeVFSFileInfoOptions                options,
						       GnomeVFSAsyncSetFileInfoCallback       callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_load_directory         (GnomeVFSAsyncHandle                  **handle_return,
						       const gchar                           *text_uri,
						       GnomeVFSFileInfoOptions                options,
						       GnomeVFSDirectoryFilterType            filter_type,
						       GnomeVFSDirectoryFilterOptions         filter_options,
						       const gchar                           *filter_pattern,
						       guint                                  items_per_notification,
						       GnomeVFSAsyncDirectoryLoadCallback     callback,
						       gpointer                               callback_data);
void           gnome_vfs_async_load_directory_uri     (GnomeVFSAsyncHandle                  **handle_return,
						       GnomeVFSURI                           *uri,
						       GnomeVFSFileInfoOptions                options,
						       GnomeVFSDirectoryFilterType            filter_type,
						       GnomeVFSDirectoryFilterOptions         filter_options,
						       const gchar                           *filter_pattern,
						       guint                                  items_per_notification,
						       GnomeVFSAsyncDirectoryLoadCallback     callback,
						       gpointer                               callback_data);
GnomeVFSResult gnome_vfs_async_xfer                   (GnomeVFSAsyncHandle                  **handle_return,
						       const GList                           *source_uri_list,
						       const GList                           *target_uri_list,
						       GnomeVFSXferOptions                    xfer_options,
						       GnomeVFSXferErrorMode                  error_mode,
						       GnomeVFSXferOverwriteMode              overwrite_mode,
						       GnomeVFSAsyncXferProgressCallback      progress_update_callback,
						       gpointer                               update_callback_data,
						       GnomeVFSXferProgressCallback           progress_sync_callback,
						       gpointer                               sync_callback_data);
void           gnome_vfs_async_find_directory         (GnomeVFSAsyncHandle                  **handle_return,
						       GList                                 *near_uri_list,
						       GnomeVFSFindDirectoryKind              kind,
						       gboolean                               create_if_needed,
						       gboolean                               find_if_needed,
						       guint                                  permissions,
						       GnomeVFSAsyncFindDirectoryCallback     callback,
						       gpointer                               user_data);

#endif /* GNOME_VFS_ASYNC_OPS_H */
