/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-async-ops.c - Asynchronous operations supported by the
   GNOME Virtual File System (version for POSIX threads).

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

#include "gnome-vfs-job.h"
#include "gnome-vfs-async-job-map.h"

#include <unistd.h>

void           pthread_gnome_vfs_async_cancel                 (GnomeVFSAsyncHandle                 *handle);
void           pthread_gnome_vfs_async_open_uri               (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       GnomeVFSOpenMode                     open_mode,
							       GnomeVFSAsyncOpenCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_open                   (GnomeVFSAsyncHandle                **handle_return,
							       const gchar                         *text_uri,
							       GnomeVFSOpenMode                     open_mode,
							       GnomeVFSAsyncOpenCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_open_uri_as_channel    (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       GnomeVFSOpenMode                     open_mode,
							       guint                                advised_block_size,
							       GnomeVFSAsyncOpenAsChannelCallback   callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_open_as_channel        (GnomeVFSAsyncHandle                **handle_return,
							       const gchar                         *text_uri,
							       GnomeVFSOpenMode                     open_mode,
							       guint                                advised_block_size,
							       GnomeVFSAsyncOpenAsChannelCallback   callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_create_uri             (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       GnomeVFSOpenMode                     open_mode,
							       gboolean                             exclusive,
							       guint                                perm,
							       GnomeVFSAsyncOpenCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_create_symbolic_link   (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       const gchar                         *uri_reference,
							       GnomeVFSAsyncOpenCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_create                 (GnomeVFSAsyncHandle                **handle_return,
							       const gchar                         *text_uri,
							       GnomeVFSOpenMode                     open_mode,
							       gboolean                             exclusive,
							       guint                                perm,
							       GnomeVFSAsyncOpenCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_create_as_channel      (GnomeVFSAsyncHandle                **handle_return,
							       const gchar                         *text_uri,
							       GnomeVFSOpenMode                     open_mode,
							       gboolean                             exclusive,
							       guint                                perm,
							       GnomeVFSAsyncOpenAsChannelCallback   callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_close                  (GnomeVFSAsyncHandle                 *handle,
							       GnomeVFSAsyncCloseCallback           callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_read                   (GnomeVFSAsyncHandle                 *handle,
							       gpointer                             buffer,
							       guint                                bytes,
							       GnomeVFSAsyncReadCallback            callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_write                  (GnomeVFSAsyncHandle                 *handle,
							       gconstpointer                        buffer,
							       guint                                bytes,
							       GnomeVFSAsyncWriteCallback           callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_get_file_info          (GnomeVFSAsyncHandle                **handle_return,
							       GList                               *uris,
							       GnomeVFSFileInfoOptions              options,
							       GnomeVFSAsyncGetFileInfoCallback     callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_set_file_info          (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       GnomeVFSFileInfo                    *info,
							       GnomeVFSSetFileInfoMask              mask,
							       GnomeVFSFileInfoOptions              options,
							       GnomeVFSAsyncSetFileInfoCallback     callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_load_directory         (GnomeVFSAsyncHandle                **handle_return,
							       const gchar                         *text_uri,
							       GnomeVFSFileInfoOptions              options,
							       GnomeVFSDirectoryFilterType          filter_type,
							       GnomeVFSDirectoryFilterOptions       filter_options,
							       const gchar                         *filter_pattern,
							       guint                                items_per_notification,
							       GnomeVFSAsyncDirectoryLoadCallback   callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_load_directory_uri     (GnomeVFSAsyncHandle                **handle_return,
							       GnomeVFSURI                         *uri,
							       GnomeVFSFileInfoOptions              options,
							       GnomeVFSDirectoryFilterType          filter_type,
							       GnomeVFSDirectoryFilterOptions       filter_options,
							       const gchar                         *filter_pattern,
							       guint                                items_per_notification,
							       GnomeVFSAsyncDirectoryLoadCallback   callback,
							       gpointer                             callback_data);
void           pthread_gnome_vfs_async_find_directory         (GnomeVFSAsyncHandle                **handle_return,
							       GList                               *uris,
							       GnomeVFSFindDirectoryKind            kind,
							       gboolean                             create_if_needed,
							       gboolean                             find_if_needed,
							       guint                                permissions,
							       GnomeVFSAsyncFindDirectoryCallback   callback,
							       gpointer                             user_data);
GnomeVFSResult pthread_gnome_vfs_async_xfer                   (GnomeVFSAsyncHandle                **handle_return,
							       GList                               *source_uri_list,
							       GList                               *target_uri_list,
							       GnomeVFSXferOptions                  xfer_options,
							       GnomeVFSXferErrorMode                error_mode,
							       GnomeVFSXferOverwriteMode            overwrite_mode,
							       GnomeVFSAsyncXferProgressCallback    progress_update_callback,
							       gpointer                             update_callback_data,
							       GnomeVFSXferProgressCallback         progress_sync_callback,
							       gpointer                             sync_callback_data);
guint          pthread_gnome_vfs_async_add_status_callback    (GnomeVFSAsyncHandle                 *handle,
							       GnomeVFSStatusCallback               callback,
							       gpointer                             user_data);
void           pthread_gnome_vfs_async_remove_status_callback (GnomeVFSAsyncHandle                 *handle,
							       guint                                callback_id);


void
pthread_gnome_vfs_async_cancel (GnomeVFSAsyncHandle *handle)
{
	GnomeVFSJob *job;
	
	gnome_vfs_async_job_map_lock ();

	job = gnome_vfs_async_job_map_get_job (handle);
	if (job == NULL) {
		JOB_DEBUG (("job %u - job no longer exists", GPOINTER_TO_UINT (handle)));
		/* have to cancel the callbacks because they still can be pending */
		gnome_vfs_async_job_cancel_job_and_callbacks (handle, NULL);
	} else {
		/* Cancel the job in progress. OK to do outside of job->access_lock,
		 * job lifetime is protected by gnome_vfs_async_job_map_lock.
		 */
		gnome_vfs_job_module_cancel (job);
		gnome_vfs_async_job_cancel_job_and_callbacks (handle, job);
	}

	gnome_vfs_async_job_map_unlock ();
}

static GnomeVFSAsyncHandle *
async_open (GnomeVFSURI *uri,
	    GnomeVFSOpenMode open_mode,
	    GnomeVFSAsyncOpenCallback callback,
	    gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSOpenOp *open_op;
	GnomeVFSAsyncHandle *result;
	
	job = gnome_vfs_job_new (GNOME_VFS_OP_OPEN, (GFunc) callback, callback_data);
	
	open_op = &job->op->specifics.open;
	
	open_op->uri = uri == NULL ? NULL : gnome_vfs_uri_ref (uri);
	open_op->open_mode = open_mode;

	result = job->job_handle;
	gnome_vfs_job_go (job);

	return result;
}

void
pthread_gnome_vfs_async_open_uri (GnomeVFSAsyncHandle **handle_return,
				  GnomeVFSURI *uri,
				  GnomeVFSOpenMode open_mode,
				  GnomeVFSAsyncOpenCallback callback,
				  gpointer callback_data)
{
	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (callback != NULL);
	
	*handle_return = async_open (uri, open_mode,
				     callback, callback_data);
}

void
pthread_gnome_vfs_async_open (GnomeVFSAsyncHandle **handle_return,
			      const gchar *text_uri,
			      GnomeVFSOpenMode open_mode,
			      GnomeVFSAsyncOpenCallback callback,
			      gpointer callback_data)
{
	GnomeVFSURI *uri;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (text_uri != NULL);
	g_return_if_fail (callback != NULL);

	uri = gnome_vfs_uri_new (text_uri);
	*handle_return = async_open (uri, open_mode,
				     callback, callback_data);
	if (uri != NULL) {
		gnome_vfs_uri_unref (uri);
	}
}

static GnomeVFSAsyncHandle *
async_open_as_channel (GnomeVFSURI *uri,
		       GnomeVFSOpenMode open_mode,
		       guint advised_block_size,
		       GnomeVFSAsyncOpenAsChannelCallback callback,
		       gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSOpenAsChannelOp *open_as_channel_op;
	GnomeVFSAsyncHandle *result;

	job = gnome_vfs_job_new (GNOME_VFS_OP_OPEN_AS_CHANNEL, (GFunc) callback, callback_data);

	open_as_channel_op = &job->op->specifics.open_as_channel;
	open_as_channel_op->uri = uri == NULL ? NULL : gnome_vfs_uri_ref (uri);
	open_as_channel_op->open_mode = open_mode;
	open_as_channel_op->advised_block_size = advised_block_size;

	result = job->job_handle;
	gnome_vfs_job_go (job);

	return result;
}

void
pthread_gnome_vfs_async_open_uri_as_channel (GnomeVFSAsyncHandle **handle_return,
					     GnomeVFSURI *uri,
					     GnomeVFSOpenMode open_mode,
					     guint advised_block_size,
					     GnomeVFSAsyncOpenAsChannelCallback callback,
					     gpointer callback_data)
{
	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (callback != NULL);

	*handle_return = async_open_as_channel (uri, open_mode, advised_block_size,
						callback, callback_data);
}

void
pthread_gnome_vfs_async_open_as_channel (GnomeVFSAsyncHandle **handle_return,
					 const gchar *text_uri,
					 GnomeVFSOpenMode open_mode,
					 guint advised_block_size,
					 GnomeVFSAsyncOpenAsChannelCallback callback,
					 gpointer callback_data)
{
	GnomeVFSURI *uri;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (text_uri != NULL);
	g_return_if_fail (callback != NULL);

	uri = gnome_vfs_uri_new (text_uri);
	*handle_return = async_open_as_channel (uri, open_mode, advised_block_size,
						callback, callback_data);
	if (uri != NULL) {
		gnome_vfs_uri_unref (uri);
	}
}

static GnomeVFSAsyncHandle *
async_create (GnomeVFSURI *uri,
	      GnomeVFSOpenMode open_mode,
	      gboolean exclusive,
	      guint perm,
	      GnomeVFSAsyncOpenCallback callback,
	      gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSCreateOp *create_op;
	GnomeVFSAsyncHandle *result;

	job = gnome_vfs_job_new (GNOME_VFS_OP_CREATE, (GFunc) callback, callback_data);

	create_op = &job->op->specifics.create;
	create_op->uri = uri == NULL ? NULL : gnome_vfs_uri_ref (uri);
	create_op->open_mode = open_mode;
	create_op->exclusive = exclusive;
	create_op->perm = perm;

	result = job->job_handle;
	gnome_vfs_job_go (job);

	return result;
}

void
pthread_gnome_vfs_async_create_uri (GnomeVFSAsyncHandle **handle_return,
				    GnomeVFSURI *uri,
				    GnomeVFSOpenMode open_mode,
				    gboolean exclusive,
				    guint perm,
				    GnomeVFSAsyncOpenCallback callback,
				    gpointer callback_data)
{
	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (callback != NULL);

	*handle_return = async_create (uri, open_mode, exclusive, perm,
				       callback, callback_data);
}

void
pthread_gnome_vfs_async_create (GnomeVFSAsyncHandle **handle_return,
				const gchar *text_uri,
				GnomeVFSOpenMode open_mode,
				gboolean exclusive,
				guint perm,
				GnomeVFSAsyncOpenCallback callback,
				gpointer callback_data)
{
	GnomeVFSURI *uri;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (text_uri != NULL);
	g_return_if_fail (callback != NULL);

	uri = gnome_vfs_uri_new (text_uri);
	*handle_return = async_create (uri, open_mode, exclusive, perm,
				       callback, callback_data);
	if (uri != NULL) {
		gnome_vfs_uri_unref (uri);
	}
}

void
pthread_gnome_vfs_async_create_as_channel (GnomeVFSAsyncHandle **handle_return,
					   const gchar *text_uri,
					   GnomeVFSOpenMode open_mode,
					   gboolean exclusive,
					   guint perm,
					   GnomeVFSAsyncOpenAsChannelCallback callback,
					   gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSCreateAsChannelOp *create_as_channel_op;
	GnomeVFSAsyncHandle *result;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (text_uri != NULL);
	g_return_if_fail (callback != NULL);

	job = gnome_vfs_job_new (GNOME_VFS_OP_CREATE_AS_CHANNEL, (GFunc) callback, callback_data);


	create_as_channel_op = &job->op->specifics.create_as_channel;
	create_as_channel_op->uri = gnome_vfs_uri_new (text_uri);
	create_as_channel_op->open_mode = open_mode;
	create_as_channel_op->exclusive = exclusive;
	create_as_channel_op->perm = perm;

	result = job->job_handle;
	gnome_vfs_job_go (job);
}

void
pthread_gnome_vfs_async_close (GnomeVFSAsyncHandle *handle,
			       GnomeVFSAsyncCloseCallback callback,
			       gpointer callback_data)
{
	GnomeVFSJob *job;

	g_return_if_fail (handle != NULL);
	g_return_if_fail (callback != NULL);

	for (;;) {
		gnome_vfs_async_job_map_lock ();
		job = gnome_vfs_async_job_map_get_job (handle);
		if (job == NULL) {
			g_warning ("trying to read a non-existing handle");
			gnome_vfs_async_job_map_unlock ();
			return;
		}

		if (job->op->type != GNOME_VFS_OP_READ
			&& job->op->type != GNOME_VFS_OP_WRITE) {
			gnome_vfs_job_set (job, GNOME_VFS_OP_CLOSE,
					   (GFunc) callback, callback_data);
			gnome_vfs_job_go (job);
			gnome_vfs_async_job_map_unlock ();
			return;
		}
		/* Still reading, wait a bit, cancel should be pending.
		 * This mostly handles a race condition that can happen
		 * on a dual CPU machine where a cancel stops a read before
		 * the read thread picks up and a close then gets scheduled
		 * on a new thread. Without this the job op type would be
		 * close for both threads and two closes would get executed
		 */
		gnome_vfs_async_job_map_unlock ();
		usleep (100);
	}
}

void
pthread_gnome_vfs_async_read (GnomeVFSAsyncHandle *handle,
			      gpointer buffer,
			      guint bytes,
			      GnomeVFSAsyncReadCallback callback,
			      gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSReadOp *read_op;

	g_return_if_fail (handle != NULL);
	g_return_if_fail (buffer != NULL);
	g_return_if_fail (callback != NULL);

	gnome_vfs_async_job_map_lock ();
	job = gnome_vfs_async_job_map_get_job (handle);
	if (job == NULL) {
		g_warning ("trying to read from a non-existing handle");
		gnome_vfs_async_job_map_unlock ();
		return;
	}

	gnome_vfs_job_set (job, GNOME_VFS_OP_READ,
			   (GFunc) callback, callback_data);

	read_op = &job->op->specifics.read;
	read_op->buffer = buffer;
	read_op->num_bytes = bytes;

	gnome_vfs_job_go (job);
	gnome_vfs_async_job_map_unlock ();
}

void
pthread_gnome_vfs_async_write (GnomeVFSAsyncHandle *handle,
			       gconstpointer buffer,
			       guint bytes,
			       GnomeVFSAsyncWriteCallback callback,
			       gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSWriteOp *write_op;

	g_return_if_fail (handle != NULL);
	g_return_if_fail (buffer != NULL);
	g_return_if_fail (callback != NULL);

	gnome_vfs_async_job_map_lock ();
	job = gnome_vfs_async_job_map_get_job (handle);
	if (job == NULL) {
		g_warning ("trying to write to a non-existing handle");
		gnome_vfs_async_job_map_unlock ();
		return;
	}

	gnome_vfs_job_set (job, GNOME_VFS_OP_WRITE,
			   (GFunc) callback, callback_data);

	write_op = &job->op->specifics.write;
	write_op->buffer = buffer;
	write_op->num_bytes = bytes;

	gnome_vfs_job_go (job);
	gnome_vfs_async_job_map_unlock ();
}

void
pthread_gnome_vfs_async_create_symbolic_link (GnomeVFSAsyncHandle **handle_return,
					      GnomeVFSURI *uri,
					      const gchar *uri_reference,
					      GnomeVFSAsyncOpenCallback callback,
					      gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSCreateLinkOp *create_op;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (callback != NULL);

	job = gnome_vfs_job_new (GNOME_VFS_OP_CREATE_SYMBOLIC_LINK, (GFunc) callback, callback_data);

	create_op = &job->op->specifics.create_symbolic_link;
	create_op->uri = gnome_vfs_uri_ref (uri);
	create_op->uri_reference = g_strdup (uri_reference);

	*handle_return = job->job_handle;
	gnome_vfs_job_go (job);
}

void
pthread_gnome_vfs_async_get_file_info (GnomeVFSAsyncHandle **handle_return,
				       GList *uris,
				       GnomeVFSFileInfoOptions options,
				       GnomeVFSAsyncGetFileInfoCallback callback,
				       gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSGetFileInfoOp *get_info_op;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (callback != NULL);

	job = gnome_vfs_job_new (GNOME_VFS_OP_GET_FILE_INFO, (GFunc) callback, callback_data);

	get_info_op = &job->op->specifics.get_file_info;

	get_info_op->uris = gnome_vfs_uri_list_copy (uris);
	get_info_op->options = options;


	*handle_return = job->job_handle;
	gnome_vfs_job_go (job);
}

void
pthread_gnome_vfs_async_set_file_info (GnomeVFSAsyncHandle **handle_return,
				       GnomeVFSURI *uri,
				       GnomeVFSFileInfo *info,
				       GnomeVFSSetFileInfoMask mask,
				       GnomeVFSFileInfoOptions options,
				       GnomeVFSAsyncSetFileInfoCallback callback,
				       gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSSetFileInfoOp *op;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (info != NULL);
	g_return_if_fail (callback != NULL);

	job = gnome_vfs_job_new (GNOME_VFS_OP_SET_FILE_INFO, (GFunc) callback, callback_data);

	op = &job->op->specifics.set_file_info;

	op->uri = gnome_vfs_uri_ref (uri);
	op->info = gnome_vfs_file_info_new ();
	gnome_vfs_file_info_copy (op->info, info);
	op->mask = mask;
	op->options = options;

	*handle_return = job->job_handle;
	gnome_vfs_job_go (job);
}

void
pthread_gnome_vfs_async_find_directory (GnomeVFSAsyncHandle **handle_return,
					GList *uris,
					GnomeVFSFindDirectoryKind kind,
					gboolean create_if_needed,
					gboolean find_if_needed,
					guint permissions,
					GnomeVFSAsyncFindDirectoryCallback callback,
					gpointer user_data)
{
	GnomeVFSJob *job;
	GnomeVFSFindDirectoryOp *get_info_op;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (callback != NULL);

	job = gnome_vfs_job_new (GNOME_VFS_OP_FIND_DIRECTORY, (GFunc) callback, user_data);

	get_info_op = &job->op->specifics.find_directory;

	get_info_op->uris = gnome_vfs_uri_list_copy (uris);
	get_info_op->kind = kind;
	get_info_op->create_if_needed = create_if_needed;
	get_info_op->find_if_needed = find_if_needed;
	get_info_op->permissions = permissions;

	*handle_return = job->job_handle;
	gnome_vfs_job_go (job);
}

static GnomeVFSAsyncHandle *
async_load_directory (GnomeVFSURI *uri,
		      GnomeVFSFileInfoOptions options,
		      GnomeVFSDirectoryFilterType filter_type,
		      GnomeVFSDirectoryFilterOptions filter_options,
		      const gchar *filter_pattern,
		      guint items_per_notification,
		      GnomeVFSAsyncDirectoryLoadCallback callback,
		      gpointer callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSLoadDirectoryOp *load_directory_op;
	GnomeVFSAsyncHandle *result;

	job = gnome_vfs_job_new (GNOME_VFS_OP_LOAD_DIRECTORY, (GFunc) callback, callback_data);

	load_directory_op = &job->op->specifics.load_directory;
	load_directory_op->uri = uri == NULL ? NULL : gnome_vfs_uri_ref (uri);
	load_directory_op->options = options;
	load_directory_op->filter_type = filter_type;
	load_directory_op->filter_options = filter_options;
	load_directory_op->filter_pattern = g_strdup (filter_pattern);
	load_directory_op->items_per_notification = items_per_notification;


	result = job->job_handle;
	gnome_vfs_job_go (job);

	return result;
}

void
pthread_gnome_vfs_async_load_directory (GnomeVFSAsyncHandle **handle_return,
					const gchar *text_uri,
					GnomeVFSFileInfoOptions options,
					GnomeVFSDirectoryFilterType filter_type,
					GnomeVFSDirectoryFilterOptions filter_options,
					const gchar *filter_pattern,
					guint items_per_notification,
					GnomeVFSAsyncDirectoryLoadCallback callback,
					gpointer callback_data)
{
	GnomeVFSURI *uri;

	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (text_uri != NULL);
	g_return_if_fail (callback != NULL);

	uri = gnome_vfs_uri_new (text_uri);
	*handle_return = async_load_directory (uri, options,
				               filter_type, filter_options, filter_pattern,
				               items_per_notification,
				               callback, callback_data);
	if (uri != NULL) {
		gnome_vfs_uri_unref (uri);
	}
}

void
pthread_gnome_vfs_async_load_directory_uri (GnomeVFSAsyncHandle **handle_return,
					    GnomeVFSURI *uri,
					    GnomeVFSFileInfoOptions options,
					    GnomeVFSDirectoryFilterType filter_type,
					    GnomeVFSDirectoryFilterOptions filter_options,
					    const gchar *filter_pattern,
					    guint items_per_notification,
					    GnomeVFSAsyncDirectoryLoadCallback callback,
					    gpointer callback_data)
{
	g_return_if_fail (handle_return != NULL);
	g_return_if_fail (uri != NULL);
	g_return_if_fail (callback != NULL);

	*handle_return = async_load_directory (uri, options,
					       filter_type, filter_options, filter_pattern,
					       items_per_notification,
					       callback, callback_data);
}

GnomeVFSResult
pthread_gnome_vfs_async_xfer (GnomeVFSAsyncHandle **handle_return,
			      GList *source_uri_list,
			      GList *target_uri_list,
			      GnomeVFSXferOptions xfer_options,
			      GnomeVFSXferErrorMode error_mode,
			      GnomeVFSXferOverwriteMode overwrite_mode,
			      GnomeVFSAsyncXferProgressCallback progress_update_callback,
			      gpointer update_callback_data,
			      GnomeVFSXferProgressCallback progress_sync_callback,
			      gpointer sync_callback_data)
{
	GnomeVFSJob *job;
	GnomeVFSXferOp *xfer_op;

	g_return_val_if_fail (handle_return != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (progress_update_callback != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);

	job = gnome_vfs_job_new (GNOME_VFS_OP_XFER,
			         (GFunc) progress_update_callback,
			         update_callback_data);


	xfer_op = &job->op->specifics.xfer;
	xfer_op->source_uri_list = gnome_vfs_uri_list_copy (source_uri_list);
	xfer_op->target_uri_list = gnome_vfs_uri_list_copy (target_uri_list);
	xfer_op->xfer_options = xfer_options;
	xfer_op->error_mode = error_mode;
	xfer_op->overwrite_mode = overwrite_mode;
	xfer_op->progress_sync_callback = progress_sync_callback;
	xfer_op->sync_callback_data = sync_callback_data;

	gnome_vfs_job_go (job);

	*handle_return = job->job_handle;

	return GNOME_VFS_OK;
}

#ifdef OLD_CONTEXT_DEPRECATED

guint
pthread_gnome_vfs_async_add_status_callback (GnomeVFSAsyncHandle *handle,
					     GnomeVFSStatusCallback callback,
					     gpointer user_data)
{
	GnomeVFSJob *job;
	guint result;
	
	g_return_val_if_fail (handle != NULL, 0);
	g_return_val_if_fail (callback != NULL, 0);

	gnome_vfs_async_job_map_lock ();
	job = gnome_vfs_async_job_map_get_job (handle);

	if (job->op != NULL || job->op->context != NULL) {
		g_warning ("job or context not found");
		gnome_vfs_async_job_map_unlock ();
		return 0;
	}

	result = gnome_vfs_message_callbacks_add
		(gnome_vfs_context_get_message_callbacks (job->op->context),
		 callback, user_data);
	gnome_vfs_async_job_map_unlock ();
	
	return result;
}

void
pthread_gnome_vfs_async_remove_status_callback (GnomeVFSAsyncHandle *handle,
						guint callback_id)
{
	GnomeVFSJob *job;

	g_return_if_fail (handle != NULL);
	g_return_if_fail (callback_id > 0);

	gnome_vfs_async_job_map_lock ();
	job = gnome_vfs_async_job_map_get_job (handle);

	if (job->op != NULL || job->op->context != NULL) {
		g_warning ("job or context not found");
		gnome_vfs_async_job_map_unlock ();
		return;
	}

	gnome_vfs_message_callbacks_remove
		(gnome_vfs_context_get_message_callbacks (job->op->context),
		 callback_id);

	gnome_vfs_async_job_map_unlock ();
}

#endif /* OLD_CONTEXT_DEPRECATED */
