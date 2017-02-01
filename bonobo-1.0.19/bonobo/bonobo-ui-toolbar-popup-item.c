/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-popup-item.h
 *
 * Author:
 *    Ettore Perazzoli
 *
 * Copyright (C) 2000 Helix Code, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>

#include "bonobo-ui-toolbar-popup-item.h"


#define PARENT_TYPE bonobo_ui_toolbar_toggle_button_item_get_type ()
static BonoboUIToolbarToggleButtonItemClass *parent_class = NULL;

static GdkPixbuf *right_arrow_pixbuf = NULL;
static GdkPixbuf *down_arrow_pixbuf = NULL;

static const char *right_arrow_xpm_data[] = {
	"8 10 2 1",
	" 	c none",
	".	c #000000000000",
	"        ",
	" .      ",
	" ..     ",
	" ...    ",
	" ....   ",
	" .....  ",
	" ....   ",
	" ...    ",
	" ..     ",
	" .      ",
	"        "
};

static const char *down_arrow_xpm_data[] = {
	"11 7 2 1",
	" 	c none",
	".	c #000000000000",
	"           ",
	" ......... ",
	"  .......  ",
	"   .....   ",
	"    ...    ",
	"     .     ",
	" 	    ",
};


/* Utility functions.  */

static void
create_arrow_pixbufs (void)
{
	g_assert (right_arrow_pixbuf == NULL);
	right_arrow_pixbuf = gdk_pixbuf_new_from_xpm_data (right_arrow_xpm_data);

	g_assert (down_arrow_pixbuf == NULL);
	down_arrow_pixbuf = gdk_pixbuf_new_from_xpm_data (down_arrow_xpm_data);
}

static GdkPixbuf *
get_icon_for_orientation (BonoboUIToolbarPopupItem *popup_item)
{
	GtkOrientation orientation;

	orientation = bonobo_ui_toolbar_item_get_orientation (BONOBO_UI_TOOLBAR_ITEM (popup_item));

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		return right_arrow_pixbuf;
	else
		return down_arrow_pixbuf;
}


static void
impl_set_orientation (BonoboUIToolbarItem *item,
		      GtkOrientation orientation)
{
	BonoboUIToolbarPopupItem *popup_item;
	GdkPixbuf *icon;

	if (BONOBO_UI_TOOLBAR_ITEM_CLASS (parent_class)->set_orientation != NULL)
		(* BONOBO_UI_TOOLBAR_ITEM_CLASS (parent_class)->set_orientation) (item, orientation);

	popup_item = BONOBO_UI_TOOLBAR_POPUP_ITEM (item);

	icon = get_icon_for_orientation (popup_item);
	bonobo_ui_toolbar_button_item_set_icon (BONOBO_UI_TOOLBAR_BUTTON_ITEM (item), icon);
}


static void
class_init (BonoboUIToolbarPopupItemClass *popup_item_class)
{
	BonoboUIToolbarItemClass *toolbar_item_class;

	toolbar_item_class = BONOBO_UI_TOOLBAR_ITEM_CLASS (popup_item_class);
	toolbar_item_class->set_orientation = impl_set_orientation;

	parent_class = gtk_type_class (PARENT_TYPE);

	create_arrow_pixbufs ();
}

static void
init (BonoboUIToolbarPopupItem *toolbar_popup_item)
{
	/* Nothing to do here.  */
}


GtkType
bonobo_ui_toolbar_popup_item_get_type (void)
{
	static GtkType type = 0;

	if (type == 0) {
		static const GtkTypeInfo info = {
			"BonoboUIToolbarPopupItem",
			sizeof (BonoboUIToolbarPopupItem),
			sizeof (BonoboUIToolbarPopupItemClass),
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
bonobo_ui_toolbar_popup_item_construct (BonoboUIToolbarPopupItem *popup_item)
{
	GdkPixbuf *icon;

	g_return_if_fail (popup_item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_POPUP_ITEM (popup_item));

	icon = get_icon_for_orientation (popup_item);

	bonobo_ui_toolbar_toggle_button_item_construct (BONOBO_UI_TOOLBAR_TOGGLE_BUTTON_ITEM (popup_item), icon, NULL);
}

GtkWidget *
bonobo_ui_toolbar_popup_item_new (void)
{
	BonoboUIToolbarPopupItem *popup_item;

	popup_item = gtk_type_new (bonobo_ui_toolbar_popup_item_get_type ());

	bonobo_ui_toolbar_popup_item_construct (popup_item);

	return GTK_WIDGET (popup_item);
}
