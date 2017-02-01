/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/**
 * bonobo-ui-toolbar.h
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

#include "bonobo-ui-toolbar-item.h"
#include "bonobo-ui-toolbar-popup-item.h"

#include "bonobo-ui-toolbar.h"


#define PARENT_TYPE gtk_container_get_type ()
static GtkContainerClass *parent_class = NULL;

enum {
	ARG_0,
	ARG_ORIENTATION,
	ARG_IS_FLOATING,
	ARG_PREFERRED_WIDTH,
	ARG_PREFERRED_HEIGHT
};

struct _BonoboUIToolbarPrivate {
	/* The orientation of this toolbar.  */
	GtkOrientation orientation;

	/* Is the toolbar currently floating */
	gboolean is_floating;

	/* The style of this toolbar.  */
	BonoboUIToolbarStyle style;

	/* Styles to use in different orientations */
	BonoboUIToolbarStyle hstyle;
	BonoboUIToolbarStyle vstyle;

	/* Sizes of the toolbar.  This is actually the height for
           horizontal toolbars and the width for vertical toolbars.  */
	int max_width, max_height;
	int total_width, total_height;

	/* position of left edge of left-most pack-end item */
	int end_position;

	/* List of all the items in the toolbar.  Both the ones that have been
           unparented because they don't fit, and the ones that are visible.
           The BonoboUIToolbarPopupItem is not here though.  */
	GList *items;

	/* Pointer to the first element in the `items' list that doesn't fit in
           the available space.  This is updated at size_allocate.  */
	GList *first_not_fitting_item;

	/* The pop-up button.  When clicked, it pops up a window with all the
           items that don't fit.  */
	BonoboUIToolbarItem *popup_item;

	/* The window we pop-up when the pop-up item is clicked.  */
	GtkWidget *popup_window;

	/* The vbox within the pop-up window.  */
	GtkWidget *popup_window_vbox;

	/* Whether we have moved items to the pop-up window.  This is to
           prevent the size_allocation code to incorrectly hide the pop-up
           button in that case.  */
	gboolean items_moved_to_popup_window;

	GtkTooltips *tooltips;
};

enum {
	SET_ORIENTATION,
	STYLE_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };


/* Width of the pop-up window.  */
#define POPUP_WINDOW_WIDTH 200


/* Utility functions.  */

static void
parentize_widget (BonoboUIToolbar *toolbar,
		  GtkWidget *widget)
{
	g_assert (widget->parent == NULL);

	/* The following is done according to the Bible, widget_system.txt, IV, 1.  */

	gtk_widget_set_parent (widget, GTK_WIDGET (toolbar));

	if (GTK_WIDGET_REALIZED (toolbar) && ! GTK_WIDGET_REALIZED (widget))
		gtk_widget_realize (widget);

	if (GTK_WIDGET_MAPPED (toolbar) && ! GTK_WIDGET_MAPPED (widget) &&  GTK_WIDGET_VISIBLE (widget))
		gtk_widget_map (widget);

	if (GTK_WIDGET_MAPPED (widget))
		gtk_widget_queue_resize (GTK_WIDGET (toolbar));
}

static void
set_attributes_on_child (BonoboUIToolbarItem *item,
			 GtkOrientation orientation,
			 BonoboUIToolbarStyle style)
{
	bonobo_ui_toolbar_item_set_orientation (item, orientation);

	switch (style) {
	case BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT:
		if (! bonobo_ui_toolbar_item_get_want_label (item))
			bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_ONLY);
		else if (orientation == GTK_ORIENTATION_HORIZONTAL)
			bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_HORIZONTAL);
		else
			bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_VERTICAL);
		break;

	case BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT:
		if (orientation == GTK_ORIENTATION_VERTICAL)
			bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_HORIZONTAL);
		else
			bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_AND_TEXT_VERTICAL);
		break;
	case BONOBO_UI_TOOLBAR_STYLE_ICONS_ONLY:
		bonobo_ui_toolbar_item_set_style (item, BONOBO_UI_TOOLBAR_ITEM_STYLE_ICON_ONLY);
		break;
	default:
		g_assert_not_reached ();
	}
}


/* Callbacks to do widget housekeeping.  */

static void
item_destroy_cb (GtkObject *object,
		 void *data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GtkWidget *widget;

	toolbar = BONOBO_UI_TOOLBAR (data);
	priv = toolbar->priv;

	widget = GTK_WIDGET (object);
	priv->items = g_list_remove (priv->items, object);
}

static void
item_activate_cb (BonoboUIToolbarItem *item,
		  void *data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;

	toolbar = BONOBO_UI_TOOLBAR (data);
	priv = toolbar->priv;

	bonobo_ui_toolbar_toggle_button_item_set_active (
		BONOBO_UI_TOOLBAR_TOGGLE_BUTTON_ITEM (priv->popup_item), FALSE);
}

static void
item_set_want_label_cb (BonoboUIToolbarItem *item,
			gboolean want_label,
			void *data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;

	toolbar = BONOBO_UI_TOOLBAR (data);
	priv = toolbar->priv;

	set_attributes_on_child (item, priv->orientation, priv->style);

	gtk_widget_queue_resize (GTK_WIDGET (toolbar));
}


/* The pop-up window foo.  */

/* Return TRUE if there are actually any items in the pop-up menu.  */
static void
create_popup_window (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GtkWidget *hbox;
	GList *p;
	int row_width;

	priv = toolbar->priv;

	row_width = 0;
	hbox = NULL;

	for (p = priv->first_not_fitting_item; p != NULL; p = p->next) {
		GtkRequisition item_requisition;
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);

		if (! GTK_WIDGET_VISIBLE (item_widget) ||
			bonobo_ui_toolbar_item_get_pack_end (BONOBO_UI_TOOLBAR_ITEM (item_widget)))

			continue;

		if (item_widget->parent != NULL)
			gtk_container_remove (GTK_CONTAINER (item_widget->parent), item_widget);

		gtk_widget_get_child_requisition (item_widget, &item_requisition);

		set_attributes_on_child (BONOBO_UI_TOOLBAR_ITEM (item_widget),
					 GTK_ORIENTATION_HORIZONTAL,
					 priv->style);

		if (hbox == NULL
		    || (row_width > 0 && item_requisition.width + row_width > POPUP_WINDOW_WIDTH)) {
			hbox = gtk_hbox_new (FALSE, 0);
			gtk_box_pack_start (GTK_BOX (priv->popup_window_vbox), hbox, FALSE, TRUE, 0);
			gtk_widget_show (hbox);
			row_width = 0;
		}

		gtk_box_pack_start (GTK_BOX (hbox), item_widget, FALSE, TRUE, 0);

		row_width += item_requisition.width;
	}
}

static void
hide_popup_window (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GList *p;

	priv = toolbar->priv;

	gdk_pointer_ungrab (GDK_CURRENT_TIME);

	gtk_grab_remove (priv->popup_window);
	gtk_widget_hide (priv->popup_window);

	priv->items_moved_to_popup_window = FALSE;

	/* Reset the attributes on all the widgets that were moved to the
           window and move them back to the toolbar.  */
	for (p = priv->items; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);
		if (item_widget->parent != GTK_WIDGET (toolbar)) {
			set_attributes_on_child (BONOBO_UI_TOOLBAR_ITEM (item_widget),
						 priv->orientation, priv->style);
			gtk_container_remove (GTK_CONTAINER (item_widget->parent), item_widget);
			parentize_widget (toolbar, item_widget);
		}
	}

	gtk_widget_queue_resize (GTK_WIDGET (toolbar));
}

static void
popup_window_button_release_cb (GtkWidget *widget,
				GdkEventButton *event,
				void *data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;

	toolbar = BONOBO_UI_TOOLBAR (data);
	priv = toolbar->priv;

	bonobo_ui_toolbar_toggle_button_item_set_active
		(BONOBO_UI_TOOLBAR_TOGGLE_BUTTON_ITEM (priv->popup_item), FALSE);
}

static void
popup_window_map_cb (GtkWidget *widget,
		     void *data)
{
	BonoboUIToolbar *toolbar;

	toolbar = BONOBO_UI_TOOLBAR (data);

	if (gdk_pointer_grab (widget->window, TRUE,
			      (GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK
			       | GDK_ENTER_NOTIFY_MASK
			       | GDK_LEAVE_NOTIFY_MASK
			       | GDK_POINTER_MOTION_MASK),
			      NULL, NULL, GDK_CURRENT_TIME) != 0) {
		g_warning ("Toolbar pop-up pointer grab failed.");
		return;
	}

	gtk_grab_add (widget);
}

static void
show_popup_window (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	const GtkAllocation *toolbar_allocation;
	GtkRequisition requisition;
	int x, y;

	priv = toolbar->priv;

	priv->items_moved_to_popup_window = TRUE;

	create_popup_window (toolbar);

	gdk_window_get_origin (GTK_WIDGET (toolbar)->window, &x, &y);

	toolbar_allocation = & GTK_WIDGET (toolbar)->allocation;

	gtk_widget_size_request (priv->popup_window, &requisition);

	if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
		x += toolbar_allocation->x + toolbar_allocation->width;
		if (x >= gdk_screen_width () - requisition.width)
			y += toolbar_allocation->height;
	} else {
		y += toolbar_allocation->y + toolbar_allocation->height;
		if (y >= gdk_screen_height () - requisition.height)
			x += toolbar_allocation->width;
	}

	x = CLAMP (x, 0, MAX (0, gdk_screen_width () - requisition.width));
	y = CLAMP (y, 0, MAX (0, gdk_screen_height () - requisition.height));

	gtk_widget_set_uposition (GTK_WIDGET (priv->popup_window), x, y);

	gtk_signal_connect (GTK_OBJECT (priv->popup_window), "map",
			    GTK_SIGNAL_FUNC (popup_window_map_cb), toolbar);

	gtk_widget_show (priv->popup_window);
}

static void
popup_item_toggled_cb (BonoboUIToolbarToggleButtonItem *toggle_button_item,
		       void *data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	gboolean active;

	toolbar = BONOBO_UI_TOOLBAR (data);
	priv = toolbar->priv;

	active = bonobo_ui_toolbar_toggle_button_item_get_active (toggle_button_item);

	if (active)
		show_popup_window (toolbar);
	else 
		hide_popup_window (toolbar);
}


/* Layout handling.  */

static int
get_popup_item_size (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GtkRequisition requisition;

	priv = toolbar->priv;

	gtk_widget_get_child_requisition (
		GTK_WIDGET (priv->popup_item), &requisition);

	if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
		return requisition.width;
	else
		return requisition.height;
}

/* Update the various sizes.  This is performed during ::size_request.  */
static void
update_sizes (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	int max_width, max_height;
	int total_width, total_height;
	GList *p;

	priv = toolbar->priv;

	max_width = max_height = total_width = total_height = 0;

	for (p = priv->items; p != NULL; p = p->next) {
		GtkWidget *item_widget;
		GtkRequisition item_requisition;

		item_widget = GTK_WIDGET (p->data);
		if (! GTK_WIDGET_VISIBLE (item_widget) || item_widget->parent != GTK_WIDGET (toolbar))
			continue;

		gtk_widget_size_request (item_widget, &item_requisition);

		max_width     = MAX (max_width,  item_requisition.width);
		total_width  += item_requisition.width;
		max_height    = MAX (max_height, item_requisition.height);
		total_height += item_requisition.height;
	}

	priv->max_width = max_width;
	priv->total_width = total_width;
	priv->max_height = max_height;
	priv->total_height = total_height;
}

static void
allocate_popup_item (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GtkRequisition popup_item_requisition;
	GtkAllocation popup_item_allocation;
	GtkAllocation *toolbar_allocation;
	int border_width;

	priv = toolbar->priv;

	/* FIXME what if there is not enough space?  */

	toolbar_allocation = & GTK_WIDGET (toolbar)->allocation;

	border_width = GTK_CONTAINER (toolbar)->border_width;

	gtk_widget_get_child_requisition (
		GTK_WIDGET (priv->popup_item), &popup_item_requisition);

	popup_item_allocation.x = toolbar_allocation->x;
	popup_item_allocation.y = toolbar_allocation->y;

	if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
		popup_item_allocation.x      = priv->end_position - popup_item_requisition.width - border_width;
		popup_item_allocation.y      += border_width;
		popup_item_allocation.width  = popup_item_requisition.width;
		popup_item_allocation.height = toolbar_allocation->height - 2 * border_width;
	} else {
		popup_item_allocation.x      += border_width;
		popup_item_allocation.y      = priv->end_position - popup_item_requisition.height - border_width;
		popup_item_allocation.width  = toolbar_allocation->width - 2 * border_width;
		popup_item_allocation.height = popup_item_requisition.height;
	}

	gtk_widget_size_allocate (GTK_WIDGET (priv->popup_item), &popup_item_allocation);
}

static void
setup_popup_item (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GList *p;

	priv = toolbar->priv;

	if (priv->items_moved_to_popup_window) {
		gtk_widget_show (GTK_WIDGET (priv->popup_item));
		allocate_popup_item (toolbar);
		return;
	}

	for (p = priv->first_not_fitting_item; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);

		if (GTK_WIDGET_VISIBLE (item_widget)) {
			gtk_widget_show (GTK_WIDGET (priv->popup_item));
			allocate_popup_item (toolbar);
			return;
		}
	}

	gtk_widget_hide (GTK_WIDGET (priv->popup_item));
}

/*
 * This is a dirty hack.  We cannot hide the items with gtk_widget_hide ()
 * because we want to let the user be in control of the physical hidden/shown
 * state, so we just move the widget to a non-visible area.
 */
static void
hide_not_fitting_items (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	const GtkAllocation *allocation;
	GtkAllocation child_allocation;
	GList *p;

	priv = toolbar->priv;

	allocation = & GTK_WIDGET (toolbar)->allocation;

	child_allocation.x      = 40000;
	child_allocation.y      = 40000;
	child_allocation.width  = 1;
	child_allocation.height = 1;

	for (p = priv->first_not_fitting_item; p != NULL; p = p->next) {
		if (bonobo_ui_toolbar_item_get_pack_end (BONOBO_UI_TOOLBAR_ITEM (p->data)))
			continue;
		gtk_widget_size_allocate (GTK_WIDGET (p->data), &child_allocation);
	}
}

static void
size_allocate_helper (BonoboUIToolbar *toolbar,
		      const GtkAllocation *allocation)
{
	BonoboUIToolbarPrivate *priv;
	GtkAllocation child_allocation;
	BonoboUIToolbarItem *item;
	GtkRequisition child_requisition;
	int border_width;
	int space_required;
	int available_space;
	int extra_space;
	int num_expandable_items;
	int popup_item_size;
	gboolean first_expandable;
	GList *p;

	GTK_WIDGET (toolbar)->allocation = *allocation;

	priv = toolbar->priv;

	border_width = GTK_CONTAINER (toolbar)->border_width;
	popup_item_size = get_popup_item_size (toolbar);

	if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
		available_space = MAX ((int) allocation->width - 2 * border_width, popup_item_size);
	else
		available_space = MAX ((int) allocation->height - 2 * border_width, popup_item_size);

	child_allocation.x = allocation->x + border_width;
	child_allocation.y = allocation->y + border_width;

	/* 
	 * if there is exactly one toolbar item, handle it specially, by giving it all of the available space,
	 * even if it doesn't fit, since we never want everything in the pop-up.
	 */	 
	if (priv->items != NULL && priv->items->next == NULL) {
		item = BONOBO_UI_TOOLBAR_ITEM (priv->items->data);		
		gtk_widget_get_child_requisition (GTK_WIDGET (item), &child_requisition);
		child_allocation.width = child_requisition.width;
		child_allocation.height = child_requisition.height;

		if (bonobo_ui_toolbar_item_get_expandable (item)) {
			if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
				child_allocation.width = available_space;
			else
				child_allocation.height = available_space;
		
		} 
		gtk_widget_size_allocate (GTK_WIDGET (item), &child_allocation);		
		
		return;
	}

	/* first, make a pass through the items to layout the ones that are packed on the right */
	priv->end_position = allocation->x + available_space;
	for (p = g_list_last (priv->items); p != NULL; p = p->prev) {

		item = BONOBO_UI_TOOLBAR_ITEM (p->data);
		if (! bonobo_ui_toolbar_item_get_pack_end (item))
			continue;

		gtk_widget_get_child_requisition (GTK_WIDGET (item), &child_requisition);

		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
			available_space -= child_requisition.width;
			priv->end_position -= child_requisition.width;
			
			child_allocation.x = priv->end_position;
			child_allocation.width = child_requisition.width;
			child_allocation.height = priv->max_height;
		} else {
			available_space -= child_requisition.height;
			priv->end_position -= child_requisition.height;
			
			child_allocation.y = priv->end_position;
			child_allocation.height = child_requisition.height;
			child_allocation.width = priv->max_width;
		}
		
		gtk_widget_size_allocate (GTK_WIDGET (item), &child_allocation);
	}
	
	/* make a pass through the items to determine how many fit */	
	space_required = 0;
	num_expandable_items = 0;

	child_allocation.x = allocation->x + border_width;
	child_allocation.y = allocation->y + border_width;

	for (p = priv->items; p != NULL; p = p->next) {
		int item_size;

		item = BONOBO_UI_TOOLBAR_ITEM (p->data);
		if (! GTK_WIDGET_VISIBLE (item) || GTK_WIDGET (item)->parent != GTK_WIDGET (toolbar) ||
			bonobo_ui_toolbar_item_get_pack_end (item))
			continue;

		gtk_widget_get_child_requisition (GTK_WIDGET (item), &child_requisition);

		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
			item_size = child_requisition.width;
		else
			item_size = child_requisition.height;

		if (p->next == NULL) {
			if (space_required + item_size > available_space)
				break;
		} else {
			if (space_required + item_size > available_space - popup_item_size)
				break;
		}

		space_required += item_size;

		if (bonobo_ui_toolbar_item_get_expandable (item))
			num_expandable_items ++;
	}

	priv->first_not_fitting_item = p;

	/* determine the amount of space available for expansion */
	if (priv->first_not_fitting_item != NULL) {
		extra_space = 0;
	} else {
		extra_space = available_space - space_required;
		if (priv->first_not_fitting_item != NULL)
			extra_space -= popup_item_size;
	}

	first_expandable = FALSE;

	for (p = priv->items; p != priv->first_not_fitting_item; p = p->next) {
		BonoboUIToolbarItem *item;
		GtkRequisition child_requisition;
		int expansion_amount;

		item = BONOBO_UI_TOOLBAR_ITEM (p->data);
		if (! GTK_WIDGET_VISIBLE (item) || GTK_WIDGET (item)->parent != GTK_WIDGET (toolbar) ||
			bonobo_ui_toolbar_item_get_pack_end (item))
			continue;

		gtk_widget_get_child_requisition (GTK_WIDGET (item), &child_requisition);

		if (! bonobo_ui_toolbar_item_get_expandable (item)) {
			expansion_amount = 0;
		} else {
			g_assert (num_expandable_items != 0);

			expansion_amount = extra_space / num_expandable_items;
			if (first_expandable) {
				expansion_amount += extra_space % num_expandable_items;
				first_expandable = FALSE;
			}
		}

		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
			child_allocation.width  = child_requisition.width + expansion_amount;
			child_allocation.height = priv->max_height;
		} else {
			child_allocation.width  = priv->max_width;
			child_allocation.height = child_requisition.height + expansion_amount;
		}

		gtk_widget_size_allocate (GTK_WIDGET (item), &child_allocation);

		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL)
			child_allocation.x += child_allocation.width;
		else
			child_allocation.y += child_allocation.height;
	}

	hide_not_fitting_items (toolbar);
	setup_popup_item (toolbar);
}


/* GtkObject methods.  */

static void
impl_destroy (GtkObject *object)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GList *p, *next;

	toolbar = BONOBO_UI_TOOLBAR (object);
	priv = toolbar->priv;

	for (p = priv->items; p != NULL; p = next) {
		GtkWidget *item_widget;

		next = p->next;
		item_widget = GTK_WIDGET (p->data);
		if (item_widget->parent == NULL)
			gtk_widget_destroy (item_widget);
	}

	if (GTK_WIDGET (priv->popup_item)->parent == NULL)
		gtk_widget_destroy (GTK_WIDGET (priv->popup_item));

	if (priv->popup_window != NULL)
		gtk_widget_destroy (priv->popup_window);
	priv->popup_window = NULL;

	gtk_object_unref (GTK_OBJECT (priv->tooltips));
	priv->tooltips = NULL;

	if (GTK_OBJECT_CLASS (parent_class)->destroy != NULL)
		GTK_OBJECT_CLASS (parent_class)->destroy (object);

}

static void
impl_finalize (GtkObject *object)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;

	toolbar = BONOBO_UI_TOOLBAR (object);
	priv = toolbar->priv;

	g_list_free (priv->items);
	priv->items = NULL;
	
	g_free (priv);

	if (GTK_OBJECT_CLASS (parent_class)->finalize != NULL)
		GTK_OBJECT_CLASS (parent_class)->finalize (object);
}


/* GtkWidget methods.  */

static void
impl_size_request (GtkWidget *widget,
		   GtkRequisition *requisition)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	int border_width;

	toolbar = BONOBO_UI_TOOLBAR (widget);
	priv = toolbar->priv;

	g_assert (priv->popup_item != NULL);

	update_sizes (toolbar);

	border_width = GTK_CONTAINER (toolbar)->border_width;

	if (priv->is_floating) {
		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
			requisition->width  = priv->total_width;
			requisition->height = priv->max_height;
		} else {
			requisition->width  = priv->max_width;
			requisition->height = priv->total_height;
		}
	} else {
		GtkRequisition popup_item_requisition;

		gtk_widget_size_request (GTK_WIDGET (priv->popup_item), &popup_item_requisition);

		if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
			requisition->width  = popup_item_requisition.width;
			requisition->height = MAX (popup_item_requisition.height, priv->max_height);
		} else {
			requisition->width  = MAX (popup_item_requisition.width,  priv->max_width);
			requisition->height = popup_item_requisition.height;
		}
	}

	requisition->width  += 2 * border_width;
	requisition->height += 2 * border_width;
}

static void
impl_size_allocate (GtkWidget *widget,
		    GtkAllocation *allocation)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;

	toolbar = BONOBO_UI_TOOLBAR (widget);
	priv = toolbar->priv;

	size_allocate_helper (toolbar, allocation);
}

static void
impl_map (GtkWidget *widget)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GList *p;

	toolbar = BONOBO_UI_TOOLBAR (widget);
	priv = toolbar->priv;

	GTK_WIDGET_SET_FLAGS (toolbar, GTK_MAPPED);

	for (p = priv->items; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);
		if (item_widget->parent != GTK_WIDGET (toolbar))
			continue;

		if (GTK_WIDGET_VISIBLE (item_widget) && ! GTK_WIDGET_MAPPED (item_widget))
			gtk_widget_map (item_widget);
	}

	if (GTK_WIDGET_VISIBLE (priv->popup_item) && ! GTK_WIDGET_MAPPED (priv->popup_item))
		gtk_widget_map (GTK_WIDGET (priv->popup_item));
}

static void
impl_unmap (GtkWidget *widget)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GList *p;

	toolbar = BONOBO_UI_TOOLBAR (widget);
	priv = toolbar->priv;

	for (p = priv->items; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);
		if (item_widget->parent != GTK_WIDGET (toolbar))
			continue;

		if (GTK_WIDGET_VISIBLE (item_widget) && GTK_WIDGET_MAPPED (item_widget))
			gtk_widget_unmap (item_widget);
	}

	if (GTK_WIDGET_VISIBLE (priv->popup_item) && GTK_WIDGET_MAPPED (priv->popup_item))
		gtk_widget_unmap (GTK_WIDGET (priv->popup_item));
}

static void
impl_draw (GtkWidget *widget,
	   GdkRectangle *area)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GdkRectangle item_area;
	GList *p;

	if (! GTK_WIDGET_DRAWABLE (widget))
		return;

	toolbar = BONOBO_UI_TOOLBAR (widget);
	priv = toolbar->priv;

	for (p = priv->items; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);
		if (item_widget->parent != GTK_WIDGET (toolbar))
			continue;

		if (gtk_widget_intersect (item_widget, area, &item_area))
			gtk_widget_draw (item_widget, &item_area);
	}

	if (gtk_widget_intersect (GTK_WIDGET (priv->popup_item), area, &item_area))
		gtk_widget_draw (GTK_WIDGET (priv->popup_item), &item_area);
}

static int
impl_expose_event (GtkWidget *widget,
		   GdkEventExpose *event)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GdkEventExpose item_event;
	GList *p;

	if (! GTK_WIDGET_DRAWABLE (widget))
		return FALSE;

	toolbar = BONOBO_UI_TOOLBAR (widget);
	priv = toolbar->priv;

	item_event = *event;

	for (p = priv->items; p != NULL; p = p->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (p->data);
		if (item_widget->parent != GTK_WIDGET (toolbar))
			continue;

		if (! GTK_WIDGET_NO_WINDOW (item_widget))
			continue;

		if (gtk_widget_intersect (item_widget, &event->area, &item_event.area))
			gtk_widget_event (item_widget, (GdkEvent *) &item_event);
	}

	if (gtk_widget_intersect (GTK_WIDGET (priv->popup_item), &event->area, &item_event.area))
		gtk_widget_event (GTK_WIDGET (priv->popup_item), (GdkEvent *) &item_event);

	return FALSE;
}


/* GtkContainer methods.  */

static void
impl_remove (GtkContainer *container,
	     GtkWidget    *child)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;

	toolbar = BONOBO_UI_TOOLBAR (container);
	priv = toolbar->priv;

	if (child == GTK_WIDGET (priv->popup_item))
		priv->popup_item = NULL;

	gtk_widget_unparent (child);

	gtk_widget_queue_resize (GTK_WIDGET (container));
}

static void
impl_forall (GtkContainer *container,
	     gboolean include_internals,
	     GtkCallback callback,
	     void *callback_data)
{
	BonoboUIToolbar *toolbar;
	BonoboUIToolbarPrivate *priv;
	GList *p;

	toolbar = BONOBO_UI_TOOLBAR (container);
	priv = toolbar->priv;

	p = priv->items;
	while (p != NULL) {
		GtkWidget *child;
		GList *pnext;

		pnext = p->next;

		child = GTK_WIDGET (p->data);
		if (child->parent == GTK_WIDGET (toolbar))
			(* callback) (child, callback_data);

		p = pnext;
	}

	if (priv->popup_item)
		(* callback) (GTK_WIDGET (priv->popup_item),
			      callback_data);
}


/* BonoboUIToolbar signals.  */

static void
impl_set_orientation (BonoboUIToolbar *toolbar,
		      GtkOrientation orientation)
{
	BonoboUIToolbarPrivate *priv;
	GList *p;

	priv = toolbar->priv;

	if (orientation == priv->orientation)
		return;

	priv->orientation = orientation;

	for (p = priv->items; p != NULL; p = p->next) {
		BonoboUIToolbarItem *item;

		item = BONOBO_UI_TOOLBAR_ITEM (p->data);
		set_attributes_on_child (item, orientation, priv->style);
	}

	bonobo_ui_toolbar_item_set_orientation (
		BONOBO_UI_TOOLBAR_ITEM (priv->popup_item), orientation);

	gtk_widget_queue_resize (GTK_WIDGET (toolbar));
}

static void
impl_style_changed (BonoboUIToolbar *toolbar)
{
	GList *p;
	BonoboUIToolbarStyle style;
	BonoboUIToolbarPrivate *priv;

	priv = toolbar->priv;

	style = (priv->orientation == GTK_ORIENTATION_HORIZONTAL) ? priv->hstyle : priv->vstyle;

	if (style == priv->style)
		return;

	priv->style = style;

	for (p = priv->items; p != NULL; p = p->next) {
		BonoboUIToolbarItem *item;

		item = BONOBO_UI_TOOLBAR_ITEM (p->data);
		set_attributes_on_child (item, priv->orientation, style);
	}

	gtk_widget_queue_resize (GTK_WIDGET (toolbar));
}

static void
impl_get_arg (GtkObject *obj,
	      GtkArg *arg,
	      guint arg_id)
{
	BonoboUIToolbar *toolbar = BONOBO_UI_TOOLBAR (obj);
	BonoboUIToolbarPrivate *priv = toolbar->priv;

	switch (arg_id) {
	case ARG_ORIENTATION:
		GTK_VALUE_UINT (*arg) = bonobo_ui_toolbar_get_orientation (toolbar);
		break;
	case ARG_IS_FLOATING:
		GTK_VALUE_BOOL (*arg) = priv->is_floating;
		break;
	case ARG_PREFERRED_WIDTH:
		update_sizes (toolbar);
		if (bonobo_ui_toolbar_get_orientation (toolbar) == GTK_ORIENTATION_HORIZONTAL)
			GTK_VALUE_UINT (*arg) = priv->total_width;
		else
			GTK_VALUE_UINT (*arg) = priv->max_width;
		break;
	case ARG_PREFERRED_HEIGHT:
		update_sizes (toolbar);
		if (bonobo_ui_toolbar_get_orientation (toolbar) == GTK_ORIENTATION_HORIZONTAL)
			GTK_VALUE_UINT (*arg) = priv->max_height;
		else
			GTK_VALUE_UINT (*arg) = priv->total_height;
		break;
	default:
		break;
	};
}

static void
impl_set_arg (GtkObject *obj,
	      GtkArg *arg,
	      guint arg_id)
{
	BonoboUIToolbar *toolbar = BONOBO_UI_TOOLBAR (obj);
	BonoboUIToolbarPrivate *priv = toolbar->priv;

	switch (arg_id) {
	case ARG_ORIENTATION:
		bonobo_ui_toolbar_set_orientation (toolbar, GTK_VALUE_UINT(*arg));
		break;
	case ARG_IS_FLOATING:
		priv->is_floating = GTK_VALUE_BOOL(*arg);
		break;
	default:
		break;
	};
}

static void
class_init (BonoboUIToolbarClass *toolbar_class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	GtkContainerClass *container_class;

	object_class = GTK_OBJECT_CLASS (toolbar_class);
	object_class->destroy  = impl_destroy;
	object_class->finalize = impl_finalize;
	object_class->get_arg  = impl_get_arg;
	object_class->set_arg  = impl_set_arg;

	widget_class = GTK_WIDGET_CLASS (toolbar_class);
	widget_class->size_request  = impl_size_request;
	widget_class->size_allocate = impl_size_allocate;
	widget_class->map           = impl_map;
	widget_class->unmap         = impl_unmap;
	widget_class->draw          = impl_draw;
	widget_class->expose_event  = impl_expose_event;

	container_class = GTK_CONTAINER_CLASS (toolbar_class);
	container_class->remove = impl_remove;
	container_class->forall = impl_forall;

	toolbar_class->set_orientation = impl_set_orientation;
	toolbar_class->style_changed   = impl_style_changed;

	parent_class = gtk_type_class (gtk_container_get_type ());

	gtk_object_add_arg_type("BonoboUIToolbar::orientation",
				GTK_TYPE_UINT, GTK_ARG_READWRITE, ARG_ORIENTATION);
	gtk_object_add_arg_type("BonoboUIToolbar::is_floating",
				GTK_TYPE_BOOL, GTK_ARG_READWRITE, ARG_IS_FLOATING);

	gtk_object_add_arg_type("BonoboUIToolbar::preferred_width",
				GTK_TYPE_UINT, GTK_ARG_READABLE, ARG_PREFERRED_WIDTH);
	gtk_object_add_arg_type("BonoboUIToolbar::preferred_height",
				GTK_TYPE_UINT, GTK_ARG_READABLE, ARG_PREFERRED_HEIGHT);

	signals[SET_ORIENTATION]
		= gtk_signal_new ("set_orientation",
				  GTK_RUN_LAST,
				  object_class->type,
				  GTK_SIGNAL_OFFSET (BonoboUIToolbarClass, set_orientation),
				  gtk_marshal_NONE__INT,
				  GTK_TYPE_NONE, 1,
				  GTK_TYPE_INT);

	signals[STYLE_CHANGED]
		= gtk_signal_new ("set_style",
				  GTK_RUN_LAST,
				  object_class->type,
				  GTK_SIGNAL_OFFSET (BonoboUIToolbarClass, style_changed),
				  gtk_marshal_NONE__NONE,
				  GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
}

static void
init (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	BonoboUIToolbarStyle style;

	GTK_WIDGET_SET_FLAGS (toolbar, GTK_NO_WINDOW);
	GTK_WIDGET_UNSET_FLAGS (toolbar, GTK_CAN_FOCUS);

	priv = g_new (BonoboUIToolbarPrivate, 1);

	style = gnome_preferences_get_toolbar_labels ()
		? BONOBO_UI_TOOLBAR_STYLE_ICONS_AND_TEXT
		: BONOBO_UI_TOOLBAR_STYLE_ICONS_ONLY;

	priv->orientation                 = GTK_ORIENTATION_HORIZONTAL;
	priv->is_floating		  = FALSE;
	priv->style                       = style;
	priv->hstyle                      = style;
	priv->vstyle                      = style;
	priv->max_width			  = 0;
	priv->total_width		  = 0;
	priv->max_height		  = 0;
	priv->total_height		  = 0;
	priv->popup_item                  = NULL;
	priv->items                       = NULL;
	priv->first_not_fitting_item      = NULL;
	priv->popup_window                = NULL;
	priv->popup_window_vbox           = NULL;
	priv->items_moved_to_popup_window = FALSE;
	priv->tooltips                    = gtk_tooltips_new ();

	toolbar->priv = priv;
}


GtkType
bonobo_ui_toolbar_get_type (void)
{
	static GtkType type = 0;

	if (type == 0) {
		static const GtkTypeInfo info = {
			"BonoboUIToolbar",
			sizeof (BonoboUIToolbar),
			sizeof (BonoboUIToolbarClass),
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
bonobo_ui_toolbar_construct (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;
	GtkWidget *frame;

	g_return_if_fail (toolbar != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar));

	priv = toolbar->priv;

	priv->popup_item = BONOBO_UI_TOOLBAR_ITEM (bonobo_ui_toolbar_popup_item_new ());
	bonobo_ui_toolbar_item_set_orientation (priv->popup_item, priv->orientation);
	parentize_widget (toolbar, GTK_WIDGET (priv->popup_item));

	gtk_signal_connect (GTK_OBJECT (priv->popup_item), "toggled",
			    GTK_SIGNAL_FUNC (popup_item_toggled_cb), toolbar);

	priv->popup_window = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_signal_connect (GTK_OBJECT (priv->popup_window), "button_release_event",
			    GTK_SIGNAL_FUNC (popup_window_button_release_cb), toolbar);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
	gtk_container_add (GTK_CONTAINER (priv->popup_window), frame);

	priv->popup_window_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (priv->popup_window_vbox);
	gtk_container_add (GTK_CONTAINER (frame), priv->popup_window_vbox);
}

GtkWidget *
bonobo_ui_toolbar_new (void)
{
	BonoboUIToolbar *toolbar;

	toolbar = gtk_type_new (bonobo_ui_toolbar_get_type ());

	bonobo_ui_toolbar_construct (toolbar);

	return GTK_WIDGET (toolbar);
}


void
bonobo_ui_toolbar_set_orientation (BonoboUIToolbar *toolbar,
				   GtkOrientation orientation)
{
	g_return_if_fail (toolbar != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar));
	g_return_if_fail (orientation == GTK_ORIENTATION_HORIZONTAL
			  || orientation == GTK_ORIENTATION_VERTICAL);

	gtk_signal_emit (GTK_OBJECT (toolbar), signals[SET_ORIENTATION], orientation);

	gtk_signal_emit (GTK_OBJECT (toolbar), signals[STYLE_CHANGED]);
}

GtkOrientation
bonobo_ui_toolbar_get_orientation (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;

	g_return_val_if_fail (toolbar != NULL, GTK_ORIENTATION_HORIZONTAL);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar), GTK_ORIENTATION_HORIZONTAL);

	priv = toolbar->priv;

	return priv->orientation;
}


BonoboUIToolbarStyle
bonobo_ui_toolbar_get_style (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;

	g_return_val_if_fail (toolbar != NULL, BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar), BONOBO_UI_TOOLBAR_STYLE_PRIORITY_TEXT);

	priv = toolbar->priv;

	return priv->style;
}

GtkTooltips *
bonobo_ui_toolbar_get_tooltips (BonoboUIToolbar *toolbar)
{
	BonoboUIToolbarPrivate *priv;

	g_return_val_if_fail (toolbar != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar), NULL);

	priv = toolbar->priv;

	return priv->tooltips;
}


void
bonobo_ui_toolbar_insert (BonoboUIToolbar *toolbar,
			  BonoboUIToolbarItem *item,
			  int position)
{
	BonoboUIToolbarPrivate *priv;

	g_return_if_fail (toolbar != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar));
	g_return_if_fail (item != NULL);
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR_ITEM (item));

	gtk_object_ref (GTK_OBJECT (item));
	gtk_object_sink (GTK_OBJECT (item));

	priv = toolbar->priv;

	/*
	 *  This ugly hack is here since we might have unparented
	 * a widget and then re-added it to the toolbar at a later
	 * date, and un-parenting doesn't work quite properly yet.
	 */
	if (!g_list_find (priv->items, item))
		priv->items = g_list_insert (priv->items, item, position);

	gtk_signal_connect_while_alive (GTK_OBJECT (item), "destroy",
					GTK_SIGNAL_FUNC (item_destroy_cb), toolbar,
					GTK_OBJECT (toolbar));
	gtk_signal_connect_while_alive (GTK_OBJECT (item), "activate",
					GTK_SIGNAL_FUNC (item_activate_cb), toolbar,
					GTK_OBJECT (toolbar));
	gtk_signal_connect_while_alive (GTK_OBJECT (item), "set_want_label",
					GTK_SIGNAL_FUNC (item_set_want_label_cb), toolbar,
					GTK_OBJECT (toolbar));

	set_attributes_on_child (item, priv->orientation, priv->style);
	parentize_widget (toolbar, GTK_WIDGET (item));

	g_assert (GTK_WIDGET (item)->parent == GTK_WIDGET (toolbar));

	gtk_widget_queue_resize (GTK_WIDGET (toolbar));
}

GList *
bonobo_ui_toolbar_get_children (BonoboUIToolbar *toolbar)
{
	GList *ret = NULL, *l;

	g_return_val_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar), NULL);

	for (l = toolbar->priv->items; l; l = l->next) {
		GtkWidget *item_widget;

		item_widget = GTK_WIDGET (l->data);
		if (item_widget->parent != NULL) /* Unparented but still here */
			ret = g_list_prepend (ret, item_widget);
	}

	return g_list_reverse (ret);
}

void
bonobo_ui_toolbar_set_hv_styles (BonoboUIToolbar      *toolbar,
				 BonoboUIToolbarStyle  hstyle,
				 BonoboUIToolbarStyle  vstyle)
{
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar));

	toolbar->priv->hstyle = hstyle;
	toolbar->priv->vstyle = vstyle;

	gtk_signal_emit (GTK_OBJECT (toolbar), signals [STYLE_CHANGED]);
}

void
bonobo_ui_toolbar_show_tooltips (BonoboUIToolbar *toolbar,
				 gboolean         show_tips)
{
	g_return_if_fail (BONOBO_IS_UI_TOOLBAR (toolbar));

	if (show_tips)
		gtk_tooltips_enable (toolbar->priv->tooltips);
	else
		gtk_tooltips_disable (toolbar->priv->tooltips);
}
