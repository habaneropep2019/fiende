/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-labeled-image.c - A labeled image.

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
#include "eel-labeled-image.h"

#include "eel-art-extensions.h"
#include "eel-art-gtk-extensions.h"
#include "eel-debug-drawing.h"
#include "eel-gtk-container.h"
#include "eel-gtk-extensions.h"
#include "eel-gtk-macros.h"
#include "eel-image.h"
#include "eel-label.h"
#include <gtk/gtkbutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtktogglebutton.h>

#define DEFAULT_SPACING 0
#define DEFAULT_X_PADDING 0
#define DEFAULT_Y_PADDING 0
#define DEFAULT_X_ALIGNMENT 0.5
#define DEFAULT_Y_ALIGNMENT 0.5

/* Arguments */
enum
{
	ARG_0,
	ARG_FILL,
	ARG_LABEL,
	ARG_LABEL_POSITION,
	ARG_PIXBUF,
	ARG_SHOW_IMAGE,
	ARG_SHOW_LABEL,
	ARG_SPACING,
	ARG_X_ALIGNMENT,
	ARG_X_PADDING,
	ARG_Y_ALIGNMENT,
	ARG_Y_PADDING,
};

/* Detail member struct */
struct EelLabeledImageDetails
{
	GtkWidget *image;
	GtkWidget *label;
	GtkPositionType label_position;
	gboolean show_label;
	gboolean show_image;
	guint spacing;
	float x_alignment;
	float y_alignment;
	int x_padding;
	int y_padding;
	int fixed_image_height;
	gboolean fill;
};

/* GtkObjectClass methods */
static void          eel_labeled_image_initialize_class   (EelLabeledImageClass  *labeled_image_class);
static void          eel_labeled_image_initialize         (EelLabeledImage       *image);
static void          eel_labeled_image_destroy            (GtkObject             *object);
static void          eel_labeled_image_set_arg            (GtkObject             *object,
							   GtkArg                *arg,
							   guint                  arg_id);
static void          eel_labeled_image_get_arg            (GtkObject             *object,
							   GtkArg                *arg,
							   guint                  arg_id);
/* GtkWidgetClass methods */
static void          eel_labeled_image_size_request       (GtkWidget             *widget,
							   GtkRequisition        *requisition);
static int           eel_labeled_image_expose_event       (GtkWidget             *widget,
							   GdkEventExpose        *event);
static void          eel_labeled_image_draw               (GtkWidget             *widget,
							   GdkRectangle          *area);
static void          eel_labeled_image_size_allocate      (GtkWidget             *widget,
							   GtkAllocation         *allocation);
static void          eel_labeled_image_map                (GtkWidget             *widget);
static void          eel_labeled_image_unmap              (GtkWidget             *widget);

/* GtkContainerClass methods */
static void          eel_labeled_image_add                (GtkContainer          *container,
							   GtkWidget             *widget);
static void          eel_labeled_image_remove             (GtkContainer          *container,
							   GtkWidget             *widget);
static void          eel_labeled_image_forall             (GtkContainer          *container,
							   gboolean               include_internals,
							   GtkCallback            callback,
							   gpointer               callback_data);

/* Private EelLabeledImage methods */
static EelDimensions labeled_image_get_image_dimensions   (const EelLabeledImage *labeled_image);
static EelDimensions labeled_image_get_label_dimensions   (const EelLabeledImage *labeled_image);
static void          labeled_image_ensure_label           (EelLabeledImage       *labeled_image);
static void          labeled_image_ensure_image           (EelLabeledImage       *labeled_image);
static ArtIRect      labeled_image_get_content_bounds     (const EelLabeledImage *labeled_image);
static EelDimensions labeled_image_get_content_dimensions (const EelLabeledImage *labeled_image);
static void          labeled_image_update_alignments      (EelLabeledImage       *labeled_image);
static gboolean      labeled_image_show_label             (const EelLabeledImage *labeled_image);
static gboolean      labeled_image_show_image             (const EelLabeledImage *labeled_image);

EEL_DEFINE_CLASS_BOILERPLATE (EelLabeledImage, eel_labeled_image, GTK_TYPE_CONTAINER)

/* Class init methods */
	static void
eel_labeled_image_initialize_class (EelLabeledImageClass *labeled_image_class)
{
	GtkObjectClass *object_class = GTK_OBJECT_CLASS (labeled_image_class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (labeled_image_class);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (labeled_image_class);

	/* GtkObjectClass */
	object_class->destroy = eel_labeled_image_destroy;
	object_class->set_arg = eel_labeled_image_set_arg;
	object_class->get_arg = eel_labeled_image_get_arg;
	
 	/* GtkWidgetClass */
 	widget_class->size_request = eel_labeled_image_size_request;
	widget_class->size_allocate = eel_labeled_image_size_allocate;
 	widget_class->expose_event = eel_labeled_image_expose_event;
 	widget_class->draw = eel_labeled_image_draw;
	widget_class->map = eel_labeled_image_map;
	widget_class->unmap = eel_labeled_image_unmap;

 	/* GtkContainerClass */
	container_class->add = eel_labeled_image_add;
	container_class->remove = eel_labeled_image_remove;
	container_class->forall = eel_labeled_image_forall;
  
	/* Arguments */
	gtk_object_add_arg_type ("EelLabeledImage::pixbuf",
				 GTK_TYPE_POINTER,
				 GTK_ARG_READWRITE,
				 ARG_PIXBUF);
	gtk_object_add_arg_type ("EelLabeledImage::label",
				 GTK_TYPE_STRING,
				 GTK_ARG_READWRITE,
				 ARG_LABEL);
	gtk_object_add_arg_type ("EelLabeledImage::label_position",
				 GTK_TYPE_POSITION_TYPE,
				 GTK_ARG_READWRITE,
				 ARG_LABEL_POSITION);
	gtk_object_add_arg_type ("EelLabeledImage::show_label",
				 GTK_TYPE_BOOL,
				 GTK_ARG_READWRITE,
				 ARG_SHOW_LABEL);
	gtk_object_add_arg_type ("EelLabeledImage::show_image",
				 GTK_TYPE_BOOL,
				 GTK_ARG_READWRITE,
				 ARG_SHOW_IMAGE);
	gtk_object_add_arg_type ("EelLabeledImage::spacing",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE,
				 ARG_SPACING);
	gtk_object_add_arg_type ("EelLabeledImage::x_padding",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_X_PADDING);
	gtk_object_add_arg_type ("EelLabeledImage::y_padding",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_Y_PADDING);
	gtk_object_add_arg_type ("EelLabeledImage::x_alignment",
				 GTK_TYPE_FLOAT,
				 GTK_ARG_READWRITE,
				 ARG_X_ALIGNMENT);
	gtk_object_add_arg_type ("EelLabeledImage::y_alignment",
				 GTK_TYPE_FLOAT,
				 GTK_ARG_READWRITE,
				 ARG_Y_ALIGNMENT);
	gtk_object_add_arg_type ("EelLabeledImage::fill",
				 GTK_TYPE_BOOL,
				 GTK_ARG_READWRITE,
				 ARG_FILL);
}

void
eel_labeled_image_initialize (EelLabeledImage *labeled_image)
{
	GTK_WIDGET_SET_FLAGS (labeled_image, GTK_NO_WINDOW);

	labeled_image->details = g_new0 (EelLabeledImageDetails, 1);
	labeled_image->details->show_label = TRUE;
	labeled_image->details->show_image = TRUE;
	labeled_image->details->label_position = GTK_POS_BOTTOM;
	labeled_image->details->spacing = DEFAULT_SPACING;
	labeled_image->details->x_padding = DEFAULT_X_PADDING;
	labeled_image->details->y_padding = DEFAULT_Y_PADDING;
	labeled_image->details->x_alignment = DEFAULT_X_ALIGNMENT;
	labeled_image->details->y_alignment = DEFAULT_Y_ALIGNMENT;
	labeled_image->details->fixed_image_height = 0;

	eel_labeled_image_set_fill (labeled_image, FALSE);
}

/* GtkObjectClass methods */
static void
eel_labeled_image_destroy (GtkObject *object)
{
 	EelLabeledImage *labeled_image;
	
	g_return_if_fail (EEL_IS_LABELED_IMAGE (object));

	labeled_image = EEL_LABELED_IMAGE (object);

	if (labeled_image->details->image != NULL) {
		gtk_widget_destroy (labeled_image->details->image);
		labeled_image->details->image = NULL;
	}

	if (labeled_image->details->label != NULL) {
		gtk_widget_destroy (labeled_image->details->label);
		labeled_image->details->label = NULL;
	}

	g_free (labeled_image->details);

	/* Chain destroy */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

static void
eel_labeled_image_set_arg (GtkObject *object,
			   GtkArg *arg,
			   guint arg_id)
{
	EelLabeledImage *labeled_image;
	
	g_return_if_fail (EEL_IS_LABELED_IMAGE (object));

 	labeled_image = EEL_LABELED_IMAGE (object);

 	switch (arg_id)
	{
	case ARG_PIXBUF:
		eel_labeled_image_set_pixbuf (labeled_image, (GdkPixbuf *) GTK_VALUE_POINTER (*arg));
		break;

	case ARG_LABEL:
		eel_labeled_image_set_text (labeled_image, GTK_VALUE_STRING (*arg));
		break;

	case ARG_LABEL_POSITION:
		eel_labeled_image_set_label_position (labeled_image, GTK_VALUE_ENUM (*arg));
		break;

	case ARG_SHOW_LABEL:
		eel_labeled_image_set_show_label (labeled_image, GTK_VALUE_BOOL (*arg));
		break;

	case ARG_SHOW_IMAGE:
		eel_labeled_image_set_show_image (labeled_image, GTK_VALUE_BOOL (*arg));
		break;

	case ARG_SPACING:
		eel_labeled_image_set_spacing (labeled_image, GTK_VALUE_UINT (*arg));
		break;

	case ARG_X_PADDING:
		eel_labeled_image_set_x_padding (labeled_image, GTK_VALUE_INT (*arg));
		break;

	case ARG_Y_PADDING:
		eel_labeled_image_set_y_padding (labeled_image, GTK_VALUE_INT (*arg));
		break;

	case ARG_X_ALIGNMENT:
		eel_labeled_image_set_x_alignment (labeled_image, GTK_VALUE_FLOAT (*arg));
		break;

	case ARG_Y_ALIGNMENT:
		eel_labeled_image_set_y_alignment (labeled_image, GTK_VALUE_FLOAT (*arg));
		break;

	case ARG_FILL:
		eel_labeled_image_set_fill (labeled_image, GTK_VALUE_BOOL (*arg));
		break;
 	default:
		g_assert_not_reached ();
	}
}

static void
eel_labeled_image_get_arg (GtkObject *object,
			   GtkArg *arg,
			   guint arg_id)
{
	EelLabeledImage *labeled_image;

	g_return_if_fail (EEL_IS_LABELED_IMAGE (object));
	
	labeled_image = EEL_LABELED_IMAGE (object);

 	switch (arg_id)
	{
	case ARG_PIXBUF:
		GTK_VALUE_POINTER (*arg) = eel_labeled_image_get_pixbuf (labeled_image);
		break;

	case ARG_LABEL:
		GTK_VALUE_STRING (*arg) = eel_labeled_image_get_text (labeled_image);
		break;

	case ARG_LABEL_POSITION:
		GTK_VALUE_ENUM (*arg) = eel_labeled_image_get_label_position (labeled_image);
		break;

	case ARG_SHOW_LABEL:
		GTK_VALUE_BOOL (*arg) = eel_labeled_image_get_show_label (labeled_image);
		break;

	case ARG_SHOW_IMAGE:
		GTK_VALUE_BOOL (*arg) = eel_labeled_image_get_show_image (labeled_image);
		break;

	case ARG_SPACING:
		GTK_VALUE_UINT (*arg) = eel_labeled_image_get_spacing (labeled_image);
		break;

	case ARG_X_PADDING:
		GTK_VALUE_INT (*arg) = eel_labeled_image_get_x_padding (labeled_image);
		break;

	case ARG_Y_PADDING:
		GTK_VALUE_INT (*arg) = eel_labeled_image_get_y_padding (labeled_image);
		break;

	case ARG_X_ALIGNMENT:
		GTK_VALUE_FLOAT (*arg) = eel_labeled_image_get_x_alignment (labeled_image);
		break;

	case ARG_Y_ALIGNMENT:
		GTK_VALUE_FLOAT (*arg) = eel_labeled_image_get_y_alignment (labeled_image);
		break;

	case ARG_FILL:
		GTK_VALUE_BOOL (*arg) = eel_labeled_image_get_fill (labeled_image);
		break;

 	default:
		g_assert_not_reached ();
	}
}

/* GtkWidgetClass methods */
static void
eel_labeled_image_size_request (GtkWidget *widget,
				GtkRequisition *requisition)
{
	EelLabeledImage *labeled_image;
 	EelDimensions content_dimensions;

 	g_return_if_fail (EEL_IS_LABELED_IMAGE (widget));
 	g_return_if_fail (requisition != NULL);

  	labeled_image = EEL_LABELED_IMAGE (widget);
	
 	content_dimensions = labeled_image_get_content_dimensions (labeled_image);

	requisition->width = 
		MAX (1, content_dimensions.width) +
		2 * labeled_image->details->x_padding;

	requisition->height = 
		MAX (1, content_dimensions.height) +
		2 * labeled_image->details->y_padding;
}

static void
eel_labeled_image_size_allocate (GtkWidget *widget,
				 GtkAllocation *allocation)
{
	EelLabeledImage *labeled_image;
 	ArtIRect image_bounds;
	ArtIRect label_bounds;

 	g_return_if_fail (EEL_IS_LABELED_IMAGE (widget));
 	g_return_if_fail (allocation != NULL);

  	labeled_image = EEL_LABELED_IMAGE (widget);

	widget->allocation = *allocation;
	
 	label_bounds = eel_labeled_image_get_label_bounds (labeled_image);
	eel_gtk_container_child_size_allocate (GTK_CONTAINER (widget),
					       labeled_image->details->label,
					       label_bounds);

 	image_bounds = eel_labeled_image_get_image_bounds (labeled_image);
	eel_gtk_container_child_size_allocate (GTK_CONTAINER (widget),
					       labeled_image->details->image,
					       image_bounds);
}

static int
eel_labeled_image_expose_event (GtkWidget *widget,
				GdkEventExpose *event)
{
	EelLabeledImage *labeled_image;

	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (widget), TRUE);
	g_return_val_if_fail (GTK_WIDGET_REALIZED (widget), TRUE);
	g_return_val_if_fail (event != NULL, TRUE);

  	labeled_image = EEL_LABELED_IMAGE (widget);
	
	if (labeled_image_show_label (labeled_image)) {
		eel_gtk_container_child_expose_event (GTK_CONTAINER (widget),
						      labeled_image->details->label,
						      event);
	}
	
	if (labeled_image_show_image (labeled_image)) {
		eel_gtk_container_child_expose_event (GTK_CONTAINER (widget),
						      labeled_image->details->image,
						      event);
	}

	return FALSE;
}

static void
eel_labeled_image_draw (GtkWidget *widget,
			GdkRectangle *area)
{
	EelLabeledImage *labeled_image;
	
	g_return_if_fail (EEL_IS_LABELED_IMAGE (widget));
	g_return_if_fail (area != NULL);
	
  	labeled_image = EEL_LABELED_IMAGE (widget);
	
	if (labeled_image_show_label (labeled_image)) {
		eel_gtk_container_child_draw (GTK_CONTAINER (widget),
					      labeled_image->details->label,
					      area);
	}
	
	if (labeled_image_show_image (labeled_image)) {
		eel_gtk_container_child_draw (GTK_CONTAINER (widget),
					      labeled_image->details->image,
					      area);
	}
}

static void
eel_labeled_image_map (GtkWidget *widget)
{
	EelLabeledImage *labeled_image;

	g_return_if_fail (EEL_IS_LABELED_IMAGE (widget));
	
	labeled_image = EEL_LABELED_IMAGE (widget);

 	GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);
	
	if (labeled_image_show_label (labeled_image)) {
		eel_gtk_container_child_map (GTK_CONTAINER (widget), labeled_image->details->label);
	}

	if (labeled_image_show_image (labeled_image)) {
		eel_gtk_container_child_map (GTK_CONTAINER (widget), labeled_image->details->image);
	}
}

static void
eel_labeled_image_unmap (GtkWidget *widget)
{
	EelLabeledImage *labeled_image;

	g_return_if_fail (EEL_IS_LABELED_IMAGE (widget));
	
	labeled_image = EEL_LABELED_IMAGE (widget);

	GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);
	
	eel_gtk_container_child_unmap (GTK_CONTAINER (widget), labeled_image->details->label);
	eel_gtk_container_child_unmap (GTK_CONTAINER (widget), labeled_image->details->image);
}

/* GtkContainerClass methods */
static void
eel_labeled_image_add (GtkContainer *container,
		       GtkWidget *child)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (container));
	g_return_if_fail (EEL_IS_LABEL (child) || EEL_IS_IMAGE (child));

	eel_gtk_container_child_add (container, child);
}

static void
eel_labeled_image_remove (GtkContainer *container,
			  GtkWidget *child)
{
	EelLabeledImage *labeled_image;
	
	g_return_if_fail (EEL_IS_LABELED_IMAGE (container));
	g_return_if_fail (EEL_IS_LABEL (child) || EEL_IS_IMAGE (child));
	
	labeled_image = EEL_LABELED_IMAGE (container);;

	g_return_if_fail (child == labeled_image->details->image || child == labeled_image->details->label);

	eel_gtk_container_child_remove (container, child);

	if (labeled_image->details->image == child) {
		labeled_image->details->image = NULL;
	}

	if (labeled_image->details->label == child) {
		labeled_image->details->label = NULL;
	}
}

static void
eel_labeled_image_forall (GtkContainer *container,
			  gboolean include_internals,
			  GtkCallback callback,
			  gpointer callback_data)
{
	EelLabeledImage *labeled_image;
	
	g_return_if_fail (EEL_IS_LABELED_IMAGE (container));
	g_return_if_fail (callback != NULL);
	
	labeled_image = EEL_LABELED_IMAGE (container);;
	
	if (include_internals) {
		if (labeled_image->details->image != NULL) {
			(* callback) (labeled_image->details->image, callback_data);
		}

		if (labeled_image->details->label != NULL) {
			(* callback) (labeled_image->details->label, callback_data);
		}
	}
}

/* Private EelLabeledImage methods */
static gboolean
is_fixed_height (const EelLabeledImage *labeled_image)
{
	return labeled_image->details->fixed_image_height > 0;
}

static EelDimensions
labeled_image_get_image_dimensions (const EelLabeledImage *labeled_image)
{
	EelDimensions image_dimensions;
	GtkRequisition image_requisition;	

	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), eel_dimensions_empty);

	if (!labeled_image_show_image (labeled_image)) {
		return eel_dimensions_empty;
	}
	
	gtk_widget_size_request (labeled_image->details->image, &image_requisition);

	image_dimensions.width = (int) image_requisition.width;
	image_dimensions.height = (int) image_requisition.height;

	if (is_fixed_height (labeled_image)) {
		image_dimensions.height = labeled_image->details->fixed_image_height;
	}

	return image_dimensions;
}

static EelDimensions
labeled_image_get_label_dimensions (const EelLabeledImage *labeled_image)
{
	EelDimensions label_dimensions;
	GtkRequisition label_requisition;	

	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), eel_dimensions_empty);

	if (!labeled_image_show_label (labeled_image)) {
		return eel_dimensions_empty;
	}
	
	gtk_widget_size_request (labeled_image->details->label, &label_requisition);
	
	label_dimensions.width = (int) label_requisition.width;
	label_dimensions.height = (int) label_requisition.height;

	return label_dimensions;
}

static ArtIRect
labeled_image_get_image_bounds_fill (const EelLabeledImage *labeled_image)
{
	ArtIRect image_bounds;
	EelDimensions image_dimensions;
	ArtIRect content_bounds;
	ArtIRect bounds;

	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), eel_art_irect_empty);

	image_dimensions = labeled_image_get_image_dimensions (labeled_image);

	if (eel_dimensions_are_empty (image_dimensions)) {
		return eel_art_irect_empty;
	}

	content_bounds = labeled_image_get_content_bounds (labeled_image);
	bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (labeled_image));
	
	if (!labeled_image_show_label (labeled_image)) {
		image_bounds = bounds;
	} else {
		switch (labeled_image->details->label_position) {
		case GTK_POS_LEFT:
			image_bounds.y0 = bounds.y0;
			image_bounds.x0 = content_bounds.x1 - image_dimensions.width;
			image_bounds.y1 = bounds.y1;
			image_bounds.x1 = bounds.x1;
			break;

		case GTK_POS_RIGHT:
			image_bounds.y0 = bounds.y0;
			image_bounds.x0 = bounds.x0;
			image_bounds.y1 = bounds.y1;
			image_bounds.x1 = content_bounds.x0 + image_dimensions.width;
			break;

		case GTK_POS_TOP:
			image_bounds.x0 = bounds.x0;
			image_bounds.y0 = content_bounds.y1 - image_dimensions.height;
			image_bounds.x1 = bounds.x1;
			image_bounds.y1 = bounds.y1;
			break;

		case GTK_POS_BOTTOM:
			image_bounds.x0 = bounds.x0;
			image_bounds.y0 = bounds.y0;
			image_bounds.x1 = bounds.x1;
			image_bounds.y1 = content_bounds.y0 + image_dimensions.height;
			break;
		}
	}

	return image_bounds;
}

ArtIRect
eel_labeled_image_get_image_bounds (const EelLabeledImage *labeled_image)
{
	EelDimensions image_dimensions;
	EelDimensions label_dimensions;
	GtkRequisition image_requisition;
	ArtIRect image_bounds;
	ArtIRect content_bounds;

	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), eel_art_irect_empty);

	if (labeled_image->details->fill) {
		return labeled_image_get_image_bounds_fill (labeled_image);
	}

	/* get true real dimensions if we're in fixed height mode */
	if (is_fixed_height (labeled_image) && labeled_image_show_image (labeled_image)) {
		gtk_widget_size_request (labeled_image->details->image, &image_requisition);
		image_dimensions.width = (int) image_requisition.width;
		image_dimensions.height = (int) image_requisition.height;
	} else {
		image_dimensions = labeled_image_get_image_dimensions (labeled_image);
	}
	
	label_dimensions = labeled_image_get_label_dimensions (labeled_image);

	if (eel_dimensions_are_empty (image_dimensions)) {
		return eel_art_irect_empty;
	}

	content_bounds = labeled_image_get_content_bounds (labeled_image);
	
	if (!labeled_image_show_label (labeled_image)) {
		image_bounds.x0 = 
			content_bounds.x0 +
			(eel_art_irect_get_width (content_bounds) - image_dimensions.width) / 2;
		image_bounds.y0 = 
			content_bounds.y0 +
			(eel_art_irect_get_height (content_bounds) - image_dimensions.height) / 2;
	} else {
		switch (labeled_image->details->label_position) {
		case GTK_POS_LEFT:
			image_bounds.x0 = content_bounds.x1 - image_dimensions.width;
			image_bounds.y0 = 
				content_bounds.y0 +
				(eel_art_irect_get_height (content_bounds) - image_dimensions.height) / 2;
			break;

		case GTK_POS_RIGHT:
			image_bounds.x0 = content_bounds.x0;
			image_bounds.y0 = 
				content_bounds.y0 +
				(eel_art_irect_get_height (content_bounds) - image_dimensions.height) / 2;
			break;

		case GTK_POS_TOP:
			image_bounds.x0 = 
				content_bounds.x0 +
				(eel_art_irect_get_width (content_bounds) - image_dimensions.width) / 2;
			image_bounds.y0 = content_bounds.y1 - image_dimensions.height;
			break;

		case GTK_POS_BOTTOM:
			image_bounds.x0 = 
				content_bounds.x0 +
				(eel_art_irect_get_width (content_bounds) - image_dimensions.width) / 2;
				
			if (is_fixed_height (labeled_image)) {	
				image_bounds.y0 = content_bounds.y0 + eel_art_irect_get_height (content_bounds)
					- image_dimensions.height
					- label_dimensions.height
					- labeled_image->details->spacing;
			} else {
				image_bounds.y0 = content_bounds.y0;
			}	

			break;
		}
	}
	
	image_bounds.x1 = image_bounds.x0 + image_dimensions.width;
	image_bounds.y1 = image_bounds.y0 + image_dimensions.height;

	return image_bounds;
}

static ArtIRect
labeled_image_get_label_bounds_fill (const EelLabeledImage *labeled_image)
{
	ArtIRect label_bounds;
	EelDimensions label_dimensions;
	ArtIRect content_bounds;
	ArtIRect bounds;

	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), eel_art_irect_empty);

	label_dimensions = labeled_image_get_label_dimensions (labeled_image);

	if (eel_dimensions_are_empty (label_dimensions)) {
		return eel_art_irect_empty;
	}

	content_bounds = labeled_image_get_content_bounds (labeled_image);
	bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (labeled_image));

	/* Only the label is shown */
	if (!labeled_image_show_image (labeled_image)) {
		label_bounds = bounds;
		/* Both label and image are shown */
	} else {
		switch (labeled_image->details->label_position) {
		case GTK_POS_LEFT:
			label_bounds.y0 = bounds.y0;
			label_bounds.x0 = bounds.x0;
			label_bounds.y1 = bounds.y1;
			label_bounds.x1 = content_bounds.x0 + label_dimensions.width;
			break;

		case GTK_POS_RIGHT:
			label_bounds.y0 = bounds.y0;
			label_bounds.x0 = content_bounds.x1 - label_dimensions.width;
			label_bounds.y1 = bounds.y1;
			label_bounds.x1 = bounds.x1;
			break;

		case GTK_POS_TOP:
			label_bounds.x0 = bounds.x0;
			label_bounds.y0 = bounds.y0;
			label_bounds.x1 = bounds.x1;
			label_bounds.y1 = content_bounds.y0 + label_dimensions.height;
			break;

		case GTK_POS_BOTTOM:
			label_bounds.x0 = bounds.x0;
			label_bounds.y0 = content_bounds.y1 - label_dimensions.height;
			label_bounds.x1 = bounds.x1;
			label_bounds.y1 = bounds.y1;
			break;
		}
	}

	return label_bounds;
}

ArtIRect
eel_labeled_image_get_label_bounds (const EelLabeledImage *labeled_image)
{
	ArtIRect label_bounds;
	EelDimensions label_dimensions;
	ArtIRect content_bounds;

	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), eel_art_irect_empty);

	if (labeled_image->details->fill) {
		return labeled_image_get_label_bounds_fill (labeled_image);
	}

	label_dimensions = labeled_image_get_label_dimensions (labeled_image);

	if (eel_dimensions_are_empty (label_dimensions)) {
		return eel_art_irect_empty;
	}

	content_bounds = labeled_image_get_content_bounds (labeled_image);

	/* Only the label is shown */
	if (!labeled_image_show_image (labeled_image)) {
		label_bounds.x0 = 
			content_bounds.x0 +
			(eel_art_irect_get_width (content_bounds) - label_dimensions.width) / 2;
		label_bounds.y0 = 
			content_bounds.y0 +
			(eel_art_irect_get_height (content_bounds) - label_dimensions.height) / 2;
		/* Both label and image are shown */
	} else {
		switch (labeled_image->details->label_position) {
		case GTK_POS_LEFT:
			label_bounds.x0 = content_bounds.x0;
			label_bounds.y0 = 
				content_bounds.y0 +
				(eel_art_irect_get_height (content_bounds) - label_dimensions.height) / 2;
			break;

		case GTK_POS_RIGHT:
			label_bounds.x0 = content_bounds.x1 - label_dimensions.width;
			label_bounds.y0 = 
				content_bounds.y0 +
				(eel_art_irect_get_height (content_bounds) - label_dimensions.height) / 2;
			break;

		case GTK_POS_TOP:
			label_bounds.x0 = 
				content_bounds.x0 +
				(eel_art_irect_get_width (content_bounds) - label_dimensions.width) / 2;
			label_bounds.y0 = content_bounds.y0;
			break;

		case GTK_POS_BOTTOM:
			label_bounds.x0 = 
				content_bounds.x0 +
				(eel_art_irect_get_width (content_bounds) - label_dimensions.width) / 2;
			label_bounds.y0 = content_bounds.y1 - label_dimensions.height;
			break;
		}
	}
	
	label_bounds.x1 = label_bounds.x0 + label_dimensions.width;
	label_bounds.y1 = label_bounds.y0 + label_dimensions.height;

	return label_bounds;
}

static void
labeled_image_update_alignments (EelLabeledImage *labeled_image)
{

	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	if (labeled_image->details->label != NULL) {
		float x_alignment;
		float y_alignment;
		
		if (labeled_image->details->fill) {	
			x_alignment = GTK_MISC (labeled_image->details->label)->xalign;
			y_alignment = GTK_MISC (labeled_image->details->label)->yalign;
			
			/* Only the label is shown */
			if (!labeled_image_show_image (labeled_image)) {
				x_alignment = 0.5;
				y_alignment = 0.5;
				/* Both label and image are shown */
			} else {
				switch (labeled_image->details->label_position) {
				case GTK_POS_LEFT:
					x_alignment = 1.0;
					y_alignment = 0.5;
					break;
					
				case GTK_POS_RIGHT:
					x_alignment = 0.0;
					y_alignment = 0.5;
					break;
					
				case GTK_POS_TOP:
					x_alignment = 0.5;
					y_alignment = 1.0;
					break;
					
				case GTK_POS_BOTTOM:
					x_alignment = 0.5;
					y_alignment = 0.0;
					break;
				}
				
			}

			gtk_misc_set_alignment (GTK_MISC (labeled_image->details->label),
						x_alignment,
						y_alignment);
		}
	}

	if (labeled_image->details->image != NULL) {
		float x_alignment;
		float y_alignment;
		
		if (labeled_image->details->fill) {	
			x_alignment = GTK_MISC (labeled_image->details->image)->xalign;
			y_alignment = GTK_MISC (labeled_image->details->image)->yalign;
			
			/* Only the image is shown */
			if (!labeled_image_show_label (labeled_image)) {
				x_alignment = 0.5;
				y_alignment = 0.5;
				/* Both label and image are shown */
			} else {
				switch (labeled_image->details->label_position) {
				case GTK_POS_LEFT:
					x_alignment = 0.0;
					y_alignment = 0.5;
					break;

				case GTK_POS_RIGHT:
					x_alignment = 1.0;
					y_alignment = 0.5;
					break;
					
				case GTK_POS_TOP:
					x_alignment = 0.5;
					y_alignment = 0.0;
					break;
					
				case GTK_POS_BOTTOM:
					x_alignment = 0.5;
					y_alignment = 1.0;
					break;
				}
			}
			
			gtk_misc_set_alignment (GTK_MISC (labeled_image->details->image),
						x_alignment,
						y_alignment);
		}
	}
}

static EelDimensions
labeled_image_get_content_dimensions (const EelLabeledImage *labeled_image)
{
	EelDimensions image_dimensions;
	EelDimensions label_dimensions;
	EelDimensions content_dimensions;

	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), eel_dimensions_empty);

	image_dimensions = labeled_image_get_image_dimensions (labeled_image);
	label_dimensions = labeled_image_get_label_dimensions (labeled_image);

	content_dimensions = eel_dimensions_empty;

	/* Both shown */
	if (!eel_dimensions_are_empty (image_dimensions) && !eel_dimensions_are_empty (label_dimensions)) {
		content_dimensions.width = 
			image_dimensions.width + labeled_image->details->spacing + label_dimensions.width;
		switch (labeled_image->details->label_position) {
		case GTK_POS_LEFT:
		case GTK_POS_RIGHT:
			content_dimensions.width = 
				image_dimensions.width + labeled_image->details->spacing + label_dimensions.width;
			content_dimensions.height = MAX (image_dimensions.height, label_dimensions.height);
			break;

		case GTK_POS_TOP:
		case GTK_POS_BOTTOM:
			content_dimensions.width = MAX (image_dimensions.width, label_dimensions.width);
			content_dimensions.height = 
				image_dimensions.height + labeled_image->details->spacing + label_dimensions.height;
			break;
		}
		/* Only image shown */
	} else if (!eel_dimensions_are_empty (image_dimensions)) {
		content_dimensions.width = image_dimensions.width;
		content_dimensions.height = image_dimensions.height;
		/* Only label shown */
	} else {
		content_dimensions.width = label_dimensions.width;
		content_dimensions.height = label_dimensions.height;
	}

	return content_dimensions;
}

static ArtIRect
labeled_image_get_content_bounds (const EelLabeledImage *labeled_image)
{
	EelDimensions content_dimensions;
	ArtIRect content_bounds;
	ArtIRect bounds;

	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), eel_art_irect_empty);

	bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (labeled_image));

	content_dimensions = labeled_image_get_content_dimensions (labeled_image);
	content_bounds = eel_art_irect_align (bounds,
					      content_dimensions.width,
					      content_dimensions.height,
					      labeled_image->details->x_alignment,
					      labeled_image->details->y_alignment);

	return content_bounds;
}

static void
labeled_image_ensure_label (EelLabeledImage *labeled_image)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	
	if (labeled_image->details->label != NULL) {
		return;
	}

 	labeled_image->details->label = eel_label_new (NULL);
	gtk_container_add (GTK_CONTAINER (labeled_image), labeled_image->details->label);
	gtk_widget_show (labeled_image->details->label);
}

static void
labeled_image_ensure_image (EelLabeledImage *labeled_image)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	
	if (labeled_image->details->image != NULL) {
		return;
	}

 	labeled_image->details->image = eel_image_new (NULL);
	gtk_container_add (GTK_CONTAINER (labeled_image), labeled_image->details->image);
	gtk_widget_show (labeled_image->details->image);
}

static gboolean
labeled_image_show_image (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), FALSE);
	
	return labeled_image->details->image != NULL && labeled_image->details->show_image;
}

static gboolean
labeled_image_show_label (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), FALSE);

	return labeled_image->details->label != NULL && labeled_image->details->show_label;
}

/**
 * eel_labeled_image_new:
 * @text: Text to use for label or NULL.
 * @pixbuf: Pixbuf to use for image or NULL.
 *
 * Returns A newly allocated EelLabeledImage.  If the &text parameter is not
 * NULL then the LabeledImage will show a label.  If the &pixbuf parameter is not
 * NULL then the LabeledImage will show a pixbuf.  Either of these can be NULL at
 * creation time.  
 *
 * Later in the lifetime of the widget you can invoke methods that affect the 
 * label and/or the image.  If at creation time these were NULL, then they will
 * be created as neeeded.
 *
 * Thus, using this widget in place of EelImage or EelLabel is "free" with
 * only the GtkObject and function call overhead.
 *
 */
GtkWidget*
eel_labeled_image_new (const char *text,
		       GdkPixbuf *pixbuf)
{
	EelLabeledImage *labeled_image;

	labeled_image = EEL_LABELED_IMAGE (gtk_widget_new (eel_labeled_image_get_type (), NULL));
	
	if (text != NULL) {
		eel_labeled_image_set_text (labeled_image, text);
	}

	if (pixbuf != NULL) {
		eel_labeled_image_set_pixbuf (labeled_image, pixbuf);
	}

	labeled_image_update_alignments (labeled_image);

	return GTK_WIDGET (labeled_image);
}

/**
 * eel_labeled_image_new_from_file_name:
 * @text: Text to use for label or NULL.
 * @file_name: File name of picture to use for pixbuf.  Cannot be NULL.
 *
 * Returns A newly allocated EelLabeledImage.  If the &text parameter is not
 * NULL then the LabeledImage will show a label.
 *
 */
GtkWidget*
eel_labeled_image_new_from_file_name (const char *text,
				      const char *pixbuf_file_name)
{
	EelLabeledImage *labeled_image;

	g_return_val_if_fail (pixbuf_file_name != NULL, NULL);

	labeled_image = EEL_LABELED_IMAGE (eel_labeled_image_new (text, NULL));
	eel_labeled_image_set_pixbuf_from_file_name (labeled_image, pixbuf_file_name);
	return GTK_WIDGET (labeled_image);
}

/**
 * eel_labeled_image_set_label_position:
 * @labeled_image: A EelLabeledImage.
 * @label_position: The position of the label with respect to the image.
 *
 * Set the position of the label with respect to the image as follows:
 *
 * GTK_POS_LEFT:
 *   [ <label> <image> ]
 *
 * GTK_POS_RIGHT:
 *   [ <image> <label> ]
 *
 * GTK_POS_TOP:
 *   [ <label> ]
 *   [ <image> ]
 *
 * GTK_POS_BOTTOM:
 *   [ <image> ]
 *   [ <label> ]
 *
 */
void
eel_labeled_image_set_label_position (EelLabeledImage *labeled_image,
				      GtkPositionType label_position)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	g_return_if_fail (label_position >= GTK_POS_LEFT);
	g_return_if_fail (label_position <= GTK_POS_BOTTOM);
	
	if (labeled_image->details->label_position == label_position) {
		return;
	}

	labeled_image->details->label_position = label_position;

	labeled_image_update_alignments (labeled_image);

	gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
}

/**
 * eel_labeled_image_get_label_postiion:
 * @labeled_image: A EelLabeledImage.
 *
 * Returns an enumeration indicating the position of the label with respect to the image.
 */
GtkPositionType
eel_labeled_image_get_label_position (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), 0);
	
	return labeled_image->details->label_position;
}

/**
 * eel_labeled_image_set_show_label:
 * @labeled_image: A EelLabeledImage.
 * @show_image: A boolean value indicating whether the label should be shown.
 *
 * Update the labeled image to either show or hide the internal label widget.
 * This function doesnt have any effect if the LabeledImage doesnt already
 * contain an label.
 */
void
eel_labeled_image_set_show_label (EelLabeledImage *labeled_image,
				  gboolean show_label)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	
	if (labeled_image->details->show_label == show_label) {
		return;
	}

	labeled_image->details->show_label = show_label;

	if (labeled_image->details->label != NULL) {
		if (labeled_image->details->show_label) {
			gtk_widget_show (labeled_image->details->label);
		} else {
			gtk_widget_hide (labeled_image->details->label);
		}
	}

	labeled_image_update_alignments (labeled_image);

	gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
}

/**
 * eel_labeled_image_get_show_label:
 * @labeled_image: A EelLabeledImage.
 *
 * Returns a boolean value indicating whether the internal label is shown.
 */
gboolean
eel_labeled_image_get_show_label (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), 0);
	
	return labeled_image->details->show_label;
}

/**
 * eel_labeled_image_set_show_image:
 * @labeled_image: A EelLabeledImage.
 * @show_image: A boolean value indicating whether the image should be shown.
 *
 * Update the labeled image to either show or hide the internal image widget.
 * This function doesnt have any effect if the LabeledImage doesnt already
 * contain an image.
 */
void
eel_labeled_image_set_show_image (EelLabeledImage *labeled_image,
				  gboolean show_image)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	
	if (labeled_image->details->show_image == show_image) {
		return;
	}

	labeled_image->details->show_image = show_image;

	if (labeled_image->details->image != NULL) {
		if (labeled_image->details->show_image) {
			gtk_widget_show (labeled_image->details->image);
		} else {
			gtk_widget_hide (labeled_image->details->image);
		}
	}

	labeled_image_update_alignments (labeled_image);

	gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
}

/**
 * eel_labeled_image_get_show_image:
 * @labeled_image: A EelLabeledImage.
 *
 * Returns a boolean value indicating whether the internal image is shown.
 */
gboolean
eel_labeled_image_get_show_image (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), 0);
	
	return labeled_image->details->show_image;
}


/**
 * eel_labeled_image_set_fixed_image_height:
 * @labeled_image: A EelLabeledImage.
 * @fixed_image_height: The new fixed image height.
 *
 * Normally, we measure the height of images, but it's sometimes useful
 * to use a fixed height for all the images.  This routine sets the
 * image height to the passed in value
 *
 */
void
eel_labeled_image_set_fixed_image_height (EelLabeledImage *labeled_image,
					  int new_height)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	
	if (labeled_image->details->fixed_image_height == new_height) {
		return;
	}
	
	labeled_image->details->fixed_image_height = new_height;

	labeled_image_update_alignments (labeled_image);
	
	gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
}

/**
 * eel_labeled_image_set_spacing:
 * @labeled_image: A EelLabeledImage.
 * @spacing: The new spacing between label and image.
 *
 * Set the spacing between label and image.  This will only affect
 * the geometry of the widget if both a label and image are currently
 * visible.
 *
 */
void
eel_labeled_image_set_spacing (EelLabeledImage *labeled_image,
			       guint spacing)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	
	if (labeled_image->details->spacing == spacing) {
		return;
	}
	
	labeled_image->details->spacing = spacing;

	labeled_image_update_alignments (labeled_image);
	
	gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
}

/**
 * eel_labeled_image_get_spacing:
 * @labeled_image: A EelLabeledImage.
 *
 * Returns: The spacing between the label and image.
 */
guint
eel_labeled_image_get_spacing (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), 0);
	
	return labeled_image->details->spacing;
}

/**
 * eel_labeled_image_set_x_padding:
 * @labeled_image: A EelLabeledImage.
 * @x_padding: The new horizontal padding.
 *
 * Set horizontal padding for the EelLabeledImage.  The padding
 * attribute work just like that in GtkMisc.
 */
void
eel_labeled_image_set_x_padding (EelLabeledImage *labeled_image,
				 int x_padding)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	x_padding = MAX (0, x_padding);
	
	if (labeled_image->details->x_padding == x_padding) {
		return;
	}
	
	labeled_image->details->x_padding = x_padding;
	labeled_image_update_alignments (labeled_image);
	gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
}

/**
 * eel_labeled_image_get_x_padding:
 * @labeled_image: A EelLabeledImage.
 *
 * Returns: The horizontal padding for the LabeledImage's content.
 */
int
eel_labeled_image_get_x_padding (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), 0);

	return labeled_image->details->x_padding;
}

/**
 * eel_labeled_image_set_y_padding:
 * @labeled_image: A EelLabeledImage.
 * @x_padding: The new vertical padding.
 *
 * Set vertical padding for the EelLabeledImage.  The padding
 * attribute work just like that in GtkMisc.
 */
void
eel_labeled_image_set_y_padding (EelLabeledImage *labeled_image,
				 int y_padding)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	y_padding = MAX (0, y_padding);
	
	if (labeled_image->details->y_padding == y_padding) {
		return;
	}
	
	labeled_image->details->y_padding = y_padding;
	labeled_image_update_alignments (labeled_image);
	gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
}

/**
 * eel_labeled_image_get_x_padding:
 * @labeled_image: A EelLabeledImage.
 *
 * Returns: The vertical padding for the LabeledImage's content.
 */
int
eel_labeled_image_get_y_padding (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), 0);

	return labeled_image->details->y_padding;
}

/**
 * eel_labeled_image_set_x_alignment:
 * @labeled_image: A EelLabeledImage.
 * @x_alignment: The new horizontal alignment.
 *
 * Set horizontal alignment for the EelLabeledImage's content.
 * The 'content' is the union of the image and label.  The alignment
 * attribute work just like that in GtkMisc.
 */
void
eel_labeled_image_set_x_alignment (EelLabeledImage *labeled_image,
				   float x_alignment)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	x_alignment = MAX (0, x_alignment);
	x_alignment = MIN (1.0, x_alignment);
	
	if (labeled_image->details->x_alignment == x_alignment) {
		return;
	}
	
	labeled_image->details->x_alignment = x_alignment;
	gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
}

/**
 * eel_labeled_image_get_x_alignment:
 * @labeled_image: A EelLabeledImage.
 *
 * Returns: The horizontal alignment for the LabeledImage's content.
 */
float
eel_labeled_image_get_x_alignment (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), 0);

	return labeled_image->details->x_alignment;
}

/**
 * eel_labeled_image_set_y_alignment:
 * @labeled_image: A EelLabeledImage.
 * @y_alignment: The new vertical alignment.
 *
 * Set vertical alignment for the EelLabeledImage's content.
 * The 'content' is the union of the image and label.  The alignment
 * attribute work just like that in GtkMisc.
 */
void
eel_labeled_image_set_y_alignment (EelLabeledImage *labeled_image,
				   float y_alignment)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	y_alignment = MAX (0, y_alignment);
	y_alignment = MIN (1.0, y_alignment);
	
	if (labeled_image->details->y_alignment == y_alignment) {
		return;
	}
	
	labeled_image->details->y_alignment = y_alignment;
	gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
}

/**
 * eel_labeled_image_get_y_alignment:
 * @labeled_image: A EelLabeledImage.
 *
 * Returns: The vertical alignment for the LabeledImage's content.
 */
float
eel_labeled_image_get_y_alignment (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), 0);

	return labeled_image->details->y_alignment;
}

/**
 * eel_labeled_image_set_fill:
 * @labeled_image: A EelLabeledImage.
 * @fill: A boolean value indicating whether the internal image and label
 * widgets should fill all the available allocation.
 *
 * By default the internal image and label wigets are sized to their natural
 * preferred geometry.  You can use the 'fill' attribute of LabeledImage
 * to have the internal widgets fill as much of the LabeledImage allocation
 * as is available.  This is useful if you install a tile_pixbuf and want it
 * to cover the whole widget, and not just the areas occupied by the internal
 * widgets.
 */
void
eel_labeled_image_set_fill (EelLabeledImage *labeled_image,
			    gboolean fill)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	
	if (labeled_image->details->fill == fill) {
		return;
	}

	labeled_image->details->fill = fill;

	labeled_image_update_alignments (labeled_image);
	
	gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
}

/**
 * eel_labeled_image_get_fill:
 * @labeled_image: A EelLabeledImage.
 *
 * Retruns a boolean value indicating whether the internal widgets fill
 * all the available allocation.
 */
gboolean
eel_labeled_image_get_fill (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), 0);
	
	return labeled_image->details->fill;
}

/**
 * eel_labeled_image_button_new:
 * @text: Text to use for label or NULL.
 * @pixbuf: Pixbuf to use for image or NULL.
 *
 * Create a stock GtkButton with a EelLabeledImage child.
 *
 */
GtkWidget *
eel_labeled_image_button_new (const char *text,
			      GdkPixbuf *pixbuf)
{
	GtkWidget *button;
	GtkWidget *labeled_image;
	
	button = gtk_button_new ();
	labeled_image = eel_labeled_image_new (text, pixbuf);
	gtk_container_add (GTK_CONTAINER (button), labeled_image);
	gtk_widget_show (labeled_image);
	
	return button;
}

/**
 * eel_labeled_image_button_new_from_file_name:
 * @text: Text to use for label or NULL.
 * @pixbuf_file_name: Name of pixbuf to use for image.  Cannot be NULL.
 *
 * Create a stock GtkToggleButton with a EelLabeledImage child.
 *
 */
GtkWidget *
eel_labeled_image_button_new_from_file_name (const char *text,
					     const char *pixbuf_file_name)
{
	GtkWidget *button;
	GtkWidget *labeled_image;

	g_return_val_if_fail (pixbuf_file_name != NULL, NULL);
	
	button = gtk_button_new ();
	labeled_image = eel_labeled_image_new_from_file_name (text, pixbuf_file_name);
	gtk_container_add (GTK_CONTAINER (button), labeled_image);
	gtk_widget_show (labeled_image);
	
	return button;
}

/**
 * eel_labeled_image_toggle_button_new:
 * @text: Text to use for label or NULL.
 * @pixbuf: Pixbuf to use for image or NULL.
 *
 * Create a stock GtkToggleButton with a EelLabeledImage child.
 *
 */
GtkWidget *
eel_labeled_image_toggle_button_new (const char *text,
				     GdkPixbuf *pixbuf)
{
	GtkWidget *toggle_button;
	GtkWidget *labeled_image;
	
	toggle_button = gtk_toggle_button_new ();
	labeled_image = eel_labeled_image_new (text, pixbuf);
	gtk_container_add (GTK_CONTAINER (toggle_button), labeled_image);
	gtk_widget_show (labeled_image);
	
	return toggle_button;
}

/**
 * eel_labeled_image_toggle_button_new_from_file_name:
 * @text: Text to use for label or NULL.
 * @pixbuf_file_name: Name of pixbuf to use for image.  Cannot be NULL.
 *
 * Create a stock GtkToggleButton with a EelLabeledImage child.
 *
 */
GtkWidget *
eel_labeled_image_toggle_button_new_from_file_name (const char *text,
						    const char *pixbuf_file_name)
{
	GtkWidget *toggle_button;
	GtkWidget *labeled_image;

	g_return_val_if_fail (pixbuf_file_name != NULL, NULL);
	
	toggle_button = gtk_toggle_button_new ();
	labeled_image = eel_labeled_image_new_from_file_name (text, pixbuf_file_name);
	gtk_container_add (GTK_CONTAINER (toggle_button), labeled_image);
	gtk_widget_show (labeled_image);
	
	return toggle_button;
}

/*
 * Workaround some bugs in GtkCheckButton where the widget 
 * does not redraw properly after leave or focus out events
 * 
 * The workaround is to draw a little bit more than the 
 * widget itself - 4 pixels worth.  For some reason the
 * widget does not properly redraw its edges.
 */
static void
button_leave_callback (GtkWidget *widget,
		       gpointer callback_data)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));

	if (GTK_WIDGET_DRAWABLE (widget)) {
		const int fudge = 4;
		ArtIRect bounds;

		bounds = eel_gtk_widget_get_bounds (widget);
		
		bounds.x0 -= fudge;
		bounds.y0 -= fudge;
		bounds.x1 += fudge;
		bounds.y1 += fudge;
		
		gtk_widget_queue_draw_area (widget->parent,
					    bounds.x0,
					    bounds.y0,
					    eel_art_irect_get_width (bounds),
					    eel_art_irect_get_height (bounds));
	}
}

static gint
button_focus_out_event_callback (GtkWidget *widget,
				 GdkEventFocus *event,
				 gpointer callback_data)
{
	g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

	button_leave_callback (widget, callback_data);

	return FALSE;
}

/**
 * eel_labeled_image_check_button_new:
 * @text: Text to use for label or NULL.
 * @pixbuf: Pixbuf to use for image or NULL.
 *
 * Create a stock GtkCheckButton with a EelLabeledImage child.
 *
 */
GtkWidget *
eel_labeled_image_check_button_new (const char *text,
				    GdkPixbuf *pixbuf)
{
	GtkWidget *check_button;
	GtkWidget *labeled_image;
	
	check_button = gtk_check_button_new ();
	labeled_image = eel_labeled_image_new (text, pixbuf);
	gtk_container_add (GTK_CONTAINER (check_button), labeled_image);
	gtk_widget_show (labeled_image);
	
	/*
	 * Workaround some bugs in GtkCheckButton where the widget 
	 * does not redraw properly after leave or focus out events
	 */
	gtk_signal_connect_while_alive (GTK_OBJECT (check_button),
					"leave",
					GTK_SIGNAL_FUNC (button_leave_callback),
					NULL,
					GTK_OBJECT (check_button));
	gtk_signal_connect_while_alive (GTK_OBJECT (check_button),
					"focus_out_event",
					GTK_SIGNAL_FUNC (button_focus_out_event_callback),
					NULL,
					GTK_OBJECT (check_button));
	
	return check_button;
}

/**
 * eel_labeled_image_check_button_new_from_file_name:
 * @text: Text to use for label or NULL.
 * @pixbuf_file_name: Name of pixbuf to use for image.  Cannot be NULL.
 *
 * Create a stock GtkCheckButton with a EelLabeledImage child.
 *
 */
GtkWidget *
eel_labeled_image_check_button_new_from_file_name (const char *text,
						   const char *pixbuf_file_name)
{
	GtkWidget *check_button;
	GtkWidget *labeled_image;

	g_return_val_if_fail (pixbuf_file_name != NULL, NULL);
	
	check_button = gtk_check_button_new ();
	labeled_image = eel_labeled_image_new_from_file_name (text, pixbuf_file_name);
	gtk_container_add (GTK_CONTAINER (check_button), labeled_image);
	gtk_widget_show (labeled_image);
	
	return check_button;
}

/*
 * The rest of the methods are proxies for those in EelImage and 
 * EelLabel.  We have all these so that we dont have to expose 
 * our internal widgets at all.  Probably more of these will be added
 * as they are needed.
 */

/**
 * eel_labeled_image_set_pixbuf:
 * @labaled_image: A EelLabeledImage.
 * @pixbuf: New pixbuf to use or NULL.
 *
 * Change the pixbuf displayed by the LabeledImage.  Note that the widget display
 * is only updated if the show_image attribute is TRUE.
 *
 * If no internal image widget exists as of yet, a new one will be created.
 *
 * A NULL &pixbuf will cause the internal image widget (if alive) to be destroyed.
 */
void
eel_labeled_image_set_pixbuf (EelLabeledImage *labeled_image,
			      GdkPixbuf *pixbuf)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	if (pixbuf == NULL) {
		if (labeled_image->details->image != NULL) {
			gtk_widget_destroy (labeled_image->details->image);
			labeled_image->details->image = NULL;
		}
		
		gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
	} else {
		labeled_image_ensure_image (labeled_image);
		eel_image_set_pixbuf (EEL_IMAGE (labeled_image->details->image), pixbuf);
	}
}

void
eel_labeled_image_set_pixbuf_from_file_name (EelLabeledImage *labeled_image,
					     const char *pixbuf_file_name)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	labeled_image_ensure_image (labeled_image);
	eel_image_set_pixbuf_from_file_name (EEL_IMAGE (labeled_image->details->image), pixbuf_file_name);
}

void
eel_labeled_image_set_tile_pixbuf (EelLabeledImage *labeled_image,
				   GdkPixbuf *tile_pixbuf)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	if (labeled_image->details->image != NULL) {
		eel_image_set_tile_pixbuf (EEL_IMAGE (labeled_image->details->image), tile_pixbuf);
		eel_image_set_tile_mode_horizontal (EEL_IMAGE (labeled_image->details->image),
						    EEL_SMOOTH_TILE_ANCESTOR);
		eel_image_set_tile_mode_vertical (EEL_IMAGE (labeled_image->details->image),
						  EEL_SMOOTH_TILE_ANCESTOR);
	}

	if (labeled_image->details->label != NULL) {
		eel_label_set_tile_pixbuf (EEL_LABEL (labeled_image->details->label), tile_pixbuf);
		eel_label_set_tile_mode_horizontal (EEL_LABEL (labeled_image->details->label),
						    EEL_SMOOTH_TILE_ANCESTOR);
		eel_label_set_tile_mode_vertical (EEL_LABEL (labeled_image->details->label),
						  EEL_SMOOTH_TILE_ANCESTOR);
	}
}

void
eel_labeled_image_set_tile_pixbuf_from_file_name (EelLabeledImage *labeled_image,
						  const char *pixbuf_file_name)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	
	if (labeled_image->details->image != NULL) {
		eel_image_set_tile_pixbuf_from_file_name (EEL_IMAGE (labeled_image->details->image),
							  pixbuf_file_name);
		eel_image_set_tile_mode_horizontal (EEL_IMAGE (labeled_image->details->image),
						    EEL_SMOOTH_TILE_ANCESTOR);
		eel_image_set_tile_mode_vertical (EEL_IMAGE (labeled_image->details->image),
						  EEL_SMOOTH_TILE_ANCESTOR);
	}

	if (labeled_image->details->label != NULL) {
		eel_label_set_tile_pixbuf_from_file_name (EEL_LABEL (labeled_image->details->label),
							  pixbuf_file_name);
		eel_label_set_tile_mode_horizontal (EEL_LABEL (labeled_image->details->label),
						    EEL_SMOOTH_TILE_ANCESTOR);
		eel_label_set_tile_mode_vertical (EEL_LABEL (labeled_image->details->label),
						  EEL_SMOOTH_TILE_ANCESTOR);
	}
}

GdkPixbuf*
eel_labeled_image_get_pixbuf (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), NULL);

	if (labeled_image->details->image == NULL) {
		return NULL;
	}
	
	return eel_image_get_pixbuf (EEL_IMAGE (labeled_image->details->image));
}

/**
 * eel_labeled_image_set_text:
 * @labaled_image: A EelLabeledImage.
 * @text: New text to use or NULL.
 *
 * Change the text displayed by the LabeledImage.  Note that the widget display
 * is only updated if the show_label attribute is TRUE.
 *
 * If no internal label widget exists as of yet, a new one will be created.
 *
 * A NULL &text will cause the internal label widget (if alive) to be destroyed.
 */
void
eel_labeled_image_set_text (EelLabeledImage *labeled_image,
			    const char *text)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	if (text == NULL) {
		if (labeled_image->details->label) {
			gtk_widget_destroy (labeled_image->details->label);
			labeled_image->details->label = NULL;
		}
		
		gtk_widget_queue_resize (GTK_WIDGET (labeled_image));
	} else {
		labeled_image_ensure_label (labeled_image);
		eel_label_set_text (EEL_LABEL (labeled_image->details->label), text);
	}
}

char *
eel_labeled_image_get_text (const EelLabeledImage *labeled_image)
{
	g_return_val_if_fail (EEL_IS_LABELED_IMAGE (labeled_image), NULL);
	
	if (labeled_image->details->label == NULL) {
		return NULL;
	}

	return eel_label_get_text (EEL_LABEL (labeled_image->details->label));
}

void
eel_labeled_image_make_bold (EelLabeledImage *labeled_image)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	labeled_image_ensure_label (labeled_image);
	eel_label_make_bold (EEL_LABEL (labeled_image->details->label));
}

void
eel_labeled_image_make_larger (EelLabeledImage *labeled_image,
			       guint num_sizes)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	labeled_image_ensure_label (labeled_image);
	eel_label_make_larger (EEL_LABEL (labeled_image->details->label), num_sizes);
}

void
eel_labeled_image_make_smaller (EelLabeledImage *labeled_image,
				guint num_sizes)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	labeled_image_ensure_label (labeled_image);
	eel_label_make_smaller (EEL_LABEL (labeled_image->details->label), num_sizes);
}

void
eel_labeled_image_set_tile_width (EelLabeledImage *labeled_image,
				  int tile_width)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	if (labeled_image->details->image != NULL) {
		eel_image_set_tile_width (EEL_IMAGE (labeled_image->details->image),
					  tile_width);
	}

	if (labeled_image->details->label != NULL) {
		eel_label_set_tile_width (EEL_LABEL (labeled_image->details->label),
					  tile_width);
	}
}

void
eel_labeled_image_set_tile_height (EelLabeledImage *labeled_image,
				   int tile_height)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	if (labeled_image->details->image != NULL) {
		eel_image_set_tile_height (EEL_IMAGE (labeled_image->details->image),
					   tile_height);
	}

	if (labeled_image->details->label != NULL) {
		eel_label_set_tile_height (EEL_LABEL (labeled_image->details->label),
					   tile_height);
	}
}

void
eel_labeled_image_set_background_mode (EelLabeledImage *labeled_image,
				       EelSmoothBackgroundMode background_mode)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	g_return_if_fail (background_mode >= EEL_SMOOTH_BACKGROUND_GTK);
	g_return_if_fail (background_mode <= EEL_SMOOTH_BACKGROUND_SOLID_COLOR);

	if (labeled_image->details->image != NULL) {
		eel_image_set_background_mode (EEL_IMAGE (labeled_image->details->image),
					       background_mode);
	}

	if (labeled_image->details->label != NULL) {
		eel_label_set_background_mode (EEL_LABEL (labeled_image->details->label),
					       background_mode);
	}
}

void
eel_labeled_image_set_solid_background_color (EelLabeledImage *labeled_image,
					      guint32 solid_background_color)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	
	if (labeled_image->details->image != NULL) {
		eel_image_set_solid_background_color (EEL_IMAGE (labeled_image->details->image),
						      solid_background_color);
	}

	if (labeled_image->details->label != NULL) {
		eel_label_set_solid_background_color (EEL_LABEL (labeled_image->details->label),
						      solid_background_color);
	}
}

void
eel_labeled_image_set_smooth_drop_shadow_offset (EelLabeledImage *labeled_image,
						 guint drop_shadow_offset)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	
	if (labeled_image->details->label != NULL) {
		eel_label_set_smooth_drop_shadow_offset (EEL_LABEL (labeled_image->details->label),
							 drop_shadow_offset);
	}
}

void
eel_labeled_image_set_smooth_drop_shadow_color (EelLabeledImage *labeled_image,
						guint32 drop_shadow_color)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));
	
	if (labeled_image->details->label != NULL) {
		eel_label_set_smooth_drop_shadow_color (EEL_LABEL (labeled_image->details->label),
							drop_shadow_color);
	}
}

void
eel_labeled_image_set_text_color (EelLabeledImage *labeled_image,
				  guint32 text_color)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	if (labeled_image->details->label != NULL) {
		eel_label_set_text_color (EEL_LABEL (labeled_image->details->label),
					  text_color);
	}
}

void
eel_labeled_image_set_label_never_smooth (EelLabeledImage *labeled_image,
					  gboolean never_smooth)
{
	g_return_if_fail (EEL_IS_LABELED_IMAGE (labeled_image));

	if (labeled_image->details->label != NULL) {
		eel_label_set_never_smooth (EEL_LABEL (labeled_image->details->label),
					    never_smooth);
	}
}
