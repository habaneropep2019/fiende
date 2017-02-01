/*
 * gnome-print-frgba.c: A Fake RGBA print context for GNOME
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * (C) 2000 Lauris Kaplinski
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

#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-print-frgba.h>
#include <libgnomeprint/gp-gc.h>
#include <libgnomeprint/gnome-print-rbuf.h>
#include <libgnomeprint/gnome-print-meta.h>

#define GP_RENDER_DPI 72.0

struct _GnomePrintFRGBA {
	GnomePrintContext pc;

	GnomePrintFRGBAPrivate * private;
};

struct _GnomePrintFRGBAClass {
	GnomePrintContextClass parent_class;
};

/*
 * Private structures
 */

struct _GnomePrintFRGBAPrivate {
	GnomePrintContext * context;
	GnomePrintMeta * meta;
};

static void gpf_class_init (GnomePrintFRGBAClass *class);
static void gpf_init (GnomePrintFRGBA *frgba);
static void gpf_destroy (GtkObject *object);

static gint gpf_fill (GnomePrintContext * pc, ArtWindRule rule);
static gint gpf_stroke (GnomePrintContext * pc);
static gint gpf_glyphlist (GnomePrintContext * pc, GnomeGlyphList *gl);
static gint gpf_show_sized (GnomePrintContext * pc, const char *text, int bytes);
static gint gpf_textline (GnomePrintContext * pc, GnomeTextLine * line);

static gint gpf_grayimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride);
static gint gpf_rgbimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride);
static gint gpf_rgbaimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride);

static gint gpf_gsave (GnomePrintContext * pc);
static gint gpf_grestore (GnomePrintContext * pc);

static gint gpf_newpath (GnomePrintContext * pc);
static gint gpf_moveto (GnomePrintContext * pc, gdouble x, gdouble y);
static gint gpf_lineto (GnomePrintContext * pc, gdouble x, gdouble y);
static gint gpf_curveto (GnomePrintContext * pc, gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble x3, gdouble y3);
static gint gpf_closepath (GnomePrintContext * pc);
static gint gpf_strokepath (GnomePrintContext * pc);

static gint gpf_setrgbcolor (GnomePrintContext * pc, gdouble r, gdouble g, gdouble b);
static gint gpf_setlinewidth (GnomePrintContext * pc, gdouble width);
static gint gpf_setmiterlimit (GnomePrintContext * pc, gdouble limit);
static gint gpf_setlinejoin (GnomePrintContext * pc, gint jointype);
static gint gpf_setlinecap (GnomePrintContext * pc, gint captype);
static gint gpf_setdash (GnomePrintContext * pc, gint n_values, const gdouble * values, gdouble offset);
static gint gpf_setopacity (GnomePrintContext * pc, gdouble opacity);

static gint gpf_setfont (GnomePrintContext * pc, GnomeFont * font);

static gint gpf_concat (GnomePrintContext * pc, const gdouble matrix[6]);

static gint gpf_clip (GnomePrintContext * pc, ArtWindRule rule);

static gint gpf_beginpage (GnomePrintContext * pc, const gchar * name);
static gint gpf_showpage (GnomePrintContext * pc);
static gint gpf_close (GnomePrintContext * pc);

static ArtDRect * gpf_currentpath_bbox (GPGC * gc, ArtDRect * box);
static void gpf_render_buf (GnomePrintFRGBA * frgba, ArtDRect * box);

static GnomePrintContextClass * print_frgba_parent_class;

/**
 * gnome_print_frgba_get_type:
 *
 * GTK type identification routine for #GnomePrintFRGBA
 *
 * Returns: The Gtk type for the #GnomePrintFRGBA object
 */

GtkType
gnome_print_frgba_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnomePrintFRGBA",
			sizeof (GnomePrintFRGBA),
			sizeof (GnomePrintFRGBAClass),
			(GtkClassInitFunc) gpf_class_init,
			(GtkObjectInitFunc) gpf_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		type = gtk_type_unique (gnome_print_context_get_type (), &info);
	}
	return type;
}

static void
gpf_class_init (GnomePrintFRGBAClass *class)
{
	GtkObjectClass * object_class;
	GnomePrintContextClass * pc_class;

	object_class = (GtkObjectClass *) class;
	pc_class = (GnomePrintContextClass *) class;

	print_frgba_parent_class = gtk_type_class (gnome_print_context_get_type ());

	object_class->destroy = gpf_destroy;
	
	pc_class->fill = gpf_fill;
	pc_class->stroke = gpf_stroke;
	pc_class->glyphlist = gpf_glyphlist;
	pc_class->show_sized = gpf_show_sized;
	pc_class->textline = gpf_textline;
	pc_class->grayimage = gpf_grayimage;
	pc_class->rgbimage = gpf_rgbimage;
	pc_class->rgbaimage = gpf_rgbaimage;

	pc_class->newpath = gpf_newpath;
	pc_class->moveto = gpf_moveto;
	pc_class->lineto = gpf_lineto;
	pc_class->curveto = gpf_curveto;
	pc_class->closepath = gpf_closepath;
	pc_class->strokepath = gpf_strokepath;

	pc_class->setrgbcolor = gpf_setrgbcolor;
	pc_class->setopacity = gpf_setopacity;
	pc_class->setlinewidth = gpf_setlinewidth;
	pc_class->setmiterlimit = gpf_setmiterlimit;
	pc_class->setlinejoin = gpf_setlinejoin;
	pc_class->setlinecap = gpf_setlinecap;
	pc_class->setdash = gpf_setdash;

	pc_class->setfont = gpf_setfont;

	pc_class->concat = gpf_concat;
	pc_class->gsave = gpf_gsave;
	pc_class->grestore = gpf_grestore;

	pc_class->clip = gpf_clip;

	pc_class->beginpage = gpf_beginpage;
	pc_class->showpage = gpf_showpage;
	pc_class->close = gpf_close;
}

static void
gpf_init (GnomePrintFRGBA * frgba)
{
	frgba->private = g_new (GnomePrintFRGBAPrivate, 1);
	g_assert (frgba->private != NULL);

	frgba->private->context = NULL;
	frgba->private->meta = NULL;
}

static void
gpf_destroy (GtkObject *object)
{
	GnomePrintFRGBA * frgba;

	frgba = GNOME_PRINT_FRGBA (object);

	if (frgba->private) {
		if (frgba->private->context)
			gtk_object_unref (GTK_OBJECT (frgba->private->context));
		if (frgba->private->meta)
			gtk_object_unref (GTK_OBJECT (frgba->private->meta));
		g_free (frgba->private);
	}

	if (GTK_OBJECT_CLASS (print_frgba_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (print_frgba_parent_class)->destroy) (object);
}

/*
 * Drawing methods
 */

static gint
gpf_fill (GnomePrintContext * pc, ArtWindRule rule)
{
	GnomePrintFRGBA * frgba;
	GnomePrintFRGBAPrivate * fp;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);
	fp = frgba->private;

	if (rule == ART_WIND_RULE_NONZERO) {
		gnome_print_fill (GNOME_PRINT_CONTEXT (fp->meta));
	} else {
		gnome_print_eofill (GNOME_PRINT_CONTEXT (fp->meta));
	}

	if (gp_gc_get_opacity (pc->gc) <= (255.0 / 256.0)) {
		/* We have alpha! */
		ArtDRect bbox;

		gpf_currentpath_bbox (pc->gc, &bbox);

		gnome_print_gsave (frgba->private->context);
		gnome_print_clip (frgba->private->context);
		gpf_render_buf (frgba, &bbox);
		gnome_print_grestore (frgba->private->context);
	} else {
		if (rule == ART_WIND_RULE_NONZERO) {
			return gnome_print_fill (fp->context);
		} else {
			return gnome_print_fill (fp->context);
		}
	}

	return 1;
}

/* fixme: do buffering - but we have to find the right bbox! */

static gint
gpf_stroke (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;
	GnomePrintFRGBAPrivate * fp;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);
	fp = frgba->private;

	gnome_print_stroke (GNOME_PRINT_CONTEXT (fp->meta));
	return gnome_print_stroke (fp->context);
}

static gint
gpf_glyphlist (GnomePrintContext * pc, GnomeGlyphList *gl)
{
	GnomePrintFRGBA * frgba;
	GnomePrintFRGBAPrivate * fp;

	frgba = GNOME_PRINT_FRGBA (pc);
	fp = frgba->private;

	gnome_print_glyphlist (GNOME_PRINT_CONTEXT (fp->meta), gl);
	return gnome_print_glyphlist (fp->context, gl);
}

static gint
gpf_show_sized (GnomePrintContext * pc, const char *text, int bytes)
{
	GnomePrintFRGBA * frgba;
	GnomePrintFRGBAPrivate * fp;

	frgba = GNOME_PRINT_FRGBA (pc);
	fp = frgba->private;

	gnome_print_show_sized (GNOME_PRINT_CONTEXT (fp->meta), text, bytes);
	return gnome_print_show_sized (fp->context, text, bytes);
}

static gint
gpf_textline (GnomePrintContext * pc, GnomeTextLine * line)
{
	GnomePrintFRGBA * frgba;
	GnomePrintFRGBAPrivate * fp;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);
	fp = frgba->private;

	gnome_print_textline (GNOME_PRINT_CONTEXT (fp->meta), line);
	return gnome_print_textline (fp->context, line);
}

static gint
gpf_grayimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride)
{
	GnomePrintFRGBA * frgba;
	GnomePrintFRGBAPrivate * fp;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);
	g_return_val_if_fail (data != NULL, 0);
	g_return_val_if_fail (width > 0, 0);
	g_return_val_if_fail (height > 0, 0);

	frgba = GNOME_PRINT_FRGBA (pc);
	fp = frgba->private;

	gnome_print_grayimage (GNOME_PRINT_CONTEXT (fp->meta), data, width, height, rowstride);
	return gnome_print_grayimage (fp->context, data, width, height, rowstride);
}

static gint
gpf_rgbimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride)
{
	GnomePrintFRGBA * frgba;
	GnomePrintFRGBAPrivate * fp;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);
	g_return_val_if_fail (data != NULL, 0);
	g_return_val_if_fail (width > 0, 0);
	g_return_val_if_fail (height > 0, 0);

	frgba = GNOME_PRINT_FRGBA (pc);
	fp = frgba->private;

	gnome_print_rgbimage (GNOME_PRINT_CONTEXT (fp->meta), data, width, height, rowstride);
	return gnome_print_rgbimage (fp->context, data, width, height, rowstride);
}

/* fixme: clip */

static gint
gpf_rgbaimage (GnomePrintContext * pc, const gchar * data, gint width, gint height, gint rowstride)
{
	GnomePrintFRGBA * frgba;
	GnomePrintFRGBAPrivate * fp;
	ArtDRect bbox;
	ArtPoint p;
	const gdouble * ctm;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);
	g_return_val_if_fail (data != NULL, 0);
	g_return_val_if_fail (width > 0, 0);
	g_return_val_if_fail (height > 0, 0);

	frgba = GNOME_PRINT_FRGBA (pc);
	fp = frgba->private;

	gnome_print_rgbaimage (GNOME_PRINT_CONTEXT (fp->meta), data, width, height, rowstride);

	ctm = gp_gc_get_ctm (pc->gc);

	p.x = 0.0;
	p.y = 0.0;
	art_affine_point (&p, &p, ctm);
	bbox.x0 = bbox.x1 = p.x;
	bbox.y0 = bbox.y1 = p.y;
	p.x = 1.0;
	p.y = 0.0;
	art_affine_point (&p, &p, ctm);
	bbox.x0 = MIN (bbox.x0, p.x);
	bbox.y0 = MIN (bbox.y0, p.y);
	bbox.x1 = MAX (bbox.x1, p.x);
	bbox.y1 = MAX (bbox.y1, p.y);
	p.x = 1.0;
	p.y = 1.0;
	art_affine_point (&p, &p, ctm);
	bbox.x0 = MIN (bbox.x0, p.x);
	bbox.y0 = MIN (bbox.y0, p.y);
	bbox.x1 = MAX (bbox.x1, p.x);
	bbox.y1 = MAX (bbox.y1, p.y);
	p.x = 0.0;
	p.y = 1.0;
	art_affine_point (&p, &p, ctm);
	bbox.x0 = MIN (bbox.x0, p.x);
	bbox.y0 = MIN (bbox.y0, p.y);
	bbox.x1 = MAX (bbox.x1, p.x);
	bbox.y1 = MAX (bbox.y1, p.y);

	gnome_print_gsave (frgba->private->context);
	gnome_print_newpath (frgba->private->context);
	gnome_print_moveto (frgba->private->context, 0.0, 0.0);
	gnome_print_lineto (frgba->private->context, 1.0, 0.0);
	gnome_print_lineto (frgba->private->context, 1.0, 1.0);
	gnome_print_lineto (frgba->private->context, 0.0, 1.0);
	gnome_print_lineto (frgba->private->context, 0.0, 0.0);
	gnome_print_closepath (frgba->private->context);
	gnome_print_clip (frgba->private->context);
	gpf_render_buf (frgba, &bbox);
	gnome_print_grestore (frgba->private->context);
#if 0
	return gnome_print_rgbaimage (fp->context, data, width, height, rowstride);
#endif
	return 1;
}

/*
 * Gsave & Grestore
 */

static gint
gpf_gsave (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_gsave (GNOME_PRINT_CONTEXT (frgba->private->meta));
	return gnome_print_gsave (frgba->private->context);
}

static gint
gpf_grestore (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_grestore (GNOME_PRINT_CONTEXT (frgba->private->meta));
	return gnome_print_grestore (frgba->private->context);
}


/*
 * Path operations
 */

static gint
gpf_newpath (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_newpath (GNOME_PRINT_CONTEXT (frgba->private->meta));
	return gnome_print_newpath (frgba->private->context);
}

static gint
gpf_moveto (GnomePrintContext * pc, gdouble x, gdouble y)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_moveto (GNOME_PRINT_CONTEXT (frgba->private->meta), x, y);
	return gnome_print_moveto (frgba->private->context, x, y);
}

static gint
gpf_lineto (GnomePrintContext * pc, gdouble x, gdouble y)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_lineto (GNOME_PRINT_CONTEXT (frgba->private->meta), x, y);
	return gnome_print_lineto (frgba->private->context, x, y);
}

static gint
gpf_curveto (GnomePrintContext * pc, gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble x3, gdouble y3)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_curveto (GNOME_PRINT_CONTEXT (frgba->private->meta), x1, y1, x2, y2, x3, y3);
	return gnome_print_curveto (frgba->private->context, x1, y1, x2, y2, x3, y3);
}

static gint
gpf_closepath (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_closepath (GNOME_PRINT_CONTEXT (frgba->private->meta));
	return gnome_print_closepath (frgba->private->context);
}

static gint
gpf_strokepath (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_strokepath (GNOME_PRINT_CONTEXT (frgba->private->meta));
	return gnome_print_strokepath (frgba->private->context);
}

/*
 * Simple GC Properties
 */

static gint
gpf_setrgbcolor (GnomePrintContext * pc, gdouble r, gdouble g, gdouble b)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_setrgbcolor (GNOME_PRINT_CONTEXT (frgba->private->meta), r, g, b);
	return gnome_print_setrgbcolor (frgba->private->context, r, g, b);
}

static gint
gpf_setopacity (GnomePrintContext * pc, gdouble opacity)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_setopacity (GNOME_PRINT_CONTEXT (frgba->private->meta), opacity);
	return gnome_print_setopacity (frgba->private->context, opacity);
}

static gint
gpf_setlinewidth (GnomePrintContext * pc, gdouble width)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_setlinewidth (GNOME_PRINT_CONTEXT (frgba->private->meta), width);
	return gnome_print_setlinewidth (frgba->private->context, width);
}

static gint
gpf_setmiterlimit (GnomePrintContext * pc, gdouble limit)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_setmiterlimit (GNOME_PRINT_CONTEXT (frgba->private->meta), limit);
	return gnome_print_setmiterlimit (frgba->private->context, limit);
}

static gint
gpf_setlinejoin (GnomePrintContext * pc, gint jointype)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_setlinejoin (GNOME_PRINT_CONTEXT (frgba->private->meta), jointype);
	return gnome_print_setlinejoin (frgba->private->context, jointype);
}

static gint
gpf_setlinecap (GnomePrintContext * pc, gint captype)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_setlinecap (GNOME_PRINT_CONTEXT (frgba->private->meta), captype);
	return gnome_print_setlinecap (frgba->private->context, captype);
}

static gint
gpf_setdash (GnomePrintContext * pc, gint n_values, const gdouble * values, gdouble offset)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_setdash (GNOME_PRINT_CONTEXT (frgba->private->meta), n_values, values, offset);
	return gnome_print_setdash (frgba->private->context, n_values, values, offset);
}

/*
 * Font loading
 */

static gint
gpf_setfont (GnomePrintContext * pc, GnomeFont * font)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);
	g_return_val_if_fail (font != NULL, 0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_setfont (GNOME_PRINT_CONTEXT (frgba->private->meta), font);
	return gnome_print_setfont (frgba->private->context, font);
}

/*
 * CTM transformation
 */

static gint
gpf_concat (GnomePrintContext * pc, const gdouble matrix[6])
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);
	g_return_val_if_fail (matrix != NULL, 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_concat (GNOME_PRINT_CONTEXT (frgba->private->meta), matrix);
	return gnome_print_concat (frgba->private->context, matrix);
}

static gint
gpf_clip (GnomePrintContext * pc, ArtWindRule rule)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	if (rule == ART_WIND_RULE_NONZERO) {
		gp_gc_clip (pc->gc);
		gnome_print_clip (GNOME_PRINT_CONTEXT (frgba->private->meta));
		return gnome_print_clip (frgba->private->context);
	} else {
		gp_gc_eoclip (pc->gc);
		gnome_print_eoclip (GNOME_PRINT_CONTEXT (frgba->private->meta));
		return gnome_print_eoclip (frgba->private->context);
	}
}

/*
 * Page ops
 */

static gint
gpf_beginpage (GnomePrintContext * pc, const gchar * name)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_beginpage (GNOME_PRINT_CONTEXT (frgba->private->meta), name);
	return gnome_print_beginpage (frgba->private->context, name);
}

static gint
gpf_showpage (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	/* Release old meta */
	gtk_object_unref (GTK_OBJECT (frgba->private->meta));
	/* Create fresh meta */
	frgba->private->meta = gnome_print_meta_new ();
	/* Reset graphic context */
	return gnome_print_showpage (frgba->private->context);
}

static gint
gpf_close (GnomePrintContext * pc)
{
	GnomePrintFRGBA * frgba;

	g_return_val_if_fail (pc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_FRGBA (pc), 0);

	frgba = GNOME_PRINT_FRGBA (pc);

	gnome_print_context_close (GNOME_PRINT_CONTEXT (frgba->private->meta));
	return gnome_print_context_close (frgba->private->context);
}


GnomePrintContext *
gnome_print_frgba_new (GnomePrintContext * context)
{
	GnomePrintFRGBA * frgba;
	GnomePrintMeta * meta;

	g_return_val_if_fail (context != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (context), NULL);

	frgba = gtk_type_new (GNOME_TYPE_PRINT_FRGBA);
	g_assert (frgba != NULL);

	meta = gnome_print_meta_new ();
	g_assert (meta != NULL);

	frgba->private->context = context;
	gtk_object_ref (GTK_OBJECT (context));

	frgba->private->meta = meta;

	return GNOME_PRINT_CONTEXT (frgba);
}

static ArtDRect *
gpf_currentpath_bbox (GPGC * gc, ArtDRect * box)
{
	const ArtBpath * bpath;
	ArtVpath * vpath;

	if (!gp_gc_has_currentpath (gc)) {
		box->x0 = box->y0 = 0.0;
		box->x1 = box->y1 = -1.0;
		return NULL;
	}

	bpath = gp_path_bpath (gp_gc_get_currentpath (gc));
	vpath = art_bez_path_to_vec (bpath, 0.25);
	art_vpath_bbox_drect (vpath, box);
	art_free (vpath);

	return box;
}

static void
gpf_render_buf (GnomePrintFRGBA * frgba, ArtDRect * box)
{
	GnomePrintFRGBAPrivate * fp;
	GnomePrintContext * gpr;
	guchar * pixels;
	gdouble page2buf[6], mtc[6], a[6];
	gdouble width, height;
	gint w, h;

	fp = frgba->private;

#ifdef VERBOSE
	g_print ("box %g %g %g %g\n", box->x0, box->y0, box->x1, box->y1);
#endif

	width = ceil ((box->x1 - box->x0) * GP_RENDER_DPI / 72.0);
	height = ceil ((box->y1 - box->y0) * GP_RENDER_DPI / 72.0);
	w = (gint) width;
	h = (gint) height;

	if (width <= 0) return;
	if (height <= 0) return;

	pixels = g_new (guchar, w * h * 3);

	/* fixme: should be paper color */
	memset (pixels, 0xff, w * h * 3);
	art_affine_translate (page2buf, -box->x0, -box->y1);
	art_affine_scale (a, width / (box->x1 - box->x0), -height / (box->y1 - box->y0));
	art_affine_multiply (page2buf, page2buf, a);

	gpr = gnome_print_rbuf_new (pixels,
			w,
			h,
			w * 3,
			page2buf,
			FALSE);

	gnome_print_meta_render_from_object (gpr, fp->meta);

	gtk_object_unref (GTK_OBJECT (gpr));

	gnome_print_gsave (fp->context);
	art_affine_invert (mtc, gp_gc_get_ctm (GNOME_PRINT_CONTEXT (frgba)->gc));
	gnome_print_concat (fp->context, mtc);
	gnome_print_translate (fp->context, box->x0, box->y0);
	gnome_print_scale (fp->context, (box->x1 - box->x0), (box->y1 - box->y0));

	gnome_print_rgbimage (fp->context, pixels, w, h, w * 3);

	gnome_print_grestore (fp->context);

	g_free (pixels);
}



