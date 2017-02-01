/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
 * application/postscript embeddable
 *
 * Copyright (C) 1999, 2000 Jaka Mocnik
 *
 * based on text/plain embeddable from Nat Friedman  <nat@gnome-support.com>
 *
 * Author:
 *   Jaka Mocnik  <jaka.mocnik@kiss.uni-lj.si>
 */

#include <config.h>
#include <gnome.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>

#include "gtkgs.h"
#include "prefs.h"

static GdkCursor *pan_cursor = NULL;
static BonoboGenericFactory *factory = NULL;
static CORBA_ORB orb;
static gint active_objects = 0;

/*
 * Embeddable data
 */
typedef struct {
	BonoboEmbeddable *embeddable;

	gchar *tmp_name;
	gboolean loaded;

	GList *views;
} embeddable_data_t;

/*
 * View data
 */
typedef struct {
	embeddable_data_t *embeddable_data;

	GtkWidget        *gs;
	GtkObject        *hadj, *vadj;
	BonoboView *view;

	gint magstep;
	gboolean pan;
	gdouble prev_x, prev_y;
} view_data_t;


static void
view_system_exception_cb(BonoboView *view, CORBA_Object corba_object,
						 CORBA_Environment *ev, gpointer data)
{
	bonobo_object_unref(BONOBO_OBJECT(view));
}

static void
set_page_item_sensitivity (view_data_t *view_data)
{
	BonoboUIComponent *uic = bonobo_view_get_ui_component(view_data->view);
	GtkGS *gs = GTK_GS(view_data->gs);

	bonobo_ui_component_set_prop(uic, "/commands/NextPage",
								 "sensitive",
								 (gs->doc && (gs->current_page < gs->doc->numpages - 1))?"1":"0",
								 NULL);
	bonobo_ui_component_set_prop(uic, "/commands/PrevPage",
								 "sensitive",
								 (gs->current_page > 0)?"1":"0",
								 NULL);
}

static void
view_merge_menus(view_data_t *view_data)
{
	Bonobo_UIContainer remote_uic;
	BonoboUIComponent *uic;

	const char *ps_menu_cmds =
		"<commands>\n"
		"  <cmd name=\"NextPage\" _label=\"_Next Page\"/>\n"
		"  <cmd name=\"PrevPage\" _label=\"_Previous Page\"/>\n"
		"</commands>\n";
	const char *ps_menus =
		"<menu>\n"
		"  <submenu name=\"Document\" _label=\"_Document\">\n"
		"    <menuitem name=\"NextPage\" verb=\"NextPage\"/>\n"
		"    <menuitem name=\"PrevPage\" verb=\"PrevPage\"/>\n"
		"  </submenu>\n"
		"</menu>\n";

	remote_uic = bonobo_view_get_remote_ui_container(view_data->view);
	uic = bonobo_view_get_ui_component(view_data->view);

	if(remote_uic == CORBA_OBJECT_NIL)
		return;

	bonobo_ui_component_set_container(uic, remote_uic);
	bonobo_ui_component_set_translate(uic, "/", ps_menu_cmds, NULL);
	bonobo_ui_component_set_translate(uic, "/", ps_menus, NULL);
	bonobo_ui_component_thaw (uic, NULL);

	set_page_item_sensitivity(view_data);
}

static void
view_remove_menus(view_data_t *view_data)
{
	BonoboUIComponent *uic = bonobo_view_get_ui_component(view_data->view);

	bonobo_ui_component_unset_container(uic);
}


static void
view_activate_cb(BonoboView *view, gboolean activate, view_data_t *view_data)
{
	bonobo_view_activate_notify(view, activate);

	/* merge menus */
	if(activate)
		view_merge_menus(view_data);
	else
		view_remove_menus(view_data);
}

static void
view_destroy_cb(BonoboView *view, view_data_t *view_data)
{
	view_data->embeddable_data->views = g_list_remove(view_data->embeddable_data->views, view_data);

	gtk_object_destroy(view_data->hadj);
	gtk_object_destroy(view_data->vadj);

	g_free(view_data);
}

static void
reload_all_views(embeddable_data_t *embeddable_data)
{
	GList *l;
	GtkRequisition req;

	for(l = embeddable_data->views; l; l = l->next)
	{
		view_data_t *view_data = l->data;
		GtkGS *gs = GTK_GS(view_data->gs);

		if(embeddable_data->tmp_name == NULL ||
		   !gtk_gs_load(gs, embeddable_data->tmp_name))
			continue;

		gtk_gs_set_pagemedia(gs, -1, 0);

		if(GTK_WIDGET_REALIZED(gs))
			gtk_gs_goto_page(gs, gs->current_page);

		set_page_item_sensitivity(view_data);

		gtk_widget_size_request(GTK_WIDGET(gs), &req);
	}
}

static void
stream_read(Bonobo_Stream stream, embeddable_data_t *embeddable_data)
{
	Bonobo_Stream_iobuf *buffer;
	CORBA_Environment ev;
	CORBA_long bytes_read, bytes_written;

	if(embeddable_data->tmp_name == NULL) {
		embeddable_data->tmp_name = g_malloc(256);
		tmpnam(embeddable_data->tmp_name);
	}

	buffer = Bonobo_Stream_iobuf__alloc();

	CORBA_exception_init(&ev);

	Bonobo_Stream_copyTo(stream, embeddable_data->tmp_name,
						 -1, &bytes_read, &bytes_written, &ev);

	embeddable_data->loaded = TRUE;

	CORBA_exception_free(&ev);
}

/*
 * Loads text from a GNOME_Stream
 */
static void
load_ps_from_stream(BonoboPersistStream *ps, const Bonobo_Stream stream,
					Bonobo_Persist_ContentType type, void *data,
					CORBA_Environment *ev)
{
	embeddable_data_t *embeddable_data = data;

	if (*type && g_strcasecmp (type, "application/postscript") != 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
							 ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	stream_read(stream, embeddable_data);

	reload_all_views(embeddable_data);
}

static Bonobo_Persist_ContentTypeList *
get_ps_content_types(BonoboPersistStream *ps, void *closure,
					 CORBA_Environment *ev)
{
        return bonobo_persist_generate_content_types(1, "application/postscript");
}

static void
view_button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
        view_data_t *view_data = (view_data_t *)data;

        if(event->button == 1 && !view_data->pan) {
			gint wx = 0, wy = 0;
			
			gdk_window_get_pointer(widget->window, &wx, &wy, NULL);
			
			view_data->pan = TRUE;
			if(pan_cursor == NULL)
				pan_cursor = gdk_cursor_new(GDK_FLEUR);
			
			gtk_grab_add(widget);
			gdk_pointer_grab(widget->window, FALSE,
							 GDK_POINTER_MOTION_MASK |
							 GDK_BUTTON_RELEASE_MASK, NULL,
							 pan_cursor, GDK_CURRENT_TIME);
			view_data->prev_x = wx;
			view_data->prev_y = wy;
		}
}

static void
view_button_release_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
        view_data_t *v = (view_data_t *)data;

        if(event->button == 1 && v->pan) {
			v->pan = FALSE;
			gdk_pointer_ungrab(GDK_CURRENT_TIME);
			gtk_grab_remove(widget);
		}
}

static void
view_motion_cb(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
        view_data_t *v = (view_data_t *) data;

        if(v->pan) {
                gtk_gs_scroll(GTK_GS(v->gs), -event->x + v->prev_x, -event->y + v->prev_y);;
                v->prev_x = event->x;
                v->prev_y = event->y;
        }
}

static void
view_realize_cb(GtkWidget *w, view_data_t *view_data)
{
	GtkGS *gs = GTK_GS(w);

	if(gs->gs_filename) {
		GtkRequisition req;

		gtk_gs_set_pagemedia(gs, -1, 0);

		gtk_gs_goto_page(gs, gs->current_page);

		gtk_widget_size_request(GTK_WIDGET(gs), &req);
	}
}

static void
verb_next_page(BonoboView *view, view_data_t *data, const char *verb)
{
	GtkGS *gs = GTK_GS(data->gs);

	if(data->embeddable_data->loaded) {
		gtk_gs_goto_page(gs, gs->current_page + 1);
		set_page_item_sensitivity(data);
	}
}

static void
verb_prev_page(BonoboView *view, view_data_t *data, const char *verb)
{
	GtkGS *gs = GTK_GS(data->gs);

	if(data->embeddable_data->loaded) {
		gtk_gs_goto_page(gs, gs->current_page - 1);
		set_page_item_sensitivity(data);
	}
}

static const BonoboUIVerb ps_verbs[] = {
	{ "NextPage", (BonoboUIVerbFn)verb_next_page, NULL, NULL },
	{ "PrevPage", (BonoboUIVerbFn)verb_prev_page, NULL, NULL },
    { NULL }
};

static BonoboView *
view_factory(BonoboEmbeddable *embeddable, const Bonobo_ViewFrame view_frame,
			 void *data)
{
	BonoboView *view;
	BonoboUIComponent *uic;
	embeddable_data_t *embeddable_data = data;
	view_data_t *view_data = g_new0 (view_data_t, 1);
	GtkGS *gs;

	view_data->embeddable_data = embeddable_data;
	view_data->hadj = gtk_adjustment_new(0.1, 0.0, 1.0, 1.0, 1.0, 0.5);
	view_data->vadj = gtk_adjustment_new(0.1, 0.0, 1.0, 1.0, 1.0, 0.5);
	view_data->pan = FALSE;
	view_data->magstep = 0; /* 1:1 */

	if(embeddable_data->tmp_name)
		view_data->gs = gtk_gs_new_from_file(GTK_ADJUSTMENT(view_data->hadj),
											 GTK_ADJUSTMENT(view_data->vadj),
											 embeddable_data->tmp_name);
	else
		view_data->gs = gtk_gs_new(GTK_ADJUSTMENT(view_data->hadj),
								   GTK_ADJUSTMENT(view_data->vadj));
	gs = GTK_GS(view_data->gs);

	gs->override_media = gtk_gs_get_default_override_media();
	gs->default_page_media = gtk_gs_get_default_page_media();
	gs->antialiased = gtk_gs_get_default_antialiased();

	gtk_widget_set_events(GTK_WIDGET(gs), 
						  GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
						  GDK_POINTER_MOTION_MASK);
	gtk_signal_connect(GTK_OBJECT(gs), "button_press_event",
					   GTK_SIGNAL_FUNC(view_button_press_cb), view_data);
	gtk_signal_connect(GTK_OBJECT(gs), "button_release_event",
					   GTK_SIGNAL_FUNC(view_button_release_cb), view_data);
	gtk_signal_connect(GTK_OBJECT(gs), "motion_notify_event",
					   GTK_SIGNAL_FUNC(view_motion_cb), view_data);
	gtk_signal_connect(GTK_OBJECT(gs), "realize",
					   GTK_SIGNAL_FUNC(view_realize_cb), view_data);

	gtk_widget_show(view_data->gs);

	view = bonobo_view_new(view_data->gs);

	uic = bonobo_view_get_ui_component(view);
	bonobo_ui_component_add_verb_list_with_data(uic, ps_verbs, view_data);

	gtk_signal_connect(GTK_OBJECT(view), "activate",
					   GTK_SIGNAL_FUNC(view_activate_cb), view_data);
	gtk_signal_connect(GTK_OBJECT(view), "system_exception",
					   GTK_SIGNAL_FUNC(view_system_exception_cb), view_data);
	gtk_signal_connect(GTK_OBJECT(view), "destroy",
					   GTK_SIGNAL_FUNC(view_destroy_cb), view_data);

	view_data->view = view;

	embeddable_data->views = g_list_prepend(embeddable_data->views,
											view_data);

	return view;
}

static void
embeddable_system_exception_cb(BonoboEmbeddable *embeddable, CORBA_Object corba_object,
							   CORBA_Environment *ev, gpointer data)
{
	bonobo_object_unref(BONOBO_OBJECT(embeddable));
}

static void
embeddable_destroy_cb(BonoboEmbeddable *embeddable, embeddable_data_t *data)
{
	if(data->tmp_name) {
		unlink(data->tmp_name);
		g_free(data->tmp_name);
	}

	g_free(data);

	active_objects--;
	if(active_objects > 0)
		return;

	if(factory) {
		bonobo_object_unref(BONOBO_OBJECT(factory));
		factory = NULL;
		gtk_main_quit();
	}
}

static BonoboObject *
embeddable_factory(BonoboGenericFactory *this, void *data)
{
	BonoboEmbeddable *embeddable;
	BonoboPersistStream *stream;
	embeddable_data_t *embeddable_data = data;

	embeddable_data = g_new0(embeddable_data_t, 1);
	if(!embeddable_data)
		return NULL;

	embeddable_data->tmp_name = NULL;
	embeddable_data->loaded = FALSE;

	/*
	 * Creates the Embeddable server
	 */
	embeddable = bonobo_embeddable_new(view_factory, embeddable_data);
	if(embeddable == NULL){
		g_free(embeddable_data);
		return NULL;
	}

	/*
	 * Interface BONOBO::PersistStream 
	 */
	stream = bonobo_persist_stream_new(load_ps_from_stream, NULL,
									   NULL, get_ps_content_types,
									   embeddable_data);

	if(stream == NULL) {
		bonobo_object_unref(BONOBO_OBJECT(embeddable));
		g_free(embeddable_data);
		return NULL;
	}

	active_objects++;

	embeddable_data->embeddable = embeddable;

	/*
	 * attach our embeddable_data to the Embeddable
	 */
	gtk_object_set_data(GTK_OBJECT(embeddable), "embeddable-data",
						embeddable_data);
	
	/*
	 * Bind the interfaces
	 */
	bonobo_object_add_interface(BONOBO_OBJECT(embeddable),
								BONOBO_OBJECT(stream));

	gtk_signal_connect(GTK_OBJECT(embeddable), "system_exception",
					   GTK_SIGNAL_FUNC(embeddable_system_exception_cb),
					   embeddable_data);
	gtk_signal_connect_after(GTK_OBJECT(embeddable), "destroy",
							 GTK_SIGNAL_FUNC(embeddable_destroy_cb),
							 embeddable_data);

	return (BonoboObject *) embeddable;
}

static void
init_embeddable_application_ps_factory(void)
{
	factory = bonobo_generic_factory_new(
		"OAFIID:bonobo_application-ps_factory:GGV20001224",
		embeddable_factory, NULL);
}

static void
init_server_factory(int argc, char **argv)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	gnome_init_with_popt_table("bonobo-application-ps", VERSION,
							   argc, argv, oaf_popt_options, 0, NULL); 
	orb = oaf_init(argc, argv);

	if(bonobo_init(orb, CORBA_OBJECT_NIL, CORBA_OBJECT_NIL) == FALSE)
		g_error(_("I could not initialize Bonobo"));

	CORBA_exception_free (&ev);
}

int
main(int argc, char **argv)
{
	init_server_factory(argc, argv);
	init_embeddable_application_ps_factory();

	load_prefs("/bonobo-application-ps/");

	bonobo_main();

	return 0;
}
