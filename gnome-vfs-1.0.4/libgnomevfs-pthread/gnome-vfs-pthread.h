#ifndef GNOME_VFS_PTHREAD_H
#define GNOME_VFS_PTHREAD_H 1

#include <glib.h>
#include <pthread.h>

gboolean gnome_vfs_pthread_init(gboolean init_deps);

#ifdef PTHREAD_MUTEX_RECURSIVE
#define GnomeVFSRecursiveMutex pthread_mutex_t
#define gnome_vfs_pthread_recursive_mutex_lock pthread_mutex_lock
#define gnome_vfs_pthread_recursive_mutex_unlock pthread_mutex_unlock
#define gnome_vfs_pthread_recursive_mutex_destroy pthread_mutex_destroy

#else

typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_t owner;
	int count;
} GnomeVFSRecursiveMutex;
int gnome_vfs_pthread_recursive_mutex_lock (GnomeVFSRecursiveMutex *m);
int gnome_vfs_pthread_recursive_mutex_unlock (GnomeVFSRecursiveMutex *m);
int gnome_vfs_pthread_recursive_mutex_destroy (GnomeVFSRecursiveMutex *m);
#endif

int gnome_vfs_pthread_recursive_mutex_init (GnomeVFSRecursiveMutex *);

#endif
