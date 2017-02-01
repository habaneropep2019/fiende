/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-xfer.h - File transfers in the GNOME Virtual File System.

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

#ifndef GNOME_VFS_XFER_H
#define GNOME_VFS_XFER_H

#include <libgnomevfs/gnome-vfs-file-info.h>

/* Xfer options.  
 * FIXME bugzilla.eazel.com 1205:
 * Split these up into xfer options and xfer actions
 */
typedef enum {
	GNOME_VFS_XFER_DEFAULT = 0,
	GNOME_VFS_XFER_UNUSED_1 = 1 << 0,
	GNOME_VFS_XFER_FOLLOW_LINKS = 1 << 1,
	GNOME_VFS_XFER_UNUSED_2 = 1 << 2,
	GNOME_VFS_XFER_RECURSIVE = 1 << 3,
	GNOME_VFS_XFER_SAMEFS = 1 << 4,
	GNOME_VFS_XFER_DELETE_ITEMS = 1 << 5,
	GNOME_VFS_XFER_EMPTY_DIRECTORIES = 1 << 6,
	GNOME_VFS_XFER_NEW_UNIQUE_DIRECTORY = 1 << 7,
	GNOME_VFS_XFER_REMOVESOURCE = 1 << 8,
	GNOME_VFS_XFER_USE_UNIQUE_NAMES = 1 << 9,
	GNOME_VFS_XFER_LINK_ITEMS = 1 << 10
} GnomeVFSXferOptions;

/* Progress status, to be reported to the caller of the transfer operation.  */
typedef enum {
	GNOME_VFS_XFER_PROGRESS_STATUS_OK = 0,
	GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR = 1,
	GNOME_VFS_XFER_PROGRESS_STATUS_OVERWRITE = 2,
	/* during the duplicate status the progress callback is asked to
	   supply a new unique name */
	GNOME_VFS_XFER_PROGRESS_STATUS_DUPLICATE = 3
} GnomeVFSXferProgressStatus;

/* The different ways to deal with overwriting during a transfer operation.  */
typedef enum {
	/* Interrupt transferring with an error (GNOME_VFS_ERROR_FILEEXISTS).  */
	GNOME_VFS_XFER_OVERWRITE_MODE_ABORT = 0,
	/* Invoke the progress callback with a
	   `GNOME_VFS_XFER_PROGRESS_STATUS_OVERWRITE' status code. */
	GNOME_VFS_XFER_OVERWRITE_MODE_QUERY = 1,
	/* Overwrite files silently.  */
	GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE = 2,
	/* Ignore files silently.  */
	GNOME_VFS_XFER_OVERWRITE_MODE_SKIP = 3
} GnomeVFSXferOverwriteMode;

/* This defines the actions to perform before a file is being overwritten
   (i.e., these are the answers that can be given to a replace query).  */
typedef enum {
	GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT = 0,
	GNOME_VFS_XFER_OVERWRITE_ACTION_REPLACE = 1,
	GNOME_VFS_XFER_OVERWRITE_ACTION_REPLACE_ALL = 2,
	GNOME_VFS_XFER_OVERWRITE_ACTION_SKIP = 3,
	GNOME_VFS_XFER_OVERWRITE_ACTION_SKIP_ALL = 4,
} GnomeVFSXferOverwriteAction;

typedef enum {
	/* Interrupt transferring with an error (code returned is code of the
           operation that has caused the error).  */
	GNOME_VFS_XFER_ERROR_MODE_ABORT = 0,
	/* Invoke the progress callback with a
	   `GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR' status code. */
	GNOME_VFS_XFER_ERROR_MODE_QUERY = 1,
} GnomeVFSXferErrorMode;

/* This defines the possible actions to be performed after an error has
   occurred.  */
typedef enum {
	/* Interrupt operation and return `GNOME_VFS_ERROR_INTERRUPTED'.  */
	GNOME_VFS_XFER_ERROR_ACTION_ABORT = 0,
	/* Try the same operation again.  */
	GNOME_VFS_XFER_ERROR_ACTION_RETRY = 1,
	/* Skip this file and continue normally.  */
	GNOME_VFS_XFER_ERROR_ACTION_SKIP = 2
} GnomeVFSXferErrorAction;

/* This specifies the current phase in the transfer operation.  Phases whose
   comments are marked with `(*)' are always reported in "normal" (i.e. no
   error) condition; the other ones are only reported if an error happens in
   that specific phase.  */
typedef enum {
	/* Initial phase */
	GNOME_VFS_XFER_PHASE_INITIAL,
	/* Checking if destination can handle move/copy */
	GNOME_VFS_XFER_CHECKING_DESTINATION,
	/* Collecting file list */
	GNOME_VFS_XFER_PHASE_COLLECTING,
	/* File list collected (*) */
	GNOME_VFS_XFER_PHASE_READYTOGO,
	/* Opening source file for reading */
	GNOME_VFS_XFER_PHASE_OPENSOURCE,
	/* Creating target file for copy */
	GNOME_VFS_XFER_PHASE_OPENTARGET,
	/* Copying data from source to target (*) */
	GNOME_VFS_XFER_PHASE_COPYING,
	/* Moving file from source to target (*) */
	GNOME_VFS_XFER_PHASE_MOVING,
	/* Reading data from source file */
	GNOME_VFS_XFER_PHASE_READSOURCE,
	/* Writing data to target file */
	GNOME_VFS_XFER_PHASE_WRITETARGET,
	/* Closing source file */
	GNOME_VFS_XFER_PHASE_CLOSESOURCE,
	/* Closing target file */
	GNOME_VFS_XFER_PHASE_CLOSETARGET,
	/* Deleting source file */
	GNOME_VFS_XFER_PHASE_DELETESOURCE,
	/* Setting attributes on target file */
	GNOME_VFS_XFER_PHASE_SETATTRIBUTES,
	/* Go to the next file (*) */
	GNOME_VFS_XFER_PHASE_FILECOMPLETED,
	/* cleaning up after a move (removing source files, etc.) */
	GNOME_VFS_XFER_PHASE_CLEANUP,
	/* Operation finished (*) */
	GNOME_VFS_XFER_PHASE_COMPLETED,
	GNOME_VFS_XFER_NUM_PHASES
} GnomeVFSXferPhase;

/* Progress information for the transfer operation.  This is especially useful
   for interactive programs.  */
typedef struct {
	/* Progress status (see above for a description).  */
	GnomeVFSXferProgressStatus status;

	/* VFS status code.  If `status' is
           `GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR', you should look at this
           member to know what has happened.  */
	GnomeVFSResult vfs_status;

	/* Current phase in the transferring process.  */
	GnomeVFSXferPhase phase;

	/* Source URI. FIXME bugzilla.eazel.com 1206: change name? */
	gchar *source_name;

	/* Destination URI. FIXME bugzilla.eazel.com 1206: change name? */
	gchar *target_name;

	/* Index of file being copied. */
	gulong file_index;

	/* Total number of files to be copied.  */
	gulong files_total;

	/* Total number of bytes to be copied.  */
	GnomeVFSFileSize bytes_total;

	/* Total size of this file (in bytes).  */
	GnomeVFSFileSize file_size;

	/* Bytes copied for this file so far.  */
	GnomeVFSFileSize bytes_copied;

	/* Total amount of data copied from the beginning.  */
	GnomeVFSFileSize total_bytes_copied;
	
	/* Target unique name used when duplicating, etc. to avoid collisions */ 
	gchar *duplicate_name;

	/* Count used in the unique name e.g. (copy 2), etc. */
	int duplicate_count;

	gboolean top_level_item;
	/* indicates that the copied/moved/deleted item is an actual item
	 * passed in the uri list rather than one encountered by recursively
	 * traversing directories. Used by metadata copying.
	 */

} GnomeVFSXferProgressInfo;

/* This is the prototype for functions called during a transfer operation to
   report progress.  If the return value is `FALSE' (0), the operation is
   interrupted immediately: the transfer function returns with the value of
   `vfs_status' if it is different from `GNOME_VFS_OK', or with
   `GNOME_VFS_ERROR_INTERRUPTED' otherwise.  The effect of other values depend
   on the value of `info->status':

   - If the status is `GNOME_VFS_XFER_PROGRESS_STATUS_OK', the transfer
     operation is resumed normally.

   - If the status is `GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR', the return
     value is interpreted as a `GnomeVFSXferErrorAction' and operation is
     interrupted, continued or retried accordingly.

   - If the status is `GNOME_VFS_XFER_PROGRESS_STATUS_OVERWRITE', the return
     value is interpreted as a `GnomeVFSXferOverwriteAction'.  */

typedef gint (* GnomeVFSXferProgressCallback) 	(GnomeVFSXferProgressInfo *info,
						 gpointer data);

typedef struct GnomeVFSProgressCallbackState {

	/* xfer state */
	GnomeVFSXferProgressInfo *progress_info;	

	/* Callback called for every xfer operation. For async calls called 
	   in async xfer context. */
	GnomeVFSXferProgressCallback sync_callback;

	/* Callback called periodically every few hundred miliseconds
	   and whenever user interaction is needed. For async calls
	   called in the context of the async call caller. */
	GnomeVFSXferProgressCallback update_callback;

	/* User data passed to sync_callback. */
	gpointer user_data;

	/* Async job state passed to the update callback. */
	gpointer async_job_data;

	/* When will update_callback be called next. */
	gint64 next_update_callback_time;

	/* When will update_callback be called next. */
	gint64 next_text_update_callback_time;

	/* Period at which the update_callback will be called. */
	gint64 update_callback_period;
} GnomeVFSProgressCallbackState;

GnomeVFSResult gnome_vfs_xfer_uri_list    (const GList                  *source_uri_list,
					   const GList                  *target_uri_list,
					   GnomeVFSXferOptions           xfer_options,
					   GnomeVFSXferErrorMode         error_mode,
					   GnomeVFSXferOverwriteMode     overwrite_mode,
					   GnomeVFSXferProgressCallback  progress_callback,
					   gpointer                      data);
GnomeVFSResult gnome_vfs_xfer_uri         (const GnomeVFSURI            *source_uri,
					   const GnomeVFSURI            *target_uri,
					   GnomeVFSXferOptions           xfer_options,
					   GnomeVFSXferErrorMode         error_mode,
					   GnomeVFSXferOverwriteMode     overwrite_mode,
					   GnomeVFSXferProgressCallback  progress_callback,
					   gpointer                      data);
GnomeVFSResult gnome_vfs_xfer_delete_list (const GList                  *source_uri_list,
					   GnomeVFSXferErrorMode         error_mode,
					   GnomeVFSXferOptions           xfer_options,
					   GnomeVFSXferProgressCallback  progress_callback,
					   gpointer                      data);

#endif /* GNOME_VFS_XFER_H */
