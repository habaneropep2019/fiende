/* Threading for guile-gtk
 * Copyright (C) 2000 Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* Define the following option to include the primitive
 * `gtk-threads-update' which makes the main loop wake up.
 */

/* #define GUILE_GTKTHREADS_UPDATE */

#include <config.h>
#include <libguile.h>
#include <unistd.h>
#include <glib.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "gtk-threads.h"

#ifdef USE_THREADS

extern int errno;

#ifdef FD_SET
#  define SELECT_MASK fd_set
#else /* !NO_FD_SET */
#  ifdef _IBMR2 /*fixme* not defined*/
#    define SELECT_MASK void
#  else /* !_IBMR2 */
#    define SELECT_MASK int
#  endif /* !_IBMR2 */
#endif /* !NO_FD_SET */

#ifdef GUILE_GTKTHREADS_UPDATE
static int poll_waiting = 0;
static int wake_up_pipe[2] = { -1, -1 };
static GPollFD wake_up_rec;
#endif

static gint 
g_poll (GPollFD *fds,
	guint    nfds,
	gint     timeout)
{
  struct timeval tv;
  SELECT_MASK rset, wset, xset;
  GPollFD *f;
  int ready;
  int maxfd = 0;

  FD_ZERO (&rset);
  FD_ZERO (&wset);
  FD_ZERO (&xset);

  for (f = fds; f < &fds[nfds]; ++f)
    if (f->fd >= 0)
      {
	if (f->events & G_IO_IN)
	  FD_SET (f->fd, &rset);
	if (f->events & G_IO_OUT)
	  FD_SET (f->fd, &wset);
	if (f->events & G_IO_PRI)
	  FD_SET (f->fd, &xset);
	if (f->fd > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI)))
	  maxfd = f->fd;
      }

  tv.tv_sec = timeout / 1000;
  tv.tv_usec = (timeout % 1000) * 1000;

#ifdef GUILE_GTKTHREADS_UPDATE
  poll_waiting = TRUE;
#endif
  
  ready = scm_internal_select (maxfd + 1, &rset, &wset, &xset,
		               timeout == -1 ? NULL : &tv);

#ifdef GUILE_GTKTHREADS_UPDATE
  if (!poll_waiting)
    {
#ifndef NATIVE_WIN32
      gchar c;
      read (wake_up_pipe[0], &c, 1);
#endif
    }
  else
    poll_waiting = FALSE;
#endif

  if (ready > 0)
    for (f = fds; f < &fds[nfds]; ++f)
      {
	f->revents = 0;
	if (f->fd >= 0)
	  {
	    if (FD_ISSET (f->fd, &rset))
	      f->revents |= G_IO_IN;
	    if (FD_ISSET (f->fd, &wset))
	      f->revents |= G_IO_OUT;
	    if (FD_ISSET (f->fd, &xset))
	      f->revents |= G_IO_PRI;
	  }
      }

  return ready;
}

#ifdef GUILE_GTKTHREADS_UPDATE
/* Wake the main loop up from a poll() */
static void
g_main_wakeup (void)
{
  if (poll_waiting)
    {
      poll_waiting = FALSE;
#ifndef NATIVE_WIN32
      write (wake_up_pipe[1], "A", 1);
#else
      ReleaseSemaphore (wake_up_semaphore, 1, NULL);
#endif
    }
}

SCM
sgtk_threads_update ()
{
  g_main_wakeup ();
  return SCM_UNSPECIFIED;
}

#else  /* !GUILE_GTKTHREADS_UPDATE */

SCM
sgtk_threads_update ()
{
  return SCM_UNSPECIFIED;
}

#endif /* !GUILE_GTKTHREADS_UPDATE */

#define guile_print_error( name, num )                          \
  g_error( "file %s: line %d (%s): error %s during %s",         \
           __FILE__, __LINE__, G_GNUC_PRETTY_FUNCTION,          \
           g_strerror((num)), #name )

#define guile_check_for_error( what ) G_STMT_START{             \
   int error = (what);                                           \
   if( error ) { guile_print_error( what, error ); }             \
   }G_STMT_END

static GMutex *
g_mutex_new_guile_impl (void)
{
  GMutex *result = (GMutex *) g_new (scm_mutex_t, 1);
  guile_check_for_error (scm_mutex_init ((scm_mutex_t *) result));
  return result;
}

static void
g_mutex_free_guile_impl (GMutex * mutex)
{
  guile_check_for_error (scm_mutex_destroy ((scm_mutex_t *) mutex));
  g_free (mutex);
}

/* NOTE: the functions g_mutex_lock and g_mutex_unlock may not use
 * functions from gmem.c and gmessages.c;
 */

/* scm_mutex_lock, scm_mutex_unlock can be taken directly, as
 * signature and semantics are right, but without error check
 * We might want to change this.
 */

static gboolean
g_mutex_trylock_guile_impl (GMutex * mutex)
{
  int result;

  result = scm_mutex_trylock ((scm_mutex_t *) mutex);

  if (result == EBUSY)
    return FALSE;

  guile_check_for_error (result);
  return TRUE;
}

static GCond *
g_cond_new_guile_impl (void)
{
  GCond *result = (GCond *) g_new (scm_cond_t, 1);
  guile_check_for_error (scm_cond_init ((scm_cond_t *) result, NULL));
  return result;
}

/* scm_cond_signal, scm_cond_broadcast and scm_cond_wait can be taken
 * directly, as signatures and semantics are right, but without error
 * check.  We might want to change this.
 */

#define G_MICROSEC 1000000
#define G_NANOSEC 1000000000

static gboolean
g_cond_timed_wait_guile_impl (GCond * cond,
			      GMutex * entered_mutex,
			      GTimeVal * abs_time)
{
  int result;
  struct timespec end_time;
  gboolean timed_out;

  g_return_val_if_fail (cond != NULL, FALSE);
  g_return_val_if_fail (entered_mutex != NULL, FALSE);

  if (!abs_time)
    {
      g_cond_wait (cond, entered_mutex);
      return TRUE;
    }

  end_time.tv_sec = abs_time->tv_sec;
  end_time.tv_nsec = abs_time->tv_usec * (G_NANOSEC / G_MICROSEC);
  g_assert (end_time.tv_nsec < G_NANOSEC);
  result = scm_cond_timedwait ((scm_cond_t *) cond,
			       (scm_mutex_t *) entered_mutex,
			       &end_time);

  timed_out = (result == ETIME);

  if (!timed_out)
    guile_check_for_error (result);
  return !timed_out;
}

static void
g_cond_free_guile_impl (GCond * cond)
{
  guile_check_for_error (scm_cond_destroy ((scm_cond_t *) cond));
  g_free (cond);
}

static GPrivate *
g_private_new_guile_impl (GDestroyNotify destructor)
{
  GPrivate *result = (GPrivate *) g_new (scm_key_t, 1);
  guile_check_for_error (scm_key_create ((scm_key_t *) result,
					 destructor));
  return result;
}

/* NOTE: the functions g_private_get and g_private_set may not use
   functions from gmem.c and gmessages.c */

static void
g_private_set_guile_impl (GPrivate * private_key, gpointer value)
{
  if (!private_key)
    return;

  scm_setspecific (*(scm_key_t *) private_key, value);
}

static gpointer
g_private_get_guile_impl (GPrivate * private_key)
{
  if (!private_key)
    return NULL;
  return scm_getspecific (*(scm_key_t *) private_key);
}

#ifdef HAVE_THREAD_CREATE
struct spawn_data {
  GThreadFunc func;
  gpointer arg;
  gpointer thread;
};

static SCM
spawn (void *arg)
{
  struct spawn_data *data = (struct spawn_data *) arg;
  * (coop_t **) data->thread = coop_global_curr;
  data->func (data->arg);
  return SCM_UNSPECIFIED;
}

static void
g_thread_create_guile_impl (GThreadFunc thread_func, 
			    gpointer arg, 
			    gulong stack_size,
			    gboolean joinable,
			    gboolean bound,
			    GThreadPriority priority,
			    gpointer thread)
{
  struct spawn_data data;
  data.func = thread_func;
  data.arg = arg;
  data.thread = thread;
  scm_spawn_thread (spawn, &data, scm_handle_by_message_noexit, 0);
}

static void
g_thread_join_guile_impl (gpointer thread)
{
  coop_join (* (coop_t **) thread);
}

extern void coop_abort (void);

static void
g_thread_set_priority_guile_impl (gpointer thread, GThreadPriority priority)
{
}

static void
g_thread_self_guile_impl (gpointer thread)
{
  * (coop_t **) thread = coop_global_curr;
}
#endif /* HAVE_THREAD_CREATE */

static GThreadFunctions g_guile_thread_functions_for_glib =
{
  g_mutex_new_guile_impl,
  (void (*)(GMutex *)) scm_mutex_lock,
  g_mutex_trylock_guile_impl,
  (void (*)(GMutex *)) scm_mutex_unlock,
  g_mutex_free_guile_impl,
  g_cond_new_guile_impl,
  (void (*)(GCond *)) scm_cond_signal,
  (void (*)(GCond *)) scm_cond_broadcast,
  (void (*)(GCond *, GMutex *)) scm_cond_wait,
  g_cond_timed_wait_guile_impl,
  g_cond_free_guile_impl,
  g_private_new_guile_impl,
  g_private_get_guile_impl,
  g_private_set_guile_impl
#ifdef HAVE_THREAD_CREATE
  ,
  g_thread_create_guile_impl,
  coop_yield,
  g_thread_join_guile_impl,
  coop_abort,
  g_thread_set_priority_guile_impl,
  g_thread_self_guile_impl
#endif /* HAVE_THREAD_CREATE */
};

void
sgtk_init_threads ()
{
  g_thread_init (&g_guile_thread_functions_for_glib);
  g_main_set_poll_func (g_poll);
#ifdef GUILE_GTKTHREADS_UPDATE
  if (pipe (wake_up_pipe) < 0)
    g_error ("Cannot create pipe main loop wake-up: %s\n",
	     g_strerror (errno));

  wake_up_rec.fd = wake_up_pipe[0];
  wake_up_rec.events = G_IO_IN;
  g_main_add_poll (&wake_up_rec, 0);
#endif
}

#else /* !USE_THREADS */

SCM
sgtk_threads_update ()
{
  return SCM_UNSPECIFIED;
}

void
sgtk_init_threads ()
{
}

#endif /* !USE_THREADS */
