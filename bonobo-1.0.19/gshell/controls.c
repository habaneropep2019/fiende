/*
 * Author:
 *   Martin Baulig <baulig@suse.de>
 *
 * Copyright 2000 SuSE GmbH.
 */

#include <config.h>
#include <fcntl.h>
#include "gshell.h"
#include "controls.h"

#if USING_OAF
#include <liboaf/liboaf.h>
#else
#include <libgnorba/gnorba.h>
#endif

/*
 * BonoboObject data
 */
typedef struct {
	BonoboEmbeddable *bonobo_object;
	gchar *filename;
	gchar *goad_id;
	GList *view_list;
} bonobo_object_data_t;

/*
 * View data
 */
typedef struct {
	bonobo_object_data_t *bod;
	BonoboObjectClient   *server;
	BonoboControlFrame   *control_frame;
	GtkWidget            *root;
	BonoboView           *view;
} view_data_t;

static void
destroy_view (BonoboView *view, view_data_t *view_data)
{
	g_return_if_fail (view_data != NULL);

	/* this is not strictly necessary, but it gives us a little "protection"
	 * if we or our component are leaking a reference somewhere.
	 */
	bonobo_control_frame_control_deactivate (view_data->control_frame);

	/* we keep a reference to this since we bonobo_object_activate()d it. */
	bonobo_object_client_unref (view_data->server, NULL);

	/* the ControlFrame holds the second reference. */
	bonobo_object_unref (BONOBO_OBJECT (view_data->control_frame));
	gtk_widget_destroy (view_data->root);

	view_data->bod->view_list = g_list_remove (view_data->bod->view_list, view_data);

	g_free (view_data);
}

static void
view_activate_cb (BonoboView *view, gboolean activate, gpointer data)
{
	/*
	 * Notify the ViewFrame that we accept to be activated or
	 * deactivated (we are an acquiescent BonoboView, yes we are).
	 */
	bonobo_view_activate_notify (view, activate);
}

static gint
do_load_pf (view_data_t *view_data, const gchar *filename)
{
	Bonobo_PersistFile persist;
	CORBA_Environment ev;
	gint retval = 0;

	CORBA_exception_init (&ev);

	persist = Bonobo_Unknown_queryInterface (
                BONOBO_OBJREF (view_data->server),
		"IDL:Bonobo/PersistFile:1.0",
		&ev);

	if (ev._major != CORBA_NO_EXCEPTION || persist == CORBA_OBJECT_NIL) {
		g_warning ("component doesn't support PersistFile!");
		CORBA_exception_free (&ev);
		return -1;
	}

	Bonobo_PersistFile_load (persist, (CORBA_char *) filename, &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		gnome_warning_dialog (_("An exception occured while trying "
					"to load data into the component with "
					"PersistFile"));
		retval = -1;
	}
	if (ev._major != CORBA_SYSTEM_EXCEPTION)
		CORBA_Object_release (persist, &ev);

	Bonobo_Unknown_unref (persist, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

static gint
do_load_ps (view_data_t *view_data, const gchar *filename)
{
	Bonobo_PersistStream persist;
	CORBA_Environment ev;
	BonoboStream *stream;
	gint retval = 0;

	if (!(stream = bonobo_stream_open ("fs", filename, Bonobo_Storage_READ, O_RDONLY))) {
		char *error_msg;
		error_msg = g_strdup_printf (_("Could not open file %s"), 
					     filename);
		gnome_warning_dialog (error_msg);
		g_free (error_msg);
		return -1;
	}

	CORBA_exception_init (&ev);

	persist = Bonobo_Unknown_queryInterface (
	        BONOBO_OBJREF (view_data->server),
		"IDL:Bonobo/PersistStream:1.0",
		&ev);

	if (ev._major != CORBA_NO_EXCEPTION || persist == CORBA_OBJECT_NIL) {
		g_warning ("component doesn't support PersistStream!");
		CORBA_exception_free (&ev);
		return -1;
	}

	Bonobo_PersistStream_load (persist,
				   (Bonobo_Stream) BONOBO_OBJREF (stream), 
				   "", &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		gnome_warning_dialog (_("An exception occured while trying "
					"to load data into the component with "
					"PersistStream"));
		retval = -1;
	}
	if (ev._major != CORBA_SYSTEM_EXCEPTION)
		CORBA_Object_release (persist, &ev);

	Bonobo_Unknown_unref (persist, &ev);
	CORBA_exception_free (&ev);
	bonobo_object_unref (BONOBO_OBJECT(stream));

	return retval;
}

static gint
do_load_file (view_data_t *view_data, const gchar *filename)
{
	gint retval = 0;

	if (bonobo_object_client_has_interface (view_data->server,
						"IDL:Bonobo/PersistFile:1.0",
						NULL)) {
		retval = do_load_pf (view_data, filename);
	} else if (bonobo_object_client_has_interface (view_data->server,
						       "IDL:Bonobo/PersistStream:1.0",
						       NULL)) {
		retval = do_load_ps (view_data, filename);
	} else {
		g_warning ("no storage interface found");
		retval = -1;
	}

	return retval;
}

static BonoboView *
view_factory (BonoboEmbeddable *bonobo_object,
	      const Bonobo_ViewFrame view_frame,
	      void *data)
{
	view_data_t *view_data = g_new0 (view_data_t, 1);
	Bonobo_UIContainer corba_uic;
	Bonobo_Control corba_control;
	CORBA_Environment  ev;

	view_data->bod = (bonobo_object_data_t *) data;
	view_data->server = bonobo_object_activate (view_data->bod->goad_id, 0);

	corba_control = BONOBO_OBJREF (view_data->server);

	CORBA_exception_init (&ev);
	corba_uic = Bonobo_ViewFrame_getUIHandler (view_frame, &ev);
	bonobo_object_check_env (BONOBO_OBJECT (bonobo_object), view_frame, &ev);
	CORBA_exception_free (&ev);

	view_data->control_frame = bonobo_control_frame_new (corba_uic);
	bonobo_control_frame_bind_to_control (view_data->control_frame, corba_control);
	bonobo_object_release_unref (corba_control, NULL);

	view_data->root = bonobo_control_frame_get_widget (view_data->control_frame);

	bonobo_control_frame_set_autoactivate (view_data->control_frame, TRUE);

	gtk_widget_show_all (view_data->root);

	view_data->view = bonobo_view_new (view_data->root);

	gtk_signal_connect (GTK_OBJECT (view_data->view), "destroy",
			    GTK_SIGNAL_FUNC (destroy_view), view_data);

	gtk_signal_connect (GTK_OBJECT (view_data->view), "activate",
			    GTK_SIGNAL_FUNC (view_activate_cb), view_data);

	if (view_data->bod->filename)
		do_load_file (view_data, view_data->bod->filename);

	view_data->bod->view_list = g_list_prepend (view_data->bod->view_list, view_data);

	return view_data->view;
}

static int
load_file (BonoboPersistFile *pf, const CORBA_char *filename, CORBA_Environment *ev, void *closure)
{
	bonobo_object_data_t *bod = closure;
	GList *c;

	g_free (bod->filename);
	bod->filename = g_strdup (filename);

	for (c = bod->view_list; c; c = c->next) {
		gint retval;

		retval = do_load_file ((view_data_t *) c->data, filename);
		if (retval != 0)
			return retval;
	}

	return 0;
}

BonoboObjectClient *
gshell_control_wrapper (const gchar *goad_id)
{
	BonoboObjectClient *object_client;
	Bonobo_Embeddable corba_embeddable;
	BonoboEmbeddable *bonobo_object;
	bonobo_object_data_t *bod;
	BonoboPersistFile *pf;

	bod = g_new0 (bonobo_object_data_t, 1);
	bod->goad_id = g_strdup (goad_id);

	bonobo_object = bonobo_embeddable_new (view_factory, bod);

	pf = bonobo_persist_file_new (load_file, NULL, bod);

	bonobo_object_add_interface (BONOBO_OBJECT (bonobo_object),
				     BONOBO_OBJECT (pf));

	corba_embeddable = BONOBO_OBJREF (bonobo_object);
	object_client = bonobo_object_client_from_corba (corba_embeddable);

	return object_client;
}
