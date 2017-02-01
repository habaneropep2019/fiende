#ifndef _GNOME_VFS_BACKEND_H_
#define _GNOME_VFS_BACKEND_H_ 1

#include <glib.h>

void        gnome_vfs_backend_loadinit      		(gpointer app,
					     		 gpointer modinfo);
const char *gnome_vfs_backend_name          		(void);
gboolean    gnome_vfs_backend_init          		(gboolean init_deps);
void        gnome_vfs_backend_shutdown      		(void);

/* debugging calls */
int         gnome_vfs_backend_get_job_count		(void);

#endif
