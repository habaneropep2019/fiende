/*
 * gnome-print-rbuf.c: A rasterizer for GnomePrint
 *
 * Author:
 *   Lauris Kaplinski <lauris@ariman.ee>
 *
 * Copyright (C) Lauris Kaplinski, 2000
 *
 * TODO
 *
 * - figure out, how is path & currentpoint handled during fill, clip, show...
 * - implement new libart rendering/clipping, when available
 * - glyph outline cache (should be in gnome-font)
 * - are dashes in device or current coordinates?
 *
 */

#include <config.h>
#include <math.h>
#include <string.h>
#include <gtk/gtk.h>

#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_vpath_stroke.h>
#include <libart_lgpl/art_svp_ops.h>
#include <libart_lgpl/art_rect_svp.h>
#include <libart_lgpl/art_vpath_dash.h>
#include <libart_lgpl/art_vpath_svp.h>
#include <libart_lgpl/art_rgb_svp.h>
#include <libart_lgpl/art_rgb_rgba_affine.h>

#include <libgnomeprint/gp-gc.h>
#include "art_rgba_svp.h"
#include "art_rgba_rgba_affine.h"

#include <libgnomeprint/gp-unicode.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-print-rbuf.h>
#include <libgnomeprint/gnome-print-rbuf-private.h>
#include <libgnomeprint/gnome-font.h>

/*
 * Private structures
 */

struct _GnomePrintRBufPrivate {
	guchar * pixels;
	gint width;
	gint height;
	gint rowstride;

	gdouble page2buf[6];
	guint32 alpha : 1;
};

static void gpb_class_init (GnomePrintRBufClass *class);
static void gpb_init (GnomePrintRBuf *rbuf);
static void gpb_destroy (GtkObject *object);

static gint gpb_fill (GnomePrintContext * pc, ArtWindRule rule);
static gint gpb_stroke (GnomePrintContext * pc);
static gint gpb_show_sized (GnomePrintContext * pc, const char *text, int bytes);

static gint gpb_grayimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride);
static gint gpb_rgbimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride);
static gint gpb_rgbaimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride);

static gint gpb_clip (GnomePrintContext * pc, ArtWindRule rule);

static gint gpb_textline (GnomePrintContext * pc, GnomeTextLine * textline);

static gint gpb_beginpage (GnomePrintContext * pc, const gchar * name);
static gint gpb_showpage (GnomePrintContext * pc);
static gint gpb_close (GnomePrintContext * pc);

static void gp_svp_uncross_to_render (GnomePrintContext * pc, const ArtSVP * svp, ArtWindRule rule);
static void gp_vpath_to_render (GnomePrintContext * pc, const ArtBpath * bpath, ArtWindRule rule);

static void gp_render_silly_rgba (GnomePrintContext * pc, const guchar * pixels, gint width, gint height, gint rowstride);

static GnomePrintContextClass * print_rbuf_parent_class;

/**
 * gnome_print_rbuf_get_type:
 *
 * GTK type identification routine for #GnomePrintRBuf
 *
 * Returns: The Gtk type for the #GnomePrintRBuf object
 */

GtkType
gnome_print_rbuf_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnomePrintRBuf",
			sizeof (GnomePrintRBuf),
			sizeof (GnomePrintRBufClass),
			(GtkClassInitFunc) gpb_class_init,
			(GtkObjectInitFunc) gpb_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		type = gtk_type_unique (gnome_print_context_get_type (), &info);
	}
	return type;
}

static void
gpb_class_init (GnomePrintRBufClass *class)
{
	GtkObjectClass * object_class;
	GnomePrintContextClass * pc_class;

	object_class = (GtkObjectClass *) class;
	pc_class = (GnomePrintContextClass *) class;

	print_rbuf_parent_class = gtk_type_class (gnome_print_context_get_type ());

	object_class->destroy = gpb_destroy;
	
	pc_class->fill = gpb_fill;
	pc_class->stroke = gpb_stroke;
	pc_class->show_sized = gpb_show_sized;
	pc_class->grayimage = gpb_grayimage;
	pc_class->rgbimage = gpb_rgbimage;
	pc_class->rgbaimage = gpb_rgbaimage;

	pc_class->clip = gpb_clip;

	pc_class->textline = gpb_textline;
	pc_class->showpage = gpb_showpage;

	pc_class->beginpage = gpb_beginpage;

	pc_class->close = gpb_close;
}

static void
gpb_init (GnomePrintRBuf * rbuf)
{
	rbuf->private = g_new (GnomePrintRBufPrivate, 1);
	g_assert (rbuf->private != NULL);

	rbuf->private->pixels = NULL;
	art_affine_identity (rbuf->private->page2buf);
#if 0
	rbuf->private->gc = NULL;
#endif
}

static void
gpb_destroy (GtkObject *object)
{
	GnomePrintRBuf * rbuf;

	rbuf = GNOME_PRINT_RBUF (object);

	if (rbuf->private) {
		g_free (rbuf->private);
	}

	if (GTK_OBJECT_CLASS (print_rbuf_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (print_rbuf_parent_class)->destroy) (object);
}

/*
 * Drawing methods
 */

static gint
gpb_fill (GnomePrintContext * pc, ArtWindRule rule)
{
	GnomePrintRBuf * rbuf;
	GnomePrintRBufPrivate * rbp;
	const GPPath * gppath;
	ArtBpath * bpath;

	g_return_val_if_fail (pc != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_RBUF (pc), -1);

	rbuf = GNOME_PRINT_RBUF (pc);
	rbp = rbuf->private;

	g_return_val_if_fail (gp_gc_has_currentpath (pc->gc), -1);

	gppath = gp_gc_get_currentpath (pc->gc);
	g_assert (gppath != NULL);

	bpath = gp_path_bpath (gppath);
	g_assert (bpath != NULL);

	gp_vpath_to_render (pc, bpath, rule);

	return 1;
}

/* fixme: use real pen transforming */

static gint
gpb_stroke (GnomePrintContext * pc)
{
	GnomePrintRBuf * rbuf;
	GnomePrintRBufPrivate * rbp;
	const GPPath * gppath;
	ArtBpath * bpath;
	ArtVpath * vpath, * pvpath;
	ArtSVP * svp;
	const ArtVpathDash * dash;
	gdouble linewidth;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_RBUF (pc), 0);

	rbuf = GNOME_PRINT_RBUF (pc);
	rbp = rbuf->private;

	g_return_val_if_fail (gp_gc_currentpath_points (pc->gc) > 1, -1);

	gppath = gp_gc_get_currentpath (pc->gc);
	g_assert (gppath != NULL);

	bpath = gp_path_bpath (gppath);
	g_assert (bpath != NULL);

	vpath = art_bez_path_to_vec (bpath, 0.25);
	g_assert (vpath != NULL);

	pvpath = art_vpath_perturb (vpath);
	art_free (vpath);

	dash = gp_gc_get_dash (pc->gc);

	if ((dash->n_dash > 0) && (dash->dash != NULL)) {
		ArtVpath * dvp;
		dvp = art_vpath_dash (pvpath, dash);
		g_assert (dvp != NULL);
		art_free (pvpath);
		pvpath = dvp;
	}

	/* fixme */

	linewidth = gp_gc_get_linewidth (pc->gc);

	svp = art_svp_vpath_stroke (pvpath,
		gp_gc_get_linejoin (pc->gc),
		gp_gc_get_linecap (pc->gc),
		linewidth,
		gp_gc_get_miterlimit (pc->gc),
		0.25);
	g_assert (svp != NULL);
	art_free (pvpath);

	gp_svp_uncross_to_render (pc, svp, ART_WIND_RULE_NONZERO);

	art_svp_free (svp);

	return 1;
}

/* fixme: */

static gint
gpb_show_sized (GnomePrintContext * pc, const char *text, int bytes)
{
	GnomePrintRBuf * rbuf;
	GnomePrintRBufPrivate * rbp;
	const GnomeFont * font;
	const GnomeFontFace * face;
	gdouble size;
	const ArtBpath * bpath;
	ArtPoint advance;
	gint code;
	const ArtPoint * p;
	gdouble affine[6];
	const gdouble * ctm;
	gdouble x, y;
	const char *tptr;
	ArtBpath * abp;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_RBUF (pc), 0);

	rbuf = GNOME_PRINT_RBUF (pc);
	rbp = rbuf->private;

	font = gp_gc_get_font (pc->gc);
	face = gnome_font_get_face ((GnomeFont *) font);
	size = gnome_font_get_size ((GnomeFont *) font);

	p = gp_gc_get_currentpoint (pc->gc);
	g_return_val_if_fail (p != NULL, -1);

	ctm = gp_gc_get_ctm (pc->gc);

	x = y = 0.0;

	for (tptr = text; tptr && tptr < (text + bytes); tptr = g_utf8_next_char (tptr)) {
		code = gnome_font_face_lookup_default ((GnomeFontFace *) face, g_utf8_get_char (tptr));
		bpath = gnome_font_face_get_glyph_stdoutline ((GnomeFontFace *) face, code);

		art_affine_scale (affine,
			size / 1000.0,
			size / 1000.0);
		affine[4] = x;
		affine[5] = y;
		art_affine_multiply (affine, affine, ctm);
		affine[4] = affine[4] - ctm[4] + p->x;
		affine[5] = affine[5] - ctm[5] + p->y;

		abp = art_bpath_affine_transform (bpath, affine);

		gp_vpath_to_render (pc, abp, ART_WIND_RULE_NONZERO);

		gnome_font_face_get_glyph_stdadvance ((GnomeFontFace *) face, code, &advance);
		x += advance.x * size / 1000.0;
	}

	return 1;
}

/* fixme: clipping */

static gint
gpb_grayimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride)
{
	GnomePrintRBuf * rbuf;
	GnomePrintRBufPrivate * rbp;
	art_u8 * ib, * ipd;
	const art_u8 * ips;
	gint x, y;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_RBUF (pc), 0);
	g_return_val_if_fail (data != NULL, 0);
	g_return_val_if_fail (width > 0, 0);
	g_return_val_if_fail (height > 0, 0);

	rbuf = GNOME_PRINT_RBUF (pc);
	rbp = rbuf->private;

	ib = g_new (art_u8, width * height * 4);

	for (y = 0; y < height; y++) {
		ips = data + y * rowstride;
		ipd = ib + y * width * 4;
		for (x = 0; x < width; x++) {
			* ipd++ = * ips;
			* ipd++ = * ips;
			* ipd++ = * ips++;
			* ipd++ = 0xff;
		}
	}

	gp_render_silly_rgba (pc, ib, width, height, rowstride);

	g_free (ib);

	return 1;
}

static gint
gpb_rgbimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride)
{
	GnomePrintRBuf * rbuf;
	GnomePrintRBufPrivate * rbp;
	art_u8 * ib, * ipd;
	const art_u8 * ips;
	gint x, y;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_RBUF (pc), 0);
	g_return_val_if_fail (data != NULL, 0);
	g_return_val_if_fail (width > 0, 0);
	g_return_val_if_fail (height > 0, 0);

	rbuf = GNOME_PRINT_RBUF (pc);
	rbp = rbuf->private;

	ib = g_new (art_u8, width * height * 4);

	for (y = 0; y < height; y++) {
		ips = data + y * rowstride;
		ipd = ib + y * width * 4;
		for (x = 0; x < width; x++) {
			* ipd++ = * ips++;
			* ipd++ = * ips++;
			* ipd++ = * ips++;
			* ipd++ = 0xff;
		}
	}

	gp_render_silly_rgba (pc, ib, width, height, width * 4);

	g_free (ib);

	return 1;
}

/* fixme: clipping */

static gint
gpb_rgbaimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride)
{
	GnomePrintRBuf * rbuf;
	GnomePrintRBufPrivate * rbp;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_RBUF (pc), 0);
	g_return_val_if_fail (data != NULL, 0);
	g_return_val_if_fail (width > 0, 0);
	g_return_val_if_fail (height > 0, 0);

	rbuf = GNOME_PRINT_RBUF (pc);
	rbp = rbuf->private;

	gp_render_silly_rgba (pc, data, width, height, rowstride);

	return 1;
}

/* Clipping */

static gint
gpb_clip (GnomePrintContext * pc, ArtWindRule rule)
{
	GnomePrintRBuf * rbuf;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_RBUF (pc), 0);

	rbuf = GNOME_PRINT_RBUF (pc);

	if (rule == ART_WIND_RULE_NONZERO) {
		gp_gc_clip (pc->gc);
	} else {
		gp_gc_eoclip (pc->gc);
	}

	return 1;
}

/* Misc */

static gint
gpb_showpage (GnomePrintContext * pc)
{
	GnomePrintRBuf * rbuf;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_RBUF (pc), 0);

	rbuf = GNOME_PRINT_RBUF (pc);

	return 1;
}

static gint
gpb_close (GnomePrintContext * pc)
{
	return 1;
}

static gint
gpb_textline (GnomePrintContext * pc, GnomeTextLine * textline)
{
	g_warning ("GnomePrintRBuf::textline is not implemented");

	return 1;
}

/*
 * Beginpage
 *
 * fixme: Currently we simply clear rbuffer
 *
 * showpage, close - do nothing
 */

static gint
gpb_beginpage (GnomePrintContext * pc, const gchar * name)
{
	GnomePrintRBuf * rbuf;
	GnomePrintRBufPrivate * rbp;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_RBUF (pc), 0);

	rbuf = GNOME_PRINT_RBUF (pc);
	rbp = rbuf->private;

	gnome_print_concat (pc, rbuf->private->page2buf);

	return 1;
}

/* Constructors */

GnomePrintContext *
gnome_print_rbuf_new (guchar * pixels,
	gint width,
	gint height,
	gint rowstride,
	gdouble page2buf[6],
	gboolean alpha)
	
{
	GnomePrintRBuf * rbuf;

	g_return_val_if_fail (pixels != NULL, NULL);
	g_return_val_if_fail (width > 0, NULL);
	g_return_val_if_fail (height > 0, NULL);
	g_return_val_if_fail (rowstride >= 3 * width, NULL);
	g_return_val_if_fail (page2buf != NULL, NULL);

	rbuf = gtk_type_new (GNOME_TYPE_PRINT_RBUF);

	if (!gnome_print_rbuf_construct (rbuf, pixels, width, height, rowstride, page2buf, alpha)) {
		gtk_object_unref (GTK_OBJECT (rbuf));
	}

	return GNOME_PRINT_CONTEXT (rbuf);
}

GnomePrintRBuf *
gnome_print_rbuf_construct (GnomePrintRBuf * rbuf,
	guchar * pixels,
	gint width,
	gint height,
	gint rowstride,
	gdouble page2buf[6],
	gboolean alpha)
{
	g_return_val_if_fail (rbuf != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_RBUF (rbuf), NULL);
	g_return_val_if_fail (pixels != NULL, NULL);
	g_return_val_if_fail (width > 0, NULL);
	g_return_val_if_fail (height > 0, NULL);
	g_return_val_if_fail (rowstride >= 3 * width, NULL);
	g_return_val_if_fail (page2buf != NULL, NULL);

	g_assert (rbuf->private != NULL);

	rbuf->private->pixels = pixels;
	rbuf->private->width = width;
	rbuf->private->height = height;
	rbuf->private->rowstride = rowstride;
	rbuf->private->alpha = alpha;

	memcpy (rbuf->private->page2buf, page2buf, sizeof (gdouble) * 6);

	return rbuf;
}

/* Private helpers */

static void
gp_svp_uncross_to_render (GnomePrintContext * pc, const ArtSVP * svp, ArtWindRule rule)
{
	GnomePrintRBufPrivate * rbp;
	ArtSVP * svp1, * svp2;

	g_assert (pc != NULL);
	g_assert (svp != NULL);

	rbp = GNOME_PRINT_RBUF (pc)->private;

	svp2 = art_svp_uncross ((ArtSVP *) svp);
	g_assert (svp2 != NULL);

	svp1 = art_svp_rewind_uncrossed (svp2, rule);
	g_assert (svp1 != NULL);
	art_svp_free (svp2);

	if (gp_gc_has_clipsvp (pc->gc)) {
		svp2 = art_svp_intersect (svp1, gp_gc_get_clipsvp (pc->gc));
		g_assert (svp2 != NULL);
		art_svp_free (svp1);
		svp1 = svp2;
	}

	if (rbp->alpha) {
		art_rgba_svp_alpha (svp1,
			0, 0, rbp->width, rbp->height,
			gp_gc_get_rgba (pc->gc),
			rbp->pixels, rbp->rowstride,
			NULL);
	} else {
		art_rgb_svp_alpha (svp1,
			0, 0, rbp->width, rbp->height,
			gp_gc_get_rgba (pc->gc),
			rbp->pixels, rbp->rowstride,
			NULL);
	}

	art_svp_free (svp1);
}

static void
gp_vpath_to_render (GnomePrintContext * pc, const ArtBpath * bpath, ArtWindRule rule)
{
	GnomePrintRBufPrivate * rbp;
	ArtVpath * vpath1, * vpath2;
	ArtSVP * svp;

	g_assert (pc != NULL);
	g_assert (bpath != NULL);

	rbp = GNOME_PRINT_RBUF (pc)->private;

	vpath1 = art_bez_path_to_vec (bpath, 0.25);
	g_assert (vpath1 != NULL);

	vpath2 = art_vpath_perturb (vpath1);
	g_assert (vpath2 != NULL);
	art_free (vpath1);

	svp = art_svp_from_vpath (vpath2);
	g_assert (svp != NULL);
	art_free (vpath2);

	gp_svp_uncross_to_render (pc, svp, rule);

	art_svp_free (svp);
}

static void
gp_render_silly_rgba (GnomePrintContext * pc,
	const guchar * pixels, gint width, gint height, gint rowstride)
{
	GnomePrintRBufPrivate * rbp;
	gdouble affine[6];
	ArtVpath vp[6];
	ArtVpath * vpath, * pvpath;
	ArtSVP * svp1, * svp2;
	ArtDRect bbox, pbox;
	ArtIRect ibox;
	gdouble ba[6];
	guchar * cbuf, * ibuf;
	guchar * p, * ip, * cp;
	gint bw, bh, x, y;

	rbp = GNOME_PRINT_RBUF (pc)->private;

	art_affine_scale (affine, 1.0 / width, -1.0 / height);
	affine[5] = 1.0;
	art_affine_multiply (affine, affine, gp_gc_get_ctm (pc->gc));

	vp[0].code = ART_MOVETO;
	vp[0].x = 0.0;
	vp[0].y = 0.0;
	vp[1].code = ART_LINETO;
	vp[1].x = width;
	vp[1].y = 0.0;
	vp[2].code = ART_LINETO;
	vp[2].x = width;
	vp[2].y = height;
	vp[3].code = ART_LINETO;
	vp[3].x = 0.0;
	vp[3].y = height;
	vp[4].code = ART_LINETO;
	vp[4].x = 0.0;
	vp[4].y = 0.0;
	vp[5].code = ART_END;

	vpath = art_vpath_affine_transform (vp, affine);

	pvpath = art_vpath_perturb (vpath);
	art_free (vpath);

	svp1 = art_svp_from_vpath (pvpath);
	art_free (pvpath);

	svp2 = art_svp_uncross (svp1);
	art_svp_free (svp1);

	svp1 = art_svp_rewind_uncrossed (svp2, ART_WIND_RULE_NONZERO);
	art_svp_free (svp2);

	if (gp_gc_has_clipsvp (pc->gc)) {
		svp2 = art_svp_intersect (svp1, gp_gc_get_clipsvp (pc->gc));
		art_svp_free (svp1);
		svp1 = svp2;
	}

	art_drect_svp (&bbox, svp1);

	pbox.x0 = pbox.y0 = 0.0;
	pbox.x1 = rbp->width;
	pbox.y1 = rbp->height;

	art_drect_intersect (&bbox, &bbox, &pbox);

	if (art_drect_empty (&bbox)) {
		art_svp_free (svp1);
		return;
	}

	art_drect_to_irect (&ibox, &bbox);

	bw = ibox.x1 - ibox.x0;
	bh = ibox.y1 - ibox.y0;

	/* Create coverage */

	cbuf = g_new (guchar, bw * bh * 4);
	for (y = 0; y < bh; y++) {
		p = cbuf + y * bw * 4;
		for (x = 0; x < bw; x++) {
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
		}
	}

	art_rgba_svp_alpha (svp1,
		ibox.x0, ibox.y0, ibox.x1, ibox.y1,
		0xffffffff,
		cbuf, bw * 4,
		NULL);

	art_svp_free (svp1);

	/* Create image */

	ibuf = g_new (guchar, bw * bh * 4);
	for (y = 0; y < bh; y++) {
		p = ibuf + y * bw * 4;
		for (x = 0; x < bw; x++) {
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
			*p++ = 0;
		}
	}

	memcpy (ba, affine, 6 * sizeof (gdouble));
	ba[4] -= ibox.x0;
	ba[5] -= ibox.y0;

	art_rgba_rgba_affine (ibuf,
		0, 0, bw, bh, bw * 4,
		pixels, width, height, rowstride,
		ba,
		ART_FILTER_NEAREST, NULL);

	/* Composite */

	for (y = 0; y < bh; y++) {
		ip = ibuf + y * bw * 4;
		cp = cbuf + y * bw * 4;
		for (x = 0; x < bw; x++) {
			ip += 3;
			cp += 3;
			*ip = (*ip) * (*cp) >> 8;
			ip++;
			cp++;
		}
	}

	art_affine_translate (ba, ibox.x0, ibox.y0);

	/* Render */

	if (rbp->alpha) {
		art_rgba_rgba_affine (rbp->pixels,
			0, 0, rbp->width, rbp->height, rbp->rowstride,
			ibuf, bw, bh, bw * 4,
			ba,
			ART_FILTER_NEAREST, NULL);
	} else {
		art_rgb_rgba_affine (rbp->pixels,
			0, 0, rbp->width, rbp->height, rbp->rowstride,
			ibuf, bw, bh, bw * 4,
			ba,
			ART_FILTER_NEAREST, NULL);
	}

	g_free (ibuf);
	g_free (cbuf);
}


