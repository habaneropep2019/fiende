/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-storage.c: Storage manipulation.
 *
 * Authors:
 *   Miguel de Icaza (miguel@gnu.org)
 *   Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gmodule.h>

#include <bonobo/bonobo-storage.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-storage-plugin.h>

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

#define CLASS(o) BONOBO_STORAGE_CLASS (GTK_OBJECT(o)->klass)

static inline BonoboStorage *
bonobo_storage_from_servant (PortableServer_Servant servant)
{
	return BONOBO_STORAGE (bonobo_object_from_servant (servant));
}

static Bonobo_StorageInfo*
impl_Bonobo_Storage_getInfo (PortableServer_Servant servant,
			     const CORBA_char * path,
			     const Bonobo_StorageInfoFields mask,
			     CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);

	return CLASS (storage)->get_info (storage, path, mask, ev);
}

static void          
impl_Bonobo_Storage_setInfo (PortableServer_Servant servant,
			     const CORBA_char * path,
			     const Bonobo_StorageInfo *info,
			     const Bonobo_StorageInfoFields mask,
			     CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);

	CLASS (storage)->set_info (storage, path, info, mask, ev);
}

static Bonobo_Stream
impl_Bonobo_Storage_openStream (PortableServer_Servant servant,
				const CORBA_char       *path,
				Bonobo_Storage_OpenMode mode,
				CORBA_Environment      *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	BonoboStream *stream;
	
	if ((stream = CLASS (storage)->open_stream (storage, path, mode, ev)))
		return (Bonobo_Stream) CORBA_Object_duplicate (
			BONOBO_OBJREF (stream), ev);
	else
		return CORBA_OBJECT_NIL;
}

static Bonobo_Storage
impl_Bonobo_Storage_openStorage (PortableServer_Servant  servant,
				 const CORBA_char       *path,
				 Bonobo_Storage_OpenMode mode,
				 CORBA_Environment      *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	BonoboStorage *open_storage;
	
	if ((open_storage = CLASS (storage)->open_storage (
		storage, path, mode, ev)))

		return (Bonobo_Storage) CORBA_Object_duplicate (
			BONOBO_OBJREF (open_storage), ev);
	else
		return CORBA_OBJECT_NIL;
}

static void
impl_Bonobo_Storage_copyTo (PortableServer_Servant servant,
			    Bonobo_Storage         target,
			    CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	Bonobo_Storage src = BONOBO_OBJREF (storage);	

	if (CLASS (storage)->copy_to)
		CLASS (storage)->copy_to (storage, target, ev);
        else
		bonobo_storage_copy_to (src, target, ev);
}

static void
impl_Bonobo_Storage_rename (PortableServer_Servant servant,
			    const CORBA_char      *path_name,
			    const CORBA_char      *new_path_name,
			    CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->rename (storage, path_name, new_path_name, ev);
}

static void
impl_Bonobo_Storage_commit (PortableServer_Servant servant, 
			    CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->commit (storage, ev);
}

static void
impl_Bonobo_Storage_revert (PortableServer_Servant servant, 
			    CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->commit (storage, ev);
}

static Bonobo_Storage_DirectoryList *
impl_Bonobo_Storage_listContents (PortableServer_Servant servant,
				  const CORBA_char      *path,
				  Bonobo_StorageInfoFields mask,
				  CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);

	return CLASS (storage)->list_contents (storage, path, mask, ev);
}

static void
impl_Bonobo_Storage_erase (PortableServer_Servant servant, 
			   const CORBA_char      *path,
			   CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->erase (storage, path, ev);
}

static void
bonobo_storage_class_init (BonoboStorageClass *klass)
{
	POA_Bonobo_Storage__epv *epv = &klass->epv;

	epv->getInfo      = impl_Bonobo_Storage_getInfo;
	epv->setInfo      = impl_Bonobo_Storage_setInfo;
	epv->openStream	  = impl_Bonobo_Storage_openStream;
	epv->openStorage  = impl_Bonobo_Storage_openStorage;
	epv->copyTo       = impl_Bonobo_Storage_copyTo;
	epv->rename       = impl_Bonobo_Storage_rename;
	epv->commit       = impl_Bonobo_Storage_commit;
	epv->revert       = impl_Bonobo_Storage_revert;
	epv->listContents = impl_Bonobo_Storage_listContents;
	epv->erase        = impl_Bonobo_Storage_erase;
}

static void 
bonobo_storage_init (GtkObject *object)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC_FULL (BonoboStorage, 
			 Bonobo_Storage,
			 PARENT_TYPE,
			 bonobo_storage);

/**
 * bonobo_storage_open:
 * @driver: driver to use for opening.
 * @path: path where the base file resides
 * @flags: Bonobo Storage OpenMode
 * @mode: Unix open(2) mode
 *
 * Opens or creates the file named at @path with the stream driver @driver.
 *
 * @driver is one of: "efs", "vfs" or "fs" for now, please use
 * the macros for this though, see bonobo-stream.h eg.
 * BONOBO_IO_DRIVER_FS
 *
 * Returns: a created BonoboStorage object.
 */
BonoboStorage *
bonobo_storage_open_full (const char *driver, const char *path,
			  gint flags, gint mode,
			  CORBA_Environment *opt_ev)
{
	BonoboStorage *storage = NULL;
	StoragePlugin *p;
	CORBA_Environment ev, *my_ev;
	
	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	if (!driver || !path)
		CORBA_exception_set (my_ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);

	else if (!(p = bonobo_storage_plugin_find (driver)) ||
		 !p->storage_open)
		CORBA_exception_set (my_ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
	else
		storage = p->storage_open (path, flags, mode, my_ev);

	if (!opt_ev) {
		if (BONOBO_EX (my_ev))
			g_warning ("bonobo_storage_open failed '%s'",
				   bonobo_exception_get_text (my_ev));
		CORBA_exception_free (&ev);
	}

	return storage;
}

/**
 * bonobo_storage_open:
 * @driver: driver to use for opening.
 * @path: path where the base file resides
 * @flags: Bonobo Storage OpenMode
 * @mode: Unix open(2) mode
 *
 * Opens or creates the file named at @path with the stream driver
 * @driver.
 *
 * @driver is one of: "efs", "vfs" or "fs" for now, please use
 * the macros for this though, see bonobo-stream.h eg.
 * BONOBO_IO_DRIVER_FS
 *
 * Returns: a created BonoboStorage object.
 **/
BonoboStorage *
bonobo_storage_open (const char *driver, const char *path,
		     gint flags, gint mode)
{
	return bonobo_storage_open_full (driver, path, flags, mode, NULL);
}

static void
copy_stream (Bonobo_Stream src, Bonobo_Stream dest, CORBA_Environment *ev) 
{
	Bonobo_Stream_iobuf *buf;

	do {
		Bonobo_Stream_read (src, 4096, &buf, ev);
		if (BONOBO_EX (ev)) 
			break;

		if (buf->_length == 0) {
			CORBA_free (buf);
			break;
		}

		Bonobo_Stream_write (dest, buf, ev);
		CORBA_free (buf);
		if (BONOBO_EX (ev)) 
			break;

	} while (1);

	if (BONOBO_EX (ev)) /* we must return a Bonobo_Storage exception */
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);
}

/**
 * bonobo_storage_copy_to:
 * @src: the source storage
 * @dest: the destination storage
 * @ev: CORBA exception environment
 * 
 * Implements a pure CORBA method for copying one storage into
 * another, this is used by several BonoboStorage implemetations
 * where a fast case localy copy cannot work.
 **/
void
bonobo_storage_copy_to (Bonobo_Storage src, Bonobo_Storage dest,
			CORBA_Environment *ev) 
{
	Bonobo_Storage new_src, new_dest;
	Bonobo_Stream src_stream, dest_stream;
	Bonobo_Storage_DirectoryList *list;
	gint i;

	if ((src == CORBA_OBJECT_NIL) || (dest == CORBA_OBJECT_NIL) || !ev) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);
		return;
	}

	list = Bonobo_Storage_listContents (src, "", 
					    Bonobo_FIELD_CONTENT_TYPE |
					    Bonobo_FIELD_TYPE,
					    ev);
	if (BONOBO_EX (ev))
		return;

	for (i = 0; i <list->_length; i++) {

		if (list->_buffer[i].type == Bonobo_STORAGE_TYPE_DIRECTORY) {

			new_dest = Bonobo_Storage_openStorage
				(dest, list->_buffer[i].name, 
				 Bonobo_Storage_CREATE | 
				 Bonobo_Storage_FAILIFEXIST, ev);

			if (BONOBO_EX (ev)) 
				break;

			Bonobo_Storage_setInfo (new_dest, "",
						&list->_buffer[i],
						Bonobo_FIELD_CONTENT_TYPE,
						ev);

			if (BONOBO_EX (ev)) {
				bonobo_object_release_unref (new_dest, NULL);
				break;
			}

			new_src = Bonobo_Storage_openStorage
				(src, list->_buffer[i].name, 
				 Bonobo_Storage_READ, ev);
			
			if (BONOBO_EX (ev)) {
				bonobo_object_release_unref (new_dest, NULL);
				break;
			}

			bonobo_storage_copy_to (new_src, new_dest, ev);
			
			bonobo_object_release_unref (new_src, NULL);
			bonobo_object_release_unref (new_dest, NULL);

			if (BONOBO_EX (ev))
				break;

		} else {
			dest_stream = Bonobo_Storage_openStream 
				(dest, list->_buffer[i].name,
				 Bonobo_Storage_CREATE | 
				 Bonobo_Storage_FAILIFEXIST, ev);

			if (BONOBO_EX (ev))
				break;

			Bonobo_Stream_setInfo (dest_stream,
					       &list->_buffer[i],
					       Bonobo_FIELD_CONTENT_TYPE,
					       ev);

			if (BONOBO_EX (ev)) {
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
						     ex_Bonobo_Storage_IOError,
						     NULL);
				bonobo_object_release_unref (dest_stream,
							     NULL);
				break;
			}

			src_stream = Bonobo_Storage_openStream 
				(src, list->_buffer[i].name,
				 Bonobo_Storage_READ, ev);

			if (BONOBO_EX (ev)) {
				bonobo_object_release_unref (dest_stream, 
							     NULL);
				break;
			}

			copy_stream (src_stream, dest_stream, ev);

			bonobo_object_release_unref (src_stream, NULL);
			bonobo_object_release_unref (dest_stream, NULL);

			if (BONOBO_EX (ev))
				break;
		}
	}

	CORBA_free (list);
}
