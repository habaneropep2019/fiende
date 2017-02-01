/*
 * An embeddable "mines" component.
 *
 * This looks extremely useless, but so do you :-)
 *
 * Author:
 *   Michael Meeks <michael@imaginator.com>
 *
 */

#include <config.h>

#include <bonobo.h>

#include "minefield.h"

/* Include the face */
#include "face-sad.xpm"
#include "face-smile.xpm"
#include "face-win.xpm"
#include "face-cool.xpm"
#include "face-worried.xpm"

/*
 * BonoboControl data
 */
typedef struct {
	BonoboControl        *bonobo_object;
	BonoboUIComponent    *uic;

	MineField            *data;
	GtkMineFieldView     *mfield;
} control_data_t;

/*
 * This callback is invoked when the BonoboControl object
 * encounters a fatal CORBA exception.
 */
static void
control_system_exception_cb (BonoboControl *control, CORBA_Object corba_object,
			     CORBA_Environment *ev, gpointer data)
{
	bonobo_object_unref (BONOBO_OBJECT (control));
}

/*
 * This function updates all of an control's views to reflect the
 * image data stored in the control.
 */
static void
control_update (control_data_t *control_data)
{
	gtk_widget_queue_draw (GTK_WIDGET (control_data->mfield));
}

static void
load_board (BonoboPersistStream        *ps,
	    const Bonobo_Stream         stream,
	    Bonobo_Persist_ContentType  type,
	    void                       *closure,
	    CORBA_Environment          *ev)
{
	control_data_t       *control_data = closure;
	MineField            *mf;
	Bonobo_Stream_iobuf  *buffer;
	char                 *str;
	int                   bx, by, j;

	g_return_if_fail (control_data != NULL);
	g_return_if_fail (control_data->data != NULL);

	if (*type && g_strcasecmp (type, "application/x-mines") != 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	mf = control_data->data;

	bonobo_stream_client_read_string (stream, &str, ev);
	if (ev->_major != CORBA_NO_EXCEPTION || str == NULL) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}
	sscanf (str, "%2u%2u\n", &bx, &by);
	g_free (str);

	if (bx > 128 || by > 128) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	minefield_set_size (mf, bx, by);
	minefield_restart  (mf);
	
	for (j = 0; j < by; j++) {
		int i;

		Bonobo_Stream_read (stream, bx * 2 + 1, &buffer, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			return;
		else if (buffer->_length != bx * 2 + 1) {
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_Persist_WrongDataType,
					     NULL);
			return;
		}

		for (i = 0; i < bx; i++)
			minefield_set_at (mf, i, j, buffer->_buffer [(i << 1)],
					  buffer->_buffer [(i << 1) + 1]);

		CORBA_free (buffer);
	}

	control_update (control_data);
}

static void
save_board (BonoboPersistStream        *ps,
	    const Bonobo_Stream         stream,
	    Bonobo_Persist_ContentType  type,
	    void                       *closure,
	    CORBA_Environment          *ev)
{
	control_data_t       *control_data = closure;
	MineField            *mf;
	char                 *data;
	int                   j;

	g_return_if_fail (control_data != NULL);
	g_return_if_fail (control_data->data != NULL);

	if (*type && g_strcasecmp (type, "application/x-mines") != 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	mf = control_data->data;

	bonobo_stream_client_printf (stream, TRUE, ev, "%2u%2u\n",
				     mf->xsize, mf->ysize);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return;

	data = g_malloc (mf->xsize * 2 + 1);
	for (j = 0; j < mf->ysize; j++) {
		int i;

		for (i = 0; i < mf->xsize; i++)
			minefield_get_at (mf, i, j, &data [(i << 1)],
					  &data [(i << 1) + 1]);
		data [(i << 1)] = '\n';

		bonobo_stream_client_write (stream, data, mf->xsize * 2 + 1, ev);

		if (ev->_major != CORBA_NO_EXCEPTION)
			return;
	}
	g_free (data);
}

static Bonobo_Persist_ContentTypeList *
content_types (BonoboPersistStream *ps, void *closure, CORBA_Environment *ev)
{
	return bonobo_persist_generate_content_types (1, "application/x-mines");
}

static void
verb_NewGame_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	control_data_t *control_data = (control_data_t *) user_data;
	GdkPixbuf *pixbuf;

	/* Start a new game: Happy face. */
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) face_smile_xpm);
	bonobo_ui_util_set_pixbuf (uic, "/commands/NewGame", pixbuf);
	gdk_pixbuf_unref (pixbuf);

	minefield_restart (control_data->data);
	control_update (control_data);
}

/*
 * When one of our controls is activated, we merge our menus
 * in with our container's menus.
 */
static void
control_create_menus (control_data_t *control_data)
{
	BonoboControl *control = control_data->bonobo_object;
	Bonobo_UIContainer remote_uic;
	GdkPixbuf *pixbuf;

	static char ui [] = 
		"<Root>"
		"	<commands>"
		"		<cmd name=\"NewGame\" _label=\"New game\" _tip=\"Start a new game\"/>"
		"		<cmd name=\"OpenGame\" _label=\"Open game\" _tip=\"Load a saved game\"/>"
		"	</commands>"
		"	<menu>"
		"		<submenu name=\"Game\" _label=\"_Game\">"
		"			<menuitem name=\"NewGame\" verb=\"\"/>"
		"			<menuitem name=\"OpenGame\" verb=\"\"/>"
		"		</submenu>"
		"	</menu>"
		"	<dockitem name=\"Game\">"
		"		<toolitem name=\"NewGame\" verb=\"\"/>"
		"	</dockitem>"
		"</Root>";

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
	bonobo_ui_component_set_container (control_data->uic, remote_uic);

	/*
	 * Unref the UI container we have been passed.
	 */
	bonobo_object_release_unref (remote_uic, NULL);

	/* Set up the UI from the XML string. */
	{
		BonoboUINode *node;

		node = bonobo_ui_node_from_string (ui);
		bonobo_ui_util_translate_ui (node);
		bonobo_ui_util_fixup_help (control_data->uic, node,
					   DATADIR, "gnomines");
					   
		bonobo_ui_component_set_tree (control_data->uic, "/", node, NULL);

		bonobo_ui_node_free (node);
	}

	/* Start a new game: Happy face. */
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) face_smile_xpm);
	bonobo_ui_util_set_pixbuf (control_data->uic, "/commands/NewGame", pixbuf);
	gdk_pixbuf_unref (pixbuf);
}

static void
control_remove_menus (control_data_t *control_data)
{
	bonobo_ui_component_unset_container (control_data->uic);
}

/*
 * Clean up our supplementary BonoboControl data sturctures.
 */
static void
control_destroy_cb (BonoboControl *control, gpointer data)
{
	control_data_t *control_data = (control_data_t *) data;

	g_message ("control_destroy_cb");

	if (control_data->data)
		minefield_destroy (control_data->data);
	control_data->data = NULL;
	
	g_free (control_data); 
}

static void
control_activate_cb (BonoboControl *control, gboolean activate, gpointer data)
{
	control_data_t *control_data = (control_data_t *) data;

	/*
	 * The ControlFrame has just asked the Control (that's us) to be
	 * activated or deactivated.  We must reply to the ControlFrame
	 * and say whether or not we want our activation state to
	 * change.  We are an acquiescent BonoboControl, so we just agree
	 * with whatever the ControlFrame told us.  Most components
	 * should behave this way.
	 */
	bonobo_control_activate_notify (control, activate);

	/*
	 * If we were just activated, we merge in our menu entries.
	 * If we were just deactivated, we remove them.
	 */
	if (activate)
		control_create_menus (control_data);
	else
		control_remove_menus (control_data);
}

static void
control_set_frame_cb (BonoboControl *control, gpointer data)
{
	control_create_menus ((control_data_t *) data);
}

static void
update_control (GtkWidget *widget, control_data_t *control_data)
{
	control_update (control_data);
}

static void
game_over_cb (GtkWidget *widget, control_data_t *control_data)
{
	BonoboControl *control = control_data->bonobo_object;
	BonoboUIComponent *uic;
	GdkPixbuf *pixbuf;

	uic = bonobo_control_get_ui_component (control);

	/* Dead: Sad face in toolbar */
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) face_sad_xpm);
	bonobo_ui_util_set_pixbuf (uic, "/commands/NewGame", pixbuf);
	gdk_pixbuf_unref (pixbuf);
}

static void
win_cb (GtkWidget *widget, control_data_t *control_data)
{
	BonoboControl *control = control_data->bonobo_object;
	BonoboUIComponent *uic;
	GdkPixbuf *pixbuf;

	uic = bonobo_control_get_ui_component (control);

	/* Win: Happy face */
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) face_win_xpm);
	bonobo_ui_util_set_pixbuf (uic, "/commands/NewGame", pixbuf);
	gdk_pixbuf_unref (pixbuf);
}

static void
look_cb (GtkWidget *widget, control_data_t *control_data)
{
	BonoboControl *control = control_data->bonobo_object;
	BonoboUIComponent *uic;
	GdkPixbuf *pixbuf;

	uic = bonobo_control_get_ui_component (control);

	/* Look: Puzzled face */
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) face_worried_xpm);
	bonobo_ui_util_set_pixbuf (uic, "/commands/NewGame", pixbuf);
	gdk_pixbuf_unref (pixbuf);
}

static void
unlook_cb (GtkWidget *widget, control_data_t *control_data)
{
	BonoboControl *control = control_data->bonobo_object;
	BonoboUIComponent *uic;
	GdkPixbuf *pixbuf;

	uic = bonobo_control_get_ui_component (control);

	/* Unlooking means the game has started: Cool face */
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) face_cool_xpm);
	bonobo_ui_util_set_pixbuf (uic, "/commands/NewGame", pixbuf);
	gdk_pixbuf_unref (pixbuf);
}

static BonoboObject *
bonobo_mines_factory (BonoboGenericFactory *this, void *data)
{
	BonoboControl        *bonobo_object;
	control_data_t       *control_data;
	BonoboPersistStream  *stream;
	GtkWidget            *vbox;

	/*
	 * Create a data structure in which we can store
	 * Control-object-specific data about this document.
	 */
	control_data = g_new0 (control_data_t, 1);
	if (control_data == NULL)
		return NULL;

	control_data->data = minefield_new ();
	minefield_set_size  (control_data->data, 16, 16);
	minefield_set_mines (control_data->data, 40);
	minefield_restart   (control_data->data);

	vbox = gtk_vbox_new (TRUE, 0);
	control_data->mfield = (GtkMineFieldView *)gtk_minefield_new_view (control_data->data);

	/* Second set of signals is to control the face in the toolbar */
	gtk_signal_connect(GTK_OBJECT (control_data->mfield), "marks_changed",
			   GTK_SIGNAL_FUNC (update_control), control_data);

	gtk_signal_connect(GTK_OBJECT (control_data->mfield), "explode",
			   GTK_SIGNAL_FUNC (update_control), control_data);
	gtk_signal_connect(GTK_OBJECT (control_data->mfield), "explode",
			   GTK_SIGNAL_FUNC (game_over_cb), control_data);

	gtk_signal_connect(GTK_OBJECT (control_data->mfield), "win",
			   GTK_SIGNAL_FUNC (update_control), control_data);
	gtk_signal_connect(GTK_OBJECT (control_data->mfield), "win",
			   GTK_SIGNAL_FUNC (win_cb), control_data);

	gtk_signal_connect(GTK_OBJECT (control_data->mfield), "look",
			   GTK_SIGNAL_FUNC (update_control), control_data);
	gtk_signal_connect(GTK_OBJECT (control_data->mfield), "look",
			   GTK_SIGNAL_FUNC (look_cb), control_data);

	gtk_signal_connect(GTK_OBJECT (control_data->mfield), "unlook",
			   GTK_SIGNAL_FUNC (update_control), control_data);
	gtk_signal_connect(GTK_OBJECT (control_data->mfield), "unlook",
			   GTK_SIGNAL_FUNC (unlook_cb), control_data);

	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (control_data->mfield),
			    TRUE, TRUE, 0);
	gtk_widget_show_all (vbox);

	/*
	 * Create the BonoboControl object.
	 */
	bonobo_object = bonobo_control_new (vbox);

	if (bonobo_object == NULL) {
		gtk_widget_destroy (vbox);
		minefield_destroy (control_data->data);
		g_free (control_data);
		return NULL;
	}

	control_data->bonobo_object = bonobo_object;

	control_data->uic = bonobo_control_get_ui_component (bonobo_object);

	/*
	 * When our container wants to activate this component, we will get
	 * the "activate" signal.
	 */
	gtk_signal_connect (GTK_OBJECT (bonobo_object), "activate",
			    GTK_SIGNAL_FUNC (control_activate_cb), control_data);
	gtk_signal_connect (GTK_OBJECT (bonobo_object), "set_frame",
			    GTK_SIGNAL_FUNC (control_set_frame_cb), control_data);

	/*
	 * The "system_exception" signal is raised when the BonoboControl
	 * encounters a fatal CORBA exception.
	 */
	gtk_signal_connect (GTK_OBJECT (bonobo_object), "system_exception",
			    GTK_SIGNAL_FUNC (control_system_exception_cb), control_data);

	/*
	 * We'll need to be able to cleanup when this control gets
	 * destroyed.
	 */
	gtk_signal_connect (GTK_OBJECT (bonobo_object), "destroy",
			    GTK_SIGNAL_FUNC (control_destroy_cb), control_data);

	/*
	 * Create the PersistStream object.
	 */
	stream = bonobo_persist_stream_new (load_board, save_board,
					    NULL, content_types,
					    control_data);

	if (stream == NULL) {
		bonobo_object_unref (BONOBO_OBJECT (bonobo_object));
		gtk_widget_destroy (vbox);
		minefield_destroy (control_data->data);
		g_free (control_data);
		return NULL;
	}
	bonobo_object_add_interface (BONOBO_OBJECT (bonobo_object),
				     BONOBO_OBJECT (stream));

	/*
	 * Add some verbs to the control.
	 *
	 * The container application will then have the programmatic
	 * ability to execute the verbs on the component.  It will
	 * also provide a simple mechanism whereby the user can
	 * right-click on the component to create a popup menu
	 * listing the available verbs.
	 *
	 * We provide one simple verb whose job it is to clear the
	 * window.
	 */
	control_data->uic = bonobo_control_get_ui_component (bonobo_object);

	bonobo_ui_component_add_verb (control_data->uic, "NewGame",
				      verb_NewGame_cb, control_data);

	return BONOBO_OBJECT (bonobo_object);
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Sample_Mines_Factory",
		    "application-x-mines", VERSION,
		    bonobo_mines_factory,
		    NULL)
