/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-viewport.c - A subclass of GtkViewport with non broken drawing.

   Copyright (C) 1999, 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#include <config.h>

#include "eel-viewport.h"

#include "eel-gtk-macros.h"

#include <gtk/gtksignal.h>

/* Detail member struct */
struct EelViewportDetails
{
	gboolean is_smooth;
	gboolean never_smooth;
       
	gboolean constrain_width;
	gboolean constrain_height;
};

/* GtkObjectClass methods */
static void eel_viewport_initialize_class     (EelViewportClass *viewport_class);
static void eel_viewport_initialize           (EelViewport      *viewport);
static void eel_viewport_destroy              (GtkObject        *object);

/* GtkWidgetClass methods */
static void eel_viewport_realize              (GtkWidget        *widget);
static void eel_viewport_draw                 (GtkWidget        *widget,
					       GdkRectangle     *area);
static void eel_viewport_size_allocate        (GtkWidget        *widget,
					       GtkAllocation    *allocation);
static gint eel_viewport_expose_event         (GtkWidget        *widget,
					       GdkEventExpose   *event);
static void eel_viewport_paint                (GtkWidget        *widget,
					       GdkRectangle     *area);

/* EelViewport signals */
static void eel_viewport_set_is_smooth_signal (GtkWidget        *widget,
					       gboolean          is_smooth);

/* Signals */
typedef enum
{
	SET_IS_SMOOTH,
	LAST_SIGNAL
} EelViewportSignal;

/* Signals */
static guint eel_viewport_signals[LAST_SIGNAL] = { 0 };

EEL_DEFINE_CLASS_BOILERPLATE (EelViewport, eel_viewport, GTK_TYPE_VIEWPORT)

/* GtkObjectClass methods */
static void
eel_viewport_initialize_class (EelViewportClass *eel_viewport_class)
{
	GtkObjectClass *object_class = GTK_OBJECT_CLASS (eel_viewport_class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (eel_viewport_class);

	/* GtkObjectClass */
	object_class->destroy = eel_viewport_destroy;
	
	/* GtkWidgetClass */
	widget_class->realize = eel_viewport_realize;
	widget_class->expose_event = eel_viewport_expose_event;
	widget_class->draw = eel_viewport_draw;
	widget_class->size_allocate = eel_viewport_size_allocate;
	
	/* EelViewportClass */
	eel_viewport_class->set_is_smooth = eel_viewport_set_is_smooth_signal;
	
	eel_viewport_signals[SET_IS_SMOOTH] = 
		gtk_signal_new ("set_is_smooth",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (EelViewportClass, set_is_smooth),
				gtk_marshal_NONE__BOOL,
				GTK_TYPE_NONE, 
				1,
				GTK_TYPE_BOOL);
	
	gtk_object_class_add_signals (object_class, eel_viewport_signals, LAST_SIGNAL);

	/* Let the smooth widget machinery know that our class can be smooth */
	eel_smooth_widget_register_type (EEL_TYPE_VIEWPORT);
}

void
eel_viewport_initialize (EelViewport *eel_viewport)
{
	eel_viewport->details = g_new0 (EelViewportDetails, 1);
	
	eel_smooth_widget_register (GTK_WIDGET (eel_viewport));
	eel_viewport->details->never_smooth = TRUE;
}

void
eel_viewport_destroy (GtkObject *object)
{
	EelViewport *viewport;

	g_return_if_fail (EEL_IS_VIEWPORT (object));

	viewport = EEL_VIEWPORT (object);
	
	g_free (viewport->details);

	/* Chain destroy */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/* GtkWidgetClass methods */
static void
eel_viewport_draw (GtkWidget *widget,
			GdkRectangle *area)
{
	EelViewport *eel_viewport;
	GtkViewport *viewport;
	GtkBin *bin;
	GdkRectangle tmp_area;
	GdkRectangle child_area;
	gint border_width;
	
	g_return_if_fail (EEL_IS_VIEWPORT (widget));
	g_return_if_fail (area != NULL);
	
	if (!GTK_WIDGET_DRAWABLE (widget)) {
		return;
	}

	eel_viewport = EEL_VIEWPORT (widget);
	viewport = GTK_VIEWPORT (widget);
	bin = GTK_BIN (widget);
	
	border_width = GTK_CONTAINER (widget)->border_width;
	
	tmp_area = *area;
	tmp_area.x -= border_width;
	tmp_area.y -= border_width;
	
	eel_viewport_paint (widget, &tmp_area);
	
	tmp_area.x += viewport->hadjustment->value - widget->style->klass->xthickness;
	tmp_area.y += viewport->vadjustment->value - widget->style->klass->ythickness;
	
	/* The gtk_viewport_draw() version does not adjust the width
	 * and height of the tmp_area for the class x/y thickness.  This
	 * causes some drawing to be clipped on the bottom.  This is a bug
	 * in GTK+.
	 */
	
	/* FIXME bugzilla.eazel.com xxxx: 
	 * Remove this widget once the fix makes it to GTK+.
	 */
	tmp_area.width += 2 * widget->style->klass->xthickness;
	tmp_area.height += 2 * widget->style->klass->ythickness;
	
	if (!eel_viewport_get_is_smooth (eel_viewport)) {
		gtk_paint_flat_box (widget->style, viewport->bin_window, 
				    GTK_STATE_NORMAL, GTK_SHADOW_NONE,
				    &tmp_area, widget, "viewportbin",
				    0, 0, -1, -1);
	}
	
	if (bin->child) {
		if (gtk_widget_intersect (bin->child, &tmp_area, &child_area)) {
			gtk_widget_draw (bin->child, &child_area);
		}
	}
}

static void
eel_viewport_size_allocate (GtkWidget *widget,
				 GtkAllocation *allocation)
{
	EelViewport *eel_viewport;
	GtkViewport *viewport;
	GtkBin *bin;
	GtkAllocation child_allocation;
	gint hval, vval;
	gint border_width;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_VIEWPORT (widget));
	g_return_if_fail (allocation != NULL);
	
	widget->allocation = *allocation;
	eel_viewport = EEL_VIEWPORT (widget);
	viewport = GTK_VIEWPORT (widget);
	bin = GTK_BIN (widget);
	  
	border_width = GTK_CONTAINER (widget)->border_width;
	
	child_allocation.x = 0;
	child_allocation.y = 0;
	
	if (viewport->shadow_type != GTK_SHADOW_NONE)
	{
		child_allocation.x = GTK_WIDGET (viewport)->style->klass->xthickness;
		child_allocation.y = GTK_WIDGET (viewport)->style->klass->ythickness;
	}
	
	child_allocation.width = MAX (1, allocation->width - child_allocation.x * 2 - border_width * 2);
	child_allocation.height = MAX (1, allocation->height - child_allocation.y * 2 - border_width * 2);
	  
	if (GTK_WIDGET_REALIZED (widget))
	{
		gdk_window_move_resize (widget->window,
					allocation->x + border_width,
					allocation->y + border_width,
					allocation->width - border_width * 2,
					allocation->height - border_width * 2);
		
		gdk_window_move_resize (viewport->view_window,
					child_allocation.x,
					child_allocation.y,
					child_allocation.width,
					child_allocation.height);
	}
	
	viewport->hadjustment->page_size = child_allocation.width;
	viewport->hadjustment->page_increment = viewport->hadjustment->page_size / 2;
	viewport->hadjustment->step_increment = 10;
	
	viewport->vadjustment->page_size = child_allocation.height;
	viewport->vadjustment->page_increment = viewport->vadjustment->page_size / 2;
	viewport->vadjustment->step_increment = 10;
	
	hval = viewport->hadjustment->value;
	vval = viewport->vadjustment->value;
	
	if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
	{
		GtkRequisition child_requisition;
		gtk_widget_get_child_requisition (bin->child, &child_requisition);
		
		viewport->hadjustment->lower = 0;
		viewport->hadjustment->upper = MAX (child_requisition.width,
						    child_allocation.width);
		
		hval = CLAMP (hval, 0,
			      viewport->hadjustment->upper -
			      viewport->hadjustment->page_size);
		
		viewport->vadjustment->lower = 0;
		viewport->vadjustment->upper = MAX (child_requisition.height,
						    child_allocation.height);
		
		vval = CLAMP (vval, 0,
			      viewport->vadjustment->upper -
			      viewport->vadjustment->page_size);
	}
	
	if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
	{
		child_allocation.x = 0;
		child_allocation.y = 0;
		
		child_allocation.width = viewport->hadjustment->upper;
		child_allocation.height = viewport->vadjustment->upper;
		
		if (eel_viewport_get_constrain_width (eel_viewport)) {
			child_allocation.width = widget->allocation.width;
		}
		if (eel_viewport_get_constrain_height (eel_viewport)) {
			child_allocation.height = widget->allocation.height;
		}
		
		if (GTK_WIDGET_REALIZED (widget))
			gdk_window_resize (viewport->bin_window,
					   child_allocation.width,
					   child_allocation.height);
		
		child_allocation.x = 0;
		child_allocation.y = 0;
		gtk_widget_size_allocate (bin->child, &child_allocation);
	}
	
	gtk_signal_emit_by_name (GTK_OBJECT (viewport->hadjustment), "changed");
	gtk_signal_emit_by_name (GTK_OBJECT (viewport->vadjustment), "changed");
	if (viewport->hadjustment->value != hval)
	{
		viewport->hadjustment->value = hval;
		gtk_signal_emit_by_name (GTK_OBJECT (viewport->hadjustment), "value_changed");
	}
	if (viewport->vadjustment->value != vval)
	{
		viewport->vadjustment->value = vval;
		gtk_signal_emit_by_name (GTK_OBJECT (viewport->vadjustment), "value_changed");
	}
}

static gint
eel_viewport_expose_event (GtkWidget *widget,
				GdkEventExpose *event)
{
	EelViewport *eel_viewport;
	GtkViewport *viewport;
	GtkBin *bin;
	
	g_return_val_if_fail (EEL_IS_VIEWPORT (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	if (!GTK_WIDGET_DRAWABLE (widget)) {
		return FALSE;
	}

	eel_viewport = EEL_VIEWPORT (widget);
	viewport = GTK_VIEWPORT (widget);
	bin = GTK_BIN (widget);
	
	if (event->window == widget->window) {
		eel_viewport_paint (widget, &event->area);
	} else if (event->window == viewport->bin_window) {
		GdkEventExpose child_event;

		child_event = *event;
		
		if (!eel_viewport_get_is_smooth (eel_viewport)) {
			gtk_paint_flat_box (widget->style, viewport->bin_window, 
					    GTK_STATE_NORMAL, GTK_SHADOW_NONE,
					    &event->area, widget, "viewportbin",
					    0, 0, -1, -1);
		}
		
		if ((bin->child != NULL) &&
		    GTK_WIDGET_NO_WINDOW (bin->child) &&
		    gtk_widget_intersect (bin->child, &event->area, &child_event.area))
			gtk_widget_event (bin->child, (GdkEvent*) &child_event);
	}
	
	return FALSE;
}

static void
eel_viewport_realize (GtkWidget *widget)
{
	EelViewport *eel_viewport;
	
	g_return_if_fail (EEL_IS_VIEWPORT (widget));
	
	eel_viewport = EEL_VIEWPORT (widget);

	/* GtkViewport does the actual realization */
	EEL_CALL_PARENT (GTK_WIDGET_CLASS, realize, (widget));

 	gdk_window_set_static_gravities (GTK_VIEWPORT (eel_viewport)->bin_window,
					 eel_viewport_get_is_smooth (eel_viewport));
}

static void
eel_viewport_paint (GtkWidget *widget,
			 GdkRectangle *area)
{
	GtkViewport *viewport;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_VIEWPORT (widget));
	g_return_if_fail (area != NULL);
	
	if (!GTK_WIDGET_DRAWABLE (widget)) {
		return;
	}

	viewport = GTK_VIEWPORT (widget);
	
	gtk_draw_shadow (widget->style, widget->window,
			 GTK_STATE_NORMAL, viewport->shadow_type,
			 0, 0, -1, -1);
}

/* EelViewport signals */
static void
eel_viewport_set_is_smooth_signal (GtkWidget *widget,
					gboolean is_smooth)
{
	g_return_if_fail (EEL_IS_VIEWPORT (widget));
	
	eel_viewport_set_is_smooth (EEL_VIEWPORT (widget), is_smooth);
}

/* Public EelViewport methods */
GtkWidget*
eel_viewport_new (GtkAdjustment *hadjustment,
		       GtkAdjustment *vadjustment)
{
	EelViewport *eel_viewport;

	eel_viewport = EEL_VIEWPORT (gtk_widget_new (eel_viewport_get_type (), NULL));
	
	gtk_viewport_set_hadjustment (GTK_VIEWPORT (eel_viewport), hadjustment);
	gtk_viewport_set_vadjustment (GTK_VIEWPORT (eel_viewport), vadjustment);

	return GTK_WIDGET (eel_viewport);
}

void
eel_viewport_set_is_smooth (EelViewport *eel_viewport,
				 gboolean is_smooth)
{
	g_return_if_fail (EEL_IS_VIEWPORT (eel_viewport));

	if (eel_viewport->details->is_smooth == is_smooth) {
		return;
	}

	eel_viewport->details->is_smooth = is_smooth;
	
	if (!GTK_WIDGET_REALIZED (eel_viewport)) {
		return;
	}
	
 	gdk_window_set_static_gravities (GTK_VIEWPORT (eel_viewport)->bin_window,
					 eel_viewport->details->is_smooth);
}

gboolean
eel_viewport_get_is_smooth (const EelViewport *eel_viewport)
{
	g_return_val_if_fail (EEL_IS_VIEWPORT (eel_viewport), FALSE);
	
	return !eel_viewport->details->never_smooth && eel_viewport->details->is_smooth;
}

void
eel_viewport_set_constrain_width (EelViewport *eel_viewport,
				       gboolean constrain_width)
{
	g_return_if_fail (EEL_IS_VIEWPORT (eel_viewport));

	eel_viewport->details->constrain_width = constrain_width;
}

gboolean
eel_viewport_get_constrain_width (const EelViewport *eel_viewport)
{
	g_return_val_if_fail (EEL_IS_VIEWPORT (eel_viewport), FALSE);

	return eel_viewport->details->constrain_width;
}

void eel_viewport_set_constrain_height (EelViewport *eel_viewport,
					     gboolean constrain_height)
{
	g_return_if_fail (EEL_IS_VIEWPORT (eel_viewport));
	
	eel_viewport->details->constrain_height = constrain_height;
}

gboolean
eel_viewport_get_constrain_height (const EelViewport *eel_viewport)
{
	g_return_val_if_fail (EEL_IS_VIEWPORT (eel_viewport), FALSE);

	return eel_viewport->details->constrain_height;
}

void
eel_viewport_set_never_smooth (EelViewport *eel_viewport,
				    gboolean never_smooth)
{
	g_return_if_fail (EEL_IS_VIEWPORT (eel_viewport));

	eel_viewport->details->never_smooth = never_smooth;

	if (!GTK_WIDGET_REALIZED (eel_viewport)) {
		return;
	}

 	gdk_window_set_static_gravities (GTK_VIEWPORT (eel_viewport)->bin_window,
					 eel_viewport_get_is_smooth (eel_viewport));
}

EelArtIPoint
eel_viewport_get_scroll_offset (const EelViewport *eel_viewport)
{
	EelArtIPoint scroll_offset;
	
	g_return_val_if_fail (EEL_IS_VIEWPORT (eel_viewport), eel_art_ipoint_zero);

	if (!GTK_WIDGET_REALIZED (eel_viewport)) {
		return eel_art_ipoint_zero;
	}

	gdk_window_get_position (GTK_VIEWPORT (eel_viewport)->bin_window,
				 &scroll_offset.x,
				 &scroll_offset.y);

	return scroll_offset;
}

void
eel_gtk_scrolled_window_add_with_viewport (GtkScrolledWindow *scrolled_window,
					   GtkWidget *child)
{
	GtkBin *bin;
	GtkWidget *viewport;
	
	g_return_if_fail (GTK_IS_SCROLLED_WINDOW (scrolled_window));
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (child->parent == NULL);
	
	bin = GTK_BIN (scrolled_window);
	
	if (bin->child != NULL) {
		g_return_if_fail (EEL_IS_VIEWPORT (bin->child));
		g_return_if_fail (GTK_BIN (bin->child)->child == NULL);
		
		viewport = bin->child;
	} else {
		viewport = eel_viewport_new (gtk_scrolled_window_get_hadjustment (scrolled_window),
					     gtk_scrolled_window_get_vadjustment (scrolled_window));
		gtk_container_add (GTK_CONTAINER (scrolled_window), viewport);
	}

	gtk_widget_show (viewport);
	gtk_container_add (GTK_CONTAINER (viewport), child);
}
