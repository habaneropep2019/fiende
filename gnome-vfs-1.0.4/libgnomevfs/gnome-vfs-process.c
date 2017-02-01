/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-process.c - Unified method for executing external processes.

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

   Author: Ettore Perazzoli <ettore@gnu.org>
*/

/* WARNING: This is *NOT* MT-safe at all.  It is designed to call all processes
   from the main thread exclusively.  But for now this is fine, because we are
   only using this module internally.  */

#include <config.h>
#include "gnome-vfs-process.h"

#include "gnome-vfs-private.h"
#include "gnome-vfs.h"
#include <errno.h>
#include <glib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


/* A launched process.  */
struct _GnomeVFSProcess {
	pid_t pid;
	GnomeVFSProcessCallback callback;
	gpointer callback_data;
};


/* Have we been initialized yet?  */
static gboolean initialized = FALSE;

/* Table to get a pointer to a GnomeVFSProcess struct from a PID value.  */
static GHashTable *pid_to_process = NULL;

/* Input channel for waking up the main loop whenever a SIGCHLD is received,
   and call our SIGCHLD handling callback.  */
static GIOChannel *wake_up_channel_in = NULL;

/* The output side of the previous channel.  We use low-level I/O in the signal
   handler instead of `g_io_*' stuff.  */
static volatile gint wake_up_channel_out_fd = -1;

/* The sigaction we had before installing the SIGCHLD handler.  */
static struct sigaction old_sigchld_action;


static void
foreach_pid_func (gpointer key,
		  gpointer value,
		  gpointer data)
{
	GnomeVFSProcess *process;
	pid_t pid;
	gint status;
	gboolean *found;

	pid = GPOINTER_TO_INT (key);
	process = (GnomeVFSProcess *) value;
	found = (gboolean *) data;

	if (waitpid (pid, &status, WNOHANG) == pid) {
		write (wake_up_channel_out_fd, &process, sizeof (process));
		write (wake_up_channel_out_fd, &status, sizeof (status));
		*found = TRUE;
	}
}

static void
sigchld_handler (int signum)
{
	gboolean found = FALSE;

	found = FALSE;
	g_hash_table_foreach (pid_to_process, foreach_pid_func, &found);

	if (! found && old_sigchld_action.sa_handler != NULL)
		(* old_sigchld_action.sa_handler) (signum);
}

static gboolean
wake_up (GIOChannel *source,
	 GIOCondition condition,
	 gpointer data)
{
	GnomeVFSProcess *process;
	GIOError result;
	guint bytes_read;
	gint status;

	do {
		result = g_io_channel_read (source, (gchar *) &process,
					    sizeof (process), &bytes_read);
	} while (result == G_IO_ERROR_AGAIN);
	if (result != G_IO_ERROR_NONE) {
		g_warning (__FILE__ ": Cannot read from the notification channel (error %d)",
			   result);
		return TRUE;
	}

	do {
		result = g_io_channel_read (source, (gchar *) &status,
					    sizeof (status), &bytes_read);
	} while (result == G_IO_ERROR_AGAIN);
	if (result != G_IO_ERROR_NONE) {
		g_warning (__FILE__ ": Cannot read from the notification channel (error %d)",
			   result);
		return TRUE;
	}

	if (process->callback != NULL)
		(* process->callback) (process, status,
				       process->callback_data);

	if (WIFSIGNALED (status)) {
		g_hash_table_remove (pid_to_process,
				     GINT_TO_POINTER (process->pid));
		gnome_vfs_process_free (process);
	}

	return TRUE;
}


gboolean
gnome_vfs_process_init (void)
{
	gint pipe_fd[2];
	struct sigaction sigchld_action;
	sigset_t sigchld_mask;

	if (initialized)
		return TRUE;

	if (pipe (pipe_fd) == -1) {
		g_warning ("Cannot create pipe for GnomeVFSProcess initialization: %s",
			   g_strerror (errno));
		return FALSE;
	}

	sigchld_action.sa_handler =  sigchld_handler;
	sigemptyset (&sigchld_action.sa_mask);
	sigchld_action.sa_flags = 0;

	sigaction (SIGCHLD, &sigchld_action, &old_sigchld_action);

	pid_to_process = g_hash_table_new (NULL, NULL);

	wake_up_channel_in = g_io_channel_unix_new (pipe_fd[0]);
	wake_up_channel_out_fd = pipe_fd[1];

	g_io_add_watch (wake_up_channel_in, G_IO_IN, wake_up, NULL);

	sigemptyset (&sigchld_mask);
	sigaddset (&sigchld_mask, SIGCHLD);
	sigprocmask (SIG_UNBLOCK, &sigchld_mask, NULL);

	return TRUE;
}


/**
 * gnome_vfs_process_new:
 * @file_name: Name of the executable.
 * @argv: NULL-terminated parameter list.
 * @use_search_path: If TRUE, use the `PATH' environment variable to locate
 * the executable.
 * @close_file_descriptors: If TRUE, close all the open file descriptors.
 * except stdio, stdin and stderr before launching the process.
 * @init_func: Function to be called before launching the process.
 * @init_data: Value to pass to @init_func.
 * @callback: Function to invoke when the process die.
 * @callback_data: Data to pass to @callback when the process dies.
 * 
 * Launch a new process.  @init_func is called immediately after calling
 * fork(), and before closing the file descriptors and executing the program in
 * the new process.
 * 
 * Return value: An opaque structure describing the launched process.
 **/
GnomeVFSProcess *
gnome_vfs_process_new (const gchar *file_name,
		       const gchar * const argv[],
		       GnomeVFSProcessOptions options,
		       GnomeVFSProcessInitFunc init_func,
		       gpointer init_data,
		       GnomeVFSProcessCallback callback,
		       gpointer callback_data)
{
	GnomeVFSProcess *new;
	sigset_t sigchld_mask, old_mask;
	pid_t child_pid;

	/* Make sure no SIGCHLD happens while we set things up.  */

	sigemptyset (&sigchld_mask);
	sigaddset (&sigchld_mask, SIGCHLD);
	sigprocmask (SIG_BLOCK, &sigchld_mask, &old_mask);

	child_pid = gnome_vfs_forkexec (file_name, argv, options,
					init_func, init_data);

	if (child_pid == -1)
		return NULL;

	new = g_new (GnomeVFSProcess, 1);
	new->pid = child_pid;
	new->callback = callback;
	new->callback_data = callback_data;

	g_hash_table_insert (pid_to_process, GINT_TO_POINTER (child_pid), new);

	sigprocmask (SIG_SETMASK, &old_mask, NULL);

	return new;
}

/**
 * gnome_vfs_process_free:
 * @process: An existing process.
 * 
 * Free @process.  This will not kill the process, but will prevent the
 * associated callbacks to be called.
 **/
void
gnome_vfs_process_free (GnomeVFSProcess *process)
{
	g_hash_table_remove (pid_to_process, GINT_TO_POINTER (process->pid));
	g_free (process);
}

/**
 * gnome_vfs_process_signal:
 * @process: A launched process
 * @signal_number: A signal number
 * 
 * Send signal @signal_number to the specified @process.
 * 
 * Return value: A numeric value reporting the result of the operation.
 **/
GnomeVFSProcessRunResult
gnome_vfs_process_signal (GnomeVFSProcess *process,
			  guint signal_number)
{
	gint kill_result;

	kill_result = kill (process->pid, signal_number);

	switch (kill_result) {
	case 0:
		return GNOME_VFS_PROCESS_OK;
	case EINVAL:
		return GNOME_VFS_PROCESS_ERROR_INVALIDSIGNAL;
	case EPERM:
		return GNOME_VFS_PROCESS_ERROR_NOPERM;
	case ESRCH:
		return GNOME_VFS_PROCESS_ERROR_NOPROCESS;
	default:
		return GNOME_VFS_PROCESS_ERROR_UNKNOWN;
	}
}

