/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-xfer.c - File transfers in the GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation
   Copyright (C) 2000, 2001 Eazel, Inc.

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

   Authors: 
   Ettore Perazzoli <ettore@comm2000.it> 
   Pavel Cisler <pavel@eazel.com> 
*/

/* FIXME bugzilla.eazel.com 1197: 
   Check that all the flags passed by address are set at least once by
   functions that are expected to set them.  */
/* FIXME bugzilla.eazel.com 1198: 
   There should be a concept of a `context' in the file methods that would
   allow us to set a prefix URI for all the operations.  This way we can
   greatly optimize access to "strange" file systems.  */

#include <config.h>
#include "gnome-vfs-xfer.h"

#include "gnome-vfs-private.h"
#include "gnome-vfs.h"
#include <string.h>
#include <sys/time.h>

/* Implementation of file transfers (`gnome_vfs_xfer*()' functions).  */

static GnomeVFSResult 	remove_directory 	(GnomeVFSURI *uri,
						 gboolean recursive,
						 GnomeVFSProgressCallbackState *progress,
						 GnomeVFSXferOptions xfer_options,
						 GnomeVFSXferErrorMode *error_mode,
						 gboolean *skip);


enum {
	/* size used for accounting for the expense of copying directories, 
	 * doing a move operation, etc. in a progress callback
	 * (during a move the file size is used for a regular file).
	 */
	DEFAULT_SIZE_OVERHEAD = 1024
};

/* in asynch mode the progress callback does a context switch every time
 * it gets called. We'll only call it every now and then to not loose a
 * lot of performance
 */
#define UPDATE_PERIOD ((gint64) (100 * 1000))

static gint64
system_time (void)
{
	struct timeval time_of_day;

	gettimeofday (&time_of_day, NULL);
	return (gint64) time_of_day.tv_usec + ((gint64) time_of_day.tv_sec) * 1000000;
}

static void
init_progress (GnomeVFSProgressCallbackState *progress_state,
	       GnomeVFSXferProgressInfo *progress_info)
{
	progress_info->source_name = NULL;
	progress_info->target_name = NULL;
	progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_OK;
	progress_info->vfs_status = GNOME_VFS_OK;
	progress_info->phase = GNOME_VFS_XFER_PHASE_INITIAL;
	progress_info->file_index = 0;
	progress_info->files_total = 0;
	progress_info->bytes_total = 0;
	progress_info->file_size = 0;
	progress_info->bytes_copied = 0;
	progress_info->total_bytes_copied = 0;
	progress_info->duplicate_name = NULL;
	progress_info->top_level_item = FALSE;

	progress_state->progress_info = progress_info;
	progress_state->sync_callback = NULL;
	progress_state->update_callback = NULL;
	progress_state->async_job_data = NULL;
	progress_state->next_update_callback_time = 0;
	progress_state->next_text_update_callback_time = 0;
	progress_state->update_callback_period = UPDATE_PERIOD;
}

static void
free_progress (GnomeVFSXferProgressInfo *progress_info)
{
	g_free (progress_info->source_name);
	progress_info->source_name = NULL;
	g_free (progress_info->target_name);
	progress_info->target_name = NULL;
}

static void
progress_set_source_target_uris (GnomeVFSProgressCallbackState *progress, 
	      const GnomeVFSURI *source_uri, const GnomeVFSURI *dest_uri)
{
	g_free (progress->progress_info->source_name);
	progress->progress_info->source_name = source_uri ? gnome_vfs_uri_to_string (source_uri,
						       GNOME_VFS_URI_HIDE_NONE) : NULL;
	g_free (progress->progress_info->target_name);
	progress->progress_info->target_name = dest_uri ? gnome_vfs_uri_to_string (dest_uri,
						       GNOME_VFS_URI_HIDE_NONE) : NULL;

}

static void
progress_set_source_target (GnomeVFSProgressCallbackState *progress, 
	      const char *source, const char *dest)
{
	g_free (progress->progress_info->source_name);
	progress->progress_info->source_name = source ? g_strdup (source) : NULL;
	g_free (progress->progress_info->target_name);
	progress->progress_info->target_name = dest ? g_strdup (dest) : NULL;

}

static int
call_progress (GnomeVFSProgressCallbackState *progress, GnomeVFSXferPhase phase)
{
	int result;

	/* FIXME bugzilla.eazel.com 1199: should use proper progress result values from an enum here */

	result = 0;
	progress_set_source_target_uris (progress, NULL, NULL);

	progress->next_update_callback_time = system_time () + progress->update_callback_period;
	
	progress->progress_info->phase = phase;

	if (progress->sync_callback != NULL)
		result = (* progress->sync_callback) (progress->progress_info, progress->user_data);

	if (progress->update_callback != NULL) {
		result = (* progress->update_callback) (progress->progress_info, 
			progress->async_job_data);
	}

	return result;	
}

static GnomeVFSXferErrorAction
call_progress_with_current_names (GnomeVFSProgressCallbackState *progress, GnomeVFSXferPhase phase)
{
	int result;

	result = GNOME_VFS_XFER_ERROR_ACTION_ABORT;

	progress->next_update_callback_time = system_time () + progress->update_callback_period;
	progress->next_update_callback_time = progress->next_text_update_callback_time;
	
	progress->progress_info->phase = phase;

	if (progress->sync_callback != NULL) {
		result = (* progress->sync_callback) (progress->progress_info, progress->user_data);
	}
	
	if (progress->update_callback != NULL) {
		result = (* progress->update_callback) (progress->progress_info, 
			progress->async_job_data);
	}

	return result;	
}

static int
call_progress_uri (GnomeVFSProgressCallbackState *progress, 
		   const GnomeVFSURI *source_uri, const GnomeVFSURI *dest_uri, 
		   GnomeVFSXferPhase phase)
{
	int result;

	result = 0;
	progress_set_source_target_uris (progress, source_uri, dest_uri);

	progress->next_text_update_callback_time = system_time () + progress->update_callback_period;
	progress->next_update_callback_time = progress->next_text_update_callback_time;
	
	progress->progress_info->phase = phase;

	if (progress->sync_callback != NULL) {
		result = (* progress->sync_callback) (progress->progress_info, progress->user_data);
	}
	
	if (progress->update_callback != NULL) {
		result = (* progress->update_callback) (progress->progress_info,
			progress->async_job_data);
	}
	
	return result;	
}

static gboolean
at_end (GnomeVFSProgressCallbackState *progress)
{
	return progress->progress_info->total_bytes_copied
		>= progress->progress_info->bytes_total;
}

static int
call_progress_often_internal (GnomeVFSProgressCallbackState *progress,
			      GnomeVFSXferPhase phase,
			      gint64 *next_time)
{
	int result;
	gint64 now;

	result = 1;
	now = system_time ();

	progress->progress_info->phase = phase;

	if (progress->sync_callback != NULL) {
		result = (* progress->sync_callback) (progress->progress_info, progress->user_data);
	}

	if (now < *next_time && !at_end (progress)) {
		return result;
	}

	*next_time = now + progress->update_callback_period;
	if (progress->update_callback != NULL) {
		result = (* progress->update_callback) (progress->progress_info, progress->async_job_data);
	}

	return result;
}

static int
call_progress_often (GnomeVFSProgressCallbackState *progress,
		     GnomeVFSXferPhase phase)
{
	return call_progress_often_internal
		(progress, phase, &progress->next_update_callback_time);
}

static int
call_progress_with_uris_often (GnomeVFSProgressCallbackState *progress, 
			       const GnomeVFSURI *source_uri, const GnomeVFSURI *dest_uri, 
			       GnomeVFSXferPhase phase)
{
	progress_set_source_target_uris (progress, source_uri, dest_uri);
	return call_progress_often_internal
		(progress, phase, &progress->next_text_update_callback_time);
}

/* Handle an error condition according to `error_mode'.  Returns `TRUE' if the
 * operation should be retried, `FALSE' otherwise.  `*result' will be set to
 * the result value we want to be returned to the caller of the xfer
 * function.
 */
static gboolean
handle_error (GnomeVFSResult *result,
	      GnomeVFSProgressCallbackState *progress,
	      GnomeVFSXferErrorMode *error_mode,
	      gboolean *skip)
{
	GnomeVFSXferErrorAction action;

	*skip = FALSE;

	switch (*error_mode) {
	case GNOME_VFS_XFER_ERROR_MODE_ABORT:
		return FALSE;

	case GNOME_VFS_XFER_ERROR_MODE_QUERY:
		progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_VFSERROR;
		progress->progress_info->vfs_status = *result;
		action = call_progress_with_current_names (progress, progress->progress_info->phase);
		progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_OK;

		switch (action) {
		case GNOME_VFS_XFER_ERROR_ACTION_RETRY:
			return TRUE;
		case GNOME_VFS_XFER_ERROR_ACTION_ABORT:
			*result = GNOME_VFS_ERROR_INTERRUPTED;
			return FALSE;
		case GNOME_VFS_XFER_ERROR_ACTION_SKIP:
			*result = GNOME_VFS_OK;
			*skip = TRUE;
			return FALSE;
		}
		break;
	}

	*skip = FALSE;
	return FALSE;
}

/* This is conceptually similiar to the previous `handle_error()' function, but
 * handles the overwrite case. 
 */
static gboolean
handle_overwrite (GnomeVFSResult *result,
		  GnomeVFSProgressCallbackState *progress,
		  GnomeVFSXferErrorMode *error_mode,
		  GnomeVFSXferOverwriteMode *overwrite_mode,
		  gboolean *replace,
		  gboolean *skip)
{
	GnomeVFSXferOverwriteAction action;

	switch (*overwrite_mode) {
	case GNOME_VFS_XFER_OVERWRITE_MODE_ABORT:
		*replace = FALSE;
		*result = GNOME_VFS_ERROR_FILE_EXISTS;
		*skip = FALSE;
		return FALSE;
	case GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE:
		*replace = TRUE;
		*skip = FALSE;
		return TRUE;
	case GNOME_VFS_XFER_OVERWRITE_MODE_SKIP:
		*replace = FALSE;
		*skip = TRUE;
		return FALSE;
	case GNOME_VFS_XFER_OVERWRITE_MODE_QUERY:
		progress->progress_info->vfs_status = *result;
		progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_OVERWRITE;
		action = call_progress_with_current_names (progress, progress->progress_info->phase);
		progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_OK;

		switch (action) {
		case GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT:
			*replace = FALSE;
			*result = GNOME_VFS_ERROR_FILE_EXISTS;
			*skip = FALSE;
			return FALSE;
		case GNOME_VFS_XFER_OVERWRITE_ACTION_REPLACE:
			*replace = TRUE;
			*skip = FALSE;
			return TRUE;
		case GNOME_VFS_XFER_OVERWRITE_ACTION_REPLACE_ALL:
			*replace = TRUE;
			*overwrite_mode = GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE;
			*skip = FALSE;
			return TRUE;
		case GNOME_VFS_XFER_OVERWRITE_ACTION_SKIP:
			*replace = FALSE;
			*skip = TRUE;
			return FALSE;
		case GNOME_VFS_XFER_OVERWRITE_ACTION_SKIP_ALL:
			*replace = FALSE;
			*skip = TRUE;
			*overwrite_mode = GNOME_VFS_XFER_OVERWRITE_MODE_SKIP;
			return FALSE;
		}
	}

	*replace = FALSE;
	*skip = FALSE;
	return FALSE;
}

static GnomeVFSResult
remove_file (GnomeVFSURI *uri,
	     GnomeVFSProgressCallbackState *progress,
	     GnomeVFSXferOptions xfer_options,
	     GnomeVFSXferErrorMode *error_mode,
	     gboolean *skip)
{
	GnomeVFSResult result;
	gboolean retry;

	progress->progress_info->file_index++;
	do {
		retry = FALSE;
		result = gnome_vfs_unlink_from_uri (uri);		
		if (result != GNOME_VFS_OK) {
			retry = handle_error (&result, progress,
					      error_mode, skip);
		} else {
			/* add some small size for each deleted item because delete overhead
			 * does not depend on file/directory size
			 */
			progress->progress_info->total_bytes_copied += DEFAULT_SIZE_OVERHEAD;

			if (call_progress_with_uris_often (progress, uri, NULL, 
							   GNOME_VFS_XFER_PHASE_DELETESOURCE) 
				== GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT) {
				result = GNOME_VFS_ERROR_INTERRUPTED;
				break;
			}
		}


	} while (retry);

	return result;
}

static GnomeVFSResult
empty_directory (GnomeVFSURI *uri,
		 GnomeVFSProgressCallbackState *progress,
		 GnomeVFSXferOptions xfer_options,
		 GnomeVFSXferErrorMode *error_mode,
		 gboolean *skip)
{
	GnomeVFSResult result;
	GnomeVFSDirectoryHandle *directory_handle;
	gboolean retry;


	*skip = FALSE;
	do {
		result = gnome_vfs_directory_open_from_uri (&directory_handle, uri, 
							    GNOME_VFS_FILE_INFO_DEFAULT, 
							    NULL);
		retry = FALSE;
		if (result != GNOME_VFS_OK) {
			retry = handle_error (&result, progress,
					      error_mode, skip);
		}
	} while (retry);

	if (result != GNOME_VFS_OK || *skip) {
		return result;
	}
	
	for (;;) {
		GnomeVFSFileInfo *info;
		GnomeVFSURI *item_uri;
		
		item_uri = NULL;
		info = gnome_vfs_file_info_new ();
		result = gnome_vfs_directory_read_next (directory_handle, info);
		if (result != GNOME_VFS_OK) {
			gnome_vfs_file_info_unref (info);
			break;
		}

		/* Skip "." and "..".  */
		if (strcmp (info->name, ".") != 0 && strcmp (info->name, "..") != 0) {

			item_uri = gnome_vfs_uri_append_file_name (uri, info->name);
			
			progress_set_source_target_uris (progress, item_uri, NULL);
			
			if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
				result = remove_directory (item_uri, TRUE, 
							   progress, xfer_options, error_mode, 
							   skip);
			} else {
				result = remove_file (item_uri, progress, 
						      xfer_options,
						      error_mode,
						      skip);
			}

		}

		gnome_vfs_file_info_unref (info);
		
		if (item_uri != NULL) {
			gnome_vfs_uri_unref (item_uri);
		}
		
		if (result != GNOME_VFS_OK) {
			break;
		}
	}
	gnome_vfs_directory_close (directory_handle);

	if (result == GNOME_VFS_ERROR_EOF) {
		result = GNOME_VFS_OK;
	}

	return result;
}

typedef struct {
	const GnomeVFSURI *base_uri;
	GList *uri_list;
} PrependOneURIParams;

static gboolean
PrependOneURIToList (const gchar *rel_path, GnomeVFSFileInfo *info,
	gboolean recursing_will_loop, gpointer cast_to_params, gboolean *recurse)
{
	PrependOneURIParams *params;

	params = (PrependOneURIParams *)cast_to_params;
	params->uri_list = g_list_prepend (params->uri_list, gnome_vfs_uri_append_string (
		params->base_uri, rel_path));

	if (recursing_will_loop) {
		return FALSE;
	}
	*recurse = TRUE;
	return TRUE;
}

static GnomeVFSResult
non_recursive_empty_directory (GnomeVFSURI *directory_uri,
			       GnomeVFSProgressCallbackState *progress,
			       GnomeVFSXferOptions xfer_options,
			       GnomeVFSXferErrorMode *error_mode,
			       gboolean *skip)
{
	/* Used as a fallback when we run out of file descriptors during 
	 * a deep recursive deletion. 
	 * We instead compile a flat list of URIs, doing a recursion that does not
	 * keep directories open.
	 */

	GList *uri_list;
	GList *p;
	GnomeVFSURI *uri;
	GnomeVFSResult result;
	GnomeVFSFileInfo *info;
	PrependOneURIParams visit_params;

	/* Build up the list so that deep items appear before their parents
	 * so that we can delete directories, children first.
	 */
	visit_params.base_uri = directory_uri;
	visit_params.uri_list = NULL;
	result = gnome_vfs_directory_visit_uri (directory_uri, GNOME_VFS_FILE_INFO_DEFAULT, 
		NULL, GNOME_VFS_DIRECTORY_VISIT_SAMEFS | GNOME_VFS_DIRECTORY_VISIT_LOOPCHECK,
		PrependOneURIToList, &visit_params);

	uri_list = visit_params.uri_list;

	if (result == GNOME_VFS_OK) {
		for (p = uri_list; p != NULL; p = p->next) {

			uri = (GnomeVFSURI *)p->data;
			info = gnome_vfs_file_info_new ();
			result = gnome_vfs_get_file_info_uri (uri, info, GNOME_VFS_FILE_INFO_DEFAULT);
			progress_set_source_target_uris (progress, uri, NULL);
			if (result == GNOME_VFS_OK) {
				if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
					result = remove_directory (uri, FALSE, 
						progress, xfer_options, error_mode, skip);
				} else {
					result = remove_file (uri, progress, 
						xfer_options, error_mode, skip);
				}
			}
			gnome_vfs_file_info_unref (info);
		}
	}

	gnome_vfs_uri_list_free (uri_list);

	return result;
}

static GnomeVFSResult
remove_directory (GnomeVFSURI *uri,
		  gboolean recursive,
		  GnomeVFSProgressCallbackState *progress,
		  GnomeVFSXferOptions xfer_options,
		  GnomeVFSXferErrorMode *error_mode,
		  gboolean *skip)
{
	GnomeVFSResult result;
	gboolean retry;

	result = GNOME_VFS_OK;

	if (recursive) {
		result = empty_directory (uri, progress, xfer_options, error_mode, skip);
		if (result == GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES) {
			result = non_recursive_empty_directory (uri, progress, xfer_options,
				error_mode, skip);
		}
	}

	if (result == GNOME_VFS_ERROR_EOF) {
		result = GNOME_VFS_OK;
	}

	if (result == GNOME_VFS_OK) {
		progress->progress_info->file_index++;

		do {
			retry = FALSE;

			result = gnome_vfs_remove_directory_from_uri (uri);
			if (result != GNOME_VFS_OK) {
				retry = handle_error (&result, progress,
						      error_mode, skip);
			} else {
				/* add some small size for each deleted item */
				progress->progress_info->total_bytes_copied += DEFAULT_SIZE_OVERHEAD;

				if (call_progress_with_uris_often (progress, uri, NULL, 
					GNOME_VFS_XFER_PHASE_DELETESOURCE) 
					== GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT) {
					result = GNOME_VFS_ERROR_INTERRUPTED;
					break;
				}
			}

		} while (retry);
	}

	return result;
}

/* iterates the list of names in a given directory, applies @callback on each,
 * optionally recurses into directories
 */
static GnomeVFSResult
gnome_vfs_visit_list (const GList *name_uri_list,
		      GnomeVFSFileInfoOptions info_options,
		      GnomeVFSDirectoryVisitOptions visit_options,
		      gboolean recursive,
		      GnomeVFSDirectoryVisitFunc callback,
		      gpointer data)
{
	GnomeVFSResult result;
	const GList *p;
	GnomeVFSURI *uri;
	GnomeVFSFileInfo *info;
	gboolean tmp_recurse;
	
	result = GNOME_VFS_OK;

	/* go through our list of items */
	for (p = name_uri_list; p != NULL; p = p->next) {
		
		/* get the URI and VFSFileInfo for each */
		uri = (GnomeVFSURI *)p->data;
		info = gnome_vfs_file_info_new ();
		result = gnome_vfs_get_file_info_uri (uri, info, info_options);
		
		if (result == GNOME_VFS_OK) {
			tmp_recurse = TRUE;
			
			/* call our callback on each item*/
			if (!callback (gnome_vfs_uri_get_path (uri), info, FALSE, data, &tmp_recurse)) {
				result = GNOME_VFS_ERROR_INTERRUPTED;
			}
			
			if (result == GNOME_VFS_OK
			    && recursive
			    && info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
				/* let gnome_vfs_directory_visit_uri call our callback 
				 * recursively 
				 */
				result = gnome_vfs_directory_visit_uri (uri, info_options, 
					NULL, visit_options, callback, data);
			}
		}
		gnome_vfs_file_info_unref (info);
		
		if (result != GNOME_VFS_OK) {
			break;
		}
	}
	return result;
}

typedef struct {
	GnomeVFSProgressCallbackState *progress;
	GnomeVFSResult result;
} CountEachFileSizeParams;

/* iteratee for count_items_and_size */
static gboolean
count_each_file_size_one (const gchar *rel_path,
			  GnomeVFSFileInfo *info,
			  gboolean recursing_will_loop,
			  gpointer data,
			  gboolean *recurse)
{
	CountEachFileSizeParams *params;

	params = (CountEachFileSizeParams *)data;

	if (call_progress_often (params->progress, params->progress->progress_info->phase) == 0) {
		/* progress callback requested to stop */
		params->result = GNOME_VFS_ERROR_INTERRUPTED;
		*recurse = FALSE;
		return FALSE;
	}

	/* keep track of the item we are counting so we can correctly report errors */
	progress_set_source_target (params->progress, rel_path, NULL);

	/* count each file, folder, symlink */
	params->progress->progress_info->files_total++;
	if (info->type == GNOME_VFS_FILE_TYPE_REGULAR) {
		/* add each file size */
		params->progress->progress_info->bytes_total += info->size + DEFAULT_SIZE_OVERHEAD;
	} else if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
		/* add some small size for each directory */
		params->progress->progress_info->bytes_total += DEFAULT_SIZE_OVERHEAD;
	}

	/* watch out for infinite recursion*/
	if (recursing_will_loop) {
		params->result = GNOME_VFS_ERROR_LOOP;
		return FALSE;
	}

	*recurse = TRUE;

	return TRUE;
}

/* calculate the number of items and their total size; used as a preflight
 * before the transfer operation starts
 */
static GnomeVFSResult
count_items_and_size (const GList *name_uri_list,
		      GnomeVFSXferOptions xfer_options,
		      GnomeVFSProgressCallbackState *progress,
		      gboolean move,
		      gboolean link)
{
	/*
	 * FIXME bugzilla.eazel.com 1200:
	 * Deal with errors here, respond to skip by pulling items from the name list
	 */
	GnomeVFSFileInfoOptions info_options;
	GnomeVFSDirectoryVisitOptions visit_options;
	CountEachFileSizeParams each_params;

 	/* initialize the results */
	progress->progress_info->files_total = 0;
	progress->progress_info->bytes_total = 0;

	/* set up the params for recursion */
	visit_options = GNOME_VFS_DIRECTORY_VISIT_LOOPCHECK;
	if (xfer_options & GNOME_VFS_XFER_SAMEFS)
		visit_options |= GNOME_VFS_DIRECTORY_VISIT_SAMEFS;
	each_params.progress = progress;
	each_params.result = GNOME_VFS_OK;

	if (xfer_options & GNOME_VFS_XFER_FOLLOW_LINKS) {
		info_options = GNOME_VFS_FILE_INFO_FOLLOW_LINKS;
	} else {
		info_options = GNOME_VFS_FILE_INFO_DEFAULT;
	}

	return gnome_vfs_visit_list (name_uri_list, info_options,
		visit_options, !link && !move && (xfer_options & GNOME_VFS_XFER_RECURSIVE) != 0,
		count_each_file_size_one, &each_params);
}

/* calculate the number of items and their total size; used as a preflight
 * before the transfer operation starts
 */
static GnomeVFSResult
directory_add_items_and_size (GnomeVFSURI *dir_uri,
			      GnomeVFSXferOptions xfer_options,
			      GnomeVFSProgressCallbackState *progress)
{
	GnomeVFSFileInfoOptions info_options;
	GnomeVFSDirectoryVisitOptions visit_options;
	CountEachFileSizeParams each_params;

	/* set up the params for recursion */
	visit_options = GNOME_VFS_DIRECTORY_VISIT_LOOPCHECK;
	if (xfer_options & GNOME_VFS_XFER_SAMEFS)
		visit_options |= GNOME_VFS_DIRECTORY_VISIT_SAMEFS;
	each_params.progress = progress;
	each_params.result = GNOME_VFS_OK;

	if (xfer_options & GNOME_VFS_XFER_FOLLOW_LINKS) {
		info_options = GNOME_VFS_FILE_INFO_FOLLOW_LINKS;
	} else {
		info_options = GNOME_VFS_FILE_INFO_DEFAULT;
	}

	return gnome_vfs_directory_visit_uri (dir_uri, info_options, NULL, 
		visit_options, count_each_file_size_one, &each_params);

}

/* Compares the list of files about to be created by a transfer with
 * any possible existing files with conflicting names in the target directory.
 * Handles conflicts, optionaly removing the conflicting file/directory
 */
static GnomeVFSResult
handle_name_conflicts (GList **source_uri_list,
		       GList **target_uri_list,
		       GnomeVFSXferOptions xfer_options,
		       GnomeVFSXferErrorMode *error_mode,
		       GnomeVFSXferOverwriteMode *overwrite_mode,
		       GnomeVFSProgressCallbackState *progress)
{
	GnomeVFSResult result;
	GList *source_item;
	GList *target_item;

	int conflict_count; /* values are 0, 1, many */
	
	result = GNOME_VFS_OK;
	conflict_count = 0;

	/* Go through the list of names, find out if there is 0, 1 or more conflicts. */
	for (target_item = *target_uri_list; target_item != NULL;
	     target_item = target_item->next) {
	     	if (gnome_vfs_uri_exists ((GnomeVFSURI *)target_item->data)) {
			conflict_count++;
			if (conflict_count > 1) {
				break;
			}
	     	}
	}

	if (conflict_count == 0) {
		/* No conflicts, we are done. */
		return GNOME_VFS_OK;
	}

	/* Pass in the conflict count so that we can decide to present the Replace All
	 * for multiple conflicts.
	 */
	progress->progress_info->duplicate_count = conflict_count;

	
	/* Go through the list of names again, present overwrite alerts for each. */
	for (target_item = *target_uri_list, source_item = *source_uri_list; 
	     target_item != NULL;) {
		gboolean replace;
		gboolean skip;
		gboolean is_move_to_self;
		GnomeVFSURI *uri, *source_uri;
		GnomeVFSFileInfo *info;
		
		skip = FALSE;
		source_uri = (GnomeVFSURI *)source_item->data;
		uri = (GnomeVFSURI *)target_item->data;
		is_move_to_self = (xfer_options & GNOME_VFS_XFER_REMOVESOURCE) != 0
			&& gnome_vfs_uri_equal (source_uri, uri);
		if (!is_move_to_self && gnome_vfs_uri_exists (uri)) {
			progress_set_source_target_uris (progress, source_uri, uri);
			 
			/* no error getting info -- file exists, ask what to do */
			replace = handle_overwrite (&result, progress, error_mode,
				overwrite_mode, &replace, &skip);

			/* FIXME bugzilla.eazel.com 1207:
			 * move items to Trash here
			 */

			/* get rid of the conflicting file */
			if (replace) {
				info = gnome_vfs_file_info_new ();
				gnome_vfs_get_file_info_uri (uri, info, GNOME_VFS_FILE_INFO_DEFAULT);
				progress_set_source_target_uris (progress, uri, NULL);
				if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
					remove_directory (uri, TRUE, progress, 
						xfer_options, error_mode, &skip);
				} else {
					remove_file (uri, progress,
						xfer_options, error_mode, &skip);
				}
				gnome_vfs_file_info_unref (info);
			}
		}

		
		if (result != GNOME_VFS_OK) {
			break;
		}

		if (skip) {
			/* skipping a file, remove it's name from the source and target name
			 * lists.
			 */
			GList *source_item_to_remove;
			GList *target_item_to_remove;

			source_item_to_remove = source_item;
			target_item_to_remove = target_item;
			
			source_item = source_item->next;
			target_item = target_item->next;

			gnome_vfs_uri_unref ((GnomeVFSURI *)source_item_to_remove->data);
			gnome_vfs_uri_unref ((GnomeVFSURI *)target_item_to_remove->data);
			*source_uri_list = g_list_remove_link (*source_uri_list, source_item_to_remove);
			*target_uri_list = g_list_remove_link (*target_uri_list, target_item_to_remove);
			
			continue;
		}


		target_item = target_item->next; 
		source_item = source_item->next;
	}

	return result;
}

/* Create new directory. If GNOME_VFS_XFER_USE_UNIQUE_NAMES is set, 
 * return with an error if a name conflict occurs, else
 * handle the overwrite.
 * On success, opens the new directory
 */
static GnomeVFSResult
create_directory (GnomeVFSURI *dir_uri,
		  GnomeVFSDirectoryHandle **return_handle,
		  GnomeVFSXferOptions xfer_options,
		  GnomeVFSXferErrorMode *error_mode,
		  GnomeVFSXferOverwriteMode *overwrite_mode,
		  GnomeVFSProgressCallbackState *progress,
		  gboolean *skip)
{
	GnomeVFSResult result;
	gboolean retry;
	
	*skip = FALSE;
	do {
		retry = FALSE;

		result = gnome_vfs_make_directory_for_uri (dir_uri, 0777);

		if (result == GNOME_VFS_ERROR_FILE_EXISTS) {
			gboolean force_replace;

			if ((xfer_options & GNOME_VFS_XFER_USE_UNIQUE_NAMES) != 0) {
				/* just let the caller pass a unique name*/
				return result;
			}

			retry = handle_overwrite (&result,
						  progress,
						  error_mode,
						  overwrite_mode,
						  &force_replace,
						  skip);

			if (*skip) {
				return GNOME_VFS_OK;
			}
			if (force_replace) {
				result = remove_directory (dir_uri, TRUE, progress, 
							   xfer_options, error_mode, 
							   skip);
			} else {
				result = GNOME_VFS_OK;
			}
		}

		if (result == GNOME_VFS_OK) {
			return gnome_vfs_directory_open_from_uri (return_handle, 
								  dir_uri, 
								  GNOME_VFS_FILE_INFO_DEFAULT, 
								  NULL);
		}
		/* handle the error case */
		retry = handle_error (&result, progress,
				      error_mode, skip);

		if (*skip) {
			return GNOME_VFS_OK;
		}

	} while (retry);

	return result;
}

/* Copy the data of a single file. */
static GnomeVFSResult
copy_file_data (GnomeVFSHandle *target_handle,
		GnomeVFSHandle *source_handle,
		GnomeVFSProgressCallbackState *progress,
		GnomeVFSXferOptions xfer_options,
		GnomeVFSXferErrorMode *error_mode,
		guint block_size,
		gboolean *skip)
{
	GnomeVFSResult result;
	gpointer buffer;
	const char *write_buffer;
	GnomeVFSFileSize total_bytes_read;

	*skip = FALSE;

	if (call_progress_often (progress, GNOME_VFS_XFER_PHASE_COPYING) == 0) {
		return GNOME_VFS_ERROR_INTERRUPTED;
	}

	buffer = g_malloc (block_size);
	total_bytes_read = 0;
	
	do {
		GnomeVFSFileSize bytes_read;
		GnomeVFSFileSize bytes_to_write;
		GnomeVFSFileSize bytes_written;
		gboolean retry;

		progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_OK;
		progress->progress_info->vfs_status = GNOME_VFS_OK;

		progress->progress_info->phase = GNOME_VFS_XFER_PHASE_READSOURCE;

		do {
			retry = FALSE;

			result = gnome_vfs_read (source_handle, buffer,
						 block_size, &bytes_read);
			if (result != GNOME_VFS_OK && result != GNOME_VFS_ERROR_EOF)
				retry = handle_error (&result, progress,
						      error_mode, skip);
		} while (retry);

		if (result != GNOME_VFS_OK || bytes_read == 0 || *skip) {
			break;
		}

		total_bytes_read += total_bytes_read;
		bytes_to_write = bytes_read;

		progress->progress_info->phase = GNOME_VFS_XFER_PHASE_WRITETARGET;

		write_buffer = buffer;
		do {
			retry = FALSE;

			result = gnome_vfs_write (target_handle, write_buffer,
						  bytes_to_write,
						  &bytes_written);

			if (result != GNOME_VFS_OK) {
				retry = handle_error (&result, progress, error_mode, skip);
			}

			bytes_to_write -= bytes_written;
			write_buffer += bytes_written;
		} while ((result == GNOME_VFS_OK && bytes_to_write > 0) || retry);

		progress->progress_info->phase = GNOME_VFS_XFER_PHASE_COPYING;

		progress->progress_info->bytes_copied += bytes_read;
		progress->progress_info->total_bytes_copied += bytes_read;

		if (call_progress_often (progress, GNOME_VFS_XFER_PHASE_COPYING) == 0) {
			g_free (buffer);
			return GNOME_VFS_ERROR_INTERRUPTED;
		}

		if (*skip) {
			break;
		}

	} while (result == GNOME_VFS_OK);

	if (result == GNOME_VFS_ERROR_EOF) {
		result = GNOME_VFS_OK;
	}

	if (result == GNOME_VFS_OK) {
		/* tiny (0 - sized) files still present non-zero overhead during a copy, make sure
		 * we count at least a default amount for each file
		 */
		progress->progress_info->total_bytes_copied += DEFAULT_SIZE_OVERHEAD;

		call_progress_often (progress, GNOME_VFS_XFER_PHASE_COPYING);
	}

	g_free (buffer);

	return result;
}

static GnomeVFSResult
xfer_open_source (GnomeVFSHandle **source_handle,
		  GnomeVFSURI *source_uri,
		  GnomeVFSProgressCallbackState *progress,
		  GnomeVFSXferOptions xfer_options,
		  GnomeVFSXferErrorMode *error_mode,
		  gboolean *skip)
{
	GnomeVFSResult result;
	gboolean retry;

	*skip = FALSE;
	do {
		retry = FALSE;

		result = gnome_vfs_open_uri (source_handle, source_uri,
					     GNOME_VFS_OPEN_READ);

		if (result != GNOME_VFS_OK) {
			retry = handle_error (&result, progress, error_mode, skip);
		}
	} while (retry);

	return result;
}

static GnomeVFSResult
xfer_create_target (GnomeVFSHandle **target_handle,
		    GnomeVFSURI *target_uri,
		    GnomeVFSProgressCallbackState *progress,
		    GnomeVFSXferOptions xfer_options,
		    GnomeVFSXferErrorMode *error_mode,
		    GnomeVFSXferOverwriteMode *overwrite_mode,
		    gboolean *skip)
{
	GnomeVFSResult result;
	gboolean retry;
	gboolean exclusive;

	exclusive = (*overwrite_mode != GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE);

	*skip = FALSE;
	do {
		retry = FALSE;

		result = gnome_vfs_create_uri (target_handle, target_uri,
					       GNOME_VFS_OPEN_WRITE,
					       exclusive, 0666);

		if (result == GNOME_VFS_ERROR_FILE_EXISTS) {
			gboolean replace;

			retry = handle_overwrite (&result,
						  progress,
						  error_mode,
						  overwrite_mode,
						  &replace,
						  skip);

			if (replace) {
				exclusive = FALSE;
			}

		} else if (result != GNOME_VFS_OK) {
			retry = handle_error (&result,  progress, error_mode, skip);
		}
	} while (retry);

	return result;
}

static GnomeVFSResult
copy_symlink (GnomeVFSURI *source_uri,
	      GnomeVFSURI *target_uri,
	      const char *link_name,
	      GnomeVFSProgressCallbackState *progress)
{
	GnomeVFSResult result;
	
	result = gnome_vfs_create_symbolic_link (target_uri, link_name);
	
	if (result == GNOME_VFS_OK && call_progress_with_uris_often (progress, source_uri,
		target_uri, GNOME_VFS_XFER_PHASE_OPENTARGET) == 0) {
		result = GNOME_VFS_ERROR_INTERRUPTED;
	}
	return result;
}

static GnomeVFSResult
copy_file (GnomeVFSFileInfo *info,  
	   GnomeVFSURI *source_uri,
	   GnomeVFSURI *target_uri,
	   GnomeVFSXferOptions xfer_options,
	   GnomeVFSXferErrorMode *error_mode,
	   GnomeVFSXferOverwriteMode *overwrite_mode,
	   GnomeVFSProgressCallbackState *progress,
	   gboolean *skip)
{
	GnomeVFSResult close_result, result;
	GnomeVFSHandle *source_handle, *target_handle;
	GnomeVFSSetFileInfoMask set_mask;

	progress->progress_info->phase = GNOME_VFS_XFER_PHASE_OPENSOURCE;
	progress->progress_info->bytes_copied = 0;
	result = xfer_open_source (&source_handle, source_uri,
				   progress, xfer_options,
				   error_mode, skip);
	if (*skip) {
		return GNOME_VFS_OK;
	}
	
	if (result != GNOME_VFS_OK) {
		return result;
	}

	progress->progress_info->phase = GNOME_VFS_XFER_PHASE_OPENTARGET;
	result = xfer_create_target (&target_handle, target_uri,
				     progress, xfer_options,
				     error_mode, overwrite_mode,
				     skip);


	if (*skip) {
		gnome_vfs_close (source_handle);
		return GNOME_VFS_OK;
	}
	if (result != GNOME_VFS_OK) {
		gnome_vfs_close (source_handle);
		return result;
	}

	if (call_progress_with_uris_often (progress, source_uri, target_uri, 
		GNOME_VFS_XFER_PHASE_OPENTARGET) != GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT) {

		result = copy_file_data (target_handle, source_handle,
					progress, xfer_options, error_mode,
					/* use an arbitrary default block size of 4096 
					 * if one isn't available for this file system 
					 */
					(info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_IO_BLOCK_SIZE)
						? info->io_block_size : 4096,
					skip);
	}

	if (result == GNOME_VFS_OK 
		&& call_progress_often (progress, GNOME_VFS_XFER_PHASE_CLOSETARGET) == 0) {
		result = GNOME_VFS_ERROR_INTERRUPTED;
	}

	close_result = gnome_vfs_close (source_handle);
	if (result == GNOME_VFS_OK && close_result != GNOME_VFS_OK) {
		handle_error (&close_result, progress, error_mode, skip);
		return close_result;
	}

	close_result = gnome_vfs_close (target_handle);
	if (result == GNOME_VFS_OK && close_result != GNOME_VFS_OK) {
		handle_error (&close_result, progress, error_mode, skip);
		return close_result;
	}

	if (result == GNOME_VFS_OK) {
		/* FIXME the modules should ignore setting of permissions if
		 * "valid_fields & GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS" is clear
		 * for now, make sure permissions aren't set to 000
		 */

		if ((info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS) == 0) {
			set_mask = GNOME_VFS_SET_FILE_INFO_TIME;
		} else {
			set_mask = GNOME_VFS_SET_FILE_INFO_PERMISSIONS
					| GNOME_VFS_SET_FILE_INFO_OWNER
					| GNOME_VFS_SET_FILE_INFO_TIME;
		}

		/* ignore errors while setting file info attributes at this point */
		gnome_vfs_set_file_info_uri (target_uri, info, set_mask);
	}

	if (*skip) {
		return GNOME_VFS_OK;
	}

	return result;
}

static GnomeVFSResult
copy_directory (GnomeVFSFileInfo *source_file_info,
		GnomeVFSURI *source_dir_uri,
		GnomeVFSURI *target_dir_uri,
		GnomeVFSXferOptions xfer_options,
		GnomeVFSXferErrorMode *error_mode,
		GnomeVFSXferOverwriteMode *overwrite_mode,
		GnomeVFSProgressCallbackState *progress,
		gboolean *skip)
{
	GnomeVFSResult result;
	GnomeVFSDirectoryHandle *source_directory_handle;
	GnomeVFSDirectoryHandle *dest_directory_handle;
	GnomeVFSSetFileInfoMask set_mask;

	source_directory_handle = NULL;
	dest_directory_handle = NULL;
	
	result = gnome_vfs_directory_open_from_uri (&source_directory_handle, source_dir_uri, 
						    GNOME_VFS_FILE_INFO_DEFAULT, NULL);

	if (result != GNOME_VFS_OK) {
		return result;
	}

	progress->progress_info->bytes_copied = 0;
	if (call_progress_with_uris_often (progress, source_dir_uri, target_dir_uri, 
			       GNOME_VFS_XFER_PHASE_COPYING) 
		== GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT) {
		return GNOME_VFS_ERROR_INTERRUPTED;
	}

	result = create_directory (target_dir_uri, 
				   &dest_directory_handle,
				   xfer_options,
				   error_mode,
				   overwrite_mode,
				   progress,
				   skip);

	if (*skip) {
		gnome_vfs_directory_close (source_directory_handle);
		return GNOME_VFS_OK;
	}
	
	if (result != GNOME_VFS_OK) {
		gnome_vfs_directory_close (source_directory_handle);
		return result;
	}

	if (call_progress_with_uris_often (progress, source_dir_uri, target_dir_uri, 
					   GNOME_VFS_XFER_PHASE_OPENTARGET) != 0) {

		progress->progress_info->total_bytes_copied += DEFAULT_SIZE_OVERHEAD;
		progress->progress_info->top_level_item = FALSE;

		/* We do not deal with symlink loops here. 
		 * That's OK because we don't follow symlinks.
		 */
		do {
			GnomeVFSURI *source_uri;
			GnomeVFSURI *dest_uri;
			GnomeVFSFileInfo *info;

			source_uri = NULL;
			dest_uri = NULL;
			info = gnome_vfs_file_info_new ();

			result = gnome_vfs_directory_read_next (source_directory_handle, info);
			if (result != GNOME_VFS_OK) {
				gnome_vfs_file_info_unref (info);	
				break;
			}
			
			/* Skip "." and "..".  */
			if (strcmp (info->name, ".") != 0 && strcmp (info->name, "..") != 0) {

				progress->progress_info->file_index++;

				source_uri = gnome_vfs_uri_append_file_name (source_dir_uri, info->name);
				dest_uri = gnome_vfs_uri_append_file_name (target_dir_uri, info->name);
				
				if (info->type == GNOME_VFS_FILE_TYPE_REGULAR) {
					result = copy_file (info, source_uri, dest_uri, 
							    xfer_options, error_mode, overwrite_mode, 
							    progress, skip);
				} else if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
					result = copy_directory (info, source_uri, dest_uri, 
								 xfer_options, error_mode, overwrite_mode, 
								 progress, skip);
				} else if (info->type == GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK) {
					result = copy_symlink (source_uri, dest_uri, info->symlink_name,
							       progress);
				}
				/* just ignore all the other special file system objects here */
			}
			
			gnome_vfs_file_info_unref (info);
			
			if (dest_uri != NULL) {
				gnome_vfs_uri_unref (dest_uri);
			}
			if (source_uri != NULL) {
				gnome_vfs_uri_unref (source_uri);
			}

		} while (result == GNOME_VFS_OK);
	}

	if (result == GNOME_VFS_ERROR_EOF) {
		/* all is well, we just finished iterating the directory */
		result = GNOME_VFS_OK;
	}

	gnome_vfs_directory_close (dest_directory_handle);
	gnome_vfs_directory_close (source_directory_handle);

	if (result == GNOME_VFS_OK) {
		/* FIXME the modules should ignore setting of permissions if
		 * "valid_fields & GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS" is clear
		 * for now, make sure permissions aren't set to 000
		 */

		if ((source_file_info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_PERMISSIONS) == 0) {
			set_mask = GNOME_VFS_SET_FILE_INFO_TIME;
		} else {
			set_mask = GNOME_VFS_SET_FILE_INFO_PERMISSIONS
					| GNOME_VFS_SET_FILE_INFO_OWNER
					| GNOME_VFS_SET_FILE_INFO_TIME;
		}

		/* ignore errors while setting file info attributes at this point */
		gnome_vfs_set_file_info_uri (target_dir_uri, source_file_info, set_mask);
	}

	return result;
}

static GnomeVFSResult
copy_items (const GList *source_uri_list,
	    const GList *target_uri_list,
	    GnomeVFSXferOptions xfer_options,
	    GnomeVFSXferErrorMode *error_mode,
	    GnomeVFSXferOverwriteMode overwrite_mode,
	    GnomeVFSProgressCallbackState *progress,
	    GList **p_source_uris_copied_list)
{
	GnomeVFSResult result;
	const GList *source_item, *target_item;
	
	result = GNOME_VFS_OK;

	/* go through the list of names */
	for (source_item = source_uri_list, target_item = target_uri_list; source_item != NULL;) {

		GnomeVFSURI *source_uri;
		GnomeVFSURI *target_uri;
		GnomeVFSURI *target_dir_uri;

		GnomeVFSFileInfo *info;
		gboolean skip;
		int count;
		int progress_result;

		progress->progress_info->file_index++;

		skip = FALSE;
		target_uri = NULL;

		source_uri = (GnomeVFSURI *)source_item->data;
		target_dir_uri = gnome_vfs_uri_get_parent ((GnomeVFSURI *)target_item->data);
		
		/* get source URI and file info */
		info = gnome_vfs_file_info_new ();
		result = gnome_vfs_get_file_info_uri (source_uri, info, 
						      GNOME_VFS_FILE_INFO_DEFAULT);

		progress->progress_info->duplicate_name =
			gnome_vfs_uri_extract_short_path_name
			((GnomeVFSURI *)target_item->data);

		if (result == GNOME_VFS_OK) {
			/* optionally keep trying until we hit a unique target name */
			for (count = 1; ; count++) {
				GnomeVFSXferOverwriteMode overwrite_mode_abort;

				target_uri = gnome_vfs_uri_append_string
					(target_dir_uri, 
					 progress->progress_info->duplicate_name);

				progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_OK;
				progress->progress_info->file_size = info->size;
				progress->progress_info->bytes_copied = 0;
				progress->progress_info->top_level_item = TRUE;

				if (call_progress_with_uris_often (progress, source_uri, target_uri, 
						       GNOME_VFS_XFER_PHASE_COPYING) == 0) {
					result = GNOME_VFS_ERROR_INTERRUPTED;
				}

				overwrite_mode_abort = GNOME_VFS_XFER_OVERWRITE_MODE_ABORT;
				
				
				if (info->type == GNOME_VFS_FILE_TYPE_REGULAR) {
					result = copy_file (info, source_uri, target_uri, 
							    xfer_options, error_mode, 
							    &overwrite_mode_abort, 
							    progress, &skip);
				} else if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
					result = copy_directory (info, source_uri, target_uri, 
								 xfer_options, error_mode,
								 &overwrite_mode_abort,
								 progress, &skip);
                                } else if (info->type == GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK) {
					result = copy_symlink (source_uri, target_uri, info->symlink_name,
							       progress);
                                }
				/* just ignore all the other special file system objects here */

				if (result == GNOME_VFS_OK && !skip) {
					/* Add to list of successfully copied files... */
					*p_source_uris_copied_list = g_list_prepend (*p_source_uris_copied_list, source_uri);
					gnome_vfs_uri_ref (source_uri);
				}

				if (result != GNOME_VFS_ERROR_FILE_EXISTS) {
					/* whatever happened, it wasn't a name conflict */
					break;
				}

				if (overwrite_mode != GNOME_VFS_XFER_OVERWRITE_MODE_QUERY
				    || (xfer_options & GNOME_VFS_XFER_USE_UNIQUE_NAMES) == 0)
					break;

				/* pass in the current target name, progress will update it to 
				 * a new unique name such as 'foo (copy)' or 'bar (copy 2)'
				 */
				g_free (progress->progress_info->duplicate_name);
				progress->progress_info->duplicate_name = 
					gnome_vfs_uri_extract_short_path_name
					((GnomeVFSURI *)target_item->data);
				progress->progress_info->duplicate_count = count;
				progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_DUPLICATE;
				progress->progress_info->vfs_status = result;
				progress_result = call_progress_uri (progress, source_uri, target_uri, 
						       GNOME_VFS_XFER_PHASE_COPYING);
				progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_OK;

				if (progress_result == GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT) {
					break;
				}

				if (skip) {
					break;
				}
				
				/* try again with new uri */
				gnome_vfs_uri_unref (target_uri);

			}
		}

		gnome_vfs_file_info_unref (info);
		g_free (progress->progress_info->duplicate_name);

		if (result != GNOME_VFS_OK) {
			break;
		}

		gnome_vfs_uri_unref (target_dir_uri);

		source_item = source_item->next;
		target_item = target_item->next;

		g_assert ((source_item != NULL) == (target_item != NULL));
	}

	return result;
}

static GnomeVFSResult
move_items (const GList *source_uri_list,
	    const GList *target_uri_list,
	    GnomeVFSXferOptions xfer_options,
	    GnomeVFSXferErrorMode *error_mode,
	    GnomeVFSXferOverwriteMode *overwrite_mode,
	    GnomeVFSProgressCallbackState *progress)
{
	GnomeVFSResult result;
	const GList *source_item, *target_item;
	
	result = GNOME_VFS_OK;

	/* go through the list of names */
	for (source_item = source_uri_list, target_item = target_uri_list; source_item != NULL;) {

		GnomeVFSURI *source_uri;
		GnomeVFSURI *target_uri;
		GnomeVFSURI *target_dir_uri;
		gboolean retry;
		gboolean skip;
		int conflict_count;
		int progress_result;

		source_uri = (GnomeVFSURI *)source_item->data;
		target_dir_uri = gnome_vfs_uri_get_parent ((GnomeVFSURI *)target_item->data);

		progress->progress_info->duplicate_name =  
			gnome_vfs_uri_extract_short_path_name
			((GnomeVFSURI *)target_item->data);

		skip = FALSE;
		conflict_count = 1;

		do {
			retry = FALSE;
			target_uri = gnome_vfs_uri_append_string (target_dir_uri, 
				 progress->progress_info->duplicate_name);

			progress->progress_info->file_size = 0;
			progress->progress_info->bytes_copied = 0;
			progress_set_source_target_uris (progress, source_uri, target_uri);
			progress->progress_info->top_level_item = TRUE;

			/* no matter what the replace mode, just overwrite the destination
			 * handle_name_conflicts took care of conflicting files
			 */
			result = gnome_vfs_move_uri (source_uri, target_uri, 
						     (xfer_options & GNOME_VFS_XFER_USE_UNIQUE_NAMES) != 0
						     ? GNOME_VFS_XFER_OVERWRITE_MODE_ABORT
						     : GNOME_VFS_XFER_OVERWRITE_MODE_REPLACE);


			if (result == GNOME_VFS_ERROR_FILE_EXISTS) {
				/* deal with a name conflict -- ask the progress_callback for a better name */
				g_free (progress->progress_info->duplicate_name);
				progress->progress_info->duplicate_name =
					gnome_vfs_uri_extract_short_path_name
					((GnomeVFSURI *)target_item->data);
				progress->progress_info->duplicate_count = conflict_count;
				progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_DUPLICATE;
				progress->progress_info->vfs_status = result;
				progress_result = call_progress_uri (progress, source_uri, target_uri, 
						       GNOME_VFS_XFER_PHASE_COPYING);
				progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_OK;

				if (progress_result == GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT) {
					gnome_vfs_uri_unref (target_uri);
					break;
				}
				conflict_count++;
				result = GNOME_VFS_OK;
				retry = TRUE;
				continue;
			}

			if (result != GNOME_VFS_OK) {
				retry = handle_error (&result, progress, error_mode, &skip);
			}

			if (result == GNOME_VFS_OK 
			    && !skip
			    && call_progress_with_uris_often (progress, source_uri,
					target_uri, GNOME_VFS_XFER_PHASE_MOVING) == 0) {
				result = GNOME_VFS_ERROR_INTERRUPTED;
				gnome_vfs_uri_unref (target_uri);
				break;
			}
			gnome_vfs_uri_unref (target_uri);
		} while (retry);
		
		gnome_vfs_uri_unref (target_dir_uri);

		if (result != GNOME_VFS_OK && !skip)
			break;

		source_item = source_item->next;
		target_item = target_item->next;
		g_assert ((source_item != NULL) == (target_item != NULL));
	}

	return result;
}

static GnomeVFSResult
link_items (const GList *source_uri_list,
	    const GList *target_uri_list,
	    GnomeVFSXferOptions xfer_options,
	    GnomeVFSXferErrorMode *error_mode,
	    GnomeVFSXferOverwriteMode *overwrite_mode,
	    GnomeVFSProgressCallbackState *progress)
{
	GnomeVFSResult result;
	const GList *source_item, *target_item;
	GnomeVFSURI *source_uri;
	GnomeVFSURI *target_dir_uri;
	GnomeVFSURI *target_uri;
	gboolean retry;
	gboolean skip;
	int conflict_count;
	int progress_result;
	char *source_reference;

	result = GNOME_VFS_OK;

	/* go through the list of names, create a link to each item */
	for (source_item = source_uri_list, target_item = target_uri_list; source_item != NULL;) {

		source_uri = (GnomeVFSURI *)source_item->data;
		source_reference = gnome_vfs_uri_to_string (source_uri, GNOME_VFS_URI_HIDE_NONE);

		target_dir_uri = gnome_vfs_uri_get_parent ((GnomeVFSURI *)target_item->data);
		progress->progress_info->duplicate_name =
			gnome_vfs_uri_extract_short_path_name
			((GnomeVFSURI *)target_item->data);

		skip = FALSE;
		conflict_count = 1;

		do {
			retry = FALSE;
			target_uri = gnome_vfs_uri_append_string
				(target_dir_uri,
				 progress->progress_info->duplicate_name);

			progress->progress_info->file_size = 0;
			progress->progress_info->bytes_copied = 0;
			progress->progress_info->top_level_item = TRUE;
			progress_set_source_target_uris (progress, source_uri, target_uri);

			/* no matter what the replace mode, just overwrite the destination
			 * handle_name_conflicts took care of conflicting files
			 */
			result = gnome_vfs_create_symbolic_link (target_uri, source_reference); 
			if (result == GNOME_VFS_ERROR_FILE_EXISTS) {
				/* deal with a name conflict -- ask the progress_callback for a better name */
				g_free (progress->progress_info->duplicate_name);
				progress->progress_info->duplicate_name =
					gnome_vfs_uri_extract_short_path_name
					((GnomeVFSURI *)target_item->data);
				progress->progress_info->duplicate_count = conflict_count;
				progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_DUPLICATE;
				progress->progress_info->vfs_status = result;
				progress_result = call_progress_uri (progress, source_uri, target_uri, 
						       GNOME_VFS_XFER_PHASE_COPYING);
				progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_OK;

				if (progress_result == GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT) {
					gnome_vfs_uri_unref (target_uri);
					break;
				}
				conflict_count++;
				result = GNOME_VFS_OK;
				retry = TRUE;
				continue;
			}
			
			if (result != GNOME_VFS_OK) {
				retry = handle_error (&result, progress, error_mode, &skip);
			}

			if (result == GNOME_VFS_OK
				&& call_progress_with_uris_often (progress, source_uri,
						target_uri, GNOME_VFS_XFER_PHASE_OPENTARGET) == 0) {
				result = GNOME_VFS_ERROR_INTERRUPTED;
				gnome_vfs_uri_unref (target_uri);
				break;
			}
			gnome_vfs_uri_unref (target_uri);
		} while (retry);
		
		gnome_vfs_uri_unref (target_dir_uri);
		g_free (source_reference);

		if (result != GNOME_VFS_OK && !skip)
			break;

		source_item = source_item->next;
		target_item = target_item->next;
		g_assert ((source_item != NULL) == (target_item != NULL));
	}

	return result;
}

static GnomeVFSResult
gnome_vfs_xfer_empty_directories (const GList *trash_dir_uris,
			          GnomeVFSXferErrorMode error_mode,
			          GnomeVFSProgressCallbackState *progress)
{
	GnomeVFSResult result;
	const GList *p;
	gboolean skip;

	result = GNOME_VFS_OK;

		/* initialize the results */
	progress->progress_info->files_total = 0;
	progress->progress_info->bytes_total = 0;
	progress->progress_info->phase = GNOME_VFS_XFER_PHASE_COLLECTING;


	for (p = trash_dir_uris;  p != NULL; p = p->next) {
		result = directory_add_items_and_size (p->data,
			GNOME_VFS_XFER_REMOVESOURCE | GNOME_VFS_XFER_RECURSIVE, 
			progress);
		if (result == GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES) {
			/* out of file descriptors -- we will deal with that */
			result = GNOME_VFS_OK;
			break;
		}
		/* set up a fake total size to represent the bulk of the operation
		 * -- we'll subtract a proportional value for every deletion
		 */
		progress->progress_info->bytes_total 
			= progress->progress_info->files_total * DEFAULT_SIZE_OVERHEAD;
	}
	call_progress (progress, GNOME_VFS_XFER_PHASE_READYTOGO);
	for (p = trash_dir_uris;  p != NULL; p = p->next) {
		result = empty_directory ((GnomeVFSURI *)p->data, progress, 
			GNOME_VFS_XFER_REMOVESOURCE | GNOME_VFS_XFER_RECURSIVE, 
			&error_mode, &skip);
		if (result == GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES) {
			result = non_recursive_empty_directory ((GnomeVFSURI *)p->data, 
				progress, GNOME_VFS_XFER_REMOVESOURCE | GNOME_VFS_XFER_RECURSIVE, 
				&error_mode, &skip);
		}
	}

	return result;
}

static GnomeVFSResult
gnome_vfs_xfer_delete_items_common (const GList *source_uri_list,
			            GnomeVFSXferErrorMode error_mode,
			            GnomeVFSXferOptions xfer_options,
			            GnomeVFSProgressCallbackState *progress)
{
	GnomeVFSFileInfo *info;
	GnomeVFSResult result;
	GnomeVFSURI *uri;
	const GList *p;
	gboolean skip;

	result = GNOME_VFS_OK;
	
	for (p = source_uri_list;  p != NULL; p = p->next) {
	
		skip = FALSE;
		/* get the URI and VFSFileInfo for each */
		uri = p->data;

		info = gnome_vfs_file_info_new ();
		result = gnome_vfs_get_file_info_uri (uri, info, 
						      GNOME_VFS_FILE_INFO_DEFAULT);

		if (result != GNOME_VFS_OK) {
			break;
		}

		progress_set_source_target_uris (progress, uri, NULL);
		if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
			remove_directory (uri, TRUE, progress, xfer_options, 
					  &error_mode, &skip);
		} else {
			remove_file (uri, progress, xfer_options, &error_mode,
				     &skip);
		}
	}

	return result;
}

static GnomeVFSResult
gnome_vfs_xfer_delete_items (const GList *source_uri_list,
			     GnomeVFSXferErrorMode error_mode,
			     GnomeVFSXferOptions xfer_options,
			     GnomeVFSProgressCallbackState *progress)
{

	GnomeVFSResult result;
		
		/* initialize the results */
	progress->progress_info->files_total = 0;
	progress->progress_info->bytes_total = 0;
	call_progress (progress, GNOME_VFS_XFER_PHASE_COLLECTING);


	result = count_items_and_size (source_uri_list,
		GNOME_VFS_XFER_REMOVESOURCE | GNOME_VFS_XFER_RECURSIVE, progress, TRUE, FALSE);

	/* When deleting, ignore the real file sizes, just count the same DEFAULT_SIZE_OVERHEAD
	 * for each file.
	 */
	progress->progress_info->bytes_total
		= progress->progress_info->files_total * DEFAULT_SIZE_OVERHEAD;
	if (result != GNOME_VFS_ERROR_INTERRUPTED) {
		call_progress (progress, GNOME_VFS_XFER_PHASE_READYTOGO);
		result = gnome_vfs_xfer_delete_items_common (source_uri_list,
			error_mode, xfer_options, progress);
	}

	return result;
}

static GnomeVFSResult
gnome_vfs_new_directory_with_unique_name (const GnomeVFSURI *target_dir_uri,
					  const char *name,
					  GnomeVFSXferErrorMode error_mode,
					  GnomeVFSXferOverwriteMode overwrite_mode,
					  GnomeVFSProgressCallbackState *progress)
{
	GnomeVFSResult result;
	GnomeVFSURI *target_uri;
	GnomeVFSDirectoryHandle *dest_directory_handle;
	gboolean dummy;
	int progress_result;
	int conflict_count;
	
	dest_directory_handle = NULL;
	progress->progress_info->top_level_item = TRUE;
	progress->progress_info->duplicate_name = g_strdup (name);

	for (conflict_count = 1; ; conflict_count++) {

		target_uri = gnome_vfs_uri_append_string
			(target_dir_uri, 
			 progress->progress_info->duplicate_name);
		result = create_directory (target_uri, 
					   &dest_directory_handle,
					   GNOME_VFS_XFER_USE_UNIQUE_NAMES,
					   &error_mode,
					   &overwrite_mode,
					   progress,
					   &dummy);

		if (result != GNOME_VFS_ERROR_FILE_EXISTS
			&& result != GNOME_VFS_ERROR_NAME_TOO_LONG) {
			break;
		}

		/* deal with a name conflict -- ask the progress_callback for a better name */
		g_free (progress->progress_info->duplicate_name);
		progress->progress_info->duplicate_name = g_strdup (name);
		progress->progress_info->duplicate_count = conflict_count;
		progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_DUPLICATE;
		progress->progress_info->vfs_status = result;
		progress_result = call_progress_uri (progress, NULL, target_uri, 
				       GNOME_VFS_XFER_PHASE_COPYING);
		progress->progress_info->status = GNOME_VFS_XFER_PROGRESS_STATUS_OK;

		if (progress_result == GNOME_VFS_XFER_OVERWRITE_ACTION_ABORT) {
			break;
		}

		gnome_vfs_uri_unref (target_uri);
	}

	call_progress_uri (progress, NULL, target_uri,
		GNOME_VFS_XFER_PHASE_OPENTARGET);

	if (dest_directory_handle != NULL) {
		gnome_vfs_directory_close (dest_directory_handle);
	}

	gnome_vfs_uri_unref (target_uri);
	g_free (progress->progress_info->duplicate_name);

	return result;
}

static GnomeVFSResult
gnome_vfs_destination_is_writable (const GnomeVFSURI *uri)
{
	GnomeVFSURI *test_uri;
	GnomeVFSResult result;
	GnomeVFSHandle *handle;

	if (!gnome_vfs_uri_is_local (uri)) {
		/* if destination is not local, do not test it for writability, just
		 * assume it's writable
		 */
		return GNOME_VFS_OK;
	}

	/* test writability by creating and erasing a temporary file */
	test_uri = gnome_vfs_uri_append_file_name (uri, ".vfs-write.tmp");
	result = gnome_vfs_create_uri (&handle, test_uri, GNOME_VFS_OPEN_WRITE, TRUE, 0600);

	if (result == GNOME_VFS_OK) {
		gnome_vfs_close (handle);
	}
	
	if (result == GNOME_VFS_OK || result == GNOME_VFS_ERROR_FILE_EXISTS) {
		gnome_vfs_unlink_from_uri (test_uri);
		result = GNOME_VFS_OK;
	}
	
	gnome_vfs_uri_unref (test_uri);
	return result;
}

static GnomeVFSResult
gnome_vfs_xfer_uri_internal (const GList *source_uris,
			     const GList *target_uris,
			     GnomeVFSXferOptions xfer_options,
			     GnomeVFSXferErrorMode error_mode,
			     GnomeVFSXferOverwriteMode overwrite_mode,
			     GnomeVFSProgressCallbackState *progress)
{
	GnomeVFSResult result;
	GList *source_uri_list, *target_uri_list;
	GList *source_uri, *target_uri;
	GList *source_uri_list_copied;
	GnomeVFSURI *target_dir_uri;
	gboolean move, link;
	GnomeVFSFileSize free_bytes;
	GnomeVFSFileSize bytes_total;
	gulong files_total;
	gboolean skip;
	
	result = GNOME_VFS_OK;
	move = FALSE;
	link = FALSE;
	target_dir_uri = NULL;
	source_uri_list_copied = NULL;

	/* Check and see if target is writable. Return error if it is not. */
	call_progress (progress, GNOME_VFS_XFER_CHECKING_DESTINATION);
	target_dir_uri = gnome_vfs_uri_get_parent ((GnomeVFSURI *)((GList *)target_uris)->data);
	result = gnome_vfs_destination_is_writable (target_dir_uri);
	progress_set_source_target_uris (progress, NULL, target_dir_uri);
	if (result != GNOME_VFS_OK) {
		handle_error (&result, progress, &error_mode, &skip);
		gnome_vfs_uri_unref (target_dir_uri);
		return result;
	}

	move = (xfer_options & GNOME_VFS_XFER_REMOVESOURCE) != 0;
	link = (xfer_options & GNOME_VFS_XFER_LINK_ITEMS) != 0;

	if (move && link) {
		return GNOME_VFS_ERROR_BAD_PARAMETERS;
	}

	/* Create an owning list of source and destination uris.
	 * We want to be able to remove items that we decide to skip during
	 * name conflict check.
	 */
	source_uri_list = gnome_vfs_uri_list_copy ((GList *)source_uris);
	target_uri_list = gnome_vfs_uri_list_copy ((GList *)target_uris);

	if ((xfer_options & GNOME_VFS_XFER_USE_UNIQUE_NAMES) == 0) {
		/* see if moved files are on the same file system so that we decide to do
		 * a simple move or have to do a copy/remove
		 */
		for (source_uri = source_uri_list, target_uri = target_uri_list;
			source_uri != NULL;
			source_uri = source_uri->next, target_uri = target_uri->next) {
			gboolean same_fs;

			g_assert (target_dir_uri != NULL);

			result = gnome_vfs_check_same_fs_uris ((GnomeVFSURI *)source_uri->data, 
				target_dir_uri, &same_fs);

			if (result != GNOME_VFS_OK) {
				break;
			}

			move &= same_fs;
		}
	}

	if (target_dir_uri != NULL) {
		gnome_vfs_uri_unref (target_dir_uri);
		target_dir_uri = NULL;
	}
	
	if (result == GNOME_VFS_OK) {
		call_progress (progress, GNOME_VFS_XFER_PHASE_COLLECTING);
		result = count_items_and_size (source_uri_list, xfer_options, progress, move, link);
		if (result != GNOME_VFS_ERROR_INTERRUPTED) {
			/* Ignore anything but interruptions here -- we will deal with the errors
			 * during the actual copy
			 */
			result = GNOME_VFS_OK;
		}
	}
			
	if (result == GNOME_VFS_OK) {
		/* Calculate free space on destination. If an error is returned, we have a non-local
		 * file system, so we just forge ahead and hope for the best 
		 */			 
		target_dir_uri = gnome_vfs_uri_get_parent ((GnomeVFSURI *)target_uri_list->data);
		result = gnome_vfs_get_volume_free_space (target_dir_uri, &free_bytes);


		if (result == GNOME_VFS_OK) {
			if (!move && progress->progress_info->bytes_total > free_bytes) {
				result = GNOME_VFS_ERROR_NO_SPACE;
				progress_set_source_target_uris (progress, NULL, target_dir_uri);
				handle_error (&result, progress, &error_mode, &skip);
			}
		} else {
			/* Errors from gnome_vfs_get_volume_free_space should be ignored */
			result = GNOME_VFS_OK;
		}
		
		if (target_dir_uri != NULL) {
			gnome_vfs_uri_unref (target_dir_uri);
			target_dir_uri = NULL;
		}

		if (result != GNOME_VFS_OK) {
			return result;
		}
			
		if ((xfer_options & GNOME_VFS_XFER_USE_UNIQUE_NAMES) == 0) {
		
			/* Save the preflight numbers, handle_name_conflicts would overwrite them */
			bytes_total = progress->progress_info->bytes_total;
			files_total = progress->progress_info->files_total;

			/* Set the preflight numbers to 0, we don't want to run progress on the
			 * removal of conflicting items.
			 */
			progress->progress_info->bytes_total = 0;
			progress->progress_info->files_total = 0;
			
			result = handle_name_conflicts (&source_uri_list, &target_uri_list,
						        xfer_options, &error_mode, &overwrite_mode,
						        progress);
						        
			progress->progress_info->bytes_total = bytes_total;
			progress->progress_info->files_total = files_total;

		}

		/* reset the preflight numbers */
		progress->progress_info->file_index = 0;
		progress->progress_info->total_bytes_copied = 0;

		if (result != GNOME_VFS_OK) {
			/* don't care about any results from handle_error */
			handle_error (&result, progress, &error_mode, &skip);

			/* whatever error it was, we handled it */
			result = GNOME_VFS_OK;
		} else {
			call_progress (progress, GNOME_VFS_XFER_PHASE_READYTOGO);

			if (move) {
				g_assert (!link);
				result = move_items (source_uri_list, target_uri_list,
						     xfer_options, &error_mode, &overwrite_mode, 
						     progress);
			} else if (link) {
				result = link_items (source_uri_list, target_uri_list,
						     xfer_options, &error_mode, &overwrite_mode,
						     progress);
			} else {
				result = copy_items (source_uri_list, target_uri_list,
						     xfer_options, &error_mode, overwrite_mode, 
						     progress, &source_uri_list_copied);
			}
			
			if (result == GNOME_VFS_OK) {
				if (xfer_options & GNOME_VFS_XFER_REMOVESOURCE
				    && ! (move || link)) {
					call_progress (progress, GNOME_VFS_XFER_PHASE_CLEANUP);
					result = gnome_vfs_xfer_delete_items_common ( 
						 	source_uri_list_copied,
						 	error_mode, xfer_options, progress);
				}
			}
		}
	}

	gnome_vfs_uri_list_free (source_uri_list);
	gnome_vfs_uri_list_free (target_uri_list);
	gnome_vfs_uri_list_free (source_uri_list_copied);

	return result;
}

GnomeVFSResult
gnome_vfs_xfer_private (const GList *source_uri_list,
			const GList *target_uri_list,
			GnomeVFSXferOptions xfer_options,
			GnomeVFSXferErrorMode error_mode,
			GnomeVFSXferOverwriteMode overwrite_mode,
			GnomeVFSXferProgressCallback progress_callback,
			gpointer data,
			GnomeVFSXferProgressCallback sync_progress_callback,
			gpointer sync_progress_data)
{
	GnomeVFSProgressCallbackState progress_state;
	GnomeVFSXferProgressInfo progress_info;
	GnomeVFSURI *target_dir_uri;
	GnomeVFSResult result;
	
	init_progress (&progress_state, &progress_info);
	progress_state.sync_callback = sync_progress_callback;
	progress_state.user_data = sync_progress_data;
	progress_state.update_callback = progress_callback;
	progress_state.async_job_data = data;


	if ((xfer_options & GNOME_VFS_XFER_EMPTY_DIRECTORIES) != 0) {
		/* Directory empty operation (Empty Trash, etc.). */
		g_assert (source_uri_list != NULL);
		g_assert (target_uri_list == NULL);
		
		call_progress (&progress_state, GNOME_VFS_XFER_PHASE_INITIAL);
	 	result = gnome_vfs_xfer_empty_directories (source_uri_list, error_mode, &progress_state);
	} else if ((xfer_options & GNOME_VFS_XFER_DELETE_ITEMS) != 0) {
		/* Delete items operation */
		g_assert (source_uri_list != NULL);
		g_assert (target_uri_list == NULL);
				
		call_progress (&progress_state, GNOME_VFS_XFER_PHASE_INITIAL);
		result = gnome_vfs_xfer_delete_items (source_uri_list,
		      error_mode, xfer_options, &progress_state);
	} else if ((xfer_options & GNOME_VFS_XFER_NEW_UNIQUE_DIRECTORY) != 0) {
		/* New directory create operation */
		g_assert (source_uri_list == NULL);
		g_assert (g_list_length ((GList *) target_uri_list) == 1);

		target_dir_uri = gnome_vfs_uri_get_parent ((GnomeVFSURI *) target_uri_list->data);
		result = GNOME_VFS_ERROR_INVALID_URI;
		if (target_dir_uri != NULL) { 
			if (gnome_vfs_uri_get_basename ((GnomeVFSURI *) target_uri_list->data) != NULL) {
		
				result = gnome_vfs_new_directory_with_unique_name (target_dir_uri, 
					gnome_vfs_uri_get_basename ((GnomeVFSURI *) target_uri_list->data),
					error_mode, overwrite_mode, &progress_state);
			}
			gnome_vfs_uri_unref (target_dir_uri);
		}
	} else {
		/* Copy items operation */
		g_assert (source_uri_list != NULL);
		g_assert (target_uri_list != NULL);
		g_assert (g_list_length ((GList *)source_uri_list) == g_list_length ((GList *)target_uri_list));

		call_progress (&progress_state, GNOME_VFS_XFER_PHASE_INITIAL);
		result = gnome_vfs_xfer_uri_internal (source_uri_list, target_uri_list,
			xfer_options, error_mode, overwrite_mode, &progress_state);
	}

	call_progress (&progress_state, GNOME_VFS_XFER_PHASE_COMPLETED);
	free_progress (&progress_info);

	/* FIXME bugzilla.eazel.com 1218:
	 * 
	 * The async job setup will try to call the callback function with the callback data
	 * even though they are usually dead at this point because the callback detected
	 * that we are giving up and cleaned up after itself.
	 * 
	 * Should fix this in the async job call setup.
	 * 
	 * For now just pretend everything worked well.
	 * 
	 */
	result = GNOME_VFS_OK;

	return result;
}

GnomeVFSResult
gnome_vfs_xfer_uri_list (const GList *source_uri_list,
			 const GList *target_uri_list,
			 GnomeVFSXferOptions xfer_options,
			 GnomeVFSXferErrorMode error_mode,
			 GnomeVFSXferOverwriteMode overwrite_mode,
			 GnomeVFSXferProgressCallback progress_callback,
			 gpointer data)
{
	GnomeVFSProgressCallbackState progress_state;
	GnomeVFSXferProgressInfo progress_info;
	GnomeVFSResult result;

	g_return_val_if_fail (source_uri_list != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (target_uri_list != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);	
	g_return_val_if_fail (progress_callback != NULL || error_mode != GNOME_VFS_XFER_ERROR_MODE_QUERY,
			      GNOME_VFS_ERROR_BAD_PARAMETERS);	
		
	init_progress (&progress_state, &progress_info);
	progress_state.sync_callback = progress_callback;
	progress_state.user_data = data;

	call_progress (&progress_state, GNOME_VFS_XFER_PHASE_INITIAL);

	result = gnome_vfs_xfer_uri_internal (source_uri_list, target_uri_list,
		xfer_options, error_mode, overwrite_mode, &progress_state);

	call_progress (&progress_state, GNOME_VFS_XFER_PHASE_COMPLETED);
	free_progress (&progress_info);

	return result;
}

GnomeVFSResult	
gnome_vfs_xfer_uri (const GnomeVFSURI *source_uri,
		    const GnomeVFSURI *target_uri,
		    GnomeVFSXferOptions xfer_options,
		    GnomeVFSXferErrorMode error_mode,
		    GnomeVFSXferOverwriteMode overwrite_mode,
		    GnomeVFSXferProgressCallback progress_callback,
		    gpointer data)
{
	GList *source_uri_list, *target_uri_list;
	GnomeVFSResult result;

	g_return_val_if_fail (source_uri != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (target_uri != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);	
	g_return_val_if_fail (progress_callback != NULL || error_mode != GNOME_VFS_XFER_ERROR_MODE_QUERY,
			      GNOME_VFS_ERROR_BAD_PARAMETERS);	

	source_uri_list = g_list_append (NULL, (void *)source_uri);
	target_uri_list = g_list_append (NULL, (void *)target_uri);

	result = gnome_vfs_xfer_uri_list (source_uri_list, target_uri_list,
		xfer_options, error_mode, overwrite_mode, progress_callback, data);

	g_list_free (source_uri_list);
	g_list_free (target_uri_list);

	return result;
}

GnomeVFSResult 
gnome_vfs_xfer_delete_list (const GList *uri_list, 
                            GnomeVFSXferErrorMode error_mode,
                            GnomeVFSXferOptions xfer_options,
		            GnomeVFSXferProgressCallback
				   progress_callback,
                            gpointer data)
{
	GnomeVFSProgressCallbackState progress_state;
	GnomeVFSXferProgressInfo progress_info;
	GnomeVFSResult result;

	g_return_val_if_fail (uri_list != NULL, GNOME_VFS_ERROR_BAD_PARAMETERS);
	g_return_val_if_fail (progress_callback != NULL || error_mode != GNOME_VFS_XFER_ERROR_MODE_QUERY,
			      GNOME_VFS_ERROR_BAD_PARAMETERS);

	init_progress (&progress_state, &progress_info);
	progress_state.sync_callback = progress_callback;
	progress_state.user_data = data;
	call_progress (&progress_state, GNOME_VFS_XFER_PHASE_INITIAL);

	result = gnome_vfs_xfer_delete_items (uri_list, error_mode, xfer_options,
		&progress_state);
	
	call_progress (&progress_state, GNOME_VFS_XFER_PHASE_COMPLETED);
	free_progress (&progress_info);

	return result;
}

