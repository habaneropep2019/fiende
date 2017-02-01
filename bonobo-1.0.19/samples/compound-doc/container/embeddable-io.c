/* $Id: embeddable-io.c,v 1.1 2000/07/25 01:06:29 mmeeks Exp $ */

#include "component.h"
#include "embeddable-io.h"

void
object_load (BonoboObjectClient *embeddable,
	     Bonobo_Stream       stream,
	     CORBA_Environment  *ev)
{
	Bonobo_PersistStream persist;

	if (stream == CORBA_OBJECT_NIL) {
		g_warning ("No stream to load component from");
		return;
	}

	/* Get the PersistStream interface of our component */
	persist = bonobo_object_client_query_interface (
		embeddable, "IDL:Bonobo/PersistStream:1.0", NULL);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return;

	if (persist == CORBA_OBJECT_NIL) {
		g_warning ("Component doesn't implement a PersistStream "
			   "interface, and it used to\n");
		return;
	}

	Bonobo_PersistStream_load (persist, stream, "", ev);

	/* See if we had any problems */
	if (ev->_major != CORBA_NO_EXCEPTION)
		gnome_warning_dialog (
			_("An exception occured while trying "
			  "to load data into the component with "
			  "PersistStorage"));

	Bonobo_Unknown_unref (persist, ev);
	CORBA_Object_release (persist, ev);
}

void
object_save (BonoboObjectClient *embeddable,
	     Bonobo_Stream       stream,
	     CORBA_Environment  *ev)
{
	Bonobo_PersistStream persist;

	/* Get the PersistStream interface of our component */
	persist = bonobo_object_client_query_interface (
		embeddable, "IDL:Bonobo/PersistStream:1.0", ev);

	if (ev->_major != CORBA_NO_EXCEPTION ||
	    persist == CORBA_OBJECT_NIL) {
		printf ("This component doesn't implement PersistStream\n"
			"If you are planning on using PersistFile, don't.");
		return;
	}

	Bonobo_PersistStream_save (persist, stream, "", ev);

	/* See if we had any problems */
	if (ev->_major != CORBA_NO_EXCEPTION)
		gnome_warning_dialog (_
				      ("An exception occured while trying "
				       "to save data from the component with "
				       "PersistStorage"));

	else {
		Bonobo_Unknown_unref (persist, ev);
		CORBA_Object_release (persist, ev);
	}
}
