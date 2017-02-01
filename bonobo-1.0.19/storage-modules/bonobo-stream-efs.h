#ifndef _BONOBO_STREAM_EFS_H_
#define _BONOBO_STREAM_EFS_H_

#include <bonobo/bonobo-stream.h>
#include <storage-modules/bonobo-storage-efs.h>

BEGIN_GNOME_DECLS

#define BONOBO_STREAM_EFS_TYPE        (bonobo_stream_efs_get_type ())
#define BONOBO_STREAM_EFS(o)          (GTK_CHECK_CAST ((o), BONOBO_STREAM_EFS_TYPE, BonoboStreamEFS))
#define BONOBO_STREAM_EFS_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STREAM_EFS_TYPE, BonoboStreamEFSClass))
#define BONOBO_IS_STREAM_EFS(o)       (GTK_CHECK_TYPE ((o), BONOBO_STREAM_EFS_TYPE))
#define BONOBO_IS_STREAM_EFS_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STREAM_EFS_TYPE))

typedef struct {
	BonoboStream stream;
	BonoboStorageEFS *storage;
	EFSFile *file;
} BonoboStreamEFS;

typedef struct {
	BonoboStreamClass parent_class;
} BonoboStreamEFSClass;

GtkType          bonobo_stream_efs_get_type     (void);
BonoboStream    *bonobo_stream_efs_open         (BonoboStorageEFS *storage,
						 const CORBA_char *path, 
						 Bonobo_Storage_OpenMode mode,
						 CORBA_Environment *ev);
gint             bonobo_mode_to_efs             (Bonobo_Storage_OpenMode mode);
	
END_GNOME_DECLS

#endif /* _BONOBO_STREAM_EFS_H_ */
