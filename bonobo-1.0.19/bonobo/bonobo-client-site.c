/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-client-site.h: a ClientSite object.
 *
 * A BonoboClientSite object acts as the point-of-contact for an
 * embedded component: the contained Bonobo::Embeddable object
 * communicates with the BonoboClientSite when it wants to talk to its
 * container.  There must be a one-to-one mapping between
 * BonoboClientSite objects and embedding BonoboEmbeddable components.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Nat Friedman    (nat@nat.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <stdio.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-client-site.h>
#include <bonobo/bonobo-embeddable.h>
#include <bonobo/bonobo-canvas-item.h>
#include <gdk/gdkprivate.h>
#include <gdk/gdkx.h>
#include <gdk/gdktypes.h>

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

enum {
	SHOW_WINDOW,
	SAVE_OBJECT,
	LAST_SIGNAL
};

static GtkObjectClass *bonobo_client_site_parent_class;
static guint bonobo_client_site_signals [LAST_SIGNAL];

static Bonobo_ItemContainer
impl_Bonobo_ClientSite_getContainer (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboObject         *object = bonobo_object_from_servant (servant);
	Bonobo_ItemContainer  corba_object;
	BonoboClientSite     *client_site = BONOBO_CLIENT_SITE (object);

	corba_object = BONOBO_OBJREF (client_site->container);

	return bonobo_object_dup_ref (corba_object, ev);
}

static void
impl_Bonobo_ClientSite_showWindow (PortableServer_Servant servant, CORBA_boolean shown,
				    CORBA_Environment *ev)
{
	BonoboClientSite *client_site = BONOBO_CLIENT_SITE (bonobo_object_from_servant (servant));
	BonoboObject *object = BONOBO_OBJECT (client_site);

	gtk_signal_emit (GTK_OBJECT (object),
			 bonobo_client_site_signals [SHOW_WINDOW],
			 shown);
}

static Bonobo_Persist_Status
impl_Bonobo_ClientSite_saveObject (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	Bonobo_Persist_Status status;

	status = Bonobo_Persist_SAVE_OK;
	
	gtk_signal_emit (GTK_OBJECT (object),
			 bonobo_client_site_signals [SAVE_OBJECT],
			 &status);
	return status;
}

static void
bonobo_client_site_destroy (GtkObject *object)
{
	BonoboClientSite *client_site = BONOBO_CLIENT_SITE (object);
	
	bonobo_object_list_unref_all (&client_site->view_frames);

	/* Destroy all canvas items */
	/* FIXME: this looks dodgy to me */
	while (client_site->canvas_items) {
		BonoboCanvasItem *item = BONOBO_CANVAS_ITEM (client_site->canvas_items->data);
		gtk_object_unref (GTK_OBJECT (item));
	}

	if (client_site->bound_embeddable) {
		bonobo_object_unref (BONOBO_OBJECT (client_site->bound_embeddable));
		client_site->bound_embeddable = NULL;
	}

	bonobo_client_site_parent_class->destroy (object);
}

static void
default_show_window (BonoboClientSite *cs, CORBA_boolean shown)
{
	cs->child_shown = shown ? 1 : 0;
}

static void
default_save_object (BonoboClientSite *cs, Bonobo_Persist_Status *status)
{
}

static void
bonobo_client_site_class_init (BonoboClientSiteClass *klass)
{
	BonoboObjectClass *gobject_class = (BonoboObjectClass *) klass;
	GtkObjectClass    *object_class  = (GtkObjectClass *) gobject_class;
	POA_Bonobo_ClientSite__epv  *epv = &klass->epv;

	bonobo_client_site_parent_class = gtk_type_class (PARENT_TYPE);

	bonobo_client_site_signals [SHOW_WINDOW] =
		gtk_signal_new ("show_window",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboClientSiteClass, show_window), 
				gtk_marshal_NONE__INT,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_INT); 
	bonobo_client_site_signals [SAVE_OBJECT] =
		gtk_signal_new ("save_object",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboClientSiteClass, save_object), 
				gtk_marshal_NONE__POINTER,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_POINTER); 

	gtk_object_class_add_signals (object_class,
				      bonobo_client_site_signals,
				      LAST_SIGNAL);
	
	object_class->destroy = bonobo_client_site_destroy;
	klass->show_window = default_show_window;
	klass->save_object = default_save_object;

	epv->getContainer = impl_Bonobo_ClientSite_getContainer;
	epv->showWindow   = impl_Bonobo_ClientSite_showWindow;
	epv->saveObject   = impl_Bonobo_ClientSite_saveObject;
}

static void
bonobo_client_site_init (BonoboClientSite *client_site)
{
	client_site->bound_embeddable = NULL;
}

/**
 * bonobo_client_site_construct:
 * @client_site: The BonoboClientSite object to initialize
 * @container: a BonoboContainer to bind to.
 *
 * This initializes an object of type BonoboClientSite.  See the description
 * for bonobo_client_site_new () for more details.
 *
 * Returns: the constructed BonoboClientSite @client_site.
 */
BonoboClientSite *
bonobo_client_site_construct (BonoboClientSite    *client_site,
			      BonoboItemContainer *container)
{
	g_return_val_if_fail (client_site != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_CLIENT_SITE (client_site), NULL);
	g_return_val_if_fail (container != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_ITEM_CONTAINER (container), NULL);
	
	BONOBO_CLIENT_SITE (client_site)->container = container;

	return client_site;
}

/**
 * bonobo_client_site_new:
 * @container: The container to which this client_site belongs.
 *
 * Container programs should provide a BonoboClientSite GTK object (ie,
 * a Bonobo::ClientSite CORBA server) for each Embeddable which they
 * embed.  This is the contact end point for the remote
 * Bonobo::Embeddable object.
 *
 * This routine creates a new BonoboClientSite.
 *
 * Returns: The activated BonoboClientSite object bound to the @container
 * container.
 */
BonoboClientSite *
bonobo_client_site_new (BonoboItemContainer *container)
{
	BonoboClientSite *client_site;

	g_return_val_if_fail (container != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_ITEM_CONTAINER (container), NULL);
	
	client_site = gtk_type_new (bonobo_client_site_get_type ());

	return bonobo_client_site_construct (client_site, container);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboClientSite, 
			   Bonobo_ClientSite,
			   PARENT_TYPE,
			   bonobo_client_site);

/** 
 * bonobo_client_site_bind_embeddable:
 * @client_site: the client site to which the remote Embeddable object will be bound.
 * @object: The remote object which supports the Bonobo::Embeddable interface.
 *
 * This routine binds a remote Embeddable object to a local
 * BonoboClientSite object.  The idea is that there is always a
 * one-to-one mapping between BonoboClientSites and BonoboEmbeddables.
 * The Embeddable uses its BonoboClientSite to communicate with the
 * container in which it is embedded.
 *
 * Returns: %TRUE if @object was successfully bound to @client_site
 * @client_site.
 */
gboolean
bonobo_client_site_bind_embeddable (BonoboClientSite   *client_site,
				    BonoboObjectClient *object)
{
	CORBA_Object embeddable_object;
	CORBA_Environment ev;
	
	g_return_val_if_fail (client_site != NULL, FALSE);
	g_return_val_if_fail (object != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_CLIENT_SITE (client_site), FALSE);
	g_return_val_if_fail (BONOBO_IS_OBJECT_CLIENT (object), FALSE);

	embeddable_object = bonobo_object_client_query_interface (
		object, "IDL:Bonobo/Embeddable:1.0", NULL);

	if (embeddable_object == CORBA_OBJECT_NIL)
		return FALSE;

	CORBA_exception_init (&ev);

	/* The QI adds a ref */
	Bonobo_Unknown_unref (BONOBO_OBJREF (object), &ev);

	Bonobo_Embeddable_setClientSite (embeddable_object, 
					 BONOBO_OBJREF (client_site), &ev);
		
	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (BONOBO_OBJECT (object),
					 embeddable_object, &ev);
		CORBA_exception_free (&ev);
		return FALSE;
	}
	CORBA_exception_free (&ev);

	if (client_site->bound_embeddable)
		bonobo_object_unref (BONOBO_OBJECT (client_site->bound_embeddable));

	client_site->bound_embeddable = bonobo_object_client_from_corba (embeddable_object);
	bonobo_object_client_ref (client_site->bound_embeddable, NULL);

	return TRUE;
}

/**
 * bonobo_client_site_get_embeddable:
 * @client_site: A BonoboClientSite object which is bound to a remote
 * BonoboObject server.
 *
 * Returns: The BonoboObjectClient object which corresponds to the
 * remote BonoboObject to which @client_site is bound.
 */
BonoboObjectClient *
bonobo_client_site_get_embeddable (BonoboClientSite *client_site)
{
	g_return_val_if_fail (
		BONOBO_IS_CLIENT_SITE (client_site), NULL);

	return client_site->bound_embeddable;
}

/**
 * bonobo_client_site_get_container:
 * @client_site: A BonoboClientSite object which is bound to a remote
 * BonoboObject server.
 *
 * Returns: The BonoboObjectClient object which corresponds to the
 * remote BonoboObject to which @client_site is bound.
 **/
BonoboItemContainer *
bonobo_client_site_get_container (BonoboClientSite *client_site)
{
	g_return_val_if_fail (
		BONOBO_IS_CLIENT_SITE (client_site), NULL);

	return client_site->container;
}

static void
bonobo_client_site_view_frame_destroy (BonoboViewFrame  *view_frame,
				       BonoboClientSite *client_site)
{
	/*
	 * Remove this view frame.
	 */
	client_site->view_frames = g_list_remove (client_site->view_frames, view_frame);
}

/**
 * bonobo_client_site_new_view_full:
 * @client_site: the client site that contains a remote Embeddable object.
 * @uic: The CORBA object for the container's UIContainer server.
 * @visible_cover: %TRUE if the cover should draw a border when it is active.
 * @active_view: %TRUE if the view should be uncovered when it is created.
 *
 * Creates a ViewFrame and asks the remote @server_object (which must
 * support the Bonobo::Embeddable interface) to provide a new view of
 * its data.  The remote @server_object will construct a BonoboView
 * object which corresponds to the new BonoboViewFrame returned by this
 * function.
 * 
 * Returns: A BonoboViewFrame object that contains the view frame for
 * the new view of @server_object.
 */
BonoboViewFrame *
bonobo_client_site_new_view_full (BonoboClientSite  *client_site,
				  Bonobo_UIContainer uic,
				  gboolean           visible_cover,
				  gboolean           active_view)
{
	Bonobo_Embeddable server_object;
	BonoboViewFrame *view_frame;
	BonoboWrapper *wrapper;
	Bonobo_View view;

	CORBA_Environment ev;

	g_return_val_if_fail (client_site != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_CLIENT_SITE (client_site), NULL);
	g_return_val_if_fail (client_site->bound_embeddable != NULL, NULL);

	/*
	 * 1. Create the view frame.
	 */
	view_frame = bonobo_view_frame_new (client_site, uic);
	wrapper = BONOBO_WRAPPER (bonobo_view_frame_get_wrapper (view_frame));
	bonobo_wrapper_set_visibility (wrapper, visible_cover);
	bonobo_wrapper_set_covered (wrapper, ! active_view);

	/*
	 * 2. Now, create the view.
	 */
	server_object = BONOBO_OBJREF (client_site->bound_embeddable);
	CORBA_exception_init (&ev);
 	view = Bonobo_Embeddable_createView (server_object, 
					     BONOBO_OBJREF (view_frame), &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_check_env (
			BONOBO_OBJECT (client_site),
			server_object,
			&ev);
		bonobo_object_unref (BONOBO_OBJECT (view_frame));
		CORBA_exception_free (&ev);
		return NULL;
	}

	bonobo_view_frame_bind_to_view (view_frame, view);
	bonobo_object_release_unref (view, &ev);
	
	/*
	 * 3. Add this new view frame to the list of ViewFrames for
	 * this embedded component.
	 */
	client_site->view_frames = g_list_prepend (client_site->view_frames, view_frame);
	
	gtk_signal_connect (GTK_OBJECT (view_frame), "destroy",
			    GTK_SIGNAL_FUNC (bonobo_client_site_view_frame_destroy),
			    client_site);

	CORBA_exception_free (&ev);		
	return view_frame;
}

/**
 * bonobo_client_site_new_view:
 * @client_site: the client site that contains a remote Embeddable
 * object.
 * @uic: The UIContainer object.
 *
 * The same as bonobo_client_site_new_view_full() with an inactive,
 * visible cover.
 * 
 * Returns: A BonoboViewFrame object that contains the view frame for
 * the new view of @server_object.
 */
BonoboViewFrame *
bonobo_client_site_new_view (BonoboClientSite  *client_site,
			     Bonobo_UIContainer uic)
{

	return bonobo_client_site_new_view_full (client_site, uic, TRUE, FALSE);
}

static void
canvas_item_destroyed (GnomeCanvasItem *item, BonoboClientSite *client_site)
{
	client_site->canvas_items = g_list_remove (client_site->canvas_items, item);
}
		      
/**
 * bonobo_client_site_new_item:
 * @client_site: The client site that contains a remote Embeddable object
 * @uic: The UI container for the item.
 * @group: The Canvas group that will be the parent for the new item.
 *
 * Returns: A GnomeCanvasItem that wraps the remote Canvas Item.
 */
GnomeCanvasItem *
bonobo_client_site_new_item (BonoboClientSite  *client_site,
			     Bonobo_UIContainer uic,
			     GnomeCanvasGroup  *group)
{
	GnomeCanvasItem *item;
	Bonobo_Embeddable corba_emb;
		
	g_return_val_if_fail (client_site != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_CLIENT_SITE (client_site), NULL);
	g_return_val_if_fail (client_site->bound_embeddable != NULL, NULL);
	g_return_val_if_fail (group != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_CANVAS_GROUP (group), NULL);

	corba_emb = BONOBO_OBJREF (client_site->bound_embeddable);

	item = gnome_canvas_item_new (group, bonobo_canvas_item_get_type (),
				      "corba_ui_container", uic, 
				      "corba_embeddable", corba_emb, NULL); 

	/*
	 * 5. Add this new item to the list of CanvasItems for
	 * this embedded component.
	 */
	client_site->canvas_items = g_list_prepend (client_site->canvas_items, item);

	gtk_signal_connect (GTK_OBJECT (item), "destroy",
			    GTK_SIGNAL_FUNC (canvas_item_destroyed), client_site);
	
	return item;
}
