/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <math.h>
#include <libart_lgpl/art_affine.h>
#include <gnome.h>

#include <libgnomeprint/gnome-print-pdf.h>
#include <libgnomeprint/gnome-print-ps.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-printer-dialog.h>

static int preview;

static struct poptOption options [] = {
	{ "preview", 0, POPT_ARG_NONE, &preview },
	{NULL}
};

#define P_WIDTH (21 * 72)
#define P_HEIGHT (30 * 72)

static void
do_print (GnomePrintContext * pc, gdouble scale)
{
	gint pages;

	for (pages = 0; pages < 100; pages++) {
		gnome_print_moveto      (pc, 100, 100);
		gnome_print_lineto      (pc, 200, 200);
		gnome_print_stroke      (pc);
		gnome_print_showpage    (pc);
		
		gnome_print_moveto      (pc, 100, 100);
		gnome_print_lineto      (pc, 200, 200);
		gnome_print_stroke      (pc);
		gnome_print_showpage    (pc);
	}

#if 0	
	GnomeFont *font;
 
	gnome_print_setrgbcolor (pc, 0,0,0);
	
	font = gnome_font_new_closest ("Times", GNOME_FONT_MEDIUM, 1, 12);
	if (font == NULL)
		g_warning ("Font is null\n");
	gnome_print_setfont     (pc, font);
	
	gnome_print_moveto      (pc, 100, 100);
	gnome_print_show        (pc, "Sheet1");


	font = gnome_font_new_closest ("Helvetica", GNOME_FONT_BOLD, 1, 12);
	if (font == NULL)
		g_warning ("Font is null\n");
	gnome_print_setfont     (pc, font);
	
	gnome_print_moveto      (pc, 100, 125);
	gnome_print_show        (pc, "Sheet2");

	font = gnome_font_new_closest ("Courier", GNOME_FONT_MEDIUM, 1, 12);
	if (font == NULL)
		g_warning ("Font is null\n");
	gnome_print_setfont     (pc, font);
	
	gnome_print_moveto      (pc, 100, 200);
	gnome_print_show        (pc, "Sheet3");


	font = gnome_font_new_closest ("Palatino", GNOME_FONT_BOLD, 1, 12);
	if (font == NULL)
		g_warning ("Font is null\n");
	gnome_print_setfont     (pc, font);
	
	gnome_print_moveto      (pc, 100, 225);
	gnome_print_show        (pc, "Sheet4");


	gnome_print_showpage    (pc);

	return;
	
	gnome_print_moveto      (pc, 286.561, 13.0866);
	gnome_print_show        (pc,"Page 1");
	gnome_print_newpath     (pc);
	gnome_print_moveto      (pc,257.575,368.5);
	gnome_print_lineto      (pc, 862.488,368.5);
	gnome_print_lineto      (pc, 862.488,1124.15);
	gnome_print_lineto      (pc, 257.575,1124.15);
	gnome_print_closepath   (pc);
	gnome_print_clip        (pc);
	gnome_print_newpath     (pc);

	gnome_print_gsave       (pc);
	gnome_print_setlinewidth (pc, 0);
	gnome_print_setrgbcolor (pc, 0, 0, 0);
	gnome_print_moveto      (pc,257.575,423.5 );
	gnome_print_lineto      (pc,305.575,423.5 );
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);

	gnome_print_gsave       (pc);
	gnome_print_setlinewidth (pc, 0);
	gnome_print_setrgbcolor (pc, 0, 0,0);
	gnome_print_moveto      (pc,257.575,423.5);
	gnome_print_lineto      (pc,257.575,410.75);
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);

	gnome_print_newpath (pc);
	gnome_print_moveto  (pc,259.575,422.5);
	gnome_print_lineto  (pc,304.575,422.5);
	gnome_print_lineto  (pc,304.575,409.75);
	gnome_print_lineto  (pc,259.575,409.75);
	gnome_print_closepath (pc);
	gnome_print_clip (pc);

	gnome_print_setrgbcolor (pc, 0, 0, 0);

	font = gnome_font_new_closest ("Times", GNOME_FONT_BOLD, 1, 9);
	if (font == NULL)
		g_warning ("Font is null\n");
	gnome_print_setfont     (pc, font);
	gnome_print_moveto (pc, 260.575,413.613 );
	gnome_print_show (pc,"Saludar al vecino, acostarse a una hora");

	/*
	gnome_print_showpage (pc);
	*/

#if 0	

	gnome_print_moveto (pc, 200, 200);
	gnome_print_lineto (pc, 500, 200);
	gnome_print_stroke (pc);
	gnome_print_showpage (pc);

	gnome_print_moveto (pc, 300, 300);
	gnome_print_lineto (pc, 500, 200);
	gnome_print_stroke (pc);
	gnome_print_showpage (pc);

	gnome_print_moveto (pc, 400, 400);
	gnome_print_lineto (pc, 500, 200);
	gnome_print_stroke (pc);
	gnome_print_showpage (pc);
#endif	

	return;

#if 0



#endif	
	
	
#ifdef PLAY_WITH_MATRICES	
	double matrix[6];

	matrix[0]=2;
	matrix[1]=0;
	matrix[2]=0;
	matrix[3]=2;
	matrix[4]=0;
	matrix[5]=0;

	gnome_print_concat (pc, matrix);
	
	gnome_print_moveto (pc, 100, 100);
	gnome_print_lineto (pc, 100, 200);
	gnome_print_lineto (pc, 200, 200);
	gnome_print_stroke (pc);
	
	gnome_print_moveto (pc, 5, 5);
	gnome_print_lineto (pc, 10, 10);
	gnome_print_stroke (pc);

	gnome_print_moveto (pc, .1, .1);
	gnome_print_moveto (pc, .2, .2);
	gnome_print_stroke (pc);

	gnome_print_showpage (pc);

	gnome_print_context_close (pc);
#endif	

#if 0	
	gnome_print_moveto (pc, 72, 72);
	gnome_print_lineto (pc, 10, 10);
	gnome_print_setlinecap (pc, 2);
	gnome_print_stroke (pc);
	gnome_print_showpage (pc);

	gnome_print_moveto (pc, 111, 222);
	gnome_print_lineto (pc, 200, 400);
	gnome_print_stroke (pc);
	gnome_print_showpage (pc);
#endif	

	
     /*
	static double dash[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
	static gint n_dash = 6;
     */

     /*

	gnome_print_beginpage (pc, "Test");
	gnome_print_scale (pc, scale, scale);

	gnome_print_newpath (pc);
	gnome_print_moveto (pc, 0.0, 0.0);
     */
	

#ifdef WE_DONT_NEED_THIS_COMMANDS_FOR_NOW
	gnome_print_lineto (pc, 0.0, P_HEIGHT);
	gnome_print_lineto (pc, P_WIDTH, P_HEIGHT);
	gnome_print_lineto (pc, P_WIDTH, 0.0);
	gnome_print_closepath (pc);
	gnome_print_gsave (pc);
	gnome_print_setopacity (pc, 1.0);
	gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
	gnome_print_setlinewidth (pc, 6.0);
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);
	gnome_print_setopacity (pc, 1.0);
	gnome_print_setrgbcolor (pc, 1.0, 1.0, 1.0);
	gnome_print_eofill (pc);

	gnome_print_gsave (pc);
	gnome_print_translate (pc, 100.0, 100.0);
	gnome_print_newpath (pc);
	do_stars (pc, 100.0, 5);
	gnome_print_setrgbcolor (pc, 1.0, 0.5, 0.0);
	gnome_print_setopacity (pc, 0.7);
	gnome_print_eofill (pc);
	gnome_print_grestore (pc);

	gnome_print_gsave (pc);
	gnome_print_translate (pc, 300.0, 100.0);
	gnome_print_newpath (pc);
	do_stars (pc, 100.0, 5);
	gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
	gnome_print_setopacity (pc, 1.0);
	gnome_print_setlinewidth (pc, 4.0);
	gnome_print_setdash (pc, n_dash, dash, 0.0);
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);

	gnome_print_translate (pc, 200, 400);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
	gnome_print_scale (pc, 3.0, 3.0);
	gnome_print_newpath (pc);
	gnome_print_moveto (pc, 20.0, 20.0);
	gnome_print_show (pc, "Hello");
	gnome_print_grestore (pc);

	gnome_print_newpath (pc);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.5, 0.0);
	gnome_print_setopacity (pc, 0.6);
	gnome_print_translate (pc, 30.0, 30.0);
	do_rosette (pc);
	gnome_print_grestore (pc);

	gnome_print_newpath (pc);
	gnome_print_moveto (pc, 50.0, 50.0);
	gnome_print_lineto (pc, 50.0, 250.0);
	gnome_print_lineto (pc, 250.0, 250.0);
	gnome_print_lineto (pc, 250.0, 50.0);
	gnome_print_closepath (pc);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);

	gnome_print_gsave (pc);
	gnome_print_eoclip (pc);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 1.0, 0.5, 0.0);
	gnome_print_setopacity (pc, 0.6);
	gnome_print_translate (pc, 130.0, 130.0);
	gnome_print_scale (pc, 2.0, 2.0);
	do_rosette (pc);
	gnome_print_grestore (pc);

	gnome_print_translate (pc, 100.0, 100.0);
	gnome_print_scale (pc, 0.5, 0.5);

	gnome_print_newpath (pc);
	do_star (pc, 200.0);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
	gnome_print_setlinewidth (pc, 4.0);
	gnome_print_stroke (pc);
	gnome_print_grestore (pc);

	gnome_print_gsave (pc);
	gnome_print_eoclip (pc);

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.5, 1.0);
	gnome_print_setopacity (pc, 0.6);
	gnome_print_translate (pc, 0.0, 0.0);
	gnome_print_scale (pc, 2.0, 2.0);
	do_rosette (pc);
	gnome_print_grestore (pc);

	do_image (pc, 150, TRUE);

	gnome_print_translate (pc, 45.0, 45.0);
	gnome_print_scale (pc, 0.5, 0.5);

	gnome_print_newpath (pc);
	do_star (pc, 100.0);

	gnome_print_gsave (pc);
	gnome_print_setlinewidth (pc, 36.0);
	gnome_print_strokepath (pc);

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

	gnome_print_gsave (pc);
	gnome_print_setrgbcolor (pc, 0.0, 0.2, 0.0);
	gnome_print_setopacity (pc, 0.9);
	gnome_print_translate (pc, 130.0, 130.0);
	gnome_print_scale (pc, 8.0, 8.0);
	do_rosette (pc);
	gnome_print_grestore (pc);

	gnome_print_rotate (pc, 30.0);
	do_image (pc, 100, FALSE);
#endif
#endif	
}

static gint
delete_event (GtkWidget * widget)
{
	gtk_main_quit ();
	return FALSE;
}

static void
do_preview (gboolean aa)
{
	GtkWidget * w, * sw, * c;
	GnomePrintContext * pc;

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


#if WE_DONT_NEDD_RBUF_I_GUESS

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

#endif


static void
do_pdf (void)
{
     GnomePrintContext *pc;
     GnomePrinter *printer;
     
     printer = gnome_printer_dialog_new_modal ();

     if (!printer)
	  return;

     pc  = gnome_print_context_new (printer);

     do_print (pc, 1);
     gnome_print_context_close (pc);
     gtk_object_unref (GTK_OBJECT (pc));
     
}
     
int
main (int argc, char ** argv)
{
	gnome_init_with_popt_table ("TestPdf", "0.1", argc, argv, options, 0, NULL);

	if (preview) {
		do_preview (TRUE);
		gtk_main ();
	} else {
                do_pdf ();
	}

#ifdef WE_DONT_NEED_THIS
	if (preview_gdk) {
		do_preview (FALSE);
	}

	if (rbuf) {
		do_rbuf (FALSE, FALSE);
	}

	if (rbuf_alpha) {
		do_rbuf (TRUE, FALSE);
	}

	if (rbuf_frgba) {
		do_rbuf (TRUE, TRUE);
	}
#endif	


	return 0;
}









































































#if WE_ARE_NOT_IMPLEMENTING_IMAGES_YET
static void do_image (GnomePrintContext * pc, gdouble size, gint alpha);

static void
do_image (GnomePrintContext * pc, gdouble size, gboolean alpha)
{
	guchar * image;
	gint x, y;
	guchar * p;

	image = g_new (guchar, 256 * 256 * 4);
	for (y = 0; y < 256; y++) {
		p = image + 256 * 4 * y;
		for (x = 0; x < 256; x++) {
			*p++ = x;
			*p++ = y;
			*p++ = 0;
			*p++ = alpha ? x : 0xff;
		}
	}

	gnome_print_gsave (pc);
	gnome_print_translate (pc, 0.0, size);
	gnome_print_scale (pc, size, -size);
	gnome_print_rgbaimage (pc, image, 256, 256, 4 * 256);
	gnome_print_grestore (pc);

	g_free (image);
}

#endif

#ifdef I_HATE_STARS_THEY_ARE_LAME
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
do_stars (GnomePrintContext * pc, gdouble size, gint num)
{
	gdouble step, i;

	step = size / ((double) num - 0.99);

	for (i = 0; i < size; i += step) {
		do_star (pc, size);
		gnome_print_translate (pc, step, step / 4);
	}
}
#endif

#ifdef I_HATE_ROSSETTES_TOO
static void
do_rosette (GnomePrintContext * pc)
{
	gdouble angle;
	gdouble m[6];

	for (angle = 0.0; angle < 360.0; angle += 54.0) {
		art_affine_rotate (m, angle);
		gnome_print_gsave (pc);
		gnome_print_concat (pc, m);
		gnome_print_moveto (pc, -45.0, 0.0);
		gnome_print_show (pc, "Hmmm... this seems to really work!");
		gnome_print_grestore (pc);
	}
}
#endif

