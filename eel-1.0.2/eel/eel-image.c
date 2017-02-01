/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-image.c - A widget to smoothly display images.

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

#include "eel-image.h"

#include "eel-gtk-macros.h"
#include "eel-gdk-extensions.h"
#include "eel-gtk-extensions.h"
#include "eel-gdk-pixbuf-extensions.h"
#include "eel-art-gtk-extensions.h"
#include "eel-string.h"
#include "eel-lib-self-check-functions.h"
#include "eel-debug-drawing.h"

/* Arguments */
enum
{
	ARG_0,
	ARG_BACKGROUND_MODE,
	ARG_IS_SMOOTH,
	ARG_PIXBUF,
	ARG_PIXBUF_INSENSITIVE_OPACITY,
	ARG_PIXBUF_OPACITY,
	ARG_TILE_HEIGHT,
	ARG_TILE_MODE_HORIZONTAL,
	ARG_TILE_MODE_VERTICAL,
	ARG_TILE_OPACITY,
	ARG_TILE_PIXBUF,
	ARG_TILE_WIDTH
};

/* Signals */
typedef enum
{
	DRAW_BACKGROUND,
	SET_IS_SMOOTH,
	LAST_SIGNAL
} ImageSignal;

/* Signals */
static guint image_signals[LAST_SIGNAL] = { 0 };

/* Detail member struct */
struct EelImageDetails
{
	gboolean is_smooth;

	/* Tile attributes */
	GdkPixbuf *tile_pixbuf;
	int tile_opacity;
	int tile_width;
	int tile_height;
	EelSmoothTileMode tile_mode_vertical;
	EelSmoothTileMode tile_mode_horizontal;

	/* Pixbuf */
	GdkPixbuf *pixbuf;
	int pixbuf_opacity;
	int pixbuf_insensitive_opacity;

	/* Background */
	EelSmoothBackgroundMode background_mode;
	guint32 solid_background_color;
	gboolean never_smooth;
};

/* GtkObjectClass methods */
static void          eel_image_initialize_class     (EelImageClass  *image_class);
static void          eel_image_initialize           (EelImage       *image);
static void          eel_image_destroy              (GtkObject      *object);
static void          eel_image_set_arg              (GtkObject      *object,
						     GtkArg         *arg,
						     guint           arg_id);
static void          eel_image_get_arg              (GtkObject      *object,
						     GtkArg         *arg,
						     guint           arg_id);
/* GtkWidgetClass methods */
static void          eel_image_size_request         (GtkWidget      *widget,
						     GtkRequisition *requisition);
static int           eel_image_expose_event         (GtkWidget      *widget,
						     GdkEventExpose *event);

/* EelImage signals */
static void          eel_image_set_is_smooth_signal (GtkWidget      *widget,
						     gboolean        is_smooth);

/* Private EelImage methods */
static EelDimensions image_get_pixbuf_dimensions    (const EelImage *image);
static ArtIRect      image_get_pixbuf_bounds        (const EelImage *image);
static EelDimensions image_get_tile_dimensions      (const EelImage *image);
static gboolean      image_is_smooth                (const EelImage *image);

EEL_DEFINE_CLASS_BOILERPLATE (EelImage, eel_image, GTK_TYPE_MISC)

/* Class init methods */
	static void
eel_image_initialize_class (EelImageClass *image_class)
{
	GtkObjectClass *object_class = GTK_OBJECT_CLASS (image_class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (image_class);

	/* GtkObjectClass */
	object_class->destroy = eel_image_destroy;
	object_class->set_arg = eel_image_set_arg;
	object_class->get_arg = eel_image_get_arg;
	
	/* GtkWidgetClass */
	widget_class->size_request = eel_image_size_request;
	widget_class->expose_event = eel_image_expose_event;

	/* EelImageClass */
	image_class->set_is_smooth = eel_image_set_is_smooth_signal;
	
	/* Signals */
	image_signals[DRAW_BACKGROUND] = gtk_signal_new ("draw_background",
							 GTK_RUN_LAST,
							 object_class->type,
							 0,
							 gtk_marshal_NONE__POINTER_POINTER,
							 GTK_TYPE_NONE, 
							 2,
							 GTK_TYPE_POINTER,
							 GTK_TYPE_POINTER);

	image_signals[SET_IS_SMOOTH] = gtk_signal_new ("set_is_smooth",
						       GTK_RUN_LAST,
						       object_class->type,
						       GTK_SIGNAL_OFFSET (EelImageClass, set_is_smooth),
						       gtk_marshal_NONE__BOOL,
						       GTK_TYPE_NONE, 
						       1,
						       GTK_TYPE_BOOL);
	
	gtk_object_class_add_signals (object_class, image_signals, LAST_SIGNAL);

	/* Arguments */
	gtk_object_add_arg_type ("EelImage::is_smooth",
				 GTK_TYPE_BOOL,
				 GTK_ARG_READWRITE,
				 ARG_IS_SMOOTH);
	gtk_object_add_arg_type ("EelImage::pixbuf",
				 GTK_TYPE_POINTER,
				 GTK_ARG_READWRITE,
				 ARG_PIXBUF);
	gtk_object_add_arg_type ("EelImage::pixbuf_opacity",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_PIXBUF_OPACITY);
	gtk_object_add_arg_type ("EelImage::pixbuf_insensitive_opacity",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_PIXBUF_INSENSITIVE_OPACITY);
	gtk_object_add_arg_type ("EelImage::background_mode",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE,
				 ARG_BACKGROUND_MODE);
	gtk_object_add_arg_type ("EelImage::tile_pixbuf",
				 GTK_TYPE_POINTER,
				 GTK_ARG_READWRITE,
				 ARG_TILE_PIXBUF);
	gtk_object_add_arg_type ("EelImage::tile_opacity",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_TILE_OPACITY);
	gtk_object_add_arg_type ("EelImage::tile_width",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_TILE_WIDTH);
	gtk_object_add_arg_type ("EelImage::tile_height",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_TILE_HEIGHT);
	gtk_object_add_arg_type ("EelImage::tile_mode_vertical",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE,
				 ARG_TILE_MODE_VERTICAL);
	gtk_object_add_arg_type ("EelImage::tile_mode_horizontal",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE,
				 ARG_TILE_MODE_HORIZONTAL);

	/* Make this class inherit the same kind of theme stuff as GtkPixmap */
	eel_gtk_class_name_make_like_existing_type ("EelImage", GTK_TYPE_PIXMAP);

	/* Let the smooth widget machinery know that our class can be smooth */
	eel_smooth_widget_register_type (EEL_TYPE_IMAGE);
}

void
eel_image_initialize (EelImage *image)
{
	GTK_WIDGET_UNSET_FLAGS (image, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS (image, GTK_NO_WINDOW);

	image->details = g_new0 (EelImageDetails, 1);

	image->details->pixbuf_opacity = EEL_OPACITY_FULLY_OPAQUE;
	image->details->pixbuf_insensitive_opacity = EEL_OPACITY_FULLY_OPAQUE / 3;
	image->details->tile_opacity = EEL_OPACITY_FULLY_OPAQUE;
 	image->details->tile_width = EEL_SMOOTH_TILE_EXTENT_FULL;
 	image->details->tile_height = EEL_SMOOTH_TILE_EXTENT_FULL;
	image->details->tile_mode_vertical = EEL_SMOOTH_TILE_SELF;
	image->details->tile_mode_horizontal = EEL_SMOOTH_TILE_SELF;
	image->details->background_mode = EEL_SMOOTH_BACKGROUND_GTK;

	eel_smooth_widget_register (GTK_WIDGET (image));
}

/* GtkObjectClass methods */
static void
eel_image_destroy (GtkObject *object)
{
 	EelImage *image;
	
	g_return_if_fail (EEL_IS_IMAGE (object));

	image = EEL_IMAGE (object);

	eel_gdk_pixbuf_unref_if_not_null (image->details->tile_pixbuf);
	image->details->tile_pixbuf = NULL;

	g_free (image->details);

	/* Chain destroy */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

static void
eel_image_set_arg (GtkObject *object,
		   GtkArg *arg,
		   guint arg_id)
{
	EelImage *image;
	
	g_return_if_fail (EEL_IS_IMAGE (object));

 	image = EEL_IMAGE (object);

 	switch (arg_id)
	{
	case ARG_IS_SMOOTH:
		eel_image_set_is_smooth (image, GTK_VALUE_BOOL (*arg));
		break;

	case ARG_PIXBUF_OPACITY:
		eel_image_set_pixbuf_opacity (image, GTK_VALUE_INT (*arg));
		break;

	case ARG_PIXBUF_INSENSITIVE_OPACITY:
		eel_image_set_pixbuf_insensitive_opacity (image, GTK_VALUE_INT (*arg));
		break;

	case ARG_BACKGROUND_MODE:
		eel_image_set_background_mode (image, GTK_VALUE_UINT (*arg));
		break;

	case ARG_PIXBUF:
		eel_image_set_pixbuf (image, (GdkPixbuf *) GTK_VALUE_POINTER (*arg));
		break;

	case ARG_TILE_OPACITY:
		eel_image_set_tile_opacity (image, GTK_VALUE_INT (*arg));
		break;

	case ARG_TILE_PIXBUF:
		eel_image_set_tile_pixbuf (image, (GdkPixbuf *) GTK_VALUE_POINTER (*arg));
		break;
		
	case ARG_TILE_WIDTH:
		eel_image_set_tile_width (image, GTK_VALUE_INT (*arg));
		break;

	case ARG_TILE_HEIGHT:
		eel_image_set_tile_height (image, GTK_VALUE_INT (*arg));
		break;

	case ARG_TILE_MODE_VERTICAL:
		eel_image_set_tile_mode_vertical (image, GTK_VALUE_UINT (*arg));
		break;

	case ARG_TILE_MODE_HORIZONTAL:
		eel_image_set_tile_mode_horizontal (image, GTK_VALUE_UINT (*arg));
		break;

 	default:
		g_assert_not_reached ();
	}
}

static void
eel_image_get_arg (GtkObject *object,
		   GtkArg *arg,
		   guint arg_id)
{
	EelImage *image;

	g_return_if_fail (EEL_IS_IMAGE (object));
	
	image = EEL_IMAGE (object);

 	switch (arg_id)
	{
	case ARG_IS_SMOOTH:
		GTK_VALUE_BOOL (*arg) = eel_image_get_is_smooth (image);
		break;
		
	case ARG_PIXBUF_OPACITY:
		GTK_VALUE_INT (*arg) = eel_image_get_pixbuf_opacity (image);
		break;
		
	case ARG_PIXBUF_INSENSITIVE_OPACITY:
		GTK_VALUE_INT (*arg) = eel_image_get_pixbuf_insensitive_opacity (image);
		break;
		
	case ARG_BACKGROUND_MODE:
		GTK_VALUE_UINT (*arg) = eel_image_get_background_mode (image);
		break;
		
	case ARG_PIXBUF:
		GTK_VALUE_POINTER (*arg) = eel_image_get_pixbuf (image);
		break;

	case ARG_TILE_OPACITY:
		GTK_VALUE_INT (*arg) = eel_image_get_tile_opacity (image);
		break;
		
	case ARG_TILE_PIXBUF:
		GTK_VALUE_POINTER (*arg) = eel_image_get_tile_pixbuf (image);
		break;

	case ARG_TILE_WIDTH:
		GTK_VALUE_INT (*arg) = eel_image_get_tile_width (image);
		break;

	case ARG_TILE_HEIGHT:
		GTK_VALUE_INT (*arg) = eel_image_get_tile_height (image);
		break;

	case ARG_TILE_MODE_VERTICAL:
		GTK_VALUE_UINT (*arg) = eel_image_get_tile_mode_vertical (image);
		break;

	case ARG_TILE_MODE_HORIZONTAL:
		GTK_VALUE_UINT (*arg) = eel_image_get_tile_mode_horizontal (image);
		break;

 	default:
		g_assert_not_reached ();
	}
}

/* GtkWidgetClass methods */
static void
eel_image_size_request (GtkWidget *widget,
			GtkRequisition *requisition)
{
	EelImage *image;

	EelDimensions pixbuf_dimensions;
	EelDimensions tile_dimensions;
	EelDimensions preferred_dimensions;
	
	g_return_if_fail (EEL_IS_IMAGE (widget));
	g_return_if_fail (requisition != NULL);
	
 	image = EEL_IMAGE (widget);
	
	pixbuf_dimensions = image_get_pixbuf_dimensions (image);
	tile_dimensions = image_get_tile_dimensions (image);
	preferred_dimensions = eel_smooth_widget_get_preferred_dimensions (widget,
									   pixbuf_dimensions,
									   tile_dimensions,
									   image->details->tile_width,
									   image->details->tile_height);
   	requisition->width = preferred_dimensions.width;
   	requisition->height = preferred_dimensions.height;
}

static void
image_paint_pixbuf_callback (GtkWidget *widget,
			     GdkDrawable *destination_drawable,
			     GdkGC *gc,
			     int source_x,
			     int source_y,
			     ArtIRect area,
			     gpointer callback_data)
{
	EelImage *image;

	g_return_if_fail (EEL_IS_IMAGE (widget));
	g_return_if_fail (GTK_WIDGET_REALIZED (widget));
	g_return_if_fail (destination_drawable != NULL);
	g_return_if_fail (gc != NULL);
	g_return_if_fail (!art_irect_empty (&area));

	image = EEL_IMAGE (widget);

	g_return_if_fail (eel_gdk_pixbuf_is_valid (image->details->pixbuf));
	
	eel_gdk_pixbuf_draw_to_drawable (image->details->pixbuf,
					 destination_drawable,
					 gc,
					 source_x,
					 source_y,
					 area,
					 GDK_RGB_DITHER_NONE,
					 GDK_PIXBUF_ALPHA_BILEVEL,
					 EEL_STANDARD_ALPHA_THRESHHOLD);
}

static void
image_composite_pixbuf_callback (GtkWidget *widget,
				 GdkPixbuf *destination_pixbuf,
				 int source_x,
				 int source_y,
				 ArtIRect area,
				 int opacity,
				 gpointer callback_data)
{
	EelImage *image;

	g_return_if_fail (EEL_IS_IMAGE (widget));
	g_return_if_fail (GTK_WIDGET_REALIZED (widget));
	g_return_if_fail (destination_pixbuf != NULL);
	g_return_if_fail (!art_irect_empty (&area));

	image = EEL_IMAGE (widget);

	g_return_if_fail (eel_gdk_pixbuf_is_valid (image->details->pixbuf));

	eel_gdk_pixbuf_draw_to_pixbuf_alpha (image->details->pixbuf,
					     destination_pixbuf,
					     source_x,
					     source_y,
					     area,
					     opacity,
					     GDK_INTERP_BILINEAR);
}	

static int
eel_image_expose_event (GtkWidget *widget,
			GdkEventExpose *event)
{
 	EelImage *image;
	ArtIRect dirty_area;
	ArtIRect screen_dirty_area;
	ArtIRect pixbuf_bounds;
	ArtIRect tile_bounds;

	g_return_val_if_fail (EEL_IS_IMAGE (widget), TRUE);
	g_return_val_if_fail (GTK_WIDGET_REALIZED (widget), TRUE);
	g_return_val_if_fail (event != NULL, TRUE);
	g_return_val_if_fail (event->window == widget->window, TRUE);
	
 	image = EEL_IMAGE (widget);

	pixbuf_bounds = image_get_pixbuf_bounds (image);
	tile_bounds = eel_smooth_widget_get_tile_bounds (widget,
							 image->details->tile_pixbuf,
							 image->details->tile_width,
							 image->details->tile_height);
	
	/* Check for the dumb case when theres nothing to do */
	if (image->details->pixbuf == NULL && image->details->tile_pixbuf == NULL) {
		return TRUE;
	}

	/* Clip the dirty area to the screen */
	dirty_area = eel_gdk_rectangle_to_art_irect (event->area);
	screen_dirty_area = eel_gdk_window_clip_dirty_area_to_screen (event->window,
								      dirty_area);
	/* Nothing to do */
	if (art_irect_empty (&screen_dirty_area)) {
		return TRUE;
	}

	/* Paint ourselves */
	eel_smooth_widget_paint (widget,
				 widget->style->white_gc,
				 image_is_smooth (image),
				 image->details->background_mode,
				 image->details->solid_background_color,
				 image->details->tile_pixbuf,
				 tile_bounds,
				 image->details->tile_opacity,
				 image->details->tile_mode_vertical,
				 image->details->tile_mode_horizontal,
				 pixbuf_bounds,
				 (widget->state == GTK_STATE_INSENSITIVE
				  ? image->details->pixbuf_insensitive_opacity
				  : image->details->pixbuf_opacity),
				 screen_dirty_area,
				 image_paint_pixbuf_callback,
				 image_composite_pixbuf_callback,
				 NULL);

	return TRUE;
}

/* EelImage signals */
static void
eel_image_set_is_smooth_signal (GtkWidget *widget,
				gboolean is_smooth)
{
	g_return_if_fail (EEL_IS_IMAGE (widget));

	eel_image_set_is_smooth (EEL_IMAGE (widget), is_smooth);
}

/* Private EelImage methods */
static EelDimensions
image_get_pixbuf_dimensions (const EelImage *image)
{
	EelDimensions pixbuf_dimensions;

	g_return_val_if_fail (EEL_IS_IMAGE (image), eel_dimensions_empty);

	if (!image->details->pixbuf) {
		return eel_dimensions_empty;
	}

	pixbuf_dimensions.width = gdk_pixbuf_get_width (image->details->pixbuf);
	pixbuf_dimensions.height = gdk_pixbuf_get_height (image->details->pixbuf);

	return pixbuf_dimensions;
}

static ArtIRect
image_get_pixbuf_bounds (const EelImage *image)
{
	EelDimensions pixbuf_dimensions;
	ArtIRect pixbuf_bounds;
	ArtIRect bounds;

	g_return_val_if_fail (EEL_IS_IMAGE (image), eel_art_irect_empty);

	pixbuf_dimensions = image_get_pixbuf_dimensions (image);

	if (eel_dimensions_are_empty (pixbuf_dimensions)) {
		return eel_art_irect_empty;
	}
	
	bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (image));
	
	pixbuf_bounds = eel_art_irect_align (bounds,
					     pixbuf_dimensions.width,
					     pixbuf_dimensions.height,
					     GTK_MISC (image)->xalign,
					     GTK_MISC (image)->yalign);

	return pixbuf_bounds;
}

static EelDimensions
image_get_tile_dimensions (const EelImage *image)
{
	EelDimensions tile_dimensions;

	g_return_val_if_fail (EEL_IS_IMAGE (image), eel_dimensions_empty);

	if (!image->details->tile_pixbuf) {
		return eel_dimensions_empty;
	}
	
	tile_dimensions.width = gdk_pixbuf_get_width (image->details->tile_pixbuf);
	tile_dimensions.height = gdk_pixbuf_get_height (image->details->tile_pixbuf);

	return tile_dimensions;
}

gboolean
image_is_smooth (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), FALSE);

	return !image->details->never_smooth && image->details->is_smooth;
}

/* Public EelImage methods */
GtkWidget*
eel_image_new (const char *file_name)
{
	EelImage *image;

	image = EEL_IMAGE (gtk_widget_new (eel_image_get_type (), NULL));
	
	if (file_name != NULL) {
		eel_image_set_pixbuf_from_file_name (image, file_name);
	}

	return GTK_WIDGET (image);
}

void
eel_image_set_is_smooth (EelImage *image,
			 gboolean is_smooth)
{
	g_return_if_fail (EEL_IS_IMAGE (image));

	if (image->details->never_smooth) {
		return;
	}

	if (image->details->is_smooth == is_smooth) {
		return;
	}

	image->details->is_smooth = is_smooth;

	/* We call queue_resize() instead queue_draw() because
	 * we want the widget's background to be cleared of 
	 * the previous pixbuf, even though the geometry of 
	 * the image does not change.
	 */ 
	gtk_widget_queue_resize (GTK_WIDGET (image));
}

gboolean
eel_image_get_is_smooth (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), FALSE);

	return image_is_smooth (image);
}

/**
 * eel_image_set_tile_pixbuf:
 *
 * @image: A EelImage
 * @pixbuf:          The new tile pixbuf
 *
 * Change the tile pixbuf.  A 'pixbuf' value of NULL, means dont use a
 * tile pixbuf - this is the default behavior for the widget.
 */
void
eel_image_set_tile_pixbuf (EelImage *image,
			   GdkPixbuf *pixbuf)
{
	g_return_if_fail (EEL_IS_IMAGE (image));
	
	if (pixbuf != image->details->tile_pixbuf) {
		eel_gdk_pixbuf_unref_if_not_null (image->details->tile_pixbuf);
		eel_gdk_pixbuf_ref_if_not_null (pixbuf);
		
		image->details->tile_pixbuf = pixbuf;

		gtk_widget_queue_draw (GTK_WIDGET (image));
	}
}

/**
 * eel_image_get_tile_pixbuf:
 *
 * @image: A EelImage
 *
 * Return value: A reference to the tile_pixbuf.  Needs to be unreferenced with 
 * gdk_pixbuf_unref()
 */
GdkPixbuf*
eel_image_get_tile_pixbuf (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), NULL);

	eel_gdk_pixbuf_ref_if_not_null (image->details->tile_pixbuf);
	
	return image->details->tile_pixbuf;
}

void
eel_image_set_pixbuf (EelImage *image,
		      GdkPixbuf *pixbuf)
{
	g_return_if_fail (EEL_IS_IMAGE (image));

	if (pixbuf != image->details->pixbuf) {
		eel_gdk_pixbuf_unref_if_not_null (image->details->pixbuf);
		eel_gdk_pixbuf_ref_if_not_null (pixbuf);
		image->details->pixbuf = pixbuf;
		gtk_widget_queue_resize (GTK_WIDGET (image));
	}
}

void
eel_image_set_pixbuf_from_file_name (EelImage *image,
				     const char *file_name)
{
	GdkPixbuf *pixbuf;

	g_return_if_fail (EEL_IS_IMAGE (image));
	g_return_if_fail (file_name != NULL);

	pixbuf = gdk_pixbuf_new_from_file (file_name);			
	
	if (pixbuf != NULL) {
		eel_image_set_pixbuf (image, pixbuf);	
		gdk_pixbuf_unref (pixbuf);
	}
}

GdkPixbuf*
eel_image_get_pixbuf (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), NULL);
	
	eel_gdk_pixbuf_ref_if_not_null (image->details->pixbuf);
	
	return image->details->pixbuf;
}

void
eel_image_set_pixbuf_opacity (EelImage *image,
			      int pixbuf_opacity)
{
	g_return_if_fail (EEL_IS_IMAGE (image));
	g_return_if_fail (pixbuf_opacity >= EEL_OPACITY_FULLY_TRANSPARENT);
	g_return_if_fail (pixbuf_opacity <= EEL_OPACITY_FULLY_OPAQUE);

	if (image->details->pixbuf_opacity == pixbuf_opacity) {
		return;
	}

	image->details->pixbuf_opacity = pixbuf_opacity;

	gtk_widget_queue_draw (GTK_WIDGET (image));
}

int
eel_image_get_pixbuf_opacity (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), EEL_OPACITY_FULLY_OPAQUE);

	return image->details->pixbuf_opacity;
}

void
eel_image_set_pixbuf_insensitive_opacity (EelImage *image,
					  int pixbuf_insensitive_opacity)
{
	g_return_if_fail (EEL_IS_IMAGE (image));
	g_return_if_fail (pixbuf_insensitive_opacity >= EEL_OPACITY_FULLY_TRANSPARENT);
	g_return_if_fail (pixbuf_insensitive_opacity <= EEL_OPACITY_FULLY_OPAQUE);

	if (image->details->pixbuf_insensitive_opacity == pixbuf_insensitive_opacity) {
		return;
	}

	image->details->pixbuf_insensitive_opacity = pixbuf_insensitive_opacity;

	if (!GTK_WIDGET_SENSITIVE (image)) {
		gtk_widget_queue_draw (GTK_WIDGET (image));
	}
}

int
eel_image_get_pixbuf_insensitive_opacity (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), EEL_OPACITY_FULLY_OPAQUE);

	return image->details->pixbuf_insensitive_opacity;
}

void
eel_image_set_tile_opacity (EelImage *image,
			    int tile_opacity)
{
	g_return_if_fail (EEL_IS_IMAGE (image));
	g_return_if_fail (tile_opacity >= EEL_OPACITY_FULLY_TRANSPARENT);
	g_return_if_fail (tile_opacity <= EEL_OPACITY_FULLY_OPAQUE);

	if (image->details->tile_opacity == tile_opacity) {
		return;
	}

	image->details->tile_opacity = tile_opacity;

	gtk_widget_queue_draw (GTK_WIDGET (image));
}

int
eel_image_get_tile_opacity (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), EEL_OPACITY_FULLY_OPAQUE);

	return image->details->tile_opacity;
}

void
eel_image_set_tile_width (EelImage *image,
			  int tile_width)
{
	g_return_if_fail (EEL_IS_IMAGE (image));
	g_return_if_fail (tile_width >= EEL_SMOOTH_TILE_EXTENT_ONE_STEP);
	
	if (image->details->tile_width == tile_width) {
		return;
	}

	image->details->tile_width = tile_width;

	gtk_widget_queue_resize (GTK_WIDGET (image));
}

/**
 * eel_image_get_tile_width:
 *
 * @image: A EelImage
 *
 * Return value: The tile width.
 */
int
eel_image_get_tile_width (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), 0);

	return image->details->tile_width;
}

void
eel_image_set_tile_height (EelImage *image,
			   int tile_height)
{
	g_return_if_fail (EEL_IS_IMAGE (image));
	g_return_if_fail (tile_height >= EEL_SMOOTH_TILE_EXTENT_ONE_STEP);
	
	if (image->details->tile_height == tile_height) {
		return;
	}

	image->details->tile_height = tile_height;

	gtk_widget_queue_resize (GTK_WIDGET (image));
}

/**
 * eel_image_get_tile_height:
 *
 * @image: A EelImage
 *
 * Return value: The tile height.
 */
int
eel_image_get_tile_height (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), 0);

	return image->details->tile_height;
}

void
eel_image_set_tile_mode_vertical (EelImage *image,
				  EelSmoothTileMode tile_mode_vertical)
{
	g_return_if_fail (EEL_IS_IMAGE (image));
	g_return_if_fail (tile_mode_vertical >= EEL_SMOOTH_TILE_SELF);
	g_return_if_fail (tile_mode_vertical <= EEL_SMOOTH_TILE_ANCESTOR);

	if (image->details->tile_mode_vertical == tile_mode_vertical) {
		return;
	}

	image->details->tile_mode_vertical = tile_mode_vertical;

	gtk_widget_queue_draw (GTK_WIDGET (image));
}

EelSmoothTileMode
eel_image_get_tile_mode_vertical (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), 0);
	
	return image->details->tile_mode_vertical;
}

void
eel_image_set_tile_mode_horizontal (EelImage *image,
				    EelSmoothTileMode tile_mode_horizontal)
{
	g_return_if_fail (EEL_IS_IMAGE (image));
	g_return_if_fail (tile_mode_horizontal >= EEL_SMOOTH_TILE_SELF);
	g_return_if_fail (tile_mode_horizontal <= EEL_SMOOTH_TILE_ANCESTOR);

	if (image->details->tile_mode_horizontal == tile_mode_horizontal) {
		return;
	}

	image->details->tile_mode_horizontal = tile_mode_horizontal;

	gtk_widget_queue_draw (GTK_WIDGET (image));
}

EelSmoothTileMode
eel_image_get_tile_mode_horizontal (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), 0);
	
	return image->details->tile_mode_horizontal;
}

void
eel_image_set_tile_pixbuf_from_file_name (EelImage *image,
					  const char *tile_file_name)
{
	GdkPixbuf *tile_pixbuf;

	g_return_if_fail (EEL_IS_IMAGE (image));
	g_return_if_fail (tile_file_name != NULL);

	tile_pixbuf = gdk_pixbuf_new_from_file (tile_file_name);
	
	if (tile_pixbuf != NULL) {
		eel_image_set_tile_pixbuf (image, tile_pixbuf);
		gdk_pixbuf_unref (tile_pixbuf);
	}
}

void
eel_image_set_background_mode (EelImage *image,
			       EelSmoothBackgroundMode background_mode)
{
	g_return_if_fail (EEL_IS_IMAGE (image));
	g_return_if_fail (background_mode >= EEL_SMOOTH_BACKGROUND_GTK);
	g_return_if_fail (background_mode <= EEL_SMOOTH_BACKGROUND_SOLID_COLOR);

	if (image->details->background_mode == background_mode) {
		return;
	}

	image->details->background_mode = background_mode;

	gtk_widget_queue_draw (GTK_WIDGET (image));
}

EelSmoothBackgroundMode
eel_image_get_background_mode (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), 0);
	
	return image->details->background_mode;
}

void
eel_image_set_solid_background_color (EelImage *image,
				      guint32 solid_background_color)
{
	g_return_if_fail (EEL_IS_IMAGE (image));
	
	if (image->details->solid_background_color == solid_background_color) {
		return;
	}

	image->details->solid_background_color = solid_background_color;
	
	gtk_widget_queue_draw (GTK_WIDGET (image));
}

guint32
eel_image_get_solid_background_color (const EelImage *image)
{
	g_return_val_if_fail (EEL_IS_IMAGE (image), 0);
	
	return image->details->solid_background_color;
}

/**
 * eel_image_new_solid:
 *
 * @pixbuf: A GdkPixbuf or NULL.
 * @x_alignment: Horizontal alignment.
 * @y_alignment: Vertical alignment.
 * @x_padding: Horizontal padding.
 * @y_padding: Vertical padding.
 * @background_color: Background color.
 * @tile_pixbuf: A GdkPixbuf or NULL.
 *
 * Create an image with a solid background.
 *
 * Return value: The newly allocated EelImage with the
 * given attributes.
 */
GtkWidget *
eel_image_new_solid (GdkPixbuf *pixbuf,
		     float x_alignment,
		     float y_alignment,
		     int x_padding,
		     int y_padding,
		     guint32 background_color,
		     GdkPixbuf *tile_pixbuf)
{
	EelImage *image;

 	image = EEL_IMAGE (eel_image_new (NULL));
	
	if (pixbuf != NULL) {
		eel_image_set_pixbuf (image, pixbuf);
	}

	eel_image_set_background_mode (image, EEL_SMOOTH_BACKGROUND_SOLID_COLOR);
	eel_image_set_solid_background_color (image, background_color);

	gtk_misc_set_padding (GTK_MISC (image), x_padding, y_padding);
	gtk_misc_set_alignment (GTK_MISC (image), x_alignment, y_alignment);
	
	if (tile_pixbuf != NULL) {
		eel_image_set_tile_pixbuf (image, tile_pixbuf);
	}
	
	return GTK_WIDGET (image);
}


/**
 * eel_image_set_never_smooth
 *
 * @image: A EelImage.
 * @never_smooth: A boolean value indicating whether the image can NEVER be smooth.
 *
 * Force an image to never be smooth.  Calls to eel_image_set_is_smooth () will
 * thus be ignored.  This is useful if you want to use a EelImage in a situation.
 */
void
eel_image_set_never_smooth (EelImage *image,
			    gboolean never_smooth)
{
	g_return_if_fail (EEL_IS_IMAGE (image));

	image->details->never_smooth = never_smooth;
	gtk_widget_queue_resize (GTK_WIDGET (image));
}
