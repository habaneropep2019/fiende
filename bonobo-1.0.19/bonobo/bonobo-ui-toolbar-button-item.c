/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-button-item.h: a toolbar button
 *
 * Author: Ettore Perazzoli
 *
 * Copyright (C) 2000 Helix Code, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "bonobo-ui-toolbar-button-item.h"
#include "bonobo-ui-icon.h"


/* Spacing between the icon and the label.  */
#define SPACING 2

/* Number of GdkStateType values */
#define NUM_STATES 5

#define PARENT_TYPE bonobo_ui_toolbar_item_get_type ()
static BonoboUIToolbarItemClass *parent_class = NULL;

struct _BonoboUIToolbarButtonItemPrivate {
	/* The icon for the button.  */
	GtkWidget *icon;

	/* The label for the button.  */
	GtkWidget *label;

	/* The box that packs the icon and the label.  It can either be an hbox
           or a vbox, depending on the style.  */
	GtkWidget *box;

	/* The widget containing the button */
	GtkButton *button_widget;
};

enum {
	CLICKED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0 };


/* Utility functions.  */

static GtkWidget *
create_pixmap_widget_from_pixbuf (GdkPixbuf *pixbuf)
{
	GtkWidget *icon;

	icon = bonobo_ui_icon_new ();
	if (!bonobo_ui_icon_set_from_pixbuf (BONOBO_UI_ICON (icon), pixbuf)) {
		gtk_widget_unref (icon);
		return NULL;
	}

	return icon;
}

static void
set_icon (BonoboUIToolbarButtonItem *button_item,
	  GdkPixbuf *pixbuf)
{
	BonoboUIToolbarButtonItemPrivate *priv;

	priv = button_item->priv;

	gtk_widget_push_style (gtk_widget_get_style (GTK_WIDGET (priv->button_widget)));

	if (priv->icon != NULL) {
		if (!(pixbuf
		      && bonobo_ui_icon_set_from_pixbuf (BONOBO_UI_ICON (priv->icon), pixbuf))) {
			gtk_widget_destroy (priv->icon);
			priv->icon = NULL;
		}
	} else {
		if (pixbuf != NULL)
			priv->icon = create_pixmap_widget_from_pixbuf (pixbuf);
	}

	gtk_widget_pop_style ();
}

static void
set_label (BonoboUIToolbarButtonItem *button_item,
	   const char *label)
{
	BonoboUIToolbarButtonItemPrivate *priv;

	priv = button_item->priv;

	/* FIXME: we should really do a string compare
	   on the label's contents perhaps before this: */
	if (priv->label != NULL)
		gtk_widget_destroy (priv->label);

	if (label != NULL)
		priv->label = gtk_label_new (label);
	else
		priv->label = NULL;
}


/* Layout.  */

static void
unparent_items (BonoboUIToolbarButtonItem *button_item)
{
	BonoboUIToolbarButtonItemPrivate *priv;

	priv = button_item->priv;

	if (priv->icon != NULL) {
		if (priv->icon->parent != NULL) {
			gtk_widget_ref (priv->icon);
			gtk_container_remove (GTK_CONTAINER (priv->icon->parent),
					      priv->icon);
		}
	}

	if (priv->label != NULL) {
		if (priv->label->parent != NULL) {
			gtk_widget_ref (priv->label);
			gtk_container_remove (GTK_CONTAINER (priv->label->parent),
					      priv->label);
		}
	}
}

static void
layout_pixmap_and_label (BonoboUIToolbarButtonItem *button_item,
			 BonoboUIToolbarItemStyle   style)
{
	BonoboUIToolbarButtonItemPrivate *priv;
	GtkWidget *button;
	gboolean   rebuild;

	priv = button_item->priv;
	button = GTK_BIN (button_item)->child;

	/* Ensure we have the right type of box */
	rebuild = FALSE;
	if (style == BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_VERTICAL) {
		if (!priv->box || !gtk_type_is_a (
			GTK_OBJECT (priv->box)->klass->type,
			GTK_TYPE_VBOX)) {
			
			unparent_items (button_item);

			if (priv->box)
				gtk_widget_destroy (priv->box);
			priv->box = gtk_vbox_new (FALSE, SPACING);
			rebuild = TRUE;
		}
	} else {
		if (!priv->box || !gtk_type_is_a (
			GTK_OBJECT (priv->box)->klass->type,
			GTK_TYPE_HBOX)) {
			
			unparent_items (button_item);

			if (priv->box)
				gtk_widget_destroy (priv->box);
			priv->box = gtk_hbox_new (FALSE, SPACING);
			rebuild = TRUE;
		}
	}

	if (rebuild) {
		gtk_container_add (GTK_CONTAINER (button), priv->box);
		gtk_widget_show (priv->box);
	}

	if (priv->icon && !priv->icon->parent)
		gtk_box_pack_start (
			GTK_BOX (priv->box), priv->icon, TRUE, TRUE, 0);

	if (priv->label && !priv->label->parent)
		gtk_box_pack_end (
			GTK_BOX (priv->box), priv->label, FALSE, TRUE, 0);

	if (priv->icon) {
		if (style == BONOBO_UI_TOOLBAR_ITEM_STYLE_TEXT_ONLY)
			gtk_widget_hide (priv->icon);
		else
			gtk_widget_show (priv->icon);
	}

	if (priv->label) {
		if (style == BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_ONLY)
			gtk_widget_hide (priv->label);
		else
			gtk_widget_show (priv->label);
	}
}


/* Callback for the GtkButton.  */

static void
button_widget_clicked_cb (GtkButton *button,
			  void *data)
{
	BonoboUIToolbarButtonItem *button_item;

	button_item = BONOBO_UI_TOOLBAR_BUTTON_ITEM (data);

	gtk_signal_emit (GTK_OBJECT (button_item), signals[CLICKED]);

	bonobo_ui_toolbar_item_activate (BONOBO_UI_TOOLBAR_ITEM (button_item));
}


/* GtkObject methods.  */

static void
impl_destroy (GtkObject *object)
{
	BonoboUIToolbarButtonItem *button_item;
	BonoboUIToolbarButtonItemPrivate *priv;

	button_item = BONOBO_UI_TOOLBAR_BUTTON_ITEM (object);
	priv = button_item->priv;

	g_free (priv);

	if (GTK_OBJECT_CLASS (parent_class)->destroy != NULL)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


/* BonoboUIToolbarItem signals.  */

static void
impl_set_style (BonoboUIToolbarItem *item,
		BonoboUIToolbarItemStyle style)
{
	BonoboUIToolbarButtonItem *button_item;

	button_item = BONOBO_UI_TOOLBAR_BUTTON_ITEM (item);

	layout_pixmap_and_label (button_item, style);

	if (BONOBO_UI_TOOLBAR_ITEM_CLASS (parent_class)->set_style != NULL)
		(* BONOBO_UI_TOOLBAR_ITEM_CLASS (parent_class)->set_style) (item, style);
}

static void
impl_set_tooltip (BonoboUIToolbarItem *item,
		  GtkTooltips         *tooltips,
		  const char          *tooltip)
{
	BonoboUIToolbarButtonItem *button_item;
	GtkButton *button;

	button_item = BONOBO_UI_TOOLBAR_BUTTON_ITEM (item);

	if (tooltip && (button = button_item->priv->button_widget))
		gtk_tooltips_set_tip (
			tooltips, GTK_WIDGET (button), tooltip, NULL);
}


/* BonoboUIToolbarButtonItem virtual methods.  */
static void
impl_set_icon  (BonoboUIToolbarButtonItem *button_item,
		GdkPixbuf                 *icon)
{
	set_icon (button_item, icon);

	layout_pixmap_and_label (
		button_item,
		bonobo_ui_toolbar_item_get_style (
			BONOBO_UI_TOOLBAR_ITEM (button_item)));
}

static void
impl_set_label (BonoboUIToolbarButtonItem *button_item,
		const char                *label)
{
	set_label (button_item, label);

	layout_pixmap_and_label (
		button_item,
		bonobo_ui_toolbar_item_get_style (
			BONOBO_UI_TOOLBAR_ITEM (button_item)));
}


/* GTK+ object initialization.  */

static void
class_init (BonoboUIToolbarButtonItemClass *button_item_class)
{
	GtkObjectClass *object_class;
	BonoboUIToolbarItemClass *item_class;

	object_class = GTK_OBJECT_CLASS (button_item_class);
	object_class->destroy = impl_destroy;

	item_class = BONOBO_UI_TOOLBAR_ITEM_CLASS (button_item_class);
	item_class->set_style = impl_set_style;
	item_class->set_tooltip = impl_set_tooltip;

	button_item_class->set_icon  = impl_set_icon;
	button_item_class->set_label = impl_set_label;

	parent_class = gtk_type_class (bonobo_ui_toolbar_item_get_type ());

	signals[CLICKED] = 
		gtk_signal_new ("clicked",
				GTK_RUN_FIRST,
				object_class->type,
				GTK_SIGNAL_OFFSET (BonoboUIToolbarButtonItemClass, clicked),
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
}

static void
init (BonoboUIToolbarButtonItem *toolbar_button_item)
{
	BonoboUIToolbarButtonItemPrivate *priv;

	priv = g_new (BonoboUIToolbarButtonItemPrivate, 1);
	priv->icon  = NULL;
	priv->label = NULL;
	priv->box   = NULL;

	toolbar_button_item->priv = priv;
}


GtkType
bonobo_ui_toolbar_button_item_get_type (void)
{
	static GtkType type = 0;

	if (type == 0) {
		static const GtkTypeInfo info = {
			"BonoboUIToolbarButtonItem",
			sizeof (BonoboUIToolbarButtonItem),
			sizeof (BonoboUIToolbarButtonItemClass),
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

void
bonobo_ui_toolbar_button_item_construct (BonoboUIToolbarButtonItem *button_item,
					 GtkButton *button_widget,
					 GdkPixbuf *pixbuf,
					 const char *label)
{
	BonoboUIToolbarButtonItemPrivate *priv;

	g_return_if_fail (button_item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_BUTTON_ITEM (button_item));
	g_return_if_fail (button_widget != NULL);
	g_return_if_fail (GTK_IS_BUTTON (button_widget));
	g_return_if_fail (GTK_BIN (button_item)->child == NULL);

	priv = button_item->priv;
	g_assert (priv->icon == NULL);
	g_assert (priv->label == NULL);

	priv->button_widget = button_widget;
	gtk_widget_show (GTK_WIDGET (button_widget));

	gtk_signal_connect_while_alive (GTK_OBJECT (button_widget), "clicked",
					GTK_SIGNAL_FUNC (button_widget_clicked_cb), button_item,
					GTK_OBJECT (button_item));

	gtk_button_set_relief (button_widget, GTK_RELIEF_NONE);

	GTK_WIDGET_UNSET_FLAGS (button_widget, GTK_CAN_FOCUS);

	gtk_container_add (GTK_CONTAINER (button_item), GTK_WIDGET (button_widget));

	set_icon  (button_item, pixbuf);
	set_label (button_item, label);

	layout_pixmap_and_label (
		button_item, bonobo_ui_toolbar_item_get_style (
			BONOBO_UI_TOOLBAR_ITEM (button_item)));
}

/**
 * bonobo_ui_toolbar_button_item_new:
 * @pixmap: 
 * @label: 
 * 
 * Create a new toolbar button item.
 * 
 * Return value: A pointer to the newly created widget.
 **/
GtkWidget *
bonobo_ui_toolbar_button_item_new (GdkPixbuf *icon,
				   const char *label)
{
	BonoboUIToolbarButtonItem *button_item;
	GtkWidget *button_widget;

	button_item = gtk_type_new (bonobo_ui_toolbar_button_item_get_type ());

	gtk_widget_push_visual (gdk_rgb_get_visual ());
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());

	button_widget = gtk_button_new ();

	gtk_widget_pop_visual ();
	gtk_widget_pop_colormap ();
	
	bonobo_ui_toolbar_button_item_construct (button_item, GTK_BUTTON (button_widget), icon, label);

	return GTK_WIDGET (button_item);
}


void
bonobo_ui_toolbar_button_item_set_icon (BonoboUIToolbarButtonItem *button_item,
					GdkPixbuf *icon)
{
	BonoboUIToolbarButtonItemClass *klass;

	g_return_if_fail (button_item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_BUTTON_ITEM (button_item));

	klass = BONOBO_UI_TOOLBAR_BUTTON_ITEM_CLASS (
		((GtkObject *)button_item)->klass);

	if (klass->set_icon)
		klass->set_icon (button_item, icon);
}

void
bonobo_ui_toolbar_button_item_set_label (BonoboUIToolbarButtonItem *button_item,
				      const char *label)
{
	BonoboUIToolbarButtonItemClass *klass;

	g_return_if_fail (button_item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_BUTTON_ITEM (button_item));

	klass = BONOBO_UI_TOOLBAR_BUTTON_ITEM_CLASS (
		((GtkObject *)button_item)->klass);

	if (klass->set_label)
		klass->set_label (button_item, label);
}


GtkButton *
bonobo_ui_toolbar_button_item_get_button_widget (BonoboUIToolbarButtonItem *button_item)
{
	g_return_val_if_fail (button_item != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR_BUTTON_ITEM (button_item), NULL);

	return GTK_BUTTON (GTK_BIN (button_item)->child);
}
