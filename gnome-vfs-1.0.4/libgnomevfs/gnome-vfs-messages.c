/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-messages.c - Status message reporting for GNOME Virtual File
   System.

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

   Author: Havoc Pennington <hp@redhat.com> */

#include "gnome-vfs-messages.h"

#include <stdio.h>

#ifdef G_THREADS_ENABLED
#define MUTEX_LOCK(a)	if ((a) != NULL) g_mutex_lock (a)
#define MUTEX_UNLOCK(a)	if ((a) != NULL) g_mutex_unlock (a)
#else
#define MUTEX_LOCK(a)
#define MUTEX_UNLOCK(a)
#endif

static guint next_id = 1;

G_LOCK_DEFINE_STATIC (next_id);

typedef struct Callback Callback;

struct Callback {
        GnomeVFSStatusCallback callback_func;
        gpointer user_data;
        GDestroyNotify notify_func;
	guint id;
};

static Callback*
callback_new (GnomeVFSStatusCallback callback_func,
              gpointer user_data,
              GDestroyNotify notify_func)
{
        Callback *cb;
        
        cb = g_new(Callback, 1);

        cb->callback_func = callback_func;
        cb->user_data = user_data;
        cb->notify_func = notify_func;

	G_LOCK(next_id);
	cb->id = next_id;
	++next_id;
	G_UNLOCK(next_id);

        return cb;
}

static void
callback_destroy (Callback *cb)
{
        if (cb->notify_func != NULL) {
                (* cb->notify_func) (cb->user_data);
        }

        g_free(cb);
}

static void
callback_invoke (Callback *cb, const gchar* message)
{
        if (cb->callback_func) {
                (* cb->callback_func) (message, cb->user_data);
        }
}

struct GnomeVFSMessageCallbacks {
        GSList *list;
#ifdef G_THREADS_ENABLED
	GMutex *list_mutex;
#endif
};

GnomeVFSMessageCallbacks*
gnome_vfs_message_callbacks_new (void)
{
        GnomeVFSMessageCallbacks *cbs;

        cbs = g_new0(GnomeVFSMessageCallbacks, 1);

#ifdef G_THREADS_ENABLED
	if (g_thread_supported ())
		cbs->list_mutex = g_mutex_new ();
	else
		cbs->list_mutex = NULL;
#endif
	
        return cbs;
}

void
gnome_vfs_message_callbacks_destroy (GnomeVFSMessageCallbacks *cbs)
{
        GSList *tmp;

	MUTEX_LOCK(cbs->list_mutex);
	
        tmp = cbs->list;

        while (tmp != NULL) {
                Callback *cb;

                cb = tmp->data;

                callback_destroy (cb);
                
                tmp = g_slist_next (tmp);
        }
        
        g_slist_free (cbs->list);

	MUTEX_UNLOCK(cbs->list_mutex);
	
#ifdef G_THREADS_ENABLED
	if (cbs->list_mutex != NULL)
		g_mutex_free (cbs->list_mutex);
#endif
	
        g_free(cbs);
}

guint
gnome_vfs_message_callbacks_add (GnomeVFSMessageCallbacks *cbs,
                                 GnomeVFSStatusCallback    callback,
                                 gpointer                  user_data)
{
        return gnome_vfs_message_callbacks_add_full (cbs, callback, user_data, NULL);
}

guint
gnome_vfs_message_callbacks_add_full (GnomeVFSMessageCallbacks *cbs,
                                      GnomeVFSStatusCallback    callback,
                                      gpointer                  user_data,
                                      GDestroyNotify            notify)
{
        Callback *cb;

        cb = callback_new (callback, user_data, notify);

	MUTEX_LOCK(cbs->list_mutex);
	
        cbs->list = g_slist_prepend (cbs->list, cb);

	MUTEX_UNLOCK(cbs->list_mutex);
	
	return cb->id;
}

typedef gboolean (* MyGSListFilterFunc) (gpointer list_element, gpointer user_data);

static GSList*
my_g_slist_filter (GSList* list, MyGSListFilterFunc func, gpointer user_data)
{
        GSList *iter;
        GSList *retval;

        retval = NULL;
        iter = list;

        while (iter != NULL) {
                GSList *freeme;
                
                if ((*func)(iter->data, user_data)) {
                        retval = g_slist_prepend (retval, iter->data);
                }
                
                freeme = iter;
                iter = g_slist_next (iter);

                g_assert(freeme != NULL);

                /* Avoids using double the amount of space; glib can
                   recycle these nodes into the new list */
                g_slist_free_1 (freeme);
        }

        /* We assembled the nodes backward */
        retval = g_slist_reverse (retval);
        
        return retval;
}

static gboolean
callback_equal_predicate (gpointer callback, gpointer func)
{
        return !(((Callback*)callback)->callback_func ==
                ((GnomeVFSStatusCallback)func));
}

static gboolean
data_equal_predicate (gpointer callback, gpointer data)
{
        return !(((Callback*)callback)->user_data == data);
}

struct func_and_data {
        GnomeVFSStatusCallback func;
        gpointer data;
};

static gboolean
all_equal_predicate (gpointer callback, gpointer func_and_data)
{
        Callback *cb = callback;
        struct func_and_data* fd = func_and_data;

        return !(cb->callback_func == fd->func && cb->user_data == fd->data);
}

void gnome_vfs_message_callbacks_remove (GnomeVFSMessageCallbacks *cbs,
					 guint num)
{
        GSList *iter;

	MUTEX_LOCK(cbs->list_mutex);
	
        iter = cbs->list;

        while (iter != NULL) {
                Callback *cb;

                cb = iter->data;

		if (cb->id == num)
			break;
			
                iter = g_slist_next (iter);
        }

	if (iter)
		cbs->list = g_slist_remove(cbs->list, iter->data);
	else
		g_warning("status callback %u not found to remove", num);

	MUTEX_UNLOCK(cbs->list_mutex);
}

void
gnome_vfs_message_callbacks_remove_by_func (GnomeVFSMessageCallbacks *cbs,
                                            GnomeVFSStatusCallback    callback)
{
	MUTEX_LOCK(cbs->list_mutex);
        cbs->list = my_g_slist_filter (cbs->list, callback_equal_predicate, callback);
	MUTEX_UNLOCK(cbs->list_mutex);
}

void
gnome_vfs_message_callbacks_remove_by_data (GnomeVFSMessageCallbacks *cbs,
                                            gpointer                  user_data)
{
	MUTEX_LOCK(cbs->list_mutex);
        cbs->list = my_g_slist_filter (cbs->list, data_equal_predicate, user_data);
	MUTEX_UNLOCK(cbs->list_mutex);
}

void
gnome_vfs_message_callbacks_remove_by_func_and_data (GnomeVFSMessageCallbacks *cbs,
						     GnomeVFSStatusCallback    callback,
                                    gpointer                  user_data)
{
        struct func_and_data fd;

	MUTEX_LOCK(cbs->list_mutex);
	
        fd.func = callback;
        fd.data = user_data;
        
        cbs->list = my_g_slist_filter (cbs->list, all_equal_predicate, &fd);

	MUTEX_UNLOCK(cbs->list_mutex);
}

void
gnome_vfs_message_callbacks_emit (GnomeVFSMessageCallbacks *cbs,
                                  const gchar              *message)
{
        GSList *iter;

	MUTEX_LOCK(cbs->list_mutex);
	
        iter = cbs->list;

        while (iter != NULL) {
                Callback *cb;

                cb = iter->data;

                callback_invoke (cb, message);
                
                iter = g_slist_next (iter);
        }

	MUTEX_UNLOCK(cbs->list_mutex);
}


