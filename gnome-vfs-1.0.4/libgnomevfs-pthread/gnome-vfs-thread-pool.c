/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-thread-pool.c - Simple thread pool implementation

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

#include <glib.h>
#include "gnome-vfs-pthread.h"
#include "gnome-vfs-thread-pool.h"

#undef DEBUG_PRINT

#if 0
#define DEBUG_PRINT(x) g_print x
#else
#define DEBUG_PRINT(x)
#endif

typedef struct {
	pthread_t thread_id;
	pthread_mutex_t waiting_for_work_lock;
	pthread_cond_t waiting_for_work_lock_condition;
	
	void *(* entry_point) (void *);
	void *entry_data;
	
	volatile gboolean exit_requested;
} GnomeVFSThreadState;

static GnomeVFSRecursiveMutex thread_list_lock;

static const int MAX_AVAILABLE_THREADS = 20; 
static GList *available_threads;
static int thread_count;

static void *thread_entry (void *cast_to_state);
static void destroy_thread_state (GnomeVFSThreadState *state);

void 
gnome_vfs_thread_pool_init (void)
{
	gnome_vfs_pthread_recursive_mutex_init (&thread_list_lock);
}

static GnomeVFSThreadState *
new_thread_state (void)
{
	GnomeVFSThreadState *state;
	int result;
	pthread_attr_t thread_attributes;
	
	state = g_new0 (GnomeVFSThreadState, 1);
	
	pthread_mutex_init (&state->waiting_for_work_lock, NULL);	
	pthread_cond_init (&state->waiting_for_work_lock_condition, NULL);

	pthread_attr_init (&thread_attributes);
	pthread_attr_setdetachstate (&thread_attributes, PTHREAD_CREATE_DETACHED);
	/* spawn a new thread, call the entry point immediately -- it will block
	 * until it receives a new entry_point for the first job to execute
	 */
	result = pthread_create (&state->thread_id, &thread_attributes, thread_entry, state);
	pthread_attr_destroy (&thread_attributes);

	DEBUG_PRINT (("new thread %x\n", (guint)state->thread_id));
	
	if (result != 0) {
		destroy_thread_state (state);
		return NULL;
	}
	
	return state;
}

static void
destroy_thread_state (GnomeVFSThreadState *state)
{
	pthread_mutex_destroy (&state->waiting_for_work_lock);
	pthread_cond_destroy (&state->waiting_for_work_lock_condition);
	g_free (state);
}

static gboolean
make_thread_available (GnomeVFSThreadState *state)
{
	/* thread is done with it's work, add it to the available pool */
	gboolean delete_thread = TRUE;

	pthread_mutex_lock (&state->waiting_for_work_lock);
	/* we are done with the last task, clear it out */
	state->entry_point = NULL;
	pthread_mutex_unlock (&state->waiting_for_work_lock);

	gnome_vfs_pthread_recursive_mutex_lock (&thread_list_lock);

	if (thread_count < MAX_AVAILABLE_THREADS) {
		/* haven't hit the max thread limit yet, add the now available
		 * thread to the pool
		 */
		available_threads = g_list_prepend (available_threads, state);
		thread_count++;
		delete_thread = FALSE;
		DEBUG_PRINT (("adding thread %x the pool, %d threads\n",
			(guint)state->thread_id, thread_count));
	}

	gnome_vfs_pthread_recursive_mutex_unlock (&thread_list_lock);
	
	return !delete_thread;
}

static void
gnome_vfs_thread_pool_wait_for_work (GnomeVFSThreadState *state)
{
	/* FIXME: The Eazel profiler should be taught about this call
	 * and ignore any timings it collects from the program hanging out
	 * in here.
	 */

	/* Wait to get scheduled to do some work. */
	DEBUG_PRINT (("thread %x getting ready to wait for work \n",
		(guint)state->thread_id));

	pthread_mutex_lock (&state->waiting_for_work_lock);
	if (state->entry_point != NULL) {
		DEBUG_PRINT (("thread %x ready to work right away \n",
			(guint)state->thread_id));
	} else {
		while (state->entry_point == NULL) {
			/* Don't have any work yet, wait till we get some. */
			DEBUG_PRINT (("thread %x waiting for work \n", (guint)state->thread_id));
			pthread_cond_wait (&state->waiting_for_work_lock_condition,
				&state->waiting_for_work_lock);
		}
	}

	pthread_mutex_unlock (&state->waiting_for_work_lock);
	DEBUG_PRINT (("thread %x woken up\n", (guint)state->thread_id));
}

static void *
thread_entry (void *cast_to_state)
{
	GnomeVFSThreadState *state = (GnomeVFSThreadState *)cast_to_state;

	for (;;) {
		
		if (state->exit_requested) {
			/* We have been explicitly asked to expire */
			break;
		}
		
		gnome_vfs_thread_pool_wait_for_work (state);
		g_assert (state->entry_point);

		/* Enter the actual thread entry point. */
		(*state->entry_point) (state->entry_data);
		
		if (!make_thread_available (state)) {
			/* Available thread pool is full of threads, just let this one
			 * expire.
			 */
			break;
		}
	}

	destroy_thread_state (state);
	return NULL;
}

int 
gnome_vfs_thread_create (pthread_t *thread, void *(* thread_routine) (void *),
	void *thread_arguments)
{
	GnomeVFSThreadState *available_thread;
	
	gnome_vfs_pthread_recursive_mutex_lock (&thread_list_lock);
	if (available_threads == NULL) {
		/* Thread pool empty, create a new thread. */
		available_thread = new_thread_state ();
	} else {
		/* Pick the next available thread from the list. */
		available_thread = (GnomeVFSThreadState *)available_threads->data;
		available_threads = g_list_remove (available_threads, available_thread);
		thread_count--;
		DEBUG_PRINT (("got thread %x from the pool, %d threads left\n",
			(guint)available_thread->thread_id, thread_count));
		

	}
	gnome_vfs_pthread_recursive_mutex_unlock (&thread_list_lock);
	
	if (available_thread == NULL) {
		/* Failed to allocate a new thread. */
		return -1;
	}
	
	/* Lock it so we can condition-signal it next. */
	pthread_mutex_lock (&available_thread->waiting_for_work_lock);

	/* Prepare work for the thread. */
	available_thread->entry_point = thread_routine;
	available_thread->entry_data = thread_arguments;
	
	*thread = available_thread->thread_id;
	
	/* Unleash the thread. */
	DEBUG_PRINT (("waking up thread %x\n", (guint)available_thread->thread_id));
	pthread_cond_signal (&available_thread->waiting_for_work_lock_condition);
	pthread_mutex_unlock (&available_thread->waiting_for_work_lock);
	
	
	return 0;
}

void 
gnome_vfs_thread_pool_shutdown (void)
{
	GnomeVFSThreadState *thread_state;
	for (;;) {
		thread_state = NULL;
		
		gnome_vfs_pthread_recursive_mutex_lock (&thread_list_lock);
		if (available_threads != NULL) {
			/* Pick the next thread from the list. */
			thread_state = (GnomeVFSThreadState *)available_threads->data;
			available_threads = g_list_remove (available_threads, thread_state);
		}
		gnome_vfs_pthread_recursive_mutex_unlock (&thread_list_lock);
		
		if (thread_state == NULL) {
			break;
		}
		
		pthread_mutex_lock (&thread_state->waiting_for_work_lock);
		/* Tell the thread to expire. */
		thread_state->exit_requested = TRUE;
		pthread_cond_signal (&thread_state->waiting_for_work_lock_condition);
		pthread_mutex_unlock (&thread_state->waiting_for_work_lock);
	}
}
