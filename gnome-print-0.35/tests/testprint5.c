#include <math.h>
#include <libart_lgpl/art_affine.h>
#include <gnome.h>

#include "libgnomeprint/gnome-printer.h"
#include "libgnomeprint/gnome-printer-dialog.h"

#include "libgnomeprint/gnome-print-rbuf.h"
#include "libgnomeprint/gnome-print-frgba.h"
#include "libgnomeprint/gnome-print-preview.h"

#include "libgnomeprint/gnome-font-dialog.h"

#define P_WIDTH (21 * 72)
#define P_HEIGHT (30 * 72)

static int preview_gdk;
static int preview;
static int rbuf;
static int rbuf_alpha;
static int rbuf_frgba;

static struct poptOption options [] = {
	{ "preview", 0, POPT_ARG_NONE, &preview },
	{ "preview-gdk", 0, POPT_ARG_NONE, &preview_gdk },
	{ "rbuf", 0, POPT_ARG_NONE, &rbuf },
	{ "rbuf-alpha", 0, POPT_ARG_NONE, &rbuf_alpha },
	{ "rbuf-frgba", 0, POPT_ARG_NONE, &rbuf_frgba },
	{NULL}
};

static void
do_star (GnomePrintContext * pc, gdouble size) {
	gdouble angle, ra, x, y;

	gnome_print_moveto (pc, 0.5 * size, 0.0);

#define ANGLE (M_PI / 3.0)
#define HANGLE (M_PI / 6.0)

	for (angle = 0.0; angle < 359.0; angle += 60.0) {
		ra = angle * M_PI / 180.0;
		x = size * cos (ra + HANGLE);
		y = size * sin (ra + HANGLE);
		gnome_print_lineto (pc, x, y);
		x = 0.5 * size * cos (ra + ANGLE);
		y = 0.5 * size * sin (ra + ANGLE);
		gnome_print_lineto (pc, x, y);
	}

	gnome_print_closepath (pc);
}


static void
do_print (GnomePrintContext * pc, gdouble scale)
{
	GnomeFont * font;

	font = gnome_font_new ("Helvetica", 12.0);
	gnome_print_beginpage (pc, "Test");

	gnome_print_scale (pc, scale, scale);
	gnome_print_translate (pc, 72*2, 72*2);
	gnome_print_scale (pc, 0.5, 0.5);

	gnome_print_newpath (pc);
	do_star (pc, 100.0);

	gnome_print_gsave (pc);
	  gnome_print_setlinewidth (pc, 36.0);
	  gnome_print_strokepath (pc);
	  gnome_print_setmiterlimit (pc, 50);

	  gnome_print_gsave (pc);
	    gnome_print_setlinewidth (pc, 2.0);
	    gnome_print_stroke (pc);
	  gnome_print_grestore (pc);

	  gnome_print_gsave (pc);
	    gnome_print_setrgbcolor (pc, 1.0, 0.5, 0.5);
	    gnome_print_eofill (pc);
	  gnome_print_grestore (pc);

	  gnome_print_gsave (pc);
	    gnome_print_eoclip (pc);
	  gnome_print_grestore (pc);

#if 0	  
	  gnome_print_gsave (pc);
	    gnome_print_setrgbcolor (pc, 0.0, 0.2, 0.0);
	    gnome_print_setopacity (pc, 0.9);
	    gnome_print_translate (pc, 130.0, 130.0);
	    gnome_print_scale (pc, 8.0, 8.0);
	    do_rosette (pc, font);
	  gnome_print_grestore (pc);
#endif	
        gnome_print_grestore (pc);

	/* add some rectangles */

	g_print ("Printing a rectangle\n");

	gnome_print_gsave (pc);
 	  gnome_print_scale (pc, 100, 100);
	  gnome_print_moveto (pc,  2, 2);
	  gnome_print_lineto (pc,  2, 3.5);
	  gnome_print_lineto (pc,  3, 3.5);
	  gnome_print_lineto (pc,  3, 2);
	  gnome_print_lineto (pc,  2, 2);
	  gnome_print_stroke (pc);
	gnome_print_grestore (pc);

	g_print ("Printing a rectangle wanabe\n");

	gnome_print_gsave (pc);
 	  gnome_print_scale (pc, 100, 100);
	  gnome_print_moveto (pc,  2, 2);
	  gnome_print_lineto (pc,  2, 3.7);
	  gnome_print_lineto (pc,  10, 3.5);
	  gnome_print_lineto (pc,  4, 2);
	  gnome_print_lineto (pc,  2, 2);
	  gnome_print_stroke (pc);
	gnome_print_grestore (pc);

	g_print ("Printing a rectangle wanabe\n");

	gnome_print_gsave (pc);
 	  gnome_print_scale (pc, 100, 100);
	  gnome_print_moveto (pc,  2, 2);
	  gnome_print_lineto (pc,  2, 3.5);
	  gnome_print_lineto (pc,  3, 3.5);
	  gnome_print_lineto (pc,  3, 2);
	  gnome_print_stroke (pc);
	gnome_print_grestore (pc);
	
	  

	/*
	do_image (pc, 100, FALSE);
	*/
}

static gint
delete_event (GtkWidget * widget)
{
	gtk_main_quit ();
	return FALSE;
}

static gint
do_dialog (void)
{
     GnomePrinter *printer;
     GnomePrintContext *pc;

     printer = gnome_printer_dialog_new_modal ();
	
     if (!printer)
	  return 0;

     pc = gnome_print_context_new_with_paper_size (printer, "US-Letter");

     do_print (pc, 1.0);

     gnome_print_context_close (pc);

     return 0;
}


static void
do_preview (gboolean aa)
{
	GtkWidget * w, * sw, * c;
	GnomePrintContext * pc;

	w = gnome_font_selection_dialog_new ("Test");
	gtk_widget_show (w);

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_widget_set_usize (w, 512, 512);

	gtk_signal_connect (GTK_OBJECT (w), "delete_event",
		GTK_SIGNAL_FUNC (delete_event), NULL);

	sw = gtk_scrolled_window_new (NULL, NULL);

	if (aa) {
		gtk_widget_push_colormap (gdk_rgb_get_cmap ());
		gtk_widget_push_visual (gdk_rgb_get_visual ());

		c = gnome_canvas_new_aa ();

		gtk_widget_pop_visual ();
		gtk_widget_pop_colormap ();

	} else {
		gtk_widget_push_colormap (gdk_rgb_get_cmap ());
		gtk_widget_push_visual (gdk_rgb_get_visual ());

		c = gnome_canvas_new ();

		gtk_widget_pop_visual ();
		gtk_widget_pop_colormap ();

	}

	gnome_canvas_set_scroll_region ((GnomeCanvas *) c, 0, 0, P_WIDTH, P_HEIGHT);

	gtk_container_add (GTK_CONTAINER (sw), c);
	gtk_container_add (GTK_CONTAINER (w), sw);

	gtk_widget_show_all (w);

	pc = gnome_print_preview_new (GNOME_CANVAS (c), "A4");

	do_print (pc, 1.0);

	gnome_print_context_close (pc);
}

#define PMSCALE 0.5
#define PMW (PMSCALE * P_WIDTH)
#define PMH (PMSCALE * P_HEIGHT)

static void
do_rbuf (gboolean alpha, gboolean frgba)
{
	GtkWidget * w, * sw, * p;
	gint bpp;
	guchar * buf;
	gdouble p2b[6];
	GnomePrintContext * pc;
	GdkPixbuf * pb;
	GdkPixmap * pm;
	GdkBitmap * bm;

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_widget_set_usize (w, 512, 512);

	gtk_signal_connect (GTK_OBJECT (w), "delete_event",
		GTK_SIGNAL_FUNC (delete_event), NULL);

	sw = gtk_scrolled_window_new (NULL, NULL);

	bpp = (alpha) ? 4 : 3;
	art_affine_scale (p2b, PMSCALE, -PMSCALE);
	p2b[5] = PMH;

	buf = g_new (guchar, PMW * PMH * bpp);

	pc = gnome_print_rbuf_new (buf, PMW, PMH, bpp * PMW, p2b, alpha);

	if (frgba) {
		pc = gnome_print_frgba_new (pc);
	}

	do_print (pc, 2.0);

	gnome_print_context_close (pc);

	pb = gdk_pixbuf_new_from_data (buf, GDK_COLORSPACE_RGB, alpha,
		8, PMW, PMH, bpp * PMW, NULL, NULL);

	gdk_pixbuf_render_pixmap_and_mask (pb, &pm, &bm, 128);

	gdk_pixbuf_unref (pb);
	g_free (buf);

	p = gtk_pixmap_new (pm, bm);

	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), p);
	gtk_container_add (GTK_CONTAINER (w), sw);

	gtk_widget_show_all (w);
}

int
main (int argc, char ** argv)
{
	gnome_init_with_popt_table ("TestPrint", "0.1", argc, argv, options, 0, NULL);

	if (preview) {
		do_preview (TRUE);
		gtk_main ();
	} else if (preview_gdk) {
		do_preview (FALSE);
		gtk_main ();
	} else if (rbuf) {
		do_rbuf (FALSE, FALSE);
	} else 	if (rbuf_alpha) {
		do_rbuf (TRUE, FALSE);
	} else if (rbuf_frgba) {
		do_rbuf (TRUE, TRUE);
	} else {
	     do_dialog ();
	}


	return 0;
}
