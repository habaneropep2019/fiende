/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Bonobo PersistFile
 *
 * Author:
 *   Matt Loper (matt@gnome-support.com)
 *
 * Copyright 1999,2000 Helix Code, Inc.
 */

#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-persist-file.h>

#define PARENT_TYPE BONOBO_PERSIST_TYPE

/* Parent GTK object class */
static BonoboPersistClass *bonobo_persist_file_parent_class;

static CORBA_char *
impl_get_current_file (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistFile *pfile = BONOBO_PERSIST_FILE (object);

	/* if our persist_file has a filename with any length, return it */
	if (pfile->filename && strlen (pfile->filename))
		return CORBA_string_dup ((CORBA_char*)pfile->filename);
	else
	{
		/* otherwise, raise a `NoCurrentName' exception */
		Bonobo_PersistFile_NoCurrentName *exception;
		exception = Bonobo_PersistFile_NoCurrentName__alloc ();
		
		exception->extension = CORBA_string_dup ("");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_PersistFile_NoCurrentName,
				     exception);
		return NULL;
	}	
}


static CORBA_boolean
impl_is_dirty (PortableServer_Servant servant, CORBA_Environment * ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistFile *pfile = BONOBO_PERSIST_FILE (object);

	return pfile->is_dirty;
}

static void
impl_load (PortableServer_Servant servant,
	   const CORBA_char      *filename,
	   CORBA_Environment     *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistFile *pf = BONOBO_PERSIST_FILE (object);
	int result;
	
	if (pf->load_fn != NULL)
		result = (*pf->load_fn)(pf, filename, ev, pf->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (pf)->klass;
		BonoboPersistFileClass *class = BONOBO_PERSIST_FILE_CLASS (oc);

		if (class->load)
			result = (*class->load)(pf, filename, ev);
		else {
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_NotSupported, NULL);
			return;
		}
			
	}
	if (result != 0) {
		CORBA_exception_set (
			ev, CORBA_USER_EXCEPTION,
			ex_Bonobo_IOError, NULL);
	}
}

static void
impl_save (PortableServer_Servant servant,
	   const CORBA_char      *filename,
	   CORBA_Environment     *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistFile *pf = BONOBO_PERSIST_FILE (object);
	int result;
	
	if (pf->save_fn != NULL)
		result = (*pf->save_fn)(pf, filename, ev, pf->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (pf)->klass;
		BonoboPersistFileClass *class = BONOBO_PERSIST_FILE_CLASS (oc);

		if (class->save)
			result = (*class->save)(pf, filename, ev);
		else {
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_NotSupported, NULL);
			return;
		}
	}
	
	if (result != 0){
		CORBA_exception_set (
			ev, CORBA_USER_EXCEPTION,
			ex_Bonobo_Persist_FileNotFound, NULL);
	}
	pf->is_dirty = FALSE;
}

static CORBA_char *
bonobo_persist_file_get_current_file (BonoboPersistFile *pf,
				      CORBA_Environment *ev)
{
	if (pf->filename)
		return pf->filename;
	return "";
}

static void
bonobo_persist_file_class_init (BonoboPersistFileClass *klass)
{
	POA_Bonobo_PersistFile__epv *epv = &klass->epv;

	bonobo_persist_file_parent_class = gtk_type_class (PARENT_TYPE);

	/* Override and initialize methods */
	klass->save = NULL;
	klass->load = NULL;
	klass->get_current_file = bonobo_persist_file_get_current_file;

	epv->load           = impl_load;
	epv->save           = impl_save;
	epv->isDirty        = impl_is_dirty;
	epv->getCurrentFile = impl_get_current_file;
}

static void
bonobo_persist_file_init (GtkObject *object)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC_FULL (BonoboPersistFile, 
			   Bonobo_PersistFile,
			   PARENT_TYPE,
			   bonobo_persist_file);

/**
 * bonobo_persist_file_construct:
 * @pf: A BonoboPersistFile
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Initializes the BonoboPersistFile object.  The @load_fn and @save_fn
 * parameters might be NULL.  If this is the case, the load and save 
 * operations are performed by the class load and save methods
 */
BonoboPersistFile *
bonobo_persist_file_construct (BonoboPersistFile    *pf,
			       BonoboPersistFileIOFn load_fn,
			       BonoboPersistFileIOFn save_fn,
			       void                 *closure)
{
	g_return_val_if_fail (pf != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PERSIST_FILE (pf), NULL);

	pf->load_fn = load_fn;
	pf->save_fn = save_fn;
	pf->closure = closure;
		
	return pf;
}

/**
 * bonobo_persist_file_new:
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Creates a BonoboPersistFile object.  The @load_fn and @save_fn
 * parameters might be NULL.  If this is the case, the load and save 
 * operations are performed by the class load and save methods
 */
BonoboPersistFile *
bonobo_persist_file_new (BonoboPersistFileIOFn load_fn,
			 BonoboPersistFileIOFn save_fn,
			 void                 *closure)
{
	BonoboPersistFile *pf;

	pf = gtk_type_new (bonobo_persist_file_get_type ());

	pf->filename = NULL;

	bonobo_persist_file_construct (pf, load_fn, save_fn, closure);

	return pf;
}

