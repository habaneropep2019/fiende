/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-control-item.c: A special toolbar item for controls.
 *
 * Author:
 *	Jon K Hellan (hellan@acm.org)
 *
 * Copyright 2000 Jon K Hellan.
 * Copyright (C) 2001 Eazel, Inc.
 */

#include "config.h"
#include <gnome.h>
#include "bonobo-ui-toolbar-control-item.h"

#include "bonobo-exception.h"

static BonoboUIToolbarButtonItemClass *parent_class = NULL;

struct _BonoboUIToolbarControlItemPrivate {
        BonoboWidget *control;	/* The wrapped control */
	GtkWidget *button;	/* Button to display instead of control in
				   vertical mode */
	GtkWidget *box;		/* Container for control and button. Which of
				   its children is visible depends on
				   orientation */
        GtkWidget *eventbox;	/* The eventbox which makes tooltips work */

	BonoboUIToolbarControlDisplay hdisplay;
	BonoboUIToolbarControlDisplay vdisplay;
};

static void
set_control_property_bag_value (BonoboUIToolbarControlItem *item,
				const char *name,
				BonoboArg *value)
{
	BonoboControlFrame *frame;
	Bonobo_PropertyBag bag;
	Bonobo_Property property;
	CORBA_Environment ev;
	
	frame = bonobo_widget_get_control_frame (item->priv->control);
	bag = bonobo_control_frame_get_control_property_bag (frame, NULL);
	if (bag == CORBA_OBJECT_NIL)
		return;
        property = bonobo_property_bag_client_get_property (bag, name, NULL);
	if (property != CORBA_OBJECT_NIL) {
		CORBA_exception_init (&ev);
		Bonobo_Property_setValue (property, value, &ev);
		CORBA_Object_release (property, &ev);
		CORBA_exception_free (&ev);
	}
	bonobo_object_release_unref (bag, NULL);
}

#define MAKE_SET_CONTROL_PROPERTY_BAG_VALUE(gtype, paramtype, capstype)	\
static void								\
set_control_property_bag_##gtype (BonoboUIToolbarControlItem *item,	\
				  const char *name,			\
				  paramtype value)			\
{									\
	BonoboArg *arg;							\
									\
	arg = bonobo_arg_new (BONOBO_ARG_##capstype);			\
	BONOBO_ARG_SET_##capstype (arg, value);				\
	set_control_property_bag_value (item, name, arg);		\
	bonobo_arg_release (arg);					\
}

MAKE_SET_CONTROL_PROPERTY_BAG_VALUE (gboolean, gboolean,     BOOLEAN)
MAKE_SET_CONTROL_PROPERTY_BAG_VALUE (gint,     gint,         INT)
MAKE_SET_CONTROL_PROPERTY_BAG_VALUE (string,   const char *, STRING)

/* BonoboUIToolbarButtonItem virtual methods.  */
static void
impl_set_icon  (BonoboUIToolbarButtonItem *button_item,
		GdkPixbuf                 *icon)
{
	BonoboUIToolbarControlItemPrivate *priv;
	BonoboUIToolbarControlItem *control_item;

	control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (button_item);
	priv = control_item->priv;

	bonobo_ui_toolbar_button_item_set_icon (
		BONOBO_UI_TOOLBAR_BUTTON_ITEM (priv->button), icon);
}

static void
impl_set_label (BonoboUIToolbarButtonItem *button_item,
		const char                *label)
{
	BonoboUIToolbarControlItemPrivate *priv;
	BonoboUIToolbarControlItem *control_item;

	control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (button_item);
	priv = control_item->priv;

	bonobo_ui_toolbar_button_item_set_label (
		BONOBO_UI_TOOLBAR_BUTTON_ITEM (priv->button), label);
	set_control_property_bag_string (control_item, "bonobo:label", label);
}

/* BonoboUIToolbarItem methods.  */

/*
 * We are assuming that there's room in horizontal orientation, but not
 * vertical. This can be made more sophisticated by looking at the control's
 * requested geometry.
 */
static void
impl_set_orientation (BonoboUIToolbarItem *item,
		      GtkOrientation orientation)
{
	BonoboUIToolbarControlItem        *control_item;
	BonoboUIToolbarControlDisplay      display;
	BonoboUIToolbarControlItemPrivate *priv;

	control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (item);
	priv = control_item->priv;

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		display = priv->hdisplay;
	else
		display = priv->vdisplay;
	
	switch (display) {

	case BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_CONTROL:
		gtk_widget_hide (priv->button);
		gtk_widget_show (GTK_WIDGET (priv->control));
		break;

	case BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_BUTTON:
		gtk_widget_hide (GTK_WIDGET (priv->control));
		gtk_widget_show (priv->button);
		break;

	case BONOBO_UI_TOOLBAR_CONTROL_DISPLAY_NONE:
		gtk_widget_hide (GTK_WIDGET (priv->control));
		gtk_widget_hide (priv->button);
		break;

	default:
		g_assert_not_reached ();
	}

	set_control_property_bag_gint (control_item, "bonobo:orientation", orientation);

	if (BONOBO_UI_TOOLBAR_ITEM_CLASS (parent_class)->set_orientation)
		(* BONOBO_UI_TOOLBAR_ITEM_CLASS (parent_class)
		 ->set_orientation) (item, orientation);	
}

static void
impl_set_style (BonoboUIToolbarItem     *item,
		BonoboUIToolbarItemStyle style)
{
	BonoboUIToolbarControlItem *control_item;

	control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (item);
	bonobo_ui_toolbar_item_set_style (
		BONOBO_UI_TOOLBAR_ITEM (control_item->priv->button), style);
	set_control_property_bag_gint (control_item, "bonobo:style", style);
}

static void
impl_set_want_label (BonoboUIToolbarItem     *item,
		     gboolean                 want_label)
{
	BonoboUIToolbarControlItem *control_item;

	control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (item);
	bonobo_ui_toolbar_item_set_want_label (
		BONOBO_UI_TOOLBAR_ITEM (control_item->priv->button),
		want_label);
	set_control_property_bag_gboolean (control_item, "bonobo:want_label", want_label);
}

static void
impl_set_tooltip (BonoboUIToolbarItem     *item,
                  GtkTooltips             *tooltips,
                  const char              *tooltip)
{
	BonoboUIToolbarControlItem *control_item;
	GtkWidget *eventbox;

	control_item = BONOBO_UI_TOOLBAR_CONTROL_ITEM (item);
	eventbox = control_item->priv->eventbox;
	
	if (tooltip && eventbox)
		gtk_tooltips_set_tip (tooltips, eventbox, tooltip, NULL);
}

/* GtkObject methods.  */

static void
impl_destroy (GtkObject *object)
{
	g_free (BONOBO_UI_TOOLBAR_CONTROL_ITEM (object)->priv);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		 (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);	
}

/* Gtk+ object initialization.  */

static void
class_init (BonoboUIToolbarControlItemClass *klass)
{
        BonoboUIToolbarButtonItemClass *button_item_class;
        BonoboUIToolbarItemClass *item_class;
	GtkObjectClass *object_class;
	
	button_item_class = BONOBO_UI_TOOLBAR_BUTTON_ITEM_CLASS (klass);
	item_class = BONOBO_UI_TOOLBAR_ITEM_CLASS (klass);
	object_class = GTK_OBJECT_CLASS (klass);

        button_item_class->set_icon = impl_set_icon;
        button_item_class->set_label = impl_set_label;
        item_class->set_tooltip     = impl_set_tooltip;
        item_class->set_orientation = impl_set_orientation;
	item_class->set_style       = impl_set_style;
	item_class->set_want_label  = impl_set_want_label;
	object_class->destroy = impl_destroy;

        parent_class = gtk_type_class (
		bonobo_ui_toolbar_button_item_get_type ());
}


static void
init (BonoboUIToolbarControlItem *control_item)
{
        control_item->priv = g_new0 (BonoboUIToolbarControlItemPrivate, 1);
}

GtkType
bonobo_ui_toolbar_control_item_get_type (void)
{
        static GtkType type = 0;

        if (type == 0) {
                static const GtkTypeInfo info = {
                        "BonoboUIToolbarControlItem",
                        sizeof (BonoboUIToolbarControlItem),
                        sizeof (BonoboUIToolbarControlItemClass),
                        (GtkClassInitFunc) class_init,
                        (GtkObjectInitFunc) init,
                        /* reserved_1 */ NULL,
                        /* reserved_2 */ NULL,
                        (GtkClassInitFunc) NULL,
                };

                type = gtk_type_unique (
			bonobo_ui_toolbar_button_item_get_type (),
			&info);
        }

        return type;
}

static void
proxy_activate_cb (GtkWidget *button, GtkObject *item)
{
	gtk_signal_emit_by_name (item, "activate");
}

GtkWidget *
bonobo_ui_toolbar_control_item_construct (
        BonoboUIToolbarControlItem *control_item,
        Bonobo_Control              control_ref)
{
        BonoboUIToolbarControlItemPrivate *priv = control_item->priv;
	GtkWidget *w  = bonobo_widget_new_control_from_objref (
                bonobo_object_dup_ref (control_ref, NULL), CORBA_OBJECT_NIL);

        if (!w)
                return NULL;

	priv->control  = BONOBO_WIDGET (w); 
	priv->button   = bonobo_ui_toolbar_button_item_new (NULL, NULL);
        priv->eventbox = gtk_event_box_new ();
        priv->box      = gtk_vbox_new (FALSE, 0);
	
	gtk_signal_connect (GTK_OBJECT (priv->button), "activate",
			    (GtkSignalFunc) proxy_activate_cb, control_item);
	
	gtk_container_add (GTK_CONTAINER (priv->box),
			   GTK_WIDGET (priv->control));
        gtk_container_add (GTK_CONTAINER (priv->box), priv->button);

        gtk_container_add (GTK_CONTAINER (priv->eventbox), priv->box);

	gtk_widget_show (GTK_WIDGET (priv->control));
	gtk_widget_show (priv->box);
	gtk_widget_show   (priv->eventbox);
        gtk_container_add (GTK_CONTAINER (control_item), priv->eventbox);

        return GTK_WIDGET (control_item);
}

GtkWidget *
bonobo_ui_toolbar_control_item_new (Bonobo_Control control_ref)
{
        BonoboUIToolbarControlItem *control_item;
	GtkWidget *ret;

        control_item = gtk_type_new (
                bonobo_ui_toolbar_control_item_get_type ());

        ret = bonobo_ui_toolbar_control_item_construct (
		control_item, control_ref);

	if (!ret)
		impl_destroy (GTK_OBJECT (control_item));

	return ret;
}

void
bonobo_ui_toolbar_control_item_set_display (BonoboUIToolbarControlItem    *item,
					    BonoboUIToolbarControlDisplay  hdisplay,
					    BonoboUIToolbarControlDisplay  vdisplay)
{
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_CONTROL_ITEM (item));

	item->priv->hdisplay = hdisplay;
	item->priv->vdisplay = vdisplay;
}
