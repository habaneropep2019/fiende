#include <config.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gnome.h>

#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-meta.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-print-pixbuf.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-printer-dialog.h>

static int do_preview;
static int do_rgb_dump;
static int dpi = 1;

static struct poptOption options [] = {
	{ "preview", 0, POPT_ARG_NONE, &do_preview },
	{ "dump",    0, POPT_ARG_NONE, &do_rgb_dump },
	{ "dpi",     0, POPT_ARG_INT,  &dpi },
	{ NULL }
};

static void
star (GnomePrintContext *pc, double x, double y)
{
	gnome_print_moveto (pc, x - 3, y - 5);
	gnome_print_lineto (pc, x + 3, y + 5);
	gnome_print_stroke (pc);

	gnome_print_moveto (pc, x + 3, y - 5);
	gnome_print_lineto (pc, x - 3, y + 5);
	gnome_print_stroke (pc);
}

static void
print_test_page (GnomePrintContext *pc)
{
	GnomeFont *font;
	int i;

	gnome_print_beginpage (pc, "testprint2 demo page");

	font = gnome_font_new ("Courier", 14);
	gnome_print_setfont (pc, font);

	for (i = 0; i < 900; i += 20){
		char buf [40];
		
		gnome_print_setrgbcolor (pc, 0.4, 0.4, 0.4);
		gnome_print_moveto (pc, 0, i);
		gnome_print_lineto (pc, 600, i);
		gnome_print_stroke (pc);

		gnome_print_setrgbcolor (pc, 1.0, 0.0, 0.0);
		sprintf (buf, "%d", i);
		gnome_print_moveto (pc, 10, i + 3);
		gnome_print_show (pc, buf);
	}

	gnome_print_setrgbcolor (pc, 0.4, 0.4, 0.4);
	for (i = 0; i < 900; i += 20){
		gnome_print_moveto (pc, i, 0);
		gnome_print_lineto (pc, i, 900);
		gnome_print_stroke (pc);
	}

	if (1){

		double aff [6];

		gnome_print_gsave (pc);
		art_affine_rotate (aff, 20);
		gnome_print_concat (pc, aff);
		art_affine_translate (aff, 100, 100);
		gnome_print_concat (pc, aff);
		gnome_print_grestore (pc);
	}

#if 0
	gnome_print_moveto (pc, 120, 120);
	gnome_print_lineto (pc, 200, 210);
	gnome_print_stroke (pc);

	gnome_print_setrgbcolor (pc, 0.1, 0.1, 0.1);
	gnome_print_moveto (pc, 200, 200);
	gnome_print_lineto (pc, 300, 300);
	gnome_print_stroke (pc);
#endif
	
	gnome_print_setrgbcolor (pc, 0, 0, 0);
	font = gnome_font_new ("Courier", 45);

	for (i = 0; i < 300; i += 50){
		char buf [90];

		sprintf (buf, "Hello-%d", i);
		gnome_print_setfont (pc, font);
		gnome_print_moveto (pc, i, i);
		gnome_print_show (pc, buf);
		star (pc, i, i); 
	}

	gnome_print_showpage (pc);
	gnome_print_context_close (pc);
	
}

static void
show_pixbuf (GnomePrintPixbuf * gpb, GdkPixbuf * pb, gint pagenum, gpointer data)
{
	int fh;
	gint y, w, h, rs;
	guchar * px, * p;

	g_print ("Showpixbuf called :)\n");

	w = gdk_pixbuf_get_width (pb);
	h = gdk_pixbuf_get_height (pb);
	rs = gdk_pixbuf_get_rowstride (pb);
	px = gdk_pixbuf_get_pixels (pb);

	fh = creat ("output.dump", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	g_return_if_fail (fh >= 0);

	for (y = 0; y < h; y++) {
		p = px + y * rs;
		write (fh, p, w * 4);
	}

	close (fh);
}

static void
test_ps (void)
{
	GnomePrinter *printer;
	GnomePrintContext *pc;
	GtkWidget *toplevel, *canvas, *sw;

	if (do_preview){
		gtk_widget_set_default_colormap (gdk_rgb_get_cmap ());
		gtk_widget_set_default_visual (gdk_rgb_get_visual ());
		
		toplevel = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_widget_set_usize (toplevel, 700, 700);
		sw = gtk_scrolled_window_new (NULL, NULL);
		canvas = gnome_canvas_new_aa ();
		gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (canvas), dpi);
		gtk_container_add (GTK_CONTAINER (toplevel), sw);
		gtk_container_add (GTK_CONTAINER (sw), canvas);
		
		pc = gnome_print_preview_new (GNOME_CANVAS(canvas), "US-Letter");
		
		gtk_widget_show_all (toplevel);
	} else {
		if (do_rgb_dump) {
			const GnomePaper * gp;
			gdouble w, h;

			gp = gnome_paper_with_name ("US-Letter");
			w = gnome_paper_pswidth (gp);
			h = gnome_paper_psheight (gp);

			pc = gnome_print_pixbuf_new (0.0, 0.0, w, h,
						dpi, dpi, TRUE);

			gtk_signal_connect (GTK_OBJECT (pc), "showpixbuf",
				GTK_SIGNAL_FUNC (show_pixbuf), NULL);

		} else {
			printer = gnome_printer_dialog_new_modal ();
	
			if (!printer)
				return;

			pc = gnome_print_context_new_with_paper_size (printer, "US-Letter");
		}
	}
	
	print_test_page (pc);
}

int
main (int argc, char **argv)
{
	gnome_init_with_popt_table ("TestPrint", VERSION, argc, argv, options, 0, NULL);

	test_ps ();

	if (do_preview)
		gtk_main ();

	return 0;
}
