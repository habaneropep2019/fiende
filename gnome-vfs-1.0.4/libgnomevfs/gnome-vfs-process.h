/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-process.h - Unified method for executing external processes.

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

#ifndef GNOME_VFS_PROCESS_H
#define GNOME_VFS_PROCESS_H

#include <glib.h>
#include <signal.h>		/* For the signal values.  */

typedef enum {
	GNOME_VFS_PROCESS_OK,
	GNOME_VFS_PROCESS_ERROR_UNKNOWN,
	GNOME_VFS_PROCESS_ERROR_INVALIDSIGNAL,
	GNOME_VFS_PROCESS_ERROR_NOPERM,
	GNOME_VFS_PROCESS_ERROR_NOPROCESS
} GnomeVFSProcessResult;

typedef enum {
	GNOME_VFS_PROCESS_RUN_OK,
	GNOME_VFS_PROCESS_RUN_ERROR,
	GNOME_VFS_PROCESS_RUN_CANCELLED,
	GNOME_VFS_PROCESS_RUN_SIGNALED,
	GNOME_VFS_PROCESS_RUN_STOPPED
} GnomeVFSProcessRunResult;

typedef enum {
	GNOME_VFS_PROCESS_DEFAULT = 0,
	GNOME_VFS_PROCESS_USEPATH = 1 << 0,
	GNOME_VFS_PROCESS_CLOSEFDS = 1 << 1,
	GNOME_VFS_PROCESS_SETSID = 1 << 2
} GnomeVFSProcessOptions;

typedef struct _GnomeVFSProcess GnomeVFSProcess;

typedef void (* GnomeVFSProcessInitFunc) (gpointer data);

typedef void (* GnomeVFSProcessCallback) (GnomeVFSProcess *process,
					  gint status,
					  gpointer data);

gboolean              gnome_vfs_process_init   (void);
GnomeVFSProcess *     gnome_vfs_process_new    (const gchar             *file_name,
						const gchar * const      argv[],
						GnomeVFSProcessOptions   options,
						GnomeVFSProcessInitFunc  init_func,
						gpointer                 init_data,
						GnomeVFSProcessCallback  callback,
						gpointer                 callback_data);
GnomeVFSProcessResult gnome_vfs_process_signal (GnomeVFSProcess         *process,
						guint                    signal_number);
void                  gnome_vfs_process_free   (GnomeVFSProcess         *process);

#endif /* GNOME_VFS_PROCESS_H */
