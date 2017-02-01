/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-scalable-font.c - A GtkObject subclass for access to scalable fonts.

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
#include "eel-scalable-font-private.h"

#include "eel-background.h"
#include "eel-font-manager.h"
#include "eel-gdk-pixbuf-extensions.h"
#include "eel-glib-extensions.h"
#include "eel-glyph.h"
#include "eel-gtk-macros.h"
#include "eel-lib-self-check-functions.h"
#include "eel-string.h"
#include <librsvg/rsvg-ft.h>
#include <libgnome/gnome-util.h>

/* Detail member struct */
struct EelScalableFontDetails
{
	RsvgFTFontHandle font_handle;
	char *font_file_name;
};

/* Global things */
static RsvgFTCtx		*global_rsvg_ft_context = NULL;
static GHashTable		*global_font_handle_table = NULL;

/* GtkObjectClass methods */
static void eel_scalable_font_initialize_class (EelScalableFontClass *font_class);
static void eel_scalable_font_initialize       (EelScalableFont      *font);
static void eel_scalable_font_destroy          (GtkObject            *object);
static void initialize_global_stuff_if_needed  (void);

EEL_DEFINE_CLASS_BOILERPLATE (EelScalableFont, eel_scalable_font, GTK_TYPE_OBJECT)

/* Class init methods */
static void
eel_scalable_font_initialize_class (EelScalableFontClass *font_class)
{
	GtkObjectClass *object_class = GTK_OBJECT_CLASS (font_class);

	initialize_global_stuff_if_needed ();

	/* GtkObjectClass */
	object_class->destroy = eel_scalable_font_destroy;
}

void
eel_scalable_font_initialize (EelScalableFont *font)
{
	font->details = g_new0 (EelScalableFontDetails, 1);

	font->details->font_handle = EEL_SCALABLE_FONT_UNDEFINED_HANDLE;
}

/* GtkObjectClass methods */
static void
eel_scalable_font_destroy (GtkObject *object)
{
 	EelScalableFont *font;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (EEL_IS_SCALABLE_FONT (object));

	font = EEL_SCALABLE_FONT (object);

	g_free (font->details->font_file_name);
	g_free (font->details);
	
	/* Chain destroy */
	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

/* Public EelScalableFont methods */

/**
 * eel_scalable_font_new:
 * @file_name: Postscript or TrueType font file name.
 *
 * Returns a font for the given font file name.
 *
 */
EelScalableFont *
eel_scalable_font_new (const char *file_name)
{
	EelScalableFont *font;
	gpointer font_handle_as_pointer;
	RsvgFTFontHandle font_handle = -1;

	g_return_val_if_fail (eel_strlen (file_name) > 0, NULL);

	initialize_global_stuff_if_needed ();

	font = EEL_SCALABLE_FONT (gtk_object_new (eel_scalable_font_get_type (), NULL));
	gtk_object_ref (GTK_OBJECT (font));
	gtk_object_sink (GTK_OBJECT (font));
	
	font->details->font_file_name = g_strdup (file_name);

	if (g_hash_table_lookup_extended (global_font_handle_table,
					  font->details->font_file_name,
					  NULL,
					  &font_handle_as_pointer)) {
		font_handle = GPOINTER_TO_INT (font_handle_as_pointer);
	} else {
		font_handle = rsvg_ft_intern (global_rsvg_ft_context, font->details->font_file_name);
		g_hash_table_insert (global_font_handle_table,
				     g_strdup (font->details->font_file_name),
				     GINT_TO_POINTER (font_handle));
	}
	g_assert (font_handle >= 0);

	font->details->font_handle = font_handle;

	return font;
}

EelScalableFont *
eel_scalable_font_make_bold (EelScalableFont *font)
{
	char *bold_font_file_name;
	EelScalableFont *bold_font;

	g_return_val_if_fail (EEL_IS_SCALABLE_FONT (font), NULL);

	bold_font_file_name = eel_font_manager_get_bold (font->details->font_file_name);
	bold_font = eel_scalable_font_new (bold_font_file_name);
	g_free (bold_font_file_name);

	return bold_font;
}

EelDimensions
eel_scalable_font_measure_text (const EelScalableFont *font,
				int font_size,
				const char *text,
				guint text_length)
{
	EelDimensions dimensions;
	EelGlyph *glyph;

	g_return_val_if_fail (EEL_IS_SCALABLE_FONT (font), eel_dimensions_empty);
	g_return_val_if_fail (font_size > 0, eel_dimensions_empty);

	if (text == NULL || text[0] == '\0' || text_length == 0) {
		return eel_dimensions_empty;
	}

	g_return_val_if_fail (text_length <= strlen (text), eel_dimensions_empty);

	glyph = eel_glyph_new (font, font_size, text, text_length);
	dimensions = eel_glyph_get_dimensions (glyph);
	eel_glyph_free (glyph);

	return dimensions;
}

int
eel_scalable_font_text_width (const EelScalableFont *font,
			      int font_size,
			      const char *text,
			      guint text_length)
{
	EelDimensions dimensions;

	g_return_val_if_fail (EEL_IS_SCALABLE_FONT (font), 0);
	g_return_val_if_fail (font_size > 0, 0);

	if (text == NULL || text[0] == '\0' || text_length == 0) {
		return 0;
	}

	g_return_val_if_fail (text_length <= strlen (text), 0);

	dimensions = eel_scalable_font_measure_text (font, font_size, text, text_length);

	return dimensions.width;
}

void
eel_scalable_font_draw_text (const EelScalableFont *font,
			     GdkPixbuf *destination_pixbuf,
			     int x,
			     int y,
			     ArtIRect clip_area,
			     int font_size,
			     const char *text,
			     guint text_length,
			     guint32 color,
			     int opacity)
{
	EelGlyph *glyph;

	g_return_if_fail (EEL_IS_SCALABLE_FONT (font));
	g_return_if_fail (destination_pixbuf != NULL);
	g_return_if_fail (font_size > 0);
	g_return_if_fail (opacity >= EEL_OPACITY_FULLY_TRANSPARENT);
	g_return_if_fail (opacity <= EEL_OPACITY_FULLY_OPAQUE);

	if (text == NULL || text[0] == '\0' || text_length == 0) {
		return;
	}

	g_return_if_fail (text_length <= strlen (text));

	glyph = eel_glyph_new (font, font_size, text, text_length);

	eel_glyph_draw_to_pixbuf (glyph,
				  destination_pixbuf,
				  x,
				  y,
				  clip_area,
				  color,
				  opacity);

	eel_glyph_free (glyph);
}

/**
 * eel_scalable_font_largest_fitting_font_size
 * @font: A EelScalableFont
 * @text: Text to use for measurement.
 * @available_width: How much space is available in pixels.
 * @minimum_acceptable_font_size: The minimum acceptable font size in pixels.
 * @maximum_acceptable_font_size: The maximum acceptable font size in pixels.
 *
 * Returns: A font size than when used to render &text, will fit it all in 
 *          &available_width.  The minimum and maximum acceptable dimensions
 *          control the limits on the size of the font.  The font size is
 *          guranteed to be within this range.
 */
int
eel_scalable_font_largest_fitting_font_size (const EelScalableFont *font,
					     const char *text,
					     int available_width,
					     int minimum_acceptable_font_size,
					     int maximum_acceptable_font_size)
{
	EelStringList *tokenized_string;
	int i;
	char *longest_string;
	int longest_string_length;

	g_return_val_if_fail (EEL_IS_SCALABLE_FONT (font), 0);
	g_return_val_if_fail (text != NULL, 0);
	g_return_val_if_fail (text[0] != '\0', 0);
	g_return_val_if_fail (available_width > 0, 0);
	g_return_val_if_fail (minimum_acceptable_font_size > 0, 0);
	g_return_val_if_fail (maximum_acceptable_font_size > 0, 0);
	g_return_val_if_fail (maximum_acceptable_font_size > minimum_acceptable_font_size, 0);

	tokenized_string = eel_string_list_new_from_tokens (text, "\n", FALSE);
	longest_string = eel_string_list_get_longest_string (tokenized_string);
	g_assert (longest_string != NULL);
	eel_string_list_free (tokenized_string);
	longest_string_length = strlen (longest_string);
	
	for (i = maximum_acceptable_font_size; i >= minimum_acceptable_font_size; i--) {
		int width;

		width = eel_scalable_font_text_width (font,
						      i,
						      longest_string,
						      longest_string_length);
		
		if (width <= available_width) {
			g_free (longest_string);
			return i;
		}
	}

	g_free (longest_string);

	return minimum_acceptable_font_size;
}

EelScalableFont *
eel_scalable_font_get_default_font (void)
{
	char *default_font_file_name;
	static EelScalableFont *default_font = NULL;

	if (default_font == NULL) {
		default_font_file_name = eel_font_manager_get_default_font ();
		g_assert (default_font_file_name != NULL);
		default_font = eel_scalable_font_new (default_font_file_name);
		g_free (default_font_file_name);
		g_assert (EEL_IS_SCALABLE_FONT (default_font));
	}

	gtk_object_ref (GTK_OBJECT (default_font));
	return default_font;
}

EelScalableFont *
eel_scalable_font_get_default_bold_font (void)
{
	char *default_bold_font_file_name;
	static EelScalableFont *default_bold_font = NULL;

	if (default_bold_font == NULL) {
		default_bold_font_file_name = eel_font_manager_get_default_bold_font ();
		g_assert (default_bold_font_file_name != NULL);
		default_bold_font = eel_scalable_font_new (default_bold_font_file_name);
		g_free (default_bold_font_file_name);
		g_assert (EEL_IS_SCALABLE_FONT (default_bold_font));
	}

	gtk_object_ref (GTK_OBJECT (default_bold_font));
	return default_bold_font;
}

/* 'atexit' destructor for rsvg context */
static void
destroy_global_rsvg_ft_context (void)
{
	rsvg_ft_ctx_done (global_rsvg_ft_context);
}

static void
free_global_font_handle_table (void)
{
	if (global_font_handle_table != NULL) {
		eel_g_hash_table_destroy_deep_custom
			(global_font_handle_table,
			 (GFunc) g_free, NULL, NULL, NULL);
	}

	global_font_handle_table = NULL;
}

static void
initialize_global_stuff_if_needed (void)
{
	/* Initialize the rsvg font context shared by all fonts */
	if (global_rsvg_ft_context == NULL) {
		global_rsvg_ft_context = rsvg_ft_ctx_new ();
		g_atexit (destroy_global_rsvg_ft_context);
	}

	if (global_font_handle_table == NULL) {
		global_font_handle_table = g_hash_table_new (g_str_hash, g_str_equal);
		g_atexit (free_global_font_handle_table);
	}
}

/* Private EelScalableFont things */
int
eel_scalable_font_get_rsvg_handle (const EelScalableFont *font)
{
	g_return_val_if_fail (EEL_IS_SCALABLE_FONT (font), EEL_SCALABLE_FONT_UNDEFINED_HANDLE);

	return font->details->font_handle;
}

struct _RsvgFTCtx *
eel_scalable_font_get_rsvg_context (const EelScalableFont *font)
{
	g_return_val_if_fail (EEL_IS_SCALABLE_FONT (font), NULL);

	initialize_global_stuff_if_needed ();

	return global_rsvg_ft_context;
}

#if !defined (EEL_OMIT_SELF_CHECK)

void
eel_self_check_scalable_font (void)
{
	/* const char *fonts_place = EEL_DATADIR "/fonts/urw"; */
}

#endif /* !EEL_OMIT_SELF_CHECK */
