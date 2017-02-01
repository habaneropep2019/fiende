#include "test.h"

int 
main (int argc, char* argv[])
{
	const guint pixbuf_width = 640;
	const guint pixbuf_height = 480;
	const gboolean has_alpha = TRUE;
	const char *text = NULL;
	const int font_size = 25;

	GdkPixbuf *pixbuf;
	EelScalableFont *font;
	EelScalableFont *bold_font;
	EelSmoothTextLayout *smooth_text_layout;
	ArtIRect dest;


	test_init (&argc, &argv);

	font = eel_scalable_font_get_default_font ();
	g_assert (font != NULL);

	bold_font = eel_scalable_font_make_bold (font);
	g_assert (bold_font != NULL);

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, has_alpha, 8, pixbuf_width, pixbuf_height);
	g_assert (pixbuf != NULL);

	test_pixbuf_draw_rectangle_tiled (pixbuf, 
					  "patterns/brushed_metal.png",
					  -1, -1, -1, -1,
					  EEL_OPACITY_FULLY_OPAQUE);
	
	{
		const int border = 50;
		dest = eel_gdk_pixbuf_intersect (pixbuf, 0, 0, eel_gdk_pixbuf_whole_pixbuf);
		dest.x0 += border;
		dest.y0 += border;
		dest.x1 = 340;
		dest.y1 -= border;
	}
	eel_debug_pixbuf_draw_rectangle_inset (pixbuf,
					       FALSE,
					       dest.x0,
					       dest.y0,
					       dest.x1,
					       dest.y1,
					       0x00FFFF,
					       0xFF,
					       -1);
	
	text = "This\nis\nmulti\nline\ntext";
	smooth_text_layout = eel_smooth_text_layout_new (text,
							 strlen (text),
							 font,
							 font_size,
							 FALSE);
	eel_smooth_text_layout_draw_to_pixbuf (smooth_text_layout,
					       pixbuf,
					       0,
					       0,
					       dest,
					       GTK_JUSTIFY_LEFT,
					       FALSE,
					       0xFF0000,
					       0xff);
	gtk_object_unref (GTK_OBJECT (smooth_text_layout));

	{
		const int border = 50;
		dest = eel_gdk_pixbuf_intersect (pixbuf, 0, 0, eel_gdk_pixbuf_whole_pixbuf);
		dest.x0 += 350;
		dest.y0 += border;
		dest.x1 -= border;
		dest.y1 -= border;
	}
	eel_debug_pixbuf_draw_rectangle_inset (pixbuf,
					       FALSE,
					       dest.x0,
					       dest.y0,
					       dest.x1,
					       dest.y1,
					       0xFFFF00,
					       0xFF,
					       -1);

	text = "This is text that needs t be wrapped to fit and stuff and foo and bar and more stuff.";
	smooth_text_layout = eel_smooth_text_layout_new (text,
							 strlen (text),
							 font,
							 font_size,
							 TRUE);
 	eel_smooth_text_layout_set_line_wrap_width (smooth_text_layout,
						    eel_art_irect_get_width (dest));
	eel_smooth_text_layout_draw_to_pixbuf (smooth_text_layout,
					       pixbuf,
					       0,
					       0,
					       dest,
					       GTK_JUSTIFY_CENTER,
					       FALSE,
					       0x00FF00,
					       0xff);
	dest.y0 += eel_smooth_text_layout_get_height (smooth_text_layout) + 10;
	dest.y1 += eel_smooth_text_layout_get_height (smooth_text_layout) + 10;
	gtk_object_unref (GTK_OBJECT (smooth_text_layout));

	smooth_text_layout = eel_smooth_text_layout_new (text,
							 strlen (text),
							 bold_font,
							 font_size * 1.5,
							 TRUE);
 	eel_smooth_text_layout_set_line_wrap_width (smooth_text_layout,
						    eel_art_irect_get_width (dest) + 2);
	
	if (1) eel_smooth_text_layout_draw_to_pixbuf (smooth_text_layout,
						      pixbuf,
						      1,
						      1,
						      dest,
						      GTK_JUSTIFY_CENTER,
						      TRUE,
						      0x00FF00,
						      0xff);
	gtk_object_unref (GTK_OBJECT (smooth_text_layout));

	eel_debug_show_pixbuf_in_external_viewer (pixbuf, "ee");
	
	gdk_pixbuf_unref (pixbuf);
	gtk_object_unref (GTK_OBJECT (font));
	gtk_object_unref (GTK_OBJECT (bold_font));

	return test_quit (EXIT_SUCCESS);
}
