#ifndef HELLO_OBJECT_IO_H
#define HELLO_OBJECT_IO_H

void       hello_object_pstream_load (BonoboPersistStream         *ps,
				      const Bonobo_Stream          stream,
				      Bonobo_Persist_ContentType   type,
				      void                        *data,
				      CORBA_Environment           *ev);

void       hello_object_pstream_save (BonoboPersistStream         *ps,
				      const Bonobo_Stream          stream,
				      Bonobo_Persist_ContentType   type,
				      void                        *data,
				      CORBA_Environment           *ev);

CORBA_long hello_object_pstream_get_max_size (BonoboPersistStream *ps,
					      void                *data,
					      CORBA_Environment   *ev);
Bonobo_Persist_ContentTypeList *
            hello_object_pstream_get_types   (BonoboPersistStream *ps,
					      void                *data,
					      CORBA_Environment   *ev);

#endif
