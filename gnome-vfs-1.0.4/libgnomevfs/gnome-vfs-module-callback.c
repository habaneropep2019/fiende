/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*

   Copyright (C) 2001 Eazel, Inc
   Copyright (C) 2001 Maciej Stachowiak

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

   Author: Michael Fleming <mfleming@eazel.com>
           Maciej Stachowiak <mjs@enoisehavoc.org>
*/


#include <config.h>
#include "gnome-vfs-module-callback.h"

#include "gnome-vfs-module-callback-module-api.h"
#include "gnome-vfs-module-callback-private.h"
#include "gnome-vfs-backend-private.h"

#include <pthread.h>


/* -- Private data structure declarations -- */

typedef struct CallbackInfo {
	GnomeVFSModuleCallback callback;
	gpointer callback_data;
	GDestroyNotify destroy_notify;
	int ref_count;
} CallbackInfo;

typedef struct AsyncCallbackInfo {
	GnomeVFSAsyncModuleCallback callback;
	gpointer callback_data;
	GDestroyNotify destroy_notify;
} AsyncCallbackInfo;

typedef struct CallbackResponseData {
	gboolean done;
} CallbackResponseData;

struct GnomeVFSModuleCallbackStackInfo {
	GHashTable *current_callbacks;
	GHashTable *current_async_callbacks;
};


/* -- Global variables -- */

static pthread_mutex_t callback_table_lock = PTHREAD_MUTEX_INITIALIZER;
static GHashTable *default_callbacks = NULL;
static GHashTable *default_async_callbacks = NULL;
static GHashTable *stack_tables_to_free = NULL;

static pthread_once_t stack_keys_once = PTHREAD_ONCE_INIT;
static pthread_key_t callback_stacks_key;
static pthread_key_t async_callback_stacks_key;
static pthread_key_t in_async_thread_key;

static pthread_mutex_t async_callback_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t async_callback_cond = PTHREAD_COND_INITIALIZER;

/* -- Helper functions -- */

/* managing callback structs */

static CallbackInfo *
callback_info_new (GnomeVFSModuleCallback callback_func,
		   gpointer callback_data,
		   GDestroyNotify notify)
{
	CallbackInfo *callback;

	callback = g_new (CallbackInfo, 1);

	callback->callback = callback_func;
	callback->callback_data = callback_data;
	callback->destroy_notify = notify;
	callback->ref_count = 1;

	return callback;
}

#include <stdio.h>

static void
callback_info_ref (CallbackInfo *callback)
{
	callback->ref_count++;
}

static void
callback_info_unref (CallbackInfo *callback)
{
	callback->ref_count--;
	
	if (callback->ref_count == 0) {
		if (callback->destroy_notify != NULL) {
			callback->destroy_notify (callback->callback_data);
		}
		g_free (callback); 
	}
}

/* code for handling async callbacks */

static void
async_callback_response (gpointer data)
{
	CallbackResponseData *response_data;
	
	pthread_mutex_lock (&async_callback_mutex);
	response_data = data;
	response_data->done = TRUE;
	pthread_mutex_unlock (&async_callback_mutex);

	pthread_cond_broadcast (&async_callback_cond);
}

static void
async_callback_invoke (gconstpointer in,
		       gsize         in_size,
		       gpointer      out,
		       gsize         out_size,
		       gpointer      callback_data)
{
	AsyncCallbackInfo *async_callback;
	CallbackResponseData response_data;
	
	response_data.done = FALSE;
	async_callback = callback_data;
	
	/* Using a single mutex and condition variable could mean bad
	 * performance if many async callbacks are active at once but
	 * this is unlikeley, so we avoid the overhead of creating
	 * new mutexes and condition variables all the time.
	 */
	
	pthread_mutex_lock (&async_callback_mutex);
	gnome_vfs_backend_dispatch_module_callback (async_callback->callback,
						    in, in_size,
						    out, out_size,
						    async_callback->callback_data, 
						    async_callback_response,
						    &response_data);
	
	while (!response_data.done) {
		pthread_cond_wait (&async_callback_cond, &async_callback_mutex);
	}
	pthread_mutex_unlock (&async_callback_mutex);
}

static void
async_callback_destroy (gpointer callback_data)
{
	AsyncCallbackInfo *async_callback;

	async_callback = callback_data;

	if (async_callback->destroy_notify != NULL) {
		async_callback->destroy_notify (async_callback->callback_data);
	}

	g_free (async_callback);

}

static CallbackInfo *
async_callback_info_new (GnomeVFSAsyncModuleCallback callback_func,
		    gpointer callback_data,
		    GDestroyNotify notify)
{
	AsyncCallbackInfo *async_callback;

	async_callback = g_new (AsyncCallbackInfo, 1);

	async_callback->callback = callback_func;
	async_callback->callback_data = callback_data;
	async_callback->destroy_notify = notify;

	return callback_info_new (async_callback_invoke, async_callback, async_callback_destroy);
}



/* Adding items to hash tables or stack tables */
static void
insert_callback_into_table (GHashTable *table,
			    const char *callback_name,
			    CallbackInfo *callback)
{
	gpointer orig_key;
	gpointer old_value;
	
	callback_info_ref (callback);

	if (g_hash_table_lookup_extended (table,
					  callback_name,
					  &orig_key,
					  &old_value)) {
		g_hash_table_remove (table, orig_key);
		g_free (orig_key);
		callback_info_unref ((CallbackInfo *) old_value);
	}

	g_hash_table_insert (table,
			     g_strdup (callback_name),
			     callback);
}

static void
push_callback_into_stack_table (GHashTable *table,
				const char *callback_name,
				CallbackInfo *callback)
{
	gpointer orig_key;
	gpointer old_value;
	GSList *stack;
	
	callback_info_ref (callback);
	
	if (g_hash_table_lookup_extended (table,
					  callback_name,
					  &orig_key,
					  &old_value)) {
		g_hash_table_remove (table, orig_key);
		g_free (orig_key);
		stack = old_value;
	} else {
		stack = NULL;
	}
	
	stack = g_slist_prepend (stack, callback);
	
	g_hash_table_insert (table,
			     g_strdup (callback_name),
			     stack);
}

static void
pop_stack_table (GHashTable *table,
		 const char *callback_name)
{
	GSList *stack;
	GSList *first_link;
	gpointer orig_key;
	gpointer old_value;
	
	if (g_hash_table_lookup_extended (table,
					  callback_name,
					  &orig_key,
					  &old_value)) {
		g_hash_table_remove (table, orig_key);
		g_free (orig_key);
		stack = old_value;
	} else {
		return;
	}
	
	/* Would not be in the hash table if it were NULL */
	g_assert (stack != NULL);
	
	callback_info_unref ((CallbackInfo *) stack->data);
	
	first_link = stack;
	stack = stack->next;
	g_slist_free_1 (first_link);
	
	if (stack != NULL) {
		g_hash_table_insert (table,
				     g_strdup (callback_name),
				     stack);
	}
}


/* Functions to copy, duplicate and clear callback tables and callback
 * stack tables, and helpers for these functions.
 */

static void
copy_one_callback  (gpointer	key,
		    gpointer	value,
		    gpointer	callback_data)
{
	const char *callback_name;
	CallbackInfo *callback;
	GHashTable *table;
	
	callback_name = key;
	callback = value;
	table = callback_data;
	
	insert_callback_into_table (table, callback_name, callback);
}

static void
copy_one_stack_top (gpointer key,
		    gpointer value,
		    gpointer callback_data)
{
	GSList *stack;
	const char *callback_name;
	CallbackInfo *callback;
	GHashTable *table;
	
	callback_name = key;
	stack = value;
	callback = stack->data;
	table = callback_data;
	
	insert_callback_into_table (table, callback_name, callback);
}

static void
copy_one_callback_to_stack (gpointer key,
			    gpointer value,
			    gpointer callback_data)
{
	const char *callback_name;
	CallbackInfo *callback;
	GHashTable *table;
	
	callback_name = key;
	callback = value;
	table = callback_data;
	
	push_callback_into_stack_table (table, callback_name, callback);
}

static GHashTable *
duplicate_callback_table (GHashTable *table)
{
	GHashTable *copy;
	
	copy = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_foreach (table, copy_one_callback, copy);
	
	return copy;
}

static void
copy_callback_stack_tops (GHashTable *source, 
			  GHashTable *target)
{
	g_hash_table_foreach (source,
			      copy_one_stack_top,
			      target);
}

static void
copy_callback_table_to_stack_table  (GHashTable *source, 
				     GHashTable *target)
{
	g_hash_table_foreach (source,
			      copy_one_callback_to_stack,
			      target);
}

static void
callback_info_unref_func (gpointer data,
			  gpointer callback_data)
{
	callback_info_unref ((CallbackInfo *) data); 
}

static gboolean
remove_one_stack (gpointer key,
		  gpointer value,
		  gpointer callback_data)
{
	char *callback_name;
	GSList *stack;
	
	callback_name = key;
	stack = value;

	g_free (callback_name);
	g_slist_foreach (stack, callback_info_unref_func, NULL);
	g_slist_free (stack);

	return TRUE;
}

static gboolean
remove_one_callback (gpointer key,
		     gpointer value,
		     gpointer callback_data)
{
	char *callback_name;
	CallbackInfo *callback;
	
	callback_name = key;
	callback = value;
	
	g_free (callback_name);
	callback_info_unref (callback);
	
	return TRUE;
}

static void
clear_stack_table (GHashTable *stack_table)
{
	g_hash_table_foreach_remove (stack_table, 
				     remove_one_stack, 
				     NULL);
}

static void
clear_callback_table (GHashTable *stack_table)
{
	g_hash_table_foreach_remove (stack_table,
				     remove_one_callback, 
				     NULL);
}


/* Functions to inialize global and per-thread data on demand and
 * associated cleanup functions. 
 */
static void
stack_table_destroy (gpointer specific)
{
	GHashTable *stack_table;

	stack_table = specific;

	pthread_mutex_lock (&callback_table_lock);
	g_hash_table_remove (stack_tables_to_free, stack_table);
	pthread_mutex_unlock (&callback_table_lock);

	clear_stack_table (stack_table);
	g_hash_table_destroy (stack_table);
}

static gboolean
stack_table_free_hr_func (gpointer key,
			  gpointer value,
			  gpointer callback_data)
{
	GHashTable *table;

	table = key;

	clear_stack_table (table); 
	g_hash_table_destroy (table); 

	return TRUE;
}


static void
free_stack_tables_to_free (void)
{
	pthread_mutex_lock (&callback_table_lock);
	g_hash_table_foreach_remove (stack_tables_to_free, stack_table_free_hr_func , NULL);
	g_hash_table_destroy (stack_tables_to_free);
	pthread_mutex_unlock (&callback_table_lock);
}

static void
stack_keys_alloc (void)
{
	pthread_key_create (&callback_stacks_key, stack_table_destroy);
	pthread_key_create (&async_callback_stacks_key, stack_table_destroy);
	pthread_key_create (&in_async_thread_key, NULL);

	pthread_mutex_lock (&callback_table_lock);
	stack_tables_to_free = g_hash_table_new (g_direct_hash, g_direct_equal);
	pthread_mutex_unlock (&callback_table_lock);

	g_atexit (free_stack_tables_to_free);
}

static void
free_default_callbacks (void)
{
	pthread_mutex_lock (&callback_table_lock);

	clear_callback_table (default_callbacks);
	g_hash_table_destroy (default_callbacks);

	clear_callback_table (default_async_callbacks);
	g_hash_table_destroy (default_async_callbacks);

	pthread_mutex_unlock (&callback_table_lock);
}

/* This function should only be called with the mutex held. */
static void
initialize_global_if_needed (void)
{
	if (default_callbacks == NULL) {
		default_callbacks = g_hash_table_new (g_str_hash, g_str_equal);
		default_async_callbacks = g_hash_table_new (g_str_hash, g_str_equal);

		g_atexit (free_default_callbacks);
	}
}

/* No need for a mutex, since it's all per-thread data. */
static void
initialize_per_thread_if_needed (void)
{
	/* Initialize keys for thread-specific data, once per program. */
	pthread_once (&stack_keys_once, stack_keys_alloc);

	/* Initialize per-thread data, if needed. */
	if (pthread_getspecific (callback_stacks_key) == NULL) {
		pthread_mutex_lock (&callback_table_lock);
		pthread_setspecific (callback_stacks_key,
				     g_hash_table_new (g_str_hash, g_str_equal));
		g_hash_table_insert (stack_tables_to_free,
				     pthread_getspecific (callback_stacks_key),
				     GINT_TO_POINTER (1));
		pthread_mutex_unlock (&callback_table_lock);
	}

	if (pthread_getspecific (async_callback_stacks_key) == NULL) {
		pthread_mutex_lock (&callback_table_lock);
		pthread_setspecific (async_callback_stacks_key,
				     g_hash_table_new (g_str_hash, g_str_equal));
		g_hash_table_insert (stack_tables_to_free,
				     pthread_getspecific (async_callback_stacks_key),
				     GINT_TO_POINTER (1));
		pthread_mutex_unlock (&callback_table_lock);
	}
}

/* -- Public entry points -- */

/**
 * GnomeVFSModuleCallback
 * @in: The in argument for this callback; the exact type depends on the specific callback
 * @in_size: Size of the in argument; useful for sanity-checking
 * @out: The out argument for this callback; the exact type depends on the specific callback
 * @out_size: Size of the out argument; useful for sanity-checking
 * @callback_data: The @callback_data specified when this callback was set
 *
 * This is the type of a callback function that gets set for a module
 * callback. 
 *
 * When the callback is invoked, the user function is called with an
 * @in argument, the exact type of which depends on the specific
 * callback. It is generally a pointer to a struct with several fields
 * that provide information to the callback.
 *
 * The @out argument is used to return a values from the
 * callback. Once again the exact type depends on the specific
 * callback. It is generally a pointer to a pre-allocated struct with
 * several fields that the callback function should fill in before
 * returning.
 *    
 */


/**
 * GnomeVFSModuleCallbackResponse
 * @response_data: Pass the @response_data argument originally passed to the async callback
 *
 * This is the type of the response function passed to a
 * GnomeVFSAsyncModuleCallback(). It should be called when the async
 * callback has completed.  
 */


/**
 * GnomeVFSAsyncModuleCallback
 * @in: The in argument for this callback; the exact type depends on the specific callback
 * @in_size: Size of the in argument; useful for sanity-checking
 * @out: The out argument for this callback; the exact type depends on the specific callback
 * @out_size: Size of the out argument; useful for sanity-checking
 * @callback_data: The @callback_data specified when this callback was set
 * @response: Response function to call when the callback is completed
 * @response_data: Argument to pass to @response
 *
 * This is the type of a callback function that gets set for an async
 * module callback. 
 *
 * Such callbacks are useful when you are using the API and want
 * callbacks to be handled from the main thread, for instance if they
 * need to put up a dialog.
 *
 * Like a GnomeVFSModuleCallback(), an async callback has @in and @out
 * arguments for passing data into and out of the callback. However,
 * an async callback does not need to fill in the @out argument before
 * returning. Instead, it can arrange to have the work done from a
 * callback on the main loop, from another thread, etc. The @response
 * function should be called by whatever code finishes the work of the
 * callback with @response_data as an argument once the @out argument
 * is filled in and the callback is done.
 *
 * The @in and @out arguments are guaranteed to remain valid until the
 * @response function is called.
 * 
 */





/**
 * gnome_vfs_module_callback_set_default
 * @callback_name: The name of the module callback to set
 * @callback: The function to call when the callback is invoked
 * @callback_data: Pointer to pass as the @callback_data argument to @callback
 * @destroy_notify: Function to call when @callback_data is to be freed.
 * 
 * Set the default callback for @callback_name to
 * @callback. @callback will be called with @callback_data on the
 * same thread as the gnome-vfs operation that invokes it. The default
 * value is shared for all threads, but setting it is thread-safe.
 *
 * Use this function if you want to set a handler to be used by your
 * whole application. You can use gnome_vfs_module_callback_push() to
 * set a callback function that will temporarily override the default
 * on the current thread instead. Or you can also use
 * gnome_vfs_async_module_callback_set_default() to set an async
 * callback function.
 *
 * Note: @destroy_notify may be called on any thread - it is not
 * guaranteed to be called on the main thread.
 *
 **/
void
gnome_vfs_module_callback_set_default (const char *callback_name,
				       GnomeVFSModuleCallback callback,
				       gpointer callback_data,
				       GDestroyNotify destroy_notify)
{
	CallbackInfo *callback_info;

	callback_info = callback_info_new (callback, callback_data, destroy_notify);

	pthread_mutex_lock (&callback_table_lock);

	initialize_global_if_needed ();
	insert_callback_into_table (default_callbacks, callback_name, callback_info);

	pthread_mutex_unlock (&callback_table_lock);

	callback_info_unref (callback_info);
}

/**
 * gnome_vfs_module_callback_push
 * @callback_name: The name of the module callback to set temporarily
 * @callback: The function to call when the callback is invoked
 * @callback_data: Pointer to pass as the @callback_data argument to @callback
 * @destroy_notify: Function to call when @callback_data is to be freed.
 * 
 * Set @callback as a temprary handler for @callback_name. @callback
 * will be called with @callback_data on the same thread as the
 * gnome-vfs operation that invokes it. The temporary handler is set
 * per-thread.
 *
 * gnome_vfs_module_callback_pop() removes the most recently set
 * temporary handler. The temporary handlers are treated as a first-in
 * first-out stack.
 *
 * Use this function to set a temporary callback handler for a single
 * call or a few calls. You can use
 * gnome_vfs_module_callback_set_default() to set a callback function
 * that will establish a permanent global setting for all threads
 * instead.
 *
 * Note: @destroy_notify may be called on any thread - it is not
 * guaranteed to be called on the main thread.
 *
 **/
void
gnome_vfs_module_callback_push (const char *callback_name,
				GnomeVFSModuleCallback callback,
				gpointer callback_data,
				GDestroyNotify notify)
{
	CallbackInfo *callback_info;

	initialize_per_thread_if_needed ();

	callback_info = callback_info_new (callback, callback_data, notify);
	push_callback_into_stack_table (pthread_getspecific (callback_stacks_key),
					callback_name,
					callback_info);
	callback_info_unref (callback_info);
}

/**
 * gnome_vfs_module_callback_pop
 * @callback_name: The name of the module callback to remove a temporary handler for
 * 
 * Remove the temporary handler for @callback_name most recently set
 * with gnome_vfs_module_callback_push().  If another temporary
 * handler was previously set on the same thread, it becomes the
 * current handler. Otherwise, the default handler, if any, becomes
 * current. 
 *
 * The temporary handlers are treated as a first-in first-out
 * stack.
 *
 **/
void
gnome_vfs_module_callback_pop (const char *callback_name)
{
	initialize_per_thread_if_needed ();
	pop_stack_table (pthread_getspecific (callback_stacks_key), 
			 callback_name);
}


/**
 * gnome_vfs_async_module_callback_set_default
 * @callback_name: The name of the async module callback to set
 * @callback: The function to call when the callback is invoked
 * @callback_data: Pointer to pass as the @callback_data argument to @callback
 * @destroy_notify: Function to call when @callback_data is to be freed.
 * 
 * Set the default async callback for @callback_name to
 * @callback. @callback will be called with @callback_data
 * from a callback on the main thread. It will be passed a response
 * function which should be called to signal completion of the callback.
 * The callback function itself may return in the meantime.
 *
 * The default value is shared for all threads, but setting it is
 * thread-safe.
 *
 * Use this function if you want to globally set a callback handler
 * for use with async operations.
 *
 * You can use gnome_vfs_async_module_callback_push() to set an async
 * callback function that will temporarily override the default on the
 * current thread instead. Or you can also use
 * gnome_vfs_module_callback_set_default() to set a regular callback
 * function.
 *
 * Note: @destroy_notify may be called on any thread - it is not
 * guaranteed to be called on the main thread.
 *
 **/
void
gnome_vfs_async_module_callback_set_default (const char *callback_name,
					     GnomeVFSAsyncModuleCallback callback,
					     gpointer callback_data,
					     GDestroyNotify notify)
{
	CallbackInfo *callback_info;

	callback_info = async_callback_info_new (callback, callback_data, notify);

	pthread_mutex_lock (&callback_table_lock);

	initialize_global_if_needed ();
	insert_callback_into_table (default_async_callbacks, callback_name, callback_info); 

	pthread_mutex_unlock (&callback_table_lock);

	callback_info_unref (callback_info);
}

/**
 * gnome_vfs_async_module_callback_push
 * @callback_name: The name of the module callback to set temporarily
 * @callback: The function to call when the callback is invoked
 * @callback_data: Pointer to pass as the @callback_data argument to @callback
 * @destroy_notify: Function to call when @callback_data is to be freed.
 * 
 * Set @callback_func as a temprary async handler for
 * @callback_name. @callback will be called with @callback_data
 * from a callback on the main thread. It will be passed a response
 * function which should be called to signal completion of the
 * callback. The callback function itself may return in the meantime.
 *
 * The temporary async handler is set per-thread.
 *
 * gnome_vfs_async_module_callback_pop() removes the most recently set
 * temporary temporary handler. The temporary async handlers are
 * treated as a first-in first-out stack.
 *
 * Use this function to set a temporary async callback handler for a
 * single call or a few calls. You can use
 * gnome_vfs_async_module_callback_set_default() to set an async
 * callback function that will establish a permanent global setting
 * for all threads instead.
 *
 * Note: @destroy_notify may be called on any thread - it is not
 * guaranteed to be called on the main thread.
 *
 **/
void
gnome_vfs_async_module_callback_push (const char *callback_name,
				      GnomeVFSAsyncModuleCallback callback,
				      gpointer callback_data,
				      GDestroyNotify notify)
{
	CallbackInfo *callback_info;

	initialize_per_thread_if_needed ();

	callback_info = async_callback_info_new (callback, callback_data, notify);
	
	push_callback_into_stack_table (pthread_getspecific (async_callback_stacks_key),
					callback_name,
					callback_info);

	callback_info_unref (callback_info);
}

/**
 * gnome_vfs_async_module_callback_pop
 * @callback_name: The name of the module callback to remove a temporary handler for
 * 
 * Remove the temporary async handler for @callback_name most recently
 * set with gnome_vfs_async_module_callback_push().  If another
 * temporary async handler was previously set on the same thread, it
 * becomes the current handler. Otherwise, the default async handler,
 * if any, becomes current.
 *
 * The temporary async handlers are treated as a first-in first-out
 * stack.
 *
 **/
void
gnome_vfs_async_module_callback_pop (const char *callback_name)
{
	initialize_per_thread_if_needed ();
	pop_stack_table (pthread_getspecific (async_callback_stacks_key),
			 callback_name);
}


/* -- Module-only entry points -- */

/**
 * gnome_vfs_module_callback_invoke
 * @callback_name: The name of the module callback to set
 * @in: In argument - type dependent on the specific callback
 * @in_size: Size of the in argument
 * @out: Out argument - type dependent on the specific callback
 * @out_size: Size of the out argument
 * 
 * Invoke a default callback for @callback_name, with in arguments
 * specified by @in and @in_size, and out arguments specified by @out
 * and @out_size.
 *
 * This function should only be called by gnome-vfs modules.
 *
 * If this function is called from an async job thread, it will invoke
 * the current async handler for @callback_name, if any. If no async
 * handler is set, or the function is not called from an async job
 * thread, the regular handler, if any, will be invoked instead. If no
 * handler at all is found for @callback_name, the function returns
 * FALSE.
 *
 * Return value: TRUE if a callback was invoked, FALSE if none was set.
 **/
gboolean
gnome_vfs_module_callback_invoke (const char    *callback_name,
				  gconstpointer  in,
				  gsize          in_size,
				  gpointer       out,
				  gsize          out_size)
{
	CallbackInfo *callback;
	gboolean invoked;
	GSList *stack;

	callback = NULL;

	initialize_per_thread_if_needed ();

	if (pthread_getspecific (in_async_thread_key) != NULL) {
		stack = g_hash_table_lookup (pthread_getspecific (async_callback_stacks_key),
					     callback_name);

		if (stack != NULL) {
			callback = stack->data;
			g_assert (callback != NULL);
			callback_info_ref (callback);
		} else {
			pthread_mutex_lock (&callback_table_lock);
			initialize_global_if_needed ();
			callback = g_hash_table_lookup (default_async_callbacks, callback_name);
			if (callback != NULL) {
				callback_info_ref (callback);
			}
			pthread_mutex_unlock (&callback_table_lock);
		}
	}

	if (callback == NULL) {
		stack = g_hash_table_lookup (pthread_getspecific (callback_stacks_key),
					     callback_name);
		
		if (stack != NULL) {
			callback = stack->data;
			g_assert (callback != NULL);
			callback_info_ref (callback);
		} else {
			pthread_mutex_lock (&callback_table_lock);
			initialize_global_if_needed ();
			callback = g_hash_table_lookup (default_callbacks, callback_name);
			if (callback != NULL) {
				callback_info_ref (callback);
			}
			pthread_mutex_unlock (&callback_table_lock);
		}
	}

	if (callback == NULL) {
		invoked = FALSE;
	} else {
		callback->callback (in, in_size, out, out_size, callback->callback_data);
		invoked = TRUE;
		callback_info_unref (callback);
	}

	return invoked;
}


/* -- Private entry points -- */

/* (used by job mechanism to implement callback
 * state copying semantics for async jobs.  
 */

GnomeVFSModuleCallbackStackInfo *
gnome_vfs_module_callback_get_stack_info (void)
{
	GnomeVFSModuleCallbackStackInfo *stack_info;

	stack_info = g_new (GnomeVFSModuleCallbackStackInfo, 1);

	pthread_mutex_lock (&callback_table_lock);
	initialize_global_if_needed ();
	stack_info->current_callbacks = duplicate_callback_table (default_callbacks);
	stack_info->current_async_callbacks = duplicate_callback_table (default_async_callbacks);
	pthread_mutex_unlock (&callback_table_lock);

	initialize_per_thread_if_needed ();
	copy_callback_stack_tops (pthread_getspecific (callback_stacks_key),
				  stack_info->current_callbacks);
	copy_callback_stack_tops (pthread_getspecific (async_callback_stacks_key),
				  stack_info->current_async_callbacks);

	return stack_info;
}

void
gnome_vfs_module_callback_free_stack_info (GnomeVFSModuleCallbackStackInfo *stack_info)
{
	clear_callback_table (stack_info->current_callbacks);
	g_hash_table_destroy (stack_info->current_callbacks); 
	clear_callback_table (stack_info->current_async_callbacks);
	g_hash_table_destroy (stack_info->current_async_callbacks);
	
	g_free (stack_info);
}

void
gnome_vfs_module_callback_use_stack_info (GnomeVFSModuleCallbackStackInfo *stack_info)
{
	initialize_per_thread_if_needed ();
	copy_callback_table_to_stack_table (stack_info->current_callbacks, 
					    pthread_getspecific (callback_stacks_key));
	copy_callback_table_to_stack_table (stack_info->current_async_callbacks, 
					    pthread_getspecific (async_callback_stacks_key));
}

void
gnome_vfs_module_callback_clear_stacks (void)
{
	initialize_per_thread_if_needed ();
	clear_stack_table (pthread_getspecific (callback_stacks_key));
	clear_stack_table (pthread_getspecific (async_callback_stacks_key));
}

void
gnome_vfs_module_callback_set_in_async_thread (gboolean in_async_thread)
{
	initialize_per_thread_if_needed ();
	pthread_setspecific (in_async_thread_key, GINT_TO_POINTER (in_async_thread));
}
