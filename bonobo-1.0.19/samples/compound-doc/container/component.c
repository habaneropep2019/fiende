/* $Id: component.c,v 1.24 2001/10/24 03:52:19 jody Exp $ */

#include "component.h"
#include "embeddable-io.h"
#include "container-filesel.h"

BonoboClientSiteClass *sample_client_site_parent_class = NULL;

void
object_print (BonoboObjectClient *object,
	      GnomePrintContext  *ctx,
	      gdouble x, gdouble y,
	      gdouble width, gdouble height)
{
	BonoboPrintClient *print_client = bonobo_print_client_get (object);
	BonoboPrintData   *print_data;

	if (!print_client)
		return;

	print_data = bonobo_print_data_new (width, height);
	bonobo_print_client_render (print_client, print_data);
	bonobo_print_data_render (ctx, x, y, print_data, 0.0, 0.0);
	bonobo_print_data_free (print_data);

	gtk_object_unref (GTK_OBJECT (print_client));
}

static void
activate_request_cb (BonoboViewFrame  *view_frame,
		     SampleClientSite *site)
{
	SampleApp *app;

	g_return_if_fail (site != NULL);
	g_return_if_fail (site->app != NULL);

	app = site->app;

	if (app->curr_view) {
		bonobo_view_frame_view_deactivate (app->curr_view);
		if (app->curr_view)
			bonobo_view_frame_set_covered (app->curr_view, FALSE);
	}

	/*
	 * Activate the View which the user clicked on.  This just
	 * sends a request to the embedded View to activate itself.
	 * When it agrees to be activated, it will notify its
	 * ViewFrame, and our view_activated_cb callback will be
	 * called.
	 *
	 * We do not uncover the View here, because it may not wish to
	 * be activated, and so we wait until it notifies us that it
	 * has been activated to uncover it.
	 */
	bonobo_view_frame_view_activate (view_frame);
}

static void
view_activated_cb (BonoboViewFrame  *view_frame,
		   gboolean          activated,
		   SampleClientSite *site)
{
	SampleApp *app = site->app;

	if (activated) {
		/*
		 * If the View is requesting to be activated, then we
		 * check whether or not there is already an active
		 * View.
		 */
		if (app->curr_view) {
			g_warning ("View requested to be activated but "
				   "there is already an active View!\n");
			return;
		}

		/*
		 * Otherwise, uncover it so that it can receive
		 * events, and set it as the active View.
		 */
		bonobo_view_frame_set_covered (view_frame, FALSE);
		app->curr_view = view_frame;
	} else {
		/*
		 * If the View is asking to be deactivated, always
		 * oblige.  We may have already deactivated it (see
		 * user_activation_request_cb), but there's no harm in
		 * doing it again.  There is always the possibility
		 * that a View will ask to be deactivated when we have
		 * not told it to deactivate itself, and that is
		 * why we cover the view here.
		 */
		bonobo_view_frame_set_covered (view_frame, TRUE);

		if (view_frame == app->curr_view)
			app->curr_view = NULL;
	}
}

static void
component_user_context_cb (BonoboViewFrame  *view_frame,
			   SampleClientSite *site)
{
	g_warning ("We need something exciting here ...");
}

void
sample_client_site_add_frame (SampleClientSite *site)
{
	BonoboViewFrame *view_frame;
	GtkWidget       *view_widget;

	/*
	 * Create the remote view and the local ViewFrame.  This also
	 * sets the BonoboUIHandler for this ViewFrame.  That way, the
	 * embedded component can get access to our UIHandler server
	 * so that it can merge menu and toolbar items when it gets
	 * activated.
	 */
	view_frame = bonobo_client_site_new_view (BONOBO_CLIENT_SITE (site),
		BONOBO_OBJREF (site->app->ui_container));
	
	/*
	 * Embed the view frame into the application.
	 */
	view_widget = bonobo_view_frame_get_wrapper (view_frame);
	gtk_box_pack_start (GTK_BOX (site->views_hbox), view_widget,
			    FALSE, FALSE, 5);

	/*
	 * The "user_activate" signal will be emitted when the user
	 * double clicks on the "cover".  The cover is a transparent
	 * window which sits on top of the component and keeps any
	 * events (mouse, keyboard) from reaching it.  When the user
	 * double clicks on the cover, the container (that's us)
	 * can choose to activate the component.
	 */
	gtk_signal_connect (GTK_OBJECT (view_frame), "user_activate",
			    GTK_SIGNAL_FUNC (activate_request_cb),
			    site);

	/*
	 * In-place activation of a component is a two-step process.
	 * After the user double clicks on the component, our signal
	 * callback (component_user_activate_request_cb()) asks the
	 * component to activate itself (see
	 * bonobo_view_frame_view_activate()).  The component can then
	 * choose to either accept or refuse activation.  When an
	 * embedded component notifies us of its decision to change
	 * its activation state, the "activated" signal is
	 * emitted from the view frame.  It is at that point that we
	 * actually remove the cover so that events can get through.
	 */
	gtk_signal_connect (GTK_OBJECT (view_frame), "activated",
			    GTK_SIGNAL_FUNC (view_activated_cb),
			    site);

	/*
	 * The "user_context" signal is emitted when the user right
	 * clicks on the wrapper.  We use it to pop up a verb menu.
	 */
	gtk_signal_connect (GTK_OBJECT (view_frame), "user_context",
			    GTK_SIGNAL_FUNC (component_user_context_cb),
			    site);

	/*
	 * Show the component.
	 */
	gtk_widget_show_all (view_widget);
}

static void
sample_site_del_frame (SampleClientSite *site)
{
	BonoboViewFrame *view;

	g_return_if_fail (site != NULL);
	g_return_if_fail (site->parent.view_frames != NULL);

	view = site->parent.view_frames->data;

	gtk_container_remove (GTK_CONTAINER (site->views_hbox),
			      bonobo_view_frame_get_wrapper (view));

	bonobo_object_unref (BONOBO_OBJECT (view));
}

static void
add_frame_cb (GtkWidget *caller, SampleClientSite *site)
{
	sample_client_site_add_frame (site);
}

static void
del_frame_cb (GtkWidget *caller, SampleClientSite *site)
{
	sample_site_del_frame (site);
}

static void
del_cb (GtkWidget *caller, SampleClientSite *site)
{
	bonobo_object_unref (BONOBO_OBJECT (site));
}

static void
load_stream_cb (GtkWidget *caller, SampleClientSite *site)
{
	GtkWidget *fs = site->app->fileselection;
	gchar *filename = g_strdup (gtk_file_selection_get_filename
				    (GTK_FILE_SELECTION (fs)));
	gtk_widget_destroy (fs);

	if (filename) {
		CORBA_Environment ev;
		Bonobo_PersistStream persist;
		BonoboStream *stream;

		stream = bonobo_stream_open ("fs", filename, 
					     Bonobo_Storage_READ, 0644);

		if (!stream) {
			gchar *error_msg;

			error_msg =
			    g_strdup_printf (_("Could not open file %s"),
					     filename);
			gnome_warning_dialog (error_msg);
			g_free (error_msg);

			return;
		}

		/*
		 * Now get the PersistStream interface off the embedded
		 * component.
		 */
		persist = bonobo_object_client_query_interface (
			bonobo_client_site_get_embeddable (BONOBO_CLIENT_SITE (site)),
			"IDL:Bonobo/PersistStream:1.0", NULL);

		/*
		 * If the component doesn't support PersistStream (and it
		 * really ought to -- we query it to see if it supports
		 * PersistStream before we even give the user the option of
		 * loading data into it with PersistStream), then we destroy
		 * the stream we created and bail.
		 */
		if (persist == CORBA_OBJECT_NIL) {
			gnome_warning_dialog (_
					      ("The component now claims that it "
					       "doesn't support PersistStream!"));
			bonobo_object_unref (BONOBO_OBJECT (stream));
			g_free (filename);
			return;
		}

		CORBA_exception_init (&ev);

		/*
		 * Load the file into the component using PersistStream.
		 */
		Bonobo_PersistStream_load (persist, BONOBO_OBJREF (stream),
					   "", &ev);

		if (ev._major != CORBA_NO_EXCEPTION) {
			char *msg = bonobo_exception_get_text (&ev);
			gnome_warning_dialog (msg);
			g_free (msg);
		}

		/*
		 * Now we destroy the PersistStream object.
		 */
		Bonobo_Unknown_unref (persist, &ev);
		CORBA_Object_release (persist, &ev);

		bonobo_object_unref (BONOBO_OBJECT (stream));

		CORBA_exception_free (&ev);
	}

	g_free (filename);
}

static void
fill_cb (GtkWidget *caller, SampleClientSite *site)
{
	container_request_file (site->app, FALSE,
				load_stream_cb, site);
}

static void
sample_client_site_destroy (GtkObject *object)
{
	SampleClientSite *site = SAMPLE_CLIENT_SITE (object);

	site->app->components = g_list_remove (
		site->app->components, site);

/* FIXME: unref the server as we pop it in */
/*	bonobo_object_unref (BONOBO_OBJECT (component->server));*/

	if (site->obj_id) {
		g_free (site->obj_id);
		site->obj_id = NULL;
	}
	gtk_widget_destroy (site->frame);
	GTK_OBJECT_CLASS (sample_client_site_parent_class)->destroy
		(GTK_OBJECT (site));
}

static void
sample_client_site_class_init (SampleClientSiteClass *klass)
{
	BonoboObjectClass *gobject_class = (BonoboObjectClass *) klass;
	GtkObjectClass *object_class = (GtkObjectClass *) gobject_class;
	
	sample_client_site_parent_class = gtk_type_class (
		bonobo_client_site_get_type ());

	object_class->destroy = sample_client_site_destroy;
}

static void 
sample_client_site_init (GtkObject *object)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC (SampleClientSite, 
		      bonobo_client_site_get_type (),
		      sample_client_site);

static void
site_create_widgets (SampleClientSite *site)
{
	GtkWidget *frame;
	GtkWidget *vbox, *hbox;
	GtkWidget *new_view_button, *del_view_button;
	GtkWidget *del_comp_button, *fill_comp_button;

	g_return_if_fail (site != NULL);

	/* Display widgets */
	frame = site->frame = gtk_frame_new (site->obj_id);
	vbox = gtk_vbox_new (FALSE, 10);
	hbox = gtk_hbox_new (TRUE, 5);
	new_view_button = gtk_button_new_with_label ("New view");
	del_view_button = gtk_button_new_with_label ("Remove view");
	del_comp_button = gtk_button_new_with_label ("Remove component");

	/* The views of the component */
	site->views_hbox = gtk_hbox_new (FALSE, 2);
	gtk_signal_connect (GTK_OBJECT (new_view_button), "clicked",
			    GTK_SIGNAL_FUNC (add_frame_cb), site);
	gtk_signal_connect (GTK_OBJECT (del_view_button), "clicked",
			    GTK_SIGNAL_FUNC (del_frame_cb), site);
	gtk_signal_connect (GTK_OBJECT (del_comp_button), "clicked",
 			    GTK_SIGNAL_FUNC (del_cb), site);

	gtk_container_add (GTK_CONTAINER (hbox), new_view_button);
	gtk_container_add (GTK_CONTAINER (hbox), del_view_button);
	gtk_container_add (GTK_CONTAINER (hbox), del_comp_button);

	if (bonobo_object_client_has_interface (
		bonobo_client_site_get_embeddable (BONOBO_CLIENT_SITE (site)),
		"IDL:Bonobo/PersistStream:1.0", NULL)) {

		fill_comp_button =
		    gtk_button_new_with_label ("Fill with stream");
		gtk_container_add (GTK_CONTAINER (hbox), fill_comp_button);

		gtk_signal_connect (GTK_OBJECT (fill_comp_button),
				    "clicked", GTK_SIGNAL_FUNC (fill_cb),
				    site);
	}


	gtk_container_add (GTK_CONTAINER (vbox),  site->views_hbox);
	gtk_container_add (GTK_CONTAINER (vbox),  hbox);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
}

GtkWidget *
sample_client_site_get_widget (SampleClientSite *site)
{
	g_return_val_if_fail (site != NULL, NULL);

	return site->frame;
}

SampleClientSite *
sample_client_site_new (BonoboItemContainer *container,
			SampleApp           *app,
			BonoboObjectClient  *embeddable,
			const char          *embeddable_id)
{
	SampleClientSite *site;

	g_return_val_if_fail (app != NULL, NULL);
	g_return_val_if_fail (embeddable_id != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_OBJECT_CLIENT (embeddable), NULL);
	g_return_val_if_fail (BONOBO_IS_ITEM_CONTAINER (container), NULL);
	
	site = gtk_type_new (sample_client_site_get_type ());
	
	site = SAMPLE_CLIENT_SITE (bonobo_client_site_construct (
		BONOBO_CLIENT_SITE (site), container));

	if (site) {
		bonobo_client_site_bind_embeddable (BONOBO_CLIENT_SITE (site),
						    embeddable);
		bonobo_object_unref (BONOBO_OBJECT (embeddable));

		site->app = app;
		g_free (site->obj_id);
		site->obj_id = g_strdup (embeddable_id);

		site_create_widgets (site);
	}

	return site;
}
