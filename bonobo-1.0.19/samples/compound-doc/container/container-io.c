#include <bonobo.h>

#include "container-io.h"
#include "embeddable-io.h"

#define STORAGE_TYPE "efs"


#define GOAD_FILE "goad.id"
#define DATA_FILE "data"

static void
save_component (BonoboStorage    *storage,
		SampleClientSite *site,
		int index)
{
	char *curr_dir = g_strdup_printf ("%08d", index);

	Bonobo_Storage corba_storage = BONOBO_OBJREF (storage);
	Bonobo_Storage corba_subdir;

	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	corba_subdir = Bonobo_Storage_openStorage 
		(corba_storage, curr_dir, Bonobo_Storage_CREATE, &ev);

	if (ev._major != CORBA_NO_EXCEPTION)
		g_warning ("Can't create '%s'", curr_dir);
	else {
		Bonobo_Stream       corba_stream;
		BonoboObjectClient *embeddable;

		corba_stream = Bonobo_Storage_openStream 
			(corba_subdir, GOAD_FILE,
			 Bonobo_Storage_CREATE | Bonobo_Storage_WRITE, &ev);

		if (BONOBO_EX (&ev)) {
			g_warning ("Exception opening stream '%s'",
				   bonobo_exception_get_text (&ev));
			return;
		}
		bonobo_stream_client_write_string (corba_stream,
						   site->obj_id,
						   TRUE, &ev);

		Bonobo_Unknown_unref (corba_stream, &ev);
		CORBA_Object_release (corba_stream, &ev);

		embeddable = bonobo_client_site_get_embeddable (
			BONOBO_CLIENT_SITE (site));

		corba_stream = Bonobo_Storage_openStream 
			(corba_subdir, DATA_FILE, Bonobo_Storage_CREATE, &ev);

		object_save (embeddable, corba_stream, &ev);

		Bonobo_Unknown_unref (corba_stream, &ev);
		CORBA_Object_release (corba_stream, &ev);

		Bonobo_Unknown_unref (corba_subdir, &ev);
		CORBA_Object_release (corba_subdir, &ev);
	}

	g_free (curr_dir);

	CORBA_exception_free (&ev);
}

static char *
load_component_id_stream_read (Bonobo_Stream      stream,
			       CORBA_Environment *ev)
{
	Bonobo_Stream_iobuf *buffer;
	GString             *str;
	char                *ans;

	str = g_string_sized_new (32);

	/* We will read the data in chunks of the specified size */
#define READ_CHUNK_SIZE 65536
	do {
		int i;

		Bonobo_Stream_read (stream, READ_CHUNK_SIZE, &buffer, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			return NULL;

		for (i = 0; i < buffer->_length; i++)
			g_string_append_c (str, buffer->_buffer [i]);

		if (buffer->_length <= 0)
			break;
		CORBA_free (buffer);
	} while (1);
#undef READ_CHUNK_SIZE
	CORBA_free (buffer);

	ans = str->str;
	g_string_free (str, FALSE);

	return ans;
}

static char *
load_component_id (Bonobo_Storage     storage,
		   CORBA_Environment *ev)
{
	Bonobo_Stream  corba_stream;
	char *goad_id;

	corba_stream = Bonobo_Storage_openStream (storage, GOAD_FILE,
						   Bonobo_Storage_READ, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return NULL;

	if (corba_stream) {
		goad_id = load_component_id_stream_read (corba_stream, ev);
		Bonobo_Unknown_unref (corba_stream, ev);
		CORBA_Object_release (corba_stream, ev);
	} else {
		g_warning ("Can't find '%s'", GOAD_FILE);
		goad_id = NULL;
	}

	return goad_id;
}

static void
load_component (SampleApp *app, BonoboStorage *storage, int index)
{
	char *curr_dir = g_strdup_printf ("%08d", index);
	char *goad_id;
	Bonobo_Storage corba_subdir;
	Bonobo_Storage corba_storage = BONOBO_OBJREF (storage);
	SampleClientSite *site;

	CORBA_Environment ev;
	CORBA_exception_init (&ev);

	corba_subdir = Bonobo_Storage_openStorage (corba_storage,
						    curr_dir,
						    Bonobo_Storage_READ,
						    &ev);
	goad_id = load_component_id (corba_subdir, &ev);
	if (goad_id) {
		Bonobo_Stream corba_stream;

		site = sample_app_add_component (app, goad_id);

		if (site) {
			BonoboObjectClient *embeddable;

			corba_stream =
				Bonobo_Storage_openStream (corba_subdir, DATA_FILE,
							    Bonobo_Storage_READ, &ev);

			if (ev._major != CORBA_NO_EXCEPTION)
				return;

			embeddable = bonobo_client_site_get_embeddable (
				BONOBO_CLIENT_SITE (site));

			object_load (embeddable, corba_stream, &ev);
		} else
			g_warning ("Component '%s' activation failed", goad_id);

		g_free (goad_id);
	}

	CORBA_exception_free (&ev);

	g_free (curr_dir);
}

void
sample_container_load (SampleApp *app, const char *filename)
{
	CORBA_Environment ev;
	BonoboStorage *storage;
	Bonobo_Storage corba_storage;
	Bonobo_Storage_DirectoryList *list;
	int i;

	storage = bonobo_storage_open (STORAGE_TYPE, filename,
				       Bonobo_Storage_READ |
				       Bonobo_Storage_WRITE |
				       Bonobo_Storage_CREATE,
				       0664);
	g_return_if_fail (storage);

	CORBA_exception_init (&ev);

	corba_storage = BONOBO_OBJREF (storage);

	list = Bonobo_Storage_listContents (corba_storage, "/", 0, &ev);

	if (!list) {
		CORBA_exception_free (&ev);
		return;
	}

	for (i = 0; i < list->_length; i++)
		load_component (app, storage, i);

	CORBA_free (list);
	CORBA_exception_free (&ev);
}

void
sample_container_save (SampleApp *app, const char *filename)
{
	CORBA_Environment ev;
	BonoboStorage *storage;
	Bonobo_Storage corba_storage;
	GList *components;
	int i;

	unlink (filename);
	storage = bonobo_storage_open (STORAGE_TYPE, filename,
				       Bonobo_Storage_READ |
				       Bonobo_Storage_WRITE |
				       Bonobo_Storage_CREATE,
				       0664);
	g_return_if_fail (storage);

	CORBA_exception_init (&ev);

	corba_storage = BONOBO_OBJREF (storage);

	i = 0;
	for (components = g_list_first (app->components);
	     components; components = g_list_next (components))
		save_component (storage, components->data, i++);

	Bonobo_Storage_commit (corba_storage, &ev);

	CORBA_exception_free (&ev);

	bonobo_object_unref (BONOBO_OBJECT (storage));
}
