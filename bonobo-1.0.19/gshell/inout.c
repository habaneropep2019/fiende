/*
 * input/output routines
 *
 * based on sample-container from  Nat Friedman (nat@nat.org)
 *
 * Author:
 *   Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright 1999 Maurer IT Systemlösungen (http://www.maurer-it.com)
 */

#include <config.h>
#include <fcntl.h>
#include "inout.h"
#include "ui.h"

static gint last_verb_id = 0;

static void
buffer_save_pf (Buffer *buffer, gchar *filename)
{
	Bonobo_PersistFile persist;
	CORBA_Environment ev;

	g_return_if_fail (buffer != NULL);
	g_return_if_fail (filename != NULL);
	
	CORBA_exception_init (&ev);

	persist = Bonobo_Unknown_queryInterface (
	        BONOBO_OBJREF (buffer->server),
		"IDL:Bonobo/PersistFile:1.0",
		&ev);
	
	if (ev._major != CORBA_NO_EXCEPTION || persist == CORBA_OBJECT_NIL) {
		g_warning ("component doesn't support PersistFile!");
		CORBA_exception_free (&ev);
		return;
	}

	/* fixme: only save when dirty - this is currently not 
	   supported by most components */
	/*
	if (!Bonobo_PersistFile_isDirty (persist, &ev)) {
	        g_warning ("No changes need to be saved");
		Bonobo_Unknown_unref (persist, &ev);
		CORBA_exception_free (&ev);
		return;
	}
	*/

	Bonobo_PersistFile_save (persist, (CORBA_char *) filename, &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		gnome_warning_dialog (_("An exception occured while trying "
					"to save data from the component with "
					"PersistFile"));
	}
	if (ev._major != CORBA_SYSTEM_EXCEPTION)
		CORBA_Object_release (persist, &ev);

	Bonobo_Unknown_unref (persist, &ev);
	CORBA_exception_free (&ev);
}

static void
buffer_load_pf (Buffer *buffer, gchar *filename)
{
	Bonobo_PersistFile persist;
	CORBA_Environment ev;

	g_return_if_fail (buffer != NULL);
	g_return_if_fail (filename != NULL);
	
	CORBA_exception_init (&ev);

	persist = Bonobo_Unknown_queryInterface (
	        BONOBO_OBJREF (buffer->server),
		"IDL:Bonobo/PersistFile:1.0",
		&ev);

	if (ev._major != CORBA_NO_EXCEPTION || persist == CORBA_OBJECT_NIL) {
		g_warning ("component doesn't support PersistFile!");
		CORBA_exception_free (&ev);
		return;
	}

	Bonobo_PersistFile_load (persist, (CORBA_char *) filename, &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		gnome_warning_dialog (_("An exception occured while trying "
					"to load data into the component with "
					"PersistFile"));
	}
	if (ev._major != CORBA_SYSTEM_EXCEPTION)
		CORBA_Object_release (persist, &ev);

	Bonobo_Unknown_unref (persist, &ev);	
	CORBA_exception_free (&ev);
}

static void
buffer_save_ps (Buffer *buffer, gchar *filename)
{
	Bonobo_PersistStream persist;
	CORBA_Environment ev;
	BonoboStream *stream;

	g_return_if_fail (buffer != NULL);
	g_return_if_fail (filename != NULL);
	
	if (!(stream = bonobo_stream_open (
		"fs", filename, Bonobo_Storage_WRITE, O_WRONLY))) {
		char *error_msg;
		error_msg = g_strdup_printf (_("Could not save file %s"), 
					     filename);
		gnome_warning_dialog (error_msg);
		g_free (error_msg);
		return;
	}

	CORBA_exception_init (&ev);

	persist = Bonobo_Unknown_queryInterface (
	        BONOBO_OBJREF (buffer->server),
		"IDL:Bonobo/PersistStream:1.0",
		&ev);
	
	if (ev._major != CORBA_NO_EXCEPTION || persist == CORBA_OBJECT_NIL) {
		g_warning ("component doesn't support PersistStream!");
		CORBA_exception_free (&ev);
		return;
	}

	/* fixme: only save when dirty - this is currently not 
	   supported by most components */
	/*
	if (!Bonobo_PersistStream_isDirty (persist, &ev)) {
		g_warning ("No changes need to be saved");
		Bonobo_Unknown_unref (persist, &ev);
		CORBA_exception_free (&ev);
		return;
	}
	*/

	Bonobo_PersistStream_save (persist,
				   (Bonobo_Stream) BONOBO_OBJREF (stream),
				   "", &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		gnome_warning_dialog (_("An exception occured while trying "
					"to save data from the component with "
					"PersistStream"));
	}
	if (ev._major != CORBA_SYSTEM_EXCEPTION)
		CORBA_Object_release (persist, &ev);

	Bonobo_Unknown_unref (persist, &ev);
	CORBA_exception_free (&ev);
	bonobo_object_unref (BONOBO_OBJECT(stream));
}

static void
buffer_load_ps (Buffer *buffer, gchar *filename)
{
	Bonobo_PersistStream persist;
	CORBA_Environment ev;
	BonoboStream *stream;

	g_return_if_fail (buffer != NULL);
	g_return_if_fail (filename != NULL);
	
	if (!(stream = bonobo_stream_open (
		"fs", filename, Bonobo_Storage_READ, O_RDONLY))) {
		char *error_msg;
		error_msg = g_strdup_printf (_("Could not open file %s"), 
					     filename);
		gnome_warning_dialog (error_msg);
		g_free (error_msg);
		return;
	}

	CORBA_exception_init (&ev);

	persist = Bonobo_Unknown_queryInterface (
	        BONOBO_OBJREF (buffer->server),
		"IDL:Bonobo/PersistStream:1.0",
		&ev);

	if (ev._major != CORBA_NO_EXCEPTION || persist == CORBA_OBJECT_NIL) {
		g_warning ("component doesn't support PersistStream!");
		CORBA_exception_free (&ev);
		return;
	}

	Bonobo_PersistStream_load (persist,
				   (Bonobo_Stream) BONOBO_OBJREF (stream),
				   "", &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		gnome_warning_dialog (_("An exception occured while trying "
					"to load data into the component with "
					"PersistStream"));
	}
	if (ev._major != CORBA_SYSTEM_EXCEPTION)
		CORBA_Object_release (persist, &ev);

	Bonobo_Unknown_unref (persist, &ev);
	CORBA_exception_free (&ev);
	bonobo_object_unref (BONOBO_OBJECT(stream));
}

static void
file_save_as_cb (GtkWidget *widget, Frame *frame)
{
	Buffer *buffer;
	BonoboViewFrame *view_frame;
	gchar *name;

	name = g_strdup(gtk_file_selection_get_filename(app.fs));
	gtk_widget_destroy (GTK_WIDGET(app.fs));

	if (!(view_frame = get_active_view_frame (frame))) {
		g_warning ("unable to get active view frame");
		g_free (name);
		return;
	}

	if (!(buffer = gtk_object_get_data(GTK_OBJECT(view_frame),"gshell:buffer"))) {
		g_warning ("unable to get active buffer");
		g_free (name);
		return;
	}

	if (bonobo_object_client_has_interface(buffer->server,
					       "IDL:Bonobo/PersistFile:1.0",
					       NULL)) {
		buffer_save_pf (buffer, name);
	} else if (bonobo_object_client_has_interface(buffer->server,
						      "IDL:Bonobo/PersistStream:1.0",
						      NULL)) {
		buffer_save_ps (buffer, name);
	} else {
		g_warning ("no storage interface found");
	}

	g_free (name);
}

typedef struct
{
	Frame *frame;
	Buffer *buffer;
} BufferSelect_CbData;

static void
verb_BufferSelect_cb (BonoboUIComponent *component, BufferSelect_CbData *cb_data, const char *cname)
{
	Buffer *tmp;
	BonoboViewFrame *view_frame;
	gint pos = -1;

	g_message ("set_buffer: `%s'", cname);

	if ((view_frame = get_active_view_frame (cb_data->frame))) {
		tmp = gtk_object_get_data(GTK_OBJECT(view_frame), 
					  "gshell:buffer");
		if (tmp == cb_data->buffer) return;
		pos = view_remove (cb_data->frame, view_frame);
	}
	buffer_add_view (cb_data->buffer, cb_data->frame, pos);
}

void
file_open (Frame *frame, gchar *name)
{
	const gchar *mime, *bid;
	Buffer *buffer;
	BonoboViewFrame *view_frame;
	BufferSelect_CbData *cb_data;
	gint pos = -1;

	if (!g_file_test(name, G_FILE_TEST_ISFILE)) return;
	
	if (!(mime = gnome_mime_type_of_file (name))) {
		g_warning ("unable to detect mime type");
		return;
	}

#if USING_OAF
	if (!(bid = gnome_mime_get_value (mime, "default_component_iid"))) {
		g_warning ("unable to detect the default_component_iid");
		printf ("for mime type %s\n",mime);
		return;
	}
#else
	if (!(bid = gnome_mime_get_value (mime, "bonobo-goad-id"))) {
		g_warning ("unable to detect the bonobo-goad-id");
		printf ("for mime type %s\n",mime);
		return;
	}
#endif

	if (!(buffer = buffer_create (bid))) {
		g_warning ("unable to create the bonobo component");
		return;
	}
	
	buffer->name = g_strdup (name);
	buffer->verb_id = ++last_verb_id;
	buffer->verb = g_strdup_printf ("SelectBuffer%d", buffer->verb_id);

	cb_data = g_new0 (BufferSelect_CbData, 1);
	cb_data->frame = frame;
	cb_data->buffer = buffer;

	bonobo_ui_component_add_verb_full (frame->component, buffer->verb,
					   (BonoboUIVerbFn) verb_BufferSelect_cb,
					   cb_data, (GDestroyNotify) g_free);

	if ((view_frame = get_active_view_frame (frame))) {
		pos = view_remove (frame, view_frame);
	}

	if (bonobo_object_client_has_interface(buffer->server,
					       "IDL:Bonobo/PersistFile:1.0",
					       NULL)) {
		buffer_load_pf (buffer, name);
	} else if (bonobo_object_client_has_interface(buffer->server,
						      "IDL:Bonobo/PersistStream:1.0",
						      NULL)) {
		buffer_load_ps (buffer, name);
	} else {
		g_warning ("no storage interface found");
	}

	buffer_add_view (buffer, frame, pos);
	update_buffer_menu ();
}

static void
file_open_cb (GtkWidget *widget, Frame *frame)
{
	gchar *name;

	name = g_strdup (gtk_file_selection_get_filename (app.fs));
	gtk_widget_destroy (GTK_WIDGET(app.fs));
	app.fs = NULL;

	if (!name)
		return;

	file_open (frame, name);

	g_free(name);
}

static void
verb_FileOpen_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	g_return_if_fail(component != NULL);
	g_return_if_fail(BONOBO_IS_UI_COMPONENT(component));
	
	app.fs=GTK_FILE_SELECTION(gtk_file_selection_new(_("Open")));
	gtk_file_selection_hide_fileop_buttons(app.fs);
	gtk_signal_connect_object(GTK_OBJECT (app.fs->cancel_button), 
				  "clicked",
				  (GtkSignalFunc) gtk_widget_destroy,
				  (gpointer)app.fs);
	gtk_signal_connect(GTK_OBJECT(app.fs->ok_button), 
			   "clicked",
			   (GtkSignalFunc)file_open_cb,
			   (gpointer) frame);

	gtk_widget_show(GTK_WIDGET(app.fs));
	gtk_window_set_modal (GTK_WINDOW(app.fs), TRUE);
}

static void
verb_LaunchControl_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	const gchar *interfaces[] = { "IDL:Bonobo/Control:1.0", NULL };
	BonoboViewFrame *view_frame;
	BufferSelect_CbData *cb_data;
	GtkWidget *selector;
	Buffer *buffer;
	gchar *oaf_iid, *name;
	gint pos = -1, n;

	g_return_if_fail(component != NULL);
	g_return_if_fail(BONOBO_IS_UI_COMPONENT(component));

	selector = bonobo_selector_new (_("Launch component"), interfaces);
	n = gnome_dialog_run (GNOME_DIALOG (selector));

	if (n == -1)
		return;

	if (n != 0) {
		gtk_widget_destroy (selector);
		return;
	}

	oaf_iid = bonobo_selector_get_selected_id (BONOBO_SELECTOR (selector));
	name = bonobo_selector_get_selected_name (BONOBO_SELECTOR (selector));

	gtk_widget_destroy (selector);

	if (!(buffer = buffer_create_for_control (oaf_iid))) {
		g_warning ("unable to create the bonobo component");
		return;
	}
	
	buffer->name = g_strdup (name);
	buffer->verb_id = ++last_verb_id;
	buffer->verb = g_strdup_printf ("SelectBuffer%d", buffer->verb_id);

	cb_data = g_new0 (BufferSelect_CbData, 1);
	cb_data->frame = frame;
	cb_data->buffer = buffer;

	g_free (oaf_iid);
	g_free (name);

	bonobo_ui_component_add_verb_full (frame->component, buffer->verb,
					   (BonoboUIVerbFn) verb_BufferSelect_cb,
					   cb_data, (GDestroyNotify) g_free);

	if ((view_frame = get_active_view_frame (frame)))
		pos = view_remove (frame, view_frame);

	buffer_add_view (buffer, frame, pos);
	update_buffer_menu ();
}

static void
verb_LaunchEmbeddable_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	const gchar *interfaces[] = { "IDL:Bonobo/Embeddable:1.0", NULL };
	BonoboViewFrame *view_frame;
	BufferSelect_CbData *cb_data;
	GtkWidget *selector;
	Buffer *buffer;
	gchar *oaf_iid, *name;
	gint pos = -1, n;

	g_return_if_fail(component != NULL);
	g_return_if_fail(BONOBO_IS_UI_COMPONENT(component));

	selector = bonobo_selector_new (_("Launch component"), interfaces);
	n = gnome_dialog_run (GNOME_DIALOG (selector));

	if (n == -1)
		return;

	if (n != 0) {
	    gtk_widget_destroy (selector);
	    return;
	}

	oaf_iid = bonobo_selector_get_selected_id (BONOBO_SELECTOR (selector));
	name = bonobo_selector_get_selected_name (BONOBO_SELECTOR (selector));

	gtk_widget_destroy (selector);

	if (!(buffer = buffer_create (oaf_iid))) {
		g_warning ("unable to create the bonobo component");
		return;
	}
	
	buffer->name = g_strdup (name);
	buffer->verb_id = ++last_verb_id;
	buffer->verb = g_strdup_printf ("SelectBuffer%d", buffer->verb_id);

	cb_data = g_new0 (BufferSelect_CbData, 1);
	cb_data->frame = frame;
	cb_data->buffer = buffer;

	g_free (oaf_iid);
	g_free (name);

	bonobo_ui_component_add_verb_full (frame->component, buffer->verb,
					   (BonoboUIVerbFn) verb_BufferSelect_cb,
					   cb_data, (GDestroyNotify) g_free);

	if ((view_frame = get_active_view_frame (frame)))
		pos = view_remove (frame, view_frame);

	buffer_add_view (buffer, frame, pos);
	update_buffer_menu ();
}

static void
file_load (Frame *frame, gchar *name)
{
	BonoboViewFrame *view_frame;
	Buffer *buffer;
	gint pos = -1;

	if (!g_file_test(name, G_FILE_TEST_ISFILE)) return;
	
	if (!(view_frame = get_active_view_frame (frame))) {
		g_warning ("unable to get active view frame");
		return;
	}

	if (!(buffer = gtk_object_get_data(GTK_OBJECT(view_frame),"gshell:buffer"))) {
		g_warning ("unable to get active buffer");
		return;
	}

	if (bonobo_object_client_has_interface(buffer->server,
					       "IDL:Bonobo/PersistFile:1.0",
					       NULL)) {
		buffer_load_pf (buffer, name);
	} else if (bonobo_object_client_has_interface(buffer->server,
						      "IDL:Bonobo/PersistStream:1.0",
						      NULL)) {
		buffer_load_ps (buffer, name);
	} else {
		g_warning ("no storage interface found");
	}

	pos = view_remove (frame, view_frame);

	g_free (buffer->name);
	buffer->name = g_strdup (name);

	buffer_add_view (buffer, frame, pos);
	update_buffer_menu ();
}

static void
file_load_cb (GtkWidget *widget, Frame *frame)
{
	gchar *name;

	name = g_strdup(gtk_file_selection_get_filename(app.fs));
	gtk_widget_destroy (GTK_WIDGET(app.fs));
	app.fs = NULL;
	if (!name) return;

	file_load(frame, name);
	g_free(name);
}

static void
verb_FileLoad_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	g_return_if_fail(component != NULL);
	g_return_if_fail(BONOBO_IS_UI_COMPONENT(component));
	
	app.fs=GTK_FILE_SELECTION(gtk_file_selection_new(_("Load")));
	gtk_file_selection_hide_fileop_buttons(app.fs);
	gtk_signal_connect_object(GTK_OBJECT (app.fs->cancel_button), 
				  "clicked",
				  (GtkSignalFunc) gtk_widget_destroy,
				  (gpointer)app.fs);
	gtk_signal_connect(GTK_OBJECT(app.fs->ok_button), 
			   "clicked",
			   (GtkSignalFunc)file_load_cb,
			   (gpointer) frame);

	gtk_widget_show(GTK_WIDGET(app.fs));
	gtk_window_set_modal (GTK_WINDOW(app.fs), TRUE);
}

static void
verb_FileSave_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	Buffer *buffer;
	BonoboViewFrame *view_frame;

	if (!(view_frame = get_active_view_frame (frame))) {
		g_warning ("unable to get active view frame");
		return;
	}

	if (!(buffer = gtk_object_get_data(GTK_OBJECT(view_frame),"gshell:buffer"))) {
		g_warning ("unable to get active buffer");
		return;
	}

	if (bonobo_object_client_has_interface(buffer->server,
					       "IDL:Bonobo/PersistFile:1.0",
					       NULL)) {
		buffer_save_pf (buffer, buffer->name);
	} else if (bonobo_object_client_has_interface(buffer->server,
						      "IDL:Bonobo/PersistStream:1.0",
						      NULL)) {
		buffer_save_ps (buffer, buffer->name);
	} else {
		g_warning ("no storage interface found");
	}
}

static void
verb_FileSaveAs_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	BonoboViewFrame *view_frame;

	if (!(view_frame = get_active_view_frame (frame))) return;
	
	app.fs=GTK_FILE_SELECTION(gtk_file_selection_new(_("Save As")));
	gtk_file_selection_hide_fileop_buttons(app.fs);
	gtk_signal_connect_object(GTK_OBJECT (app.fs->cancel_button), 
				  "clicked",
				  (GtkSignalFunc) gtk_widget_destroy,
				  (gpointer)app.fs);
	gtk_signal_connect(GTK_OBJECT(app.fs->ok_button), 
			   "clicked",
			   (GtkSignalFunc)file_save_as_cb,
			   (gpointer) frame);

	gtk_widget_show(GTK_WIDGET(app.fs));
	gtk_window_set_modal (GTK_WINDOW(app.fs), TRUE);
}

BonoboUIVerb inout_verbs[] = {
    BONOBO_UI_UNSAFE_VERB ("FileOpen", verb_FileOpen_cb),
    BONOBO_UI_UNSAFE_VERB ("LaunchControl", verb_LaunchControl_cb),
    BONOBO_UI_UNSAFE_VERB ("LaunchEmbeddable", verb_LaunchEmbeddable_cb),
    BONOBO_UI_UNSAFE_VERB ("FileLoad", verb_FileLoad_cb),
    BONOBO_UI_UNSAFE_VERB ("FileSave", verb_FileSave_cb),
    BONOBO_UI_UNSAFE_VERB ("FileSaveAs", verb_FileSaveAs_cb),
    BONOBO_UI_VERB_END
};
