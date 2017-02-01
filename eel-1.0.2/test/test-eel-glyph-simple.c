#include "test.h"

static EelGlyph *
glyph_new (const char *text, int font_size, gboolean bold)
{
	EelScalableFont *font;
	EelScalableFont *bold_font;
	EelGlyph *glyph;
	
	g_return_val_if_fail (text != NULL, NULL);
	g_return_val_if_fail (text[0] != '\0', NULL);
	g_return_val_if_fail (font_size >= 5, NULL);
	g_return_val_if_fail (font_size <= 200, NULL);
	
	font = eel_scalable_font_get_default_font ();
	g_return_val_if_fail (font != NULL, NULL);
	
	if (bold) {
		bold_font = eel_scalable_font_make_bold (font);
		g_return_val_if_fail (bold_font != NULL, NULL);
		gtk_object_unref (GTK_OBJECT (font));
		font = bold_font;
	}

	glyph = eel_glyph_new (font, font_size, text, strlen (text));
	g_return_val_if_fail (glyph != NULL, NULL);

	gtk_object_unref (GTK_OBJECT (font));

	return glyph;
}

int 
main (int argc, char* argv[])
{
	GdkPixbuf *pixbuf;
	EelScalableFont *font;
	EelGlyph *glyph;
	int x;
	int y;
	const guint font_size = 60;
	const guint underlined_font_size = 100;
	const int opacity = EEL_OPACITY_FULLY_OPAQUE;

	const guint pixbuf_width = 640;
	const guint pixbuf_height = 480;
	const gboolean has_alpha = FALSE;
	const guint32 background_color = EEL_RGB_COLOR_WHITE;
	const char text[] = "Somethin";

	test_init (&argc, &argv);

	font = eel_scalable_font_get_default_font ();
	g_assert (font != NULL);

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, has_alpha, 8, pixbuf_width, pixbuf_height);
	g_assert (pixbuf != NULL);

	eel_debug_pixbuf_draw_rectangle (pixbuf,
					 TRUE,
					 -1, -1, -1, -1,
					 background_color,
					 EEL_OPACITY_FULLY_OPAQUE);
	
	glyph = glyph_new (text, font_size, FALSE);

	x = 50;
	y = 10;

	eel_glyph_draw_to_pixbuf (glyph,
				  pixbuf,
				  x,
				  y,
				  eel_gdk_pixbuf_whole_pixbuf,
				  EEL_RGBA_COLOR_OPAQUE_BLUE,
				  opacity);
	y += eel_glyph_get_height (glyph) + 10;
	eel_glyph_free (glyph);

	glyph = glyph_new (text, font_size, TRUE);
	eel_glyph_draw_to_pixbuf (glyph,
				  pixbuf,
				  x,
				  y,
				  eel_gdk_pixbuf_whole_pixbuf,
				  EEL_RGBA_COLOR_OPAQUE_BLUE,
				  opacity);
	y += eel_glyph_get_height (glyph) + 10;
	eel_glyph_free (glyph);


	glyph = glyph_new (text, underlined_font_size, FALSE);
	eel_glyph_draw_to_pixbuf (glyph,
				  pixbuf,
				  x,
				  y,
				  eel_gdk_pixbuf_whole_pixbuf,
				  EEL_RGBA_COLOR_OPAQUE_BLUE,
				  opacity);

	{
		EelScalableFont *font;
		ArtIRect glyph_rect;
		ArtIRect underline_rect;

		font = eel_scalable_font_get_default_font ();
		glyph_rect = eel_glyph_intersect (glyph, x, y, eel_gdk_pixbuf_whole_pixbuf);

		if (0) eel_debug_pixbuf_draw_rectangle (pixbuf,
							FALSE,
							glyph_rect.x0,
							glyph_rect.y0,
							glyph_rect.x1,
							glyph_rect.y1,
							0xFF0000,
							EEL_OPACITY_FULLY_OPAQUE);
		
		underline_rect = eel_glyph_get_underline_rectangle (glyph);
		
		eel_debug_pixbuf_draw_rectangle (pixbuf,
						 TRUE,
						 underline_rect.x0,
						 underline_rect.y0,
						 underline_rect.x1,
						 underline_rect.y1,
						 EEL_RGBA_COLOR_OPAQUE_BLUE,
						 EEL_OPACITY_FULLY_OPAQUE);
	}		
	eel_glyph_free (glyph);

	eel_debug_show_pixbuf_in_external_viewer (pixbuf, "ee");
	
	gdk_pixbuf_unref (pixbuf);
	gtk_object_unref (GTK_OBJECT (font));

	return test_quit (EXIT_SUCCESS);
}
