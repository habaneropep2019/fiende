/*
 * This program is a bonobo container whith an emacs like user interface
 * (based on sample-container from  Nat Friedman (nat@nat.org))
 *
 * Author:
 *   Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright 1999 Maurer IT Systemlösungen (http://www.maurer-it.com)
 */

#include <config.h>
#include "gshell.h"
#include "inout.h"
#include "ui.h"
#include "properties.h"
#include "controls.h"
#include <liboaf/liboaf.h>

Application app;

static void
verb_HelpGNOME_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	gchar *p;

	p = gnome_help_file_path ("help-browser", "default-page.html");
	if (p)
		gnome_help_goto (NULL, p);
}

static void
verb_HelpAbout_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	static GtkWidget *about;

	const gchar *authors[] = {
		"Dietmar Maurer",
		NULL
	};

	about = gnome_about_new 
		( "Gnome Shell", VERSION,
		  "Copyright (C) 1999 Maurer IT Systemlösungen"
		  "                     ",
		  authors,
		  _("This program is part of the GNOME project. "
		    "Gnome Shell comes with ABSOLUTELY NO "
		    "WARRANTY. This is free software, and you are "
		    "welcome to redistribute it under the conditions "
		    "of the GNU General Public License. "
		    "Please report bugs to dietmar@maurer-it.com"),
		  NULL);
	
	gtk_widget_show (about);
}

static void
verb_SettingsPreferences_cb (BonoboUIComponent *component,
			     Frame *frame, const char *cname)
{
	show_property_dialog (
		BONOBO_CONTROL_FRAME (frame->active_view_frame));
}

Frame *
get_active_frame (GtkWidget *widget)
{
	GtkWidget *top;
	GList *l;
	Frame *frame;

	g_return_val_if_fail (widget != NULL, NULL);
	g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

	top = gtk_widget_get_toplevel(widget);
	for (l = app.frame_list;l;l = l->next) {
		frame = (Frame *)l->data;
		if (GTK_WIDGET (frame->win) == top)
			return frame;
	}
	return NULL;
}

static void
container_system_exception_cb (BonoboObject *container_object, 
			       CORBA_Object cobject,
			       CORBA_Environment *ev, gpointer data)
{
	gnome_warning_dialog (_("fatal CORBA exception!  Shutting down..."));

	bonobo_object_unref (BONOBO_OBJECT (app.container));

	gtk_main_quit ();
}

static void
client_site_system_exception_cb (BonoboObject *client, CORBA_Object cobject,
				 CORBA_Environment *ev, gpointer data)
{
	/* Buffer *buffer = (Buffer *) data; */
	g_warning ("component shutdown not implemented");
	/* bonobo_object_unref (BONOBO_OBJECT (buffer->client_site)); */
}

static void
view_system_exception_cb (BonoboViewFrame *view_frame, CORBA_Object cobject,
			  CORBA_Environment *ev, gpointer data)
{
	g_warning ("view frame system exception not implemented");
	bonobo_object_unref (BONOBO_OBJECT (view_frame));
}

static void
user_activate_request_cb (BonoboViewFrame *view_frame, Frame *frame)
{

	if (frame->active_view_frame == view_frame)
		return;

        if (frame->active_view_frame != NULL) {
                bonobo_view_frame_view_deactivate (frame->active_view_frame);
		if (frame->active_view_frame != NULL)
			bonobo_view_frame_set_covered 
				(frame->active_view_frame, TRUE); 
		frame->active_view_frame = NULL;
	}

        bonobo_view_frame_view_activate (view_frame);
}

static void
view_activated_cb (BonoboViewFrame *view_frame, gboolean activated, 
		   Frame *frame)
{
	GtkWidget *w;

        if (activated) {
		if (frame->active_view_frame != NULL) {
			g_warning ("View requested to be activated "
				   "but there is already "
				   "an active View!\n");
			return;
		}
		bonobo_view_frame_set_covered (view_frame, FALSE);
                frame->active_view_frame = view_frame;
		w = bonobo_view_frame_get_wrapper (view_frame);
		/* gtk_container_focus (GTK_CONTAINER(w), GTK_DIR_UP); */
		gtk_widget_grab_focus(w);
		frame_set_sensitive (frame, TRUE);
        } else {
		bonobo_view_frame_set_covered (view_frame, TRUE);
		if (view_frame == frame->active_view_frame)
			frame->active_view_frame = NULL;
        }                                                   
}

static void
zoom_level_changed_cb (GtkWidget *widget, float zoom_level, Frame *frame)
{
	gchar *message;

	message = g_strdup_printf (_("New zoom level is %.3g."), zoom_level);
	bonobo_ui_component_set_status (frame->component, message, NULL);
	g_free (message);
}

static void
buffer_create_zoomable_frame (Buffer *buffer, Frame *frame, BonoboViewFrame *view_frame)
{
	Bonobo_Control control;
	Bonobo_Zoomable zoomable;

	control = bonobo_control_frame_get_control (BONOBO_CONTROL_FRAME (view_frame));
	zoomable = Bonobo_Unknown_queryInterface (control, "IDL:Bonobo/Zoomable:1.0", NULL);
	if (zoomable != CORBA_OBJECT_NIL) {
		g_message ("Zooming interface found");
		buffer->zoomable_frame = bonobo_zoomable_frame_new ();
		bonobo_zoomable_frame_bind_to_zoomable (buffer->zoomable_frame, zoomable);

		gtk_signal_connect (GTK_OBJECT (buffer->zoomable_frame), "zoom_level_changed",
				    GTK_SIGNAL_FUNC (zoom_level_changed_cb), frame);

		frame_set_zoomable (frame, TRUE);
	} else
		frame_set_zoomable (frame, FALSE);
}

void
buffer_add_view (Buffer *buffer, Frame *frame, gint pos)
{
	BonoboViewFrame *view_frame;
	GtkWidget *view_widget;
	GtkWidget *box,*l;

	view_frame = bonobo_client_site_new_view_full (
		buffer->client_site,
		bonobo_object_corba_objref (BONOBO_OBJECT (frame->container)),
		FALSE, TRUE);

	if (!view_frame) {
		g_warning ("create view failed");
		return;
	}

	gtk_object_set_data (GTK_OBJECT(view_frame), "gshell:buffer", buffer);
	frame->view_list = g_list_append (frame->view_list, view_frame);

	gtk_signal_connect (GTK_OBJECT (view_frame), "system_exception",
			    view_system_exception_cb, buffer);
	gtk_signal_connect (GTK_OBJECT (view_frame), "user_activate",
			    GTK_SIGNAL_FUNC (user_activate_request_cb), frame);
	gtk_signal_connect (GTK_OBJECT (view_frame), "activated",
			    GTK_SIGNAL_FUNC (view_activated_cb), frame);

	view_widget = bonobo_view_frame_get_wrapper (view_frame);
	
	box = gtk_vbox_new (FALSE,0);
	gtk_object_set_data (GTK_OBJECT(box), "gshell:view_frame", view_frame);

	l = gtk_label_new (buffer->name);
	gtk_box_pack_start (GTK_BOX (box), l, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), view_widget, TRUE, TRUE, 0);

	gtk_box_pack_end (GTK_BOX (frame->vbox), box, TRUE, TRUE, 0);

	if (pos>=0) gtk_box_reorder_child (GTK_BOX (frame->vbox), box, pos);

	gtk_widget_show_all (box);

	user_activate_request_cb (view_frame, frame);

	buffer_create_zoomable_frame (buffer, frame, view_frame);
}

static Buffer *
real_buffer_create (BonoboObjectClient *server)
{
	Buffer *buffer;
	BonoboClientSite *client_site;

	client_site = bonobo_client_site_new (app.container);

	if (!bonobo_client_site_bind_embeddable (client_site, server)) {
		bonobo_object_unref (BONOBO_OBJECT (server));
		bonobo_object_unref (BONOBO_OBJECT (client_site));
		g_warning("Bind component failed");
		return NULL;
	}

	bonobo_object_client_unref (server, NULL);
	
	buffer = g_new0 (Buffer, 1);
	buffer->client_site = client_site;
	buffer->server = server;
	app.buffer_list = g_list_append (app.buffer_list, buffer);

	gtk_signal_connect (GTK_OBJECT (client_site), "system_exception",
			    client_site_system_exception_cb, buffer);

	return buffer;
}

static Buffer *
real_buffer_create_embeddable (const char *component_oaf_iid)
{
	BonoboObjectClient *server;

	server = bonobo_object_activate (component_oaf_iid, 0);

	if (!server) {
		g_warning ("Launching component failed");
		return NULL;
	}

	return real_buffer_create (server);
}

static Buffer *
real_buffer_create_control (const char *component_oaf_iid)
{
	BonoboObjectClient *server;

	server = gshell_control_wrapper (component_oaf_iid);
	if (!server) {
		g_warning ("Launching component failed");
		return NULL;
	}

	return real_buffer_create (server);
}

Buffer *
buffer_create (const char *component_oaf_iid)
{
	Buffer *retval;
        OAF_ServerInfoList *servers;
	CORBA_Environment ev;
	gchar *query;

        CORBA_exception_init (&ev);
	query = g_strdup_printf ("(iid == '%s') AND repo_ids.has('IDL:Bonobo/Embeddable:1.0')",
				 component_oaf_iid);
        servers = oaf_query (query, NULL, &ev);
        g_free (query);
        CORBA_exception_free (&ev);

	if ((servers != NULL) && (servers->_length > 0))
		retval = real_buffer_create_embeddable (component_oaf_iid);
	else
		retval = real_buffer_create_control (component_oaf_iid);

	CORBA_free (servers);

	return retval;
}

Buffer *
buffer_create_for_control (const char *component_oaf_iid)
{
	return real_buffer_create_control (component_oaf_iid);
}

static GtkWidget *
find_view_frame_pos (Frame *frame, BonoboViewFrame *view_frame, gint *pos)
{
	GList *l;
	gint i = 0;
	GtkObject *o = NULL;

	l = GTK_BOX(frame->vbox)->children;
	
	while (l && (o = GTK_OBJECT (((GtkBoxChild *)l->data)->widget)) && 
	       (gtk_object_get_data (o, "gshell:view_frame") != view_frame)) {
		i++; l = l->next; 
	}

	if (pos) *pos = i;

	if (o)
		return GTK_WIDGET (o);

	return NULL;
}

gint
view_remove (Frame *frame, BonoboViewFrame *view_frame)
{
	GtkWidget *w,*b;
	gint pos;

	w = bonobo_view_frame_get_wrapper (view_frame);
	frame->view_list = g_list_remove (frame->view_list, view_frame);
	if (frame->active_view_frame == view_frame) 
		frame->active_view_frame = NULL;

	b = find_view_frame_pos (frame, view_frame, &pos);
	
	gtk_widget_destroy (w);
	bonobo_object_unref (BONOBO_OBJECT (view_frame));

	if (b)
		gtk_widget_destroy (b);

	return pos;
}

static void
frame_close_cb (GtkWidget *widget, Frame *frame)
{
	BonoboViewFrame *view_frame;
	GList *v;

	while ((v = frame->view_list)) {
		view_frame = (BonoboViewFrame *)v->data;
		view_remove (frame, view_frame);
	}

	app.frame_list = g_list_remove (app.frame_list, frame);
	gtk_widget_destroy (GTK_WIDGET (frame->win));
	if (!g_list_length(app.frame_list))
		gtk_main_quit ();
}

static void
verb_FileExit_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	bonobo_object_unref (BONOBO_OBJECT (app.container));
	gtk_main_quit ();
}

static void
verb_WindowClose_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	frame_close_cb (NULL, frame);
}

static Frame *
frame_create ()
{
	Frame *frame;
	GtkWidget *statusbar;

	frame = g_new0 (Frame, 1);
	frame->win = BONOBO_WINDOW (bonobo_window_new ("gshell", "Gnome Shell"));

	gtk_window_set_default_size (GTK_WINDOW (frame->win), 640, 400);
	gtk_window_set_policy (GTK_WINDOW (frame->win), TRUE, TRUE, FALSE);

	gtk_signal_connect (GTK_OBJECT (frame->win), "delete_event",
			    GTK_SIGNAL_FUNC (frame_close_cb), frame);

	statusbar = gtk_statusbar_new ();

	frame->vbox = gtk_vbox_new (FALSE, 0);
	bonobo_window_set_contents (frame->win, frame->vbox);

	frame->container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (frame->container, frame->win);

	frame_create_ui (frame);
	frame_set_sensitive (frame, FALSE);
	frame_set_zoomable (frame, FALSE);

	gtk_widget_show_all (GTK_WIDGET (frame->win));

	app.frame_list = g_list_append (app.frame_list, frame);

	return frame;
}

BonoboViewFrame *
get_active_view_frame (Frame *frame)
{
	GtkWidget *w = NULL;
	BonoboViewFrame *view_frame = NULL;

	if (!(w = GTK_CONTAINER(frame->vbox)->focus_child)) {
		if (g_list_length(frame->view_list)) {
			view_frame = BONOBO_VIEW_FRAME(frame->view_list->data);
			return view_frame;
		}

	}

       	if (w)
		return gtk_object_get_data (GTK_OBJECT (w), 
					    "gshell:view_frame");

	return NULL;
}

static void
verb_WindowCreateNew_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	BonoboViewFrame *view_frame;
	Buffer *buffer = NULL;

	if ((view_frame = get_active_view_frame (frame))) {
		buffer = gtk_object_get_data(GTK_OBJECT(view_frame), 
					     "gshell:buffer");
	}

	frame = frame_create ();
	if (buffer)
		buffer_add_view (buffer, frame, -1);
	update_buffer_menu ();
}

static void
verb_WindowSplit_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	BonoboViewFrame *view_frame;
	Buffer *buffer = NULL;
	gint pos;

	if ((view_frame = get_active_view_frame (frame))) {
		find_view_frame_pos (frame, view_frame, &pos);
		buffer = gtk_object_get_data(GTK_OBJECT(view_frame), 
					     "gshell:buffer");
		if (buffer)
			buffer_add_view (buffer, frame, pos);
	}
}

static void
verb_WindowOne_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	BonoboViewFrame *view_frame, *vf;
	GList *v;

	if ((view_frame = get_active_view_frame (frame))) {
		for (v = frame->view_list; v;) {
			vf = (BonoboViewFrame *) v->data;
			v = v->next;
			if (view_frame != vf)
				view_remove (frame, vf);
		}
	}
}

static void
verb_FileKill_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	Buffer *buffer=NULL;
	BonoboViewFrame *view_frame;
	GList *f,*v,*b;
	gchar *path;

	if ((view_frame = get_active_view_frame (frame))) {
		buffer = gtk_object_get_data(GTK_OBJECT(view_frame), 
					     "gshell:buffer");
	} else
		return;	  
	
	if (!buffer)
		return;	  

	app.buffer_list = g_list_remove (app.buffer_list, buffer);

	for (f = app.frame_list; f; f = f->next) {
		frame = (Frame *) f->data;

		for (v = frame->view_list; v;) {

			view_frame = (BonoboViewFrame *) v->data;
			v = v->next;
			if (buffer == gtk_object_get_data (
				GTK_OBJECT(view_frame), "gshell:buffer")) {
				view_remove (frame, view_frame);

				if (!g_list_length (GTK_BOX (frame->vbox)->children)) {

					if ((b = g_list_last (app.buffer_list)))
						buffer_add_view ((Buffer *) b->data,
								 frame, -1);
					else
						frame_set_sensitive (frame, FALSE);
				}
			}
		}
	}

	bonobo_object_unref (BONOBO_OBJECT (buffer->client_site));

	path = g_strdup_printf ("/commands/%s", buffer->verb);
	bonobo_ui_component_rm (frame->component, path, NULL);
	g_free (path);

	update_buffer_menu ();

	if (buffer->name) {
		g_free (buffer->name);
		buffer->name = NULL;
	}
	g_free (buffer);
}

void
open_files (Frame *frame, int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++)
		file_open (frame, argv [i]);
}

static void
zoom_custom_cb (gchar *string, gpointer user_data)
{
	BonoboViewFrame *view_frame;
	Frame *frame = user_data;
	gchar *end = NULL;
	double zoom_level;

	if (string == NULL)
		return;

	zoom_level = g_strtod (string, &end);
	if (end && *end != '\x00') {
		g_warning ("Invalid zoom level `%s'", string);
		return;
	}

	view_frame = get_active_view_frame (frame);
	if (!view_frame)
		return;

	bonobo_view_frame_set_zoom_factor (view_frame, zoom_level);
}

static void
zoom_custom (Frame *frame)
{
	GtkWidget *dialog;

	dialog = gnome_request_dialog (FALSE, _("Set zoom level:"), NULL, 10,
				       zoom_custom_cb, frame, GTK_WINDOW (frame->win));

	gnome_dialog_run (GNOME_DIALOG (dialog));
}

static void
verb_Zoom_cb (BonoboUIComponent *component, Frame *frame, const char *cname)
{
	BonoboViewFrame *view_frame;
	Buffer *buffer = NULL;

	view_frame = get_active_view_frame (frame);
	if (!view_frame)
		return;

	buffer = gtk_object_get_data (GTK_OBJECT (view_frame), "gshell:buffer");
	if (!buffer)
		return;

	if (!buffer->zoomable_frame)
		return;

	if (!strcmp (cname, "ZoomIn"))
		bonobo_zoomable_frame_zoom_in (buffer->zoomable_frame);
	else if (!strcmp (cname, "ZoomOut"))
		bonobo_zoomable_frame_zoom_out (buffer->zoomable_frame);
	else if (!strcmp (cname, "ZoomToFit"))
		bonobo_zoomable_frame_zoom_to_fit (buffer->zoomable_frame);
	else if (!strcmp (cname, "ZoomToDefault"))
		bonobo_zoomable_frame_zoom_to_default (buffer->zoomable_frame);
	else
		zoom_custom (frame);
}

int
main (int argc, char **argv)
{
	CORBA_Environment ev;
	CORBA_ORB orb;
	Frame *frame;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	CORBA_exception_init (&ev);

	gnome_init_with_popt_table ("gshell", "1.0",
				    argc, argv,
				    oaf_popt_options, 0, NULL);

	CORBA_exception_free (&ev);

	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error (_("Could not initialize Bonobo!"));

	app.container = bonobo_item_container_new ();
	gtk_signal_connect (GTK_OBJECT (app.container), "system_exception",
			    container_system_exception_cb, NULL);

	frame = frame_create ();

	bonobo_activate ();

	open_files (frame, argc, argv);

	bonobo_main ();

	return 0;
}

BonoboUIVerb gshell_verbs[] = {
	BONOBO_UI_UNSAFE_VERB ("FileKill", verb_FileKill_cb),
	BONOBO_UI_UNSAFE_VERB ("FileExit", verb_FileExit_cb),
	BONOBO_UI_UNSAFE_VERB ("SettingsPreferences", verb_SettingsPreferences_cb),
	BONOBO_UI_UNSAFE_VERB ("WindowCreateNew", verb_WindowCreateNew_cb),
	BONOBO_UI_UNSAFE_VERB ("WindowClose", verb_WindowClose_cb),
	BONOBO_UI_UNSAFE_VERB ("WindowSplit", verb_WindowSplit_cb),
	BONOBO_UI_UNSAFE_VERB ("WindowOne", verb_WindowOne_cb),
	BONOBO_UI_UNSAFE_VERB ("ZoomIn", verb_Zoom_cb),
	BONOBO_UI_UNSAFE_VERB ("ZoomOut", verb_Zoom_cb),
	BONOBO_UI_UNSAFE_VERB ("ZoomToFit", verb_Zoom_cb),
	BONOBO_UI_UNSAFE_VERB ("ZoomToDefault", verb_Zoom_cb),
	BONOBO_UI_UNSAFE_VERB ("WindowZoom", verb_Zoom_cb),
	BONOBO_UI_UNSAFE_VERB ("HelpGNOME", verb_HelpGNOME_cb),
	BONOBO_UI_UNSAFE_VERB ("HelpAbout", verb_HelpAbout_cb),
	BONOBO_UI_VERB_END
};

