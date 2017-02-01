#define __GNOME_RFONT_C__

#include <math.h>
#include <string.h>
#include <ctype.h>
#include <gdk/gdkx.h>
#include <libgnomeprint/gnome-rfont.h>
#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_rect_svp.h>
#include <libart_lgpl/art_gray_svp.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-pgl-private.h>

#define noVERBOSE

struct _GnomeRFont {
	GtkObject object;
	GnomeFontFace * face;
	GnomeFont * font;
	gdouble transform[6];
	GHashTable * bpaths;
	GHashTable * svps;
	GHashTable * graymaps;
	GHashTable * dimensions;
	GHashTable * displays;
	/* Best fitting X font */
	GdkFont * gdk_font;
	gchar * gdk_name;
};

struct _GnomeRFontClass {
	GtkObjectClass parent_class;
};

typedef struct {
	guchar * pixels;
	gint x0, y0;
	gint width, height, rowstride;
} GnomeRFontGrayMap;

typedef struct {
	ArtDRect ddim;
	ArtIRect idim;
} GnomeRFontDimension;

typedef struct {
	GnomeRFont * rfont;
	GHashTable * pixmaps;
	GHashTable * bitmaps;
	GdkGC * gc;
	GdkGC * bmgc;
	GdkWindow * window;
	GdkVisual * visual;
} GnomeRFontDisplay;

static GHashTable * rfonts = NULL;

static void gnome_rfont_class_init (GnomeRFontClass * klass);
static void gnome_rfont_init (GnomeRFont * rfont);

static void gnome_rfont_destroy (GtkObject * object);

static const GnomeRFontGrayMap * gnome_rfont_get_glyph_graymap (const GnomeRFont * rfont, gint glyph);
static const GnomeRFontDimension * gnome_rfont_get_glyph_dimension (const GnomeRFont * rfont, gint glyph);
static const GnomeRFontDisplay * gnome_rfont_get_display (const GnomeRFont * rfont, GdkDrawable * drawable);

static const GdkPixmap * dsp_get_glyph_pixmap (const GnomeRFontDisplay * dsp, gint glyph, guint32 rgba, guint32 background);
static const GdkBitmap * dsp_get_glyph_bitmap (const GnomeRFontDisplay * dsp, gint glyph);

static gboolean rfont_free_pixmap (gpointer key, gpointer value, gpointer data);
static gboolean rfont_free_bitmap (gpointer key, gpointer value, gpointer data);
static gboolean rfont_free_display (gpointer key, gpointer value, gpointer data);
static gboolean rfont_free_dimension (gpointer key, gpointer value, gpointer data);
static gboolean rfont_free_graymap (gpointer key, gpointer value, gpointer data);
static gboolean rfont_free_svp (gpointer key, gpointer value, gpointer data);
static gboolean rfont_free_bpath (gpointer key, gpointer value, gpointer data);

static guint rfont_hash (gconstpointer key);
static gboolean rfont_equal (gconstpointer key1, gconstpointer key2);

/* DisplayFont stuff */

static gboolean gdf_find_gdk_font (GnomeRFont * rfont);

#define GDF_XFONT(f) ((f->gdk_font != NULL) || (gdf_find_gdk_font ((GnomeRFont *) f)))

static GtkObjectClass * parent_class;

GtkType
gnome_rfont_get_type (void)
{
	static GtkType rfont_type = 0;
	if (!rfont_type) {
		GtkTypeInfo rfont_info = {
			"GnomeRFont",
			sizeof (GnomeRFont),
			sizeof (GnomeRFontClass),
			(GtkClassInitFunc) gnome_rfont_class_init,
			(GtkObjectInitFunc) gnome_rfont_init,
			NULL, NULL,
			NULL
		};
		rfont_type = gtk_type_unique (gtk_object_get_type (), &rfont_info);
	}
	return rfont_type;
}

static void
gnome_rfont_class_init (GnomeRFontClass * klass)
{
	GtkObjectClass * object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->destroy = gnome_rfont_destroy;
}

static void
gnome_rfont_init (GnomeRFont * rfont)
{
	art_affine_identity (rfont->transform);
	rfont->bpaths = g_hash_table_new (NULL, NULL);
	rfont->svps = g_hash_table_new (NULL, NULL);
	rfont->graymaps = g_hash_table_new (NULL, NULL);
	rfont->dimensions = g_hash_table_new (NULL, NULL);
	rfont->displays = g_hash_table_new (NULL, NULL);
}

static void
gnome_rfont_destroy (GtkObject * object)
{
	GnomeRFont * rfont;

	rfont = (GnomeRFont *) object;

	g_hash_table_remove (rfonts, rfont);

	if (rfont->gdk_name) g_free (rfont->gdk_name);
	if (rfont->gdk_font) gdk_font_unref (rfont->gdk_font);

	if (rfont->displays) {
		g_hash_table_foreach_remove (rfont->displays, rfont_free_display, NULL);
		g_hash_table_destroy (rfont->displays);
		rfont->displays = NULL;
	}
	if (rfont->dimensions) {
		g_hash_table_foreach_remove (rfont->dimensions, rfont_free_dimension, NULL);
		g_hash_table_destroy (rfont->dimensions);
		rfont->dimensions = NULL;
	}
	if (rfont->graymaps) {
		g_hash_table_foreach_remove (rfont->graymaps, rfont_free_graymap, NULL);
		g_hash_table_destroy (rfont->graymaps);
		rfont->graymaps = NULL;
	}
	if (rfont->svps) {
		g_hash_table_foreach_remove (rfont->svps, rfont_free_svp, NULL);
		g_hash_table_destroy (rfont->svps);
		rfont->svps = NULL;
	}
	if (rfont->bpaths) {
		g_hash_table_foreach_remove (rfont->bpaths, rfont_free_bpath, NULL);
		g_hash_table_destroy (rfont->bpaths);
		rfont->bpaths = NULL;
	}

	if (rfont->font) {
		gtk_object_unref (GTK_OBJECT (rfont->font));
		rfont->font = NULL;
	}

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

GnomeRFont *
gnome_font_get_rfont (GnomeFont * font, gdouble * transform)
{
	GnomeRFont search;
	GnomeRFont * rfont;

	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (transform != NULL, NULL);

	if (rfonts == NULL) {
		rfonts = g_hash_table_new (rfont_hash, rfont_equal);
	}

	search.font = font;
	memcpy (search.transform, transform, 4 * sizeof (gdouble));
	search.transform[4] = search.transform[5] = 0.0;

	rfont = g_hash_table_lookup (rfonts, &search);

	if (rfont != NULL) {
#ifdef VERBOSE
		g_print ("found cached rfont %s\n", gnome_font_get_name (font));
#endif
		gnome_rfont_ref (rfont);
		return rfont;
	}

	rfont = gtk_type_new (gnome_rfont_get_type ());

	rfont->face = (GnomeFontFace *) gnome_font_get_face (font);
	rfont->font = font;
	gtk_object_ref (GTK_OBJECT (font));
	memcpy (rfont->transform, transform, 4 * sizeof (gdouble));
	rfont->transform[4] = rfont->transform[5] = 0.0;

	g_hash_table_insert (rfonts, rfont, rfont);

	return rfont;
}

const GnomeFont *
gnome_rfont_get_font (const GnomeRFont * rfont)
{
	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);

	return rfont->font;
}

const GnomeFontFace *
gnome_rfont_get_face (const GnomeRFont * rfont)
{
	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);

	return rfont->face;
}

gdouble *
gnome_rfont_get_matrix (const GnomeRFont * rfont, gdouble * matrix)
{
	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);
	g_return_val_if_fail (matrix != NULL, NULL);

	memcpy (matrix, rfont->transform, 4 * sizeof (gdouble));

	return matrix;
}

ArtPoint *
gnome_rfont_get_stdadvance (const GnomeRFont * rfont, ArtPoint * advance)
{
	gdouble size;

	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);
	g_return_val_if_fail (advance != NULL, NULL);

	size = gnome_font_get_size (rfont->font);

	advance->x = size * (rfont->transform[0] + rfont->transform[2]);
	advance->y = size * (rfont->transform[1] + rfont->transform[3]);

	return advance;
}

ArtPoint *
gnome_rfont_get_glyph_stdadvance (const GnomeRFont * rfont, gint glyph, ArtPoint * advance)
{
	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);

	gnome_font_get_glyph_stdadvance (rfont->font, glyph, advance);
	art_affine_point (advance, advance, rfont->transform);

	return advance;
}

ArtDRect *
gnome_rfont_get_glyph_stdbbox (const GnomeRFont * rfont, gint glyph, ArtDRect * bbox)
{
	const GnomeRFontDimension * dim;

	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);

	dim = gnome_rfont_get_glyph_dimension (rfont, glyph);

	* bbox = dim->ddim;

	return bbox;
}

ArtPoint *
gnome_rfont_get_glyph_stdkerning (const GnomeRFont * rfont, gint glyph0, gint glyph1, ArtPoint * kerning)
{
	g_return_val_if_fail (rfont != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (rfont), NULL);
	g_return_val_if_fail (glyph0 > 0, NULL);
	g_return_val_if_fail (glyph1 > 0, NULL);
	g_return_val_if_fail (kerning != NULL, NULL);

	kerning->x = 0.0;
	kerning->y = 0.0;

	return kerning;
}

const ArtBpath *
gnome_rfont_get_glyph_bpath (const GnomeRFont * rfont, gint glyph)
{
	ArtBpath * bpath;
	gdouble affine[6];
	gdouble size;

	bpath = g_hash_table_lookup (rfont->bpaths, GINT_TO_POINTER (glyph));

	if (bpath) return bpath;

	size = gnome_font_get_size (rfont->font);
	affine[0] = rfont->transform[0] * size * 0.001;
	affine[1] = rfont->transform[1] * size * 0.001;
	affine[2] = rfont->transform[2] * size * 0.001;
	affine[3] = rfont->transform[3] * size * 0.001;
	affine[4] = affine[5] = 0.0;

	bpath = (ArtBpath *) gnome_font_face_get_glyph_stdoutline (rfont->face, glyph);

	g_return_val_if_fail (bpath != NULL, NULL);

	bpath = art_bpath_affine_transform (bpath, affine);

	g_hash_table_insert (rfont->bpaths, GINT_TO_POINTER (glyph), bpath);

	return bpath;
}

const ArtSVP *
gnome_rfont_get_glyph_svp (const GnomeRFont * rfont, gint glyph)
{
	ArtSVP * svp, * svp1;
	ArtBpath * bpath;
	ArtVpath * vpath, * vpath1;

	svp = g_hash_table_lookup (rfont->svps, GINT_TO_POINTER (glyph));

	if (svp) return svp;

	/* fixme: */

	bpath = (ArtBpath *) gnome_rfont_get_glyph_bpath (rfont, glyph);

	g_return_val_if_fail (bpath != NULL, NULL);

	vpath = art_bez_path_to_vec (bpath, 0.25);
	vpath1 = art_vpath_perturb (vpath);
	art_free (vpath);
	svp = art_svp_from_vpath (vpath1);
	art_free (vpath1);
	svp1 = art_svp_uncross (svp);
	art_svp_free (svp);
	svp = art_svp_rewind_uncrossed (svp1, ART_WIND_RULE_ODDEVEN);
	art_svp_free (svp1);

	g_hash_table_insert (rfont->svps, GINT_TO_POINTER (glyph), svp);

	return svp;
}

static const GnomeRFontGrayMap *
gnome_rfont_get_glyph_graymap (const GnomeRFont * rfont, gint glyph)
{
	GnomeRFontGrayMap * gmap;
	ArtSVP * svp;
	ArtDRect bbox;
	ArtIRect ibox;

	gmap = g_hash_table_lookup (rfont->graymaps, GINT_TO_POINTER (glyph));

	if (gmap) return gmap;

	/* fixme: */

	svp = (ArtSVP *) gnome_rfont_get_glyph_svp (rfont, glyph);
	art_drect_svp (&bbox, svp);
	art_drect_to_irect (&ibox, &bbox);

	gmap = g_new (GnomeRFontGrayMap, 1);
	gmap->width = gmap->rowstride = ibox.x1 - ibox.x0;
	gmap->height = ibox.y1 - ibox.y0;
	gmap->x0 = ibox.x0;
	gmap->y0 = ibox.y0;
	gmap->pixels = g_new0 (guchar, gmap->width * gmap->height);

	art_gray_svp_aa (svp,
		ibox.x0, ibox.y0, ibox.x1, ibox.y1,
		gmap->pixels, gmap->rowstride);

	g_hash_table_insert (rfont->graymaps, GINT_TO_POINTER (glyph), gmap);

	return gmap;
}

static const GnomeRFontDimension *
gnome_rfont_get_glyph_dimension (const GnomeRFont * rfont, gint glyph)
{
	GnomeRFontDimension * dim;
	ArtSVP * svp;

	dim = g_hash_table_lookup (rfont->dimensions, GINT_TO_POINTER (glyph));

	if (dim) return dim;

	/* fixme: */

	svp = (ArtSVP *) gnome_rfont_get_glyph_svp (rfont, glyph);

	dim = g_new (GnomeRFontDimension, 1);

	if (svp) {
		art_drect_svp (&dim->ddim, svp);
	} else {
		/* fixme: This fixes crash for some unclear situation (Lauris) */
		/* fixme: Theoretically everything should be handled by face (Lauris) */
		dim->ddim.x0 = dim->ddim.y0 = 0.0;
		dim->ddim.x1 = dim->ddim.y1 = 1.0;
	}

	art_drect_to_irect (&dim->idim, &dim->ddim);

	g_hash_table_insert (rfont->dimensions, GINT_TO_POINTER (glyph), dim);

	return dim;
}

/* fixme: How to do it The Right Way (TM)? */

static const GnomeRFontDisplay *
gnome_rfont_get_display (const GnomeRFont * rfont, GdkDrawable * drawable)
{
	GdkWindow * toplevel;
	GdkVisual * visual;
	GnomeRFontDisplay * dsp;

#if 0
	toplevel = gdk_window_get_toplevel (drawable);
	g_return_val_if_fail (toplevel != NULL, NULL);
	visual = gdk_window_get_visual (toplevel);
	if (!visual) visual = gdk_visual_get_best ();
	g_return_val_if_fail (visual != NULL, NULL);
#else
	toplevel = NULL;
	visual = gdk_visual_get_best ();
	g_return_val_if_fail (visual != NULL, NULL);
#endif

	dsp = g_hash_table_lookup (rfont->displays, toplevel);

	if (dsp) return dsp;

	/* fixme: */

	dsp = g_new (GnomeRFontDisplay, 1);

	dsp->rfont = (GnomeRFont *) rfont;
	dsp->pixmaps = g_hash_table_new (NULL, NULL);
	dsp->bitmaps = g_hash_table_new (NULL, NULL);
	dsp->gc = gdk_gc_new (drawable);
	dsp->bmgc = NULL;
	dsp->window = toplevel;
	dsp->visual = visual;

	g_hash_table_insert (rfont->displays, toplevel, dsp);

	return dsp;
}

void
gnome_rfont_render_glyph_rgba8 (const GnomeRFont * rfont, gint glyph,
				guint32 rgba,
				gdouble x, gdouble y,
				guchar * buf,
				gint width, gint height, gint rowstride,
				guint flags)
{
	const GnomeRFontGrayMap * gmap;
	gint xp, yp, x0, y0, x1, y1;
	guint inkr, inkg, inkb, inka;
	guchar * s, * s0, * d, * d0;

	gmap = gnome_rfont_get_glyph_graymap (rfont, glyph);

	xp = (gint) floor (x + 0.5);
	yp = (gint) floor (y + 0.5);

	x0 = MAX (0, xp + gmap->x0);
	y0 = MAX (0, yp + gmap->y0);
	x1 = MIN (width, xp + gmap->x0 + gmap->width);
	y1 = MIN (height, yp + gmap->y0 + gmap->height);

	inkr = rgba >> 24;
	inkg = (rgba >> 16) & 0xff;
	inkb = (rgba >> 8) & 0xff;
	inka = rgba & 0xff;

	d0 = d = buf + y0 * rowstride + x0 * 4;
	s0 = s = gmap->pixels + (y0 - yp - gmap->y0) * gmap->rowstride + (x0 - xp - gmap->x0);

	for (y = y0; y < y1; y++) {
		for (x = x0; x < x1; x++) {
			guint bgr, bgg, bgb, bga;
			guint pixr, pixg, pixb;
			guint fgr, fgg, fgb;
			guint alpha;

			bgr = * (d + 0);
			bgg = * (d + 1);
			bgb = * (d + 2);
			bga = * (d + 3);

			alpha = (inka * (*s++) + 0x80) >> 8;

			pixr = (bgr * bga + 0x80) >> 8;
			pixg = (bgg * bga + 0x80) >> 8;
			pixb = (bgb * bga + 0x80) >> 8;

			fgr = ((inkr - pixr) * alpha + 0x80) >> 8;
			fgg = ((inkg - pixg) * alpha + 0x80) >> 8;
			fgb = ((inkb - pixb) * alpha + 0x80) >> 8;

			*d++ = pixr + fgr;
			*d++ = pixg + fgg;
			*d++ = pixb + fgb;
			*d++ = bga + (((0xff - bga) * alpha + 0x80) >> 8);
		}

		s = s0 += gmap->rowstride;
		d = d0 += rowstride;
	}
}

void
gnome_rfont_render_glyph_rgb8 (const GnomeRFont * rfont, gint glyph,
				guint32 rgba,
				gdouble x, gdouble y,
				guchar * buf,
				gint width, gint height, gint rowstride,
				guint flags)
{
	const GnomeRFontGrayMap * gmap;
	gint xp, yp, x0, y0, x1, y1;
	guint inkr, inkg, inkb, inka;
	guchar * s, * s0, * d, * d0;

	gmap = gnome_rfont_get_glyph_graymap (rfont, glyph);

	xp = (gint) floor (x + 0.5);
	yp = (gint) floor (y + 0.5);

	x0 = MAX (0, xp + gmap->x0);
	y0 = MAX (0, yp + gmap->y0);
	x1 = MIN (width, xp + gmap->x0 + gmap->width);
	y1 = MIN (height, yp + gmap->y0 + gmap->height);

	inkr = rgba >> 24;
	inkg = (rgba >> 16) & 0xff;
	inkb = (rgba >> 8) & 0xff;
	inka = rgba & 0xff;

	d0 = d = buf + y0 * rowstride + x0 * 3;
	s0 = s = gmap->pixels + (y0 - yp - gmap->y0) * gmap->rowstride + (x0 - xp - gmap->x0);

	for (y = y0; y < y1; y++) {
		for (x = x0; x < x1; x++) {
			guint bgr, bgg, bgb;
			guint fgr, fgg, fgb;
			guint alpha;

			bgr = * (d + 0);
			bgg = * (d + 1);
			bgb = * (d + 2);
			alpha = (inka * (*s++) + 0x80) >> 8;

			fgr = (inkr * alpha + 0x80) >> 8;
			fgg = (inkg * alpha + 0x80) >> 8;
			fgb = (inkb * alpha + 0x80) >> 8;

			*d++ = ((bgr * (0xff - alpha) + 0x80) >> 8) + fgr;
			*d++ = ((bgg * (0xff - alpha) + 0x80) >> 8) + fgg;
			*d++ = ((bgb * (0xff - alpha) + 0x80) >> 8) + fgb;
		}

		s = s0 += gmap->rowstride;
		d = d0 += rowstride;
	}
}

void gnome_rfont_render_pgl_rgb8 (const GnomePosGlyphList * pgl,
				    gdouble x, gdouble y,
				    guchar * buf,
				    gint width, gint height, gint rowstride,
				    guint flags)
{
	gint s;
	gint i;

	g_return_if_fail (pgl != NULL);
	g_return_if_fail (buf != NULL);

	for (s = 0; s < pgl->num_strings; s++) {
		GnomePosString * string;
		string = pgl->strings + s;
		for (i = string->start; i < string->start + string->length; i++) {
			gnome_rfont_render_glyph_rgb8 (string->rfont, pgl->glyphs[i].glyph,
						       string->color,
						       x + pgl->glyphs[i].x, y + pgl->glyphs[i].y,
						       buf,
						       width, height, rowstride,
						       flags);
		}
	}
}

/* fixme: collect all bits of cached data into single struct */

void
gnome_rfont_render_glyph_gdk_drawable (const GnomeRFont * rfont, gint glyph,
				guint32 rgba,
				gdouble x, gdouble y,
				GdkDrawable * drawable,
				guint32 background,
				guint flags)
{
	const GnomeRFontDisplay * dsp;
	GdkPixmap * pixmap;
	GdkBitmap * bitmap;
	const GnomeRFontDimension * dim;
	gint xp, yp;

	g_return_if_fail (rfont != NULL);
	g_return_if_fail (GNOME_IS_RFONT (rfont));

	dsp = gnome_rfont_get_display (rfont, drawable);
	g_return_if_fail (dsp != NULL);

	pixmap = (GdkPixmap *) dsp_get_glyph_pixmap (dsp, glyph, rgba, background);
	if (!pixmap) return;
	bitmap = (GdkBitmap *) dsp_get_glyph_bitmap (dsp, glyph);
	g_return_if_fail (bitmap != NULL);

	dim = gnome_rfont_get_glyph_dimension (rfont, glyph);

	xp = (gint) floor (x + 0.5) + dim->idim.x0;
	yp = (gint) floor (y + 0.5) + dim->idim.y0;

	gdk_gc_set_clip_mask (dsp->gc, bitmap);
	gdk_gc_set_clip_origin (dsp->gc, xp, yp);
	gdk_draw_pixmap (drawable, dsp->gc, pixmap,
		0, 0,
		xp, yp,
		dim->idim.x1 - dim->idim.x0, dim->idim.y1 - dim->idim.y0);
}

void gnome_rfont_render_pgl_gdk_drawable (const GnomePosGlyphList * pgl,
				gdouble x, gdouble y,
				GdkDrawable * drawable,
				guint32 background,
				guint flags)
{
	gint s;
	gint i;

	g_return_if_fail (pgl != NULL);
	g_return_if_fail (drawable != NULL);

	for (s = 0; s < pgl->num_strings; s++) {
		GnomePosString * string;
		string = pgl->strings + s;
		for (i = string->start; i < string->start + string->length; i++) {
			gnome_rfont_render_glyph_gdk_drawable (string->rfont, pgl->glyphs[i].glyph,
							       string->color,
							       x + pgl->glyphs[i].x, y + pgl->glyphs[i].y,
							       drawable,
							       background,
							       flags);
		}
	}
}

/* Helpers */

static gboolean
rfont_free_pixmap (gpointer key, gpointer value, gpointer data)
{
	gdk_pixmap_unref ((GdkPixmap *) value);

	return TRUE;
}

static gboolean
rfont_free_bitmap (gpointer key, gpointer value, gpointer data)
{
	gdk_bitmap_unref ((GdkBitmap *) value);

	return TRUE;
}

static gboolean
rfont_free_display (gpointer key, gpointer value, gpointer data)
{
	GnomeRFontDisplay * display;

	display = (GnomeRFontDisplay *) value;

	if (display->pixmaps) {
		g_hash_table_foreach_remove (display->pixmaps, rfont_free_pixmap, NULL);
		g_hash_table_destroy (display->pixmaps);
	}

	if (display->bitmaps) {
		g_hash_table_foreach_remove (display->bitmaps, rfont_free_bitmap, NULL);
		g_hash_table_destroy (display->bitmaps);
	}

	if (display->gc) gdk_gc_unref (display->gc);
	if (display->bmgc) gdk_gc_unref (display->bmgc);

	g_free (display);

	return TRUE;
}

static gboolean
rfont_free_dimension (gpointer key, gpointer value, gpointer data)
{
	g_free (value);

	return TRUE;
}

static gboolean
rfont_free_graymap (gpointer key, gpointer value, gpointer data)
{
	GnomeRFontGrayMap * gmap;

	gmap = (GnomeRFontGrayMap *) value;

	if (gmap->pixels) g_free (gmap->pixels);
	g_free (gmap);

	return TRUE;
}

static gboolean
rfont_free_svp (gpointer key, gpointer value, gpointer data)
{
	art_svp_free ((ArtSVP *) value);

	return TRUE;
}

static gboolean
rfont_free_bpath (gpointer key, gpointer value, gpointer data)
{
	art_free (value);

	return TRUE;
}

static guint
rfont_hash (gconstpointer key)
{
	GnomeRFont * rfont;

	rfont = (GnomeRFont *) key;

	return (guint) rfont->font;
}

static gint
rfont_equal (gconstpointer key1, gconstpointer key2)
{
	GnomeRFont * f1, * f2;

	f1 = (GnomeRFont *) key1;
	f2 = (GnomeRFont *) key2;

	if (f1->font != f2->font) return FALSE;

	return art_affine_equal (f1->transform, f2->transform);
}

static const GdkPixmap *
dsp_get_glyph_pixmap (const GnomeRFontDisplay * dsp, gint glyph, guint32 rgba, guint32 background)
{
	gint hash;
	GdkPixmap * pixmap;
	const GnomeRFontGrayMap * gmap;
	gint x, y;
	guchar * tmp, * tp;
	guint bgr, bgg, bgb;

	/*
	 * Well - we should do intelligent hashing here - this is test
	 */

	hash = glyph + (rgba & 0xf0f0f0f0) + ((background & 0xf0f0f000) >> 8);

	pixmap = g_hash_table_lookup (dsp->pixmaps, GINT_TO_POINTER (hash));

	if (pixmap) return pixmap;

	gmap = gnome_rfont_get_glyph_graymap (dsp->rfont, glyph);
	g_return_val_if_fail (gmap != NULL, NULL);

	if ((gmap->width < 1) || (gmap->height < 1)) return NULL;

	tmp = tp = g_new0 (gchar, gmap->width * gmap->height * 3);

	bgr = background >> 24;
	bgg = (background >> 16) & 0xff;
	bgb = (background >> 8) & 0xff;

	for (y = 0; y < gmap->height; y++) {
		for (x = 0; x < gmap->width; x++) {
			*tp++ = bgr;
			*tp++ = bgg;
			*tp++ = bgb;
		}
	}

	gnome_rfont_render_glyph_rgb8 (dsp->rfont, glyph, rgba, -gmap->x0, -gmap->y0,
				       tmp,
				       gmap->width, gmap->height, gmap->width * 3, 0);

	pixmap = gdk_pixmap_new (dsp->window, gmap->width, gmap->height, dsp->visual->depth);

	gdk_gc_set_clip_mask (dsp->gc, NULL);
	gdk_gc_set_function (dsp->gc, GDK_SET);
	gdk_draw_rectangle (pixmap, dsp->gc, TRUE, 0, 0, gmap->width, gmap->height);
	gdk_gc_set_function (dsp->gc, GDK_COPY);

	gdk_draw_rgb_image (pixmap, dsp->gc, 0, 0, gmap->width, gmap->height,
		GDK_RGB_DITHER_NONE, tmp, gmap->width * 3);

	g_free (tmp);

#if 0
	gdk_gc_set_function (dsp->gc, GDK_CLEAR);
	for (y = 0; y < gmap->height; y++) {
		for (x = 0; x < gmap->width; x++) {
			if (gmap->pixels[y * gmap->rowstride + x] > 0) {
				alpha = (gmap->pixels[y * gmap->rowstride + x] * fga + 0x80) >> 8;
				gdk_draw_point (pixmap, dsp->gc, x, y);
			}
		}
	}
#endif

	g_hash_table_insert (dsp->pixmaps, GINT_TO_POINTER (hash), pixmap);

	return pixmap;
}

static const GdkBitmap *
dsp_get_glyph_bitmap (const GnomeRFontDisplay * dsp, gint glyph)
{
	GdkBitmap * bitmap;
	const GnomeRFontGrayMap * gmap;
	gint x, y;

	bitmap = g_hash_table_lookup (dsp->bitmaps, GINT_TO_POINTER (glyph));

	if (bitmap) return bitmap;

	gmap = gnome_rfont_get_glyph_graymap (dsp->rfont, glyph);
	g_return_val_if_fail (gmap != NULL, NULL);

	if ((gmap->width < 1) || (gmap->height < 1)) return NULL;

	bitmap = gdk_pixmap_new (dsp->window, gmap->width, gmap->height, 1);

	if (!dsp->bmgc) {
		((GnomeRFontDisplay *) dsp)->bmgc = gdk_gc_new (bitmap);
	}

	gdk_gc_set_clip_mask (dsp->bmgc, NULL);
	gdk_gc_set_function (dsp->bmgc, GDK_CLEAR);
	gdk_draw_rectangle (bitmap, dsp->bmgc, TRUE, 0, 0, gmap->width, gmap->height);

	gdk_gc_set_function (dsp->bmgc, GDK_SET);
	for (y = 0; y < gmap->height; y++) {
		for (x = 0; x < gmap->width; x++) {
			if (gmap->pixels[y * gmap->rowstride + x] > 0) {
				gdk_draw_point (bitmap, dsp->bmgc, x, y);
			}
		}
	}

	g_hash_table_insert (dsp->bitmaps, GINT_TO_POINTER (glyph), bitmap);

	return bitmap;
}

/*
 * GnomeDisplayFont stuff follows
 */

typedef struct _GDFMap GDFMap;

struct _GDFMap {
	gchar * name; /* Full name */
	gchar * x_name[6]; /* ["*"], name1, name2, ..., NULL */
};

/* Default is Helvetica, so we add times for serifed font lookups */

GDFMap gp_2_x_map[] = {
	{"Bitstream Charter", {"bitstream charter", "charter", "times", "times new roman", NULL}},
	{"Century Schoolbook L", {"century schoolbook", "new century schoolbook", "times", "times new roman", NULL}},
	{"Courier", {"courier", "nimbus mono", NULL}},
	{"ITC Avant Garde Gothic", {"goth", NULL}},
	{"ITC Bookman", {"bookman", "times", "times new roman", NULL}},
	{"ITC Zapf Chancery", {"zapf chancery", "chancery", NULL}},
	{"ITC Zapf Dingbats", {"zapf dingbats", "dingbats", NULL}},
	{"New Century Schoolbook", {"new century schoolbook", "times", "times new roman", NULL}},
	{"Nimbus Mono L", {"nimbus mono", "courier", NULL}},
	{"Nimbus Roman No9 L", {"nimbus roman", "times", "times new roman", NULL}},
	{"Nimbus Sans L", {"nimbus sans", "helvetica", NULL}},
	{"Palatino", {"palatino", "times", "times new roman", NULL}},
	{"Standard Symbols L", {"standard symbols", "symbol", NULL}},
	{"Symbol", {"symbol", "standard symbols", NULL}},
	{"Times", {"times", "times new roman", NULL}},
	{"URW Bookman L", {"bookman", "times", "times new roman", NULL}},
	{"URW Chancery L", {"chancery", "zapf chancery", NULL}},
	{"URW Gothic L", {"goth", NULL}},
	{"URW Palladio L", {"palladio", "times", "times new roman", NULL}},
	{"Utopia", {"utopia", "times", "times new roman", NULL}},
	{"Dingbats", {"dingbats", "zapf dingbats", NULL}},
	{"Helvetica", {"helvetica", "nimbus sans", NULL}},
	{"Kochi Gothic", {"gothic", "mincho", "fixed", NULL}},
	{"Kochi Mincho", {"mincho", "gothic", "fixed", NULL}},
	{"MS Mincho", {"mincho", "gothic", "fixed", NULL}},
	{"MS PMincho", {"mincho", "gothic", "fixed", NULL}},
	{"Bitstream Cyberbit", {"fixed", NULL}},
};

#define gp_2_x_map_size (sizeof (gp_2_x_map) / sizeof (gp_2_x_map[0]))


/* construct an x font name and find the best point size */

static GnomeDisplayFont *create_display_font (const char *family,
					      GnomeFontWeight weight,
					      gboolean italic,
					      double points,
					      double scale)
{
	GnomeFont * gnomefont;
	GnomeRFont * rfont;
	gdouble transform[6];

	g_return_val_if_fail (family, NULL);

#if 0
	if (! scaled_display_fonts) initialize_hashes ();
#endif

	gnomefont = gnome_font_new_closest (family, weight, italic, ceil (points * scale));
	g_return_val_if_fail (gnomefont != NULL, NULL);

	art_affine_scale (transform, 1.0, -1.0);
	rfont = gnome_font_get_rfont (gnomefont, transform);

	gnome_font_unref (gnomefont);

	return rfont;
}

/* cache for create_display_font */

GnomeDisplayFont *gnome_get_display_font (const char *family,
					  GnomeFontWeight weight,
					  gboolean italic,
					  double points,
					  double scale)
{
  GnomeDisplayFont *wpdf;

      wpdf = create_display_font (family, weight, italic, points, scale);

  return wpdf;
}

int
gnome_display_font_height (GnomeDisplayFont * gdf)
{
	g_return_val_if_fail (gdf != NULL, 0);
	g_return_val_if_fail (gdf->gdk_font != NULL, 0);
	if (!GDF_XFONT (gdf)) {
		g_warning ("Cannot create X Font for GnomeDisplayFont %s %g", gnome_font_get_name (gdf->font), gdf->font->size);
		return 0;
	}

	/* We return GnomeFont height here */
	return MAX (gnome_font_get_size (gdf->font) * gdf->transform[0], gdf->gdk_font->ascent + gdf->gdk_font->descent);
}

GnomeDisplayFont * gnome_font_get_display_font (GnomeFont * font)
{
	return gnome_get_display_font (font->face->entry->familyname,
				       gnome_font_face_get_weight_code (font->face),
				       gnome_font_face_is_italic (font->face),
				       font->size,
				       1.0);
}

/*
 * Here are GnomeDisplayFont methods
 */

const GnomeFontFace *
gnome_display_font_get_face (const GnomeDisplayFont * gdf)
{
	g_return_val_if_fail (gdf != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (gdf), NULL);

	return gnome_rfont_get_face (gdf);
}

const GnomeFont *
gnome_display_font_get_font (const GnomeDisplayFont * gdf)
{
	g_return_val_if_fail (gdf != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (gdf), NULL);

	return gnome_rfont_get_font (gdf);
}

gdouble
gnome_display_font_get_scale (const GnomeDisplayFont * gdf)
{
	g_return_val_if_fail (gdf != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_RFONT (gdf), 0.0);

	return gdf->transform[0];
}

const gchar *
gnome_display_font_get_x_font_name (const GnomeDisplayFont * gdf)
{
	g_return_val_if_fail (gdf != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (gdf), NULL);
	if (!GDF_XFONT (gdf)) {
		g_warning ("Cannot create X Font for GnomeDisplayFont %s %g", gnome_font_get_name (gdf->font), gdf->font->size);
		return 0;
	}

	return gdf->gdk_name;
}

GdkFont *
gnome_display_font_get_gdk_font (const GnomeDisplayFont * gdf)
{
	g_return_val_if_fail (gdf != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_RFONT (gdf), NULL);
	if (!GDF_XFONT (gdf)) {
		g_warning ("Cannot create X Font for GnomeDisplayFont %s %g", gnome_font_get_name (gdf->font), gdf->font->size);
		return 0;
	}

	return gdf->gdk_font;
}

void
gnome_display_font_ref (GnomeDisplayFont * gdf)
{
	gtk_object_ref (GTK_OBJECT (gdf));
}

void
gnome_display_font_unref (GnomeDisplayFont * gdf)
{
	gtk_object_unref (GTK_OBJECT (gdf));
}

static void gdf_find_measured_gdk_font (const gchar * name, GnomeFontWeight weight, gboolean italic, gdouble size,
					GdkFont ** gdkfont, gchar ** gdkname, gint * best);
static void gdf_measure_string_name (const gchar * xname,
				     const gchar * familyname, GnomeFontWeight weight, gboolean italic, gdouble size,
				     gchar ** bestname, gint * best);
static gint gdf_measure_x_name (const gchar * xname, GnomeFontWeight weight, gboolean italic, gdouble size);
static void split_name (gchar * c[], gchar * name);

#ifdef VERBOSE
/*
 * Return newly allocated full name
 */

static gchar *
get_font_name (const GdkFont * font)
{
	Atom font_atom, atom;
	Bool status;

#ifdef VERBOSE
	g_print ("Extracting X font info\n");
#endif

	font_atom = gdk_atom_intern ("FONT", FALSE);

	if (font->type == GDK_FONT_FONTSET) {
		XFontStruct **font_structs;
		gint num_fonts;
		gchar **font_names;

		num_fonts = XFontsOfFontSet (GDK_FONT_XFONT (font), &font_structs, &font_names);
		status = XGetFontProperty (font_structs[0], font_atom, &atom);
	} else {
		status = XGetFontProperty (GDK_FONT_XFONT (font), font_atom, &atom);
	}

	if (status) {
		return gdk_atom_name (atom);
	}

	return NULL;
}
#endif

static gboolean
gdf_find_gdk_font (GnomeRFont * rfont)
{
	static GHashTable * gp2x = NULL;
	const gchar * familyname;
	GnomeFontWeight weight;
	gboolean italic;
	gdouble size;
	gint best;
	GdkFont * gdkfont;
	gchar * gdkname = NULL, * xname;

	if (rfont->gdk_font) return TRUE;

	familyname = gnome_font_face_get_family_name (rfont->face);
	weight = gnome_font_face_get_weight_code (rfont->face);
	italic = gnome_font_face_is_italic (rfont->face);
	size = gnome_font_get_size (rfont->font) * rfont->transform[0];

	if (!gp2x) {
		gint i;
		gp2x = g_hash_table_new (g_str_hash, g_str_equal);
		for (i = 0; i < gp_2_x_map_size; i++) {
			g_hash_table_insert (gp2x, gp_2_x_map[i].name, gp_2_x_map[i].x_name);
		}
	}

	best = G_MAXINT;
	gdkfont = NULL;

	/* First try with original font name */

	xname = g_strdup (familyname);
	g_strdown (xname);
	gdf_find_measured_gdk_font (xname, weight, italic, size, &gdkfont, &gdkname, &best);
	g_free (xname);

	if (best > 0) {
		gchar ** xn;

		/* Try with known alternative names */

		xn = g_hash_table_lookup (gp2x, familyname);
		if (xn) {
			while (*xn) {
				gdf_find_measured_gdk_font (*xn, weight, italic, size, &gdkfont, &gdkname, &best);
				if (best == 0) break;
				xn++;
			}
		}
	}

#if 0
	if (!gdkfont) {
		/* Try Helvetica */

		gdf_find_measured_gdk_font ("helvetica", weight, italic, size, &gdkfont, &gdkname, &best);
	}
#endif

	if (!gdkfont) {
		/* No useful font found, so load 'fixed' */
		
		gdkfont = gdk_fontset_load ("fixed");
		if (!gdkfont) gdkfont = gdk_font_load ("fixed");
		if (!gdkfont) {
			g_warning ("Serious error: Cannot load font 'fixed' - your program most probably will not work");
			return FALSE;
		}
		gdkname = g_strdup ("fixed");
	}

	rfont->gdk_font = gdkfont;
	rfont->gdk_name = gdkname;

#ifdef VERBOSE
	g_print ("Asked: %s %d %d %f\n", familyname, weight, italic, size);
	g_print ("Tried: %s\n", gdkname);
	xname = get_font_name (gdkfont);
	g_print ("Got: %s\n", xname);
	if (xname) g_free (xname);
#endif

	return TRUE;
}

static void
gdf_find_measured_gdk_font (const gchar * name, GnomeFontWeight weight, gboolean italic, gdouble size,
			    GdkFont ** gdkfont, gchar ** gdkname, gint * best)
{
	gchar xname[1024];
	gint pxsize;
	gchar *bestname;
	gint mybest;

	g_return_if_fail (strlen (name) < 512);

	bestname = NULL;
	pxsize = 0;
	mybest = *best;

	/* Try 0-sized font */

	g_snprintf (xname, 1024, "-%s-%s-%s-%s-%s-%s-%d-%s-%s-%s-%s-%s-%s",
		    "*", /* Foundry */
		    name, /* Family */
		    "*", /* Weight */
		    "*", /* Slant */
		    "normal", /* Set width */
		    "*", /* Add Style */
		    pxsize, /* Pixel Size */
		    "*", /* Point size */
		    "*", "*", /* Resolution X, Y */
		    "*", "*", "*"); /* Spacing, Avg Width, Charset */

	gdf_measure_string_name (xname, name, weight, italic, size, &bestname, &mybest);

	if (!bestname) {
		/* Try exact size */
		pxsize = (gint) floor (size + 0.5);
		g_snprintf (xname, 1024, "-%s-%s-%s-%s-%s-%s-%d-%s-%s-%s-%s-%s-%s",
			    "*", name, "*", "*", "normal", "*", pxsize, "*", "*", "*", "*", "*", "*");
		gdf_measure_string_name (xname, name, weight, italic, size, &bestname, &mybest);
	}

	if (!bestname) {
		/* We didn't find any name - try walking up'n'down */
		gint dev, i;
		dev = pxsize / 3;
		for (i = 1; i <= dev; i++) {
			g_snprintf (xname, 1024, "-%s-%s-%s-%s-%s-%s-%d-%s-%s-%s-%s-%s-%s",
				    "*", name, "*", "*", "normal", "*", (pxsize + i), "*", "*", "*", "*", "*", "*");
			gdf_measure_string_name (xname, name, weight, italic, size, &bestname, &mybest);
			if (*best == 0) break;
			g_snprintf (xname, 1024, "-%s-%s-%s-%s-%s-%s-%d-%s-%s-%s-%s-%s-%s",
				    "*", name, "*", "*", "normal", "*", (pxsize - i), "*", "*", "*", "*", "*", "*");
			gdf_measure_string_name (xname, name, weight, italic, size, &bestname, &mybest);
			if (*best == 0) break;
		}
	}

	if (bestname) {
		GdkFont * f;
		gchar * c[14];

		split_name (c, bestname);

		/* If font is scalable, use exact size */
		if (pxsize == 0) pxsize = (gint) floor (size + 0.5);

		/* Clear unimportant fields */
		g_snprintf (xname, 1024, "-*-%s-%s-%s-%s-*-%d-*-*-*-*-*-%s",
			    c[2], c[3], c[4], c[5], pxsize, "*-*" /* c[13] */);
#ifdef VERBOSE
		g_print ("Trying: %s\n", xname);
#endif
		f = gdk_fontset_load (xname);
#if 0
		if (!f) {
			g_snprintf (xname, 1024, "-*-%s-%s-%s-%s-*-%d-*-*-*-*-*-%s",
				    c[2], c[3], c[4], c[5], pxsize, c[13]);
#ifdef VERBOSE
			g_print ("Trying: %s\n", xname);
#endif
			f = gdk_fontset_load (xname);
			if (!f && (MB_CUR_MAX <= 1)) f = gdk_font_load (xname);
		}
#else
		if (!f && (MB_CUR_MAX <= 1)) f = gdk_font_load (xname);
#endif

		if (f) {
#ifdef VERBOSE
			g_print ("Got it\n");
#endif
			if (*gdkname) g_free (*gdkname);
			*gdkname = g_strdup (xname);
			if (*gdkfont) gdk_font_unref (*gdkfont);
			*gdkfont = f;
			*best = mybest;
		}

		g_free (bestname);
	}
}

static void
gdf_measure_string_name (const gchar * xname,
			 const gchar * familyname, GnomeFontWeight weight, gboolean italic, gdouble size,
			 gchar ** bestname, gint * best)
{
	gchar ** namelist;
	gint numfonts;

	namelist = XListFonts (GDK_DISPLAY (), xname, 256, &numfonts);

	if (namelist) {
		gchar * nptr;
		gint i;

		nptr = NULL;

		for (i = 0; i < numfonts; i++) {
			gint dist;

			dist = gdf_measure_x_name (namelist[i], weight, italic, size);

			if (dist < *best) {
				*best = dist;
				nptr = namelist[i];
				if (*best == 0) break;
			}
		}

		if (nptr) {
			if (*bestname) g_free (*bestname);
			*bestname = g_strdup (nptr);
		}

		XFreeFontNames (namelist);
	}
}

static gint
gdf_measure_x_name (const gchar * xname, GnomeFontWeight weight, gboolean italic, gdouble size)
{
	static GHashTable * xw2gw = NULL;
	gchar * c[14];
	gchar * p;
	gdouble actualsize, actualweight;
	gdouble dist_size, dist_weight, dist_italic;

	p = g_strdup (xname);
	split_name (c, p);

	/* <size> = (((actualsize - reqsize) / reqsize) / 0.05) ^ 2 */

	actualsize = atof (c[7]);

	if (actualsize == 0.0) {
		/* 0 means fonts is scalable */
		dist_size = 0.0;
	} else if (floor (size + 0.5) == actualsize) {
		/* We differ no more than 0.5 pixel */
		dist_size = 0.0;
	} else {
		dist_size = (((actualsize - size) / size) / 0.05);
		dist_size = dist_size * dist_size;
	}

	if (!xw2gw) {
		xw2gw = g_hash_table_new (g_str_hash, g_str_equal);
		g_hash_table_insert (xw2gw, "extralight", GINT_TO_POINTER (GNOME_FONT_EXTRA_LIGHT));
		g_hash_table_insert (xw2gw, "thin", GINT_TO_POINTER (GNOME_FONT_THIN));
		g_hash_table_insert (xw2gw, "light", GINT_TO_POINTER (GNOME_FONT_LIGHT));
		g_hash_table_insert (xw2gw, "regular", GINT_TO_POINTER (GNOME_FONT_BOOK));
		g_hash_table_insert (xw2gw, "medium", GINT_TO_POINTER (GNOME_FONT_MEDIUM));
		g_hash_table_insert (xw2gw, "demibold", GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (xw2gw, "bold", GINT_TO_POINTER (GNOME_FONT_BOLD));
		g_hash_table_insert (xw2gw, "heavy", GINT_TO_POINTER (GNOME_FONT_HEAVY));
		g_hash_table_insert (xw2gw, "extrabold", GINT_TO_POINTER (GNOME_FONT_EXTRABOLD));
		g_hash_table_insert (xw2gw, "black", GINT_TO_POINTER (GNOME_FONT_BLACK));
		g_hash_table_insert (xw2gw, "extrablack", GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
	}

	/* <weight> = actualweight - reqweight */

	g_strdown (c[3]);
	actualweight = (gdouble) GPOINTER_TO_INT (g_hash_table_lookup (xw2gw, c[3]));
	dist_weight = actualweight - (gdouble) weight;

	/* <italic> = 0 (i-i, r-r), 5 (i-r, r-i) */

	if (italic) {
		if ((tolower (*c[4]) == 'i') || (tolower (*c[4]) == 'o')) dist_italic = 0.0;
		else dist_italic = 5.0;
	} else {
		if (tolower (*c[4]) == 'r') dist_italic = 0.0;
		else dist_italic = 5.0;
	}

	g_free (p);

	return (gint) floor (sqrt (dist_size * dist_size + dist_weight * dist_weight + dist_italic * dist_italic) + 0.5);
}

/*
 * Splits full X font name into pieces, overwriting hyphens
 */

static void
split_name (gchar * c[], gchar * name)
{
	gchar *p;
	gint i;

	p = name;

	for (i = 0; i < 13; i++) {
		c[i] = p;
		/* Skip text */
		while (*p && (*p != '-')) p++;
		/* Replace hyphen with '\0' */
		if (*p) *p++ = '\0';
	}

	c[i] = p;
}


