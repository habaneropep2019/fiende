/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar-separator-item.h
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
#include "bonobo-ui-toolbar-separator-item.h"


#define PARENT_TYPE bonobo_ui_toolbar_item_get_type ()
static BonoboUIToolbarItemClass *parent_class = NULL;


#define BORDER_WIDTH        2

#define SPACE_LINE_DIVISION 10
#define SPACE_LINE_START    3
#define SPACE_LINE_END      7


/* GtkWidget methods.  */

static void
impl_size_request (GtkWidget *widget,
		   GtkRequisition *requisition)
{
	int border_width;

	border_width = GTK_CONTAINER (widget)->border_width;

	requisition->width  = 2 * border_width + widget->style->klass->xthickness;
	requisition->height = 2 * border_width + widget->style->klass->ythickness;
}

static void
impl_draw (GtkWidget *widget,
	   GdkRectangle *area)
{
	BonoboUIToolbarItem *item;
	const GtkAllocation *allocation;
	GtkOrientation orientation;
	int border_width;

	item = BONOBO_UI_TOOLBAR_ITEM (widget);

	allocation = &widget->allocation;
	border_width = GTK_CONTAINER (widget)->border_width;

	orientation = bonobo_ui_toolbar_item_get_orientation (item);

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		gtk_paint_vline (widget->style, widget->window,
				 GTK_WIDGET_STATE (widget), area, widget,
				 "toolbar",
				 allocation->y + allocation->height * SPACE_LINE_START / SPACE_LINE_DIVISION,
				 allocation->y + allocation->height * SPACE_LINE_END / SPACE_LINE_DIVISION,
				 allocation->x + border_width);
	else
		gtk_paint_hline (widget->style, widget->window,
				 GTK_WIDGET_STATE (widget), area, widget,
				 "toolbar",
				 allocation->x + allocation->width * SPACE_LINE_START / SPACE_LINE_DIVISION,
				 allocation->x + allocation->width * SPACE_LINE_END / SPACE_LINE_DIVISION,
				 allocation->y + border_width);
}

static int
impl_expose_event (GtkWidget *widget,
		   GdkEventExpose *expose)
{
	gtk_widget_draw (widget, &expose->area);

	return FALSE;
}


static void
class_init (BonoboUIToolbarSeparatorItemClass *separator_item_class)
{
	GtkWidgetClass *widget_class;

	widget_class = GTK_WIDGET_CLASS (separator_item_class);
	widget_class->size_request = impl_size_request;
	widget_class->draw         = impl_draw;
	widget_class->expose_event = impl_expose_event;

	parent_class = gtk_type_class (bonobo_ui_toolbar_item_get_type ());
}

static void
init (BonoboUIToolbarSeparatorItem *toolbar_separator_item)
{
	gtk_container_set_border_width (GTK_CONTAINER (toolbar_separator_item), BORDER_WIDTH);
}


GtkType
bonobo_ui_toolbar_separator_item_get_type (void)
{
	static GtkType type = 0;

	if (type == 0) {
		static const GtkTypeInfo info = {
			"BonoboUIToolbarSeparatorItem",
			sizeof (BonoboUIToolbarSeparatorItem),
			sizeof (BonoboUIToolbarSeparatorItemClass),
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
bonobo_ui_toolbar_separator_item_new (void)
{
	BonoboUIToolbarSeparatorItem *new;

	new = gtk_type_new (bonobo_ui_toolbar_separator_item_get_type ());

	return GTK_WIDGET (new);
}
