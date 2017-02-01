#include "hello-embeddable.h"
#include "hello-object-io.h"

void
hello_object_pstream_load (BonoboPersistStream       *ps,
			   const Bonobo_Stream        stream,
			   Bonobo_Persist_ContentType type,
			   void                      *data,
			   CORBA_Environment         *ev)
{
	HelloBonoboEmbeddable *embeddable = data;
	char                  *str;

	/* FIXME: Check the Content Type */
	bonobo_stream_client_read_string (stream, &str, ev);
	if (ev->_major == CORBA_NO_EXCEPTION) {
		hello_bonobo_embeddable_set_text (embeddable, str);
		g_free (str);
	}
}

void
hello_object_pstream_save (BonoboPersistStream       *ps,
			   const Bonobo_Stream        stream,
			   Bonobo_Persist_ContentType type,
			   void                      *data,
			   CORBA_Environment         *ev)
{
	HelloBonoboEmbeddable *embeddable = data;

	bonobo_stream_client_write_string (
		stream, embeddable->text ? embeddable->text:"",
		TRUE, ev);
}

CORBA_long
hello_object_pstream_get_max_size (BonoboPersistStream *ps,
				   void                *data,
				   CORBA_Environment   *ev)
{
	HelloBonoboEmbeddable *embeddable = data;

	return embeddable ? embeddable->text ? strlen (embeddable->text) : 0 : 0;
}

Bonobo_Persist_ContentTypeList *
hello_object_pstream_get_types (BonoboPersistStream *ps,
				void                *closure,
				CORBA_Environment   *ev)
{
	/* FIXME */
	return bonobo_persist_generate_content_types (1, "");
}
