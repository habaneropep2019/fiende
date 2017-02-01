#define __GNOME_PRINT_PS2_C__

/*
 * gnome-print-ps2.c: A Postscript driver for GnomePrint based	in
 * gnome-print-pdf which was based on the PS driver by Raph Levien.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors:
 *   Chema Celorio <chema@celorio.com>
 *   Lauris Kaplinski <lauris@helixcode.com>
 *   Michael Sweet <mike@easysw.com>
 *
 * Copyright 2000-2001 Ximian, Inc. and authors
 *
 */

/* __FUNCTION__ is not defined in Irix according to David Kaelbling <drk@sgi.com>*/
#ifndef __GNUC__
  #define __FUNCTION__   ""
#endif
#define debug(section,str) if (FALSE) printf ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str); 
	
#include "config.h"
#include <math.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <locale.h>
#include <unistd.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_misc.h>
#include <libgnome/gnome-paper.h>
#include <libgnomeprint/gp-unicode.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-printer-private.h>
#include <libgnomeprint/gnome-print-ps2.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-print-encode.h>
#include <libgnomeprint/gnome-print-encode-private.h>

#include <libgnomeprint/gnome-pgl-private.h>

#ifdef ENABLE_LIBGPA
#include <libgpa/gpa-printer.h>
#include <libgpa/gpa-settings.h>
#endif

#define EOL "\n"

typedef struct _GPPS2Font GPPS2Font;
typedef struct _GPPS2Page GPPS2Page;

struct _GPPS2Font {
	GPPS2Font *next;
	GnomeFontFace *face;
	GFPSObject *pso;
	gdouble currentsize;
};

struct _GPPS2Page {
	GPPS2Page *next;
	gchar *name;
	gint number;
	gboolean shown;
	GSList *usedfonts;
};

struct _GnomePrintPs2 {
	GnomePrintContext pc;

#ifdef ENABLE_LIBGPA	
	/* GpaPrinter */
	GpaPrinter *gpa_printer;
	GpaSettings *gpa_settings;
#endif

	GPPS2Font *fonts;
	GPPS2Font *selectedfont;

        gdouble r, g, b;
	gint private_color_flag;

	GPPS2Page *pages;

	gint gsave_level;
	gint ps_level;

	FILE *buf;
	gchar *bufname;

	ArtDRect bbox;
};

struct _GnomePrintPs2Class
{
	GnomePrintContextClass parent_class;
};

static void gnome_print_ps2_class_init (GnomePrintPs2Class *klass);
static void gnome_print_ps2_init (GnomePrintPs2 *PS2);
static void gnome_print_ps2_destroy (GtkObject *object);

static gint gnome_print_ps2_gsave (GnomePrintContext *pc);
static gint gnome_print_ps2_grestore (GnomePrintContext *pc);
static gint gnome_print_ps2_fill (GnomePrintContext *pc, ArtWindRule rule);
static gint gnome_print_ps2_clip (GnomePrintContext *pc, ArtWindRule rule);
static gint gnome_print_ps2_stroke (GnomePrintContext *pc);
static gint gnome_print_ps2_glyphlist (GnomePrintContext *pc, GnomeGlyphList *gl);
static gint gnome_print_ps2_grayimage (GnomePrintContext *pc, const char *data, gint width, gint height, gint rowstride);
static gint gnome_print_ps2_rgbimage (GnomePrintContext *pc, const char *data, gint width, gint height, gint rowstride);
static gint gnome_print_ps2_beginpage (GnomePrintContext *pc, const char *name);
static gint gnome_print_ps2_showpage (GnomePrintContext *pc);
static gint gnome_print_ps2_close (GnomePrintContext *pc);

static gint gp_ps2_set_color (GnomePrintContext *pc);
static gint gp_ps2_set_line (GnomePrintContext *pc);
static gint gp_ps2_set_dash (GnomePrintContext *pc);

static gint gp_ps2_set_color_private (GnomePrintContext *pc, gdouble r, gdouble g, gdouble b);
static gint gp_ps2_set_font_private (GnomePrintContext *pc, const GnomeFont *font);

static gint gp_ps2_print_path (GnomePrintContext *pc, const GPPath *gppath);
static int gnome_print_ps2_image (GnomePrintContext *pc, const char *data, int width, int height, int rowstride, int bytes_per_pixel);

static gchar* gnome_print_ps2_get_date (void);
static gint gp_ps2_fprintf (GnomePrintPs2 *ps2, const char *format, ...);


#ifdef ENABLE_LIBGPA
static gint gnome_print_ps2_get_level (GnomePrintPs2 *ps2);
#endif

static GnomePrintContextClass *parent_class;

GtkType
gnome_print_ps2_get_type (void)
{
	static GtkType ps2_type = 0;

	if (!ps2_type) {
		GtkTypeInfo ps2_info = {
			"GnomePrintps2",
			sizeof (GnomePrintPs2),
			sizeof (GnomePrintPs2Class),
			(GtkClassInitFunc)  gnome_print_ps2_class_init,
			(GtkObjectInitFunc) gnome_print_ps2_init,
			NULL, NULL, NULL
		};
		ps2_type = gtk_type_unique (gnome_print_context_get_type (), &ps2_info);
	}

	return ps2_type;
}

static void
gnome_print_ps2_class_init (GnomePrintPs2Class *klass)
{
	GtkObjectClass *object_class;
	GnomePrintContextClass *pc_class;

	object_class = (GtkObjectClass *) klass;
	pc_class = (GnomePrintContextClass *)klass;

	parent_class = gtk_type_class (gnome_print_context_get_type ());
	
	object_class->destroy = gnome_print_ps2_destroy;

	pc_class->gsave = gnome_print_ps2_gsave;
	pc_class->grestore = gnome_print_ps2_grestore;
	pc_class->fill = gnome_print_ps2_fill;
	pc_class->clip = gnome_print_ps2_clip;
	pc_class->stroke = gnome_print_ps2_stroke;
	pc_class->glyphlist = gnome_print_ps2_glyphlist;
	pc_class->grayimage = gnome_print_ps2_grayimage;
	pc_class->rgbimage = gnome_print_ps2_rgbimage;
	pc_class->beginpage = gnome_print_ps2_beginpage;
	pc_class->showpage = gnome_print_ps2_showpage;
	pc_class->close = gnome_print_ps2_close;
}

static void
gnome_print_ps2_init (GnomePrintPs2 *ps2)
{
	ps2->ps_level = 2;
	
	ps2->gsave_level = 0;

	ps2->fonts = NULL;
	ps2->selectedfont = NULL;

	ps2->private_color_flag = GP_GC_FLAG_UNSET;

	ps2->pages = NULL;

	ps2->buf = NULL;
	ps2->bufname = NULL;

	/* We empty bbox here, as the real thing has to be filled in ::new */
	ps2->bbox.x0 = ps2->bbox.y0 = 0.0;
	ps2->bbox.x1 = ps2->bbox.y1 = 0.0;
}

static void
gnome_print_ps2_destroy (GtkObject *object)
{
	GnomePrintPs2 *ps2;

	ps2 = GNOME_PRINT_PS2 (object);

	if (ps2->buf) {
		g_warning ("Destroying PS2 context with open buffer");
		if (fclose (ps2->buf)) {
			g_warning ("Error closing buffer");
		}
		ps2->buf = NULL;
		unlink (ps2->bufname);
		g_free (ps2->bufname);
		ps2->bufname = NULL;
	}

	while (ps2->pages) {
		GPPS2Page *p;
		p = ps2->pages;
		if (!p->shown) g_warning ("page %d was not shown", p->number);
		if (p->name) g_free (p->name);
		while (ps2->pages->usedfonts) {
			ps2->pages->usedfonts = g_slist_remove (ps2->pages->usedfonts, ps2->pages->usedfonts->data);
		}
		ps2->pages = p->next;
		g_free (p);
	}

	while (ps2->fonts) {
		GPPS2Font *f;
		f = ps2->fonts;
		if (f->face) gtk_object_unref (GTK_OBJECT (f->face));
		if (f->pso) gnome_font_face_pso_free (f->pso);
		ps2->fonts = f->next;
		g_free (f);
	}
	ps2->selectedfont = NULL;

	ps2->private_color_flag = GP_GC_FLAG_UNSET;

	if (* GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static gint
gnome_print_ps2_gsave (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;

	ps2 = GNOME_PRINT_PS2 (pc);

	if (!ps2->pages || ps2->pages->shown) {
		gint ret;
		ret = gnome_print_beginpage (pc, "Unnamed");
		g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);
	}

	ps2->gsave_level += 1;

	return gp_ps2_fprintf (ps2, "q" EOL);
}

static gint
gnome_print_ps2_grestore (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;

	ps2 = (GnomePrintPs2 *) pc;

	g_return_val_if_fail (ps2->gsave_level > 0, GNOME_PRINT_ERROR_UNKNOWN);
	g_assert (ps2->pages && !ps2->pages->shown);

	ps2->gsave_level -= 1;

	ps2->selectedfont = NULL;
	ps2->private_color_flag = GP_GC_FLAG_UNSET;

	return gp_ps2_fprintf (ps2, "Q" EOL);
}

static gint
gnome_print_ps2_fill (GnomePrintContext *pc, ArtWindRule rule)
{
	GnomePrintPs2 *ps2;
	gint ret;

	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);
	g_return_val_if_fail (gp_path_all_closed (gp_gc_get_currentpath (pc->gc)), GNOME_PRINT_ERROR_BADVALUE);

	ps2 = GNOME_PRINT_PS2 (pc);

	if (!ps2->pages || ps2->pages->shown) {
		ret = gnome_print_beginpage (pc, "Unnamed");
		g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);
	}

	gp_ps2_set_color (pc);

	ret = gp_ps2_print_path (pc, gp_gc_get_currentpath (pc->gc));

	if (rule == ART_WIND_RULE_NONZERO) {
		ret += gp_ps2_fprintf (ps2, "f" EOL);
	} else {
		ret += gp_ps2_fprintf (ps2, "f*" EOL);
	}

	return ret;
}

static gint
gnome_print_ps2_clip (GnomePrintContext *pc, ArtWindRule rule)
{
	GnomePrintPs2 *ps2;
	gint ret;

	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);
	g_return_val_if_fail (gp_path_all_closed (gp_gc_get_currentpath (pc->gc)), GNOME_PRINT_ERROR_BADVALUE);

	ps2 = GNOME_PRINT_PS2 (pc);

	if (!ps2->pages || ps2->pages->shown) {
		ret = gnome_print_beginpage (pc, "Unnamed");
		g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);
	}

	ret = gp_ps2_print_path (pc, gp_gc_get_currentpath (pc->gc));

	if (rule == ART_WIND_RULE_NONZERO) {
		ret += gp_ps2_fprintf (ps2, "W n" EOL);
	} else {
		ret += gp_ps2_fprintf (ps2, "W* n" EOL);
	}

	return ret;
}


static gint
gnome_print_ps2_stroke (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	gint ret;

	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPATH);

	ps2 = GNOME_PRINT_PS2 (pc);

	if (!ps2->pages || ps2->pages->shown) {
		ret = gnome_print_beginpage (pc, "Unnamed");
		g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);
	}

	gp_ps2_set_color (pc);
	gp_ps2_set_line (pc);
	gp_ps2_set_dash (pc);

	ret = gp_ps2_print_path (pc, gp_gc_get_currentpath (pc->gc));

	ret += gp_ps2_fprintf (ps2, "S" EOL);

	return ret;
}

static gint
gnome_print_ps2_glyphlist (GnomePrintContext *pc, GnomeGlyphList *gl)
{
	static gdouble id[] = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
	GnomePrintPs2 *ps2;
	GnomePosGlyphList *pgl;
	const gdouble *ctm;
	const ArtPoint *cp;
	gboolean identity;
	gdouble dx, dy;
	gint ret, s;

	ps2 = (GnomePrintPs2 *) pc;

	g_return_val_if_fail (gp_gc_has_currentpoint (pc->gc), GNOME_PRINT_ERROR_NOCURRENTPOINT);

	if (!ps2->pages || ps2->pages->shown) {
		ret = gnome_print_beginpage (pc, "Unnamed");
		g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);
	}

	ctm = gp_gc_get_ctm (pc->gc);
	cp = gp_gc_get_currentpoint (pc->gc);
	identity = art_affine_equal ((double *) ctm, (double *) id);

	if (!identity) {
		ret = gp_ps2_fprintf (ps2, "q" EOL);
		ret += gp_ps2_fprintf (ps2, "[%g %g %g %g %g %g]cm" EOL, ctm[0], ctm[1], ctm[2], ctm[3], cp->x, cp->y);
		dx = dy = 0.0;
	} else {
		dx = cp->x;
		dy = cp->y;
	}

	pgl = gnome_pgl_from_gl (gl, id, GNOME_PGL_RENDER_DEFAULT);

	for (s = 0; s < pgl->num_strings; s++) {
		GnomePosString * ps;
		gint i;

		ps = pgl->strings + s;

		ret += gp_ps2_set_font_private (pc, gnome_rfont_get_font (ps->rfont));
		g_return_val_if_fail (ps2->selectedfont && ps2->selectedfont->pso, GNOME_PRINT_ERROR_UNKNOWN);
		ret += gp_ps2_set_color_private (pc,
						 ((ps->color >> 24) & 0xff) / 255.0,
						 ((ps->color >> 16) & 0xff) / 255.0,
						 ((ps->color >>  8) & 0xff) / 255.0);
		ret += gp_ps2_fprintf (ps2, "%g %g Tm" EOL, pgl->glyphs[ps->start].x + dx, pgl->glyphs[ps->start].y + dy);

		/* Build string */
		ret += gp_ps2_fprintf (ps2, "(");
		if (ps2->selectedfont->pso->encodedbytes == 1) {
			/* 8-bit encoding */
			for (i = ps->start; i < ps->start + ps->length; i++) {
				gint glyph;
				glyph = pgl->glyphs[i].glyph & 0xff;
				gnome_font_face_pso_mark_glyph (ps2->selectedfont->pso, glyph);
				ret += gp_ps2_fprintf (ps2, "\\%o", glyph);
			}
		} else {
			/* 16-bit encoding */
			for (i = ps->start; i < ps->start + ps->length; i++) {
				gint glyph, page;
				gnome_font_face_pso_mark_glyph (ps2->selectedfont->pso, pgl->glyphs[i].glyph);
				glyph = pgl->glyphs[i].glyph & 0xff;
				page = (pgl->glyphs[i].glyph >> 8) & 0xff;
				ret += gp_ps2_fprintf (ps2, "\\%o\\%o", page, glyph);
			}
		}
		ret += gp_ps2_fprintf (ps2, ")" EOL);

		/* Build array */
		ret += gp_ps2_fprintf (ps2, "[");
		for (i = ps->start + 1; i < ps->start + ps->length; i++) {
			ret += gp_ps2_fprintf (ps2, "%g %g ",
							    pgl->glyphs[i].x - pgl->glyphs[i-1].x,
							    pgl->glyphs[i].y - pgl->glyphs[i-1].y);
		}
		ret += gp_ps2_fprintf (ps2, "0 0] ");

		/* xyshow */
		ret += gp_ps2_fprintf (ps2, "xyshow" EOL);
	}

	if (!identity) {
		ret = gp_ps2_fprintf (ps2, "Q" EOL);
		ps2->selectedfont = NULL;
		ps2->private_color_flag = GP_GC_FLAG_UNSET;
	}

	gnome_pgl_destroy (pgl);

	return 1;
}

static gint
gnome_print_ps2_grayimage (GnomePrintContext *pc, const char *data, gint width, gint height, gint rowstride)
{
	return gnome_print_ps2_image (pc, data, width, height, rowstride, 1);
}

static gint
gnome_print_ps2_rgbimage (GnomePrintContext *pc, const char *data, gint width, gint height, gint rowstride)
{
	return gnome_print_ps2_image (pc, data, width, height, rowstride, 3);
}

static gint
gnome_print_ps2_beginpage (GnomePrintContext *pc, const char *name)
{
	GnomePrintPs2 *ps2;
	GPPS2Page *p;
	gint number;

	ps2 = GNOME_PRINT_PS2 (pc);

	g_return_val_if_fail (!ps2->pages || ps2->pages->shown, GNOME_PRINT_ERROR_UNKNOWN);

	/* fixme: Does frontend check for this? */
	if (!name || !*name) name = "Unnamed";

	number = ps2->pages ? ps2->pages->number : 0;

	p = g_new (GPPS2Page, 1);
	p->next = ps2->pages;
	p->name = g_strdup (name);
	p->number = number + 1;
	p->shown = FALSE;
	p->usedfonts = NULL;

	ps2->pages = p;

	ps2->selectedfont = NULL;

	ps2->private_color_flag = GP_GC_FLAG_UNSET;

	gp_ps2_fprintf (ps2, "%%%%Page: %s %d" EOL, name, p->number);
	gp_ps2_fprintf (ps2, "%%%%PageResources: (atend)" EOL);

	/* Set clip path */
	pc = GNOME_PRINT_CONTEXT (ps2);
	gnome_print_newpath (pc);
	gnome_print_moveto (pc, 0.0, 0.0);
	gnome_print_lineto (pc, ps2->bbox.x1, 0.0);
	gnome_print_lineto (pc, ps2->bbox.x1, ps2->bbox.y1);
	gnome_print_lineto (pc, 0.0, ps2->bbox.y1);
	gnome_print_lineto (pc, 0.0, 0.0);
	gnome_print_clip (pc);
	gnome_print_newpath (pc);


	return GNOME_PRINT_OK;
}


static gint
gnome_print_ps2_showpage (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;

	ps2 = GNOME_PRINT_PS2 (pc);

	if (!ps2->pages || ps2->pages->shown) {
		gint ret;
		g_warning ("Missing beginpage in print job");
		ret = gnome_print_beginpage (pc, "Unnamed");
		g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);
	}

	g_return_val_if_fail (ps2->gsave_level == 0, GNOME_PRINT_ERROR_UNKNOWN);

	if (ps2->pages) ps2->pages->shown = TRUE;

	ps2->selectedfont = NULL;

	ps2->private_color_flag = GP_GC_FLAG_UNSET;

	gp_ps2_fprintf (ps2,"SP" EOL);
	/* PageTrailer */
	gp_ps2_fprintf (ps2,"%%%%PageTrailer" EOL);
	/* PageResources */
	gp_ps2_fprintf (ps2, "%%%%PageResources: procset gnome-print-procs-%s" EOL, VERSION);
	while (ps2->pages->usedfonts) {
		GPPS2Font *font;
		font = (GPPS2Font *) ps2->pages->usedfonts->data;
		gp_ps2_fprintf (ps2, "%%%%+ font %s" EOL, font->pso->encodedname);
		ps2->pages->usedfonts = g_slist_remove (ps2->pages->usedfonts, ps2->pages->usedfonts->data);
	}

	return GNOME_PRINT_OK;
}

static gint
gp_ps2_set_color (GnomePrintContext *pc)
{
	return gp_ps2_set_color_private (pc, gp_gc_get_red (pc->gc), gp_gc_get_green (pc->gc), gp_gc_get_blue (pc->gc));
}

static gint
gp_ps2_set_line (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	gint ret;

	ps2 = GNOME_PRINT_PS2 (pc);

	if (gp_gc_get_line_flag (pc->gc) == GP_GC_FLAG_CLEAR) return 0;

	ret = gp_ps2_fprintf (ps2, "%g w %i J %i j %g M" EOL,
			      gp_gc_get_linewidth (pc->gc),
			      gp_gc_get_linecap (pc->gc),
			      gp_gc_get_linejoin (pc->gc),
			      gp_gc_get_miterlimit (pc->gc));
	gp_gc_set_line_flag (pc->gc, GP_GC_FLAG_CLEAR);

	return ret;
}

static gint
gp_ps2_set_dash (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	const ArtVpathDash *dash;
	gint ret, i;

	ps2 = GNOME_PRINT_PS2 (pc);

	if (gp_gc_get_dash_flag (pc->gc) == GP_GC_FLAG_CLEAR) return 0;

	dash = gp_gc_get_dash (pc->gc);
	ret = gp_ps2_fprintf (ps2, "[");
	for (i = 0; i < dash->n_dash; i++)
		ret += gp_ps2_fprintf (ps2, " %g", dash->dash[i]);
	ret += gp_ps2_fprintf (ps2, "]%g d" EOL, dash->n_dash > 0 ? dash->offset : 0.0);
	gp_gc_set_dash_flag (pc->gc, GP_GC_FLAG_CLEAR);

	return ret;
}

static gint
gp_ps2_set_color_private (GnomePrintContext *pc, gdouble r, gdouble g, gdouble b)
{
	GnomePrintPs2 *ps2;
	gint ret;

	ps2 = (GnomePrintPs2 *) pc;

	if ((ps2->private_color_flag == GP_GC_FLAG_CLEAR) && (r == ps2->r) && (g == ps2->g) && (b == ps2->b)) return 0;

	ret = gp_ps2_fprintf (ps2, "%.3g %.3g %.3g rg" EOL, r, g, b);

	ps2->r = r;
	ps2->g = g;
	ps2->b = b;
	ps2->private_color_flag = GP_GC_FLAG_CLEAR;

	return ret;
}

static gint
gp_ps2_set_font_private (GnomePrintContext *pc, const GnomeFont *font)
{
	GnomePrintPs2 *ps2;
	const GnomeFontFace *face;
	GPPS2Font *f;
	gint ret;
	GSList *l;

	ps2 = (GnomePrintPs2 *) pc;

	if (ps2->selectedfont && (ps2->selectedfont->face == font->face) && (ps2->selectedfont->currentsize = font->size)) return 0;

	face = gnome_font_get_face (font);

	for (f = ps2->fonts; f != NULL; f = f->next) {
		if (f->face == face) break;
	}
	if (!f) {
		/* No entry, so create one */
		f = g_new (GPPS2Font, 1);
		f->next = ps2->fonts;
		ps2->fonts = f;
		f->face = (GnomeFontFace *) face;
		gnome_font_face_ref (face);
		f->pso = gnome_font_face_pso_new ((GnomeFontFace *) face, NULL);
		g_return_val_if_fail (f->pso != NULL, GNOME_PRINT_ERROR_UNKNOWN);
	}

	for (l = ps2->pages->usedfonts; l != NULL; l = l->next) {
		if ((GPPS2Font *) l->data == f) break;
	}
	if (!l) {
		ps2->pages->usedfonts = g_slist_prepend (ps2->pages->usedfonts, f);
	}

	ret = gp_ps2_fprintf (ps2, "/%s FF %g F" EOL, f->pso->encodedname, gnome_font_get_size (font));

	f->currentsize = font->size;
	ps2->selectedfont = f;

	return ret;
}

GnomePrintPs2 *
gnome_print_ps2_new (GnomePrinter *printer, const char *paper_name)
{
	GnomePrintPs2 *ps2;
	const GnomePaper *paper;
	gint fd;

	g_return_val_if_fail (printer != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINTER (printer), NULL);
	
	ps2 = gtk_type_new (gnome_print_ps2_get_type ());

#ifdef ENABLE_LIBGPA
	g_return_val_if_fail (GPA_IS_PRINTER (printer->gpa_printer), NULL);
	/* FIXME: We take the first settings for now */
	g_return_val_if_fail (GPA_IS_SETTINGS (printer->gpa_settings), NULL);
	
	ps2->gpa_printer = printer->gpa_printer;
	ps2->gpa_settings = printer->gpa_settings;

	/* We don't need to ref the printer, because the settings should
	 * ref it (and unref it) but I don't think libgpa settings is
	 * refing the printer now. Just be safe now, but FIX gpa-settings
	 * so that it refs the printer. Chema
	 */
	gpa_printer_ref (printer->gpa_printer);
	gpa_settings_ref (printer->gpa_settings);

	ps2->ps_level = gnome_print_ps2_get_level (ps2);
	g_print ("PSLEVEL IS %i\n", ps2->ps_level);
	
#endif

	ps2->bufname = g_strdup ("/tmp/gnome-print-XXXXXX");
	fd = mkstemp (ps2->bufname);
	if (fd < 0) {
		g_warning ("Cannot create temporary file");
		g_free (ps2->bufname);
		ps2->bufname = NULL;
		gtk_object_unref (GTK_OBJECT (ps2));
		return NULL;
	}
	ps2->buf = fdopen (fd, "r+");

	if (!gnome_print_context_open_file (GNOME_PRINT_CONTEXT (ps2), printer->filename)) goto failure;

	paper = gnome_paper_with_name (paper_name);

	/* Fill fake bbox */
	ps2->bbox.x0 = ps2->bbox.y0 = 0.0;
	ps2->bbox.x1 = paper ? gnome_paper_pswidth (paper) : 21 * 72 / 2.54;
	ps2->bbox.y1 = paper ? gnome_paper_psheight (paper) : 29.7 * 72 / 2.54;

	return ps2;

 failure:
	g_warning ("gnome_print_ps2_new: ps2 new failure ..\n");
	gtk_object_unref (GTK_OBJECT (ps2));
	return NULL;
}

static gint
gp_ps2_print_path (GnomePrintContext *pc, const GPPath *gppath)
{
	GnomePrintPs2 *ps2;
	const ArtBpath *path;
	gboolean started, closed;
	
	ps2 = GNOME_PRINT_PS2 (pc);

	path = gp_path_bpath (gppath);
	
	started = FALSE;
	closed = FALSE;
	for (; path->code != ART_END; path++)
		switch (path->code) {
		case ART_MOVETO_OPEN:
			if (started && closed) gp_ps2_fprintf (ps2, "h" EOL);
			closed = FALSE;
			started = FALSE;
			gp_ps2_fprintf (ps2, "%g %g m" EOL, path->x3, path->y3);
			break;
		case ART_MOVETO:
			if (started && closed) gp_ps2_fprintf (ps2, "h" EOL);
			closed = TRUE;
			started = TRUE;
			gp_ps2_fprintf (ps2, "%g %g m" EOL, path->x3, path->y3);
			break;
		case ART_LINETO:
			gp_ps2_fprintf (ps2, "%g %g l" EOL, path->x3, path->y3);
			break;
		case ART_CURVETO:
			gp_ps2_fprintf (ps2, "%g %g %g %g %g %g c" EOL,
						     path->x1, path->y1,
						     path->x2, path->y2,
						     path->x3, path->y3);
			break;
		default:
			g_warning ("Path structure is corrupted");
			return -1;
		}

	if (started && closed) gp_ps2_fprintf (ps2, "h" EOL);
	
	return 0;
}

static int
gnome_print_ps2_image (GnomePrintContext *pc, const char *px, int w, int h, int rowstride, int ch)
{
	GnomePrintPs2 *ps2;
	const gdouble *ctm;
	gint ret, r, c;

#if 0
	gchar *hex_data;
	gint data_size, data_size_real;
#endif

	ps2 = GNOME_PRINT_PS2 (pc);

	if (!ps2->pages || ps2->pages->shown) {
		ret = gnome_print_beginpage (pc, "Unnamed");
		g_return_val_if_fail (ret == GNOME_PRINT_OK, ret);
	}

	ctm = gp_gc_get_ctm (pc->gc);
	gp_ps2_fprintf (ps2, "q" EOL);
	gp_ps2_fprintf (ps2, "[%g %g %g %g %g %g]cm" EOL, ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]);

	/*
        * MRS - Optimize image rendering using the best available image command.
	*       This means using ASCII 85 encoding instead of HEX encoding
	*       for Level 2 and 3 printers...
	*/

#if 0
        if (ps2->ps_level == 1);
#else
	if (TRUE) {
#endif
		/* Level 1 image commands */
			gchar *hex;
		gint hex_size;

		ret = gp_ps2_fprintf (ps2, "/buf %d string def" EOL "%d %d 8" EOL, w * ch, w, h);
		ret += gp_ps2_fprintf (ps2, "[%d 0 0 %d 0 %d]" EOL, w, -h, h);
		ret += gp_ps2_fprintf (ps2, "{ currentfile buf readhexstring pop }" EOL);

		if (ch == 1) {
			ret += gp_ps2_fprintf (ps2, "image" EOL);
		} else {
			ret += gp_ps2_fprintf (ps2, "false %d colorimage" EOL, ch);
		}

		hex = g_new (gchar, gnome_print_encode_hex_wcs (w * ch));

	       /*
	        * MRS - Keep lines < 255 characters long for DSC conformance. Since
		*       images will likely be wider than 127 grayscale pixels, we
		*       need to do this...  We wrap every 80 characters to make it
		*       easier to "more" the files if we have to...
		*/

		for (r = 0; r < h; r++) {
			hex_size = gnome_print_encode_hex (px + r * rowstride, hex, w * ch);
			fwrite (hex, sizeof (gchar), hex_size, ps2->buf);
			gp_ps2_fprintf (ps2, EOL);
		}

		g_free (hex);
	} else {
		/* Level 2/3 image commands */
		gchar *a85;
		gint a85_size;

		ret += gp_ps2_fprintf (ps2, "[%d 0 0 %d 0 %d]" EOL, w, -h, h);
		ret += gp_ps2_fprintf (ps2, "{ currentfile buf reada85string pop }" EOL);

		if (ch == 1)
			ret = gp_ps2_fprintf (ps2, "/DeviceGray setcolorspace" EOL);
		else
			ret = gp_ps2_fprintf (ps2, "/DeviceRGB setcolorspace" EOL);

		ret += gp_ps2_fprintf (ps2, "<<"
							"/ImageType 1"
							"/Width %d"
							"/Height %d"
							"/BitsPerComponent 8"
							"/ImageMatrix[%d 0 0 %d 0 %d]"
							"/Decode[%s]"
							"/Interpolate true"
							"/DataSource currentfile/ASCII85Decode filter"
							">>image" EOL,
						    w, h,
						    w, -h, h,
						    ch == 1 ? "0 1" : "0 1 0 1 0 1");

		a85 = g_new (gchar, gnome_print_encode_ascii85_wcs (w * ch));

	       /*
	        * MRS - Keep lines < 255 characters long for DSC conformance. Since
		*       images will likely be wider than ~200 grayscale pixels, we
		*       need to do this...  We wrap every 80 characters to make it
		*       easier to "more" the files if we have to...
		*/

		for (r = 0; r < h; r++) {
			a85_size = gnome_print_encode_ascii85 (px + r * rowstride, a85, w * ch);

			for (c = 0; c < a85_size; c += 80) {
				if ((c + 80) < a85_size)
					fwrite (a85 + c, sizeof (gchar), 80, ps2->buf);
				else
					fwrite (a85 + c, sizeof (gchar), a85_size - c, ps2->buf);

				gp_ps2_fprintf (ps2, EOL);
			}
		}

		/* End the ASCII 85 stream with "~>"... */
                gp_ps2_fprintf (ps2, "~>" EOL);

		g_free (a85);
	}

	gp_ps2_fprintf (ps2, "Q" EOL);

	return 0;
#if 0
	/* Image commands */
	ret = gp_ps2_fprintf (ps2, "/buf %d string def" EOL "%d %d 8" EOL, width * bytes_per_pixel, width, height);

	ret += gp_ps2_fprintf (ps2, "[%d 0 0 %d 0 %d]" EOL, width, -height, height);

	ret += gp_ps2_fprintf (ps2, "{ currentfile buf readhexstring pop }\n");

	if (bytes_per_pixel == 1) {
		ret += gp_ps2_fprintf (ps2, "image" EOL);
	} else if (bytes_per_pixel == 3) {
		ret += gp_ps2_fprintf (ps2, "false %d colorimage" EOL, bytes_per_pixel);
	}

	data_size = width * bytes_per_pixel;
	hex_data = g_new (gchar, gnome_print_encode_hex_wcs (data_size));

	for (r = 0; r < height; r++) {
		data_size_real = gnome_print_encode_hex (data + r * rowstride, hex_data, data_size);
		gnome_print_context_write_file (pc, hex_data, data_size_real);
		gp_ps2_fprintf (ps2, EOL);
	}

	g_free (hex_data);

	return 0;
#endif
}


/* Other stuff */

static gchar*
gnome_print_ps2_get_date (void)
{
	time_t clock;
	struct tm *now;
	gchar *date;

	clock = time (NULL);
	now = localtime (&clock);

	date = g_strdup_printf ("D:%04d%02d%02d%02d%02d%02d",
				now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

	return date;
}

static gint
gp_ps2_fprintf (GnomePrintPs2 *ps2, const char *format, ...)
{
	va_list arguments;
	gchar *text;
	gchar *oldlocale;
	gint len;
	
	oldlocale = setlocale (LC_NUMERIC, NULL);
	setlocale (LC_NUMERIC, "C");
		
	va_start (arguments, format);
	text = g_strdup_vprintf (format, arguments);
	va_end (arguments);
	len = fwrite (text, sizeof (gchar), strlen (text), ps2->buf);
	g_free (text);

	setlocale (LC_NUMERIC, oldlocale);

	return len;
}

#ifdef ENABLE_LIBGPA
static gint
gnome_print_ps2_get_level (GnomePrintPs2 *ps2)
{
	const gchar *temp;

	g_return_val_if_fail (GNOME_IS_PRINT_PS2 (ps2), 2);

	
	temp = gpa_settings_query_options (ps2->gpa_settings,
					   "PsLevel");

	if (temp == NULL)
		return 2;
	/* FIXME: for printers other than generic Postscript
		 we get the PsLevel from the values hash, not from
		 a selected option. Just keep this in mind */
	if (strcmp (temp, "PsLevel1") == 0)
		return 1;
	else if (strcmp (temp, "PsLevel2") == 0)
		return 2;
	else if (strcmp (temp, "PsLevel3") == 0)
		return 3;
	else
		g_warning ("Invalid PS Level %s", temp);
	
	return 2;
}
#endif

static gint
gnome_print_ps2_close (GnomePrintContext *pc)
{
	GnomePrintPs2 *ps2;
	GPPS2Font *f;
	gchar *date;
	guchar b[256];
	gint len;

	ps2 = GNOME_PRINT_PS2 (pc);

	g_return_val_if_fail (ps2->buf != NULL, GNOME_PRINT_ERROR_UNKNOWN);

	if (!ps2->pages || !ps2->pages->shown) {
		g_warning ("Closing PS2 Context without final showpage");
		gnome_print_showpage (pc);
	}

	/* Do header */
	/* Comments */
	date = gnome_print_ps2_get_date ();
	gnome_print_context_fprintf (pc, "%%!PS-Adobe-3.0" EOL);
	/* fixme: %%BoundingBox: */
	gnome_print_context_fprintf (pc, "%%%%Creator: Gnome Print Version %s" EOL, VERSION);
	gnome_print_context_fprintf (pc, "%%%%CreationDate: %s" EOL, date);
	/* fixme: %%DocumentData: */
	/* fixme: Should we use %%Extensions: and drop LanguageLevel? */
	gnome_print_context_fprintf (pc, "%%%%LanguageLevel: 2" EOL);
	gnome_print_context_fprintf (pc, "%%%%Pages: %d" EOL, ps2->pages ? ps2->pages->number : 0);
	gnome_print_context_fprintf (pc, "%%%%BoundingBox: %d %d %d %d" EOL,
				     (gint) floor (ps2->bbox.x0),
				     (gint) floor (ps2->bbox.y0),
				     (gint) ceil (ps2->bbox.x1),
				     (gint) ceil (ps2->bbox.y1));
	/* fixme: Orientation: */
	gnome_print_context_fprintf (pc, "%%%%PageOrder: Ascend" EOL);
	gnome_print_context_fprintf (pc, "%%%%Title: %s" EOL, "Document Title goes here");
	gnome_print_context_fprintf (pc, "%%%%DocumentSuppliedResources: procset gnome-print-procs-%s" EOL, VERSION);
	if (ps2->fonts) {
		/* %%DocumentSuppliedResources: */
		for (f = ps2->fonts; f != NULL; f = f->next) {
			gnome_print_context_fprintf (pc, "%%%%+ font %s" EOL, f->pso->encodedname);
		}
	}
	g_free (date);
	gnome_print_context_fprintf (pc, "%%%%EndComments" EOL);
	/* Prolog */
	gnome_print_context_fprintf (pc, "%%%%BeginProlog" EOL);
	/* Abbreviations */
	gnome_print_context_fprintf (pc, "%%%%BeginResource: procset gnome-print-procs-%s" EOL, VERSION);
	gnome_print_context_fprintf (pc, "/|/def load def/,/load load" EOL);
	gnome_print_context_fprintf (pc, "|/n/newpath , |/m/moveto , |/l/lineto , |/c/curveto ," EOL);
	gnome_print_context_fprintf (pc, "|/q/gsave , |/Q/grestore , |/rg/setrgbcolor , |/J/setlinecap ," EOL);
	gnome_print_context_fprintf (pc, "|/j/setlinejoin , |/w/setlinewidth , |/M/setmiterlimit ," EOL);
	gnome_print_context_fprintf (pc, "|/d/setdash , |/i/pop , |/W/clip , |/W*/eoclip , |/n/newpath ," EOL);
	gnome_print_context_fprintf (pc, "|/S/stroke , |/f/fill , |/f*/eofill , |/Tj/show , |/Tm/moveto ," EOL);
	gnome_print_context_fprintf (pc, "|/FF/findfont ," EOL);
	gnome_print_context_fprintf (pc, "|/h/closepath , |/cm/concat , |/rm/rmoveto , |/sp/strokepath ," EOL);
	gnome_print_context_fprintf (pc, "|/SP/showpage , |/p/pop , |/EX/exch , |/DF/definefont , |" EOL);
	gnome_print_context_fprintf (pc, "/F {scalefont setfont} def" EOL);
	gnome_print_context_fprintf (pc, "%%%%EndResource" EOL);
	gnome_print_context_fprintf (pc, "%%%%EndProlog" EOL);

	/* Prolog */
	gnome_print_context_fprintf (pc, "%%%%BeginSetup" EOL);
	/* Download fonts */
	for (f = ps2->fonts; f != NULL; f = f->next) {
		gnome_font_face_pso_ensure_buffer (f->pso);
		gnome_print_context_fprintf (pc, "%%%%BeginResource: font %s" EOL, f->pso->encodedname);
		gnome_print_context_write_file (pc, f->pso->buf, f->pso->length);
		gnome_print_context_fprintf (pc, "%%%%EndResource" EOL);
	}
	gnome_print_context_fprintf (pc, "%%%%EndSetup" EOL);
	
	/* Write buffer */
	rewind (ps2->buf);
	len = 256;
	while (len > 0) {
		len = fread (b, 1, 256, ps2->buf);
		if (len > 0) {
			gnome_print_context_write_file (pc, b, len);
		}
	}
	fclose (ps2->buf);
	ps2->buf = NULL;
	unlink (ps2->bufname);
	g_free (ps2->bufname);
	ps2->bufname = NULL;

	gnome_print_context_fprintf (pc, "%%%%Trailer" EOL);
	gnome_print_context_fprintf (pc, "%%%%EOF" EOL);

	gnome_print_context_close_file (pc);
	
	return GNOME_PRINT_OK;
}

