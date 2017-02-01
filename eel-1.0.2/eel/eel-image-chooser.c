/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-image-chooser.c - A widget to choose an image from a list.

   Copyright (C) 2001 Eazel, Inc.

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
#include "eel-image-chooser.h"

#include "eel-art-gtk-extensions.h"
#include "eel-gdk-extensions.h"
#include "eel-gdk-pixbuf-extensions.h"
#include "eel-glib-extensions.h"
#include "eel-gtk-macros.h"
#include "eel-image.h"
#include "eel-label.h"
#include "eel-region.h"
#include "eel-viewport.h"
#include <gtk/gtkclist.h>
#include <gtk/gtkclist.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkvbox.h>

#define IMAGE_CHOOSER_INVALID_INDEX -1
#define IMAGE_CHOOSER_ROW_BORDER 4
#define IMAGE_CHOOSER_ALL_VISIBLE -1
#define AUTO_SCROLL_TIMEOUT 150

typedef enum
{
	AUTO_SCROLL_NOPE = 0,
	AUTO_SCROLL_UP,
	AUTO_SCROLL_DOWN
} AutoScrollDirecton;

typedef struct
{
	GtkWidget *hbox;
	GtkWidget *text_column;
	EelImage *icon;
	EelLabel *title;
	EelLabel *description;
	gpointer row_data;
	GFreeFunc row_data_free_func;
} ImageChooserRow;

struct EelImageChooserDetails
{
	GList *rows;
 	GtkWidget *row_box;
	ImageChooserRow *selected_row;
	guint32 background_color;
	guint32 selected_background_color;
	guint32 text_color;
	guint32 selected_text_color;
	GdkGC *clear_gc;
	ImageChooserRow *row_being_browsed;
	guint auto_scroll_timeout;
	GtkScrolledWindow *scrolled_window;
	AutoScrollDirecton auto_scroll_direction;
};

/* Signals */
typedef enum
{
	SELECTION_CHANGED,
	LAST_SIGNAL
} EelStringListSignals;

static guint image_chooser_signals[LAST_SIGNAL];

/* EelImageChooserClass methods */
static void                  eel_image_chooser_initialize_class   (EelImageChooserClass  *image_chooser_class);
static void                  eel_image_chooser_initialize         (EelImageChooser       *image_chooser);

/* GtkObjectClass methods */
static void                  eel_image_chooser_destroy            (GtkObject            *object);

/* GtkWidgetClass methods */
static void                  image_chooser_style_set              (GtkWidget            *widget,
								  GtkStyle             *previous_style);
static int                   image_chooser_expose_event           (GtkWidget            *widget,
								  GdkEventExpose       *event);
static int                   image_chooser_motion_notify_event    (GtkWidget            *widget,
								  GdkEventMotion       *event);
static int                   image_chooser_button_press_event     (GtkWidget            *widget,
								  GdkEventButton       *event);
static int                   image_chooser_button_release_event   (GtkWidget            *widget,
								  GdkEventButton       *event);
static ImageChooserRow *      image_chooser_find_row_at_point      (const EelImageChooser *image_chooser,
								  int                   x,
								  int                   y);
static void                  image_chooser_update_colors          (EelImageChooser       *image_chooser);
static void                  image_chooser_set_row_colors         (EelImageChooser       *image_chooser,
								  ImageChooserRow       *row,
								  gboolean              selected);
static GdkGC *               image_chooser_peek_clear_gc          (EelImageChooser       *image_chooser);
static void                  image_chooser_paint_row_selected     (EelImageChooser       *image_chooser,
								  ArtIRect              area,
								  int                   row);
static void                  image_chooser_paint_row_unselected   (EelImageChooser       *image_chooser,
								  ArtIRect              area,
								  int                   row);
static int                   image_chooser_row_to_position        (const EelImageChooser *image_chooser,
								  const ImageChooserRow *row);
static ImageChooserRow       *image_chooser_position_to_row        (const EelImageChooser *image_chooser,
								  int                   position);
static int                   image_chooser_auto_scroll_timeout    (gpointer              callback_data);
static EelDimensions         image_chooser_get_partial_dimensions (const EelImageChooser *image_chooser,
								  guint                 num_rows);

EEL_DEFINE_CLASS_BOILERPLATE (EelImageChooser, eel_image_chooser, GTK_TYPE_BIN)

/*
 * EelImageChooserClass methods
 */
static void
eel_image_chooser_initialize_class (EelImageChooserClass *image_chooser_class)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	
	object_class = GTK_OBJECT_CLASS (image_chooser_class);
	widget_class = GTK_WIDGET_CLASS (image_chooser_class);

	/* GtkObjectClass */
	object_class->destroy = eel_image_chooser_destroy;

	/* GtkWidgetClass */
	widget_class->realize = eel_gtk_widget_standard_realize;
	widget_class->size_request = eel_gtk_bin_standard_size_request;
	widget_class->size_allocate = eel_gtk_bin_standard_size_allocate;
	widget_class->draw = eel_gtk_widget_standard_draw;
	widget_class->style_set = image_chooser_style_set;
	widget_class->expose_event = image_chooser_expose_event;
	widget_class->button_press_event = image_chooser_button_press_event;
	widget_class->button_release_event = image_chooser_button_release_event;
	widget_class->motion_notify_event = image_chooser_motion_notify_event;

	/* Signals */
	image_chooser_signals[SELECTION_CHANGED] = gtk_signal_new ("selection_changed",
								  GTK_RUN_LAST,
								  object_class->type,
								  0,
								  gtk_marshal_NONE__NONE,
								  GTK_TYPE_NONE, 
								  0);
	
	gtk_object_class_add_signals (object_class, image_chooser_signals, LAST_SIGNAL);

	/* Make this class inherit the same kind of theme stuff as GtkLabel */
	eel_gtk_class_name_make_like_existing_type ("EelImageChooser", GTK_TYPE_CLIST);
}

static void
eel_image_chooser_initialize (EelImageChooser *image_chooser)
{
	GTK_WIDGET_UNSET_FLAGS (image_chooser, GTK_NO_WINDOW);

	image_chooser->details = g_new0 (EelImageChooserDetails, 1);
	image_chooser->details->row_box = gtk_vbox_new (FALSE, 0);

 	gtk_container_add (GTK_CONTAINER (image_chooser), image_chooser->details->row_box); 
   	gtk_widget_show_all (image_chooser->details->row_box);

	gtk_widget_set_events (GTK_WIDGET (image_chooser),
			       GDK_BUTTON_MOTION_MASK
 			       | GDK_BUTTON_PRESS_MASK
 			       | GDK_BUTTON_RELEASE_MASK
			       | GDK_EXPOSURE_MASK
			       | GDK_ENTER_NOTIFY_MASK
			       | GDK_LEAVE_NOTIFY_MASK
			       | GDK_POINTER_MOTION_MASK);
}

/* GtkObjectClass methods */
static void
eel_image_chooser_destroy (GtkObject* object)
{
	EelImageChooser *image_chooser;
	
	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (object));
	
	image_chooser = EEL_IMAGE_CHOOSER (object);

	if (image_chooser->details->clear_gc != NULL) {
		gdk_gc_unref (image_chooser->details->clear_gc);
		image_chooser->details->clear_gc = NULL;
	}

	eel_image_chooser_clear (image_chooser);
	g_free (image_chooser->details);

	/* Chain */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/* GtkWidgetClass methods */
static int
image_chooser_expose_event (GtkWidget *widget,
			   GdkEventExpose *event)
{
	EelImageChooser *image_chooser;
	GtkBin *bin;
	GdkEventExpose child_event;
	ArtIRect dirty_area;
	
	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	image_chooser = EEL_IMAGE_CHOOSER (widget);
	
	bin = GTK_BIN (widget);

	dirty_area = eel_gdk_rectangle_to_art_irect (event->area);

	gdk_draw_rectangle (GTK_WIDGET (image_chooser)->window,
			    GTK_WIDGET (image_chooser)->style->base_gc[GTK_STATE_NORMAL],
			    TRUE,
			    dirty_area.x0,
			    dirty_area.y0,
			    eel_art_irect_get_width (dirty_area),
			    eel_art_irect_get_height (dirty_area));
	
	child_event = *event;
	if (bin->child &&
	    GTK_WIDGET_NO_WINDOW (bin->child) &&
	    gtk_widget_intersect (bin->child, &event->area, &child_event.area)) {
		gtk_widget_event (bin->child, (GdkEvent*) &child_event);
	}

	if (image_chooser->details->row_being_browsed != NULL) {
		image_chooser_paint_row_selected (image_chooser,
						 dirty_area,
						 image_chooser_row_to_position (image_chooser,
									       image_chooser->details->row_being_browsed));
		return TRUE;
	}
	
	if (image_chooser->details->selected_row != NULL) {
		image_chooser_paint_row_selected (image_chooser,
						 dirty_area,
						 image_chooser_row_to_position (image_chooser,
									       image_chooser->details->selected_row));
	}

	return TRUE;
}

static void
image_chooser_style_set (GtkWidget *widget,
			GtkStyle *previous_style)
{
	EelImageChooser *image_chooser;
	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (widget));

	image_chooser = EEL_IMAGE_CHOOSER (widget);

	EEL_CALL_PARENT (GTK_WIDGET_CLASS, style_set, (widget, previous_style));

	if (GTK_WIDGET_REALIZED (widget)) {
		gtk_style_set_background (widget->style, widget->window, widget->state);
		gdk_window_set_background (widget->window, &widget->style->base[GTK_STATE_NORMAL]);
	}
	
	image_chooser->details->background_color = 
		eel_gdk_color_to_rgb (&widget->style->base[GTK_STATE_NORMAL]);

	image_chooser->details->selected_background_color = 
		eel_gdk_color_to_rgb (&widget->style->bg[GTK_STATE_SELECTED]);
	
	image_chooser->details->selected_text_color = 
		eel_gdk_color_to_rgb (&widget->style->fg[GTK_STATE_SELECTED]);

	image_chooser->details->text_color = 
		eel_gdk_color_to_rgb (&widget->style->text[GTK_STATE_NORMAL]);

	image_chooser_update_colors (image_chooser);
}

static void
image_chooser_browsing_event (EelImageChooser *image_chooser,
			     int x,
			     int y)
{
	ImageChooserRow *row;
	int row_index;
	ArtIRect row_bounds;

	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));

	row = image_chooser_find_row_at_point (image_chooser, x, y);

	if (image_chooser->details->row_being_browsed == row) {
		return;
	}

	if (image_chooser->details->row_being_browsed != NULL) {
		row_bounds = eel_gtk_widget_get_bounds (image_chooser->details->row_being_browsed->hbox);
		image_chooser_paint_row_unselected (image_chooser,
						   row_bounds,
						   image_chooser_row_to_position (image_chooser,
										 image_chooser->details->row_being_browsed));
	}
	image_chooser->details->row_being_browsed = NULL;

	if (row == NULL) {
		return;
	}

	row_index = image_chooser_row_to_position (image_chooser, row);		
	g_return_if_fail (row_index >= 0);
	g_return_if_fail ((guint)row_index < eel_image_chooser_get_num_rows (image_chooser));

	row_bounds = eel_gtk_widget_get_bounds (row->hbox);
	image_chooser_paint_row_selected (image_chooser, row_bounds, row_index);
	image_chooser->details->row_being_browsed = row;
}

static int
image_chooser_motion_notify_event (GtkWidget *widget,
				  GdkEventMotion *event)
{
	EelImageChooser *image_chooser;
	ArtIRect bin_window_bounds;
	ArtIRect scrolled_window_bounds;
	GtkWidget *viewport;
	int browsed_row_offset;
	
	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	image_chooser = EEL_IMAGE_CHOOSER (widget);

	if (image_chooser->details->auto_scroll_timeout == 0) {
		return FALSE;
	}

	if (widget->window != event->window) {
		return FALSE;
	}

	if (image_chooser->details->auto_scroll_direction == AUTO_SCROLL_UP
	    || image_chooser->details->auto_scroll_direction == AUTO_SCROLL_DOWN) {
		viewport = GTK_BIN (image_chooser->details->scrolled_window)->child;
		
		bin_window_bounds = eel_gdk_window_get_bounds (GTK_VIEWPORT (viewport)->bin_window);
		scrolled_window_bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (image_chooser->details->scrolled_window));
		browsed_row_offset = 0;

		if (image_chooser->details->auto_scroll_direction == AUTO_SCROLL_UP) {
			browsed_row_offset = ABS (bin_window_bounds.y0) + 1;
			image_chooser_browsing_event (image_chooser, 1, browsed_row_offset);
			return TRUE;
		} else if (image_chooser->details->auto_scroll_direction == AUTO_SCROLL_DOWN) {
			browsed_row_offset = ABS (bin_window_bounds.y0) + eel_art_irect_get_height (scrolled_window_bounds) - 1;
			image_chooser_browsing_event (image_chooser, 1, browsed_row_offset);
			return TRUE;
		}
	}

	image_chooser_browsing_event (image_chooser, event->x, event->y);

	return TRUE;
}

static int
image_chooser_button_press_event (GtkWidget *widget,
				  GdkEventButton *event)
{
	EelImageChooser *image_chooser;
	ImageChooserRow *row;
	ArtIRect row_area;
	GdkEventMask mask;

	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	image_chooser = EEL_IMAGE_CHOOSER (widget);

	mask = ((1 << (4 + event->button)) |
		GDK_POINTER_MOTION_HINT_MASK |
		GDK_BUTTON_RELEASE_MASK);

	if (gdk_pointer_grab (widget->window,
			      FALSE,
			      mask,
			      NULL, NULL,
			      event->time) != 0) {
		return FALSE;
	}

	gtk_grab_add (widget);
	
	row = image_chooser_find_row_at_point (image_chooser, event->x, event->y);
	if (row != image_chooser->details->selected_row && image_chooser->details->selected_row != NULL) {
		row_area = eel_gtk_widget_get_bounds (GTK_WIDGET (image_chooser));
		image_chooser_paint_row_unselected (image_chooser,
						   row_area,
						   image_chooser_row_to_position (image_chooser,
										 image_chooser->details->selected_row));
	}

	image_chooser_browsing_event (image_chooser, event->x, event->y);

	image_chooser->details->auto_scroll_timeout = gtk_timeout_add (AUTO_SCROLL_TIMEOUT,
								      image_chooser_auto_scroll_timeout,
								      image_chooser);
	
	return TRUE;
}

static int
image_chooser_button_release_event (GtkWidget *widget,
				    GdkEventButton *event)
{
	EelImageChooser *image_chooser;
	
	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	image_chooser = EEL_IMAGE_CHOOSER (widget);

	gtk_grab_remove (widget);
	gdk_pointer_ungrab (event->time);

	image_chooser->details->auto_scroll_direction = AUTO_SCROLL_NOPE;
	gtk_timeout_remove (image_chooser->details->auto_scroll_timeout);
	image_chooser->details->auto_scroll_timeout = 0;

	if (widget->window != event->window) {
		return FALSE;
	}
	
	if (image_chooser->details->row_being_browsed != NULL) {
		eel_image_chooser_set_selected_row (image_chooser,
						    image_chooser_row_to_position (image_chooser, 
										   image_chooser->details->row_being_browsed));
	} else {
		eel_image_chooser_set_selected_row (image_chooser, 0);
	}
	
	image_chooser->details->row_being_browsed = NULL;

	gtk_widget_queue_draw (GTK_WIDGET (image_chooser));

	return TRUE;
}

static int
image_chooser_row_to_position (const EelImageChooser *image_chooser,
			      const ImageChooserRow *row)
{
	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser), IMAGE_CHOOSER_INVALID_INDEX);
	g_return_val_if_fail (row != NULL, IMAGE_CHOOSER_INVALID_INDEX);
	
	return g_list_index (image_chooser->details->rows, (gpointer) row);
}

static ImageChooserRow *
image_chooser_position_to_row (const EelImageChooser *image_chooser,
			      int position)
{
	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser), NULL);

	if (position < 0) {
		return NULL;
	}

	if (position >= (int)eel_image_chooser_get_num_rows (image_chooser)) {
		return NULL;
	}

	return g_list_nth_data (image_chooser->details->rows, position);
}

static GdkGC *
image_chooser_peek_clear_gc (EelImageChooser *image_chooser)
{
	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser), NULL);

	if (image_chooser->details->clear_gc == NULL) {
		image_chooser->details->clear_gc = gdk_gc_new (GTK_WIDGET (image_chooser)->window);
		gdk_gc_set_function (image_chooser->details->clear_gc, GDK_COPY);
	}

	gdk_rgb_gc_set_foreground (image_chooser->details->clear_gc, image_chooser->details->background_color);

	return image_chooser->details->clear_gc;
}

static EelRegion *
image_chooser_get_row_negative (const EelImageChooser *image_chooser,
			       const ImageChooserRow *row)
{
	ArtIRect row_bounds;
	ArtIRect icon_bounds;
	ArtIRect title_bounds;
	ArtIRect description_bounds;
	EelRegion *negative;

	ArtIRect text_bounds;

	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser), NULL);
	g_return_val_if_fail (row != NULL, NULL);

	row_bounds = eel_gtk_widget_get_bounds (row->hbox);
	icon_bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (row->icon));
	title_bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (row->title));
	description_bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (row->description));

	art_irect_union (&text_bounds, &title_bounds, &description_bounds);

	negative = eel_region_new ();
	eel_region_add_rectangle (negative, row_bounds);

	return negative;
}

static void
image_chooser_paint_row_selected (EelImageChooser *image_chooser,
				 ArtIRect area,
				 int row_index)
{
	ArtIRect row_bounds;
	ImageChooserRow *row;
	ArtIRect final_area;

	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));
	g_return_if_fail (row_index >= 0);
	g_return_if_fail ((guint)row_index < eel_image_chooser_get_num_rows (image_chooser));

	row = image_chooser_position_to_row (image_chooser, row_index);
	g_return_if_fail (row != NULL);

	row_bounds = eel_gtk_widget_get_bounds (row->hbox);

	art_irect_intersect (&final_area, &area, &row_bounds);
 	if (art_irect_empty (&final_area)) {
		return;
	}
	gdk_draw_rectangle (GTK_WIDGET (image_chooser)->window,
			    GTK_WIDGET (image_chooser)->style->bg_gc[GTK_STATE_SELECTED],
			    TRUE,
			    final_area.x0,
			    final_area.y0,
			    eel_art_irect_get_width (final_area),
			    eel_art_irect_get_height (final_area));

	image_chooser_set_row_colors (image_chooser, row, TRUE);

	{
		GdkRectangle r;

		r.x = row->hbox->allocation.x;
		r.y = row->hbox->allocation.y;
		r.width = row->hbox->allocation.width;
		r.height = row->hbox->allocation.height;
		gtk_widget_draw (row->hbox, &r);
	}
}

static void
image_chooser_paint_row_unselected (EelImageChooser *image_chooser,
				   ArtIRect area,
				   int row_index)
{
	GtkWidget *widget;
	ArtIRect row_bounds;
	ImageChooserRow *row;
	ArtIRect final_area;
	GdkGC *gc;
	EelRegion *negative;

	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));
	g_return_if_fail (row_index >= 0);
	g_return_if_fail ((guint)row_index < eel_image_chooser_get_num_rows (image_chooser));

	widget = GTK_WIDGET (image_chooser);

	row = image_chooser_position_to_row (image_chooser, row_index);
	g_return_if_fail (row != NULL);

	row_bounds = eel_gtk_widget_get_bounds (row->hbox);

	art_irect_intersect (&final_area, &area, &row_bounds);
	
 	if (art_irect_empty (&final_area)) {
		return;
	}
 	gc = image_chooser_peek_clear_gc (image_chooser);

	negative = image_chooser_get_row_negative (image_chooser, row);
	g_return_if_fail (negative != NULL);

 	eel_region_set_gc_clip_region (negative, gc);
	
	gdk_draw_rectangle (widget->window,
			    gc,
			    TRUE,
			    final_area.x0,
			    final_area.y0,
			    eel_art_irect_get_width (final_area),
			    eel_art_irect_get_height (final_area));
	
	eel_region_free (negative);

	image_chooser_set_row_colors (image_chooser, row, FALSE);
	
	{
		GdkRectangle r;

		r.x = row->hbox->allocation.x;
		r.y = row->hbox->allocation.y;
		r.width = row->hbox->allocation.width;
		r.height = row->hbox->allocation.height;
		gtk_widget_draw (row->hbox, &r);
	}
}

static ImageChooserRow *
image_chooser_find_row_at_point (const EelImageChooser *image_chooser,
				int x,
				int y)
{
	GList *node;
	ImageChooserRow *row;
	ArtIRect row_bounds;
	ArtIRect image_chooser_bounds;
	EelDimensions column_dimensions;

  	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser), NULL);

	image_chooser_bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (image_chooser));

	if (eel_image_chooser_get_num_rows (image_chooser) == 0) {
		return NULL;
	}

	column_dimensions = image_chooser_get_partial_dimensions (image_chooser,
								 eel_image_chooser_get_num_rows (image_chooser));

	if (y < image_chooser_bounds.y0) {
		return image_chooser_position_to_row (image_chooser, 0);
	} else if (y > column_dimensions.height) {
		return image_chooser_position_to_row (image_chooser, eel_image_chooser_get_num_rows (image_chooser) - 1);
	}
	
	for (node = image_chooser->details->rows; node != NULL; node = node->next) {
		g_assert (node->data != NULL);

		row = node->data;
		
		if (GTK_WIDGET_VISIBLE (row->hbox)) {
			row_bounds = eel_gtk_widget_get_bounds (row->hbox);

 			if (y >= row_bounds.y0 && y <= row_bounds.y1) {
				return row;
			}
		}
	}

	return NULL;
}

static void
image_chooser_set_row_colors (EelImageChooser *image_chooser,
			     ImageChooserRow *row,
			     gboolean selected)
{
  	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));
  	g_return_if_fail (row != NULL);

	if (selected) {
		eel_image_set_solid_background_color (row->icon, image_chooser->details->selected_background_color);
		eel_label_set_solid_background_color (row->title, image_chooser->details->selected_background_color);
		eel_label_set_solid_background_color (row->description, image_chooser->details->selected_background_color);
		eel_label_set_text_color (row->title, image_chooser->details->selected_text_color);
		eel_label_set_text_color (row->description, image_chooser->details->selected_text_color);
	} else {
		eel_image_set_solid_background_color (row->icon, image_chooser->details->background_color);
		eel_label_set_solid_background_color (row->title, image_chooser->details->background_color);
		eel_label_set_solid_background_color (row->description, image_chooser->details->background_color);
		eel_label_set_text_color (row->title, image_chooser->details->text_color);
		eel_label_set_text_color (row->description, image_chooser->details->text_color);
	}
}

static void
image_chooser_update_colors (EelImageChooser *image_chooser)
{
	GList *node;
	ImageChooserRow *row;
	gboolean selected;

  	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));

	for (node = image_chooser->details->rows; node != NULL; node = node->next) {
		g_assert (node->data != NULL);
		
		row = node->data;

		if (image_chooser->details->row_being_browsed != NULL) {
			selected = row == image_chooser->details->row_being_browsed;
		} else {
			selected = row == image_chooser->details->selected_row;
		}
		image_chooser_set_row_colors (image_chooser, row, selected);
	}
}

static int
image_chooser_auto_scroll_timeout (gpointer callback_data)
{
	EelImageChooser *image_chooser;
	ArtIRect visible_image_chooser_area;
	ArtIRect clipped_visible_image_chooser_area;
	EelDimensions screen_dimensions;
	ArtIRect screen_bounds;
	EelArtIPoint pointer_position;
	GtkWidget *viewport;
	GtkAdjustment *vadjustment;
	float new_value;
	EelDimensions row_dimensions;

	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (callback_data), FALSE);

	image_chooser = EEL_IMAGE_CHOOSER (callback_data);

	viewport = 
		image_chooser->details->scrolled_window != NULL ?
		GTK_BIN (image_chooser->details->scrolled_window)->child :
		NULL;

	if (viewport == NULL) {
		return FALSE;
	}

	visible_image_chooser_area = eel_gdk_window_get_screen_relative_bounds (GTK_VIEWPORT (viewport)->view_window);
	pointer_position = eel_gdk_get_pointer_position ();
	screen_dimensions = eel_screen_get_dimensions ();

	row_dimensions = image_chooser_get_partial_dimensions (image_chooser, 1);
	screen_bounds = eel_art_irect_assign (0, 0, screen_dimensions.width, screen_dimensions.height);

	art_irect_intersect (&clipped_visible_image_chooser_area, &screen_bounds, &visible_image_chooser_area);
	if (!art_irect_empty (&clipped_visible_image_chooser_area)) {
		vadjustment = gtk_scrolled_window_get_vadjustment (image_chooser->details->scrolled_window);
		new_value = vadjustment->value;

		image_chooser->details->auto_scroll_direction = AUTO_SCROLL_NOPE;
		if (pointer_position.y <= clipped_visible_image_chooser_area.y0) {
			new_value = vadjustment->value - row_dimensions.height;
			new_value = MAX (new_value, 0.0);
			image_chooser->details->auto_scroll_direction = AUTO_SCROLL_UP;
		} else if (pointer_position.y >= clipped_visible_image_chooser_area.y1) {
			new_value = vadjustment->value + row_dimensions.height;
			new_value = MIN (new_value, vadjustment->upper - vadjustment->page_size);
			image_chooser->details->auto_scroll_direction = AUTO_SCROLL_DOWN;
		}

//	new_value = CLAMP (row_bounds.y0 - center_offset, 0.0, vadjustment->upper - vadjustment->page_size);


		/* If the value changed, update the adjustment and
		 * send the icon list a synthetic motion event so that
		 * it browses to the right row */
		if (vadjustment->value != new_value) {
			vadjustment->value = new_value;
			gtk_adjustment_value_changed (vadjustment);

			eel_image_chooser_synthetic_motion (image_chooser,
							   pointer_position.x,
							   pointer_position.y);
		}
	}

	return TRUE;
}

/*
 * EelImageChooser public methods
 */
GtkWidget *
eel_image_chooser_new (void)
{
	return gtk_widget_new (EEL_TYPE_IMAGE_CHOOSER, NULL);
}

void
eel_image_chooser_insert_row (EelImageChooser *image_chooser,
			     GdkPixbuf *pixbuf,
			     const char *title,
			     const char *description,
			     gpointer row_data,
			     GFreeFunc row_data_free_func)
{
	ImageChooserRow *row;
	
	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));
	g_return_if_fail (eel_gdk_pixbuf_is_valid (pixbuf));
	g_return_if_fail (title != NULL);
	g_return_if_fail (description != NULL);

	row = g_new0 (ImageChooserRow, 1);

	row->icon = EEL_IMAGE (eel_image_new_solid (pixbuf,
						    0.5,
						    0.5,
						    IMAGE_CHOOSER_ROW_BORDER,
						    IMAGE_CHOOSER_ROW_BORDER,
						    image_chooser->details->background_color,
						    NULL));
	eel_image_set_pixbuf (row->icon, pixbuf);
	row->title = EEL_LABEL (eel_label_new_solid (title,
						     0,
						     0,
						     image_chooser->details->text_color,
						     0.0,
						     0.5,
						     IMAGE_CHOOSER_ROW_BORDER / 2,
						     IMAGE_CHOOSER_ROW_BORDER / 2,
						     image_chooser->details->background_color,
						     NULL));
	eel_label_set_never_smooth (row->title, TRUE);
	row->description = EEL_LABEL (eel_label_new_solid (description,
							   0,
							   0,
							   image_chooser->details->text_color,
							   0.0,
							   0.5,
							   IMAGE_CHOOSER_ROW_BORDER / 2,
							   IMAGE_CHOOSER_ROW_BORDER / 2,
							   image_chooser->details->background_color,
							   NULL));
	row->row_data = row_data;
	row->row_data_free_func = row_data_free_func;
	
	eel_label_set_never_smooth (row->description, TRUE);
	
	row->hbox = gtk_hbox_new (FALSE, 0);
	row->text_column = gtk_vbox_new (FALSE, 0);
	
	eel_label_make_larger (row->title, 2);
	eel_label_make_bold (row->title);

	gtk_box_pack_start (GTK_BOX (row->text_column), GTK_WIDGET (row->title), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (row->text_column), GTK_WIDGET (row->description), FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (row->hbox), GTK_WIDGET (row->icon), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (row->hbox), GTK_WIDGET (row->text_column), FALSE, FALSE, 8);

	gtk_widget_show_all (row->hbox);

 	gtk_box_pack_start (GTK_BOX (image_chooser->details->row_box), row->hbox, FALSE, FALSE, 0);
	
	image_chooser->details->rows = g_list_append (image_chooser->details->rows, row);
}

int
eel_image_chooser_get_selected_row (const EelImageChooser *image_chooser)
{
	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser), IMAGE_CHOOSER_INVALID_INDEX);

	if (image_chooser->details->selected_row == NULL) {
		return IMAGE_CHOOSER_INVALID_INDEX;
	}

	return g_list_index (image_chooser->details->rows, image_chooser->details->selected_row);
}

gpointer
eel_image_chooser_get_row_data (const EelImageChooser *image_chooser,
			       guint row_index)
{
	ImageChooserRow *row;

	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser), NULL);
	g_return_val_if_fail (row_index >= 0, NULL);
	g_return_val_if_fail ((guint)row_index < eel_image_chooser_get_num_rows (image_chooser), NULL);

	row = image_chooser_position_to_row (image_chooser, row_index);
	g_return_val_if_fail (row != NULL, NULL);

	return row->row_data;
}

void
eel_image_chooser_set_selected_row (EelImageChooser *image_chooser,
				   int icon_position)
{
	ImageChooserRow *new_selected_row;
	ArtIRect row_bounds;

	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));

	new_selected_row = (icon_position >= 0) ? image_chooser_position_to_row (image_chooser, icon_position) : NULL;
	if (new_selected_row == image_chooser->details->selected_row) {
		return;
	}

	if (image_chooser->details->selected_row != NULL) {
		row_bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (image_chooser));

		if (GTK_WIDGET_REALIZED (image_chooser)) {
			image_chooser_paint_row_unselected (image_chooser,
							   row_bounds,
							   image_chooser_row_to_position (image_chooser,
											 image_chooser->details->selected_row));
		}
	}

	image_chooser->details->selected_row = new_selected_row;
	image_chooser_update_colors (image_chooser);
	gtk_widget_queue_draw (GTK_WIDGET (image_chooser));

#if 0
	if (image_chooser->details->scrolled_window != NULL) {
		eel_scrolled_image_chooser_show_selected_row (image_chooser,
							     GTK_WIDGET (image_chooser->details->scrolled_window));
	}
#endif
	
	gtk_signal_emit (GTK_OBJECT (image_chooser), image_chooser_signals[SELECTION_CHANGED]);
}

guint
eel_image_chooser_get_num_rows (const EelImageChooser *image_chooser)
{
	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser), 0);

	return g_list_length (image_chooser->details->rows);
}

void
eel_image_chooser_clear (EelImageChooser *image_chooser)
{
	GList *node;
	ImageChooserRow *row;

	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));
	
	for (node = image_chooser->details->rows; node != NULL; node = node->next) {
		g_assert (node->data != NULL);
		row = node->data;

		if (row->row_data_free_func != NULL) {
			(* row->row_data_free_func) (row->row_data);
		}

		gtk_widget_destroy (row->hbox);
		g_free (row);
	}
	g_list_free (image_chooser->details->rows);
	image_chooser->details->rows = NULL;

	image_chooser->details->selected_row = NULL;
	image_chooser->details->row_being_browsed = NULL;

	gtk_widget_queue_resize (GTK_WIDGET (image_chooser));
}

static EelDimensions
image_chooser_get_partial_dimensions (const EelImageChooser *image_chooser,
				     guint num_rows)
{
	GList *node;
	ImageChooserRow *row;
	EelDimensions partial_dimensions;
	EelDimensions row_dimensions;
	guint i;

	g_return_val_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser), eel_dimensions_empty);
	g_return_val_if_fail (num_rows >= 0, eel_dimensions_empty);
	g_return_val_if_fail (num_rows <= eel_image_chooser_get_num_rows (image_chooser), eel_dimensions_empty);

	partial_dimensions = eel_dimensions_empty;
	for (node = image_chooser->details->rows, i = 0;
	     node != NULL && i < num_rows;
	     node = node->next, i++) {
		g_assert (node->data != NULL);
		
		row = node->data;

		row_dimensions = eel_gtk_widget_get_preferred_dimensions (row->hbox);

		partial_dimensions.width = MAX (partial_dimensions.width, row_dimensions.width);
		partial_dimensions.height += row_dimensions.height;
	}

	return partial_dimensions;
}

void
eel_image_chooser_synthetic_motion (EelImageChooser *image_chooser,
				   int x,
				   int y)
{
	GdkEventMotion motion_event = { 0 };
	GtkWidget *widget;

	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));

	widget = GTK_WIDGET (image_chooser);
	
	motion_event.type = GDK_MOTION_NOTIFY;
	motion_event.send_event = TRUE;
	motion_event.window = widget->window;
	motion_event.x = x;
	motion_event.y = y;
	
	gdk_window_ref (motion_event.window);
	gtk_widget_event (widget, (GdkEvent*) &motion_event);
	gdk_window_unref (motion_event.window);
	
	gtk_widget_queue_draw (widget);
}

GtkWidget *
eel_scrolled_image_chooser_new (GtkWidget **image_chooser_out)
{
	GtkWidget *scrolled_window;
	GtkWidget *image_chooser;

	g_return_val_if_fail (image_chooser_out != NULL, NULL);

 	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
 	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
 					GTK_POLICY_NEVER,
 					GTK_POLICY_AUTOMATIC);
	image_chooser = eel_image_chooser_new ();
	
 	eel_gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window),
 						   image_chooser);
	
	gtk_widget_show_all (scrolled_window);
	gtk_widget_hide (scrolled_window);

	EEL_IMAGE_CHOOSER (image_chooser)->details->scrolled_window = GTK_SCROLLED_WINDOW (scrolled_window);
	*image_chooser_out = image_chooser;

	return scrolled_window;
}

void
eel_scrolled_image_chooser_set_num_visible_rows (EelImageChooser *image_chooser,
						GtkWidget *scrolled_window,
						guint num_visible_rows)
{
	EelDimensions row_dimensions;
	EelDimensions scroller_dimensions;

	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));
	g_return_if_fail (GTK_IS_SCROLLED_WINDOW (scrolled_window));
	g_return_if_fail (image_chooser->details->scrolled_window == GTK_SCROLLED_WINDOW (scrolled_window));
	g_return_if_fail (num_visible_rows > 0);

	if (eel_image_chooser_get_num_rows (image_chooser) == 0) {
		return;
	}

	num_visible_rows = MIN (num_visible_rows, eel_image_chooser_get_num_rows (image_chooser));

	row_dimensions = image_chooser_get_partial_dimensions (image_chooser,
							      num_visible_rows);
	
	scroller_dimensions = eel_gtk_widget_get_preferred_dimensions (scrolled_window);

	gtk_widget_set_usize (scrolled_window, -1, row_dimensions.height);
}

void
eel_scrolled_image_chooser_show_selected_row (EelImageChooser *image_chooser,
					     GtkWidget *scrolled_window)
{
	ArtIRect row_bounds;
	ArtIRect viewport_bounds;
	GtkWidget *viewport;
	float new_value;
	GtkAdjustment *vadjustment;
	int center_offset;

	g_return_if_fail (EEL_IS_IMAGE_CHOOSER (image_chooser));
	g_return_if_fail (GTK_IS_SCROLLED_WINDOW (scrolled_window));
	g_return_if_fail (image_chooser->details->scrolled_window == GTK_SCROLLED_WINDOW (scrolled_window));

	if (image_chooser->details->selected_row == NULL) {
		return;
	}

	viewport = GTK_BIN (image_chooser->details->scrolled_window)->child;

	row_bounds = eel_gtk_widget_get_bounds (image_chooser->details->selected_row->hbox);
	viewport_bounds = eel_gtk_widget_get_bounds (viewport);

	vadjustment = gtk_scrolled_window_get_vadjustment (image_chooser->details->scrolled_window);

	center_offset = (eel_art_irect_get_height (viewport_bounds) - eel_art_irect_get_height (row_bounds)) / 2;

	new_value = CLAMP (row_bounds.y0 - center_offset, 0.0, vadjustment->upper - vadjustment->page_size);
	
	if (vadjustment->value != new_value) {
		vadjustment->value = new_value;
		gtk_adjustment_value_changed (vadjustment);
	}
}
