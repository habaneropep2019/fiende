#include "test.h"

int 
main (int argc, char* argv[])
{
	GdkPixbuf *pixbuf;
	EelScalableFont *font;

	test_init (&argc, &argv);
	
	font = eel_scalable_font_get_default_font ();
	g_assert (font != NULL);

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, 800, 400);
	g_assert (pixbuf != NULL);
	
	eel_debug_pixbuf_draw_rectangle (pixbuf,
					 TRUE,
					 -1, -1, -1, -1,
					 EEL_RGB_COLOR_WHITE,
					 EEL_OPACITY_FULLY_OPAQUE);

	eel_scalable_font_draw_text (font,
				     pixbuf,
				     10,
				     100,
				     eel_gdk_pixbuf_whole_pixbuf,
				     80,
				     "Somoet",
				     strlen ("Somoet"),
				     EEL_RGBA_COLOR_OPAQUE_BLUE,
				     EEL_OPACITY_FULLY_OPAQUE);

	eel_debug_show_pixbuf_in_external_viewer (pixbuf, "ee");
	
	gdk_pixbuf_unref (pixbuf);
	
	gnome_vfs_shutdown ();

	return 0;
}
