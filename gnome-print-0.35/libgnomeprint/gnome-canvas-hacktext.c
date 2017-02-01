#define _GNOME_CANVAS_HACKTEXT_C_

/*
 * Hacktext item type for GnomeCanvas widget
 *
 * This is mainly useful for gnome-print preview context.
 *
 * Copyright (C) 1998,1999 The Free Software Foundation
 * Copyright (C) 2000 Helix Code, Inc.
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Raph Levien <raph@acm.org>
 *          Lauris Kaplinski <lauris@helixcode.com>
 *
 * TODO:
 * - Clipping
 */

#define noVERBOSE

/* These includes are set up for standalone compile. If/when this codebase
   is integrated into libgnomeui, the includes will need to change. */
#include <math.h>
#include <string.h>
#include <gnome.h>
#include <libart_lgpl/art_rect.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_vpath_dash.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libart_lgpl/art_svp_point.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-rfont.h>
#include <libgnomeprint/gnome-pgl.h>
#include <libgnomeprint/gnome-pgl-private.h>
#include "gnome-canvas-hacktext.h"

enum {
	ARG_0,
	ARG_TEXT,
	ARG_GLYPHLIST,
	ARG_FILL_COLOR,
	ARG_FILL_COLOR_GDK,
	ARG_FILL_COLOR_RGBA,
	ARG_FONT,
	ARG_X,
	ARG_Y
};


static void gnome_canvas_hacktext_class_init    (GnomeCanvasHacktextClass *class);
static void gnome_canvas_hacktext_init          (GnomeCanvasHacktext      *hacktext);
static void gnome_canvas_hacktext_destroy       (GtkObject               *object);
static void gnome_canvas_hacktext_set_arg       (GtkObject               *object,
						 GtkArg                  *arg,
						 guint                    arg_id);
static void gnome_canvas_hacktext_get_arg       (GtkObject               *object,
						 GtkArg                  *arg,
						 guint                    arg_id);

static void   gnome_canvas_hacktext_update      (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);
static void   gnome_canvas_hacktext_realize     (GnomeCanvasItem *item);
static void   gnome_canvas_hacktext_unrealize   (GnomeCanvasItem *item);
static void   gnome_canvas_hacktext_draw        (GnomeCanvasItem *item, GdkDrawable *drawable,
						int x, int y, int width, int height);
static double gnome_canvas_hacktext_point       (GnomeCanvasItem *item, double x, double y,
						int cx, int cy, GnomeCanvasItem **actual_item);
static void   gnome_canvas_hacktext_bounds      (GnomeCanvasItem *item, double *x1, double *y1, double *x2, double *y2);
static void   gnome_canvas_hacktext_render      (GnomeCanvasItem *item, GnomeCanvasBuf *buf);
static void   gnome_canvas_hacktext_req_repaint (GnomeCanvasHacktext *hacktext, ArtIRect *bbox);


static GnomeCanvasItemClass *parent_class;

struct _GnomeCanvasHacktextPriv {
	GnomeFont * font;
#if 0
	GnomeFontFace * face;
	GnomeRFont * rfont;
#endif
	GnomeGlyphList * glyphlist;
	GnomePosGlyphList * pgl;
	double affine[6]; /* the text to world transform (NB! mirrored Y) */
};

GtkType
gnome_canvas_hacktext_get_type (void)
{
	static GtkType hacktext_type = 0;

	if (!hacktext_type) {
		GtkTypeInfo hacktext_info = {
			"GnomeCanvasHacktext",
			sizeof (GnomeCanvasHacktext),
			sizeof (GnomeCanvasHacktextClass),
			(GtkClassInitFunc) gnome_canvas_hacktext_class_init,
			(GtkObjectInitFunc) gnome_canvas_hacktext_init,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};

		hacktext_type = gtk_type_unique (gnome_canvas_item_get_type (), &hacktext_info);
	}

	return hacktext_type;
}

static void
gnome_canvas_hacktext_class_init (GnomeCanvasHacktextClass *class)
{
	GtkObjectClass *object_class;
	GnomeCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) class;
	item_class = (GnomeCanvasItemClass *) class;

	parent_class = gtk_type_class (gnome_canvas_item_get_type ());

	/* when this gets checked into libgnomeui, change the
           GTK_TYPE_POINTER to GTK_TYPE_GNOME_CANVAS_HACKTEXT, and add an
           entry to gnome-boxed.defs */
	gtk_object_add_arg_type ("GnomeCanvasHacktext::text", GTK_TYPE_STRING, GTK_ARG_READWRITE, ARG_TEXT);
	gtk_object_add_arg_type ("GnomeCanvasHacktext::glyphlist", GTK_TYPE_POINTER, GTK_ARG_READWRITE, ARG_GLYPHLIST);
	gtk_object_add_arg_type ("GnomeCanvasHacktext::fill_color", GTK_TYPE_STRING, GTK_ARG_WRITABLE, ARG_FILL_COLOR);
	gtk_object_add_arg_type ("GnomeCanvasHacktext::fill_color_gdk", GTK_TYPE_GDK_COLOR, GTK_ARG_READWRITE, ARG_FILL_COLOR_GDK);
	gtk_object_add_arg_type ("GnomeCanvasHacktext::fill_color_rgba", GTK_TYPE_UINT, GTK_ARG_READWRITE, ARG_FILL_COLOR_RGBA);
	gtk_object_add_arg_type ("GnomeCanvasHacktext::font", GTK_TYPE_OBJECT, GTK_ARG_READWRITE, ARG_FONT);
	gtk_object_add_arg_type ("GnomeCanvasHacktext::x", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_X);
	gtk_object_add_arg_type ("GnomeCanvasHacktext::y", GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_Y);

	object_class->destroy = gnome_canvas_hacktext_destroy;
	object_class->set_arg = gnome_canvas_hacktext_set_arg;
	object_class->get_arg = gnome_canvas_hacktext_get_arg;

	item_class->update = gnome_canvas_hacktext_update;
	item_class->realize = gnome_canvas_hacktext_realize;
	item_class->unrealize = gnome_canvas_hacktext_unrealize;
	item_class->draw = gnome_canvas_hacktext_draw;
	item_class->point = gnome_canvas_hacktext_point;
	item_class->bounds = gnome_canvas_hacktext_bounds;
	item_class->render = gnome_canvas_hacktext_render;
}

static void
gnome_canvas_hacktext_init (GnomeCanvasHacktext *hacktext)
{
	hacktext->text = NULL;
	hacktext->priv = g_new (GnomeCanvasHacktextPriv, 1);
	hacktext->priv->font = NULL;
	hacktext->priv->glyphlist = NULL;
	hacktext->priv->pgl = NULL;

	art_affine_identity (hacktext->priv->affine);
}

static void
gnome_canvas_hacktext_destroy (GtkObject *object)
{
	GnomeCanvasHacktext *hacktext;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_HACKTEXT (object));

	hacktext = GNOME_CANVAS_HACKTEXT (object);

	if (hacktext->text)
		g_free (hacktext->text);

	if (hacktext->priv) {
		if (hacktext->priv->font) gnome_font_unref (hacktext->priv->font);
		if (hacktext->priv->glyphlist) gnome_glyphlist_unref (hacktext->priv->glyphlist);
		if (hacktext->priv->pgl) gnome_pgl_destroy (hacktext->priv->pgl);
		g_free (hacktext->priv);
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
art_drect_hacktext (ArtDRect *bbox, GnomeCanvasHacktext *hacktext)
{
	g_assert (bbox != NULL);
	g_assert (hacktext != NULL);

	g_return_if_fail (hacktext->priv);

	if (GTK_OBJECT_FLAGS (hacktext) & GNOME_CANVAS_UPDATE_REQUESTED) {
		gnome_canvas_update_now (GNOME_CANVAS_ITEM (hacktext)->canvas);
	}

	if (!hacktext->priv->pgl) return;

	gnome_pgl_bbox (hacktext->priv->pgl, bbox);
}

/* Computes the bounding box of the hacktext.  Assumes that the number of points in the hacktext is
 * not zero.
 */
static void
get_bounds (GnomeCanvasHacktext *hacktext, double *bx1, double *by1, double *bx2, double *by2)
{
	ArtDRect bbox;

	/* Compute bounds of hacktext */
	art_drect_hacktext (&bbox, hacktext);

	/* Done */

	*bx1 = bbox.x0;
	*by1 = bbox.y0;
	*bx2 = bbox.x1;
	*by2 = bbox.y1;
}

#ifdef IFED_OUT_BY_CHEMA_TO_KILL_COMPILE_WARNING
/* Convenience function to set a GC's foreground color to the specified pixel value */
static void
set_gc_foreground (GdkGC *gc, gulong pixel)
{
	GdkColor c;

	if (!gc)
		return;

	c.pixel = pixel;
	gdk_gc_set_foreground (gc, &c);
}
#endif

static void
gnome_canvas_hacktext_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GnomeCanvasItem *item;
	GnomeCanvasHacktext *bp;
	char *text;
	GnomeGlyphList * gl;
	GdkColor color;
	GnomeFont * font;

	item = GNOME_CANVAS_ITEM (object);
	bp = GNOME_CANVAS_HACKTEXT (object);

	switch (arg_id) {
	case ARG_TEXT:
		text = GTK_VALUE_POINTER (*arg);

		if (bp->text) {
			g_free (bp->text);
			bp->text = NULL;
		}

		if (bp->priv->glyphlist) {
			gnome_glyphlist_unref (bp->priv->glyphlist);
			bp->priv->glyphlist = NULL;
		}

		if (text) bp->text = g_strdup (text);

		gnome_canvas_item_request_update (item);
		break;

	case ARG_GLYPHLIST:
		gl = GTK_VALUE_POINTER (*arg);

		if (bp->text) {
			g_free (bp->text);
			bp->text = NULL;
		}

		if (bp->priv->glyphlist) {
			gnome_glyphlist_unref (bp->priv->glyphlist);
			bp->priv->glyphlist = NULL;
		}

		/* fixme: should be duplicate() */

		if (gl) gnome_glyphlist_ref (gl);

		bp->priv->glyphlist = gl;

		gnome_canvas_item_request_update (item);

		break;

	case ARG_FILL_COLOR:
		if (gnome_canvas_get_color (item->canvas, GTK_VALUE_STRING (*arg), &color)) {
			bp->fill_set = TRUE;
			bp->fill_pixel = color.pixel;
			bp->fill_rgba =
				((color.red & 0xff00) << 16) |
				((color.green & 0xff00) << 8) |
				(color.blue & 0xff00) |
				0xff;
		} else {
			bp->fill_set = FALSE;
			bp->fill_rgba = 0;
		}

		gnome_canvas_item_request_update (item);
		break;

	case ARG_FILL_COLOR_GDK:
		bp->fill_set = TRUE;
		bp->fill_pixel = ((GdkColor *) GTK_VALUE_BOXED (*arg))->pixel;
#if 0
		set_gc_foreground (bp->fill_gc, bp->fill_pixel);
#endif
		gnome_canvas_item_request_update (item);
		break;

	case ARG_FILL_COLOR_RGBA:
		bp->fill_set = TRUE;
		bp->fill_rgba = GTK_VALUE_UINT (*arg);

		/* should probably request repaint on the fill_svp */
		gnome_canvas_item_request_update (item);

		break;

	case ARG_FONT:
		font = GTK_VALUE_POINTER (*arg);
		if (font) gnome_font_ref (font);
		if (bp->priv->font) gnome_font_unref (bp->priv->font);
		bp->priv->font = font;
#if 0
		bp->priv->face = (GnomeFontFace *) gnome_font_get_face (bp->priv->font);
#endif
		bp->size = gnome_font_get_size (bp->priv->font);
		gnome_canvas_item_request_update (item);
		break;

	case ARG_X:
		bp->x = GTK_VALUE_DOUBLE (*arg);
		gnome_canvas_item_request_update (item);
		break;

	case ARG_Y:
		bp->y = GTK_VALUE_DOUBLE (*arg);
		gnome_canvas_item_request_update (item);
		break;

	default:
		break;
	}
}

/* Allocates a GdkColor structure filled with the specified pixel, and puts it into the specified
 * arg for returning it in the get_arg method.
 */
static void
get_color_arg (GnomeCanvasHacktext *hacktext, gulong pixel, GtkArg *arg)
{
	GdkColor *color;

	color = g_new (GdkColor, 1);
	color->pixel = pixel;
	gdk_color_context_query_color (hacktext->item.canvas->cc, color);
	GTK_VALUE_BOXED (*arg) = color;
}

static void
gnome_canvas_hacktext_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GnomeCanvasHacktext *bp;

	bp = GNOME_CANVAS_HACKTEXT (object);

	switch (arg_id) {
	case ARG_TEXT:
		if (bp->text) {
			GTK_VALUE_POINTER (*arg) = g_strdup (bp->text);
		} else
			GTK_VALUE_POINTER (*arg) = NULL;
		break;

	case ARG_FILL_COLOR_GDK:
		get_color_arg (bp, bp->fill_pixel, arg);
		break;
		
	case ARG_FILL_COLOR_RGBA:
		GTK_VALUE_UINT (*arg) = bp->fill_color;
		break;

	case ARG_FONT:
		GTK_VALUE_POINTER (*arg) = bp->priv->font;
		break;

	case ARG_X:
		GTK_VALUE_DOUBLE (*arg) = bp->x;
		break;

	case ARG_Y:
		GTK_VALUE_DOUBLE (*arg) = bp->y;
		break;

	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

static void
gnome_canvas_hacktext_update (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	GnomeCanvasHacktext *hacktext;
	ArtIRect ibbox = {0, 0, 0, 0};

	hacktext = (GnomeCanvasHacktext *) item;

	if (parent_class->update)
		(* parent_class->update) (item, affine, clip_path, flags);

	if (hacktext->priv->pgl) gnome_canvas_hacktext_req_repaint (hacktext, NULL);

	gnome_canvas_item_reset_bounds (item);

	hacktext->priv->affine[0] = affine[0];
	hacktext->priv->affine[1] = affine[1];
	hacktext->priv->affine[2] = -affine[2];
	hacktext->priv->affine[3] = -affine[3];
	hacktext->priv->affine[4] = affine[4] + hacktext->x * affine[0] + hacktext->y * affine[2];
	hacktext->priv->affine[5] = affine[5] + hacktext->x * affine[1] + hacktext->y * affine[3];

	if (hacktext->text) {
		if (hacktext->priv->glyphlist) {
			gnome_glyphlist_unref (hacktext->priv->glyphlist);
			hacktext->priv->glyphlist = NULL;
		}

		if (!hacktext->priv->font) return;

		hacktext->priv->glyphlist = gnome_glyphlist_from_text_dumb (hacktext->priv->font, hacktext->fill_rgba,
									    0.0, 0.0,
									    hacktext->text);
	}

	if (hacktext->priv->glyphlist) {
		GnomePosGlyphList * pgl;

		pgl = gnome_pgl_from_gl (hacktext->priv->glyphlist, hacktext->priv->affine, GNOME_PGL_RENDER_DEFAULT);

		if (hacktext->priv->pgl) gnome_pgl_destroy (hacktext->priv->pgl);

		hacktext->priv->pgl = pgl;
	}
	       
	gnome_canvas_hacktext_req_repaint (hacktext, &ibbox);

	hacktext->item.x1 = ibbox.x0;
	hacktext->item.y1 = ibbox.y0;
	hacktext->item.x2 = ibbox.x1;
	hacktext->item.y2 = ibbox.y1;
}

static void
gnome_canvas_hacktext_realize (GnomeCanvasItem *item)
{
	GnomeCanvasHacktext *hacktext;

	hacktext = (GnomeCanvasHacktext *) item;

	if (parent_class->realize)
		(* parent_class->realize) (item);
}

static void
gnome_canvas_hacktext_unrealize (GnomeCanvasItem *item)
{
	GnomeCanvasHacktext *hacktext;

	hacktext = (GnomeCanvasHacktext *) item;

	if (parent_class->unrealize)
		(* parent_class->unrealize) (item);
}

static double
gnome_canvas_hacktext_point (GnomeCanvasItem *item, double mx, double my,
			    int cx, int cy, GnomeCanvasItem **actual_item)
{
	GnomeCanvasHacktext * hacktext;
	gdouble dist, best;
	gint s;

	hacktext = (GnomeCanvasHacktext *) item;

	if (!hacktext->priv->pgl) return 1e18;

	*actual_item = item;
	best = dist = 1e18;

	for (s = 0; s < hacktext->priv->pgl->num_strings; s++) {
		GnomePosString * string;
		gint i;

		string = hacktext->priv->pgl->strings + s;

		for (i = string->start; i < string->start + string->length; i++) {
			const ArtSVP * svp;
			ArtWindRule wind;
			gdouble x, y;

			x = cx - hacktext->priv->pgl->glyphs[i].x;
			y = cy - hacktext->priv->pgl->glyphs[i].y;

			svp = gnome_rfont_get_glyph_svp (string->rfont, hacktext->priv->pgl->glyphs[i].glyph);

			if (svp) {
				wind = art_svp_point_wind ((ArtSVP *) svp, x, y);
				if (wind) return 0.0;
				dist = art_svp_point_dist ((ArtSVP *) svp, x, y);
				if (dist < best) best = dist;
			}
		}
	}

	return dist;
}

static void
gnome_canvas_hacktext_bounds (GnomeCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	GnomeCanvasHacktext *hacktext;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GNOME_IS_CANVAS_HACKTEXT (item));

	hacktext = GNOME_CANVAS_HACKTEXT (item);

	if (hacktext->text == NULL) {
		*x1 = *y1 = *x2 = *y2 = 0.0;
		return;
	}

	get_bounds (hacktext, x1, y1, x2, y2);
}

static void
gnome_canvas_hacktext_req_repaint (GnomeCanvasHacktext *hacktext,
				   ArtIRect *bbox)
{
	gint s;

	g_return_if_fail (hacktext->priv);

	if (!hacktext->priv->pgl) return;

	for (s = 0; s < hacktext->priv->pgl->num_strings; s++) {
		GnomePosString * string;
		gint i;

		string = hacktext->priv->pgl->strings + s;

		for (i = string->start; i < string->start + string->length; i++) {
			ArtDRect gbbox;
			ArtIRect ibox;

			gnome_rfont_get_glyph_stdbbox (string->rfont, hacktext->priv->pgl->glyphs[i].glyph, &gbbox);

			gbbox.x0 += hacktext->priv->pgl->glyphs[i].x;
			gbbox.y0 += hacktext->priv->pgl->glyphs[i].y;
			gbbox.x1 += hacktext->priv->pgl->glyphs[i].x;
			gbbox.y1 += hacktext->priv->pgl->glyphs[i].y;

			art_drect_to_irect (&ibox, &gbbox);

			gnome_canvas_request_redraw (hacktext->item.canvas,
						     ibox.x0, ibox.y0,
						     ibox.x1, ibox.y1);

			if (bbox) art_irect_union (bbox, bbox, &ibox);
		}
	}
}

static void
gnome_canvas_hacktext_render (GnomeCanvasItem *item,
			      GnomeCanvasBuf *buf)
{
	GnomeCanvasHacktext * hacktext;

	hacktext = (GnomeCanvasHacktext *) item;

	g_return_if_fail (hacktext->priv);

	if (!hacktext->priv->pgl) return;

	gnome_canvas_buf_ensure_buf (buf);
	buf->is_buf = TRUE;
	buf->is_bg = FALSE;

	gnome_rfont_render_pgl_rgb8 (hacktext->priv->pgl,
				     -buf->rect.x0, -buf->rect.y0,
				     buf->buf,
				     buf->rect.x1 - buf->rect.x0,
				     buf->rect.y1 - buf->rect.y0,
				     buf->buf_rowstride,
				     GNOME_RFONT_RENDER_DEFAULT);
}

static void
gnome_canvas_hacktext_draw (GnomeCanvasItem *item, GdkDrawable *drawable,
			   int x, int y, int width, int height)
{
	GnomeCanvasHacktext * hacktext;

	hacktext = (GnomeCanvasHacktext *) item;

	g_return_if_fail (hacktext->priv);

	if (!hacktext->priv->pgl) return;

	gnome_rfont_render_pgl_gdk_drawable (hacktext->priv->pgl,
					     -x, -y,
					     drawable,
					     0xffffffff,
					     GNOME_RFONT_RENDER_DEFAULT);
}













