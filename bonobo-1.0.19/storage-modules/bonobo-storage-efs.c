/*
 * bonobo-storage-efs.c: libefs based Storage implementation
 *
 * Author:
 *   Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright (C) 2000 Maurer IT Systemlösungen KEG
 */

#include <config.h>
#include <efs.h>
#include <storage-modules/bonobo-storage-efs.h>
#include <storage-modules/bonobo-stream-efs.h>
#include <bonobo/bonobo-storage-plugin.h>
#include <orb/corba_object_type.h>

static void
bonobo_storage_efs_destroy (GtkObject *object)
{
	BonoboStorageEFS *storage_efs = BONOBO_STORAGE_EFS (object);
	
	if (storage_efs->owner) {
		if (storage_efs->dir)
			efs_dir_close (storage_efs->dir);

		bonobo_object_unref (BONOBO_OBJECT (storage_efs->owner));
	} else {
		if (storage_efs->dir)
			efs_close (storage_efs->dir);
	}
}

static Bonobo_StorageInfo*
real_get_info (BonoboStorage *storage,
	       const CORBA_char *path,
	       const Bonobo_StorageInfoFields mask,
	       CORBA_Environment *ev)
{
	BonoboStorageEFS *storage_efs = BONOBO_STORAGE_EFS (storage);
	EFSNode *node = NULL;
	EFSResult result;
	EFSStat st;
	Bonobo_StorageInfo *si;
	gchar *content_type = NULL;

	if (mask & ~(Bonobo_FIELD_CONTENT_TYPE | Bonobo_FIELD_SIZE |
		     Bonobo_FIELD_TYPE)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
		return CORBA_OBJECT_NIL;
	}

	if ((result = efs_node_open 
	     (&node, storage_efs->dir, path, EFS_READ, 0)))
		goto get_info_except;

	if ((result = efs_node_stat (node, &st)))
		goto get_info_except;

	if ((mask & Bonobo_FIELD_CONTENT_TYPE) &&
	    ((result = efs_strtype_get (node, &content_type))))
		goto get_info_except;

	efs_node_close (node);
	
	si = Bonobo_StorageInfo__alloc ();

	si->size = st.size;
	si->name = CORBA_string_dup (path);

	if (st.type == EFS_DIR) 
		si->type = Bonobo_STORAGE_TYPE_DIRECTORY;
	else 
		si->type = Bonobo_STORAGE_TYPE_REGULAR;

	if (content_type) 
		si->content_type = CORBA_string_dup (content_type);
	else  
		si->content_type = CORBA_string_dup ("");

	return si;

 get_info_except:

	if (node)
		efs_node_close (node);
	
	if (result == EFS_ERR_PERM) 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NoPermission, 
				     NULL);
	else if (result == EFS_ERR_NOENT) 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotFound, 
				     NULL);
	else 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);
	
	return CORBA_OBJECT_NIL;
}

static void
real_set_info (BonoboStorage *storage,
	       const CORBA_char *path,
	       const Bonobo_StorageInfo *info,
	       const Bonobo_StorageInfoFields mask,
	       CORBA_Environment *ev)
{
	BonoboStorageEFS *storage_efs = BONOBO_STORAGE_EFS (storage);
	EFSNode *node = NULL;
	EFSResult result = EFS_ERR_PERM;

	if (mask !=  Bonobo_FIELD_CONTENT_TYPE) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
		return;
	}

	if ((result = efs_node_open 
	     (&node, storage_efs->dir, path, EFS_WRITE, 0)))
		goto set_info_except;

	if ((result = efs_strtype_set (node, info->content_type)))
		goto set_info_except;
	
	efs_node_close (node);

	return;

 set_info_except:

	if (node)
		efs_node_close (node);
	
	if (result == EFS_ERR_PERM) 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NoPermission, 
				     NULL);
	else if (result == EFS_ERR_NOENT) 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotFound, 
				     NULL);
	else 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);	
}

static BonoboStream *
real_open_stream (BonoboStorage *storage, const CORBA_char *path, 
		  Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	BonoboStorageEFS *storage_efs = BONOBO_STORAGE_EFS (storage);

	return bonobo_stream_efs_open (storage_efs, path, mode, ev);
}


static BonoboStorage *
real_open_storage (BonoboStorage *storage, const CORBA_char *path, 
		   Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	BonoboStorageEFS *storage_efs = BONOBO_STORAGE_EFS (storage);
	BonoboStorageEFS *sefs;
	EFSResult result;
	EFSDir *dir;
	gint efs_mode;

	efs_mode = bonobo_mode_to_efs (mode);

	if ((result = efs_dir_open (&dir, storage_efs->dir, path, efs_mode))) {

		if (result == EFS_ERR_PERM)
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NoPermission, 
					     NULL);
		else if (result == EFS_ERR_NOENT) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NotFound, 
					     NULL);
		else if (result == EFS_ERR_NOTDIR) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NotStorage, 
					     NULL);
		else if (result == EFS_ERR_EXISTS) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NameExists, 
					     NULL);
		else 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_IOError, 
					     NULL);
		return CORBA_OBJECT_NIL;
	}

	sefs = gtk_type_new (bonobo_storage_efs_get_type ());
	sefs->dir = dir;
	sefs->owner = storage;
	bonobo_object_ref (BONOBO_OBJECT (storage));

	return BONOBO_STORAGE (sefs);
}

static void
real_rename (BonoboStorage *storage, const CORBA_char *path, 
	     const CORBA_char *new_path, CORBA_Environment *ev)
{
	BonoboStorageEFS *storage_efs = BONOBO_STORAGE_EFS (storage);
	EFSResult result;

	if ((result = efs_rename (storage_efs->dir, path, new_path))) {

		if (result == EFS_ERR_PERM)
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NoPermission, 
					     NULL);
		else if (result == EFS_ERR_NOENT) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NotFound, 
					     NULL);
		else if (result == EFS_ERR_EXISTS) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NameExists, 
					     NULL);
		else 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_IOError, 
					     NULL);
	}
}

static void
real_commit (BonoboStorage *storage, CORBA_Environment *ev)
{
	BonoboStorageEFS *storage_efs = BONOBO_STORAGE_EFS (storage);
	EFSResult result;

	if (storage_efs->owner) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, 
				     NULL);
		return;
	}

	if ((result = efs_commit (storage_efs->dir))) {
		if (result == EFS_ERR_PERM)
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NoPermission, 
					     NULL);
		else 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_IOError, 
					     NULL);
	}
}

static void
real_revert (BonoboStorage *storage, CORBA_Environment *ev)
{
	BonoboStorageEFS *storage_efs = BONOBO_STORAGE_EFS (storage);
	EFSResult result;

	if (storage_efs->owner) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, 
				     NULL);
		return;
	}

	if ((result = efs_revert (storage_efs->dir))) {
		if (result == EFS_ERR_PERM)
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NoPermission, 
					     NULL);
		else 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_IOError, 
					     NULL);
	}
}


static Bonobo_Storage_DirectoryList *
real_list_contents (BonoboStorage *storage, const CORBA_char *path, 
		    Bonobo_StorageInfoFields mask, CORBA_Environment *ev)
{
	BonoboStorageEFS *storage_efs = BONOBO_STORAGE_EFS (storage);
	Bonobo_Storage_DirectoryList *list = NULL;
	Bonobo_StorageInfo *buf;
	EFSDirEntry de;
	EFSResult result;
	EFSDir *dir = NULL;
	EFSNode *node = NULL;
	EFSStat st;
	gchar *ctype;
	int i, max;
	gboolean onode;

	if (mask & ~(Bonobo_FIELD_CONTENT_TYPE | Bonobo_FIELD_SIZE |
		     Bonobo_FIELD_TYPE)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
		return CORBA_OBJECT_NIL;
	}

	if ((result = efs_dir_open (&dir, storage_efs->dir, path, 0)))
		goto list_contents_except;
	
	for (max = 0; !(efs_dir_read (dir, &de)); max++)
		/* do nothing */;
		
	if ((result = efs_dir_seek (dir, 0)))
		goto list_contents_except;
		
	buf = CORBA_sequence_Bonobo_StorageInfo_allocbuf (max);
	list = Bonobo_Storage_DirectoryList__alloc();
	list->_buffer = buf;
	CORBA_sequence_set_release (list, TRUE); 
	
	for (i = 0; !(efs_dir_read (dir, &de)) && (i < max); i++) {
		
		buf [i].name = CORBA_string_dup (de.name);
		buf [i].size = 0;
			
		if (de.type == EFS_DIR)
			buf [i].type = Bonobo_STORAGE_TYPE_DIRECTORY;
		else 
			buf [i].type = Bonobo_STORAGE_TYPE_REGULAR;

		onode = (mask & (Bonobo_FIELD_CONTENT_TYPE|Bonobo_FIELD_SIZE));

		if (onode && (result = efs_node_open (&node, dir, de.name, 
						      EFS_READ, 0)))
			goto list_contents_except;
			
		if (mask & Bonobo_FIELD_SIZE) { 
			if ((result = efs_node_stat (node, &st)))
				goto list_contents_except;
			buf [i].size = st.size;
		}
					
		if (mask & Bonobo_FIELD_CONTENT_TYPE) { 
			if ((result = efs_strtype_get(node, &ctype)))
				goto list_contents_except;
			buf [i].content_type = CORBA_string_dup(ctype);
		} else 
			buf [i].content_type = CORBA_string_dup(""); 

		if (onode)
			efs_node_close (node);
	}

	list->_length = i;
	
	efs_dir_close (dir);
	
	return list; 

 list_contents_except:

	if (node)
		efs_node_close (node);

	if (dir)
		efs_dir_close (dir);

	if (list) 
		CORBA_free (list);
	
	if (result == EFS_ERR_NOENT) 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotFound, 
				     NULL);
	else if (result == EFS_ERR_NOTDIR) 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotStorage, 
				     NULL);
	else 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);
	
	return CORBA_OBJECT_NIL;
}


static void
real_erase (BonoboStorage *storage, const CORBA_char *path,
	    CORBA_Environment *ev)
{
	BonoboStorageEFS *storage_efs = BONOBO_STORAGE_EFS (storage);
        EFSResult result;

	if ((result = efs_erase (storage_efs->dir, path))) {

		if (result == EFS_ERR_NOENT) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NotFound, 
					     NULL);
		else if (result == EFS_ERR_NOTEMPTY) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NotEmpty, 
					     NULL);
		else if (result == EFS_ERR_PERM) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NoPermission, 
					     NULL);
		else 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_IOError, NULL);
	}
}

static void
bonobo_storage_efs_class_init (BonoboStorageEFSClass *class)
{
	GtkObjectClass     *oclass = (GtkObjectClass *) class;
	BonoboStorageClass *sclass = (BonoboStorageClass *) class;

	sclass->get_info       = real_get_info;
	sclass->set_info       = real_set_info;
	sclass->open_stream    = real_open_stream;
	sclass->open_storage   = real_open_storage;
	sclass->copy_to        = NULL; /* use the generic method */
	sclass->rename         = real_rename;
	sclass->commit         = real_commit;
	sclass->revert         = real_revert;
	sclass->list_contents  = real_list_contents;
	sclass->erase          = real_erase;
	
	oclass->destroy = bonobo_storage_efs_destroy;
}

static void 
bonobo_storage_efs_init (GtkObject *object)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC (BonoboStorageEFS, 
		      bonobo_storage_get_type (),
		      bonobo_storage_efs);

/** 
 * bonobo_storage_efs_open:
 * @path: path to a file that represents the storage
 * @flags: flags
 * @mode: mode
 *
 * Opens a BonoboStorage object in @path. The storage opened is an activated
 * CORBA server for this storage.
 *
 * Returns the BonoboStorage GTK object.
 */
static BonoboStorage *
bonobo_storage_efs_open (const gchar *path, gint flags, gint mode, 
			 CORBA_Environment *ev)
{
	BonoboStorageEFS *sefs;
	EFSResult result;
	gint efs_flags = 0;

	efs_flags = bonobo_mode_to_efs (flags);

	sefs = gtk_type_new (bonobo_storage_efs_get_type ());

	if ((result = efs_open (&sefs->dir, path, efs_flags, mode, NULL))) {
		bonobo_object_unref (BONOBO_OBJECT (sefs));

		if (result == EFS_ERR_NOENT) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NotFound, 
					     NULL);
		else if (result == EFS_ERR_PERM) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NoPermission, 
					     NULL);
		else if (result == EFS_ERR_EXISTS) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NameExists, 
					     NULL);
		else if (result == EFS_ERR_NOTFILE) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_NotStorage, 
					     NULL);
		else 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_IOError, NULL);
		return CORBA_OBJECT_NIL;
	}

	return BONOBO_STORAGE (sefs);
}

gint 
init_storage_plugin (StoragePlugin *plugin)
{
	g_return_val_if_fail (plugin != NULL, -1);

	plugin->name = "efs";
	plugin->description = "Embedded Filesystem Driver";
	plugin->version = BONOBO_STORAGE_VERSION;
	
	plugin->storage_open = bonobo_storage_efs_open; 

	return 0;
}
