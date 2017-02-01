/*
 * Test program for gnome-print glyphlists
 *
 * Usage: testprint4 [options] filename
 *
 * Try it with COPYING file, to see some nice things
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright 2000-2001 Ximian, inc.
 *
 */

#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libart_lgpl/art_affine.h>
#include <gnome.h>

#include "libgnomeprint/gp-unicode.h"
#include "libgnomeprint/gnome-printer.h"
#include "libgnomeprint/gnome-printer-dialog.h"

#include "libgnomeprint/gnome-print-rbuf.h"
#include "libgnomeprint/gnome-print-frgba.h"
#include "libgnomeprint/gnome-print-preview.h"

#include "libgnomeprint/gnome-font-dialog.h"

#define P_WIDTH (21 * 72.0 / 2.54)
#define P_HEIGHT (30 * 72.0 / 2.54)
#define MARGIN (2 * 72.0 / 2.54)
#define LINESPACING 6.0
#define INDENT 20.0

static int preview_gdk = FALSE;
static int preview = FALSE;
static int rbuf = FALSE;
static int rbuf_alpha = FALSE;
static int rbuf_frgba = FALSE;
static int gtest = FALSE;
static char *itest = NULL;
static char *filename = NULL;

static struct poptOption options [] = {
	{ "preview", 0, POPT_ARG_NONE, &preview },
	{ "preview-gdk", 0, POPT_ARG_NONE, &preview_gdk },
	{ "rbuf", 0, POPT_ARG_NONE, &rbuf },
	{ "rbuf-alpha", 0, POPT_ARG_NONE, &rbuf_alpha },
	{ "rbuf-frgba", 0, POPT_ARG_NONE, &rbuf_frgba },
	{ "graphic", 0, POPT_ARG_NONE, &gtest },
	{ "image", 'i', POPT_ARG_STRING, &itest },
	{NULL}
};

#if 0
static gint latin_to_utf8 (guchar * text, guchar * utext, gint ulength);
#endif

static gint
print_paragraph (GnomePrintContext *pc, GnomeFont *font, const guchar *b, const guchar *e, gdouble x0, gdouble *y0, gdouble x1, gdouble y1)
{
	GSList *words;
	const guchar *p;
	gchar *ub, *u;
	gdouble fontheight;
	gint space;
	gdouble x, y;

	if (!font) return TRUE;
	g_print ("Ascender %g\n", gnome_font_get_ascender (font));
	g_print ("Descender %g\n", gnome_font_get_descender (font));
	fontheight = gnome_font_get_ascender (font) + gnome_font_get_descender (font);
	space = gnome_font_face_lookup_default (gnome_font_get_face (font), ' ');

	/* Test for line space */
	if (*y0 - y1 < fontheight) return TRUE;

	/* Split text into words & convert to utf-8 */
	ub = g_new (guchar, (e - b) * 2 + 1);
	u = ub;
	words = NULL;
	p = b;
	while (p < e) {
		while ((p < e) && (*p <= ' ')) p++;
		if (p < e) {
			words = g_slist_prepend (words, u);
			while ((p < e) && (*p > ' ')) {
#if 0
				u += g_unichar_to_utf8 (*p++, u);
#else
				*u++ = *p++;
#endif
			}
			*u++ = '\0';
		}
	}
	words = g_slist_reverse (words);

	x = x0 + INDENT;
	while ((*y0 - y1 >= fontheight) && (words != NULL)) {
		ArtPoint spadv;
		gdouble accwidth, spcwidth;
		gboolean stop;
		GSList *lw;
		gint numwords;
		GnomeGlyphList * gl;
		/* Find actual Y */
		y = *y0 - gnome_font_get_ascender (font);
		/* Find space advance */
		gnome_font_get_glyph_stdadvance (font, space, &spadv);
		accwidth = 0.0;
		spcwidth = 0.0;
		stop = FALSE;
		lw = NULL;
		while ((accwidth <= (x1 - x)) && !stop) {
			gdouble width;
			width = gnome_font_get_width_string (font, (gchar *) words->data);
			if (accwidth > 0.0) spcwidth += spadv.x;
			if ((accwidth == 0.0) || (accwidth + spcwidth + width < (x1 - x))) {
				/* We have room */
				accwidth += width;
				lw = g_slist_prepend (lw, words->data);
				words = g_slist_remove (words, words->data);
				if (!words) stop = TRUE;
			} else {
				stop = TRUE;
			}
		}
		lw = g_slist_reverse (lw);
		/* Typeset */
		numwords = g_slist_length (lw);
		if ((numwords > 1) && (words != NULL)) {
			spcwidth = ((x1 - x) - accwidth) / (numwords - 1);
		} else {
			spcwidth = spadv.x;
		}
		gl = gnome_glyphlist_from_text_dumb (font, 0x000000ff, 0.0, 0.0, "");
		gnome_glyphlist_advance (gl, TRUE);
		while (lw) {
			guchar *str;
			str = (guchar *) lw->data;
			gnome_glyphlist_moveto (gl, x, y);
			if (!strcmp (str, "GNU")) {
				gint glyph;
				gnome_glyphlist_color (gl, 0xff0000ff);
				gnome_glyphlist_rmoveto (gl, 0.0, -fontheight / 3.0);
				glyph = gnome_font_face_lookup_default (gnome_font_get_face (font), 'G');
				gnome_glyphlist_glyph (gl, glyph);
				gnome_glyphlist_color (gl, 0x00ff00ff);
				gnome_glyphlist_rmoveto (gl, 0.0, fontheight / 3.0);
				glyph = gnome_font_face_lookup_default (gnome_font_get_face (font), 'N');
				gnome_glyphlist_glyph (gl, glyph);
				gnome_glyphlist_color (gl, 0x0000ffff);
				gnome_glyphlist_rmoveto (gl, 0.0, fontheight / 3.0);
				glyph = gnome_font_face_lookup_default (gnome_font_get_face (font), 'U');
				gnome_glyphlist_glyph (gl, glyph);
				gnome_glyphlist_color (gl, 0x000000ff);
				gnome_glyphlist_rmoveto (gl, 0.0, -fontheight / 3.0);
			} else {
				gnome_glyphlist_text_dumb (gl, str);
			}
			x += gnome_font_get_width_string (font, str);
			x += spcwidth;
			lw = g_slist_remove (lw, str);
		}
		gnome_print_moveto (pc, 0.0, 0.0);
		gnome_print_glyphlist (pc, gl);
		gnome_glyphlist_unref (gl);
		*y0 = *y0 - gnome_font_get_size (font) - LINESPACING;
		x = x0;
	}

	if (words) g_slist_free (words);

	g_free (ub);

	return FALSE;
}

static void
do_print_text_page (GnomePrintContext *pc, GnomeFont *font)
{
	struct stat s;
	gint fh, len;
	guchar *b;
	gboolean stop = FALSE;
	gdouble x0, y0, x1, y1;

	if (!stat (filename, &s) && S_ISREG (s.st_mode)) {
		len = s.st_size;
		fh = open (filename, O_RDONLY);
		if (fh >= 0) {
			b = g_new (guchar, len + 1);
			*(b + len) = '\0';
			read (fh, b, len);
			close (fh);

			x0 = MARGIN;
			y0 = P_HEIGHT - MARGIN;
			x1 = P_WIDTH - MARGIN;
			y1 = MARGIN;
			while (!stop) {
				/* find start of paragraph */
				while (*b && *b <= ' ') b++;
				if (*b) {
					guchar *e;
					e = strstr (b, "\n\n");
					if (e == NULL) {
						e = b;
						while (*e) e++;
					}
					stop = print_paragraph (pc, font, b, e, x0, &y0, x1, y1);
					b = e;
				} else {
					stop = TRUE;
				}
			}
		}
	}

}

static void
do_print_test_page (GnomePrintContext *pc, GnomeFont *font)
{
	gdouble d;

	for (d = MARGIN; d < P_WIDTH - MARGIN - 72.0; d += 36.0) {
		gnome_print_moveto (pc, d, P_HEIGHT - MARGIN - 72.0);
		gnome_print_lineto (pc, d + 72.0, P_HEIGHT - MARGIN);
		gnome_print_stroke (pc);
	}
}

static void
do_print (GnomePrintContext * pc, gdouble scale, GnomeFont *font)
{
	/* Draw box at margins */
	gnome_print_beginpage (pc, "printtest4 demo page");
	gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
	gnome_print_setlinewidth (pc, 1.0);
	gnome_print_moveto (pc, MARGIN, P_HEIGHT - MARGIN);
	gnome_print_lineto (pc, P_WIDTH - MARGIN, P_HEIGHT - MARGIN);
	gnome_print_lineto (pc, P_WIDTH - MARGIN, MARGIN);
	gnome_print_lineto (pc, MARGIN, MARGIN);
	gnome_print_closepath (pc);
	gnome_print_stroke (pc);

	if (itest) {
		GdkPixbuf *pixbuf;
		pixbuf = gdk_pixbuf_new_from_file (itest);
		gnome_print_gsave (pc);
		gnome_print_translate (pc, 200, 200);
#if 1
		gnome_print_rotate (pc, 45);
#endif
		gnome_print_scale (pc, gdk_pixbuf_get_width (pixbuf), gdk_pixbuf_get_height (pixbuf));
		gnome_print_setrgbcolor (pc, 0.9, 0.8, 0.6);
		gnome_print_moveto (pc, 0, 0);
		gnome_print_lineto (pc, 0, 1);
		gnome_print_lineto (pc, 1, 1);
		gnome_print_lineto (pc, 1, 0);
		gnome_print_fill (pc);
		gnome_print_pixbuf (pc, pixbuf);
		gnome_print_grestore (pc);
		gdk_pixbuf_unref (pixbuf);
	} else if (gtest) {
		do_print_test_page (pc, font);
	} else {
		do_print_text_page (pc, font);
	}

	gnome_print_showpage (pc);
}

static gint
delete_event (GtkWidget * widget)
{
	gtk_main_quit ();
	return FALSE;
}

static gint
do_dialog (GnomeFont *font)
{
	GnomePrinter *printer;
	GnomePrintContext *pc;

	printer = gnome_printer_dialog_new_modal ();
	
	if (!printer) return 0;

	pc = gnome_print_context_new_with_paper_size (printer, "US-Letter");

	do_print (pc, 1.0, font);

	gnome_print_context_close (pc);

	return 0;
}


static void
do_preview (GnomeFont *font, gboolean aa)
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

	do_print (pc, 1.0, font);

	gnome_print_context_close (pc);
}

#define PMSCALE 0.5
#define PMW (PMSCALE * P_WIDTH)
#define PMH (PMSCALE * P_HEIGHT)

static void
do_rbuf (GnomeFont *font, gboolean alpha, gboolean frgba)
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
#warning I disabled this code casue it was throwing an error at me
#if 0	
	memset (buf, 0xff, PMW * PMH * bpp);
#endif

	pc = gnome_print_rbuf_new (buf, PMW, PMH, bpp * PMW, p2b, alpha);

	if (frgba) {
		pc = gnome_print_frgba_new (pc);
	}

	do_print (pc, 1.0, font);

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
	poptContext ctx = NULL;
	const char **args;
	GnomeFont *font;
	GtkWidget *d, *w;
	gint b;

	gnome_init_with_popt_table ("TestPrint", "0.1", argc, argv, options, 0, &ctx);
	args = poptGetArgs (ctx);
	if (!gtest && !itest) {
		if (!args || !*args) {
			g_print ("Usage: testprint4 [arguments] file\n");
			exit (0);
		}
		filename = g_strdup (*args);
	}

	poptFreeContext (ctx);

	d = gnome_font_selection_dialog_new ("Test");
	b = gnome_dialog_run (GNOME_DIALOG (d));
	if (b == 0) {
		w = gnome_font_selection_dialog_get_fontsel (GNOME_FONT_SELECTION_DIALOG (d));
		font = gnome_font_selection_get_font (GNOME_FONT_SELECTION (w));
	} else {
		font = gnome_font_new_closest ("test (hehe)", GNOME_FONT_BOOK, FALSE, 18.0);
	}
	gtk_widget_destroy (d);

	if (preview) {
		do_preview (font, TRUE);
		gtk_main ();
	} else if (preview_gdk) {
		do_preview (font, FALSE);
		gtk_main ();
	} else if (rbuf) {
		do_rbuf (font, FALSE, FALSE);
		gtk_main ();
	} else 	if (rbuf_alpha) {
		do_rbuf (font, TRUE, FALSE);
		gtk_main ();
	} else if (rbuf_frgba) {
		do_rbuf (font, TRUE, TRUE);
		gtk_main ();
	} else {
	     do_dialog (font);
	}


	return 0;
}

#if 0
static gint
latin_to_utf8 (guchar * text, guchar * utext, gint ulength)
{
	guchar * i, * o;

	o = utext;

	for (i = text; *i; i++) {
		o += g_unichar_to_utf8 (*i, o);
	}

	return o - utext;
}
#endif

