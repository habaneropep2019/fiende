#include "hello-object-print.h"

void
hello_object_print (GnomePrintContext         *ctx,
		    double                     width,
		    double                     height,
		    const Bonobo_PrintScissor *scissor,
		    gpointer                   user_data)
{
	HelloBonoboEmbeddable *embeddable = user_data;
	GnomeFont             *font;
	double                 w, w2, h;
	const char            *str, *descr;

	str = embeddable->text ? embeddable->text : "No text";
	descr = "Value:";

	gnome_print_setlinewidth (ctx, 2);
	font = gnome_font_new ("Helvetica", 12.0);
	g_return_if_fail (font != NULL);
	gnome_print_setrgbcolor (ctx, 0.0, 0.0, 0.0);
	gnome_print_setfont (ctx, font);

	w = gnome_font_get_width_string (font, descr);
	w2 = gnome_font_get_width_string (font, str);
	h =
	    gnome_font_get_ascender (font) +
	    gnome_font_get_descender (font);

	gnome_print_moveto (ctx, (width / 2) - (w / 2), (height / 2) + h * 2);
	gnome_print_show (ctx, descr);
	gnome_print_moveto (ctx, (width / 2) - (w2 / 2), height / 2 - h);
	gnome_print_show (ctx, str);

	gtk_object_unref (GTK_OBJECT (font));
}
