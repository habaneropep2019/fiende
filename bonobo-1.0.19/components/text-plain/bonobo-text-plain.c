/*
 * An embeddable Bonobo component to display text/plain.
 *
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 */
#include <config.h>

#include <bonobo.h>

#undef BONOBO_TEXT_PLAIN_TEST_UI_HANDLER

#ifdef BONOBO_TEXT_PLAIN_TEST_UI_HANDLER
/* XPM */
static char * bolt_xpm[] = {
"10 18 4 1",
" 	c None",
".	c #000000",
"+	c #F2FF00",
"@	c #000000",
"   .......",
"  .++++++.",
"  .+++++@.",
" .+++++@. ",
" .++++@.  ",
"..+++@.   ",
".+++@.... ",
".+++++++. ",
"....+++@. ",
"  .+++@.  ",
"..+++@.   ",
".++@@.    ",
".+++++.   ",
"...++@.   ",
" .++@.    ",
".++@.     ",
".+@.      ",
"...       "};
#endif

/*
 * BonoboControl data
 */
typedef struct {
        BonoboControl     *bonobo_object;

	gchar             *text;
        guint              text_len;

        /* TRUE if data is coming in from ProgressiveDataSink. */
        gboolean           progressive_data;

        /*
         * The total amount of the text data we're expecting from the
         * ProgressiveDataSink interface.
         */
        size_t             total_size;

        GtkWidget         *progress;
        GtkWidget         *text_widget;
	guint              text_widget_text_len;

        gboolean           active;

        /* UI stuff. */
        BonoboUIComponent *uic;
        Bonobo_UIContainer remote_uic;

        /* Internal widgets. */
        GtkWidget         *sw;
        GtkWidget         *table;
} bonobo_object_data_t;

static void
destroy_control (bonobo_object_data_t *bonobo_object_data)
{
	g_free (bonobo_object_data);
}

static void
free_text (bonobo_object_data_t *bonobo_object_data)
{
	 if (bonobo_object_data->text != NULL)
		 g_free (bonobo_object_data->text);
	 bonobo_object_data->text = NULL;
	 bonobo_object_data->text_len = 0;
}

static void
blank_control (bonobo_object_data_t *bonobo_object_data)
{
	if (bonobo_object_data->text_widget_text_len == 0)
		return;

	gtk_text_freeze (GTK_TEXT (bonobo_object_data->text_widget));

	/*
	 * Seek to the end of the text and delete it all.
	 */
	gtk_text_set_point (GTK_TEXT (bonobo_object_data->text_widget),
			    bonobo_object_data->text_widget_text_len);

	gtk_text_backward_delete (GTK_TEXT (bonobo_object_data->text_widget),
				  bonobo_object_data->text_widget_text_len);
		
	
	/*
	 * FIXME: This is a nasty hack.  I can't figure out how
	 * to get a GtkText widget to clear its focus area without
	 * doing something like this, though.
	 */
	gdk_window_clear_area (GTK_TEXT (bonobo_object_data->text_widget)->text_area,
			       0, 0,
			       bonobo_object_data->text_widget->allocation.width,
			       bonobo_object_data->text_widget->allocation.height);

	gtk_text_thaw (GTK_TEXT (bonobo_object_data->text_widget));

#ifndef NO_PROGRESS_METER
	/* Hide the progress meter. */
	gtk_widget_hide (bonobo_object_data->progress);
#endif /* ! NO_PROGRESS_METER */

	bonobo_object_data->text_widget_text_len = 0;
}

static void
update_control (bonobo_object_data_t *bonobo_object_data)
{
	/*
	 * 1. Erase the control.
	 */
	blank_control (bonobo_object_data);

	/*
	 * 2. Draw the new text data into all the control.
	 */
	gtk_text_freeze (GTK_TEXT (bonobo_object_data->text_widget));

	/*
	 * 2.1 Insert the new text.
	 */
	gtk_text_insert (GTK_TEXT (bonobo_object_data->text_widget),
			 NULL, NULL, NULL,
			 bonobo_object_data->text,
			 bonobo_object_data->text_len);

	bonobo_object_data->text_widget_text_len = bonobo_object_data->text_len;

	/*
	 * 2.2 Jump back to the beginning so that the control is
	 * positioned at the beginning of the text.
	 */
	gtk_text_set_point (GTK_TEXT (bonobo_object_data->text_widget), 0);

	gtk_text_thaw (GTK_TEXT (bonobo_object_data->text_widget));
}
		  
static void
progressive_update (bonobo_object_data_t *bonobo_object_data, char *buff, size_t count)
{
	/*
	 * 1. Append the new text data to the bonobo_object's text buffer.
	 */
	bonobo_object_data->text = g_realloc (bonobo_object_data->text,
					      bonobo_object_data->text_len
					      + count);

	memcpy (bonobo_object_data->text + bonobo_object_data->text_len,
		buff, count);

	bonobo_object_data->text_len += count;

	/*
	 * 2. Append the new text data to the end of the control.
	 */
	gtk_text_freeze (GTK_TEXT (bonobo_object_data->text_widget));

	/*
	 * 2.1 Jump to the end and append the new text.
	 */
	gtk_text_set_point (GTK_TEXT (bonobo_object_data->text_widget),
			    bonobo_object_data->text_widget_text_len);

	gtk_text_insert (GTK_TEXT (bonobo_object_data->text_widget),
			 NULL, NULL, NULL,
			 buff, count);

	bonobo_object_data->text_widget_text_len += count;

	/*
	 * 2.2 Jump back to the beginning so that when the
	 * file is done loading, the control is positioned at the
	 * beginning.
	 */
	gtk_text_set_point (GTK_TEXT (bonobo_object_data->text_widget), 0);

#ifndef NO_PROGRESS_METER
	if (bonobo_object_data->total_size != 0) {
		gtk_progress_bar_update (GTK_PROGRESS_BAR (bonobo_object_data->progress),
					 (float) bonobo_object_data->text_len /
					 bonobo_object_data->total_size);
	}

#endif /* ! NO_PROGRESS_METER */

	gtk_text_thaw (GTK_TEXT (bonobo_object_data->text_widget));

}

/*
 * Callbacks attached to signals.
 */
static void
control_system_exception_cb (BonoboControl *control, CORBA_Object corba_object,
			     CORBA_Environment *ev, gpointer data)
{
	g_warning ("Control system exception");
	bonobo_object_unref (BONOBO_OBJECT (control));
}

static void
embeddable_system_exception_cb (BonoboEmbeddable *embeddable, CORBA_Object corba_object,
				CORBA_Environment *ev, gpointer data)
{
	g_warning ("Embeddable system exception");
	bonobo_object_unref (BONOBO_OBJECT (embeddable));
}

static void
bonobo_object_destroy_cb (BonoboControl *bonobo_object,
			  bonobo_object_data_t *bonobo_object_data)
{
	free_text (bonobo_object_data);
	destroy_control (bonobo_object_data);
}

static gboolean
text_inserted_cb (GtkText *text_widget,
		  const gchar *text,
		  gint length,
		  gint *position,
		  bonobo_object_data_t *bonobo_object_data)
{
	if (length == 0)
		return FALSE;

	bonobo_object_data->text_widget_text_len += length;

	/*
	 * Update the BonoboObject's representation of the data being
	 * displayed.
	 */
	bonobo_object_data->text = g_realloc (bonobo_object_data->text,
					      bonobo_object_data->text_len + length);

	memmove (bonobo_object_data->text + *position + length,
		 bonobo_object_data->text + *position,
		 bonobo_object_data->text_len - *position);
	bonobo_object_data->text_len += length;

	memcpy (bonobo_object_data->text + *position, text, length);

	return FALSE;
}

static gboolean
text_deleted_cb (GtkText *text,
		 gint start_pos, gint end_pos,
		 bonobo_object_data_t *bonobo_object_data)
{
	bonobo_object_data->text_len -= end_pos - start_pos;

	/*
	 * Update the BonoboObject's representation of the data being
	 * displayed.
	 */
	memmove (bonobo_object_data->text + start_pos,
		 bonobo_object_data->text + end_pos,
		 bonobo_object_data->text_len - end_pos);
	bonobo_object_data->text_len -= end_pos - start_pos;
	bonobo_object_data->text = (char *) g_realloc (bonobo_object_data->text,
						       bonobo_object_data->text_len);
			
	return FALSE;
}

#ifdef BONOBO_TEXT_PLAIN_TEST_UI_HANDLER
static void
verb_ClearText_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	bonobo_object_data_t *bonobo_object_data = user_data;

	free_text (bonobo_object_data);
	blank_control (bonobo_object_data);
}
#endif

static void
create_control_menus (bonobo_object_data_t *bonobo_object_data)
{
	BonoboControl *control = bonobo_object_data->bonobo_object;
	Bonobo_UIContainer remote_uic;

#ifdef BONOBO_TEXT_PLAIN_TEST_UI_HANDLER
	BonoboUINode *parent, *node;
#endif

	/*
	 * Get our container's UIContainer server.
	 */
	remote_uic = bonobo_control_get_remote_ui_container (control);

	/*
	 * We have to deal gracefully with containers
	 * which don't have a UIContainer running.
	 */
	if (remote_uic == CORBA_OBJECT_NIL) {
		g_warning ("No UI container!");
		return;
	}

	/*
	 * Give our BonoboUIComponent object a reference to the
	 * container's UIContainer server.
	 */
	bonobo_ui_component_set_container (bonobo_object_data->uic, remote_uic);

	/*
	 * Unref the UI container we have been passed.
	 */
	bonobo_object_release_unref (remote_uic, NULL);

#ifdef BONOBO_TEXT_PLAIN_TEST_UI_HANDLER
	parent = bonobo_ui_util_new_menu (TRUE, "text-plain", "_text-plain",
					  "text-plain component", NULL);
	node = bonobo_ui_util_new_menu (FALSE, "ClearText", _("_Clear Text"),
					_("Clears the text in the component"),
					"ClearText");
	bonobo_ui_util_xml_set_pix_xpm (node, (const char **) bolt_xpm);
	bonobo_ui_node_add_child (parent, node);

	bonobo_ui_component_add_verb (bonobo_object_data->uic, "ClearText",
				      verb_ClearText_cb, bonobo_object_data);

	bonobo_ui_component_set_tree (bonobo_object_data->uic, "/menu", parent, NULL);
#endif
}

static void
control_activate_cb (BonoboControl *control, gboolean activate, gpointer data)
{
	bonobo_object_data_t *bonobo_object_data = (bonobo_object_data_t *) data;

	/*
	 * If we have been activated, create our menus.  If we have
	 * been deactivated, destroy them.
	 */
	if (activate)
		create_control_menus (bonobo_object_data);
	else
		bonobo_ui_component_unset_container (bonobo_object_data->uic);

	/*
	 * Notify the ControlFrame that we accept to be activated or
	 * deactivated (we are an acquiescent BonoboControl, yes we are).
	 */
	bonobo_control_activate_notify (control, activate);
}

/*
 * Bonobo::PersistStream
 *
 * These two functions implement the Bonobo::PersistStream load and
 * save methods which allow data to be loaded into and out of the
 * BonoboObject.
 */
static void
stream_read (Bonobo_Stream stream, bonobo_object_data_t *bonobo_object_data,
	     CORBA_Environment *ev)
{
	Bonobo_Stream_iobuf *buffer;

	do {
#define READ_CHUNK_SIZE 65536
		Bonobo_Stream_read (stream, READ_CHUNK_SIZE,
				    &buffer, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			return;

		bonobo_object_data->text = g_realloc (bonobo_object_data->text,
						      bonobo_object_data->text_len +
						      buffer->_length);

		memcpy (bonobo_object_data->text
			+ bonobo_object_data->text_len,
			buffer->_buffer, buffer->_length);

		bonobo_object_data->text_len += buffer->_length;

		if (buffer->_length <= 0)
			break;

		CORBA_free (buffer);
	} while (1);

	CORBA_free (buffer);
} /* stream_read */

/*
 * This function implements the Bonobo::PersistStream:load method.
 */
static void
pstream_load (BonoboPersistStream *ps, const Bonobo_Stream stream,
	      Bonobo_Persist_ContentType type, void *data,
	      CORBA_Environment *ev)
{
	bonobo_object_data_t *bonobo_object_data = data;

	if (*type && g_strncasecmp (type, "text/", 5) != 0) {	    
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	/*
	 * 1. Free the old text data and blank the control.
	 */
	free_text (bonobo_object_data);
	blank_control (bonobo_object_data);

	/*
	 * 2. Read the new text data.
	 */
	stream_read (stream, bonobo_object_data, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return;

	/*
	 * 3. Update the display.
	 */
	update_control (bonobo_object_data);

} /* pstream_load */

/*
 * This function implements the Bonobo::PersistStream:save method.
 */
static void
pstream_save (BonoboPersistStream *ps, const Bonobo_Stream stream,
	      Bonobo_Persist_ContentType type, void *data,
	      CORBA_Environment *ev)
{
	bonobo_object_data_t *bonobo_object_data = data;
	Bonobo_Stream_iobuf *buffer;
	size_t pos;

	if (*type && g_strcasecmp (type, "text/plain") != 0) {	    
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	/*
	 * Write the text data into the stream.
	 *
	 * FIXME: Do we really _have_ to double-buffer the text?
	 */
	buffer = Bonobo_Stream_iobuf__alloc ();

	data = CORBA_sequence_CORBA_octet_allocbuf (bonobo_object_data->text_len);
	memcpy (data, bonobo_object_data->text, bonobo_object_data->text_len);

	buffer->_buffer = data;
	buffer->_length = bonobo_object_data->text_len;

	pos = 0;
	while (pos < bonobo_object_data->text_len) {
		
		Bonobo_Stream_write (stream, buffer, ev);

		if (ev->_major != CORBA_NO_EXCEPTION) {
			CORBA_free (buffer);
			CORBA_free (data);
			return;
		}

		pos += buffer->_length;
	}

	CORBA_free (buffer);
	CORBA_free (data);
} /* pstream_save */

static CORBA_long
pstream_get_max_size (BonoboPersistStream *ps, void *data,
		      CORBA_Environment *ev)
{
	bonobo_object_data_t *bonobo_object_data = data;

	return bonobo_object_data->text_len;
}

static Bonobo_Persist_ContentTypeList *
pstream_get_content_types (BonoboPersistStream *ps, void *closure,
			   CORBA_Environment *ev)
{
	return bonobo_persist_generate_content_types (1, "text/plain");
}

/*
 * Bonobo::ProgressiveDataSink
 *
 * These functions implement the ProgressiveDataSink interface
 * methods, which are used to send a slow stream of data to the widget
 * and have it update its controls progressively.
 */

static int
progressive_start (BonoboProgressiveDataSink *psink, void *data)
{
	bonobo_object_data_t * bonobo_object_data =
		(bonobo_object_data_t *) data;

	bonobo_object_data->progressive_data = TRUE;
	
	free_text (bonobo_object_data);
	blank_control (bonobo_object_data);

#ifndef NO_PROGRESS_METER
	gtk_progress_bar_update (GTK_PROGRESS_BAR (bonobo_object_data->progress), 0.0);
	gtk_widget_show (bonobo_object_data->progress);
#endif /* ! NO_PROGRESS_METER */	

	return 0;
} /* progressive_start */

static int
progressive_end (BonoboProgressiveDataSink *psink, void *data)
{
	bonobo_object_data_t * bonobo_object_data =
		(bonobo_object_data_t *) data;

	bonobo_object_data->progressive_data = FALSE;

#ifndef NO_PROGRESS_METER
	gtk_widget_hide (bonobo_object_data->progress);
#endif /* ! NO_PROGRESS_METER */

	return 0;
} /* progressive_end */

static int
progressive_set_size (BonoboProgressiveDataSink *psink,
		      CORBA_long size, void *data)
{
	bonobo_object_data_t * bonobo_object_data =
		(bonobo_object_data_t *) data;

	bonobo_object_data->total_size = size;

	return 0;
} /* progressive_set_size */

static int
progressive_add_data (BonoboProgressiveDataSink *psink,
		      const Bonobo_ProgressiveDataSink_iobuf *buffer,
		      void *data)
{
	bonobo_object_data_t *bonobo_object_data =
		(bonobo_object_data_t *) data;

	progressive_update (bonobo_object_data, (char *) buffer->_buffer,
			    (size_t) buffer->_length);

	return 0;
} /* progressive_add_data */

static BonoboObject *
generic_factory (BonoboGenericFactory *this, void *data)
{
	BonoboControl *bonobo_object;
	BonoboPersistStream *stream;
	BonoboProgressiveDataSink *psink;
	bonobo_object_data_t *bonobo_object_data;

	bonobo_object_data = g_new0 (bonobo_object_data_t, 1);
	if (!bonobo_object_data)
		return NULL;

	bonobo_object_data->text = NULL;
	bonobo_object_data->text_len = 0;

	bonobo_object_data->text_widget = gtk_text_new (NULL, NULL);

	/*
	 * This table will contain the GtkText widget and, below it,
	 * a progress meter which will appear while progressive data
	 * is being loaded into the BonoboObject.
	 */
	bonobo_object_data->table = gtk_table_new (2, 1, FALSE);

	/*
	 * Make the widget editable and catch the relevant change
	 * signals so that we can update the other controls.
	 */
	gtk_text_set_editable (GTK_TEXT (bonobo_object_data->text_widget), TRUE);

	gtk_signal_connect (GTK_OBJECT (bonobo_object_data->text_widget), "insert_text",
			    GTK_SIGNAL_FUNC (text_inserted_cb), bonobo_object_data);
	gtk_signal_connect (GTK_OBJECT (bonobo_object_data->text_widget), "delete_text",
			    GTK_SIGNAL_FUNC (text_deleted_cb), bonobo_object_data);


	/*
	 * Stuff it into a scrolled window.
	 */
	bonobo_object_data->sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (bonobo_object_data->sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (bonobo_object_data->sw), bonobo_object_data->text_widget);

	gtk_table_attach (GTK_TABLE (bonobo_object_data->table), bonobo_object_data->sw,
			  0, 1, 0, 1,
			  (GTK_EXPAND | GTK_FILL), (GTK_EXPAND | GTK_FILL),
			  0, 0);

	/*
	 * Now create the GtkProgress bar which will be displayed when
	 * data is being loaded into the BonoboObject with the
	 * Bonobo::ProgressiveDataSink interface.
	 */
	bonobo_object_data->progress = gtk_progress_bar_new ();
	gtk_table_attach (GTK_TABLE (bonobo_object_data->table), bonobo_object_data->progress,
			  0, 1, 1, 2,
			  (GTK_EXPAND | GTK_FILL), 0,
			  0, 0);

        gtk_widget_show_all (bonobo_object_data->table);

	if (! bonobo_object_data->progressive_data)
		gtk_widget_hide (bonobo_object_data->progress);

	/*
	 * Now create the BonoboControl object.
	 */
	bonobo_object = bonobo_control_new (bonobo_object_data->table);
	bonobo_object_data->bonobo_object = bonobo_object;
	if (bonobo_object == NULL) {
		gtk_widget_destroy (bonobo_object_data->table);
		g_free (bonobo_object_data);
		return NULL;
	}

	bonobo_object_data->uic = bonobo_control_get_ui_component (bonobo_object);

	gtk_signal_connect (GTK_OBJECT (bonobo_object), "system_exception",
			    GTK_SIGNAL_FUNC (control_system_exception_cb), bonobo_object_data);

	gtk_signal_connect (GTK_OBJECT (bonobo_object), "activate",
			    GTK_SIGNAL_FUNC (control_activate_cb), bonobo_object_data);

	gtk_signal_connect (GTK_OBJECT (bonobo_object), "destroy",
			    GTK_SIGNAL_FUNC (bonobo_object_destroy_cb),
			    bonobo_object_data);

	gtk_signal_connect (GTK_OBJECT (bonobo_object), "system_exception",
			    GTK_SIGNAL_FUNC (embeddable_system_exception_cb),
			    bonobo_object_data);

	bonobo_object_data->bonobo_object = bonobo_object;

	/*
	 * Register the Bonobo::PersistStream interface.
	 */
	stream = bonobo_persist_stream_new (pstream_load, pstream_save,
					    pstream_get_max_size,
					    pstream_get_content_types,
					    bonobo_object_data);
	if (!stream) {
		bonobo_object_unref (BONOBO_OBJECT (bonobo_object));
		g_free (bonobo_object_data);
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (bonobo_object),
				     BONOBO_OBJECT (stream));


	/*
	 * Register the Bonobo::ProgressiveDataSink interface.
	 */
	psink = bonobo_progressive_data_sink_new (progressive_start,
						  progressive_end,
						  progressive_add_data,
						  progressive_set_size,
						  bonobo_object_data);
	if (!psink) {
		bonobo_object_unref (BONOBO_OBJECT (bonobo_object));
		g_free (bonobo_object_data);
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (bonobo_object),
				     BONOBO_OBJECT (psink));

	return BONOBO_OBJECT (bonobo_object);
} /* generic_factory */


BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Sample_Text_Factory",
		    "bonobo-text-plain", VERSION,
		    generic_factory,
		    NULL)
