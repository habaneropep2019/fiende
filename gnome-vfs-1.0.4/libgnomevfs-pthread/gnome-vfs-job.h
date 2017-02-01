/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-job.h - Jobs for asynchronous operation of the GNOME
   Virtual File System (version for POSIX threads).

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

   Author: Ettore Perazzoli <ettore@gnu.org>
*/

#ifndef GNOME_VFS_JOB_PTHREAD_H
#define GNOME_VFS_JOB_PTHREAD_H

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"
#include <semaphore.h>

typedef struct GnomeVFSJob GnomeVFSJob;

#define GNOME_VFS_JOB_DEBUG 0

#if GNOME_VFS_JOB_DEBUG

#include <stdio.h>

extern GStaticMutex debug_mutex;

#define JOB_DEBUG_PRINT(x)			\
G_STMT_START{					\
	g_static_mutex_lock (&debug_mutex);	\
	printf ("%d ", __LINE__);		\
	fputs (__FUNCTION__ ": ", stdout);	\
	printf x;				\
	fputc ('\n', stdout);			\
	fflush (stdout);			\
	g_static_mutex_unlock (&debug_mutex);	\
}G_STMT_END

#endif

#if GNOME_VFS_JOB_DEBUG

#define JOB_DEBUG(x) JOB_DEBUG_PRINT(x)
#define JOB_DEBUG_ONLY(x) x

#else
#define JOB_DEBUG(x)
#define JOB_DEBUG_ONLY(x)

#endif

enum GnomeVFSOpType {
	GNOME_VFS_OP_OPEN,
	GNOME_VFS_OP_OPEN_AS_CHANNEL,
	GNOME_VFS_OP_CREATE,
	GNOME_VFS_OP_CREATE_SYMBOLIC_LINK,
	GNOME_VFS_OP_CREATE_AS_CHANNEL,
	GNOME_VFS_OP_CLOSE,
	GNOME_VFS_OP_READ,
	GNOME_VFS_OP_WRITE,
	GNOME_VFS_OP_READ_WRITE_DONE,
	GNOME_VFS_OP_LOAD_DIRECTORY,
	GNOME_VFS_OP_FIND_DIRECTORY,
	GNOME_VFS_OP_XFER,
	GNOME_VFS_OP_GET_FILE_INFO,
	GNOME_VFS_OP_SET_FILE_INFO,
	/* This is not a real OpType; its intended to mark
	 * GnomeVFSAsyncModuleCallback's in the job_callback queue
	 */
	GNOME_VFS_OP_MODULE_CALLBACK
};

typedef enum GnomeVFSOpType GnomeVFSOpType;

typedef struct {
	GnomeVFSURI *uri;
	GnomeVFSOpenMode open_mode;
} GnomeVFSOpenOp;

typedef struct {
	GnomeVFSAsyncOpenCallback callback;
	void *callback_data;
	GnomeVFSResult result;
} GnomeVFSOpenOpResult;

typedef struct {
	GnomeVFSURI *uri;
	GnomeVFSOpenMode open_mode;
	guint advised_block_size;
} GnomeVFSOpenAsChannelOp;

typedef struct {
	GnomeVFSAsyncOpenAsChannelCallback callback;
	void *callback_data;
	GnomeVFSResult result;
	GIOChannel *channel;
} GnomeVFSOpenAsChannelOpResult;

typedef struct {
	GnomeVFSURI *uri;
	GnomeVFSOpenMode open_mode;
	gboolean exclusive;
	guint perm;
} GnomeVFSCreateOp;

typedef struct {
	GnomeVFSAsyncCreateCallback callback;
	void *callback_data;
	GnomeVFSResult result;
} GnomeVFSCreateOpResult;

typedef struct {
	GnomeVFSURI *uri;
	char *uri_reference;
} GnomeVFSCreateLinkOp;

typedef struct {
	GnomeVFSURI *uri;
	GnomeVFSOpenMode open_mode;
	gboolean exclusive;
	guint perm;
} GnomeVFSCreateAsChannelOp;

typedef struct {
	GnomeVFSAsyncCreateAsChannelCallback callback;
	void *callback_data;
	GnomeVFSResult result;
	GIOChannel *channel;
} GnomeVFSCreateAsChannelOpResult;

typedef struct {
	char dummy; /* ANSI C does not allow empty structs */
} GnomeVFSCloseOp;

typedef struct {
	GnomeVFSAsyncCloseCallback callback;
	void *callback_data;
	GnomeVFSResult result;
} GnomeVFSCloseOpResult;

typedef struct {
	GnomeVFSFileSize num_bytes;
	gpointer buffer;
} GnomeVFSReadOp;

typedef struct {
	GnomeVFSAsyncReadCallback callback;
	void *callback_data;
	GnomeVFSFileSize num_bytes;
	gpointer buffer;
	GnomeVFSResult result;
	GnomeVFSFileSize bytes_read;
} GnomeVFSReadOpResult;

typedef struct {
	GnomeVFSFileSize num_bytes;
	gconstpointer buffer;
} GnomeVFSWriteOp;

typedef struct {
	GnomeVFSAsyncWriteCallback callback;
	void *callback_data;
	GnomeVFSFileSize num_bytes;
	gconstpointer buffer;
	GnomeVFSResult result;
	GnomeVFSFileSize bytes_written;
} GnomeVFSWriteOpResult;

typedef struct {
	GList *uris; /* GnomeVFSURI* */
	GnomeVFSFileInfoOptions options;
} GnomeVFSGetFileInfoOp;

typedef struct {
	GnomeVFSAsyncGetFileInfoCallback callback;
	void *callback_data;
	GList *result_list; /* GnomeVFSGetFileInfoResult* */
} GnomeVFSGetFileInfoOpResult;

typedef struct {
	GnomeVFSURI *uri;
	GnomeVFSFileInfo *info;
	GnomeVFSSetFileInfoMask mask;
	GnomeVFSFileInfoOptions options;
} GnomeVFSSetFileInfoOp;

typedef struct {
	GnomeVFSAsyncSetFileInfoCallback callback;
	void *callback_data;
	GnomeVFSResult set_file_info_result;
	GnomeVFSResult get_file_info_result;
	GnomeVFSFileInfo *info;
} GnomeVFSSetFileInfoOpResult;

typedef struct {
	GList *uris; /* GnomeVFSURI* */
	GnomeVFSFindDirectoryKind kind;
	gboolean create_if_needed;
	gboolean find_if_needed;
	guint permissions;
} GnomeVFSFindDirectoryOp;

typedef struct {
	GnomeVFSAsyncFindDirectoryCallback callback;
	void *callback_data;
	GList *result_list; /* GnomeVFSFindDirectoryResult */
} GnomeVFSFindDirectoryOpResult;

typedef struct {
	GnomeVFSURI *uri;
	GnomeVFSFileInfoOptions options;
	GnomeVFSDirectoryFilterType filter_type;
	GnomeVFSDirectoryFilterOptions filter_options;
	gchar *filter_pattern;
	guint items_per_notification;
} GnomeVFSLoadDirectoryOp;

typedef struct {
	GnomeVFSAsyncDirectoryLoadCallback callback;
	void *callback_data;
	GnomeVFSResult result;
	GList *list;
	guint entries_read;
} GnomeVFSLoadDirectoryOpResult;

typedef struct {
	GList *source_uri_list;
	GList *target_uri_list;
	GnomeVFSXferOptions xfer_options;
	GnomeVFSXferErrorMode error_mode;
	GnomeVFSXferOverwriteMode overwrite_mode;
	GnomeVFSXferProgressCallback progress_sync_callback;
	gpointer sync_callback_data;
} GnomeVFSXferOp;

typedef struct {
	GnomeVFSAsyncXferProgressCallback callback;
	void *callback_data;
	GnomeVFSXferProgressInfo *progress_info;
	int reply;
} GnomeVFSXferOpResult;

typedef struct {
	GnomeVFSAsyncModuleCallback    callback;
	gpointer                       user_data;
	gconstpointer		       in;
	size_t			       in_size;
	gpointer                       out;
	size_t			       out_size;
	GnomeVFSModuleCallbackResponse response;
	gpointer                       response_data;
} GnomeVFSModuleCallbackOpResult;

typedef union {
	GnomeVFSOpenOp open;
	GnomeVFSOpenAsChannelOp open_as_channel;
	GnomeVFSCreateOp create;
	GnomeVFSCreateLinkOp create_symbolic_link;
	GnomeVFSCreateAsChannelOp create_as_channel;
	GnomeVFSCloseOp close;
	GnomeVFSReadOp read;
	GnomeVFSWriteOp write;
	GnomeVFSLoadDirectoryOp load_directory;
	GnomeVFSXferOp xfer;
	GnomeVFSGetFileInfoOp get_file_info;
	GnomeVFSSetFileInfoOp set_file_info;
	GnomeVFSFindDirectoryOp find_directory;
} GnomeVFSSpecificOp;

typedef struct {
	/* ID of the job (e.g. open, create, close...). */
	GnomeVFSOpType type;

	/* The callback for when the op is completed. */
	GFunc callback;
	gpointer callback_data;

	/* Details of the op. */
	GnomeVFSSpecificOp specifics;

	/* The context for cancelling the operation. */
	GnomeVFSContext *context;
	GnomeVFSModuleCallbackStackInfo *stack_info;
} GnomeVFSOp;

typedef union {
	GnomeVFSOpenOpResult open;
	GnomeVFSOpenAsChannelOpResult open_as_channel;
	GnomeVFSCreateOpResult create;
	GnomeVFSCreateAsChannelOpResult create_as_channel;
	GnomeVFSCloseOpResult close;
	GnomeVFSReadOpResult read;
	GnomeVFSWriteOpResult write;
	GnomeVFSGetFileInfoOpResult get_file_info;
	GnomeVFSSetFileInfoOpResult set_file_info;
	GnomeVFSFindDirectoryOpResult find_directory;
	GnomeVFSLoadDirectoryOpResult load_directory;
	GnomeVFSXferOpResult xfer;
	GnomeVFSModuleCallbackOpResult callback;
} GnomeVFSSpecificNotifyResult;

typedef struct {
	GnomeVFSAsyncHandle *job_handle;

	guint callback_id;

	/* By the time the callback got reached the job might have been cancelled.
	 * We find out by checking this flag.
	 */
	gboolean cancelled;
	
	/* ID of the job (e.g. open, create, close...). */
	GnomeVFSOpType type;

	GnomeVFSSpecificNotifyResult specifics;
} GnomeVFSNotifyResult;

/* FIXME bugzilla.eazel.com 1135: Move private stuff out of the header.  */
struct GnomeVFSJob {
	/* Handle being used for file access.  */
	GnomeVFSHandle *handle;

	/* By the time the entry routine for the job got reached the job might have been cancelled.
	 * We find out by checking this flag.
	 */
	gboolean cancelled;

	/* Read or create returned with an error - helps flagging that we do not expect a cancel */
	gboolean failed;
	
	/* Global lock for accessing data.  */
	sem_t access_lock;

	/* This condition is signalled when the master thread gets a
           notification and wants to acknowledge it.  */
	GCond *notify_ack_condition;

	/* Mutex associated with `notify_ack_condition'.  */
	GMutex *notify_ack_lock;

	/* Operations that are being done and those that are completed and
	 * ready for notification to take place.
	 */
	GnomeVFSOp *op;
	
	/* Unique identifier of this job (a uint, really) */
	GnomeVFSAsyncHandle *job_handle;
};

GnomeVFSJob 	*gnome_vfs_job_new      	  (GnomeVFSOpType  	 type,
				      		   GFunc           	 callback,
				      		   gpointer        	 callback_data);
void         	 gnome_vfs_job_destroy  	  (GnomeVFSJob     	*job);
void         	 gnome_vfs_job_set	  	  (GnomeVFSJob     	*job,
				      		   GnomeVFSOpType  	 type,
				      		   GFunc           	 callback,
				      		   gpointer        	 callback_data);
void         	 gnome_vfs_job_go       	  (GnomeVFSJob     	*job);
void     	 gnome_vfs_job_execute  	  (GnomeVFSJob     	*job);
void         	 gnome_vfs_job_module_cancel   	  (GnomeVFSJob	 	*job);
int          	 gnome_vfs_job_get_count 	  (void);

gboolean	 gnome_vfs_job_complete		  (GnomeVFSJob 		*job);

#endif /* GNOME_VFS_JOB_PTHREAD_H */
