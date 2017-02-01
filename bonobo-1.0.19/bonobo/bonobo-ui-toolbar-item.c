/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-item.c
 *
 * Author: Ettore Perazzoli
 *
 * Copyright (C) 2000 Helix Code, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include "bonobo-ui-toolbar-item.h"


#define PARENT_TYPE gtk_bin_get_type ()
static GtkBinClass *parent_class = NULL;

struct _BonoboUIToolbarItemPrivate {
	/* Whether this item wants to have a label when the toolbar style is
           `BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT'.  */
	gboolean want_label;

	/* Whether this item wants to be expanded to all the available
           width/height.  */
	gboolean expandable;

	/* if set, pack this item on the right side of the toolbar */
	gboolean pack_end;

	/* Orientation for this item.  */
	GtkOrientation orientation;

	/* Style for this item.  */
	BonoboUIToolbarItemStyle style;	
	
	/* minimum width (or height, if rotated) for this item */
	int minimum_width;
};

enum {
	SET_ORIENTATION,
	SET_STYLE,
	SET_WANT_LABEL,
	ACTIVATE,
	STATE_ALTERED,
	LAST_SIGNAL
};

static int signals[LAST_SIGNAL] = { 0 };


/* GtkObject methods.  */

static void
impl_destroy (GtkObject *object)
{
	BonoboUIToolbarItem *toolbar_item;
	BonoboUIToolbarItemPrivate *priv;

	toolbar_item = BONOBO_UI_TOOLBAR_ITEM (object);
	priv = toolbar_item->priv;

	g_free (priv);

	if (GTK_OBJECT_CLASS (parent_class)->destroy != NULL)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


/* GtkWidget methods.  */

static void
impl_size_request (GtkWidget *widget,
		   GtkRequisition *requisition_return)
{
	BonoboUIToolbarItem *toolbar_item;
	BonoboUIToolbarItemPrivate *priv;
	GtkRequisition child_requisition;
	GtkWidget *child;
	int border_width;

	toolbar_item = BONOBO_UI_TOOLBAR_ITEM (widget);
	priv = toolbar_item->priv;

	border_width = GTK_CONTAINER (widget)->border_width;
	requisition_return->width  = border_width;
	requisition_return->height = border_width;

	child = GTK_BIN (widget)->child;
	if (child == NULL)
		return;

	gtk_widget_size_request (child, &child_requisition);

	if (child_requisition.width < priv->minimum_width)
		child_requisition.width = priv->minimum_width;
	
	requisition_return->width  += child_requisition.width;
	requisition_return->height += child_requisition.height;
}

static void
impl_size_allocate (GtkWidget *widget,
		    GtkAllocation *allocation)
{
	GtkAllocation child_allocation;
	GtkWidget *child;
	int border_width;

	widget->allocation = *allocation;

	child = GTK_BIN (widget)->child;
	if (child == NULL)
		return;

	border_width = GTK_CONTAINER (widget)->border_width;

	if (allocation->width > border_width) {
		child_allocation.x = allocation->x + border_width;
		child_allocation.width = allocation->width - border_width;
	} else {
		child_allocation.x = allocation->x;
		child_allocation.width = allocation->width;
	}

	if (allocation->height > border_width) {
		child_allocation.y = allocation->y + border_width;
		child_allocation.height = allocation->height - border_width;
	} else {
		child_allocation.y = allocation->y;
		child_allocation.height = allocation->height;
	}

	gtk_widget_size_allocate (GTK_BIN (widget)->child, &child_allocation);
}


/* BonoboUIToolbarItem signals.  */

static void
impl_set_orientation (BonoboUIToolbarItem *item,
		      GtkOrientation orientation)
{
	BonoboUIToolbarItemPrivate *priv;

	priv = item->priv;

	if (priv->orientation == orientation)
		return;

	priv->orientation = orientation;

	gtk_widget_queue_resize (GTK_WIDGET (item));
}

static void
impl_set_style (BonoboUIToolbarItem *item,
		BonoboUIToolbarItemStyle style)
{
	BonoboUIToolbarItemPrivate *priv;

	priv = item->priv;

	if (priv->style == style)
		return;

	priv->style = style;

	gtk_widget_queue_resize (GTK_WIDGET (item));
}

static void
impl_set_want_label (BonoboUIToolbarItem *item,
		     gboolean want_label)
{
	BonoboUIToolbarItemPrivate *priv;

	priv = item->priv;

	priv->want_label = want_label;
}


/* Gtk+ object initialization.  */

static void
class_init (GtkObjectClass *object_class)
{
	GtkWidgetClass *widget_class;
	BonoboUIToolbarItemClass *toolbar_item_class;

	object_class->destroy = impl_destroy;

	widget_class = GTK_WIDGET_CLASS (object_class);
	widget_class->size_request  = impl_size_request;
	widget_class->size_allocate = impl_size_allocate;

	toolbar_item_class = BONOBO_UI_TOOLBAR_ITEM_CLASS (object_class);
	toolbar_item_class->set_orientation = impl_set_orientation;
	toolbar_item_class->set_style       = impl_set_style;
	toolbar_item_class->set_want_label  = impl_set_want_label;

	signals[SET_ORIENTATION]
		= gtk_signal_new ("set_orientation",
				  GTK_RUN_LAST,
				  object_class->type,
				  GTK_SIGNAL_OFFSET (BonoboUIToolbarItemClass, set_orientation),
				  gtk_marshal_NONE__INT,
				  GTK_TYPE_NONE, 1,
				  GTK_TYPE_INT);

	signals[SET_STYLE]
		= gtk_signal_new ("set_style",
				  GTK_RUN_LAST,
				  object_class->type,
				  GTK_SIGNAL_OFFSET (BonoboUIToolbarItemClass, set_style),
				  gtk_marshal_NONE__INT,
				  GTK_TYPE_NONE, 1,
				  GTK_TYPE_INT);

	signals[SET_WANT_LABEL] =
		gtk_signal_new ("set_want_label",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboUIToolbarItemClass, set_want_label),
				gtk_marshal_NONE__BOOL,
				GTK_TYPE_NONE, 1,
				GTK_TYPE_BOOL);

	signals[STATE_ALTERED]
		= gtk_signal_new ("state_altered",
				  GTK_RUN_LAST,
				  object_class->type,
				  GTK_SIGNAL_OFFSET (BonoboUIToolbarItemClass, activate),
				  gtk_marshal_NONE__STRING,
				  GTK_TYPE_NONE, 1, GTK_TYPE_STRING);

	signals[ACTIVATE]
		= gtk_signal_new ("activate",
				  GTK_RUN_LAST,
				  object_class->type,
				  GTK_SIGNAL_OFFSET (BonoboUIToolbarItemClass, activate),
				  gtk_marshal_NONE__NONE,
				  GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	parent_class = gtk_type_class (PARENT_TYPE);
}

static void
init (GtkObject *object)
{
	BonoboUIToolbarItem *toolbar_item;
	BonoboUIToolbarItemPrivate *priv;

	toolbar_item = BONOBO_UI_TOOLBAR_ITEM (object);

	priv = g_new (BonoboUIToolbarItemPrivate, 1);

	priv->want_label    = FALSE;
	priv->orientation   = GTK_ORIENTATION_HORIZONTAL;
	priv->style         = BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_VERTICAL;
	priv->expandable    = FALSE;
	priv->pack_end	    = FALSE;
	priv->minimum_width = 0;
	
	toolbar_item->priv = priv;
}


GtkType
bonobo_ui_toolbar_item_get_type (void)
{
	static GtkType type = 0;

	if (type == 0) {
		static const GtkTypeInfo info = {
			"BonoboUIToolbarItem",
			sizeof (BonoboUIToolbarItem),
			sizeof (BonoboUIToolbarItemClass),
			(GtkClassInitFunc) class_init,
			(GtkObjectInitFunc) init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		type = gtk_type_unique (PARENT_TYPE, &info);
	}

	return type;
}

GtkWidget *
bonobo_ui_toolbar_item_new (void)
{
	BonoboUIToolbarItem *new;

	new = gtk_type_new (bonobo_ui_toolbar_item_get_type ());

	return GTK_WIDGET (new);
}


void
bonobo_ui_toolbar_item_set_tooltip (BonoboUIToolbarItem *item,
				    GtkTooltips         *tooltips,
				    const char          *tooltip)
{
	BonoboUIToolbarItemClass *klass;

	g_return_if_fail (item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item));

	klass = BONOBO_UI_TOOLBAR_ITEM_CLASS (((GtkObject *)item)->klass);

	if (klass->set_tooltip)
		klass->set_tooltip (item, tooltips, tooltip);

	/* FIXME: implement setting of tooltips */
}

void
bonobo_ui_toolbar_item_set_state (BonoboUIToolbarItem *item,
				  const char          *new_state)
{
	BonoboUIToolbarItemClass *klass;

	g_return_if_fail (item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item));

	klass = BONOBO_UI_TOOLBAR_ITEM_CLASS (((GtkObject *)item)->klass);

	if (klass->set_state)
		klass->set_state (item, new_state);
}

void
bonobo_ui_toolbar_item_set_orientation (BonoboUIToolbarItem *item,
					GtkOrientation orientation)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item));
	g_return_if_fail (orientation == GTK_ORIENTATION_HORIZONTAL
			  || orientation == GTK_ORIENTATION_VERTICAL);

	gtk_signal_emit (GTK_OBJECT (item), signals[SET_ORIENTATION], orientation);
}

GtkOrientation
bonobo_ui_toolbar_item_get_orientation (BonoboUIToolbarItem *item)
{
	BonoboUIToolbarItemPrivate *priv;

	g_return_val_if_fail (item != NULL, GTK_ORIENTATION_HORIZONTAL);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item), GTK_ORIENTATION_HORIZONTAL);

	priv = item->priv;

	return priv->orientation;
}


void
bonobo_ui_toolbar_item_set_style (BonoboUIToolbarItem *item,
			       BonoboUIToolbarItemStyle style)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item));
	g_return_if_fail (style == BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_ONLY
			  || style == BONOBO_UI_TOOLBAR_ITEM_STYLE_TEXT_ONLY
			  || style == BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_HORIZONTAL
			  || style == BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_VERTICAL);

	gtk_signal_emit (GTK_OBJECT (item), signals[SET_STYLE], style);
}

BonoboUIToolbarItemStyle
bonobo_ui_toolbar_item_get_style (BonoboUIToolbarItem *item)
{
	BonoboUIToolbarItemPrivate *priv;

	g_return_val_if_fail (item != NULL,
			      BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_VERTICAL);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item),
			      BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_VERTICAL);

	priv = item->priv;

	return priv->style;
}

void
bonobo_ui_toolbar_item_set_want_label (BonoboUIToolbarItem *item,
				    gboolean want_label)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item));

	gtk_signal_emit (GTK_OBJECT (item), signals[SET_WANT_LABEL], want_label);
}

gboolean
bonobo_ui_toolbar_item_get_want_label (BonoboUIToolbarItem *item)
{
	BonoboUIToolbarItemPrivate *priv;

	g_return_val_if_fail (item != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item), FALSE);

	priv = item->priv;

	return priv->want_label;
}

void
bonobo_ui_toolbar_item_set_expandable (BonoboUIToolbarItem *item,
				       gboolean expandable)
{
	BonoboUIToolbarItemPrivate *priv;

	g_return_if_fail (item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item));

	priv = item->priv;

	if ((priv->expandable && expandable) || (! priv->expandable && ! expandable))
		return;

	priv->expandable = expandable;
	gtk_widget_queue_resize (GTK_WIDGET (item));
}

gboolean
bonobo_ui_toolbar_item_get_expandable (BonoboUIToolbarItem *item)
{
	BonoboUIToolbarItemPrivate *priv;

	g_return_val_if_fail (item != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item), FALSE);

	priv = item->priv;
	return priv->expandable;
}

void
bonobo_ui_toolbar_item_set_pack_end (BonoboUIToolbarItem *item,
				     gboolean pack_end)
{
	BonoboUIToolbarItemPrivate *priv;

	g_return_if_fail (item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item));

	priv = item->priv;

	if ((priv->pack_end && pack_end) || (! priv->pack_end && ! pack_end))
		return;

	priv->pack_end = pack_end;
	gtk_widget_queue_resize (GTK_WIDGET (item));
}

gboolean
bonobo_ui_toolbar_item_get_pack_end (BonoboUIToolbarItem *item)
{
	BonoboUIToolbarItemPrivate *priv;

	g_return_val_if_fail (item != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item), FALSE);

	priv = item->priv;
	return priv->pack_end;
}

void
bonobo_ui_toolbar_item_activate (BonoboUIToolbarItem *item)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item));

	gtk_signal_emit (GTK_OBJECT (item), signals[ACTIVATE]);
}

void
bonobo_ui_toolbar_item_set_minimum_width (BonoboUIToolbarItem *item, int minimum_width)
{
	g_return_if_fail (item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item));

	item->priv->minimum_width = minimum_width;
}
