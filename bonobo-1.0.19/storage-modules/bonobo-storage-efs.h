#ifndef _BONOBO_STORAGE_EFS_H_
#define _BONOBO_STORAGE_EFS_H_

#include <bonobo/bonobo-storage.h>
#include <efs.h>

BEGIN_GNOME_DECLS

#define BONOBO_STORAGE_EFS_TYPE        (bonobo_storage_efs_get_type ())
#define BONOBO_STORAGE_EFS(o)          (GTK_CHECK_CAST ((o), BONOBO_STORAGE_EFS_TYPE, BonoboStorageEFS))
#define BONOBO_STORAGE_EFS_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STORAGE_EFS_TYPE, BonoboStorageEFSClass))
#define BONOBO_IS_STORAGE_EFS(o)       (GTK_CHECK_TYPE ((o), BONOBO_STORAGE_EFS_TYPE))
#define BONOBO_IS_STORAGE_EFS_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STORAGE_EFS_TYPE))

typedef struct {
        BonoboStorage storage;
	BonoboStorage *owner;
	EFSDir *dir;
} BonoboStorageEFS;

typedef struct {
	BonoboStorageClass parent_class;
} BonoboStorageEFSClass;

GtkType        bonobo_storage_efs_get_type  (void);

END_GNOME_DECLS

#endif /* _BONOBO_STORAGE_EFS_H_ */
