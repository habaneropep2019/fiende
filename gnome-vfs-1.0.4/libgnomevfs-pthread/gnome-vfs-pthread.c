#include "gnome-vfs-pthread.h"
#include "gnome-vfs-async-job-map.h"
#include "gnome-vfs-thread-pool.h"

gboolean
gnome_vfs_pthread_init(gboolean init_deps)
{
	if (!g_threads_got_initialized) {
		g_thread_init(NULL);
	}
	
	gnome_vfs_async_job_map_init ();
	gnome_vfs_thread_pool_init ();
	return TRUE;
}

#ifdef PTHREAD_MUTEX_RECURSIVE

int 
gnome_vfs_pthread_recursive_mutex_init (pthread_mutex_t *mutex)
{
	pthread_mutexattr_t attr;
	int result;

	pthread_mutexattr_init (&attr);
	pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
	result = pthread_mutex_init (mutex, &attr);
	pthread_mutexattr_destroy (&attr);
	
	return result;
}

#else

int 
gnome_vfs_pthread_recursive_mutex_init (GnomeVFSRecursiveMutex *m)
{
	if (pthread_mutex_init (&m->mutex, NULL) == -1)
		return -1;
	if (pthread_cond_init (&m->cond, NULL) == -1)
		return -1;

	m->count = 0;
	m->owner = 0;
	return 0;
}

int
gnome_vfs_pthread_recursive_mutex_lock (GnomeVFSRecursiveMutex *m)
{
	pthread_t self = pthread_self ();

	if (pthread_mutex_lock (&m->mutex) == -1)
		return -1;

	while (1) {
		if (m->owner == self) {
			m->count++;
			break;
		} else if (m->owner == 0) {
			m->owner = self;
			m->count = 1;
			break;
		} else {
			if (pthread_cond_wait (&m->cond, &m->mutex) == -1)
				return -1;
		}
	}
	return pthread_mutex_unlock (&m->mutex);
}

int
gnome_vfs_pthread_recursive_mutex_unlock (GnomeVFSRecursiveMutex *m)
{
	if (pthread_mutex_lock (&m->mutex) == -1)
		return -1;

	g_assert (m->owner == pthread_self());
	m->count--;
	if (m->count == 0) {
		m->owner = 0;
		pthread_cond_signal (&m->cond);
	}
	return pthread_mutex_unlock (&m->mutex);
}

int
gnome_vfs_pthread_recursive_mutex_destroy (GnomeVFSRecursiveMutex *m)
{
	g_assert (m->count == 0);
	if (pthread_mutex_destroy (&m->mutex) == -1)
		return -1;
	return pthread_cond_destroy (&m->cond);
}
#endif
