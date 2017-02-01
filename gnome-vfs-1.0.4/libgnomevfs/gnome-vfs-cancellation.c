/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-cancellation.c - Cancellation handling for the GNOME Virtual File
   System access methods.

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

   Author: Ettore Perazzoli <ettore@gnu.org> */

#include <config.h>
#include "gnome-vfs-cancellation.h"

#include "gnome-vfs-private.h"
#include "gnome-vfs.h"
#include <unistd.h>

/* WARNING: this code is not general-purpose.  It is supposed to make the two
   sides of the VFS (i.e. the main process/thread and its asynchronous slave)
   talk in a simple way.  For this reason, only the main process/thread should
   be allowed to call `gnome_vfs_cancellation_cancel()'.  *All* the code is
   based on this assumption.  */


struct GnomeVFSCancellation {
	gboolean cancelled;
	gint pipe_in;
	gint pipe_out;
};


/**
 * gnome_vfs_cancellation_new:
 * 
 * Create a new GnomeVFSCancellation object for reporting cancellation to a
 * GNOME VFS module.
 * 
 * Return value: A pointer to the new GnomeVFSCancellation object.
 **/
GnomeVFSCancellation *
gnome_vfs_cancellation_new (void)
{
	GnomeVFSCancellation *new;
	gint pipefd[2];

	if (pipe (pipefd) == -1)
		return NULL;

	new = g_new (GnomeVFSCancellation, 1);
	new->cancelled = FALSE;
	new->pipe_in = pipefd[0];
	new->pipe_out = pipefd[1];

	return new;
}

/**
 * gnome_vfs_cancellation_destroy:
 * @cancellation: A GnomeVFSCancellation object
 * 
 * Destroy @cancellation.
 **/
void
gnome_vfs_cancellation_destroy (GnomeVFSCancellation *cancellation)
{
	g_return_if_fail (cancellation != NULL);

	close (cancellation->pipe_in);
	close (cancellation->pipe_out);
	g_free (cancellation);
}

/**
 * gnome_vfs_cancellation_cancel:
 * @cancellation: A GnomeVFSCancellation object
 * 
 * Send a cancellation request through @cancellation.
 **/
void
gnome_vfs_cancellation_cancel (GnomeVFSCancellation *cancellation)
{
	g_return_if_fail (cancellation != NULL);

	GNOME_VFS_ASSERT_PRIMARY_THREAD;

	if (cancellation->cancelled)
		return;

	write (cancellation->pipe_out, "c", 1);
	cancellation->cancelled = TRUE;
}

/**
 * gnome_vfs_cancellation_check:
 * @cancellation: A GnomeVFSCancellation object
 * 
 * Check for pending cancellation.
 * 
 * Return value: %TRUE if the operation should be interrupted.
 **/
gboolean
gnome_vfs_cancellation_check (GnomeVFSCancellation *cancellation)
{
	if (cancellation == NULL)
		return FALSE;

	return cancellation->cancelled;
}

/**
 * gnome_vfs_cancellation_ack:
 * @cancellation: A GnomeVFSCancellation object
 * 
 * Acknowledge a cancellation.  This should be called if
 * `gnome_vfs_cancellation_check()' returns %TRUE or if `select()' reports that
 * input is available on the file descriptor returned by
 * `gnome_vfs_cancellation_get_fd()'.
 **/
void
gnome_vfs_cancellation_ack (GnomeVFSCancellation *cancellation)
{
	gchar c;

	if (cancellation == NULL)
		return;

	read (cancellation->pipe_in, &c, 1);
	cancellation->cancelled = FALSE;
}

/**
 * gnome_vfs_cancellation_get_fd:
 * @cancellation: A GnomeVFSCancellation object
 * 
 * Get a file descriptor -based notificator for @cancellation.  When
 * @cancellation receives a cancellation request, a character will be made
 * available on the returned file descriptor for input.
 *
 * This is very useful for detecting cancellation during I/O operations: you
 * can use the `select()' call to check for available input/output on the file
 * you are reading/writing, and on the notificator's file descriptor at the
 * same time.  If a data is available on the notificator's file descriptor, you
 * know you have to cancel the read/write operation.
 * 
 * Return value: the notificator's file descriptor.
 **/
gint
gnome_vfs_cancellation_get_fd (GnomeVFSCancellation *cancellation)
{
	g_return_val_if_fail (cancellation != NULL, -1);

	return cancellation->pipe_in;
}
