/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo Embeddable object.
 *
 * A BonoboEmbeddable object represents the actual object being
 * embedded.  A BonoboEmbeddable may have one or more BonoboViews, each
 * of which is an identical embedded window which displays the
 * BonoboEmbeddable's contents.  The BonoboEmbeddable is associated with
 * a BonoboClientSite, which is a container-side object with which the
 * BonoboEmbeddable communicates.
 *
 * Authors:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Nat Friedman    (nat@nat.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-embeddable.h>

#define PARENT_TYPE BONOBO_X_OBJECT_TYPE

static GtkObjectClass *bonobo_embeddable_parent_class;

enum {
	HOST_NAME_CHANGED,
	URI_CHANGED,
	LAST_SIGNAL
};

static guint embeddable_signals [LAST_SIGNAL];

struct _BonoboEmbeddablePrivate {
	/* The instantiated views for this Embeddable. */
	GList *views;

	/* The instantiated Canvas Items for this Embeddable. */
	GList *canvas_items;
	
	/* The View factory */
	BonoboViewFactory view_factory;
	void *view_factory_closure;

	/* For the Canvas Item */
	GnomeItemCreator item_creator;
	void *item_creator_data;
};

static void
impl_Bonobo_Embeddable_setClientSite (PortableServer_Servant servant,
				      const Bonobo_ClientSite client_site,
				      CORBA_Environment *ev)
{
	BonoboEmbeddable *embeddable = BONOBO_EMBEDDABLE (bonobo_object_from_servant (servant));
	CORBA_Environment evx;

	CORBA_exception_init (&evx);

	if (embeddable->client_site != CORBA_OBJECT_NIL)
		CORBA_Object_release (client_site, &evx);
	
	embeddable->client_site = CORBA_Object_duplicate (client_site, &evx);
        CORBA_exception_free (&evx);							     
}

static Bonobo_ClientSite
impl_Bonobo_Embeddable_getClientSite (PortableServer_Servant servant,
				      CORBA_Environment     *ev)
{
	BonoboEmbeddable *embeddable = BONOBO_EMBEDDABLE (
		bonobo_object_from_servant (servant));

	return bonobo_object_dup_ref (BONOBO_OBJREF (embeddable->client_site),
				      ev);
}

static void
impl_Bonobo_Embeddable_setHostName (PortableServer_Servant servant,
				    const CORBA_char      *name,
				    const CORBA_char      *appname,
				    CORBA_Environment     *ev)
{
	BonoboEmbeddable *embeddable = BONOBO_EMBEDDABLE (bonobo_object_from_servant (servant));

	if (embeddable->host_name)
		g_free (embeddable->host_name);
	if (embeddable->host_appname)
		g_free (embeddable->host_appname);

	embeddable->host_name    = g_strdup (name);
	embeddable->host_appname = g_strdup (appname);

	gtk_signal_emit (GTK_OBJECT (embeddable),
			 embeddable_signals [HOST_NAME_CHANGED]);
}


static void
impl_Bonobo_Embeddable_close (PortableServer_Servant servant,
			     const Bonobo_Embeddable_CloseMode mode,
			     CORBA_Environment *ev)
{
}

static void
impl_Bonobo_Embeddable_advise (PortableServer_Servant servant,
			      const Bonobo_AdviseSink advise,
			      CORBA_Environment *ev)
{
}

static void
impl_Bonobo_Embeddable_unadvise (PortableServer_Servant servant, CORBA_Environment *ev)
{
}

static CORBA_long
impl_Bonobo_Embeddable_getMiscStatus (PortableServer_Servant servant,
				      const CORBA_long type,
				      CORBA_Environment *ev)
{

	return 0;
}

static void
ping_container (BonoboEmbeddable *embeddable)
{
	/*
	 * If all of the views are gone, that *might* mean that
	 * our container application died.  So ping it to find
	 * out if it's still alive.
	 */
	if ((embeddable->priv->views != NULL) || (embeddable->priv->canvas_items != NULL))
		return;

	/*
	 * ORBit has some issues.
	 *
	 * Calling gnome_unknown_ping on a dead object causes either:
	 * 
	 *     . SIGPIPE (handled; please see gnome-main.c)
	 *     . A blocking call to connect().
	 *
	 * So we just assume the remote end is dead here.  This ORBit
	 * problem needs to be fixed, though, so that this code can be
	 * re-enabled.
	 */
	if (0) {
/*	if (! gnome_unknown_ping (embeddable->client_site)) {*/
		/*
		 * The remote end is dead; it's time for
		 * us to die too.
		 */
		bonobo_object_unref (BONOBO_OBJECT (embeddable));
	}
}

static void
bonobo_embeddable_view_destroy_cb (BonoboView *view, gpointer data)
{
	BonoboEmbeddable *embeddable = BONOBO_EMBEDDABLE (data);

	/*
	 * Remove this view from our list of views.
	 */
	embeddable->priv->views = g_list_remove (embeddable->priv->views, view);

	ping_container (embeddable);
}

static Bonobo_View
impl_Bonobo_Embeddable_createView (PortableServer_Servant servant,
				   Bonobo_ViewFrame view_frame,
				   CORBA_Environment *ev)
{
	BonoboEmbeddable *embeddable = BONOBO_EMBEDDABLE (bonobo_object_from_servant (servant));
	BonoboView       *view;
	
	if (embeddable->priv->view_factory == NULL)
		return CORBA_OBJECT_NIL;

	view = embeddable->priv->view_factory (
		embeddable, view_frame,
		embeddable->priv->view_factory_closure);

	if (view == NULL)
		return CORBA_OBJECT_NIL;

	if (BONOBO_OBJREF (view) == CORBA_OBJECT_NIL){
		g_warning ("Returned view does not have a CORBA object bound");
		bonobo_object_unref (BONOBO_OBJECT (view));
		return CORBA_OBJECT_NIL;
	}
	bonobo_view_set_view_frame (view, view_frame);
	bonobo_view_set_embeddable (view, embeddable);

	embeddable->priv->views = g_list_prepend (embeddable->priv->views, view);

	gtk_signal_connect (GTK_OBJECT (view), "destroy",
			    GTK_SIGNAL_FUNC (bonobo_embeddable_view_destroy_cb), embeddable);

	return CORBA_Object_duplicate (BONOBO_OBJREF (view), ev);
}

static void
impl_Bonobo_Embeddable_setURI (PortableServer_Servant servant,
			       const CORBA_char      *uri,
			       CORBA_Environment     *ev)
{
	BonoboEmbeddable *embeddable = BONOBO_EMBEDDABLE (bonobo_object_from_servant (servant));

	bonobo_embeddable_set_uri (embeddable, uri);
}

static void
canvas_item_destroyed (BonoboCanvasComponent *comp, BonoboEmbeddable *embeddable)
{
	GnomeCanvasItem *item;

	item = bonobo_canvas_component_get_item (comp);
	gtk_object_destroy (GTK_OBJECT (item->canvas));
	
	/* Remove the canvas item from the list of items we keep */
	embeddable->priv->canvas_items = g_list_remove (embeddable->priv->canvas_items, comp);

	ping_container (embeddable);
}

static BonoboCanvasComponent *
make_canvas_component (BonoboEmbeddable *embeddable, gboolean aa, Bonobo_Canvas_ComponentProxy item_proxy)
{
	BonoboCanvasComponent *component;
	GnomeCanvas *pseudo_canvas;
	
	pseudo_canvas = bonobo_canvas_new (aa, item_proxy);
	
	component = (*embeddable->priv->item_creator)(
		embeddable, pseudo_canvas,
		embeddable->priv->item_creator_data);

	if (component == NULL){
		gtk_object_destroy (GTK_OBJECT (pseudo_canvas));
		return NULL;
	}

	/* Now keep track of it */
	embeddable->priv->canvas_items = g_list_prepend (embeddable->priv->canvas_items, component);
	gtk_signal_connect (GTK_OBJECT (component), "destroy",
			    GTK_SIGNAL_FUNC (canvas_item_destroyed), embeddable);
	
	return component;
}

static Bonobo_Canvas_Component
impl_Bonobo_Embeddable_createCanvasItem (PortableServer_Servant servant,
					 CORBA_boolean aa,
					 Bonobo_Canvas_ComponentProxy _item_proxy,
					 CORBA_Environment *ev)
{
	BonoboEmbeddable *embeddable = BONOBO_EMBEDDABLE (bonobo_object_from_servant (servant));
	Bonobo_Canvas_ComponentProxy item_proxy;
	BonoboCanvasComponent *component;
	
	if (embeddable->priv->item_creator == NULL)
		return CORBA_OBJECT_NIL;

	item_proxy = CORBA_Object_duplicate (_item_proxy, ev);
	
	component = make_canvas_component (embeddable, aa, item_proxy);

	return bonobo_object_dup_ref (BONOBO_OBJREF (component), ev);
}

/**
 * bonobo_embeddable_construct:
 * @embeddable: BonoboEmbeddable object to construct.
 * @view_factory: Factory routine that provides new views of the embeddable on demand
 * @view_factory_data: pointer passed to the @view_factory routine to provide context.
 * @item_factory: A factory routine that creates BonoboCanvasComponents.
 * @item_factory_data: pointer passed to the @item_factory routine.
 *
 * This routine constructs a Bonobo::Embeddable CORBA server and activates it.
 *
 * The @view_factory routine will be invoked by this CORBA server when
 * a request arrives to get a new view of the embeddable (embeddable
 * should be able to provide multiple views of themselves upon demand).
 * The @view_factory_data pointer is passed to this factory routine untouched to
 * allow the factory to get some context on what it should create.
 *
 * The @item_factory will be invoked if the container application requests a
 * canvas item version of this embeddable.  The routine @item_factory will
 * be invoked with the @item_factory_data argument
 *
 * Returns: The constructed object.
 */
BonoboEmbeddable *
bonobo_embeddable_construct_full (BonoboEmbeddable *embeddable,
				  BonoboViewFactory view_factory,
				  void             *factory_data,
				  GnomeItemCreator item_factory,
				  void             *item_factory_data)
{
	
	g_return_val_if_fail (embeddable != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_EMBEDDABLE (embeddable), NULL);

	embeddable->priv->view_factory         = view_factory;
	embeddable->priv->view_factory_closure = factory_data;
	embeddable->priv->item_creator         = item_factory;
	embeddable->priv->item_creator_data    = item_factory_data;
		
	return embeddable;
}

/**
 * bonobo_embeddable_construct:
 * @embeddable: BonoboEmbeddable object to construct.
 * @factory: Factory routine that provides new views of the embeddable on demand
 * @data: pointer passed to the @factory routine to provide context.
 * 
 * This routine constructs a Bonobo::Embeddable CORBA server and activates it.  The
 * @factory routine will be invoked by this CORBA server when a request arrives
 * to get a new view of the embeddable (embeddable should be able to provide
 * multiple views of themselves upon demand).  The @data pointer is passed
 * to this factory routine untouched to allow the factory to get some context
 * on what it should create.
 *
 * Returns: The constructed object.
 */
BonoboEmbeddable *
bonobo_embeddable_construct (BonoboEmbeddable  *embeddable,
			     BonoboViewFactory  factory,
			     void              *data)
{
	return bonobo_embeddable_construct_full (embeddable, factory, data, NULL, NULL);
}

/**
 * bonobo_embeddable_new:
 * @factory: Factory routine that provides new views of the embeddable on demand
 * @data: pointer passed to the @factory routine to provide context.
 *
 * This routine creates a Bonobo::Embeddable CORBA server and activates it.  The
 * @factory routine will be invoked by this CORBA server when a request arrives
 * to get a new view of the embeddable (embeddable should be able to provide
 * multiple views of themselves upon demand).  The @data pointer is passed
 * to this factory routine untouched to allow the factory to get some context
 * on what it should create.
 *
 * Returns a BonoboEmbeddable that contains an activated Bonobo::Embeddable
 * CORBA server.
 */
BonoboEmbeddable *
bonobo_embeddable_new (BonoboViewFactory factory, void *data)
{
	BonoboEmbeddable *embeddable;

	g_return_val_if_fail (factory != NULL, NULL);

	embeddable = gtk_type_new (BONOBO_EMBEDDABLE_TYPE);
	
	return bonobo_embeddable_construct (embeddable, factory, data);
}

/**
 * bonobo_embeddable_new_canvas_item:
 * @item_factory: Factory routine that provides new canvas items of the embeddable on demand
 * @data: pointer passed to the @factory routine to provide context.
 *
 * This routine creates a Bonobo::Embeddable CORBA server and activates it.  The
 * @factory routine will be invoked by this CORBA server when a request arrives
 * to get a new view of the embeddable (embeddable should be able to provide
 * multiple views of themselves upon demand).  The @data pointer is passed
 * to this factory routine untouched to allow the factory to get some context
 * on what it should create.
 *
 * Returns a BonoboEmbeddable that contains an activated Bonobo::Embeddable
 * CORBA server.
 */
BonoboEmbeddable *
bonobo_embeddable_new_canvas_item (GnomeItemCreator item_factory, void *data)
{
	BonoboEmbeddable *embeddable;

	g_return_val_if_fail (item_factory != NULL, NULL);

	embeddable = gtk_type_new (BONOBO_EMBEDDABLE_TYPE);
	
	return bonobo_embeddable_construct_full (
		embeddable, NULL, NULL, item_factory, data);
}

static void
bonobo_embeddable_destroy (GtkObject *object)
{
	BonoboEmbeddable *embeddable = BONOBO_EMBEDDABLE (object);

	/* Destroy all our views. */
	bonobo_object_list_unref_all (&embeddable->priv->views);
	bonobo_object_list_unref_all (&embeddable->priv->canvas_items);
	
	if (embeddable->uri)
		g_free (embeddable->uri);
	
	/*
	 * Release any references we might keep
	 */
	if (embeddable->client_site != CORBA_OBJECT_NIL){
		CORBA_Environment ev;

		CORBA_exception_init (&ev);
		CORBA_Object_release (embeddable->client_site, &ev);
		CORBA_exception_free (&ev);
	}

	g_free (embeddable->priv);
	
	bonobo_embeddable_parent_class->destroy (object);
}

static void
bonobo_embeddable_class_init (BonoboEmbeddableClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	POA_Bonobo_Embeddable__epv *epv = &klass->epv;

	bonobo_embeddable_parent_class = gtk_type_class (PARENT_TYPE);

	embeddable_signals [HOST_NAME_CHANGED] =
                gtk_signal_new ("host_name_changed",
                                GTK_RUN_LAST,
                                object_class->type,
                                GTK_SIGNAL_OFFSET(BonoboEmbeddableClass,host_name_changed), 
                                gtk_marshal_NONE__STRING,
                                GTK_TYPE_NONE, 1, GTK_TYPE_STRING);
	embeddable_signals [URI_CHANGED] =
                gtk_signal_new ("uri_changed",
                                GTK_RUN_LAST,
                                object_class->type,
                                GTK_SIGNAL_OFFSET(BonoboEmbeddableClass,uri_changed), 
                                gtk_marshal_NONE__STRING,
                                GTK_TYPE_NONE, 1, GTK_TYPE_STRING);

	gtk_object_class_add_signals (object_class, embeddable_signals,
				      LAST_SIGNAL);

	object_class->destroy = bonobo_embeddable_destroy;

	epv->setClientSite    = impl_Bonobo_Embeddable_setClientSite;
	epv->getClientSite    = impl_Bonobo_Embeddable_getClientSite;
	epv->setHostName      = impl_Bonobo_Embeddable_setHostName;
	epv->close            = impl_Bonobo_Embeddable_close;
	epv->advise           = impl_Bonobo_Embeddable_advise;
	epv->unadvise         = impl_Bonobo_Embeddable_unadvise;
	epv->getMiscStatus    = impl_Bonobo_Embeddable_getMiscStatus;
	epv->createView       = impl_Bonobo_Embeddable_createView;
	epv->setURI           = impl_Bonobo_Embeddable_setURI;
	epv->createCanvasItem = impl_Bonobo_Embeddable_createCanvasItem;
}

static void
bonobo_embeddable_init (BonoboObject *object)
{
	BonoboEmbeddable *embeddable = BONOBO_EMBEDDABLE (object);

	embeddable->priv = g_new0 (BonoboEmbeddablePrivate, 1);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboEmbeddable, 
			   Bonobo_Embeddable,
			   PARENT_TYPE,
			   bonobo_embeddable);

/**
 * bonobo_embeddable_set_view_factory:
 * @embeddable: The embeddable object to operate on.
 * @factory: A pointer to a function that can provide BonoboView objects on demand.
 * @data: data to pass to the @factory function.
 *
 * This routine defines the view factory for this embeddable component.
 * When a container requires a view, the routine specified in @factory
 * will be invoked to create a new BonoboView object to satisfy this request.
 */
void
bonobo_embeddable_set_view_factory (BonoboEmbeddable *embeddable,
				   BonoboViewFactory factory,
				   void *data)
{
	g_return_if_fail (embeddable != NULL);
	g_return_if_fail (BONOBO_IS_EMBEDDABLE (embeddable));
	g_return_if_fail (factory != NULL);

	embeddable->priv->view_factory = factory;
	embeddable->priv->view_factory_closure = data;
}

/**
 * bonobo_embeddable_get_uri:
 * @embeddable: The embeddable object to operate on.
 *
 * Returns the URI that this object represents
 */
const char *
bonobo_embeddable_get_uri (BonoboEmbeddable *embeddable)
{
	g_return_val_if_fail (embeddable != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_EMBEDDABLE (embeddable), NULL);

	return embeddable->uri;
}

/**
 * bonobo_embeddable_set_uri:
 * @embeddable: The embeddable object to operate on.
 * @uri: the URI this embeddable represents.
 *
 * Sets the URI that this object represents.
 */
void
bonobo_embeddable_set_uri (BonoboEmbeddable *embeddable, const char *uri)
{
	g_return_if_fail (embeddable != NULL);
	g_return_if_fail (BONOBO_IS_EMBEDDABLE (embeddable));

	if (embeddable->uri){
		g_free (embeddable->uri);
		embeddable->uri = NULL;
	}
	
	if (uri)
		embeddable->uri = g_strdup (uri);

	gtk_signal_emit (GTK_OBJECT (embeddable),
			 embeddable_signals [URI_CHANGED],
			 embeddable->uri);
}

/**
 * bonobo_embeddable_foreach_view:
 * @embeddable: Embeddable on which we operate
 * @fn: function to be invoked for each existing BonoboView
 * @data: data to pass to function
 *
 * Invokes the @fn function for each existing view.
 */
void
bonobo_embeddable_foreach_view (BonoboEmbeddable *embeddable,
			       BonoboEmbeddableForeachViewFn fn,
			       void *data)
{
	GList *copy, *l;
	
	g_return_if_fail (embeddable != NULL);
	g_return_if_fail (BONOBO_IS_EMBEDDABLE (embeddable));
	g_return_if_fail (fn != NULL);

	copy = g_list_copy (embeddable->priv->views);
	for (l = copy; l; l = l->next)
		(*fn)(BONOBO_VIEW (l->data), data);

	g_list_free (copy);
}

/**
 * bonobo_embeddable_foreach_item:
 * @embeddable: Embeddable on which we operate
 * @fn: function to be invoked for each existing GnomeItem
 * @data: data to pass to function
 *
 * Invokes the @fn function for each existing item.
 */
void
bonobo_embeddable_foreach_item (BonoboEmbeddable *embeddable,
			       BonoboEmbeddableForeachItemFn fn,
			       void *data)
{
	GList *copy, *l;
	
	g_return_if_fail (embeddable != NULL);
	g_return_if_fail (BONOBO_IS_EMBEDDABLE (embeddable));
	g_return_if_fail (fn != NULL);

	copy = g_list_copy (embeddable->priv->canvas_items);
	for (l = copy; l; l = l->next)
		(*fn)(BONOBO_CANVAS_COMPONENT (l->data), data);

	g_list_free (copy);
}
