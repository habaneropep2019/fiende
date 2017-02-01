/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-widget.c: BonoboWidget object.
 *
 * Authors:
 *   Nat Friedman    (nat@helixcode.com)
 *
 * Copyright 1999 Helix Code, Inc.
 * 
 * Bonobo component embedding for hydrocephalic imbeciles.
 *
 * Pure cane sugar.
 *
 * This purpose of BonoboWidget is to make container-side use of
 * Bonobo as easy as pie.  This widget has two functions:
 *
 *   1. Provide a simple wrapper for embedding a single-view
 *      subdocument.  In this case, BonoboWidget handles creating
 *      the embeddable, binding it to a local BonoboClientSite,
 *      creating a view for it, and displaying the view.  You can use
 *      the accessor functions (bonobo_widget_get_view_frame,
 *      etc) to get at the actual Bonobo objects which underlie the
 *      whole process.
 *
 *      In order to do this, just call:
 * 
 *        bw = bonobo_widget_new_subdoc ("moniker of subdoc embddable",
 *                                        top_level_uicontainer);
 * 
 *      And then insert the 'bw' widget into the widget tree of your
 *      application like so:
 *
 *        gtk_container_add (some_container, bw);
 *
 *      You are free to make the UIContainer argument to
 *      bonobo_widget_new_subdoc() be CORBA_OBJECT_NIL.
 *
 *   2. Provide a simple wrapper for embedding Controls.  Embedding
 *      controls is already really easy, but BonoboWidget reduces
 *      the work from about 5 lines to 1.  To embed a given control,
 *      just do:
 *
 *        bw = bonobo_widget_new_control ("moniker for control");
 *        gtk_container_add (some_container, bw);
 *
 *      Ta da!
 *
 *      NB. A simple moniker might look like 'file:/tmp/a.jpeg' or
 *      OAFIID:GNOME_Evolution_Calendar_Control
 */

#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-widget.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>

struct _BonoboWidgetPrivate {

	BonoboObjectClient *server;

	/*
	 * Control stuff.
	 */
	BonoboControlFrame *control_frame;
	
	/*
	 * Subdocument (Embeddable/View) things.
	 */
	BonoboItemContainer *container;
	BonoboClientSite    *client_site;
	BonoboViewFrame     *view_frame;
	Bonobo_UIContainer   uic;
};

static BonoboWrapperClass *bonobo_widget_parent_class;

static BonoboObjectClient *
bonobo_widget_launch_component (const char *moniker,
				const char *if_name)
{
	Bonobo_Unknown corba_ref;
	BonoboObjectClient *server;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	corba_ref = bonobo_get_object (moniker, if_name, &ev);

	if (BONOBO_EX (&ev)) {
		char *txt;
		g_warning ("Activation exception '%s'",
			   (txt = bonobo_exception_get_text (&ev)));
		g_free (txt);
		server = CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	if (corba_ref == CORBA_OBJECT_NIL)
		return NULL;

	return bonobo_object_client_from_corba (corba_ref);
}


/*
 *
 * Control support for BonoboWidget.
 *
 */
/**
 * bonobo_widget_construct_control_from_objref:
 * @bw: A BonoboWidget to construct
 * @control: A CORBA Object reference to an IDL:Bonobo/Control:1.0
 * @uic: Bonobo_UIContainer for the launched object.
 *
 * This is a constructor function.  Only usable for wrapping and
 * derivation of new objects.  For normal use, please refer to
 * #bonobo_widget_new_control_from_objref.
 *
 * Returns: A #BonoboWidget (the @bw)
 */
BonoboWidget *
bonobo_widget_construct_control_from_objref (BonoboWidget      *bw,
					     Bonobo_Control     control,
					     Bonobo_UIContainer uic)
{
	GtkWidget    *control_frame_widget;

	/*
	 * Create a local ControlFrame for it.
	 */
	bw->priv->control_frame = bonobo_control_frame_new (uic);

	bonobo_control_frame_bind_to_control (bw->priv->control_frame, control);

	/*
	 * People that pass us controls get them sunk.
	 */
	bonobo_object_release_unref (control, NULL);

	bonobo_control_frame_set_autoactivate (bw->priv->control_frame, TRUE);

	/*
	 * Grab the actual widget which visually contains the remote
	 * Control.  This is a GtkSocket, in reality.
	 */
	control_frame_widget = bonobo_control_frame_get_widget (bw->priv->control_frame);

	/*
	 * Now stick it into this BonoboWidget.
	 */
	gtk_container_add (GTK_CONTAINER (bw),
			   control_frame_widget);
	gtk_widget_show (control_frame_widget);

	if (uic != CORBA_OBJECT_NIL)
		bw->priv->uic = bonobo_object_dup_ref (uic, NULL);

	return bw;
}

/**
 * bonobo_widget_construct_control:
 * @bw: A BonoboWidget to construct
 * @moniker: A Moniker describing the object to be activated 
 * @uic: Bonobo_UIContainer for the launched object.
 *
 * This is a constructor function.  Only usable for wrapping and
 * derivation of new objects.  For normal use, please refer to
 * #bonobo_widget_new_control.
 *
 * This function will unref the passed in @bw in case it cannot launch
 * the component and return %NULL in such a case.  Otherwise it returns
 * the @bw itself.
 *
 * Returns: A #BonoboWidget or %NULL
 */
BonoboWidget *
bonobo_widget_construct_control (BonoboWidget      *bw,
				 const char        *moniker,
				 Bonobo_UIContainer uic)
{
	Bonobo_Control control;

	/*
	 * Create the remote Control object.
	 */
	bw->priv->server = bonobo_widget_launch_component (
		moniker, "IDL:Bonobo/Control:1.0");
	if (bw->priv->server == NULL) {
		gtk_object_unref (GTK_OBJECT (bw));
		return NULL;
	}

	control = BONOBO_OBJREF (bw->priv->server);

	return bonobo_widget_construct_control_from_objref (bw, control, uic);
}

/**
 * bonobo_widget_new_control_from_objref:
 * @control: A CORBA Object reference to an IDL:Bonobo/Control:1.0
 * @uic: Bonobo_UIContainer for the launched object.
 *
 * This function is a simple wrapper for easily embedding controls
 * into applications.  This function is used when you have already
 * a CORBA object reference to an IDL:Bonobo/Control:1.0 (the
 * @control) argument. 
 *
 * Returns: the @control wrapped as a #GtkWidget.
 */
GtkWidget *
bonobo_widget_new_control_from_objref (Bonobo_Control     control,
				       Bonobo_UIContainer uic)
{
	BonoboWidget *bw;

	g_return_val_if_fail (control != CORBA_OBJECT_NIL, NULL);

	bw = gtk_type_new (BONOBO_WIDGET_TYPE);

	bw = bonobo_widget_construct_control_from_objref (bw, control, uic);

	if (bw == NULL)
		return NULL;

	return GTK_WIDGET (bw);
}

/**
 * bonobo_widget_new_control:
 * @moniker: A Moniker describing the object to be activated 
 * @uic: Bonobo_UIContainer for the launched object.
 *
 * This function is a simple wrapper for easily embedding controls
 * into applications.  It will launch the component identified by @id
 * and will return it as a GtkWidget.
 *
 * Returns: A #GtkWidget that is bound to the Bonobo Control. 
 */
GtkWidget *
bonobo_widget_new_control (const char        *moniker,
			   Bonobo_UIContainer uic)
{
	BonoboWidget *bw;

	g_return_val_if_fail (moniker != NULL, NULL);

	bw = gtk_type_new (BONOBO_WIDGET_TYPE);

	bw = bonobo_widget_construct_control (bw, moniker, uic);

	if (bw == NULL)
		return NULL;
	else
		return GTK_WIDGET (bw);
}

/**
 * bonobo_widget_get_control_frame:
 * @bonobo_widget: a Bonobo Widget returned by one of the bonobo_widget_new() functions.
 *
 * Every IDL:Bonobo/Control:1.0 needs to be placed inside an
 * IDL:Bonobo/ControlFrame:1.0.  This returns the BonoboControlFrame
 * object that wraps the Control in the @bonobo_widget. 
 * 
 * Returns: The BonoboControlFrame associated with the @bonobo_widget
 */
BonoboControlFrame *
bonobo_widget_get_control_frame (BonoboWidget *bonobo_widget)
{
	g_return_val_if_fail (bonobo_widget != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_WIDGET (bonobo_widget), NULL);

	return bonobo_widget->priv->control_frame;
}


/*
 *
 * Subdocument support for BonoboWidget.
 *
 */
static BonoboWidget *
bonobo_widget_create_subdoc_object (BonoboWidget      *bw,
				    const char        *moniker,
				    Bonobo_UIContainer uic)
{
	GtkWidget *view_widget;
	
	/*
	 * Create the BonoboContainer.  This will contain
	 * just one BonoboClientSite.
	 */
	bw->priv->container = bonobo_item_container_new ();

	bw->priv->server = bonobo_widget_launch_component (
		moniker, "IDL:Bonobo/Embeddable:1.0");
	if (bw->priv->server == NULL)
		return NULL;
	
	/*
	 * Create the client site.  This is the container-side point
	 * of contact for the remote component.
	 */
	bw->priv->client_site = bonobo_client_site_new (bw->priv->container);

	/*
	 * Bind the local ClientSite object to the remote Embeddable
	 * component.
	 */
	if (!bonobo_client_site_bind_embeddable (bw->priv->client_site, bw->priv->server))
		return NULL;

	/*
	 * Now create a new view for the remote object.
	 */
	bw->priv->view_frame = bonobo_client_site_new_view (bw->priv->client_site, uic);

	/*
	 * Add the view frame.
	 */
	view_widget = bonobo_view_frame_get_wrapper (bw->priv->view_frame);
	gtk_container_add (GTK_CONTAINER (bw), view_widget);
	gtk_widget_show (view_widget);

	if (uic != CORBA_OBJECT_NIL)
		bw->priv->uic = bonobo_object_dup_ref (uic, NULL);
	
	return bw;
}

/**
 * bonobo_widget_new_subdoc:
 * @moniker: A moniker description of the Object to be activated.
 * @uic: Bonobo_UIContainer for the launched object.
 *
 * This function is a simple wrapper for easily embedding documents
 * into applications.  It will launch the component identified by @id
 * and will return it as a GtkWidget.
 *
 * This will launch a single view of the embeddable activated by @moniker.
 *
 * FIXME: this function should really be using bonobo_get_object() instead
 * of bonobo_activate_object() to launch the object.
 *
 * Returns: A #GtkWidget that is bound to the Bonobo Control. 
 */
GtkWidget *
bonobo_widget_new_subdoc (const char        *moniker,
			  Bonobo_UIContainer uic)
{
	BonoboWidget *bw;

	g_return_val_if_fail (moniker != NULL, NULL);

	bw = gtk_type_new (BONOBO_WIDGET_TYPE);

	if (bw == NULL)
		return NULL;

	if (!bonobo_widget_create_subdoc_object (bw, moniker, uic)) {
		gtk_object_destroy (GTK_OBJECT (bw));
		return NULL;
	}
 
	bonobo_view_frame_set_covered (bw->priv->view_frame, FALSE);

	return GTK_WIDGET (bw);
}

/**
 * bonobo_widget_get_container
 * @bonobo_widget: the #BonoboWidget to query.
 *
 * This operation is only valid for BonoboWidgets that were created
 * by the bonobo_widget_new_subdoc(). 
 *
 * Returns: the BonoboItemContainer associated with this @bonobo_widget
 */
BonoboItemContainer *
bonobo_widget_get_container (BonoboWidget *bonobo_widget)
{
	g_return_val_if_fail (bonobo_widget != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_WIDGET (bonobo_widget), NULL);

	return bonobo_widget->priv->container;
}

/**
 * bonobo_widget_get_client_site:
 * @bonobo_widget: the #BonoboWidget to query.
 *
 * This operation is only valid for BonoboWidgets that were created
 * by the bonobo_widget_new_subdoc(). 
 *
 * Returns: the #BonoboClientSite associated with this @bonobo_widget
 */
BonoboClientSite *
bonobo_widget_get_client_site (BonoboWidget *bonobo_widget)
{
	g_return_val_if_fail (bonobo_widget != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_WIDGET (bonobo_widget), NULL);

	return bonobo_widget->priv->client_site;
}

/**
 * bonobo_widget_get_view_frame:
 * @bonobo_widget: the #BonoboWidget to query.
 *
 * This operation is only valid for BonoboWidgets that were created
 * by the bonobo_widget_new_subdoc(). 
 *
 * Returns: The #BonoboViewFrame associated with this @bonobo_widget.
 */
BonoboViewFrame *
bonobo_widget_get_view_frame (BonoboWidget *bonobo_widget)
{
	g_return_val_if_fail (bonobo_widget != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_WIDGET (bonobo_widget), NULL);

	return bonobo_widget->priv->view_frame;
}

/**
 * bonobo_widget_get_uih:
 * @bonobo_widget: the #BonoboWidget to query.
 *
 * Returns: the CORBA object reference to the Bonobo_UIContainer
 * associated with the @bonobo_widget.
 */
Bonobo_UIContainer
bonobo_widget_get_uih (BonoboWidget *bonobo_widget)
{
	g_return_val_if_fail (bonobo_widget != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_WIDGET (bonobo_widget), NULL);

	return bonobo_widget->priv->uic;
}



/*
 *
 * Generic (non-control/subdoc specific) BonoboWidget stuff.
 *
 */

/**
 * bonobo_widget_get_server:
 * @bonobo_widget: the #BonoboWidget to query.
 *
 * Returns: The BonoboObjectClient (a wrapper for the CORBA object
 * reference) to the object that this BonoboWidget is wrapping. 
 */ 
BonoboObjectClient *
bonobo_widget_get_server (BonoboWidget *bonobo_widget)
{
	g_return_val_if_fail (bonobo_widget != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_WIDGET (bonobo_widget), NULL);

	return bonobo_widget->priv->server;
}

Bonobo_Unknown
bonobo_widget_get_objref (BonoboWidget *bonobo_widget)
{
	g_return_val_if_fail (bonobo_widget != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_WIDGET (bonobo_widget), NULL);

	return BONOBO_OBJREF (bonobo_widget->priv->server);
}

static void
bonobo_widget_destroy (GtkObject *object)
{
	BonoboWidget *bw = BONOBO_WIDGET (object);
	BonoboWidgetPrivate *priv = bw->priv;
	
	if (priv->control_frame)
		bonobo_object_unref (BONOBO_OBJECT (priv->control_frame));
	if (priv->container)
		bonobo_object_unref (BONOBO_OBJECT (priv->container));
	if (priv->client_site)
		bonobo_object_unref (BONOBO_OBJECT (priv->client_site));
	if (priv->view_frame)
		bonobo_object_unref (BONOBO_OBJECT (priv->view_frame));
	if (priv->uic != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (priv->uic, NULL);

	g_free (priv);
	GTK_OBJECT_CLASS (bonobo_widget_parent_class)->destroy (object);
}

static void
bonobo_widget_size_request (GtkWidget *widget,
			    GtkRequisition *requisition)
{
	GtkBin *bin;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_WIDGET (widget));
	g_return_if_fail (requisition != NULL);

	bin = GTK_BIN (widget);

	if (bin->child && GTK_WIDGET_VISIBLE (bin->child)) {
		GtkRequisition child_requisition;
      
		gtk_widget_size_request (bin->child, &child_requisition);

		requisition->width = child_requisition.width;
		requisition->height = child_requisition.height;
	}
}

static void
bonobo_widget_size_allocate (GtkWidget *widget,
			     GtkAllocation *allocation)
{
	GtkBin *bin;
	GtkAllocation child_allocation;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (BONOBO_IS_WIDGET (widget));
	g_return_if_fail (allocation != NULL);

	widget->allocation = *allocation;
	bin = GTK_BIN (widget);

	child_allocation.x = allocation->x;
	child_allocation.y = allocation->y;
	child_allocation.width = allocation->width;
	child_allocation.height = allocation->height;

	if (bin->child) {
		gtk_widget_size_allocate (bin->child, &child_allocation);
	}
}

static void
bonobo_widget_class_init (BonoboWidgetClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

	bonobo_widget_parent_class = gtk_type_class (GTK_TYPE_BIN);

	widget_class->size_request = bonobo_widget_size_request;
	widget_class->size_allocate = bonobo_widget_size_allocate;

	object_class->destroy = bonobo_widget_destroy;
}

static void
bonobo_widget_init (BonoboWidget *bw)
{
	bw->priv = g_new0 (BonoboWidgetPrivate, 1);
}

GtkType
bonobo_widget_get_type (void)
{
	static GtkType type = 0;

	if (! type) {
		static const GtkTypeInfo info = {
			"BonoboWidget",
			sizeof (BonoboWidget),
			sizeof (BonoboWidgetClass),
			(GtkClassInitFunc) bonobo_widget_class_init,
			(GtkObjectInitFunc) bonobo_widget_init,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_bin_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_widget_set_property:
 * @control: A #BonoboWidget that represents an IDL:Bonobo/Control:1.0
 * @first_prop: first property name to set.
 *
 * This is a utility function used to set a number of properties
 * in the Bonobo Control in @control.
 *
 * This function takes a variable list of arguments that must be NULL
 * terminated.  Arguments come in tuples: a string (for the argument
 * name) and the data type that is to be transfered.  The
 * implementation of the actual setting of the PropertyBag values is
 * done by the bonobo_property_bag_client_setv() function).
 *
 * FIXME: This function is error prone because it depends on the
 * client and the server to agree on the data types to be sent.  If
 * the server arguments change the data type, this routine will not
 * be able to cope gracefully with this condition.
 *
 * This only works for BonoboWidgets that represent controls (ie,
 * that were returned by bonobo_widget_new_control_from_objref() or
 * bonobo_widget_new_control().
 */
void
bonobo_widget_set_property (BonoboWidget      *control,
			    const char        *first_prop, ...)
{
	Bonobo_PropertyBag pb;
	CORBA_Environment  ev;

	va_list args;
	va_start (args, first_prop);

	g_return_if_fail (control != NULL);
	g_return_if_fail (first_prop != NULL);
	g_return_if_fail (control->priv != NULL);
	g_return_if_fail (BONOBO_IS_WIDGET (control));

	CORBA_exception_init (&ev);
	
	pb = bonobo_control_frame_get_control_property_bag (
		control->priv->control_frame, &ev);

	if (BONOBO_EX (&ev))
		g_warning ("Error getting property bag from control");
	else {
		/* FIXME: this should use ev */
		char *err = bonobo_property_bag_client_setv (pb, &ev, first_prop, args);

		if (err)
			g_warning ("Error '%s'", err);
	}

	bonobo_object_release_unref (pb, &ev);

	CORBA_exception_free (&ev);

	va_end (args);
}


/**
 * bonobo_widget_get_property:
 * @control: A #BonoboWidget that represents an IDL:Bonobo/Control:1.0
 * @first_prop: first property name to set.
 *
 * This is a utility function used to get a number of properties
 * in the Bonobo Control in @control.
 *
 * This function takes a variable list of arguments that must be NULL
 * terminated.  Arguments come in tuples: a string (for the argument
 * name) and a pointer where the data will be stored.  The
 * implementation of the actual setting of the PropertyBag values is
 * done by the bonobo_property_bag_client_setv() function).
 *
 * FIXME: This function is error prone because it depends on the
 * client and the server to agree on the data types to be sent.  If
 * the server arguments change the data type, this routine will not
 * be able to cope gracefully with this condition.
 *
 * This only works for BonoboWidgets that represent controls (ie,
 * that were returned by bonobo_widget_new_control_from_objref() or
 * bonobo_widget_new_control().
 */
void
bonobo_widget_get_property (BonoboWidget      *control,
			    const char        *first_prop, ...)
{
	Bonobo_PropertyBag pb;
	CORBA_Environment  ev;

	va_list args;
	va_start (args, first_prop);

	g_return_if_fail (control != NULL);
	g_return_if_fail (first_prop != NULL);
	g_return_if_fail (control->priv != NULL);
	g_return_if_fail (BONOBO_IS_WIDGET (control));

	CORBA_exception_init (&ev);
	
	pb = bonobo_control_frame_get_control_property_bag (
		control->priv->control_frame, &ev);

	if (BONOBO_EX (&ev))
		g_warning ("Error getting property bag from control");
	else {
		/* FIXME: this should use ev */
		char *err = bonobo_property_bag_client_getv (pb, &ev, first_prop, args);

		if (err)
			g_warning ("Error '%s'", err);
	}

	bonobo_object_release_unref (pb, &ev);

	CORBA_exception_free (&ev);

	va_end (args);
}
