/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-label.c - A widget to display a anti aliased text.

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

#include "eel-label.h"

#include <libgnome/gnome-i18n.h>
#include "eel-gtk-macros.h"
#include "eel-gdk-extensions.h"
#include "eel-gtk-extensions.h"
#include "eel-gdk-pixbuf-extensions.h"
#include "eel-art-gtk-extensions.h"
#include "eel-string.h"
#include "eel-smooth-text-layout.h"
#include "eel-scalable-font-private.h"
#include "eel-debug-drawing.h"
#include <stdlib.h>

/* These are arbitrary constants to catch insane values */
#define MIN_SMOOTH_FONT_SIZE 5
#define MAX_SMOOTH_FONT_SIZE 64

#define DEFAULT_FONT_SIZE 14
#define LINE_WRAP_SEPARATORS _(" -_,;.?/&")
#define LINE_OFFSET 2

#define SMOOTH_FONT_MULTIPLIER 1.3

/* This magic string is copied from GtkLabel.  It lives there unlocalized as well. */
#define DEFAULT_LINE_WRAP_WIDTH_TEXT "This is a good enough length for any line to have."

/* Arguments */
enum
{
	ARG_0,
	
	/* Deal with the GtkLabel arguments as well */
	ARG_LABEL,
	ARG_WRAP,
	ARG_JUSTIFY,

	ARG_BACKGROUND_MODE,
	ARG_IS_SMOOTH,
	ARG_TEXT_OPACITY,
	ARG_SMOOTH_FONT,
	ARG_SMOOTH_FONT_SIZE,
	ARG_SMOOTH_TEXT_COLOR,
	ARG_SMOOTH_DROP_SHADOW_OFFSET,
	ARG_SMOOTH_DROP_SHADOW_COLOR,
	ARG_SMOOTH_LINE_WRAP_WIDTH,
	ARG_ADJUST_WRAP_ON_RESIZE,

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
} LabelSignal;

/* Signals */
static guint label_signals[LAST_SIGNAL] = { 0 };

/* Detail member struct */
struct EelLabelDetails
{
	gboolean is_smooth;

	/* Tile attributes */
	GdkPixbuf *tile_pixbuf;
	int tile_opacity;
	int tile_width;
	int tile_height;
	EelSmoothTileMode tile_mode_vertical;
	EelSmoothTileMode tile_mode_horizontal;

 	/* Smooth attributes */
 	EelScalableFont *smooth_font;
 	int smooth_font_size;
	guint32 smooth_text_color;
	gint smooth_drop_shadow_offset;
	guint32 smooth_drop_shadow_color;
 	int smooth_line_wrap_width;
	gboolean adjust_wrap_on_resize;

	/* Text */
  	int text_opacity;

	/* Background */
	EelSmoothBackgroundMode background_mode;
	guint32 solid_background_color;

	GdkPixbuf *solid_cache_pixbuf;
	gboolean never_smooth;

	EelSmoothTextLayout *smooth_text_layout;
};

/* GtkObjectClass methods */
static void          eel_label_initialize_class        (EelLabelClass  *label_class);
static void          eel_label_initialize              (EelLabel       *label);
static void          eel_label_destroy                 (GtkObject      *object);
static void          eel_label_set_arg                 (GtkObject      *object,
							GtkArg         *arg,
							guint           arg_id);
static void          eel_label_get_arg                 (GtkObject      *object,
							GtkArg         *arg,
							guint           arg_id);

/* GtkWidgetClass methods */
static void          eel_label_size_request            (GtkWidget      *widget,
							GtkRequisition *requisition);
static void          eel_label_size_allocate           (GtkWidget      *widget,
							GtkAllocation  *allocation);
static int           eel_label_expose_event            (GtkWidget      *widget,
							GdkEventExpose *event);

/* EelLabel signals */
static void          eel_label_set_is_smooth_signal    (GtkWidget      *widget,
							gboolean        is_smooth);

/* Private EelLabel things */
const char *         label_peek_text                   (const EelLabel *label);
static ArtIRect      label_get_text_bounds             (const EelLabel *label);
static EelDimensions label_get_text_dimensions         (const EelLabel *label);
static EelDimensions label_get_tile_dimensions         (const EelLabel *label);
static int           label_get_default_line_wrap_width (const EelLabel *label);
static void          label_solid_cache_pixbuf_clear    (EelLabel       *label);
static gboolean      label_can_cache_contents          (const EelLabel *label);
static gboolean      label_is_smooth                   (const EelLabel *label);
static void          label_smooth_text_ensure          (const EelLabel *label);
static void          label_smooth_text_clear           (EelLabel       *label);
static EelDimensions label_get_content_dimensions      (const EelLabel *label);
static ArtIRect      label_get_content_bounds          (const EelLabel *label);

EEL_DEFINE_CLASS_BOILERPLATE (EelLabel, eel_label, GTK_TYPE_LABEL)

/* Class init methods */
	static void
eel_label_initialize_class (EelLabelClass *label_class)
{
	GtkObjectClass *object_class = GTK_OBJECT_CLASS (label_class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (label_class);

	/* GtkObjectClass */
	object_class->destroy = eel_label_destroy;
	object_class->set_arg = eel_label_set_arg;
	object_class->get_arg = eel_label_get_arg;
	
	/* GtkWidgetClass */
	widget_class->size_request = eel_label_size_request;
	widget_class->size_allocate = eel_label_size_allocate;
	widget_class->expose_event = eel_label_expose_event;

	/* EelLabelClass */
	label_class->set_is_smooth = eel_label_set_is_smooth_signal;

	/* Signals */
	label_signals[DRAW_BACKGROUND] = gtk_signal_new ("draw_background",
							 GTK_RUN_LAST,
							 object_class->type,
							 0,
							 gtk_marshal_NONE__POINTER_POINTER,
							 GTK_TYPE_NONE, 
							 2,
							 GTK_TYPE_POINTER,
							 GTK_TYPE_POINTER);

	label_signals[SET_IS_SMOOTH] = gtk_signal_new ("set_is_smooth",
						       GTK_RUN_LAST,
						       object_class->type,
						       GTK_SIGNAL_OFFSET (EelLabelClass, set_is_smooth),
						       gtk_marshal_NONE__BOOL,
						       GTK_TYPE_NONE, 
						       1,
						       GTK_TYPE_BOOL);

	gtk_object_class_add_signals (object_class, label_signals, LAST_SIGNAL);

	/* Arguments */
	gtk_object_add_arg_type ("EelLabel::tile_pixbuf",
				 GTK_TYPE_POINTER,
				 GTK_ARG_READWRITE,
				 ARG_TILE_PIXBUF);
	gtk_object_add_arg_type ("EelLabel::tile_opacity",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_TILE_OPACITY);
	gtk_object_add_arg_type ("EelLabel::tile_width",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_TILE_WIDTH);
	gtk_object_add_arg_type ("EelLabel::tile_height",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_TILE_HEIGHT);
	gtk_object_add_arg_type ("EelLabel::tile_mode_vertical",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE,
				 ARG_TILE_MODE_VERTICAL);
	gtk_object_add_arg_type ("EelLabel::tile_mode_horizontal",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE,
				 ARG_TILE_MODE_HORIZONTAL);
	gtk_object_add_arg_type ("EelLabel::label",
				 GTK_TYPE_STRING,
				 GTK_ARG_READWRITE,
				 ARG_LABEL);
	gtk_object_add_arg_type ("EelLabel::wrap",
				 GTK_TYPE_BOOL,
				 GTK_ARG_READWRITE,
				 ARG_WRAP);
	gtk_object_add_arg_type ("EelLabel::justify",
				 GTK_TYPE_JUSTIFICATION,
				 GTK_ARG_READWRITE,
				 ARG_JUSTIFY);
	gtk_object_add_arg_type ("EelLabel::is_smooth",
				 GTK_TYPE_BOOL,
				 GTK_ARG_READWRITE,
				 ARG_IS_SMOOTH);
	gtk_object_add_arg_type ("EelLabel::text_opacity",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_TEXT_OPACITY);
	gtk_object_add_arg_type ("EelLabel::background_mode",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE,
				 ARG_BACKGROUND_MODE);
	gtk_object_add_arg_type ("EelLabel::smooth_font",
				 GTK_TYPE_OBJECT,
				 GTK_ARG_READWRITE,
				 ARG_SMOOTH_FONT);
	gtk_object_add_arg_type ("EelLabel::smooth_font_size",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_SMOOTH_FONT_SIZE);
	gtk_object_add_arg_type ("EelLabel::smooth_text_color",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE,
				 ARG_SMOOTH_TEXT_COLOR);
	gtk_object_add_arg_type ("EelLabel::smooth_drop_shadow_offset",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_SMOOTH_DROP_SHADOW_OFFSET);
	gtk_object_add_arg_type ("EelLabel::smooth_drop_shadow_color",
				 GTK_TYPE_UINT,
				 GTK_ARG_READWRITE,
				 ARG_SMOOTH_DROP_SHADOW_COLOR);
	gtk_object_add_arg_type ("EelLabel::smooth_line_wrap_width",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_SMOOTH_LINE_WRAP_WIDTH);
	gtk_object_add_arg_type ("EelLabel::adjust_wrap_on_resize",
				 GTK_TYPE_BOOL,
				 GTK_ARG_READWRITE,
				 ARG_ADJUST_WRAP_ON_RESIZE);

	/* Make this class inherit the same kind of theme stuff as GtkLabel */
	eel_gtk_class_name_make_like_existing_type ("EelLabel", GTK_TYPE_LABEL);

	/* Let the smooth widget machinery know that our class can be smooth */
	eel_smooth_widget_register_type (EEL_TYPE_LABEL);
}

void
eel_label_initialize (EelLabel *label)
{
	GTK_WIDGET_UNSET_FLAGS (label, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS (label, GTK_NO_WINDOW);

	label->details = g_new0 (EelLabelDetails, 1);

	label->details->text_opacity = EEL_OPACITY_FULLY_OPAQUE;
	label->details->smooth_font = eel_scalable_font_get_default_font ();
 	label->details->smooth_font_size = DEFAULT_FONT_SIZE;
	label->details->smooth_text_color = EEL_RGBA_COLOR_PACK (0, 0, 0, 255);
	label->details->smooth_drop_shadow_color = EEL_RGBA_COLOR_PACK (255, 255, 255, 255);
	label->details->smooth_line_wrap_width = label_get_default_line_wrap_width (label);

	label->details->tile_opacity = EEL_OPACITY_FULLY_OPAQUE;
 	label->details->tile_width = EEL_SMOOTH_TILE_EXTENT_FULL;
 	label->details->tile_height = EEL_SMOOTH_TILE_EXTENT_FULL;
	label->details->tile_mode_vertical = EEL_SMOOTH_TILE_SELF;
	label->details->tile_mode_horizontal = EEL_SMOOTH_TILE_SELF;
	label->details->background_mode = EEL_SMOOTH_BACKGROUND_GTK;

	eel_smooth_widget_register (GTK_WIDGET (label));
}

/* GtkObjectClass methods */
static void
eel_label_destroy (GtkObject *object)
{
 	EelLabel *label;
	
	g_return_if_fail (EEL_IS_LABEL (object));

	label = EEL_LABEL (object);

	eel_gdk_pixbuf_unref_if_not_null (label->details->tile_pixbuf);
	label->details->tile_pixbuf = NULL;
	label_solid_cache_pixbuf_clear (label);
	label_smooth_text_clear (label);
	g_free (label->details);

	/* Chain destroy */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

static void
eel_label_set_arg (GtkObject *object,
		   GtkArg *arg,
		   guint arg_id)
{
	EelLabel *label;

	g_return_if_fail (EEL_IS_LABEL (object));

 	label = EEL_LABEL (object);

 	switch (arg_id)
	{

	case ARG_TILE_OPACITY:
		eel_label_set_tile_opacity (label, GTK_VALUE_INT (*arg));
		break;

	case ARG_TILE_PIXBUF:
		eel_label_set_tile_pixbuf (label, (GdkPixbuf *) GTK_VALUE_POINTER (*arg));
		break;
		
	case ARG_TILE_WIDTH:
		eel_label_set_tile_width (label, GTK_VALUE_INT (*arg));
		break;

	case ARG_TILE_HEIGHT:
		eel_label_set_tile_height (label, GTK_VALUE_INT (*arg));
		break;

	case ARG_TILE_MODE_VERTICAL:
		eel_label_set_tile_mode_vertical (label, GTK_VALUE_UINT (*arg));
		break;

	case ARG_TILE_MODE_HORIZONTAL:
		eel_label_set_tile_mode_horizontal (label, GTK_VALUE_UINT (*arg));
		break;

	case ARG_LABEL:
		eel_label_set_text (label, GTK_VALUE_STRING (*arg));
		break;
		
	case ARG_WRAP:
		eel_label_set_wrap (label, GTK_VALUE_BOOL (*arg));
		break;
		
	case ARG_JUSTIFY:
		eel_label_set_justify (label, GTK_VALUE_ENUM (*arg));
		break;
		
	case ARG_IS_SMOOTH:
		eel_label_set_is_smooth (label, GTK_VALUE_BOOL (*arg));
		break;

	case ARG_TEXT_OPACITY:
		eel_label_set_text_opacity (label, GTK_VALUE_INT (*arg));
		break;

	case ARG_BACKGROUND_MODE:
		eel_label_set_background_mode (label, GTK_VALUE_UINT (*arg));
		break;

	case ARG_SMOOTH_FONT:
		eel_label_set_smooth_font (label, (EelScalableFont *) GTK_VALUE_OBJECT (*arg));
		break;

	case ARG_SMOOTH_FONT_SIZE:
		eel_label_set_smooth_font_size (label, GTK_VALUE_INT (*arg));
		break;

	case ARG_SMOOTH_TEXT_COLOR:
		eel_label_set_text_color (label, GTK_VALUE_UINT (*arg));
		break;

	case ARG_SMOOTH_DROP_SHADOW_OFFSET:
		eel_label_set_smooth_drop_shadow_offset (label, GTK_VALUE_INT (*arg));
		break;

	case ARG_SMOOTH_DROP_SHADOW_COLOR:
		eel_label_set_smooth_drop_shadow_color (label, GTK_VALUE_UINT (*arg));
		break;

	case ARG_SMOOTH_LINE_WRAP_WIDTH:
		eel_label_set_smooth_line_wrap_width (label, GTK_VALUE_INT (*arg));
		break;

	case ARG_ADJUST_WRAP_ON_RESIZE:
		eel_label_set_adjust_wrap_on_resize (label, GTK_VALUE_BOOL (*arg));
		break;

 	default:
		g_assert_not_reached ();
	}
}

static void
eel_label_get_arg (GtkObject *object,
		   GtkArg *arg,
		   guint arg_id)
{
	EelLabel	*label;

	g_return_if_fail (EEL_IS_LABEL (object));
	
	label = EEL_LABEL (object);

 	switch (arg_id)
	{
	case ARG_TILE_OPACITY:
		GTK_VALUE_INT (*arg) = eel_label_get_tile_opacity (label);
		break;
		
	case ARG_TILE_PIXBUF:
		GTK_VALUE_POINTER (*arg) = eel_label_get_tile_pixbuf (label);
		break;

	case ARG_TILE_WIDTH:
		GTK_VALUE_INT (*arg) = eel_label_get_tile_width (label);
		break;

	case ARG_TILE_HEIGHT:
		GTK_VALUE_INT (*arg) = eel_label_get_tile_height (label);
		break;

	case ARG_TILE_MODE_VERTICAL:
		GTK_VALUE_UINT (*arg) = eel_label_get_tile_mode_vertical (label);
		break;

	case ARG_TILE_MODE_HORIZONTAL:
		GTK_VALUE_UINT (*arg) = eel_label_get_tile_mode_horizontal (label);
		break;

	case ARG_LABEL:
		GTK_VALUE_STRING (*arg) = eel_label_get_text (label);
		break;

	case ARG_WRAP:
		GTK_VALUE_BOOL (*arg) = eel_label_get_wrap (label);
		break;

	case ARG_JUSTIFY:
		GTK_VALUE_ENUM (*arg) = eel_label_get_text_justify (label);
		break;

	case ARG_IS_SMOOTH:
		GTK_VALUE_BOOL (*arg) = eel_label_get_is_smooth (label);
		break;
		
	case ARG_TEXT_OPACITY:
		GTK_VALUE_INT (*arg) = eel_label_get_text_opacity (label);
		break;
		
	case ARG_BACKGROUND_MODE:
		GTK_VALUE_UINT (*arg) = eel_label_get_background_mode (label);
		break;

	case ARG_SMOOTH_FONT:
		GTK_VALUE_OBJECT (*arg) = (GtkObject *) eel_label_get_smooth_font (label);
		break;

	case ARG_SMOOTH_FONT_SIZE:
		GTK_VALUE_INT (*arg) = eel_label_get_smooth_font_size (label);
		break;

	case ARG_SMOOTH_TEXT_COLOR:
		GTK_VALUE_UINT (*arg) = eel_label_get_text_color (label);
		break;

	case ARG_SMOOTH_DROP_SHADOW_OFFSET:
		GTK_VALUE_INT (*arg) = eel_label_get_smooth_drop_shadow_offset (label);
		break;

	case ARG_SMOOTH_DROP_SHADOW_COLOR:
		GTK_VALUE_UINT (*arg) = eel_label_get_smooth_drop_shadow_color (label);
		break;

	case ARG_SMOOTH_LINE_WRAP_WIDTH:
		GTK_VALUE_INT (*arg) = eel_label_get_smooth_line_wrap_width (label);
		break;

	case ARG_ADJUST_WRAP_ON_RESIZE:
		GTK_VALUE_BOOL (*arg) = eel_label_get_adjust_wrap_on_resize (label);
		break;

 	default:
		g_assert_not_reached ();
	}
}

/* GtkWidgetClass methods */
static void
eel_label_size_request (GtkWidget *widget,
			GtkRequisition *requisition)
{
	EelLabel *label;

	EelDimensions content_dimensions;
	EelDimensions tile_dimensions;
	EelDimensions preferred_dimensions;
	
	g_return_if_fail (EEL_IS_LABEL (widget));
	g_return_if_fail (requisition != NULL);

 	label = EEL_LABEL (widget);

	if (!label_is_smooth (label)) {
		EEL_CALL_PARENT (GTK_WIDGET_CLASS, size_request, (widget, requisition));
		return;
	}
	
	content_dimensions = label_get_content_dimensions (label);
	tile_dimensions = label_get_tile_dimensions (label);

	preferred_dimensions = eel_smooth_widget_get_preferred_dimensions (widget,
									   content_dimensions,
									   tile_dimensions,
									   label->details->tile_width,
									   label->details->tile_height);
   	requisition->width = preferred_dimensions.width;
   	requisition->height = preferred_dimensions.height;
}

static void
eel_label_size_allocate (GtkWidget *widget,
			 GtkAllocation *allocation)
{
	EelLabel *label;
	
 	g_return_if_fail (EEL_IS_LABEL (widget));
 	g_return_if_fail (allocation != NULL);
	
  	label = EEL_LABEL (widget);
	
	/* Pre chain size_allocate */
	EEL_CALL_PARENT (GTK_WIDGET_CLASS, size_allocate, (widget, allocation));

	/* Update the wrap width if needed */
	if (label->details->adjust_wrap_on_resize) {
		label->details->smooth_line_wrap_width = (int) allocation->width;
 		if (label->details->smooth_text_layout != NULL) {
 			eel_smooth_text_layout_set_line_wrap_width (label->details->smooth_text_layout,
								    label->details->smooth_line_wrap_width);
 		}
		label_solid_cache_pixbuf_clear (label);
	}
}

/* Painting callback for non smooth case */
static void
label_paint_pixbuf_callback (GtkWidget *widget,
			     GdkDrawable *destination_drawable,
			     GdkGC *gc,
			     int source_x,
			     int source_y,
			     ArtIRect area,
			     gpointer callback_data)
{
	EelLabel *label;
	const ArtIRect *screen_dirty_area;
	GdkEventExpose event;

	g_return_if_fail (EEL_IS_LABEL (widget));
	g_return_if_fail (GTK_WIDGET_REALIZED (widget));
	g_return_if_fail (destination_drawable != NULL);
	g_return_if_fail (gc != NULL);
	g_return_if_fail (!art_irect_empty (&area));

	label = EEL_LABEL (widget);

	screen_dirty_area = callback_data;

	event.type = GDK_EXPOSE;
	event.send_event = TRUE;
	event.window = widget->window;
	event.area = eel_art_irect_to_gdk_rectangle (*screen_dirty_area);
	event.count = 0;

	EEL_CALL_PARENT (GTK_WIDGET_CLASS, expose_event, (widget, &event));
}

/* Compositing callback for smooth case */
static void
label_composite_text_callback_cached (GtkWidget *widget,
				      GdkPixbuf *destination_pixbuf,
				      int source_x,
				      int source_y,
				      ArtIRect area,
				      int opacity,
				      gpointer callback_data)
{
	EelLabel *label;
	EelDimensions content_dimensions;
	ArtIRect cache_pixbuf_area;

	g_return_if_fail (EEL_IS_LABEL (widget));
	g_return_if_fail (GTK_WIDGET_REALIZED (widget));
	g_return_if_fail (destination_pixbuf != NULL);
	g_return_if_fail (!art_irect_empty (&area));
	
	label = EEL_LABEL (widget);

	g_return_if_fail (label_can_cache_contents (label));

	content_dimensions = label_get_content_dimensions (label);
	g_return_if_fail (!eel_dimensions_are_empty (content_dimensions));

	if (label->details->solid_cache_pixbuf == NULL) {
		label->details->solid_cache_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
								     FALSE,
								     8,
								     content_dimensions.width,
								     content_dimensions.height);
		
		eel_gdk_pixbuf_fill_rectangle_with_color (label->details->solid_cache_pixbuf,
							  eel_gdk_pixbuf_whole_pixbuf,
							  label->details->solid_background_color);
		
		cache_pixbuf_area = eel_gdk_pixbuf_intersect (label->details->solid_cache_pixbuf,
							      0,
							      0,
							      eel_gdk_pixbuf_whole_pixbuf);
		g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (label->details->smooth_text_layout));
			
		if (label->details->smooth_drop_shadow_offset > 0) {
			cache_pixbuf_area.x0 += label->details->smooth_drop_shadow_offset;
			cache_pixbuf_area.y0 += label->details->smooth_drop_shadow_offset;
			eel_smooth_text_layout_draw_to_pixbuf (label->details->smooth_text_layout,
							       label->details->solid_cache_pixbuf,
							       0,
							       0,
							       cache_pixbuf_area,
							       eel_label_get_text_justify (label),
							       FALSE,
							       label->details->smooth_drop_shadow_color,
							       label->details->text_opacity);
			cache_pixbuf_area.x0 -= label->details->smooth_drop_shadow_offset;
			cache_pixbuf_area.y0 -= label->details->smooth_drop_shadow_offset;
		}
		
		eel_smooth_text_layout_draw_to_pixbuf (label->details->smooth_text_layout,
						       label->details->solid_cache_pixbuf,
						       0,
						       0,
						       cache_pixbuf_area,
						       eel_label_get_text_justify (label),
						       FALSE,
						       label->details->smooth_text_color,
						       label->details->text_opacity);
	}

	g_return_if_fail (label->details->solid_cache_pixbuf != NULL);
	
	eel_gdk_pixbuf_draw_to_pixbuf (label->details->solid_cache_pixbuf,
				       destination_pixbuf,
				       source_x,
				       source_y,
				       area);
}

static void
label_composite_text_callback (GtkWidget *widget,
			       GdkPixbuf *destination_pixbuf,
			       int source_x,
			       int source_y,
			       ArtIRect area,
			       int opacity,
			       gpointer callback_data)
{
	EelLabel *label;

	g_return_if_fail (EEL_IS_LABEL (widget));
	g_return_if_fail (GTK_WIDGET_REALIZED (widget));
	g_return_if_fail (destination_pixbuf != NULL);
	g_return_if_fail (!art_irect_empty (&area));

	label = EEL_LABEL (widget);

	g_return_if_fail (!label_can_cache_contents (label));

	eel_smooth_text_layout_draw_to_pixbuf (label->details->smooth_text_layout,
					       destination_pixbuf,
					       source_x,
					       source_y,
					       area,
					       eel_label_get_text_justify (label),
					       FALSE,
					       label->details->smooth_text_color,
					       label->details->text_opacity);
}

static void
label_composite_text_and_shadow_callback (GtkWidget *widget,
					  GdkPixbuf *destination_pixbuf,
					  int source_x,
					  int source_y,
					  ArtIRect area,
					  int opacity,
					  gpointer callback_data)
{
	EelLabel *label;

	g_return_if_fail (EEL_IS_LABEL (widget));
	g_return_if_fail (GTK_WIDGET_REALIZED (widget));
	g_return_if_fail (destination_pixbuf != NULL);
	g_return_if_fail (!art_irect_empty (&area));

	label = EEL_LABEL (widget);

	g_return_if_fail (!label_can_cache_contents (label));

	eel_smooth_text_layout_draw_to_pixbuf_shadow (label->details->smooth_text_layout,
						      destination_pixbuf,
						      source_x,
						      source_y,
						      area,
						      label->details->smooth_drop_shadow_offset,
						      eel_label_get_text_justify (label),
						      FALSE,
						      label->details->smooth_text_color,
						      label->details->smooth_drop_shadow_color,
						      label->details->text_opacity);
}

static void
label_paint (EelLabel *label,
	     ArtIRect screen_dirty_area,
	     ArtIRect tile_bounds)
{
	ArtIRect widget_bounds;
	
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (GTK_WIDGET_REALIZED (label));
	
	/* The smooth and non smooth bounds are different.  We have
	 * no way to have GtkLabel tell us what its bounds are.
	 * So, we cheat and pretend that for the non smooth case,
	 * the text bounds are the whole widget.
	 *
	 * This works because the smooth widget paint handler will
	 * properly intersect these bounds and call the paint 
	 * callback.  We feed the paint callback the actual gdk
	 * expose event so that we feed the exact exposure area
	 * to GtkLabel's expose_event.
	 */
	widget_bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (label));

	/* Make sure the area is screen visible before painting */
	eel_smooth_widget_paint (GTK_WIDGET (label),
				 GTK_WIDGET (label)->style->white_gc,
				 FALSE,
				 label->details->background_mode,
				 label->details->solid_background_color,
				 label->details->tile_pixbuf,
				 tile_bounds,
				 label->details->tile_opacity,
				 label->details->tile_mode_vertical,
				 label->details->tile_mode_horizontal,
				 widget_bounds,
				 label->details->text_opacity,
				 screen_dirty_area,
				 label_paint_pixbuf_callback,
				 label_composite_text_callback,
				 (gpointer) &screen_dirty_area);
}

static void
paint_label_smooth (EelLabel *label,
		    ArtIRect screen_dirty_area,
		    ArtIRect tile_bounds)
{
	ArtIRect text_bounds;
	ArtIRect content_bounds;

	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (GTK_WIDGET_REALIZED (label));

	if (label->details->smooth_drop_shadow_offset > 0) {
		/* Paint the text and shadow if needed */
		content_bounds = label_get_content_bounds (label);
		if (!art_irect_empty (&content_bounds)) {
			eel_smooth_widget_paint (GTK_WIDGET (label),
						 GTK_WIDGET (label)->style->white_gc,
						 TRUE,
						 label->details->background_mode,
						 label->details->solid_background_color,
						 label->details->tile_pixbuf,
						 tile_bounds,
						 label->details->tile_opacity,
						 label->details->tile_mode_vertical,
						 label->details->tile_mode_horizontal,
						 content_bounds,
						 label->details->text_opacity,
						 screen_dirty_area,
						 label_paint_pixbuf_callback,
						 label_composite_text_and_shadow_callback,
						 NULL);
		}
	} else {
		/* Paint the text if needed */
		text_bounds = label_get_text_bounds (label);
		eel_smooth_widget_paint (GTK_WIDGET (label),
					 GTK_WIDGET (label)->style->white_gc,
					 TRUE,
					 label->details->background_mode,
					 label->details->solid_background_color,
					 label->details->tile_pixbuf,
					 tile_bounds,
					 label->details->tile_opacity,
					 label->details->tile_mode_vertical,
					 label->details->tile_mode_horizontal,
					 text_bounds,
					 label->details->text_opacity,
					 screen_dirty_area,
					 label_paint_pixbuf_callback,
					 label_composite_text_callback,
					 NULL);
	}
}

static void
paint_label_smooth_cached (EelLabel *label,
			   ArtIRect screen_dirty_area,
			   ArtIRect tile_bounds)
{
	ArtIRect content_bounds;

	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (GTK_WIDGET_REALIZED (label));
	g_return_if_fail (label_can_cache_contents (label));

	content_bounds = label_get_content_bounds (label);

	eel_smooth_widget_paint (GTK_WIDGET (label),
				 GTK_WIDGET (label)->style->white_gc,
				 TRUE,
				 label->details->background_mode,
				 label->details->solid_background_color,
				 label->details->tile_pixbuf,
				 tile_bounds,
				 label->details->tile_opacity,
				 label->details->tile_mode_vertical,
				 label->details->tile_mode_horizontal,
				 content_bounds,
				 label->details->text_opacity,
				 screen_dirty_area,
				 label_paint_pixbuf_callback,
				 label_composite_text_callback_cached,
				 NULL);
}

static int
eel_label_expose_event (GtkWidget *widget,
			GdkEventExpose *event)
{
 	EelLabel *label;
	ArtIRect dirty_area;
	ArtIRect screen_dirty_area;
	ArtIRect tile_bounds;

	g_return_val_if_fail (EEL_IS_LABEL (widget), TRUE);
	g_return_val_if_fail (GTK_WIDGET_REALIZED (widget), TRUE);
	g_return_val_if_fail (event != NULL, TRUE);
	g_return_val_if_fail (event->window == widget->window, TRUE);
	
 	label = EEL_LABEL (widget);

	/* Check for the dumb case when theres nothing to do */
	if (eel_strlen (label_peek_text (label)) == 0 && label->details->tile_pixbuf == NULL) {
		return TRUE;
	}

	/* Clip the dirty area to the screen */
	dirty_area = eel_gdk_rectangle_to_art_irect (event->area);
	screen_dirty_area = eel_gdk_window_clip_dirty_area_to_screen (event->window,
								      dirty_area);

	/* Bail if no work */
	if (art_irect_empty (&screen_dirty_area)) {
		return TRUE;
	}

	/* Fetch the tile bounds */
	tile_bounds = eel_smooth_widget_get_tile_bounds (widget,
							 label->details->tile_pixbuf,
							 label->details->tile_width,
							 label->details->tile_height);

	/* Paint GtkLabel */
	if (!label_is_smooth (label)) {
		label_paint (label, screen_dirty_area, tile_bounds);
		return TRUE;
	}

	/* Paint smooth */
	if (label_can_cache_contents (label)) {
		paint_label_smooth_cached (label, screen_dirty_area, tile_bounds);
	} else {
		paint_label_smooth (label, screen_dirty_area, tile_bounds);	
	}

	return TRUE;
}

/* EelLabel signals */
static void
eel_label_set_is_smooth_signal (GtkWidget *widget,
				gboolean is_smooth)
{
	g_return_if_fail (EEL_IS_LABEL (widget));

	eel_label_set_is_smooth (EEL_LABEL (widget), is_smooth);
}

/* Private EelLabel things */
static int
label_get_default_line_wrap_width (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);
	
	return eel_scalable_font_text_width (label->details->smooth_font,
					     label->details->smooth_font_size,
					     DEFAULT_LINE_WRAP_WIDTH_TEXT,
					     strlen (DEFAULT_LINE_WRAP_WIDTH_TEXT));
}

static EelDimensions
label_get_text_dimensions (const EelLabel *label)
{
	EelDimensions text_dimensions;

	g_return_val_if_fail (EEL_IS_LABEL (label), eel_dimensions_empty);
	label_smooth_text_ensure (label);
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (label->details->smooth_text_layout), eel_dimensions_empty);

	text_dimensions = eel_smooth_text_layout_get_dimensions (label->details->smooth_text_layout);

	return text_dimensions;
}

static ArtIRect
label_get_text_bounds (const EelLabel *label)
{
	ArtIRect content_bounds;
	EelDimensions text_dimensions;
	ArtIRect text_bounds;

	g_return_val_if_fail (EEL_IS_LABEL (label), eel_art_irect_empty);
	
	content_bounds = label_get_content_bounds (label);
	text_dimensions = label_get_text_dimensions (label);

	if (eel_dimensions_are_empty (text_dimensions)
	    || art_irect_empty (&content_bounds)) {
		return eel_art_irect_empty;
	}
	
	text_bounds = eel_art_irect_assign (content_bounds.x0,
					    content_bounds.y0,
					    text_dimensions.width,
					    text_dimensions.height);

	return text_bounds;
}

static EelDimensions
label_get_content_dimensions (const EelLabel *label)
{
	EelDimensions text_dimensions;

	g_return_val_if_fail (EEL_IS_LABEL (label), eel_dimensions_empty);

	text_dimensions = label_get_text_dimensions (label);
	text_dimensions.width += label->details->smooth_drop_shadow_offset;
	text_dimensions.height += label->details->smooth_drop_shadow_offset;

	return text_dimensions;
}

static ArtIRect
label_get_content_bounds (const EelLabel *label)
{
	EelDimensions content_dimensions;
	ArtIRect text_bounds;
	ArtIRect bounds;

	g_return_val_if_fail (EEL_IS_LABEL (label), eel_art_irect_empty);

	content_dimensions = label_get_content_dimensions (label);

	if (eel_dimensions_are_empty (content_dimensions)) {
		return eel_art_irect_empty;
	}
	
	bounds = eel_gtk_widget_get_bounds (GTK_WIDGET (label));
	
	text_bounds = eel_art_irect_align (bounds,
					   content_dimensions.width,
					   content_dimensions.height,
					   GTK_MISC (label)->xalign,
					   GTK_MISC (label)->yalign);
		

	return text_bounds;
}

static EelDimensions
label_get_tile_dimensions (const EelLabel *label)
{
	EelDimensions tile_dimensions;

	g_return_val_if_fail (EEL_IS_LABEL (label), eel_dimensions_empty);

	if (!label->details->tile_pixbuf) {
		return eel_dimensions_empty;
	}
	
	tile_dimensions.width = gdk_pixbuf_get_width (label->details->tile_pixbuf);
	tile_dimensions.height = gdk_pixbuf_get_height (label->details->tile_pixbuf);

	return tile_dimensions;
}

static void
label_solid_cache_pixbuf_clear (EelLabel *label)
{
	g_return_if_fail (EEL_IS_LABEL (label));

	eel_gdk_pixbuf_unref_if_not_null (label->details->solid_cache_pixbuf);
	label->details->solid_cache_pixbuf = NULL;
}

static gboolean
label_can_cache_contents (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), FALSE);

	return (label->details->background_mode == EEL_SMOOTH_BACKGROUND_SOLID_COLOR)
		&& !label->details->tile_pixbuf;
}

const char *
label_peek_text (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), NULL);

	return GTK_LABEL (label)->label;
}

static void
label_smooth_text_ensure (const EelLabel *label)
{
	const char *text;
	g_return_if_fail (EEL_IS_LABEL (label));

	if (label->details->smooth_text_layout != NULL) {
		return;
	}

	text = label_peek_text (label);
	label->details->smooth_text_layout = eel_smooth_text_layout_new (text,
									 eel_strlen (text),
									 label->details->smooth_font,
									 label->details->smooth_font_size,
									 eel_label_get_wrap (label));
	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (label->details->smooth_text_layout));
	eel_smooth_text_layout_set_line_wrap_width (label->details->smooth_text_layout,
						    label->details->smooth_line_wrap_width);
}

static void
label_smooth_text_clear (EelLabel *label)
{
	g_return_if_fail (EEL_IS_LABEL (label));

	if (label->details->smooth_text_layout != NULL) {
		gtk_object_unref (GTK_OBJECT (label->details->smooth_text_layout));
	}
	label->details->smooth_text_layout = NULL;

	/* We also need to clear the cache pixbuf */
	label_solid_cache_pixbuf_clear (label);
}

gboolean
label_is_smooth (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), FALSE);

	return !label->details->never_smooth && label->details->is_smooth;
}

/* Public EelLabel methods */
GtkWidget *
eel_label_new (const char *text)
{
	EelLabel *label;
	
	label = EEL_LABEL (gtk_widget_new (eel_label_get_type (), NULL));
	
	eel_label_set_text (label, text);
	
	return GTK_WIDGET (label);
}

void
eel_label_set_smooth_font (EelLabel *label,
			   EelScalableFont *smooth_font)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (EEL_IS_SCALABLE_FONT (smooth_font));
	
	if (label->details->smooth_font == smooth_font) {
		return;
	}
	
	if (label->details->smooth_font != NULL) {
		gtk_object_unref (GTK_OBJECT (label->details->smooth_font));
	}

	gtk_object_ref (GTK_OBJECT (smooth_font));
	label->details->smooth_font = smooth_font;

	label_smooth_text_clear (label);	
	gtk_widget_queue_resize (GTK_WIDGET (label));
}

EelScalableFont *
eel_label_get_smooth_font (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), NULL);
	
	if (label->details->smooth_font != NULL) {
		gtk_object_ref (GTK_OBJECT (label->details->smooth_font));
	}

	return label->details->smooth_font;
}

void
eel_label_set_smooth_font_size (EelLabel *label,
				int smooth_font_size)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (smooth_font_size > MIN_SMOOTH_FONT_SIZE);

	if (label->details->smooth_font_size == smooth_font_size) {
		return;
	}

	label->details->smooth_font_size = smooth_font_size;

	label_smooth_text_clear (label);
	gtk_widget_queue_resize (GTK_WIDGET (label));
}

int
eel_label_get_smooth_font_size (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);

	return label->details->smooth_font_size;
}

/* This function is an ugly hack.  The issue is that we want
 * GtkLabel to flush its cached requisition dimensions.  GtkLabel
 * caches these for efficiency.  Unfortunately, there is no public
 * way to do this for GtkLabel.  So, instead we trick the GtkLabel
 * into thinking that its 'pattern' has changed.  As a result
 * of this phony change, GtkLabel will flush the requisition cache.
 * Of course, we don't really change the pattern, we just set the
 * old one.
 */
static void
label_force_cached_requisition_flush (EelLabel *label)
{
	char *same_pattern;
	g_return_if_fail (EEL_IS_LABEL (label));

	same_pattern = g_strdup (GTK_LABEL (label)->pattern);
	gtk_label_set_pattern (GTK_LABEL (label), same_pattern);
	g_free (same_pattern);
}

void
eel_label_set_is_smooth (EelLabel *label,
			 gboolean is_smooth)
{
	g_return_if_fail (EEL_IS_LABEL (label));

	if (label->details->never_smooth) {
		return;
	}

	if (label->details->is_smooth == is_smooth) {
		return;
	}

	label->details->is_smooth = is_smooth;

	label_smooth_text_clear (label);

	/* Force GtkLabel to flush its cached requisition dimensions.
	 * GtkLabel caches its requisition for efficiency.  We need this
	 * dimensions to be flushed when our is_smooth attribute changes.
	 * The reason is that the geometry of the widget is dependent on
	 * whether it is_smooth or not.
	 */
	label_force_cached_requisition_flush (label);

	gtk_widget_queue_resize (GTK_WIDGET (label));
}

gboolean
eel_label_get_is_smooth (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), FALSE);

	return label_is_smooth (label);
}

/**
 * eel_label_set_tile_pixbuf:
 *
 * @label: A EelLabel
 * @pixbuf:          The new tile pixbuf
 *
 * Change the tile pixbuf.  A 'pixbuf' value of NULL, means dont use a
 * tile pixbuf - this is the default behavior for the widget.
 */
void
eel_label_set_tile_pixbuf (EelLabel *label,
			   GdkPixbuf *pixbuf)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	
	if (pixbuf != label->details->tile_pixbuf) {
		eel_gdk_pixbuf_unref_if_not_null (label->details->tile_pixbuf);
		eel_gdk_pixbuf_ref_if_not_null (pixbuf);
		
		label->details->tile_pixbuf = pixbuf;

		gtk_widget_queue_resize (GTK_WIDGET (label));
	}
}

/**
 * eel_label_get_tile_pixbuf:
 *
 * @label: A EelLabel
 *
 * Return value: A reference to the tile_pixbuf.  Needs to be unreferenced with 
 * gdk_pixbuf_unref()
 */
GdkPixbuf*
eel_label_get_tile_pixbuf (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), NULL);

	eel_gdk_pixbuf_ref_if_not_null (label->details->tile_pixbuf);
	
	return label->details->tile_pixbuf;
}

void
eel_label_set_text_opacity (EelLabel *label,
			    int opacity)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (opacity >= EEL_OPACITY_FULLY_TRANSPARENT);
	g_return_if_fail (opacity <= EEL_OPACITY_FULLY_OPAQUE);

	label->details->text_opacity = opacity;

	gtk_widget_queue_draw (GTK_WIDGET (label));
}

int
eel_label_get_text_opacity (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), EEL_OPACITY_FULLY_OPAQUE);

	return label->details->text_opacity;
}

void
eel_label_set_tile_opacity (EelLabel *label,
			    int tile_opacity)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (tile_opacity >= EEL_OPACITY_FULLY_TRANSPARENT);
	g_return_if_fail (tile_opacity <= EEL_OPACITY_FULLY_OPAQUE);

	if (label->details->tile_opacity == tile_opacity) {
		return;
	}

	label->details->tile_opacity = tile_opacity;

	gtk_widget_queue_draw (GTK_WIDGET (label));
}

int
eel_label_get_tile_opacity (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), EEL_OPACITY_FULLY_OPAQUE);

	return label->details->tile_opacity;
}

void
eel_label_set_tile_width (EelLabel *label,
			  int tile_width)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (tile_width >= EEL_SMOOTH_TILE_EXTENT_ONE_STEP);
	
	if (label->details->tile_width == tile_width) {
		return;
	}

	label->details->tile_width = tile_width;

	gtk_widget_queue_resize (GTK_WIDGET (label));
}

/**
 * eel_label_get_tile_width:
 *
 * @label: A EelLabel
 *
 * Return value: The tile width.
 */
int
eel_label_get_tile_width (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);

	return label->details->tile_width;
}

void
eel_label_set_tile_height (EelLabel *label,
			   int tile_height)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (tile_height >= EEL_SMOOTH_TILE_EXTENT_ONE_STEP);
	
	if (label->details->tile_height == tile_height) {
		return;
	}

	label->details->tile_height = tile_height;

	gtk_widget_queue_resize (GTK_WIDGET (label));
}

/**
 * eel_label_get_tile_height:
 *
 * @label: A EelLabel
 *
 * Return value: The tile height.
 */
int
eel_label_get_tile_height (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);

	return label->details->tile_height;
}

void
eel_label_set_tile_mode_vertical (EelLabel *label,
				  EelSmoothTileMode tile_mode_vertical)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (tile_mode_vertical >= EEL_SMOOTH_TILE_SELF);
	g_return_if_fail (tile_mode_vertical <= EEL_SMOOTH_TILE_ANCESTOR);

	if (label->details->tile_mode_vertical == tile_mode_vertical) {
		return;
	}

	label->details->tile_mode_vertical = tile_mode_vertical;

	gtk_widget_queue_draw (GTK_WIDGET (label));
}

EelSmoothTileMode
eel_label_get_tile_mode_vertical (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);
	
	return label->details->tile_mode_vertical;
}

void
eel_label_set_tile_mode_horizontal (EelLabel *label,
				    EelSmoothTileMode tile_mode_horizontal)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (tile_mode_horizontal >= EEL_SMOOTH_TILE_SELF);
	g_return_if_fail (tile_mode_horizontal <= EEL_SMOOTH_TILE_ANCESTOR);

	if (label->details->tile_mode_horizontal == tile_mode_horizontal) {
		return;
	}

	label->details->tile_mode_horizontal = tile_mode_horizontal;

	gtk_widget_queue_draw (GTK_WIDGET (label));
}

EelSmoothTileMode
eel_label_get_tile_mode_horizontal (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);
	
	return label->details->tile_mode_horizontal;
}

void
eel_label_set_tile_pixbuf_from_file_name (EelLabel *label,
					  const char *tile_file_name)
{
	GdkPixbuf *tile_pixbuf;

	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (tile_file_name != NULL);

	tile_pixbuf = gdk_pixbuf_new_from_file (tile_file_name);
	
	if (tile_pixbuf != NULL) {
		eel_label_set_tile_pixbuf (label, tile_pixbuf);
		gdk_pixbuf_unref (tile_pixbuf);
	}
}

void
eel_label_set_background_mode (EelLabel *label,
			       EelSmoothBackgroundMode background_mode)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (background_mode >= EEL_SMOOTH_BACKGROUND_GTK);
	g_return_if_fail (background_mode <= EEL_SMOOTH_BACKGROUND_SOLID_COLOR);

	if (label->details->background_mode == background_mode) {
		return;
	}

	label->details->background_mode = background_mode;

	gtk_widget_queue_draw (GTK_WIDGET (label));
}

EelSmoothBackgroundMode
eel_label_get_background_mode (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);
	
	return label->details->background_mode;
}

void
eel_label_set_solid_background_color (EelLabel *label,
				      guint32 solid_background_color)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	
	if (label->details->solid_background_color == solid_background_color) {
		return;
	}

	label->details->solid_background_color = solid_background_color;

	label_solid_cache_pixbuf_clear (label);
	
	gtk_widget_queue_draw (GTK_WIDGET (label));
}

guint32
eel_label_get_solid_background_color (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);
	
	return label->details->solid_background_color;
}

/**
 * eel_label_set_wrap_width:
 *
 * @label: A EelLabel
 * @line_wrap_width: The new line wrap width.
 *
 * The line wrap width is something.
 * 
 */
void
eel_label_set_smooth_line_wrap_width (EelLabel *label,
				      int line_wrap_width)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	
	if (label->details->smooth_line_wrap_width == line_wrap_width) {
		return;
	}
	
	label->details->smooth_line_wrap_width = line_wrap_width;
	
	label_smooth_text_clear (label);
	gtk_widget_queue_resize (GTK_WIDGET (label));
}

/**
 * eel_label_get_smooth_line_wrap_width:
 *
 * @label: A EelLabel
 *
 * Return value: A boolean value indicating whether the label
 * is currently line wrapping text.
 */
int
eel_label_get_smooth_line_wrap_width (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), FALSE);

	return label->details->smooth_line_wrap_width;
}

void
eel_label_set_text_color (EelLabel *label,
			  guint32 text_color)
{
	char *color_spec;

	g_return_if_fail (EEL_IS_LABEL (label));

	if (label->details->smooth_text_color == text_color) {
		return;
	}
	
	label->details->smooth_text_color = text_color;

	color_spec = eel_gdk_rgb_to_color_spec (text_color);

	eel_gtk_widget_set_foreground_color (GTK_WIDGET (label), color_spec);
	
	g_free (color_spec);

	label_solid_cache_pixbuf_clear (label);
	
	gtk_widget_queue_draw (GTK_WIDGET (label));
}

guint32
eel_label_get_text_color (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);
	
	return label->details->smooth_text_color;
}

/**
 * eel_label_set_smooth_drop_shadow_offset:
 *
 * @label: A EelLabel
 * @smooth_drop_shadow_offset: The new drop shadow offset.  

 * The drop shadow offset is specified in pixels.  If greater than zero,
 * the label will render on top of a nice shadow.  The shadow will be
 * offset from the label text by 'smooth_drop_shadow_offset' pixels.
 */
void
eel_label_set_smooth_drop_shadow_offset (EelLabel *label,
					 int drop_shadow_offset)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (drop_shadow_offset >= 0);
	 
	if (label->details->smooth_drop_shadow_offset == drop_shadow_offset) {
		return;
	}
	 
	label->details->smooth_drop_shadow_offset = drop_shadow_offset;
	label_solid_cache_pixbuf_clear (label);
	gtk_widget_queue_resize (GTK_WIDGET (label));
}

/**
 * eel_label_get_smooth_drop_shadow_offset:
 *
 * @label: A EelLabel
 *
 * Return value: The line offset in pixels.
 */
int
eel_label_get_smooth_drop_shadow_offset (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);

	return label->details->smooth_drop_shadow_offset;
}

/**
 * eel_label_set_smooth_drop_shadow_color:
 *
 * @label: A EelLabel
 * @smooth_drop_shadow_color: The new drop shadow color.
 *
 * Return value: The drop shadow color.
 */
void
eel_label_set_smooth_drop_shadow_color (EelLabel *label,
					guint32 drop_shadow_color)
{
	g_return_if_fail (EEL_IS_LABEL (label));

	if (label->details->smooth_drop_shadow_color == drop_shadow_color) {
		return;
	}
	
	label->details->smooth_drop_shadow_color = drop_shadow_color;
	label_solid_cache_pixbuf_clear (label);	
	gtk_widget_queue_draw (GTK_WIDGET (label));
}

/**
 * eel_label_get_smooth_drop_shadow_color:
 *
 * @label: A EelLabel
 *
 * Return value: The drop shadow color.
 */
guint32
eel_label_get_smooth_drop_shadow_color (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);
	
	return label->details->smooth_drop_shadow_color;
}

void
eel_label_set_justify (EelLabel *label,
		       GtkJustification justification)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	g_return_if_fail (justification >= GTK_JUSTIFY_LEFT);
	g_return_if_fail (justification <= GTK_JUSTIFY_FILL);

	if (eel_label_get_text_justify (label) == justification) {
		return;
	}

	gtk_label_set_justify (GTK_LABEL (label), justification);
	
	gtk_widget_queue_draw (GTK_WIDGET (label));
}

GtkJustification
eel_label_get_text_justify (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), 0);

	return GTK_LABEL (label)->jtype;
}

gboolean
eel_label_set_text (EelLabel *label,
		    const char *text)
{
	GtkLabel *gtk_label;

	g_return_val_if_fail (EEL_IS_LABEL (label), FALSE);

	gtk_label = GTK_LABEL (label);

	if (eel_str_is_equal (text, gtk_label->label)) {
		return FALSE;
	}

	gtk_label_set_text (gtk_label, text);
	
	label_smooth_text_clear (label);
	gtk_widget_queue_resize (GTK_WIDGET (label));

	return TRUE;
}

char*
eel_label_get_text (const EelLabel *label)
{
	GtkLabel *gtk_label;

	g_return_val_if_fail (EEL_IS_LABEL (label), NULL);

	gtk_label = GTK_LABEL (label);

	return gtk_label->label ? g_strdup (gtk_label->label) : NULL;
}

/**
 * eel_label_set_wrap:
 *
 * @label: A EelLabel
 * @line_wrap: A boolean value indicating whether the label should
 * line wrap words if they dont fit in the horizontally allocated
 * space.
 *
 */
void
eel_label_set_wrap (EelLabel *label,
		    gboolean line_wrap)
{
	g_return_if_fail (EEL_IS_LABEL (label));

	if (eel_label_get_wrap (label) == line_wrap) {
		return;
	}

	gtk_label_set_line_wrap (GTK_LABEL (label), line_wrap);
	
	label_smooth_text_clear (label);	
	gtk_widget_queue_resize (GTK_WIDGET (label));
}

/**
 * eel_label_get_wrap:
 *
 * @label: A EelLabel
 *
 * Return value: A boolean value indicating whether the label
 * is currently line wrapping text.
 */
gboolean
eel_label_get_wrap (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), FALSE);

	return GTK_LABEL (label)->wrap;
}

/**
 * eel_label_new_solid:
 *
 * @text: Text or NULL
 * @drop_shadow_offset: Drop shadow offset.
 * @drop_shadow_color: Drop shadow color.
 * @text_color: Text color.
 * @x_alignment: Horizontal alignment.
 * @y_alignment: Vertical alignment.
 * @x_padding: Amount to pad label in the x direction.
 * @y_padding: Amount to pad label in the y direction.
 * @background_color: Background color.
 * @tile_pixbuf: Pixbuf to use for tile or NULL.
 *
 * Create a label with a solid background.
 *
 * Return value: Newly created label with all the given values.
 */
GtkWidget *
eel_label_new_solid (const char *text,
		     int drop_shadow_offset,
		     guint32 drop_shadow_color,
		     guint32 text_color,
		     float x_alignment,
		     float y_alignment,
		     int x_padding,
		     int y_padding,
		     guint32 background_color,
		     GdkPixbuf *tile_pixbuf)
{
	EelLabel *label;

 	label = EEL_LABEL (eel_label_new (text ? text : ""));

 	eel_label_set_background_mode (EEL_LABEL (label), EEL_SMOOTH_BACKGROUND_SOLID_COLOR);
	eel_label_set_smooth_drop_shadow_color (label, drop_shadow_color);
	eel_label_set_smooth_drop_shadow_offset (label, drop_shadow_offset);
	eel_label_set_text_color (label, text_color);
 	eel_label_set_solid_background_color (EEL_LABEL (label), background_color);

 	gtk_misc_set_padding (GTK_MISC (label), x_padding, y_padding);
 	gtk_misc_set_alignment (GTK_MISC (label), x_alignment, y_alignment);
	
	if (tile_pixbuf != NULL) {
		eel_label_set_tile_pixbuf (EEL_LABEL (label), tile_pixbuf);
	}
	
	return GTK_WIDGET (label);
}

/**
 * eel_gtk_label_make_bold.
 *
 * Switches the font of label to a bold equivalent.
 * @label: The label.
 **/
void
eel_label_make_bold (EelLabel *label)
{
	EelScalableFont *bold_font;

	g_return_if_fail (EEL_IS_LABEL (label));

	eel_gtk_label_make_bold (GTK_LABEL (label));

	bold_font = eel_scalable_font_make_bold (label->details->smooth_font);
	g_assert (EEL_IS_SCALABLE_FONT (bold_font));

	if (bold_font != NULL) {
		eel_label_set_smooth_font (label, bold_font);
		gtk_object_unref (GTK_OBJECT (bold_font));
	}

	
	label_smooth_text_clear (label);
	gtk_widget_queue_resize (GTK_WIDGET (label));
}

/**
 * eel_gtk_label_make_larger.
 *
 * Switches the font of label to a larger version of the font.
 * @label: The label.
 **/
void
eel_label_make_larger (EelLabel *label,
		       guint num_steps)
{
	g_return_if_fail (EEL_IS_LABEL (label));

	label->details->smooth_font_size += num_steps;

	eel_gtk_label_make_larger (GTK_LABEL (label), num_steps);

	label_smooth_text_clear (label);
	gtk_widget_queue_resize (GTK_WIDGET (label));
}

/**
 * eel_gtk_label_make_smaller.
 *
 * Switches the font of label to a smaller version of the font.
 * @label: The label.
 **/
void
eel_label_make_smaller (EelLabel *label,
			guint num_steps)
{
	g_return_if_fail (EEL_IS_LABEL (label));

	label->details->smooth_font_size -= num_steps;
	
	eel_gtk_label_make_smaller (GTK_LABEL (label), num_steps);

	label_smooth_text_clear (label);
	gtk_widget_queue_resize (GTK_WIDGET (label));
}

/**
 * eel_label_set_never_smooth
 *
 * @label: A EelLabel.
 * @never_smooth: A boolean value indicating whether the label can NEVER be smooth.
 *
 * Force an label to never be smooth.  Calls to eel_label_set_is_smooth () will
 * thus be ignored.  This is useful if you want to use a EelLabel in a situation
 * wherre smoothness does not make sense - for example, in a dialog with other "normal"
 * GtkLabel widgets for consistency.
 */
void
eel_label_set_never_smooth (EelLabel *label,
			    gboolean never_smooth)
{
	g_return_if_fail (EEL_IS_LABEL (label));
	
	label->details->never_smooth = never_smooth;

	label_smooth_text_clear (label);

	/* Force GtkLabel to flush its cached requisition dimensions.
	 * GtkLabel caches its requisition for efficiency.  We need this
	 * dimensions to be flushed when our is_smooth attribute changes.
	 * The reason is that the geometry of the widget is dependent on
	 * whether it is_smooth or not.
	 */
	label_force_cached_requisition_flush (label);

	gtk_widget_queue_resize (GTK_WIDGET (label));
}

/**
 * eel_label_set_adjust_wrap_on_resize:
 *
 * @label: A EelLabel
 * @adjust_wrap_on_resize: A boolean value indicating whether the label should
 * automatically update the line_wrap_width when its resized.
 */
void
eel_label_set_adjust_wrap_on_resize (EelLabel *label,
				     gboolean adjust_wrap_on_resize)
{
	g_return_if_fail (EEL_IS_LABEL (label));

	if (label->details->adjust_wrap_on_resize == adjust_wrap_on_resize) {
		return;
	}

	label->details->adjust_wrap_on_resize = adjust_wrap_on_resize;
	
	gtk_widget_queue_resize (GTK_WIDGET (label));
}

/**
 * eel_label_get_wrap:
 *
 * @label: A EelLabel
 *
 * Return value: A boolean value indicating whether the label
 * automatically updates the line_wrap_width when its resized.
 */
gboolean
eel_label_get_adjust_wrap_on_resize (const EelLabel *label)
{
	g_return_val_if_fail (EEL_IS_LABEL (label), FALSE);

	return label->details->adjust_wrap_on_resize;
}
