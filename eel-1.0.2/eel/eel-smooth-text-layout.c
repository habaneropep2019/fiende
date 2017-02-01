/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-smooth-text-layout.c - A GtkObject subclass for dealing with smooth text.

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
#include "eel-smooth-text-layout.h"

#include "eel-gtk-macros.h"
#include "eel-gdk-extensions.h"
#include "eel-gdk-pixbuf-extensions.h"
#include "eel-string.h"
#include "eel-glyph.h"
#include "eel-debug-drawing.h"

#include <libgnome/gnome-i18n.h>

#define MIN_FONT_SIZE 5
#define DEFAULT_LINE_SPACING 0
#define DEFAULT_FONT_SIZE 14

#define UNDEFINED_DIMENSION -1

/* This magic string is copied from GtkLabel.  It lives there unlocalized as well. */
#define DEFAULT_LINE_WRAP_WIDTH_TEXT "This is a good enough length for any line to have."

#define DEFAULT_LINE_BREAK_CHARACTERS _(" -_,;.?/&")

/* Detail member struct */
struct EelSmoothTextLayoutDetails
{
	EelDimensions dimensions;

	char *text;
	int text_length;

 	/* Smooth attributes */
 	EelScalableFont *font;
 	int font_size;
 	int line_spacing;
 	int empty_line_height;

	/* Text lines */
	GList *text_line_list;
	int max_line_width;
	int num_empty_lines;
	int line_wrap_width;
	int total_line_height;
	
	gboolean wrap;
	char *line_break_characters;
};

/* GtkObjectClass methods */
static void   eel_smooth_text_layout_initialize_class     (EelSmoothTextLayoutClass  *smooth_text_layout_class);
static void   eel_smooth_text_layout_initialize           (EelSmoothTextLayout       *smooth_text_layout);
static void   eel_smooth_text_layout_destroy              (GtkObject                 *object);

/* Private functions */
static void   smooth_text_layout_set_text                 (EelSmoothTextLayout       *smooth_text_layout,
							   const char                *text,
							   int                        text_length);
static void   smooth_text_layout_clear_lines              (EelSmoothTextLayout       *smooth_text_layout);
static void   smooth_text_layout_ensure_lines             (const EelSmoothTextLayout *smooth_text_layout);
static int    smooth_text_layout_get_num_empty_lines      (const EelSmoothTextLayout *smooth_text_layout);
static int    smooth_text_layout_get_empty_line_height    (const EelSmoothTextLayout *smooth_text_layout);
static int    smooth_text_layout_get_max_line_width       (const EelSmoothTextLayout *smooth_text_layout);
static int    smooth_text_layout_get_total_line_height    (const EelSmoothTextLayout *smooth_text_layout);
static int    smooth_text_layout_get_line_wrap_width      (const EelSmoothTextLayout *smooth_text_layout);
static GList *smooth_text_layout_line_list_new            (const char                *text,
							   int                        text_length,
							   EelScalableFont           *font,
							   int                        font_size);
static void   smooth_text_layout_line_list_free           (GList                     *smooth_line_list);
void          smooth_text_layout_line_list_draw_to_pixbuf (GList                     *smooth_line_list,
							   GdkPixbuf                 *pixbuf,
							   int                        x,
							   int                        y,
							   GtkJustification           justification,
							   gboolean                   underlined,
							   int                        empty_line_height,
							   int                        max_line_width,
							   int                        line_spacing,
							   guint32                    color,
							   int                        opacity);
static GList *smooth_text_layout_line_list_new_wrapped    (const char                *text,
							   int                        text_length,
							   EelScalableFont           *font,
							   int                        font_size,
							   int                        max_width,
							   const char                *line_break_characters);

/*
 * The following text_layout stuff was shamelessly plundered
 * from libgnomeui/gnome-icon-text.[ch] by Federico Mena.
 *
 * It was hacked to use EelScalableFont and GdkPixbuf
 * instead of GdkFont and GdkDrawable.  We want to use the
 * same layout algorithm in Eel so that both the smooth
 * and not smooth text rendering cases have predictably 
 * similar result.
 *
 * I also made some minor Eel-like style changes. -re

 */
typedef struct
{
	char *text;
	int width;
	guint text_length;
} EelTextLayoutRow;

typedef struct
{
	GList *rows;
	const EelScalableFont *font;
	int font_size;
	int width;
	int height;
	int baseline_skip;
} EelTextLayout;

EelTextLayout *eel_text_layout_new  (const EelScalableFont *font,
				     int                    font_size,
				     const char            *text,
				     const char            *separators,
				     int                    max_width,
				     gboolean               confine);
void           eel_text_layout_free (EelTextLayout         *text_info);
					    
EEL_DEFINE_CLASS_BOILERPLATE (EelSmoothTextLayout, eel_smooth_text_layout, GTK_TYPE_OBJECT)

/* Class init methods */
	static void
eel_smooth_text_layout_initialize_class (EelSmoothTextLayoutClass *smooth_text_layout_class)
{
	GtkObjectClass *object_class = GTK_OBJECT_CLASS (smooth_text_layout_class);
	
	/* GtkObjectClass */
	object_class->destroy = eel_smooth_text_layout_destroy;
}

void
eel_smooth_text_layout_initialize (EelSmoothTextLayout *smooth_text_layout)
{
	smooth_text_layout->details = g_new0 (EelSmoothTextLayoutDetails, 1);
	smooth_text_layout->details->line_break_characters = g_strdup (DEFAULT_LINE_BREAK_CHARACTERS);
	smooth_text_layout->details->font = eel_scalable_font_get_default_font ();
	smooth_text_layout->details->font_size = DEFAULT_FONT_SIZE;
	smooth_text_layout->details->line_spacing = DEFAULT_LINE_SPACING;
	smooth_text_layout_clear_lines (smooth_text_layout);
}

/* GtkObjectClass methods */
static void
eel_smooth_text_layout_destroy (GtkObject *object)
{
 	EelSmoothTextLayout *smooth_text_layout;

	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (object));

	smooth_text_layout = EEL_SMOOTH_TEXT_LAYOUT (object);

	smooth_text_layout_clear_lines (smooth_text_layout);
	/* FIXME: it seems like we should unref the font here. */
	g_free (smooth_text_layout->details->line_break_characters);
	g_free (smooth_text_layout->details->text);

	g_free (smooth_text_layout->details);

	/* Chain destroy */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/* Private EelSmoothTextLayout functions */
static void
smooth_text_layout_clear_lines (EelSmoothTextLayout *smooth_text_layout)
{

	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));

	smooth_text_layout_line_list_free (smooth_text_layout->details->text_line_list);
	smooth_text_layout->details->text_line_list = NULL;
	smooth_text_layout->details->dimensions.width = UNDEFINED_DIMENSION;
	smooth_text_layout->details->dimensions.height = UNDEFINED_DIMENSION;
	smooth_text_layout->details->max_line_width = UNDEFINED_DIMENSION;
	smooth_text_layout->details->num_empty_lines = UNDEFINED_DIMENSION;
	smooth_text_layout->details->empty_line_height = UNDEFINED_DIMENSION;
	smooth_text_layout->details->line_wrap_width = UNDEFINED_DIMENSION;
	smooth_text_layout->details->total_line_height = UNDEFINED_DIMENSION;

}

static void
smooth_text_layout_ensure_lines (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));

	if (smooth_text_layout->details->text_line_list != NULL) {
		return;
	}
	
	/* We cheat a little here.  Pretend text_line_list or wrap_text_layouts are mutable */
	if (smooth_text_layout->details->wrap) {
		smooth_text_layout->details->text_line_list = 
			smooth_text_layout_line_list_new_wrapped (smooth_text_layout->details->text,
								  smooth_text_layout->details->text_length,
								  smooth_text_layout->details->font,
								  smooth_text_layout->details->font_size,
								  smooth_text_layout_get_line_wrap_width (smooth_text_layout),
								  smooth_text_layout->details->line_break_characters);
	} else {
		smooth_text_layout->details->text_line_list = 
			smooth_text_layout_line_list_new (smooth_text_layout->details->text,
							  smooth_text_layout->details->text_length,
							  smooth_text_layout->details->font,
							  smooth_text_layout->details->font_size);
	}
}

static GList *
smooth_text_layout_line_list_new (const char *text,
				  int text_length,
				  EelScalableFont *font,
				  int font_size)
{
	GList *line_list = NULL;
	const char *line;
	const char *end;

	g_return_val_if_fail (EEL_IS_SCALABLE_FONT (font), NULL);
	g_return_val_if_fail (text_length >= 0, NULL);
	g_return_val_if_fail (font_size >= MIN_FONT_SIZE, NULL);

	end = text + text_length;
	
	line = text;

	while (line != NULL && line <= end) {
		const char *next_line;
		int length;
		EelGlyph *glyph = NULL;

		next_line = strchr (line, '\n');

		if (next_line != NULL) {
			length = next_line - line;
		} else {
			length = end - line;
		}

		g_assert (length >= 0);

		if (length > 0) {
			glyph = eel_glyph_new (font, font_size, line, length);
		}

		line_list = g_list_append (line_list, glyph);

		if (next_line != NULL) {
			line = next_line + 1;
		}
		else {
			line = NULL;
		}
	}

	return line_list;
}

static void
smooth_text_layout_line_list_free (GList *smooth_line_list)
{
	GList *node;

	node = smooth_line_list;
	while (node) {
		if (node->data != NULL) {
			eel_glyph_free (node->data);
		}
		node = node->next;
	}

	g_list_free (smooth_line_list);
}

void
smooth_text_layout_line_list_draw_to_pixbuf (GList *text_line_list,
					     GdkPixbuf *pixbuf,
					     int x,
					     int y,
					     GtkJustification justification,
					     gboolean underlined,
					     int empty_line_height,
					     int max_line_width,
					     int line_spacing,
					     guint32 color,
					     int opacity)
{
 	GList *node;

	g_return_if_fail (text_line_list != NULL);
	g_return_if_fail (eel_gdk_pixbuf_is_valid (pixbuf));
	g_return_if_fail (justification >= GTK_JUSTIFY_LEFT && justification <= GTK_JUSTIFY_FILL);
	g_return_if_fail (empty_line_height > 0);
	g_return_if_fail (max_line_width > 0);
	g_return_if_fail (line_spacing >= 0);

	/* FIXME bugzilla.eazel.com 5087: Make sure the color we are fed is opaque.  The real solution is 
	 * to fix the callers.
	 */
	color = color | 0xFF000000;

 	node = text_line_list;
 	while (node) {
 		if (node->data != NULL) {
			EelGlyph *glyph;
			int text_x = 0;
			int text_y = 0;

			glyph = node->data;

			g_assert (max_line_width >= eel_glyph_get_width (glyph));

			switch (justification) {
			case GTK_JUSTIFY_LEFT:
				text_x = x;
				break;
				
			case GTK_JUSTIFY_CENTER:
			case GTK_JUSTIFY_FILL:
				text_x = x + (max_line_width - eel_glyph_get_width (glyph)) / 2;
				break;

			case GTK_JUSTIFY_RIGHT:
				text_x = x + (max_line_width - eel_glyph_get_width (glyph));
				break;
				
			default:
				g_assert_not_reached ();
				text_x = x;
			}
			
			text_y = y;
			
			eel_glyph_draw_to_pixbuf (glyph,
						  pixbuf,
						  text_x,
						  text_y,
						  eel_gdk_pixbuf_whole_pixbuf,
						  color,
						  opacity);

			/* Underline the text if needed */
			if (underlined) {
				ArtIRect underline_rect;

				underline_rect = eel_glyph_get_underline_rectangle (glyph);
				underline_rect.y0 += text_y;
				underline_rect.y1 += text_y;
				underline_rect.x0 += text_x;
				underline_rect.x1 += text_x;
				eel_gdk_pixbuf_fill_rectangle_with_color (pixbuf,
									  underline_rect,
									  color);
			}
			
			y += eel_glyph_get_height (glyph) + line_spacing;
 		} else {
			y += empty_line_height;
		}
 		node = node->next;
 	}
}

static GList *
smooth_text_layout_line_list_new_wrapped (const char *text,
					  int text_length,
					  EelScalableFont *font,
					  int font_size,
					  int max_width,
					  const char *line_break_characters)
{
	GList *line_list = NULL;
	GList *layout_list = NULL;
	GList *layout_node;
	const char *line;
	const char *end;

	g_return_val_if_fail (EEL_IS_SCALABLE_FONT (font), NULL);
	g_return_val_if_fail (text_length >= 0, NULL);
	g_return_val_if_fail (font_size >= MIN_FONT_SIZE, NULL);
	g_return_val_if_fail (max_width > 0, NULL);
	g_return_val_if_fail (line_break_characters != NULL, NULL);
	g_return_val_if_fail (line_break_characters[0] != '\0', NULL);

	end = text + text_length;
	line = text;

	while (line != NULL && line <= end) {
		/* NULL layout means empty line */
		EelTextLayout *layout = NULL;
		const char *next_line;
		int length;
		next_line = strchr (line, '\n');

		if (next_line != NULL) {
			length = next_line - line;
		} else {
			length = end - line;
		}

		g_assert (length >= 0);

		if (length > 0) {
			char *null_terminated_line;
			null_terminated_line = g_strndup (line, length);
			layout = eel_text_layout_new (font,
						      font_size,
						      null_terminated_line,
						      line_break_characters,
						      max_width,
						      TRUE);
			g_free (null_terminated_line);
		}

		layout_list = g_list_append (layout_list, layout);

		if (next_line != NULL) {
			line = next_line + 1;
		}
		else {
			line = NULL;
		}
	}

	layout_node = layout_list;
	while (layout_node != NULL) {
		if (layout_node->data != NULL) {
			EelTextLayout *layout;
			GList *layout_row_node;
			g_assert (layout_node->data != NULL);
			layout = layout_node->data;
			
			layout_row_node = layout->rows;
			while (layout_row_node != NULL) {
				/* NULL glyph means empty line */				
				EelGlyph *glyph = NULL;
				
				if (layout_row_node->data != NULL) {
					const EelTextLayoutRow *row;
					row = layout_row_node->data;
					
					glyph = eel_glyph_new (font, font_size, row->text, row->text_length);
				} else {
				}
				
				line_list = g_list_append (line_list, glyph);
				
				layout_row_node = layout_row_node->next;
			}
			
			eel_text_layout_free (layout);
		} else {
			line_list = g_list_append (line_list, NULL);
		}
		layout_node = layout_node->next;
	}
	
	g_list_free (layout_list);

	return line_list;
}

static int
smooth_text_layout_get_empty_line_height (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), 0);

	if (smooth_text_layout->details->empty_line_height == UNDEFINED_DIMENSION) {
		smooth_text_layout->details->empty_line_height = smooth_text_layout->details->font_size / 2;
	}

	return smooth_text_layout->details->empty_line_height;
}

static int
smooth_text_layout_get_num_empty_lines (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), 0);
	
	if (smooth_text_layout->details->num_empty_lines == UNDEFINED_DIMENSION) {
		GList *node;

		smooth_text_layout->details->num_empty_lines = 0;
		node = smooth_text_layout->details->text_line_list;
		while (node) {
			if (node->data == NULL) {
				smooth_text_layout->details->num_empty_lines++;
			}
			node = node->next;
		}
	}

	return smooth_text_layout->details->num_empty_lines;
}

static int
smooth_text_layout_get_max_line_width (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), 0);
	
	if (smooth_text_layout->details->max_line_width == UNDEFINED_DIMENSION) {
		GList *node;

		smooth_text_layout->details->max_line_width = 0;
		node = smooth_text_layout->details->text_line_list;
		while (node) {
			if (node->data != NULL) {
				smooth_text_layout->details->max_line_width = MAX (smooth_text_layout->details->max_line_width,
										   eel_glyph_get_width (node->data));
			}
			node = node->next;
		}
	}

	return smooth_text_layout->details->max_line_width;
}

static int
smooth_text_layout_get_total_line_height (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), 0);
	
	if (smooth_text_layout->details->total_line_height == UNDEFINED_DIMENSION) {
		GList *node;

		smooth_text_layout->details->total_line_height = 0;
		node = smooth_text_layout->details->text_line_list;
		while (node) {
			if (node->data != NULL) {
				smooth_text_layout->details->total_line_height += 
					eel_glyph_get_height (node->data);
			} else {
				smooth_text_layout->details->total_line_height += 
					smooth_text_layout_get_empty_line_height (smooth_text_layout);
			}

			node = node->next;
		}
	}

	return smooth_text_layout->details->total_line_height;
}

static int
smooth_text_layout_get_line_wrap_width (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), 0);
	
	if (smooth_text_layout->details->line_wrap_width == UNDEFINED_DIMENSION) {
		smooth_text_layout->details->line_wrap_width = 
			eel_scalable_font_text_width (smooth_text_layout->details->font,
						      smooth_text_layout->details->font_size,
						      DEFAULT_LINE_WRAP_WIDTH_TEXT,
						      strlen (DEFAULT_LINE_WRAP_WIDTH_TEXT));
		
	}

	return smooth_text_layout->details->line_wrap_width;
}

/* Public EelSmoothTextLayout methods */

/**
 * eel_smooth_text_layout_new:
 * @family: The desired smooth_text_layout family.
 * @weight: The desired smooth_text_layout weight.
 * @slant: The desired smooth_text_layout slant.
 * @set_width: The desired smooth_text_layout set_width.
 *
 * Returns a new smooth_text_layout.
 *
 */
EelSmoothTextLayout *
eel_smooth_text_layout_new (const char *text,
			    int text_length,
			    EelScalableFont *font,
			    int font_size,
			    gboolean wrap)
{
	EelSmoothTextLayout *smooth_text_layout;

	g_return_val_if_fail (EEL_IS_SCALABLE_FONT (font), NULL);
	g_return_val_if_fail (font_size >= MIN_FONT_SIZE, NULL);

	smooth_text_layout = EEL_SMOOTH_TEXT_LAYOUT (gtk_object_new (eel_smooth_text_layout_get_type (), NULL));
	gtk_object_ref (GTK_OBJECT (smooth_text_layout));
	gtk_object_sink (GTK_OBJECT (smooth_text_layout));

	smooth_text_layout_set_text (smooth_text_layout, text, text_length);
	eel_smooth_text_layout_set_font (smooth_text_layout, font);
	eel_smooth_text_layout_set_font_size (smooth_text_layout, font_size);
	eel_smooth_text_layout_set_wrap (smooth_text_layout, wrap);
	
	return smooth_text_layout;
}

void
eel_smooth_text_layout_draw_to_pixbuf (const EelSmoothTextLayout *smooth_text_layout,
				       GdkPixbuf *destination_pixbuf,
				       int source_x,
				       int source_y,
				       ArtIRect destination_area,
				       GtkJustification justification,
				       gboolean underlined,
				       guint32 color,
				       int opacity)
{
	EelDimensions dimensions;
	ArtIRect target;
	ArtIRect source;
	int target_width;
	int target_height;
	int source_width;
	int source_height;
	GdkPixbuf *sub_pixbuf;

	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));
	g_return_if_fail (eel_gdk_pixbuf_is_valid (destination_pixbuf));
	g_return_if_fail (justification >= GTK_JUSTIFY_LEFT && justification <= GTK_JUSTIFY_FILL);
	g_return_if_fail (!art_irect_empty (&destination_area));

	smooth_text_layout_ensure_lines (smooth_text_layout);
	
	dimensions = eel_smooth_text_layout_get_dimensions (smooth_text_layout);

	g_return_if_fail (source_x >= 0);
	g_return_if_fail (source_y >= 0);
	g_return_if_fail (source_x < dimensions.width);
	g_return_if_fail (source_y < dimensions.height);

	/* Clip the destination area to the pixbuf dimensions; bail if no work */
	target = eel_gdk_pixbuf_intersect (destination_pixbuf, 0, 0, destination_area);
	if (art_irect_empty (&target)) {
 		return;
 	}

	/* Assign the source area */
	source = eel_art_irect_assign (source_x,
				       source_y,
				       dimensions.width - source_x,
				       dimensions.height - source_y);
	
	/* Adjust the target width if the source area is smaller than the
	 * source pixbuf dimensions */
	target_width = target.x1 - target.x0;
	target_height = target.y1 - target.y0;
	source_width = source.x1 - source.x0;
	source_height = source.y1 - source.y0;

	target.x1 = target.x0 + MIN (target_width, source_width);
	target.y1 = target.y0 + MIN (target_height, source_height);

	/* Use a sub area pixbuf for simplicity */
	sub_pixbuf = eel_gdk_pixbuf_new_from_pixbuf_sub_area (destination_pixbuf, target);

	smooth_text_layout_line_list_draw_to_pixbuf (smooth_text_layout->details->text_line_list,
						     sub_pixbuf,
						     -source_x,
						     -source_y,
						     justification,
						     underlined,
						     smooth_text_layout_get_empty_line_height (smooth_text_layout),
						     smooth_text_layout_get_max_line_width (smooth_text_layout),
						     smooth_text_layout->details->line_spacing,
						     color,
						     opacity);

	gdk_pixbuf_unref (sub_pixbuf);
}

void
eel_smooth_text_layout_draw_to_pixbuf_shadow (const EelSmoothTextLayout *smooth_text_layout,
					      GdkPixbuf *destination_pixbuf,
					      int source_x,
					      int source_y,
					      ArtIRect destination_area,
					      int shadow_offset,
					      GtkJustification justification,
					      gboolean underlined,
					      guint32 color,
					      guint32 shadow_color,
					      int opacity)
{
	EelDimensions dimensions;
	ArtIRect target;
	ArtIRect source;
	int target_width;
	int target_height;
	int source_width;
	int source_height;
	GdkPixbuf *sub_pixbuf;

	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));
	g_return_if_fail (eel_gdk_pixbuf_is_valid (destination_pixbuf));
	g_return_if_fail (shadow_offset > 0);
	g_return_if_fail (justification >= GTK_JUSTIFY_LEFT && justification <= GTK_JUSTIFY_FILL);
	g_return_if_fail (!art_irect_empty (&destination_area));

	smooth_text_layout_ensure_lines (smooth_text_layout);
	
	dimensions = eel_smooth_text_layout_get_dimensions (smooth_text_layout);
	dimensions.width += shadow_offset;
	dimensions.height += shadow_offset;

	g_return_if_fail (source_x >= 0);
	g_return_if_fail (source_y >= 0);
	g_return_if_fail (source_x < dimensions.width);
	g_return_if_fail (source_y < dimensions.height);

	/* Clip the destination area to the pixbuf dimensions; bail if no work */
	target = eel_gdk_pixbuf_intersect (destination_pixbuf, 0, 0, destination_area);
	if (art_irect_empty (&target)) {
 		return;
 	}

	/* Assign the source area */
	source = eel_art_irect_assign (source_x,
				       source_y,
				       dimensions.width - source_x,
				       dimensions.height - source_y);

	/* Adjust the target width if the source area is smaller than the
	 * source pixbuf dimensions */
	target_width = target.x1 - target.x0;
	target_height = target.y1 - target.y0;
	source_width = source.x1 - source.x0;
	source_height = source.y1 - source.y0;

	target.x1 = target.x0 + MIN (target_width, source_width);
	target.y1 = target.y0 + MIN (target_height, source_height);

	/* Use a sub area pixbuf for simplicity */
	sub_pixbuf = eel_gdk_pixbuf_new_from_pixbuf_sub_area (destination_pixbuf, target);

	/* Draw the shadow text */
	smooth_text_layout_line_list_draw_to_pixbuf (smooth_text_layout->details->text_line_list,
						     sub_pixbuf,
						     -source_x + shadow_offset,
						     -source_y + shadow_offset,
						     justification,
						     underlined,
						     smooth_text_layout_get_empty_line_height (smooth_text_layout),
						     smooth_text_layout_get_max_line_width (smooth_text_layout),
						     smooth_text_layout->details->line_spacing,
						     shadow_color,
						     opacity);

	/* Draw the text */
	smooth_text_layout_line_list_draw_to_pixbuf (smooth_text_layout->details->text_line_list,
						     sub_pixbuf,
						     -source_x,
						     -source_y,
						     justification,
						     underlined,
						     smooth_text_layout_get_empty_line_height (smooth_text_layout),
						     smooth_text_layout_get_max_line_width (smooth_text_layout),
						     smooth_text_layout->details->line_spacing,
						     color,
						     opacity);

	gdk_pixbuf_unref (sub_pixbuf);
}

EelDimensions
eel_smooth_text_layout_get_dimensions (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), eel_dimensions_empty);

	smooth_text_layout_ensure_lines (smooth_text_layout);

	if (smooth_text_layout->details->dimensions.width == UNDEFINED_DIMENSION) {
		const int max_line_width = smooth_text_layout_get_max_line_width (smooth_text_layout);
		const int num_lines = g_list_length (smooth_text_layout->details->text_line_list);
		const int num_empty_lines = smooth_text_layout_get_num_empty_lines (smooth_text_layout);
		const int total_line_height = smooth_text_layout_get_total_line_height (smooth_text_layout);

		g_assert (num_lines >= 0);
		g_assert (num_empty_lines >= 0);
		g_assert (num_lines >= num_empty_lines);
		
		smooth_text_layout->details->dimensions.width = max_line_width;
		smooth_text_layout->details->dimensions.height = total_line_height;
		
		if (num_lines > 1) {
			smooth_text_layout->details->dimensions.height += 
				(num_lines - 1) * smooth_text_layout->details->line_spacing;
		}
	}

	return smooth_text_layout->details->dimensions;
}

int
eel_smooth_text_layout_get_width (const EelSmoothTextLayout *smooth_text_layout)
{
	EelDimensions dimensions;

	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), 0);

	dimensions = eel_smooth_text_layout_get_dimensions (smooth_text_layout);

	return dimensions.width;
}

int
eel_smooth_text_layout_get_height (const EelSmoothTextLayout *smooth_text_layout)
{
	EelDimensions dimensions;

	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), 0);

	dimensions = eel_smooth_text_layout_get_dimensions (smooth_text_layout);

	return dimensions.height;
}

void
eel_smooth_text_layout_set_wrap (EelSmoothTextLayout *smooth_text_layout,
				 gboolean wrap)
{
	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));

	if (smooth_text_layout->details->wrap == wrap) {
		return;
	}

	smooth_text_layout_clear_lines (smooth_text_layout);
	smooth_text_layout->details->wrap = wrap;
}

gboolean
eel_smooth_text_layout_get_wrap (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), FALSE);

	return smooth_text_layout->details->wrap;
}

void
eel_smooth_text_layout_set_font (EelSmoothTextLayout *smooth_text_layout,
				 EelScalableFont *font)
{
	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));
	g_return_if_fail (EEL_IS_SCALABLE_FONT (font));

	if (smooth_text_layout->details->font == font) {
		return;
	}

	smooth_text_layout_clear_lines (smooth_text_layout);
	gtk_object_unref (GTK_OBJECT (smooth_text_layout->details->font));
	gtk_object_ref (GTK_OBJECT (font));
	smooth_text_layout->details->font = font;
}

EelScalableFont *
eel_smooth_text_layout_get_font (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), NULL);

	gtk_object_ref (GTK_OBJECT (smooth_text_layout->details->font));
	return smooth_text_layout->details->font;
}

void
eel_smooth_text_layout_set_font_size (EelSmoothTextLayout *smooth_text_layout,
				      int font_size)
{
	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));
	g_return_if_fail (font_size >= MIN_FONT_SIZE);

	if (smooth_text_layout->details->font_size == font_size) {
		return;
	}
	
	smooth_text_layout_clear_lines (smooth_text_layout);
	smooth_text_layout->details->font_size = font_size;
}

int
eel_smooth_text_layout_get_font_size (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), 0);

	return smooth_text_layout->details->font_size;
}

void
eel_smooth_text_layout_set_line_spacing (EelSmoothTextLayout *smooth_text_layout,
					 int line_spacing)
{
	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));
	
	if (smooth_text_layout->details->line_spacing == line_spacing) {
		return;
	}
	
	smooth_text_layout_clear_lines (smooth_text_layout);
	smooth_text_layout->details->line_spacing = line_spacing;
}

int
eel_smooth_text_layout_get_line_spacing (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), 0);

	return smooth_text_layout->details->line_spacing;
}

void
eel_smooth_text_layout_set_empty_line_height (EelSmoothTextLayout *smooth_text_layout,
					      int empty_line_height)
{
	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));
	
	if (smooth_text_layout->details->empty_line_height == empty_line_height) {
		return;
	}
	
	smooth_text_layout_clear_lines (smooth_text_layout);
	smooth_text_layout->details->empty_line_height = empty_line_height;
}

int
eel_smooth_text_layout_get_empty_line_height (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), 0);

	return smooth_text_layout->details->empty_line_height;
}

static void
smooth_text_layout_set_text (EelSmoothTextLayout *smooth_text_layout,
			     const char *text,
			     int text_length)
{
	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));

	if (smooth_text_layout->details->text == text
	    && smooth_text_layout->details->text_length == text_length) {
		return;
	}
	
	smooth_text_layout_clear_lines (smooth_text_layout);
	g_free (smooth_text_layout->details->text);
	smooth_text_layout->details->text = g_strdup (text);
	smooth_text_layout->details->text_length = text_length;
}

void
eel_smooth_text_layout_set_line_break_characters (EelSmoothTextLayout *smooth_text_layout,
						  const char *line_break_characters)
{
	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));
	g_return_if_fail (eel_strlen (line_break_characters) > 0);

	if (eel_str_is_equal (smooth_text_layout->details->line_break_characters, line_break_characters)) {
		return;
	}

	smooth_text_layout_clear_lines (smooth_text_layout);
	g_free (smooth_text_layout->details->line_break_characters);
	smooth_text_layout->details->line_break_characters = g_strdup (line_break_characters);
}

char *
eel_smooth_text_layout_get_line_break_characters (const EelSmoothTextLayout *smooth_text_layout)
{
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout), FALSE);

	return g_strdup (smooth_text_layout->details->text);
}

void
eel_smooth_text_layout_set_line_wrap_width (EelSmoothTextLayout *smooth_text_layout,
					    int line_wrap_width)
{
	g_return_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (smooth_text_layout));

	if (smooth_text_layout->details->line_wrap_width == line_wrap_width) {
		return;
	}

	smooth_text_layout_clear_lines (smooth_text_layout);
	smooth_text_layout->details->line_wrap_width = line_wrap_width;
}

/*
 * The following text_layout stuff was shamelessly plundered
 * from libgnomeui/gnome-icon-text.[ch] by Federico Mena.
 *
 * It was hacked to use EelScalableFont and GdkPixbuf
 * instead of GdkFont and GdkDrawable.  We want to use the
 * same layout algorithm in Eel so that both the smooth
 * and not smooth text rendering cases have predictably 
 * similar result.
 *
 * I also made some minor Eel-like style changes. -re
 *
 */
static void
text_layout_free_row (gpointer data,
		      gpointer user_data)
{
	EelTextLayoutRow *row;

	if (data) {
		row = data;
		g_free (row->text);
		g_free (row);
	}
}

/**
 * eel_text_layout_free:
 * @ti: An icon text info structure.
 *
 * Frees a &EelTextLayout structure.  You should call this instead of
 * freeing the structure yourself.
 */
void
eel_text_layout_free (EelTextLayout *text_layout)
{
	g_list_foreach (text_layout->rows, text_layout_free_row, NULL);
	g_list_free (text_layout->rows);
	g_free (text_layout);
}

/**
 * eel_text_layout_new:
 * @font:       Name of the font that will be used to render the text.
 * @text:       Text to be formatted.
 * @separators: Separators used for word wrapping, can be NULL.
 * @max_width:  Width in pixels to be used for word wrapping.
 * @confine:    Whether it is mandatory to wrap at @max_width.
 *
 * Creates a new &EelTextLayout structure by wrapping the specified
 * text.  If non-NULL, the @separators argument defines a set of characters
 * to be used as word delimiters for performing word wrapping.  If it is
 * NULL, then only spaces will be used as word delimiters.
 *
 * The @max_width argument is used to specify the width at which word
 * wrapping will be performed.  If there is a very long word that does not
 * fit in a single line, the @confine argument can be used to specify
 * whether the word should be unconditionally split to fit or whether
 * the maximum width should be increased as necessary.
 *
 * Return value: A newly-created &EelTextLayout structure.
 */
static int
wcs_scalable_font_text_width (const EelScalableFont *font,
			      int font_size,
			      GdkWChar *wc_text)
{
	gchar *mb_text;
	int mb_text_len, width;

	mb_text = gdk_wcstombs (wc_text);
	mb_text_len = strlen (mb_text);

	width = eel_scalable_font_text_width (font, font_size, mb_text, mb_text_len);
	g_free (mb_text);
	return width;
}

EelTextLayout *
eel_text_layout_new (const EelScalableFont *font,
		     int font_size,
		     const char *text,
		     const char *separators,
		     int max_width,
		     gboolean confine)
{
	EelTextLayout *text_layout;
	EelTextLayoutRow *row;

	char *sub_text;
	int i;

	GdkWChar *wc_text, *wc_separators;
	GdkWChar *wc_text_iter; 
	GdkWChar *wc_row_end, *wc_word_start;
	GdkWChar *wc_word_end, *wc_old_word_end;
	GdkWChar *wc_s;
	GdkWChar wc_char;

	int wc_text_len, wc_separators_len, wc_word_len;
	int word_width;

	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (font_size > 0, NULL);
	g_return_val_if_fail (text != NULL, NULL);
	g_return_val_if_fail (eel_strlen (text) > 0, NULL);

	wc_text_len = strlen (text) + 1;
	wc_text = g_new0 (GdkWChar, wc_text_len);
	wc_text_len = gdk_mbstowcs (wc_text, text, wc_text_len);

	if (wc_text_len <= 0) {
		g_free (wc_text);
		return NULL;
	}

	if (!separators)
		separators = " ";

	wc_separators_len = strlen (separators) + 1;
	wc_separators = g_new0 (GdkWChar, wc_separators_len);
	wc_separators_len = gdk_mbstowcs (wc_separators, separators, wc_separators_len);

	if (wc_separators_len <= 0) {
		g_free (wc_text);
		g_free (wc_separators);
		return NULL;
	}

	text_layout = g_new0 (EelTextLayout, 1);

	text_layout->rows = NULL;
	text_layout->font = font;
	text_layout->font_size = font_size;
	text_layout->width = 0;
	text_layout->height = 0;
	text_layout->baseline_skip = font_size;

	wc_word_end = NULL;

	wc_text_iter = wc_text;
	while (*wc_text_iter != L'\0') {
		for (wc_row_end = wc_text_iter; *wc_row_end != L'\0' && *wc_row_end != L'\n'; wc_row_end++);

		/* Accumulate words from this row until they don't fit in the max_width */

		wc_s = wc_text_iter;

		while (wc_s < wc_row_end) {
			wc_word_start = wc_s;
			wc_old_word_end = wc_word_end;
			for (wc_word_end = wc_word_start; *wc_word_end != L'\0'; wc_word_end++) {
				const GdkWChar *wc_p;
				for (wc_p = wc_separators; *wc_p != L'\0'; wc_p++) {
					if (*wc_word_end == *wc_p)
						goto found;
				}
			}
		found:
			if (wc_word_end < wc_row_end)
				wc_word_end++;

			wc_char = *wc_word_end;
			*wc_word_end = L'\0';
			word_width = wcs_scalable_font_text_width (font, font_size, wc_text_iter);
			*wc_word_end = wc_char;

			if ( word_width > max_width) {
				if (wc_word_start == wc_text_iter) {
					if (confine) {
						/* We must force-split the word.  Look for a proper
                                                 * place to do it.
						 */

						wc_word_len = wc_word_end - wc_word_start;
						
						for (i = 1; i < wc_word_len; i++) {

							wc_char = *(wc_word_start+i);
							*(wc_word_start+i) = L'\0';
							word_width = wcs_scalable_font_text_width (font, font_size, wc_word_start);
							*(wc_word_start+i) = wc_char;

							if (word_width > max_width) {
								if (i == 1)
									/* Shit, not even a single character fits */
									max_width = word_width;
								else
									break;
							}
						}

						/* Create sub-row with the chars that fit */
						wc_char = *(wc_word_start+i-1);
						*(wc_word_start+i-1) = L'\0';
						sub_text = gdk_wcstombs (wc_word_start);
						*(wc_word_start+i-1) = wc_char;
						
						row = g_new0 (EelTextLayoutRow, 1);
						row->text = sub_text;
						row->text_length = strlen (sub_text);
						row->width = eel_scalable_font_text_width (font, font_size, 
											   sub_text, 
											   row->text_length);

						text_layout->rows = g_list_append (text_layout->rows, row);

						if (row->width > text_layout->width)
							text_layout->width = row->width;

						text_layout->height += text_layout->baseline_skip;

						/* Bump the text pointer */

						wc_text_iter += i - 1;
						wc_s = wc_text_iter;

						continue;
					} else {
						wc_char = *wc_word_end;
						*wc_word_end = L'\0';
						max_width = wcs_scalable_font_text_width (font, font_size, wc_text_iter);
						*wc_word_end = wc_char;
					}
					continue; /* Retry split */
				} else {
					wc_word_end = wc_old_word_end; /* Restore to region that does fit */
					break; /* Stop the loop because we found something that doesn't fit */
				}
			}

			wc_s = wc_word_end;
		}

		/* Append row */

		if (wc_text_iter == wc_row_end) {
			/* We are on a newline, so append an empty row */

			text_layout->rows = g_list_append (text_layout->rows, NULL);
			text_layout->height += text_layout->baseline_skip / 2;

			/* Next! */

			wc_text_iter = wc_row_end + 1;
		} else {
			/* Create subrow and append it to the list */
			wc_char = *wc_word_end;
			*wc_word_end = L'\0';
			sub_text = gdk_wcstombs (wc_text_iter);
			*wc_word_end = wc_char;

			row = g_new0 (EelTextLayoutRow, 1);
			row->text = sub_text;
			row->text_length = strlen (sub_text);
			row->width = eel_scalable_font_text_width (font, font_size, sub_text, row->text_length);

			text_layout->rows = g_list_append (text_layout->rows, row);

			if (row->width > text_layout->width)
				text_layout->width = row->width;

			text_layout->height += text_layout->baseline_skip;

			/* Next! */

			wc_text_iter = wc_word_end;
		}
	}
	g_free (wc_text);
	g_free (wc_separators);
	return text_layout;
}

#if !defined (EEL_OMIT_SELF_CHECK)

gboolean
eel_smooth_text_layout_compare (EelSmoothTextLayout *x,
				     EelSmoothTextLayout *y)
{
	GList *xp, *yp;

	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (x), FALSE);
	g_return_val_if_fail (EEL_IS_SMOOTH_TEXT_LAYOUT (y), FALSE);

	/* Compare simple things */
	if (x->details->dimensions.width != y->details->dimensions.width
	    || x->details->dimensions.height != y->details->dimensions.height
	    || x->details->text_length != y->details->text_length
	    || memcmp (x->details->text, y->details->text, x->details->text_length) != 0
	    || x->details->font != y->details->font
	    || x->details->font_size != y->details->font_size
	    || x->details->line_spacing != y->details->line_spacing
	    || x->details->empty_line_height != y->details->empty_line_height
	    || x->details->max_line_width != y->details->max_line_width
	    || x->details->num_empty_lines != y->details->num_empty_lines
	    || x->details->line_wrap_width != y->details->line_wrap_width
	    || x->details->total_line_height != y->details->total_line_height
	    || x->details->wrap != y->details->wrap
	    || strcmp (x->details->line_break_characters, y->details->line_break_characters) != 0) {
		return FALSE;
	}

	/* Compare glyphs */
	smooth_text_layout_ensure_lines (x);
	smooth_text_layout_ensure_lines (y);
	for (xp = x->details->text_line_list, yp = y->details->text_line_list;
	     xp != NULL && yp != NULL;
	     xp = xp->next, yp = yp->next) {
		if ((xp->data == NULL) != (yp->data == NULL)) {
			return FALSE;
		}
		if (xp->data != NULL
		    && !eel_glyph_compare (xp->data, yp->data)) {
			return FALSE;
		}
	}
	if (xp != NULL || yp != NULL) {
		return FALSE;
	}

	return TRUE;
}

#endif /* EEL_OMIT_SELF_CHECK */
