/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* translate-method.c - translator method for GNOME Virtual File System

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
#include <glib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#include "gnome-vfs-method.h"
#include "gnome-vfs-mime.h"
#include "gnome-vfs-module.h"
#include "gnome-vfs-private-utils.h"

typedef struct {
	enum {
		MODE_BASIC, MODE_EXEC
	} mode;

	char *default_mime_type;
	char *real_method_name;

	union {
		struct {
			char *trans_string;
		} basic;
		struct {
			int argc;
			char **argv;
			char *orig_string;	/* this string gets chopped up into argv--it exists for freeing only */
			gboolean retain;
		} exec;
	}u;
} ParsedArgs;

/* State used for -exec -retain */
typedef struct {
	GMutex * retain_lock;
	FILE * retain_to;		/* pipe to child */
	FILE * retain_from;		/* pipe from child */
	pid_t retain_pid;		/* PID of child */				
} ExecState;

typedef struct {
	GnomeVFSMethod base_method;
	ParsedArgs pa;
	GnomeVFSMethod *real_method;
	ExecState exec_state;
} TranslateMethod;

static void
tr_apply_default_mime_type(TranslateMethod * tm,
			   GnomeVFSFileInfo * file_info)
{
	/* Only apply the default mime-type if the real method returns
	 * unknown mime-type and the the TranslateMethod /has/ a mime-type */
	if (file_info->mime_type == NULL) {
		/* FIXME bugzilla.eazel.com 2799: 
		 * this may not be needed since methods need to
		 * return application/octet-stream if it is an unknown
		 * mime-type, not NULL */
		if (tm->pa.default_mime_type) {
			file_info->mime_type =
			       	g_strdup (tm->pa.default_mime_type);
			file_info->valid_fields |=
				GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}
			
	} else {
		if ((strcmp (file_info->mime_type, GNOME_VFS_MIME_TYPE_UNKNOWN) == 0) &&
		    (tm->pa.default_mime_type != NULL)) {
			g_free (file_info->mime_type);
			file_info->mime_type =
				g_strdup (tm->pa.default_mime_type);
			file_info->valid_fields |=
				GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
		}		
	}
}

typedef struct {
	int child_out_fd;
	int child_in_fd;
} TrForkCBData;

static void /* GnomeVFSProcessInitFunc */ tr_forkexec_cb (gpointer data)
{
	TrForkCBData *cb_data;

	g_assert (NULL != data);
	cb_data = (TrForkCBData *) data;

	if ( -1 == dup2 (cb_data->child_in_fd, STDIN_FILENO) ) {
		_exit (-1);
	}

	if ( -1 == dup2 (cb_data->child_out_fd, STDOUT_FILENO) ) {
		_exit (-1);
	}

}


static pid_t tr_exec_open_child (char **argv, /*OUT*/ FILE ** p_from_stream, /*OUT*/ FILE ** p_to_stream)
{
	pid_t ret;
	int err;
	TrForkCBData cb_data;
	void *sigpipe_old;
	int pipe_to_child[2] 	= { -1, -1 };
	int pipe_to_parent[2] 	= { -1, -1 };

	g_assert ( NULL != p_from_stream );
	g_assert ( NULL != p_to_stream );

	*p_to_stream = NULL;
	*p_from_stream = NULL;

	/* Blocking SIGPIPE here is probably unnecessary --
	 * GnomeVFS already does this on init
	 */
	sigpipe_old = signal (SIGPIPE, SIG_IGN);

	err = pipe (pipe_to_child);

	if ( 0 != err ) {
		g_warning ("pipe returned error %d", errno);
		ret = -1;
		goto error;
	}

	err = pipe (pipe_to_parent);

	if ( 0 != err ) {
		g_warning ("pipe returned error %d", errno);
		ret = -1;
		goto error;
	}

	cb_data.child_out_fd = pipe_to_parent[1];
	cb_data.child_in_fd = pipe_to_child[0];

	/* Strictly speaking, this is unnecessary since these handles will be closed anyway*/
	err = fcntl ( pipe_to_parent[0], F_SETFD, FD_CLOEXEC ) ;
	g_assert (0 == err );
	err = fcntl ( pipe_to_child[1], F_SETFD, FD_CLOEXEC ) ;
	g_assert (0 == err );

	ret = gnome_vfs_forkexec (	argv[0],
					(const char * const*)argv,
					GNOME_VFS_PROCESS_CLOSEFDS,
					tr_forkexec_cb,
					(gpointer) &cb_data
	);

	close (pipe_to_parent[1]);
	pipe_to_parent[1] = -1;
	close (pipe_to_child[0]);
	pipe_to_child[0] = -1;
			
	if ( -1 == ret ) {
		g_warning ("fork returned error %d", errno);
		goto error;
	}

	*p_to_stream = fdopen (pipe_to_child[1], "w");
	g_assert ( NULL != *p_to_stream );
	pipe_to_child[1] = -1;

	*p_from_stream = fdopen (pipe_to_parent[0], "r");
	g_assert ( NULL != *p_from_stream );
	pipe_to_parent[0] = -1;

	setvbuf (*p_to_stream, NULL, _IOLBF, 0);
	setvbuf (*p_from_stream, NULL, _IOLBF, 0);

error:
	if ( -1 != pipe_to_parent[0] ) { close ( pipe_to_parent[0]); }
	if ( -1 != pipe_to_parent[1] ) { close ( pipe_to_parent[1]); }
	if ( -1 != pipe_to_child[0] ) { close ( pipe_to_child[0]); }
	if ( -1 != pipe_to_child[1] ) { close ( pipe_to_child[1]); }
	
	signal (SIGPIPE, sigpipe_old);
	
	return ret;
}

#define GETLINE_DELTA 256
static char * tr_getline (FILE* stream)
{
	char *ret;
	size_t ret_size;
	size_t ret_used;
	int char_read;
	gboolean got_newline;

	ret = g_malloc( GETLINE_DELTA * sizeof(char) );
	ret_size = GETLINE_DELTA;

	for ( ret_used = 0, got_newline = FALSE ;
	      !got_newline && (EOF != (char_read = fgetc (stream))) ; 
	      ret_used++
	) {
		if (ret_size == (ret_used + 1)) {
			ret_size += GETLINE_DELTA;
			ret = g_realloc (ret, ret_size); 
		}
		if ('\n' == char_read || '\r' == char_read ) {
			got_newline = TRUE;
			ret [ret_used] = '\0';
		} else {
			ret [ret_used] = char_read;
		}
	}

	if (got_newline) {
		return ret;
	} else {
		g_free (ret);
		return NULL;
	}
}
#undef GETLINE_DELTA

static void tr_exec_pass_uri (char *uri_string, FILE * out_stream)
{
	char *tmpstr;

	/*shave off the scheme--pass only the right hand side to the child*/
	tmpstr = strchr (uri_string, ':');
	fprintf (out_stream, "%s\n", tmpstr ? tmpstr + 1 : uri_string);
	fflush (out_stream);
	
	tmpstr = NULL;

}

static char * tr_exec_do_retain (TranslateMethod *tm, char * uri_string)
{
	char * child_result = NULL;
	int tries;

	g_mutex_lock (tm->exec_state.retain_lock);


	/* re-execute the child process in case it unexpectedly disappears */  
	for (tries = 0 ; ( ! child_result ) && tries < 3 ; tries ++) {
		if ( 0 == tm->exec_state.retain_pid ) {
			tm->exec_state.retain_pid = tr_exec_open_child (tm->pa.u.exec.argv, &(tm->exec_state.retain_from), &(tm->exec_state.retain_to));
			if ( -1 == tm->exec_state.retain_pid ) {
				tm->exec_state.retain_pid = 0;
				goto error;
			}
		}

		g_assert (uri_string);
		tr_exec_pass_uri (uri_string, tm->exec_state.retain_to);

		child_result = tr_getline (tm->exec_state.retain_from);

		if (NULL == child_result) {
			tm->exec_state.retain_pid = 0;
		}
	}
	
error:
	g_mutex_unlock (tm->exec_state.retain_lock);

	return child_result;
}

/*
 * -exec
 * 
 * USAGE: -exec <exec_string> -real-method <method> [-retain]
 * 
 * -exec allows a child process to filter and translate URI's passed to GNOME-VFS.
 * -exec requires a URI scheme to be specified via -real-method, and does not
 * allow the child process to change that method
 *
 * <exec_string> is a string that will be tokenized and passed to execv()
 * <method> is the URI scheme that all requests will be translated to.
 * 
 * Without -retain: 
 *	The child process is exec()'d.  The URI minus the scheme is presented
 *	as standard input.  On success, the child is expected to print out
 *	a translated URI minus the scheme and exit(0).  On failure, the child
 *	should print out ":\n" or exit with a non-zero value
 *
 * With -retain
 * 	The child process is exec()'d and fed multiple URI's, each newline terminated,
 * 	via standard in.  For each URI, the child must print to standard out a
 * 	translated URI followed by a newline or a ":" followed by a newline on error.
 * 	
 * 	Note than children written to work with -retain that use stdio need
 * 	to set their stdin and stdout buffering discipline to "line" instead of "block" 
 * 	like this:
 *		setvbuf (stdin, NULL, _IOLBF, 0);
 *		setvbuf (stdout, NULL, _IOLBF, 0);
 *
 * Config file example:
 * 
 * foo: libvfs-translate.so -real-method http -exec "/bin/sed -e s/\/\/www.microsoft.com/\/\/www.eazel.com/"
 * 
 * (note that sed won't work with -retain)
 */

/* FIXME bugzilla.eazel.com 2796: this may be broken when the child produces compound URI's */
static GnomeVFSURI *tr_handle_exec (TranslateMethod *tm, const GnomeVFSURI * uri)
{

	int err;
	char *tmpstr;

	int child_status;
	char *uri_string	= NULL;
	char *child_result 	= NULL;
	GnomeVFSURI * retval 	= NULL;

	/*
	 * Note that normally having a parent and child communicate
	 * bi-directionally via pipes is a bad idea.  However, in this case,
	 * the parent will write everything and then close, so it'll all be OK
	 * as long as the child doesnt intermix reads and writes and fill
	 * up their output pipe's buffer before the parent is done writing.
	 */

	uri_string = gnome_vfs_uri_to_string (uri, 0);

	if ( NULL == uri_string ) {
		goto error;
	}

	if (tm->pa.u.exec.retain) {
		child_result = tr_exec_do_retain (tm, uri_string);

		if ( NULL == child_result ) {
			goto error;
		}
	} else {
		pid_t child_pid;
		FILE *from_stream, *to_stream;

		child_pid = tr_exec_open_child (tm->pa.u.exec.argv, &from_stream, &to_stream);

		if ( -1 == child_pid ) {
			goto error;
		}
	
		uri_string = gnome_vfs_uri_to_string (uri, 0);
		g_assert (uri_string);
		tr_exec_pass_uri (uri_string, to_stream);

		fclose (to_stream);
		to_stream = NULL;

		child_result = tr_getline (from_stream);
		
		err = waitpid (child_pid, &child_status, 0);

		g_assert ( child_pid == err );

		if (! WIFEXITED (child_status) ) {
			goto error;
		}

		if (NULL == child_result) {
			g_warning ("Child produced no result");
			goto error;
		}
	}


	/* FIXME bugzilla.eazel.com 2797: 
	 * just because we've appended the same scheme, doesn't mean
	 * we're going to get the same method from gnome_vfs_uri_new
	 */

	/* A child result that ends in a :\n indicates an error */
	if ( ':' != child_result[ strlen(child_result) - 1 ]) {
		/* append the real scheme */
		tmpstr = g_strconcat (tm->pa.real_method_name, ":", child_result, NULL);
		g_free (child_result);
		child_result = tmpstr;
		tmpstr = NULL;		

		retval = gnome_vfs_uri_new_private (child_result, FALSE, TRUE, TRUE);

		if ( NULL == retval ) {
			g_warning ("Unable to make URI from child process's result '%s'", child_result);
			goto error;
		}
	}


error:
	g_free (child_result);
	g_free (uri_string);
	
	return retval;
}

static void
tr_exec_init (ExecState *exec_state)
{
	exec_state->retain_lock = g_mutex_new();
}

static void
tr_exec_cleanup (ExecState *exec_state)
{
	if (NULL != exec_state->retain_lock) {
		g_mutex_free (exec_state->retain_lock);
	}

	if (NULL != exec_state->retain_to) {
		fclose (exec_state->retain_to);
	}
	if (NULL != exec_state->retain_from) {
		fclose (exec_state->retain_from);
	}

	if ( 0 != exec_state->retain_pid ) {
		int child_status, err;
		
		kill (exec_state->retain_pid, SIGTERM);
		err = waitpid (exec_state->retain_pid, &child_status, 0);		
		g_assert (err == exec_state->retain_pid);
	}
}

static GnomeVFSURI *tr_uri_translate(TranslateMethod * tm,
				     const GnomeVFSURI * uri)
{
	const char *text_uri;
	const char *text_uri_no_method;
	char *translated_text;
	char *translated_uri;
	GnomeVFSURI *retval;

	retval = NULL;

	if (uri->method != (GnomeVFSMethod *) tm)
		return gnome_vfs_uri_ref((GnomeVFSURI *) uri);	/* Don't translate things that don't belong to us */

	/* Hack it all up to pieces */

	if (MODE_BASIC == tm->pa.mode) {
		text_uri = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
		text_uri_no_method = strchr (text_uri, ':');

		if (text_uri_no_method == NULL) {
			text_uri_no_method = text_uri;
		} else {
			text_uri_no_method = text_uri_no_method + 1;
		}
			
		translated_text = g_strdup_printf (tm->pa.u.basic.trans_string, 
						   uri->text, uri->text,
						   uri->text, uri->text, uri->text);
		translated_uri = g_strconcat (tm->pa.real_method_name, ":", 
					      translated_text, NULL);

		retval = gnome_vfs_uri_new_private (translated_uri, FALSE, TRUE, TRUE);

		g_free (translated_text);
		g_free (translated_uri);
	} else if (MODE_EXEC == tm->pa.mode) {
		retval = tr_handle_exec (tm, uri);
	} else {
		g_assert (FALSE);
	}

	return retval;
}

static GnomeVFSResult
tr_do_open(GnomeVFSMethod * method,
	   GnomeVFSMethodHandle ** method_handle_return,
	   GnomeVFSURI * uri,
	   GnomeVFSOpenMode mode, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri;
	GnomeVFSResult retval;

	real_uri = tr_uri_translate(tm, uri);

	if ( NULL != real_uri ) {
		retval =
		    tm->real_method->open(tm->real_method, method_handle_return,
					  real_uri, mode, context);

		gnome_vfs_uri_unref(real_uri);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	return retval;
}

static GnomeVFSResult
tr_do_create(GnomeVFSMethod * method,
	     GnomeVFSMethodHandle
	     ** method_handle_return,
	     GnomeVFSURI * uri,
	     GnomeVFSOpenMode mode,
	     gboolean exclusive, guint perm, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri;
	GnomeVFSResult retval;

	real_uri = tr_uri_translate(tm, uri);

	if ( NULL != real_uri ) {
		retval =
		    tm->real_method->create(tm->real_method, method_handle_return,
					    real_uri, mode, exclusive, perm,
					    context);
		gnome_vfs_uri_unref(real_uri);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	return retval;
}

static GnomeVFSResult
tr_do_close(GnomeVFSMethod * method,
	    GnomeVFSMethodHandle * method_handle,
	    GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	return tm->real_method->close(tm->real_method, method_handle,
				      context);
}

static GnomeVFSResult
tr_do_read(GnomeVFSMethod * method,
	   GnomeVFSMethodHandle * method_handle,
	   gpointer buffer,
	   GnomeVFSFileSize num_bytes,
	   GnomeVFSFileSize * bytes_read_return, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	return tm->real_method->read(tm->real_method, method_handle,
				     buffer, num_bytes, bytes_read_return,
				     context);
}

static GnomeVFSResult
tr_do_write(GnomeVFSMethod * method,
	    GnomeVFSMethodHandle * method_handle,
	    gconstpointer buffer,
	    GnomeVFSFileSize num_bytes,
	    GnomeVFSFileSize * bytes_written_return,
	    GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	return tm->real_method->write(tm->real_method, method_handle,
				      buffer, num_bytes,
				      bytes_written_return, context);
}

static GnomeVFSResult
tr_do_seek(GnomeVFSMethod * method,
	   GnomeVFSMethodHandle * method_handle,
	   GnomeVFSSeekPosition whence,
	   GnomeVFSFileOffset offset, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	return tm->real_method->seek(tm->real_method, method_handle,
				     whence, offset, context);
}

static GnomeVFSResult
tr_do_tell(GnomeVFSMethod * method,
	   GnomeVFSMethodHandle * method_handle,
	   GnomeVFSFileOffset * offset_return)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	return tm->real_method->tell(tm->real_method, method_handle,
				     offset_return);
}

static GnomeVFSResult
tr_do_open_directory(GnomeVFSMethod * method,
		     GnomeVFSMethodHandle ** method_handle,
		     GnomeVFSURI * uri,
		     GnomeVFSFileInfoOptions options,
		     const GnomeVFSDirectoryFilter * filter,
		     GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri;
	GnomeVFSResult retval;

	real_uri = tr_uri_translate(tm, uri);

	if ( NULL != real_uri ) {
		retval =
		    tm->real_method->open_directory(tm->real_method, method_handle,
						    real_uri, options,
						    filter, context);
		gnome_vfs_uri_unref(real_uri);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	return retval;
}

static GnomeVFSResult
tr_do_close_directory(GnomeVFSMethod * method,
		      GnomeVFSMethodHandle * method_handle,
		      GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	return tm->real_method->close_directory(tm->real_method,
						method_handle, context);
}

static GnomeVFSResult
tr_do_read_directory(GnomeVFSMethod * method,
		     GnomeVFSMethodHandle * method_handle,
		     GnomeVFSFileInfo * file_info,
		     GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSResult retval;

	retval =
	    tm->real_method->read_directory(tm->real_method, method_handle,
					    file_info, context);

	tr_apply_default_mime_type(tm, file_info);

	return retval;
}

static GnomeVFSResult
tr_do_get_file_info(GnomeVFSMethod * method,
		    GnomeVFSURI * uri,
		    GnomeVFSFileInfo * file_info,
		    GnomeVFSFileInfoOptions options,
		    GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri;
	GnomeVFSResult retval;

	real_uri = tr_uri_translate(tm, uri);

	if ( NULL != real_uri ) {
		retval =
		    tm->real_method->get_file_info(tm->real_method, real_uri,
						   file_info, options,
						   context);

		gnome_vfs_uri_unref(real_uri);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	tr_apply_default_mime_type(tm, file_info);

	return retval;
}

static GnomeVFSResult
tr_do_get_file_info_from_handle(GnomeVFSMethod * method,
				GnomeVFSMethodHandle * method_handle,
				GnomeVFSFileInfo * file_info,
				GnomeVFSFileInfoOptions options,
				GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSResult retval;

	retval =
	    tm->real_method->get_file_info_from_handle(tm->real_method,
						       method_handle,
						       file_info, 
						       options,
						       context);

	tr_apply_default_mime_type(tm, file_info);

	return retval;
}

static GnomeVFSResult
tr_do_truncate(GnomeVFSMethod * method,
	       GnomeVFSURI * uri,
	       GnomeVFSFileSize length, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri;
	GnomeVFSResult retval;

	real_uri = tr_uri_translate(tm, uri);

	if ( NULL != real_uri ) {
		retval =
		    tm->real_method->truncate(tm->real_method, real_uri, length,
					      context);

		gnome_vfs_uri_unref(real_uri);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	return retval;
}

static GnomeVFSResult
tr_do_truncate_handle(GnomeVFSMethod * method,
		      GnomeVFSMethodHandle * method_handle,
		      GnomeVFSFileSize length, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;

	return tm->real_method->truncate_handle(tm->real_method,
						method_handle, length,
						context);
}

static gboolean
tr_do_is_local(GnomeVFSMethod * method, const GnomeVFSURI * uri)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri;
	GnomeVFSResult retval;

	real_uri = tr_uri_translate(tm, uri);

	if ( NULL != real_uri ) {
		retval = tm->real_method->is_local(tm->real_method, real_uri);

		gnome_vfs_uri_unref(real_uri);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	return retval;
}

static GnomeVFSResult
tr_do_make_directory(GnomeVFSMethod * method,
		     GnomeVFSURI * uri,
		     guint perm, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri;
	GnomeVFSResult retval;

	real_uri = tr_uri_translate(tm, uri);

	if ( NULL != real_uri ) {
		retval =
		    tm->real_method->make_directory(tm->real_method, real_uri,
						    perm, context);

		gnome_vfs_uri_unref(real_uri);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	return retval;
}

static GnomeVFSResult
tr_do_find_directory(GnomeVFSMethod * method,
		     GnomeVFSURI * near_uri,
		     GnomeVFSFindDirectoryKind kind,
		     GnomeVFSURI ** result_uri,
		     gboolean create_if_needed,
		     gboolean find_if_needed,
		     guint permissions, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri;
	GnomeVFSResult retval;

	real_uri = tr_uri_translate(tm, near_uri);

	if ( NULL != real_uri ) {
		retval =
		    tm->real_method->find_directory(tm->real_method, real_uri,
						    kind, result_uri,
						    create_if_needed, find_if_needed,
						    permissions,
						    context);

		gnome_vfs_uri_unref(real_uri);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	return retval;
}



static GnomeVFSResult
tr_do_remove_directory(GnomeVFSMethod * method,
		       GnomeVFSURI * uri, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri;
	GnomeVFSResult retval;

	real_uri = tr_uri_translate(tm, uri);

	if ( NULL != real_uri ) {
		retval =
		    tm->real_method->remove_directory(tm->real_method, real_uri,
						      context);

		gnome_vfs_uri_unref(real_uri);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	return retval;
}

static GnomeVFSResult
tr_do_move(GnomeVFSMethod * method,
	   GnomeVFSURI * old_uri,
	   GnomeVFSURI * new_uri,
	   gboolean force_replace, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri_old, *real_uri_new;
	GnomeVFSResult retval;

	real_uri_old = tr_uri_translate(tm, old_uri);
	real_uri_new = tr_uri_translate(tm, new_uri);

	if ( NULL != real_uri_old && NULL != real_uri_new ) {
		retval =
		    tm->real_method->move(tm->real_method, real_uri_old,
					  real_uri_new, force_replace, context);

	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	if (real_uri_old) {gnome_vfs_uri_unref(real_uri_old);}
	if (real_uri_new) {gnome_vfs_uri_unref(real_uri_new);}

	return retval;
}

static GnomeVFSResult
tr_do_unlink(GnomeVFSMethod * method,
	     GnomeVFSURI * uri, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri;
	GnomeVFSResult retval;

	real_uri = tr_uri_translate(tm, uri);

	if ( NULL != real_uri ) {
		retval =
		    tm->real_method->unlink(tm->real_method, real_uri, context);

		gnome_vfs_uri_unref(real_uri);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	return retval;
}

static GnomeVFSResult
tr_do_check_same_fs(GnomeVFSMethod * method,
		    GnomeVFSURI * a,
		    GnomeVFSURI * b,
		    gboolean * same_fs_return, GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri_a, *real_uri_b;
	GnomeVFSResult retval;

	real_uri_a = tr_uri_translate(tm, a);
	real_uri_b = tr_uri_translate(tm, b);

	if ( NULL != real_uri_a && NULL != real_uri_b ) {
		retval =
		    tm->real_method->check_same_fs(tm->real_method, real_uri_a,
						   real_uri_b, same_fs_return,
						   context);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	if (real_uri_a) {gnome_vfs_uri_unref(real_uri_a);}
	if (real_uri_b) {gnome_vfs_uri_unref(real_uri_b);}

	return retval;
}

static GnomeVFSResult
tr_do_set_file_info(GnomeVFSMethod * method,
		    GnomeVFSURI * a,
		    const GnomeVFSFileInfo * info,
		    GnomeVFSSetFileInfoMask mask,
		    GnomeVFSContext * context)
{
	TranslateMethod *tm = (TranslateMethod *) method;
	GnomeVFSURI *real_uri_a;
	GnomeVFSResult retval;

	real_uri_a = tr_uri_translate(tm, a);

	if ( NULL != real_uri_a ) {
		retval =
		    tm->real_method->set_file_info(tm->real_method, real_uri_a,
						   info, mask, context);

		gnome_vfs_uri_unref(real_uri_a);
	} else {
		retval = GNOME_VFS_ERROR_NOT_FOUND;
	}

	return retval;
}

/******** from poptparse.c:
  Copyright (c) 1998  Red Hat Software

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  Except as contained in this notice, the name of the X Consortium shall not be
  used in advertising or otherwise to promote the sale, use or other dealings
  in this Software without prior written authorization from the X Consortium.

*/

#define POPT_ARGV_ARRAY_GROW_DELTA 5
static int my_poptParseArgvString(char *buf, int *argcPtr, char ***argvPtr)
{
	char *src;
	char quote = '\0';
	int argvAlloced = POPT_ARGV_ARRAY_GROW_DELTA;
	char **argv = g_new(char *, argvAlloced);
	int argc = 0;
	char *s;

	s = alloca(strlen(buf) + 1);
	strcpy(s, buf);

	argv[argc] = buf;

	for (src = s; *src; src++) {
		if (quote == *src) {
			quote = '\0';
		} else if (quote) {
			if (*src == '\\') {
				src++;
				if (!*src) {
					g_free(argv);
					return -1;
				}
				if (*src != quote)
					*(buf++) = '\\';
			}
			*(buf++) = *src;
		} else if (isspace((unsigned char)*src)) {
			*buf = '\0';
			if (*argv[argc]) {
				buf++, argc++;
				/* leave one empty argv at the end */
				if (argc == argvAlloced - 1) {
					argvAlloced +=
					    POPT_ARGV_ARRAY_GROW_DELTA;
					argv =
					    g_realloc(argv,
						      sizeof(*argv) *
						      argvAlloced);
				}
				argv[argc] = buf;
			}
		} else
			switch (*src) {
			case '"':
			case '\'':
				quote = *src;
				break;
			case '\\':
				src++;
				if (!*src) {
					g_free(argv);
					return -1;
				}
				/*@fallthrough@ */
			default:
				*(buf++) = *src;
				break;
			}
	}

	*buf = '\0';
	if (strlen(argv[argc])) {
		argc++, buf++;
	}

	/* add NULL to the end of argv */
	argv[argc] = NULL;

	*argcPtr = argc;
	*argvPtr = argv;

	return 0;
}

static gboolean tr_args_parse(ParsedArgs * pa, const char *args)
{
	char **argv;
	int argc;
	char *tmp_args;
	int i;
	gboolean badargs = FALSE;
	gboolean mode_determined = FALSE;

	memset(pa, 0, sizeof(ParsedArgs));

	tmp_args = alloca(strlen(args) + 1);
	strcpy(tmp_args, args);

	if (my_poptParseArgvString(tmp_args, &argc, &argv)) {
		g_warning("Failed to parse arguments: %s", args);
		return FALSE;
	}

	for (i = 0; i < argc; i++) {
#define CHECK_ARG() if((++i) >= argc) { badargs = TRUE; goto out; }
#define CHECK_MODE(my_mode) if (mode_determined && pa->mode != (my_mode)) { badargs = TRUE ; goto out; } else { pa->mode = my_mode; mode_determined = TRUE; }
		if (g_strcasecmp(argv[i], "-pattern") == 0) {
			CHECK_ARG();
			CHECK_MODE(MODE_BASIC);
			pa->u.basic.trans_string = g_strdup(argv[i]);
		} else if (g_strcasecmp(argv[i], "-real-method") == 0) {
			CHECK_ARG();
			pa->real_method_name = g_strdup(argv[i]);
		} else if (g_strcasecmp(argv[i], "-exec") == 0) {
			CHECK_ARG();
			CHECK_MODE(MODE_EXEC);
			pa->u.exec.orig_string = g_strdup(argv[i]);
		} else if (g_strcasecmp(argv[i], "-retain") == 0) {
			CHECK_MODE(MODE_EXEC);
			pa->u.exec.retain = TRUE;
		} else if (g_strcasecmp(argv[i], "-default-mime-type") == 0) {
			CHECK_ARG();
			pa->default_mime_type = g_strdup(argv[i]);
		} else {
			g_warning("Unknown option `%s'", argv[i]);
			badargs = TRUE;
			goto out;
		}
#undef CHECK_ARG
#undef CHECK_MODE
	}

	if ( ! mode_determined ) {
		g_warning("Need a -pattern option or -exec option");
		badargs = TRUE;
	} else if (MODE_BASIC == pa->mode) {
		if (!pa->real_method_name) {
			g_warning("Need a -real-method option");
			badargs = TRUE;
		} else if (!pa->u.basic.trans_string) {
			g_warning("Need a -pattern option");
			badargs = TRUE;
		}
	} else if (MODE_EXEC == pa->mode) {
		if (!pa->real_method_name) {
			g_warning("Need a -real-method option");
			badargs = TRUE;
		} else if (!pa->u.exec.orig_string) {
			g_warning("Need a -exec option");
			badargs = TRUE;
		}

		/* Chop up the exec string here */
		if (my_poptParseArgvString (
			pa->u.exec.orig_string, 
			&(pa->u.exec.argc), 
			&(pa->u.exec.argv))
		) {
			g_warning ("Failed to parse -exec args");
			badargs = TRUE;
		}
	} else {
		g_assert (FALSE);
	}
	
      out:
	g_free(argv);
	return !badargs;
}

static GnomeVFSMethod base_vfs_method = {
	sizeof (GnomeVFSMethod),
	tr_do_open,
	tr_do_create,
	tr_do_close,
	tr_do_read,
	tr_do_write,
	tr_do_seek,
	tr_do_tell,
	tr_do_truncate_handle,
	tr_do_open_directory,
	tr_do_close_directory,
	tr_do_read_directory,
	tr_do_get_file_info,
	tr_do_get_file_info_from_handle,
	tr_do_is_local,
	tr_do_make_directory,
	tr_do_remove_directory,
	tr_do_move,
	tr_do_unlink,
	tr_do_check_same_fs,
	tr_do_set_file_info,
	tr_do_truncate,
	tr_do_find_directory,
	NULL				/* create_symbolic_link can't be supported */
	/* Hey YOU!  If you add a GnomeVFS method, you need to do two things
	 * in the translate-method module:
	 * 
	 * 1. Add a line to the CHECK_NULL_METHOD list in vfs_module_init
	 * 2. Implement the function in this module.  This is pretty simple--
	 *    just follow the pattern of existing functions
	 */ 
};

static void tr_args_free(ParsedArgs * pa)
{
	g_free(pa->default_mime_type);
	g_free(pa->real_method_name);

	if (MODE_BASIC == pa->mode) {
		g_free(pa->u.basic.trans_string);
	} else {
		g_free(pa->u.exec.orig_string);
	}
}

GnomeVFSMethod *vfs_module_init(const char *method_name, const char *args)
{
	TranslateMethod *retval;
	ParsedArgs pa;

	if (!tr_args_parse(&pa, args))
		return NULL;

	retval = g_new0(TranslateMethod, 1);
	retval->pa = pa;
	retval->real_method = gnome_vfs_method_get(pa.real_method_name);

	if (!retval->real_method) {
		tr_args_free(&retval->pa);
		g_free(retval);
		return NULL;
	}

	tr_exec_init(&(retval->exec_state));

	retval->base_method = base_vfs_method;
 
#define CHECK_NULL_METHOD(x) if(!VFS_METHOD_HAS_FUNC (retval->real_method, x)) retval->base_method.x = NULL
	CHECK_NULL_METHOD(open);
	CHECK_NULL_METHOD(create);
	CHECK_NULL_METHOD(close);
	CHECK_NULL_METHOD(read);
	CHECK_NULL_METHOD(write);
	CHECK_NULL_METHOD(seek);
	CHECK_NULL_METHOD(tell);
	CHECK_NULL_METHOD(truncate);
	CHECK_NULL_METHOD(open_directory);
	CHECK_NULL_METHOD(close_directory);
	CHECK_NULL_METHOD(read_directory);
	CHECK_NULL_METHOD(get_file_info);
	CHECK_NULL_METHOD(get_file_info_from_handle);
	CHECK_NULL_METHOD(is_local);
	CHECK_NULL_METHOD(make_directory);
	CHECK_NULL_METHOD(remove_directory);
	CHECK_NULL_METHOD(move);
	CHECK_NULL_METHOD(unlink);
	CHECK_NULL_METHOD(check_same_fs);
	CHECK_NULL_METHOD(set_file_info);
	CHECK_NULL_METHOD(truncate_handle);
	CHECK_NULL_METHOD(find_directory);
	/*CHECK_NULL_METHOD(create_symbolic_link);*/
	retval->base_method.create_symbolic_link = NULL;
#undef CHECK_NULL_METHOD

	return (GnomeVFSMethod *) retval;
}

void
vfs_module_shutdown (GnomeVFSMethod * method)
{
	TranslateMethod *tmethod = (TranslateMethod *) method;

	tr_exec_cleanup (&(tmethod->exec_state));

	tr_args_free(&tmethod->pa);

	g_free(tmethod);
}
