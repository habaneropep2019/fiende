/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-image-table.c - An image table.

   Copyright (C) 2000 Eazel, Inc.

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

#include "eel-image-table.h"

#include "eel-gtk-macros.h"
#include "eel-gtk-extensions.h"
#include "eel-art-extensions.h"
#include "eel-art-gtk-extensions.h"
#include "eel-region.h"
#include "eel-debug-drawing.h"

#include <gtk/gtkmain.h>

/* Arguments */
enum
{
	ARG_0,
	ARG_CHILD_UNDER_POINTER
};

/* Detail member struct */
struct EelImageTableDetails
{
	GtkWidget *child_under_pointer;
	GtkWidget *child_being_pressed;
	GdkGC *clear_gc;
	guint32 smooth_background_color;
	gboolean is_smooth;
};

/* Signals */
typedef enum
{
	CHILD_ENTER,
	CHILD_LEAVE,
	CHILD_PRESSED,
	CHILD_RELEASED,
	CHILD_CLICKED,
	SET_IS_SMOOTH,
	LAST_SIGNAL
} ImageTableSignals;

/* Signals */
static guint image_table_signals[LAST_SIGNAL] = { 0 };

/* GtkObjectClass methods */
static void    eel_image_table_initialize_class     (EelImageTableClass *image_table_class);
static void    eel_image_table_initialize           (EelImageTable      *image);
static void    eel_image_table_destroy              (GtkObject          *object);

/* GtkWidgetClass methods */
static int     eel_image_table_expose_event         (GtkWidget          *widget,
						     GdkEventExpose     *event);
static void    eel_image_table_realize              (GtkWidget          *widget);
static void    eel_image_table_unrealize            (GtkWidget          *widget);

/* GtkContainerClass methods */
static void    eel_image_table_remove               (GtkContainer       *container,
						     GtkWidget          *widget);
static GtkType eel_image_table_child_type           (GtkContainer       *container);

/* Private EelImageTable methods */
static void    image_table_clear_dirty_areas        (EelImageTable      *image_table);
static GdkGC * image_table_peek_clear_gc            (EelImageTable      *image_table);
static void    image_table_emit_signal              (EelImageTable      *image_table,
						     GtkWidget          *child,
						     guint               signal_index,
						     int                 x,
						     int                 y,
						     int                 button,
						     guint               state);

/* Ancestor callbacks */
static int     ancestor_enter_notify_event          (GtkWidget          *widget,
						     GdkEventCrossing   *event,
						     gpointer            event_data);
static int     ancestor_leave_notify_event          (GtkWidget          *widget,
						     GdkEventCrossing   *event,
						     gpointer            event_data);
static int     ancestor_motion_notify_event         (GtkWidget          *widget,
						     GdkEventMotion     *event,
						     gpointer            event_data);
static int     ancestor_button_press_event          (GtkWidget          *widget,
						     GdkEventButton     *event,
						     gpointer            event_data);
static int     ancestor_button_release_event        (GtkWidget          *widget,
						     GdkEventButton     *event,
						     gpointer            event_data);

/* EelImageTable signals */
static void    eel_image_table_set_is_smooth_signal (GtkWidget          *widget,
						     gboolean            is_smooth);

EEL_DEFINE_CLASS_BOILERPLATE (EelImageTable, eel_image_table, EEL_TYPE_WRAP_TABLE)

/* Class init methods */
	static void
eel_image_table_initialize_class (EelImageTableClass *image_table_class)
{
	GtkObjectClass *object_class = GTK_OBJECT_CLASS (image_table_class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (image_table_class);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (image_table_class);

	/* GtkObjectClass */
	object_class->destroy = eel_image_table_destroy;
	
 	/* GtkWidgetClass */
 	widget_class->expose_event = eel_image_table_expose_event;
 	widget_class->realize = eel_image_table_realize;
 	widget_class->unrealize = eel_image_table_unrealize;

 	/* GtkContainerClass */
	container_class->remove = eel_image_table_remove;
	container_class->child_type = eel_image_table_child_type;

	/* EelImageTableClass */
	image_table_class->set_is_smooth = eel_image_table_set_is_smooth_signal;
	
	/* Signals */
	image_table_signals[CHILD_ENTER] = gtk_signal_new ("child_enter",
							   GTK_RUN_LAST,
							   object_class->type,
							   0,
							   gtk_marshal_NONE__POINTER_POINTER,
							   GTK_TYPE_NONE,
							   2,
							   GTK_TYPE_POINTER,
							   GTK_TYPE_POINTER);
	image_table_signals[CHILD_LEAVE] = gtk_signal_new ("child_leave",
							   GTK_RUN_LAST,
							   object_class->type,
							   0,
							   gtk_marshal_NONE__POINTER_POINTER,
							   GTK_TYPE_NONE,
							   2,
							   GTK_TYPE_POINTER,
							   GTK_TYPE_POINTER);
	image_table_signals[CHILD_PRESSED] = gtk_signal_new ("child_pressed",
							     GTK_RUN_LAST,
							     object_class->type,
							     0,
							     gtk_marshal_NONE__POINTER_POINTER,
							     GTK_TYPE_NONE,
							     2,
							     GTK_TYPE_POINTER,
							     GTK_TYPE_POINTER);
	image_table_signals[CHILD_RELEASED] = gtk_signal_new ("child_released",
							      GTK_RUN_LAST,
							      object_class->type,
							      0,
							      gtk_marshal_NONE__POINTER_POINTER,
							      GTK_TYPE_NONE,
							      2,
							      GTK_TYPE_POINTER,
							      GTK_TYPE_POINTER);
	image_table_signals[CHILD_CLICKED] = gtk_signal_new ("child_clicked",
							     GTK_RUN_LAST,
							     object_class->type,
							     0,
							     gtk_marshal_NONE__POINTER_POINTER,
							     GTK_TYPE_NONE,
							     2,
							     GTK_TYPE_POINTER,
							     GTK_TYPE_POINTER);
	
	image_table_signals[SET_IS_SMOOTH] = gtk_signal_new ("set_is_smooth",
							     GTK_RUN_LAST,
							     object_class->type,
							     GTK_SIGNAL_OFFSET (EelImageTableClass, set_is_smooth),
							     gtk_marshal_NONE__BOOL,
							     GTK_TYPE_NONE, 
							     1,
							     GTK_TYPE_BOOL);

	gtk_object_class_add_signals (object_class, image_table_signals, LAST_SIGNAL);

	/* Let the smooth widget machinery know that our class can be smooth */
	eel_smooth_widget_register_type (EEL_TYPE_IMAGE_TABLE);
}

void
eel_image_table_initialize (EelImageTable *image_table)
{
	GTK_WIDGET_SET_FLAGS (image_table, GTK_NO_WINDOW);

	image_table->details = g_new0 (EelImageTableDetails, 1);
	image_table->details->smooth_background_color = EEL_RGB_COLOR_WHITE;

	eel_smooth_widget_register (GTK_WIDGET (image_table));
}

/* GtkObjectClass methods */
static void
eel_image_table_destroy (GtkObject *object)
{
 	EelImageTable *image_table;
	
	g_return_if_fail (EEL_IS_IMAGE_TABLE (object));

	image_table = EEL_IMAGE_TABLE (object);

	g_free (image_table->details);

	/* Chain destroy */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/* GtkWidgetClass methods */
static int
eel_image_table_expose_event (GtkWidget *widget,
			      GdkEventExpose *event)
{
	EelImageTable *image_table;

	g_return_val_if_fail (EEL_IS_WRAP_TABLE (widget), TRUE);
	g_return_val_if_fail (GTK_WIDGET_REALIZED (widget), TRUE);
	g_return_val_if_fail (event != NULL, TRUE);

	image_table = EEL_IMAGE_TABLE (widget);

	/* In smooth mode we clear dirty areas, since our background wont be
	 * force clear and thus avoid flicker.
	 */
	if (image_table->details->is_smooth) {
		image_table_clear_dirty_areas (image_table);
	}
	
	return EEL_CALL_PARENT_WITH_RETURN_VALUE
		(GTK_WIDGET_CLASS, expose_event, (widget, event));
}

static void
eel_image_table_realize (GtkWidget *widget)
{
	EelImageTable *image_table;
	GtkWidget *windowed_ancestor;

	g_return_if_fail (EEL_IS_IMAGE_TABLE (widget));
	
	image_table = EEL_IMAGE_TABLE (widget);

	/* Chain realize */
	EEL_CALL_PARENT (GTK_WIDGET_CLASS, realize, (widget));

	windowed_ancestor = eel_gtk_widget_find_windowed_ancestor (widget);
	g_assert (GTK_IS_WIDGET (windowed_ancestor));
	
	gtk_widget_add_events (windowed_ancestor,
			       GDK_BUTTON_PRESS_MASK
			       | GDK_BUTTON_RELEASE_MASK
			       | GDK_BUTTON_MOTION_MASK
			       | GDK_ENTER_NOTIFY_MASK
			       | GDK_LEAVE_NOTIFY_MASK
			       | GDK_POINTER_MOTION_MASK);

	eel_gtk_signal_connect_while_realized (GTK_OBJECT (windowed_ancestor),
					       "enter_notify_event",
					       GTK_SIGNAL_FUNC (ancestor_enter_notify_event),
					       widget,
					       widget);
	
	eel_gtk_signal_connect_while_realized (GTK_OBJECT (windowed_ancestor),
					       "leave_notify_event",
					       GTK_SIGNAL_FUNC (ancestor_leave_notify_event),
					       widget,
					       widget);
	
	eel_gtk_signal_connect_while_realized (GTK_OBJECT (windowed_ancestor),
					       "motion_notify_event",
					       GTK_SIGNAL_FUNC (ancestor_motion_notify_event),
					       widget,
					       widget);
	
	eel_gtk_signal_connect_while_realized (GTK_OBJECT (windowed_ancestor),
					       "button_press_event",
					       GTK_SIGNAL_FUNC (ancestor_button_press_event),
					       widget,
					       widget);
	
	eel_gtk_signal_connect_while_realized (GTK_OBJECT (windowed_ancestor),
					       "button_release_event",
					       GTK_SIGNAL_FUNC (ancestor_button_release_event),
					       widget,
					       widget);
}

static void
eel_image_table_unrealize (GtkWidget *widget)
{
	EelImageTable *image_table;

	g_return_if_fail (EEL_IS_IMAGE_TABLE (widget));

	image_table = EEL_IMAGE_TABLE (widget);

	if (image_table->details->clear_gc != NULL) {
		gdk_gc_unref (image_table->details->clear_gc);
		image_table->details->clear_gc = NULL;
	}

	/* Chain unrealize */
	EEL_CALL_PARENT (GTK_WIDGET_CLASS, unrealize, (widget));
}

/* GtkContainerClass methods */
static void
eel_image_table_remove (GtkContainer *container,
			GtkWidget *child)
{
	EelImageTable *image_table;
	
	g_return_if_fail (EEL_IS_IMAGE_TABLE (container));
	g_return_if_fail (EEL_IS_LABELED_IMAGE (child));
	
	image_table = EEL_IMAGE_TABLE (container);

	if (child == image_table->details->child_under_pointer) {
		image_table->details->child_under_pointer = NULL;
	}

	if (child == image_table->details->child_being_pressed) {
		image_table->details->child_being_pressed = NULL;
	}

	EEL_CALL_PARENT (GTK_CONTAINER_CLASS, remove, (container, child));
}

static GtkType
eel_image_table_child_type (GtkContainer *container)
{
	return EEL_TYPE_LABELED_IMAGE;
}

/* EelImageTable signals */
static void
eel_image_table_set_is_smooth_signal (GtkWidget *widget,
				      gboolean is_smooth)
{
	g_return_if_fail (EEL_IS_IMAGE_TABLE (widget));
	
	eel_image_table_set_is_smooth (EEL_IMAGE_TABLE (widget), is_smooth);
}

/* Private EelImageTable methods */

/* For each of the image table children, subtract their content from a region */
static void
image_table_foreach_child_subtract_content (GtkWidget *child,
					    gpointer callback_data)
{
	EelRegion *region;
	ArtIRect label_bounds;
	ArtIRect image_bounds;

	g_return_if_fail (EEL_IS_LABELED_IMAGE (child));
	g_return_if_fail (callback_data != NULL);

	if (!GTK_WIDGET_VISIBLE (child)) {
		return;
	}

	region = callback_data;

	image_bounds = eel_labeled_image_get_image_bounds (EEL_LABELED_IMAGE (child));
	if (!art_irect_empty (&image_bounds)) {
		eel_region_subtract_rectangle (region, image_bounds);
	}
	
	label_bounds = eel_labeled_image_get_label_bounds (EEL_LABELED_IMAGE (child));
	if (!art_irect_empty (&label_bounds)) {
		eel_region_subtract_rectangle (region, label_bounds);
	}
}

/* Clear the dirty areas around the children's content */
static void
image_table_clear_dirty_areas (EelImageTable *image_table)
{
	GtkWidget *widget;
	EelRegion *region;
	EelDimensions dimensions;
	ArtIRect whole_region;
	GdkGC *gc;
	
	g_return_if_fail (EEL_IS_WRAP_TABLE (image_table));
	g_return_if_fail (GTK_WIDGET_REALIZED (image_table));

	widget = GTK_WIDGET (image_table);
	
	dimensions = eel_gtk_widget_get_dimensions (widget->parent);
	whole_region = eel_art_irect_assign_dimensions (0, 0, dimensions);
	region = eel_region_new ();
	
	eel_region_add_rectangle (region, whole_region);

	gc = image_table_peek_clear_gc (image_table);
	
	gtk_container_foreach (GTK_CONTAINER (image_table), image_table_foreach_child_subtract_content, region);
	
	eel_region_set_gc_clip_region (region, gc);
	
	gdk_draw_rectangle (widget->window,
			    gc,
			    TRUE,
			    0,
			    0,
			    dimensions.width,
			    dimensions.height);
	
	eel_region_free (region);
}

static GdkGC *
image_table_peek_clear_gc (EelImageTable *image_table)
{
	g_return_val_if_fail (EEL_IS_IMAGE_TABLE (image_table), NULL);

	if (image_table->details->clear_gc == NULL) {
		image_table->details->clear_gc = gdk_gc_new (GTK_WIDGET (image_table)->window);
		gdk_gc_set_function (image_table->details->clear_gc, GDK_COPY);
	}

	gdk_rgb_gc_set_foreground (image_table->details->clear_gc, image_table->details->smooth_background_color);

	return image_table->details->clear_gc;
}

static void
image_table_emit_signal (EelImageTable *image_table,
			 GtkWidget *child,
			 guint signal_index,
			 int x,
			 int y,
			 int button,
			 guint state)
{
	EelImageTableEvent event;

	g_return_if_fail (EEL_IS_IMAGE_TABLE (image_table));
	g_return_if_fail (GTK_IS_WIDGET (child));
	g_return_if_fail (signal_index >= 0);
	g_return_if_fail (signal_index < LAST_SIGNAL);

	event.x = x;
	event.y = y;
	event.button = button;
	event.state = state;
	
	gtk_signal_emit (GTK_OBJECT (image_table), 
			 image_table_signals[signal_index],
			 child,
			 &event);
}

static void
image_table_handle_motion (EelImageTable *image_table,
			   int x,
			   int y,
			   GdkEvent *event)
{
	GtkWidget *child;
	GtkWidget *leave_emit_child = NULL;
	GtkWidget *enter_emit_child = NULL;

	g_return_if_fail (EEL_IS_IMAGE_TABLE (image_table));

	child = eel_wrap_table_find_child_at_event_point (EEL_WRAP_TABLE (image_table), x, y);

	if (child && !GTK_WIDGET_SENSITIVE (child)) {
		return;
	}

	if (child == image_table->details->child_under_pointer) {
		return;
	}

	if (child != NULL) {
		if (image_table->details->child_under_pointer != NULL) {
			leave_emit_child = image_table->details->child_under_pointer;
		}

		image_table->details->child_under_pointer = child;
		enter_emit_child = image_table->details->child_under_pointer;
	} else {
		if (image_table->details->child_under_pointer != NULL) {
			leave_emit_child = image_table->details->child_under_pointer;
		}

		image_table->details->child_under_pointer = NULL;
	}

	if (leave_emit_child != NULL) {
		image_table_emit_signal (image_table,
					 leave_emit_child,
					 CHILD_LEAVE,
					 x,
					 y,
					 0,
					 0);
	}

	if (enter_emit_child != NULL) {
		image_table_emit_signal (image_table,
					 enter_emit_child,
					 CHILD_ENTER,
					 x,
					 y,
					 0,
					 0);
	}
}

static int
ancestor_enter_notify_event (GtkWidget *widget,
			     GdkEventCrossing *event,
			     gpointer event_data)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
	g_return_val_if_fail (EEL_IS_IMAGE_TABLE (event_data), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	image_table_handle_motion (EEL_IMAGE_TABLE (event_data), event->x, event->y, (GdkEvent *) event);

	return FALSE;
}

static int
ancestor_leave_notify_event (GtkWidget *widget,
			     GdkEventCrossing *event,
			     gpointer event_data)
{
	ArtIRect bounds;
	int x = -1;
	int y = -1;
	
	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
	g_return_val_if_fail (EEL_IS_IMAGE_TABLE (event_data), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (event_data));
	
	if (eel_art_irect_contains_point (bounds, event->x, event->y)) {
		x = event->x;
		y = event->y;
	}

	image_table_handle_motion (EEL_IMAGE_TABLE (event_data), x, y, (GdkEvent *) event);
	
	return FALSE;
}

static int
ancestor_motion_notify_event (GtkWidget *widget,
			      GdkEventMotion *event,
			      gpointer event_data)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
	g_return_val_if_fail (EEL_IS_IMAGE_TABLE (event_data), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	image_table_handle_motion (EEL_IMAGE_TABLE (event_data), (int) event->x, (int) event->y, (GdkEvent *) event);

	return FALSE;
}

static int
ancestor_button_press_event (GtkWidget *widget,
			     GdkEventButton *event,
			     gpointer event_data)
{
  	EelImageTable *image_table;
	GtkWidget *child;

	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
	g_return_val_if_fail (EEL_IS_IMAGE_TABLE (event_data), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

 	image_table = EEL_IMAGE_TABLE (event_data);

	child = eel_wrap_table_find_child_at_event_point (EEL_WRAP_TABLE (image_table), event->x, event->y);

	if (child && !GTK_WIDGET_SENSITIVE (child)) {
		return FALSE;
	}

	if (child != NULL) {
		if (child == image_table->details->child_under_pointer) {
			image_table->details->child_being_pressed = child;
			image_table_emit_signal (image_table,
						 child,
						 CHILD_PRESSED,
						 event->x,
						 event->y,
						 event->button,
						 event->state);
		}
	}

	return FALSE;
}

static int
ancestor_button_release_event (GtkWidget *widget,
			       GdkEventButton *event,
			       gpointer event_data)
{
  	EelImageTable *image_table;
	GtkWidget *child;
	GtkWidget *released_emit_child = NULL;
	GtkWidget *clicked_emit_child = NULL;

	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
	g_return_val_if_fail (EEL_IS_IMAGE_TABLE (event_data), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

 	image_table = EEL_IMAGE_TABLE (event_data);

	child = eel_wrap_table_find_child_at_event_point (EEL_WRAP_TABLE (image_table), event->x, event->y);

	if (child && !GTK_WIDGET_SENSITIVE (child)) {
		return FALSE;
	}

	if (image_table->details->child_being_pressed != NULL) {
		released_emit_child = image_table->details->child_being_pressed;
	}

	if (child != NULL) {
		if (child == image_table->details->child_being_pressed) {
			clicked_emit_child = child;
		}
	}
	
	image_table->details->child_being_pressed = NULL;

	if (released_emit_child != NULL) {
		image_table_emit_signal (image_table,
					 released_emit_child,
					 CHILD_RELEASED,
					 event->x,
					 event->y,
					 event->button,
					 event->state);
	}
	
	if (clicked_emit_child != NULL) {

		image_table_emit_signal (image_table,
					 clicked_emit_child,
					 CHILD_CLICKED,
					 event->x,
					 event->y,
					 event->button,
					 event->state);
	}
	
	return FALSE;
}

/**
 * eel_image_table_new:
 */
GtkWidget*
eel_image_table_new (gboolean homogeneous)
{
	EelImageTable *image_table;

	image_table = EEL_IMAGE_TABLE (gtk_widget_new (eel_image_table_get_type (), NULL));

	eel_wrap_table_set_homogeneous (EEL_WRAP_TABLE (image_table), homogeneous);
	
	return GTK_WIDGET (image_table);
}

/**
 * eel_image_table_set_is_smooth:
 * @image_table: A EelImageTable.
 * @is_smooth: Boolean value indicating whether the image table is smooth.
 *
 */
void
eel_image_table_set_is_smooth (EelImageTable *image_table,
			       gboolean is_smooth)
{
	g_return_if_fail (EEL_IS_IMAGE_TABLE (image_table));
	
	if (image_table->details->is_smooth == is_smooth) {
		return;
	}
	
	image_table->details->is_smooth = is_smooth;
}

/**
 * eel_image_table_set_smooth_background_color:
 * @image_table: A EelImageTable.
 * @smooth_background_color: The color to use for background in smooth mode.
 *
 */
void
eel_image_table_set_smooth_background_color (EelImageTable *image_table,
					     guint32 smooth_background_color)
{
	g_return_if_fail (EEL_IS_IMAGE_TABLE (image_table));
	
	if (image_table->details->smooth_background_color == smooth_background_color) {
		return;
	}
	
	image_table->details->smooth_background_color = smooth_background_color;
}

/**
 * eel_image_table_add_empty_child:
 * @image_table: A EelImageTable.
 *
 * Add a "empty" child to the table.  Useful when you want to have
 * empty space between 2 children.
 *
 * Returns: The empty child - A EelLabeledImage widget with no label
 *          or pixbuf.
 */
GtkWidget *
eel_image_table_add_empty_image (EelImageTable *image_table)
{
	GtkWidget *empty;

	g_return_val_if_fail (EEL_IS_IMAGE_TABLE (image_table), NULL);

	empty = eel_labeled_image_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (image_table), empty);
	gtk_widget_set_sensitive (empty, FALSE);

	return empty;
}
