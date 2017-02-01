#include "test.h"

static EelGlyph *
glyph_new (const char *text,
	   int font_size)
{
	EelScalableFont *font;
	EelGlyph *glyph;
	
	g_return_val_if_fail (text != NULL, NULL);
	g_return_val_if_fail (text[0] != '\0', NULL);
	g_return_val_if_fail (font_size >= 5, NULL);
	g_return_val_if_fail (font_size <= 200, NULL);
	
	font = eel_scalable_font_get_default_font ();
	g_return_val_if_fail (font != NULL, NULL);
	
	glyph = eel_glyph_new (font, font_size, text, strlen (text));
	gtk_object_unref (GTK_OBJECT (font));

	g_return_val_if_fail (glyph != NULL, NULL);

	return glyph;
}

int 
main (int argc, char* argv[])
{
	const guint pixbuf_width = 640;
	const guint pixbuf_height = 480;
	const gboolean has_alpha = FALSE;
	const guint32 background_color = EEL_RGB_COLOR_WHITE;
	const char text[] = "ù È É Ê Ë Ì Í Î Ï Ð Ñ Ò Ó Ô Õ Ö Ø Ù Ú Û Ü Ý Þ";
	const gboolean solid_background = FALSE;
	const guint font_size = 100;
	const int opacity = EEL_OPACITY_FULLY_OPAQUE;

	GdkPixbuf *pixbuf;
	EelScalableFont *font;
	int x;
	int y;
	ArtIRect clip_area;
	EelGlyph *glyph;

	test_init (&argc, &argv);

	font = eel_scalable_font_get_default_font ();
	g_assert (font != NULL);

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, has_alpha, 8, pixbuf_width, pixbuf_height);
	g_assert (pixbuf != NULL);

	if (solid_background) {
		eel_debug_pixbuf_draw_rectangle (pixbuf,
						 TRUE,
						 -1, -1, -1, -1,
						 background_color,
						 EEL_OPACITY_FULLY_OPAQUE);
	} else {
		test_pixbuf_draw_rectangle_tiled (pixbuf, 
						  "patterns/brushed_metal.png",
						  -1, -1, -1, -1,
						  EEL_OPACITY_FULLY_OPAQUE);
	}
	
	x = 10;
	y = 50;
	
	clip_area = eel_art_irect_assign (50, 30, 200, 80);
	
	glyph = glyph_new (text, font_size);
	
	eel_glyph_draw_to_pixbuf (glyph,
				  pixbuf,
				  x,
				  y,
				  eel_gdk_pixbuf_whole_pixbuf,
				  EEL_RGBA_COLOR_OPAQUE_BLUE,
				  opacity);
	
	eel_glyph_draw_to_pixbuf (glyph,
				  pixbuf,
				  -200,
				  y + 3 * font_size,
				  eel_gdk_pixbuf_whole_pixbuf,
				  EEL_RGBA_COLOR_OPAQUE_BLUE,
				  opacity);
	
	if (solid_background) {
		eel_debug_pixbuf_draw_rectangle (pixbuf,
						 TRUE,
						 clip_area.x0,
						 clip_area.y0,
						 clip_area.x1,
						 clip_area.y1,
						 background_color,
						 EEL_OPACITY_FULLY_OPAQUE);
	} else {
		test_pixbuf_draw_rectangle_tiled (pixbuf, 
						  "patterns/brushed_metal.png",
						  clip_area.x0,
						  clip_area.y0,
						  clip_area.x1,
						  clip_area.y1,
						  EEL_OPACITY_FULLY_OPAQUE);
	}
	
	if (1) eel_debug_pixbuf_draw_rectangle (pixbuf,
						FALSE,
						clip_area.x0 - 1,
						clip_area.y0 - 1,
						clip_area.x1 + 1,
						clip_area.y1 + 1,
						EEL_RGBA_COLOR_OPAQUE_GREEN,
						EEL_OPACITY_FULLY_OPAQUE);
	eel_glyph_draw_to_pixbuf (glyph,
				  pixbuf,
				  x,
				  y,
				  clip_area,
				  EEL_RGBA_COLOR_OPAQUE_RED,
				  opacity);
	
	eel_glyph_free (glyph);
	
	clip_area = eel_art_irect_assign (50,
					  100 + font_size + 4,
					  200,
					  80);
	
	if (1) eel_scalable_font_draw_text (font,
					    pixbuf,
					    x,
					    y + font_size + 4,
					    eel_gdk_pixbuf_whole_pixbuf,
					    font_size,
					    text,
					    strlen (text),
					    EEL_RGBA_COLOR_OPAQUE_BLUE,
					    opacity);
	
	if (solid_background) {
		eel_debug_pixbuf_draw_rectangle (pixbuf,
						 TRUE,
						 clip_area.x0,
						 clip_area.y0,
						 clip_area.x1,
						 clip_area.y1,
						 background_color,
						 EEL_OPACITY_FULLY_OPAQUE);
	} else {
		test_pixbuf_draw_rectangle_tiled (pixbuf, 
						  "patterns/brushed_metal.png",
						  clip_area.x0,
						  clip_area.y0,
						  clip_area.x1,
						  clip_area.y1,
						  EEL_OPACITY_FULLY_OPAQUE);
	}
	
	if (1) eel_debug_pixbuf_draw_rectangle (pixbuf,
						FALSE,
						clip_area.x0 - 1,
						clip_area.y0 - 1,
						clip_area.x1 + 1,
						clip_area.y1 + 1,
						EEL_RGBA_COLOR_OPAQUE_GREEN,
						EEL_OPACITY_FULLY_OPAQUE);
	if (1) eel_scalable_font_draw_text (font,
					    pixbuf,
					    x,
					    y + font_size + 4,
					    clip_area,
					    font_size,
					    text,
					    strlen (text),
					    EEL_RGBA_COLOR_OPAQUE_RED,
					    opacity);

	{
		const int glyph_x = 400;
		const int glyph_y = 300;
		ArtIRect glyph_rect;
		glyph = glyph_new ("x", 50);
		
		glyph_rect = eel_glyph_intersect (glyph, glyph_x, glyph_y, eel_gdk_pixbuf_whole_pixbuf);

		eel_debug_pixbuf_draw_rectangle_inset (pixbuf,
						       FALSE,
						       glyph_rect.x0,
						       glyph_rect.y0,
						       glyph_rect.x1,
						       glyph_rect.y1,
						       0xeebbaa,
						       0xff,
						       -1);
		eel_glyph_draw_to_pixbuf (glyph,
					  pixbuf,
					  glyph_x,
					  glyph_y,
					  eel_gdk_pixbuf_whole_pixbuf,
					  0x0,
					  0xff);

		eel_glyph_free (glyph);
	}

	{
		const int glyph_x = 400;
		const int glyph_y = 350;
		ArtIRect glyph_rect;
		glyph = glyph_new ("x   y", 50);
		
		glyph_rect = eel_glyph_intersect (glyph, glyph_x, glyph_y, eel_gdk_pixbuf_whole_pixbuf);

		eel_debug_pixbuf_draw_rectangle_inset (pixbuf,
						       FALSE,
						       glyph_rect.x0,
						       glyph_rect.y0,
						       glyph_rect.x1,
						       glyph_rect.y1,
						       0xeebbaa,
						       0xff,
						       -1);
		eel_glyph_draw_to_pixbuf (glyph,
					  pixbuf,
					  glyph_x,
					  glyph_y,
					  eel_gdk_pixbuf_whole_pixbuf,
					  0x0,
					  0xff);

		eel_glyph_free (glyph);
	}

	{
		const int glyph_x = 400;
		const int glyph_y = 400;
		ArtIRect glyph_rect;
		glyph = glyph_new (" ", 50);
		
		glyph_rect = eel_glyph_intersect (glyph, glyph_x, glyph_y, eel_gdk_pixbuf_whole_pixbuf);

		eel_debug_pixbuf_draw_rectangle_inset (pixbuf,
						       FALSE,
						       glyph_rect.x0,
						       glyph_rect.y0,
						       glyph_rect.x1,
						       glyph_rect.y1,
						       0xeebbaa,
						       0xff,
						       -1);
		eel_glyph_draw_to_pixbuf (glyph,
					  pixbuf,
					  glyph_x,
					  glyph_y,
					  eel_gdk_pixbuf_whole_pixbuf,
					  0x0,
					  0xff);

		eel_glyph_free (glyph);
	}

	{
		const int glyph_x = 400;
		const int glyph_y = 420;
		ArtIRect glyph_rect;
		glyph = glyph_new ("  ", 50);
		
		glyph_rect = eel_glyph_intersect (glyph, glyph_x, glyph_y, eel_gdk_pixbuf_whole_pixbuf);

		eel_debug_pixbuf_draw_rectangle_inset (pixbuf,
						       FALSE,
						       glyph_rect.x0,
						       glyph_rect.y0,
						       glyph_rect.x1,
						       glyph_rect.y1,
						       0xeebbaa,
						       0xff,
						       -1);
		eel_glyph_draw_to_pixbuf (glyph,
					  pixbuf,
					  glyph_x,
					  glyph_y,
					  eel_gdk_pixbuf_whole_pixbuf,
					  0x0,
					  0xff);

		eel_glyph_free (glyph);
	}


	/* This should not work.  A "" glyph is invalid */
	if (0) {
		const int glyph_x = 400;
		const int glyph_y = 450;
		ArtIRect glyph_rect;
		glyph = glyph_new ("", 50);
		
		glyph_rect = eel_glyph_intersect (glyph, glyph_x, glyph_y, eel_gdk_pixbuf_whole_pixbuf);

		eel_debug_pixbuf_draw_rectangle_inset (pixbuf,
						       FALSE,
						       glyph_rect.x0,
						       glyph_rect.y0,
						       glyph_rect.x1,
						       glyph_rect.y1,
						       0xeebbaa,
						       0xff,
						       -1);
		eel_glyph_draw_to_pixbuf (glyph,
					  pixbuf,
					  glyph_x,
					  glyph_y,
					  eel_gdk_pixbuf_whole_pixbuf,
					  0x0,
					  0xff);

		eel_glyph_free (glyph);
	}
	
	eel_debug_show_pixbuf_in_external_viewer (pixbuf, "ee");
	
	gdk_pixbuf_unref (pixbuf);

	gtk_object_unref (GTK_OBJECT (font));

	test_quit (0);

	return 0;
}
