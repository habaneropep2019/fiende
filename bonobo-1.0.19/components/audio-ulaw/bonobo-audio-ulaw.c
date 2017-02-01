/*
 * A BonoboObject to display audio/ulaw.
 *
 * Author:
 *   Chris Lahey <clahey@umich.edu>
 *
 * Based on bonobo-text-plain.c by
 *   Nat Friedman (nat@gnome-support.com)
 *
 */
#include <config.h>
#include "bonobo-audio-ulaw.h"
#include "item-audio.h"
#include "color.h"

/*
 * Utility functions.
 */
static void
free_sound (bonobo_object_data_t *bonobo_object_data)
{
	if (bonobo_object_data->sound != NULL)
		g_free (bonobo_object_data->sound);
	bonobo_object_data->sound = NULL;
	bonobo_object_data->sound_len = 0;
} /* free_sound */

static void
update_control (bonobo_object_data_t *bonobo_object_data)
{
	gnome_canvas_set_scroll_region (GNOME_CANVAS (bonobo_object_data->canvas),
					0, 0, bonobo_object_data->sound_len, 256);
	
	gnome_canvas_item_set (bonobo_object_data->canvas_item,
			       "BonoboObjectData", bonobo_object_data, NULL);
} /* update_view_foreach */
		  
static gboolean
update_control_callback (gpointer data)
{
	bonobo_object_data_t *bonobo_object_data = (bonobo_object_data_t *) data;

	update_control (bonobo_object_data);

	bonobo_object_data->update_callback_id = 0;

	return FALSE;
}

static void
progressive_update (bonobo_object_data_t *bonobo_object_data,
		    char *buff, size_t count)
{
	/*
	 * 1. Append the new sound data to the control's sound buffer.
	 */
	bonobo_object_data->sound = g_realloc (bonobo_object_data->sound,
					       bonobo_object_data->sound_len
					       + count);

	memcpy (bonobo_object_data->sound + bonobo_object_data->sound_len,
		buff, count);

	bonobo_object_data->sound_len += count;

	if (bonobo_object_data->update_callback_id == 0) {
		bonobo_object_data->update_callback_id = gtk_timeout_add
			(10, update_control_callback, (gpointer) bonobo_object_data);
	}

} /* progressive_update */

/*
 * Bonobo::PersistStream
 *
 * These two functions implement the Bonobo::PersistStream load and
 * save methods which allow data to be loaded into and out of the
 * BonoboObject.
 */
static int
stream_read (Bonobo_Stream stream, bonobo_object_data_t *bonobo_object_data)
{
	Bonobo_Stream_iobuf *buffer;
	CORBA_Environment    ev;

	CORBA_exception_init (&ev);
	do {
#define READ_CHUNK_SIZE 65536
		Bonobo_Stream_read (stream, READ_CHUNK_SIZE,
				    &buffer, &ev);

		if (ev._major != CORBA_NO_EXCEPTION) {
			CORBA_exception_free (&ev);
			return -1;
		}

		bonobo_object_data->sound = g_realloc (bonobo_object_data->sound,
						       bonobo_object_data->sound_len +
						       buffer->_length);

		memcpy (bonobo_object_data->sound + bonobo_object_data->sound_len,
			buffer->_buffer, buffer->_length);

		bonobo_object_data->sound_len += buffer->_length;
		if (buffer->_length <= 0)
			break;

		CORBA_free (buffer);
	} while (1);

	CORBA_free (buffer);
	CORBA_exception_free (&ev);

	return 0;
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

	if (*type && g_strcasecmp (type, "audio/ulaw") != 0) {	    
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	/*
	 * 1. Free the old sound data.
	 */
	free_sound (bonobo_object_data);

	/*
	 * 2. Read the new sound data.
	 */
	if (stream_read (stream, bonobo_object_data) < 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_FileNotFound, NULL);
		return;
	}

	/*
	 * 3. Update the displays.
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
	Bonobo_Stream_iobuf  *buffer;
	size_t                pos;

	if (*type && g_strcasecmp (type, "audio/ulaw") != 0) {	    
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	/*
	 * Write the sound data into the stream.
	 *
	 * FIXME: Do we really _have_ to double-buffer the sound?
	 */
	buffer = Bonobo_Stream_iobuf__alloc ();

	data = CORBA_sequence_CORBA_octet_allocbuf (bonobo_object_data->sound_len);
	memcpy (data, bonobo_object_data->sound, bonobo_object_data->sound_len);

	buffer->_buffer = data;
	buffer->_length = bonobo_object_data->sound_len;

	pos = 0;
	
	while (pos < bonobo_object_data->sound_len) {
   
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

	return bonobo_object_data->sound_len;
}

static Bonobo_Persist_ContentTypeList *
pstream_get_content_types (BonoboPersistStream *ps, void *closure,
			   CORBA_Environment *ev)
{
	return bonobo_persist_generate_content_types (1, "audio/ulaw");
}

/*
 * Bonobo::ProgressiveDataSink
 *
 * These functions implement the ProgressiveDataSink interface
 * methods, which are used to send a slow stream of data to the widget
 * and have it update its views progressively.
 */

static int
progressive_start (BonoboProgressiveDataSink *psink, void *data)
{
	bonobo_object_data_t * bonobo_object_data = (bonobo_object_data_t *) data;

	free_sound (bonobo_object_data);
	update_control (bonobo_object_data);

	return 0;
} /* progressive_start */

static int
progressive_add_data (BonoboProgressiveDataSink *psink,
		      const Bonobo_ProgressiveDataSink_iobuf *buffer,
		      void *data)
{
	bonobo_object_data_t *bonobo_object_data = (bonobo_object_data_t *) data;

	progressive_update (bonobo_object_data, (char *) buffer->_buffer,
			    (size_t) buffer->_length);

	return 0;
} /* progressive_add_data */

/*
 * This callback is invoked when the BonoboControl object
 * encounters a fatal CORBA exception.
 */
static void
bonobo_object_system_exception_cb (BonoboObject *bonobo_object, CORBA_Object corba_object,
				   CORBA_Environment *ev, gpointer data)
{
	bonobo_object_unref (bonobo_object);
}

static void
bonobo_object_destroy_cb (BonoboObject *bonobo_object, bonobo_object_data_t *bonobo_object_data)
{
	if (bonobo_object_data->sound)
		g_free (bonobo_object_data->sound);
	bonobo_object_data->sound     = NULL;
	bonobo_object_data->sound_len = 0;

	gtk_object_unref (GTK_OBJECT (bonobo_object_data->vbox));

	g_free (bonobo_object_data); 
}

static BonoboObject *
generic_factory (BonoboGenericFactory *this,  void *data)
{
	BonoboControl *bonobo_object;
	BonoboPersistStream *stream;
	BonoboProgressiveDataSink *psink;
	bonobo_object_data_t *bonobo_object_data = data;

	bonobo_object_data = g_new0 (bonobo_object_data_t, 1);
	if (!bonobo_object_data)
		return NULL;
	
	bonobo_object_data->sound = NULL;
	bonobo_object_data->sound_len = 0;
	bonobo_object_data->update_callback_id = 0;

	bonobo_object_data->canvas = gnome_canvas_new ();
	gtk_widget_set_usize (GTK_WIDGET (bonobo_object_data->canvas), 256, 256);

	bonobo_object_data->canvas_item =
		gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (bonobo_object_data->canvas)),
				       item_audio_get_type (),
				       "BonoboObjectData", bonobo_object_data,
				       /* "height", 100.0, */
				       NULL);
	gnome_canvas_set_scroll_region (GNOME_CANVAS (bonobo_object_data->canvas),
					0, 0,
					bonobo_object_data->sound_len, 256);
	bonobo_object_data->hscroll = gtk_hscrollbar_new (gtk_layout_get_hadjustment
							  (GTK_LAYOUT (bonobo_object_data->canvas)));

	bonobo_object_data->vbox = gtk_vbox_new (0, FALSE);
	gtk_box_pack_start (GTK_BOX (bonobo_object_data->vbox), bonobo_object_data->canvas, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (bonobo_object_data->vbox), bonobo_object_data->hscroll, FALSE, FALSE, 0);
        gtk_widget_show_all (bonobo_object_data->vbox);

	/*
	 * Creates the Control.
	 */
	bonobo_object = bonobo_control_new (bonobo_object_data->vbox);
	if (bonobo_object == NULL) {
		gtk_object_unref (GTK_OBJECT (bonobo_object_data->vbox));
		g_free (bonobo_object_data);
		return NULL;
	}

	bonobo_object_data->bonobo_object = bonobo_object;

	/*
	 * Register the Bonobo::PersistStream interface.
	 */
	stream = bonobo_persist_stream_new (pstream_load, pstream_save,
					    pstream_get_max_size,
					    pstream_get_content_types,
					    bonobo_object_data);
	if (stream == NULL) {
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
						  NULL, /* progressive_end */
						  progressive_add_data,
						  NULL, /* progressive_set_size */
						  bonobo_object_data);
	
	if (psink == NULL) {
		bonobo_object_unref (BONOBO_OBJECT (bonobo_object));
		g_free (bonobo_object_data);
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (bonobo_object),
				     BONOBO_OBJECT (psink));

	gtk_signal_connect (GTK_OBJECT (bonobo_object), "destroy",
			    GTK_SIGNAL_FUNC (bonobo_object_destroy_cb),
			    bonobo_object_data);

	gtk_signal_connect (GTK_OBJECT (bonobo_object), "system_exception",
			    GTK_SIGNAL_FUNC (bonobo_object_system_exception_cb),
			    bonobo_object_data);

	return BONOBO_OBJECT (bonobo_object);
} /* generic_factory */

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Sample_Audio_ulaw_Factory",
		    "bonobo-audio-ulaw", VERSION,
		    generic_factory,
		    NULL)
