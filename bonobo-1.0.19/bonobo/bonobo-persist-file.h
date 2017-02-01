/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Bonobo PersistFile
 *
 * Author:
 *   Matt Loper (matt@gnome-support.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */

#ifndef _BONOBO_PERSIST_FILE_H_
#define _BONOBO_PERSIST_FILE_H_

#include <bonobo/bonobo-persist.h>

BEGIN_GNOME_DECLS

#define BONOBO_PERSIST_FILE_TYPE (bonobo_persist_file_get_type ())
#define BONOBO_PERSIST_FILE(o)   (GTK_CHECK_CAST ((o), BONOBO_PERSIST_FILE_TYPE, BonoboPersistFile))
#define BONOBO_PERSIST_FILE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PERSIST_FILE_TYPE, BonoboPersistFileClass))
#define BONOBO_IS_PERSIST_FILE(o)       (GTK_CHECK_TYPE ((o), BONOBO_PERSIST_FILE_TYPE))
#define BONOBO_IS_PERSIST_FILE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PERSIST_FILE_TYPE))

typedef struct _BonoboPersistFilePrivate BonoboPersistFilePrivate;
typedef struct _BonoboPersistFile        BonoboPersistFile;

typedef int (*BonoboPersistFileIOFn) (BonoboPersistFile *pf,
				      const CORBA_char  *filename,
				      CORBA_Environment *ev,
				      void              *closure);

struct _BonoboPersistFile {
	BonoboPersist persist;

	gboolean     is_dirty;
	char *filename;

	/*
	 * For the sample routines, NULL if we use the ::save and ::load
	 * methods from the class
	 */
	BonoboPersistFileIOFn  save_fn;
	BonoboPersistFileIOFn  load_fn;
	void *closure;

	BonoboPersistFilePrivate *priv;
};

typedef struct {
	BonoboPersistClass parent_class;

	POA_Bonobo_PersistFile__epv epv;

	/* methods */
	int   (*load)             (BonoboPersistFile *ps,
				   const CORBA_char  *filename,
				   CORBA_Environment *ev);

	int   (*save)             (BonoboPersistFile *ps,
				   const CORBA_char  *filename,
				   CORBA_Environment *ev);

	char *(*get_current_file) (BonoboPersistFile *ps,
				   CORBA_Environment *ev);

} BonoboPersistFileClass;

GtkType             bonobo_persist_file_get_type  (void);
void                bonobo_persist_file_set_dirty (BonoboPersistFile   *ps,
						   gboolean             dirty);

BonoboPersistFile *bonobo_persist_file_new       (BonoboPersistFileIOFn load_fn,
						  BonoboPersistFileIOFn save_fn,
						  void                 *closure);

BonoboPersistFile *bonobo_persist_file_construct (BonoboPersistFile    *ps,
						  BonoboPersistFileIOFn load_fn,
						  BonoboPersistFileIOFn save_fn,
						  void                 *closure);

END_GNOME_DECLS

#endif /* _BONOBO_PERSIST_FILE_H_ */
