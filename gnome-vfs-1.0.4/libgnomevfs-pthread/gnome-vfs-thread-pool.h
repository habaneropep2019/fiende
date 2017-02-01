/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-thread-pool.h - Simple thread pool implementation

   Copyright (C) 2000 Eazel, Inc.

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

   Author: Pavel Cisler <pavel@eazel.com>
*/

#ifndef GNOME_VFS_THREAD_POOL
#define GNOME_VFS_THREAD_POOL

#include <pthread.h>

int gnome_vfs_thread_create (pthread_t *thread, void *(* thread_routine) (void *),
	void *thread_arguments);
/* pthread_create replacement */

void gnome_vfs_thread_pool_init     (void);
void gnome_vfs_thread_pool_shutdown (void);
/* called during app shutdown, quit all the threads from the pool */

#endif
