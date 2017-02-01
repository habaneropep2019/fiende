/*
 * bonobo-stream-efs.c: libefs based Stream implementation
 *
 * Author:
 *   Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright (C) 2000 Maurer IT Systemlösungen KEG
 */

#include <config.h>
#include <storage-modules/bonobo-stream-efs.h>

gint
bonobo_mode_to_efs (Bonobo_Storage_OpenMode mode)
{
	gint efs_mode = 0;

	if (mode & Bonobo_Storage_READ)
		efs_mode |= EFS_READ;
	if (mode & Bonobo_Storage_WRITE)
		efs_mode |= EFS_WRITE;
	if (mode & Bonobo_Storage_CREATE)
		efs_mode |= EFS_CREATE;
	if (mode & Bonobo_Storage_FAILIFEXIST)
		efs_mode |= EFS_EXCL;
	if (mode & Bonobo_Storage_COMPRESSED)
		efs_mode |= EFS_COMP;

	return efs_mode;
}

static BonoboStream *
create_stream_efs_server (const BonoboStreamEFS *stream_efs)
{
	Bonobo_Stream corba_stream;

	corba_stream = bonobo_stream_corba_object_create (
		BONOBO_OBJECT (stream_efs));

	return BONOBO_STREAM (
		bonobo_object_construct (
			BONOBO_OBJECT (stream_efs), 
			corba_stream));
}

static void
bonobo_stream_efs_destroy (GtkObject *object)
{
	BonoboStreamEFS *stream_efs = BONOBO_STREAM_EFS (object);

	if (stream_efs->file)
		efs_file_close (stream_efs->file);

	stream_efs->file = NULL;

	if (stream_efs->storage) 
		bonobo_object_unref (BONOBO_OBJECT (stream_efs->storage));    
}

static Bonobo_StorageInfo*
real_get_info (BonoboStream *stream, 
	       const Bonobo_StorageInfoFields mask,
	       CORBA_Environment *ev)
{
	BonoboStreamEFS *stream_efs = BONOBO_STREAM_EFS (stream);
	Bonobo_StorageInfo *si;
	EFSResult result;
	EFSStat st;
	gchar *content_type = NULL;

	if (mask & ~(Bonobo_FIELD_CONTENT_TYPE | Bonobo_FIELD_SIZE |
		     Bonobo_FIELD_TYPE)) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
		return CORBA_OBJECT_NIL;
	}

	if ((result = efs_node_stat (stream_efs->file, &st)))
		goto get_info_except;
	
	if ((mask & Bonobo_FIELD_CONTENT_TYPE) &&
	    ((result = efs_strtype_get (stream_efs->file, &content_type)))) 
		goto get_info_except;

	si = Bonobo_StorageInfo__alloc ();

	si->size = st.size;
	si->type = Bonobo_STORAGE_TYPE_REGULAR;
	si->name = CORBA_string_dup ("");

	if (content_type) 
		si->content_type = CORBA_string_dup (content_type);
	else
		si->content_type = CORBA_string_dup ("");

	return si;

 get_info_except:

	if (result == EFS_ERR_PERM) 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Stream_NoPermission, 
				     NULL);
	else 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Stream_IOError, NULL);
	
	return CORBA_OBJECT_NIL;
}

static void
real_set_info (BonoboStream *stream, 
	       const Bonobo_StorageInfo *info,
	       const Bonobo_StorageInfoFields mask, 
	       CORBA_Environment *ev)
{
	BonoboStreamEFS *stream_efs = BONOBO_STREAM_EFS (stream);
	EFSResult result = EFS_ERR_PERM;

	if (mask !=  Bonobo_FIELD_CONTENT_TYPE) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_NotSupported, NULL);
		return;
	}
	
	if ((result = efs_strtype_set(stream_efs->file, info->content_type))) 
		goto set_info_except;

	return;

 set_info_except:

	if (result == EFS_ERR_PERM) 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Stream_NoPermission, 
				     NULL);
	else 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Stream_IOError, NULL);
}

static void
real_write (BonoboStream *stream, 
	    const Bonobo_Stream_iobuf *buffer,
	    CORBA_Environment *ev)
{
	BonoboStreamEFS *stream_efs = BONOBO_STREAM_EFS (stream);
	EFSResult result;

	if ((result = efs_file_write (stream_efs->file, buffer->_buffer, 
				   buffer->_length))) {
		
		if (result == EFS_ERR_PERM) 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Stream_NoPermission, 
					     NULL);
		else 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Stream_IOError, NULL);
	}
}

static void
real_read (BonoboStream *stream, 
	   CORBA_long count,
	   Bonobo_Stream_iobuf ** buffer, 
	   CORBA_Environment *ev)
{
	BonoboStreamEFS *stream_efs = BONOBO_STREAM_EFS (stream);
	CORBA_octet *data;
	gint32 bytes_read;
	EFSResult result;

	if (count < 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Stream_IOError, NULL);
		return;
	}

	*buffer = Bonobo_Stream_iobuf__alloc ();
	CORBA_sequence_set_release (*buffer, TRUE);
	data = CORBA_sequence_CORBA_octet_allocbuf (count);
	(*buffer)->_buffer = data;
	(*buffer)->_length = 0;

	if ((result = efs_file_read 
	     (stream_efs->file, data, count, &bytes_read))) {

		CORBA_free (*buffer);
		*buffer = NULL;

		if (result == EFS_ERR_PERM)
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Stream_NoPermission, 
					     NULL);
		else 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Stream_IOError, NULL);
	} else
		(*buffer)->_length = bytes_read;
}

static CORBA_long
real_seek (BonoboStream *stream, 
	   CORBA_long offset, 
	   Bonobo_Stream_SeekType whence, 
	   CORBA_Environment *ev)
{
	BonoboStreamEFS *stream_efs = BONOBO_STREAM_EFS (stream);
	EFSResult result;
	gint efs_whence;
	guint32 pos;

	if (whence == Bonobo_Stream_SEEK_CUR)
		efs_whence = EFS_SEEK_CUR;
	else if (whence == Bonobo_Stream_SEEK_END)
		efs_whence = EFS_SEEK_END;
	else
		efs_whence = EFS_SEEK_SET;
	
	if ((result = efs_file_seek(stream_efs->file, offset, efs_whence, 
				    &pos))) {
		
		if (result == EFS_ERR_NOSEEK)
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Stream_NotSupported, 
					     NULL);
		else
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Stream_IOError, NULL);
		return 0;
	}
	
	return pos;
}

static void
real_truncate (BonoboStream *stream, 
	       const CORBA_long new_size, 
	       CORBA_Environment *ev)
{
	BonoboStreamEFS *stream_efs = BONOBO_STREAM_EFS (stream);
	EFSResult result;

	result = efs_file_trunc (stream_efs->file, new_size);

	if (result == EFS_ERR_NOSEEK) 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Stream_NotSupported, NULL);

	else if (result == EFS_ERR_PERM) 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Stream_NoPermission, NULL);

	else if (result != EFS_ERR_OK) 
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Stream_IOError, NULL);

}

static void
real_copy_to (BonoboStream *stream, 
	      const CORBA_char *dest,
	      const CORBA_long bytes, 
	      CORBA_long *read_bytes,
	      CORBA_long *written_bytes, 
	      CORBA_Environment *ev)
{
	BonoboStreamEFS *efs_stream = BONOBO_STREAM_EFS (stream);
	gchar data [4096];
	gchar *content_type = NULL;
	CORBA_unsigned_long more = bytes;
	gint v;
	EFSFile *out_file = NULL;
	EFSResult result;

	*read_bytes = 0;
	*written_bytes = 0;

	if ((result = efs_file_open (&out_file, efs_stream->storage->dir, 
				     dest, EFS_CREATE|EFS_EXCL)))
		goto copy_to_except;

	if ((result = efs_strtype_get (efs_stream->file, &content_type)) ||
	    (result = efs_strtype_set(out_file, content_type)))		
		goto copy_to_except;

	do {
		if (bytes == -1) 
			more = sizeof (data);

		if ((result = efs_file_read (efs_stream->file, data, 
					     MIN (sizeof (data), more), &v)))
			goto copy_to_except;

		if (v <= 0) 
			break;

		*read_bytes += v;
		more -= v;

		if ((result = efs_file_write (out_file, data, v)))
			goto copy_to_except;

		*written_bytes += v;

	} while ((more > 0 || bytes == -1) && v > 0);

	efs_file_close (out_file);

	return;

 copy_to_except:

	if (out_file)
		efs_file_close (out_file);

	if (result == EFS_ERR_PERM)
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Stream_NoPermission, 
				     NULL);
	else
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Stream_IOError,
				     NULL);
}

static void
real_commit (BonoboStream *stream, 
	     CORBA_Environment *ev)
{
        CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
                             ex_Bonobo_Stream_NotSupported, NULL);
}

static void
real_revert (BonoboStream *stream, 
	     CORBA_Environment *ev)
{
        CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
                             ex_Bonobo_Stream_NotSupported, NULL);
}

static void
bonobo_stream_efs_class_init (BonoboStreamEFSClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	BonoboStreamClass *sclass = BONOBO_STREAM_CLASS (class);
	
	sclass->get_info = real_get_info;
	sclass->set_info = real_set_info;
	sclass->write    = real_write;
	sclass->read     = real_read;
	sclass->seek     = real_seek;
	sclass->truncate = real_truncate;
	sclass->copy_to  = real_copy_to;
	sclass->commit   = real_commit;
	sclass->revert   = real_revert;

	object_class->destroy = bonobo_stream_efs_destroy;
}

GtkType
bonobo_stream_efs_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"IDL:GNOME/StreamEFS:1.0",
			sizeof (BonoboStreamEFS),
			sizeof (BonoboStreamEFSClass),
			(GtkClassInitFunc) bonobo_stream_efs_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_stream_get_type (), &info);
	}
  
	return type;
}

BonoboStream *
bonobo_stream_efs_open (BonoboStorageEFS *storage, 
			const CORBA_char *path, 
			Bonobo_Storage_OpenMode mode,
			CORBA_Environment *ev)
{
	BonoboStreamEFS *stream;
	EFSResult result;
	gint efs_mode;

	if (!(stream = gtk_type_new (bonobo_stream_efs_get_type ()))) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);
		return NULL;
	}

	efs_mode = bonobo_mode_to_efs (mode);

	if ((result = efs_file_open 
	     (&stream->file, storage->dir, path, efs_mode))) {
		
		bonobo_object_unref (BONOBO_OBJECT (stream));

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
					     ex_Bonobo_Storage_NotStream, 
					     NULL);
		else 
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
					     ex_Bonobo_Storage_IOError, NULL);

		return NULL;
	}
	
	stream->storage = storage;
	bonobo_object_ref (BONOBO_OBJECT(storage));

	if (!create_stream_efs_server (stream)) {
		bonobo_object_unref (BONOBO_OBJECT (stream));
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
				     ex_Bonobo_Storage_IOError, NULL);
		return NULL;
	}

	return BONOBO_STREAM (stream);
}

