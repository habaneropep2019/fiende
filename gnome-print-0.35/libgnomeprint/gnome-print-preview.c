/*
 * gnome-print-preview.c: A preview driver for GnomePrint that renders
 * into a GnomeCanvas and a control object for the pages rendered.
 *
 * Authors:
 *   Miguel de Icaza (miguel@gnu.org)
 *   Lauris Kaplinski (lauris@ariman.ee)
 *
 * (C) 1999, 2000 International GNOME Support
 */

#include <config.h>
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>

#include <libgnomeui/gnome-canvas-image.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-print-preview-private.h>
#include <libgnomeprint/gnome-canvas-bpath.h>
#include <libgnomeprint/gnome-canvas-hacktext.h>
#include <libgnomeprint/gnome-canvas-clipgroup.h>
#include <libgnomeprint/gnome-font.h>
#include <libart_lgpl/art_pixbuf.h>
#include <libgnome/gnome-paper.h>

#include <libgnomeprint/gp-gc.h>

struct _GnomePrintPreviewPrivate {
	GPGC * gc;

	/* Current page displayed */
	int top_page;
	int current_page;

	/* The root group, with a translation setup */
	GnomeCanvasItem *root;

	/* The current page */
	GnomeCanvasItem *page;
};

static GnomePrintContextClass *print_preview_parent_class;

static int
gpp_stroke (GnomePrintContext *pc)
{
	GnomePrintPreview *pp = GNOME_PRINT_PREVIEW (pc);
	GnomePrintPreviewPrivate *priv = pp->priv;
	GnomeCanvasGroup *group;
	GnomeCanvasItem *item;

	/* fixme: currentpath invariants */

	group = (GnomeCanvasGroup *) gp_gc_get_data (priv->gc);
	g_assert (group != NULL);
	g_assert (GNOME_IS_CANVAS_GROUP (group));

	/* Fixme: Can we assume that linewidth == width_units? */
	/* Probably yes, as object->page ctm == identity */

	item = gnome_canvas_item_new (group,
		gnome_canvas_bpath_get_type (),
		"bpath",	gp_gc_get_currentpath (pc->gc),
		"width_units",	gp_gc_get_linewidth (pc->gc),
		"cap_style",	gp_gc_get_linecap (pc->gc),
		"join_style",	gp_gc_get_linejoin (pc->gc),
		"outline_color_rgba", gp_gc_get_rgba (pc->gc),
		"miterlimit",	gp_gc_get_miterlimit (pc->gc),
		"dash",		gp_gc_get_dash (pc->gc),
		NULL);

	return 1;
}

static int
gpp_create_fill (GnomePrintPreview * pp, ArtWindRule wind)
{
	GnomePrintContext * pc;
	GnomePrintPreviewPrivate * priv;
	GnomeCanvasGroup *group;
	GnomeCanvasItem *item;
	
	pc = GNOME_PRINT_CONTEXT (pp);
	priv = pp->priv;

	group = (GnomeCanvasGroup *) gp_gc_get_data (priv->gc);
	g_assert (group != NULL);
	g_assert (GNOME_IS_CANVAS_GROUP (group));

	item = gnome_canvas_item_new (group,
		gnome_canvas_bpath_get_type (),
		"bpath", gp_gc_get_currentpath (pc->gc),
		"outline_color", NULL,
		"fill_color_rgba", gp_gc_get_rgba (pc->gc),
		"wind", wind,
		NULL);

	return 1;
}

static int
gpp_fill (GnomePrintContext *pc, ArtWindRule rule)
{
	GnomePrintPreview *pp = GNOME_PRINT_PREVIEW (pc);

	return gpp_create_fill (pp, rule);
}

static int
gpp_create_clip (GnomePrintPreview * pp, ArtWindRule wind)
{
	GnomePrintContext * pc;
	GnomePrintPreviewPrivate * priv;
	GnomeCanvasGroup * group;
	GnomeCanvasItem * clip;

	pc = GNOME_PRINT_CONTEXT (pp);
	priv = pp->priv;

	group = (GnomeCanvasGroup *) gp_gc_get_data (priv->gc);

	clip = gnome_canvas_item_new (group,
		gnome_canvas_clipgroup_get_type (),
		"path", gp_gc_get_currentpath (pc->gc),
		"wind", wind,
		NULL);

	gp_gc_set_data (priv->gc, clip);

	return 1;
}

static int
gpp_clip (GnomePrintContext *pc, ArtWindRule rule)
{
	GnomePrintPreview *pp = GNOME_PRINT_PREVIEW (pc);

	return gpp_create_clip (pp, rule);
}

static int
gpp_image (GnomePrintContext *pc, const char *data, int width, int height, int rowstride, int bytes_per_pixel)
{
	GnomePrintPreview *pp = GNOME_PRINT_PREVIEW (pc);
	GnomeCanvasGroup * group;
	GnomeCanvasItem *item;
	ArtPixBuf *pixbuf;
	gint size, bpp;
        gchar *dup;
	gint y;
	
	/*
	 * We do convert gray scale images to RGB
	 */

	if (bytes_per_pixel == 1)
		bpp = 3;
	else
		bpp = bytes_per_pixel;
	
	size = width * height * bpp;
	dup = art_alloc (size);
	if (!dup) return -1;

	if (bytes_per_pixel == 3) {
		for (y = 0; y < height; y++) {
			memcpy (dup + y * width * 3, data + y * rowstride, width * 3);
		}
		pixbuf = art_pixbuf_new_rgb (dup, width, height, rowstride);
	} else if (bytes_per_pixel == 4){
		for (y = 0; y < height; y++) {
			memcpy (dup + y * width * 4, data + y * rowstride, width * 4);
		}
		pixbuf = art_pixbuf_new_rgba (dup, width, height, rowstride);
	} else if (bytes_per_pixel == 1){
		for (y = 0; y < height; y++) {
			gchar *sp, *dp;
			gint x;
			sp = (gchar *) data + y * rowstride;
			dp = dup + y * width * 3;
			for (x = 0; x < width; x++){
				*dp++ = *sp;
				*dp++ = *sp;
				*dp++ = *sp;
				sp++;
			}
		}
		pixbuf = art_pixbuf_new_rgb (dup, width, height, width * 3);
	} else
		return -1;

	group = (GnomeCanvasGroup *) gp_gc_get_data (pp->priv->gc);

	item = gnome_canvas_item_new (group,
		gnome_canvas_image_get_type (),
		"pixbuf", pixbuf,
		"x",      0.0,
		"y",      0.0,
		"width",  (gdouble) width,
		"height", (gdouble) height,
		"anchor", GTK_ANCHOR_NW,
		NULL);


	/* Apply the transformation for the image */
	{
		double transform [6];
		double flip [6];
		flip[0] = 1.0 / width;
		flip[1] = 0.0;
		flip[2] = 0.0;
		flip[3] = -1.0 / height;
		flip[4] = 0.0;
		flip[5] = 1.0;
		art_affine_multiply (transform, flip, gp_gc_get_ctm (pc->gc));

		gnome_canvas_item_affine_absolute (item, transform);
	}
	
	return 1;
}

static int
gpp_grayimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride)
{
	return  gpp_image (pc, data, width, height, rowstride, 1);
}

static int
gpp_rgbimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride)
{
	return gpp_image (pc, data, width, height, rowstride, 3);
}


static int
gpp_rgbaimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride)
{
	return gpp_image (pc, data, width, height, rowstride, 4);
}


static int
gpp_textline (GnomePrintContext *pc, GnomeTextLine *line)
{
	g_warning ("Implement textline");

	return -1;
}

static int
gpp_showpage (GnomePrintContext *pc)
{
	return 0;
}

static int
gpp_beginpage (GnomePrintContext *pc, const char *name_of_this_page)
{
	return 0;
}

static int
gpp_show_sized (GnomePrintContext *pc, const char *text, int bytes)
{
	GnomePrintPreview *pp = GNOME_PRINT_PREVIEW (pc);
	GnomeCanvasGroup *group;
	GnomeCanvasItem *item;
	const ArtPoint * cp;
	const GnomeFont * font;
	ArtPoint p;
	double transform [6], a[6], inverse[6];
	GnomeGlyphList * gl;

	font = gp_gc_get_font (pc->gc);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0);

	/*
	 * The X and Y positions were computed to be the base
	 * with the translation already done for the new
	 * Postscript->Canvas translation
	 */
	
	cp = gp_gc_get_currentpoint (pc->gc);

	memcpy (transform, gp_gc_get_ctm (pc->gc), sizeof (transform));
	art_affine_scale (a, 1.0, -1.0);
	art_affine_multiply (transform, a, transform);

	art_affine_invert (inverse, transform);
	art_affine_point (&p, cp, inverse);
	group = (GnomeCanvasGroup *) gp_gc_get_data (pp->priv->gc);

	gl = gnome_glyphlist_from_text_sized_dumb ((GnomeFont *) font, gp_gc_get_rgba (pc->gc),
						   0.0, 0.0, text, bytes);

	item = gnome_canvas_item_new (group,
		gnome_canvas_hacktext_get_type (),
		"x",           p.x,
		"y",           p.y,
		"glyphlist",   gl,
		NULL);

	gnome_glyphlist_unref (gl);

	gnome_canvas_item_affine_absolute (item, transform);

	gp_gc_moveto (pc->gc, cp->x + gnome_font_get_width_string_n (font, text, bytes), cp->y);
	
	return 0;
}

static int
gpp_glyphlist (GnomePrintContext *pc, GnomeGlyphList * glyphlist)
{
	GnomePrintPreview *pp = GNOME_PRINT_PREVIEW (pc);
	GnomeCanvasGroup *group;
	GnomeCanvasItem *item;
	const ArtPoint zeropoint = {0.0, 0.0};
	const ArtPoint * cp;
	ArtPoint p;
	double transform [6], a[6], inverse[6];


	/*
	 * The X and Y positions were computed to be the base
	 * with the translation already done for the new
	 * Postscript->Canvas translation
	 */
	
	if (gp_gc_has_currentpoint (pc->gc)) {
		cp = gp_gc_get_currentpoint (pc->gc);
	} else {
		cp = &zeropoint;
	}

	memcpy (transform, gp_gc_get_ctm (pc->gc), sizeof (transform));
	art_affine_scale (a, 1.0, -1.0);
	art_affine_multiply (transform, a, transform);

	art_affine_invert (inverse, transform);
	art_affine_point (&p, cp, inverse);

	group = (GnomeCanvasGroup *) gp_gc_get_data (pp->priv->gc);

	item = gnome_canvas_item_new (group,
		gnome_canvas_hacktext_get_type (),
		"x",           p.x,
		"y",           p.y,
		"glyphlist",        glyphlist,
		NULL);

	gnome_canvas_item_affine_absolute (item, transform);

	return 0;
}

static gint
gpp_gsave (GnomePrintContext *ctx)
{
	GnomePrintPreview *pp;

	pp = GNOME_PRINT_PREVIEW (ctx);

	gp_gc_gsave (pp->priv->gc);

	return GNOME_PRINT_OK;
}

static gint
gpp_grestore (GnomePrintContext *ctx)
{
	GnomePrintPreview *pp;

	pp = GNOME_PRINT_PREVIEW (ctx);

	gp_gc_grestore (pp->priv->gc);

	return GNOME_PRINT_OK;
}

static int
gpp_close (GnomePrintContext *pc)
{
	return 0;
}

static void
gpp_destroy (GtkObject *object)
{
	GnomePrintPreview *pp = GNOME_PRINT_PREVIEW (object);
	GnomePrintPreviewPrivate *priv = pp->priv;

	gp_gc_unref (priv->gc);

	if (pp->canvas)
		gtk_object_unref (GTK_OBJECT (pp->canvas));

	if (priv->page)
		gtk_object_destroy (GTK_OBJECT (priv->page));

	if (priv->root)
		gtk_object_destroy (GTK_OBJECT (priv->root));
	
	g_free (priv);

	GTK_OBJECT_CLASS (print_preview_parent_class)->destroy (object);
}

static void
gpp_class_init (GnomePrintPreviewClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	GnomePrintContextClass *pc_class = (GnomePrintContextClass *) class;

	print_preview_parent_class = gtk_type_class (gnome_print_context_get_type ());

	object_class->destroy = gpp_destroy;
	
	pc_class->fill = gpp_fill;
	pc_class->stroke = gpp_stroke;
	pc_class->show_sized = gpp_show_sized;
	pc_class->clip = gpp_clip;
	pc_class->grayimage = gpp_grayimage;
	pc_class->rgbimage = gpp_rgbimage;
	pc_class->rgbaimage = gpp_rgbaimage;
	pc_class->textline = gpp_textline;
	pc_class->showpage = gpp_showpage;
	pc_class->beginpage = gpp_beginpage;
	pc_class->glyphlist = gpp_glyphlist;
	pc_class->gsave = gpp_gsave;
	pc_class->grestore = gpp_grestore;
	
	pc_class->close = gpp_close;
}

static void
gpp_init (GnomePrintPreview *preview)
{
	GnomePrintPreviewPrivate *priv;
	
	priv = preview->priv = g_new0 (GnomePrintPreviewPrivate, 1);

	priv->gc = gp_gc_new ();
}

static void
clear_val (GtkObject *object, void **val)
{
	*val = NULL;
}

/**
 * gnome_print_preview_construct:
 * @preview: the #GnomePrintPreview object to construct
 * @canvas: Canvas on which the preview will render
 * @paper_info: a GnomePaper information
 *
 * Constructs the @preview object.
 */
void
gnome_print_preview_construct (GnomePrintPreview *preview,
			       GnomeCanvas *canvas,
			       const GnomePaper *paper_info)
{
	GnomeCanvasGroup * group;
	double page2root[6];

	g_return_if_fail (preview != NULL);
	g_return_if_fail (GNOME_IS_PRINT_PREVIEW (preview));
	g_return_if_fail (canvas != NULL);
	g_return_if_fail (GNOME_IS_CANVAS (canvas));

	if (!paper_info) {
		g_warning ("file %s: line %d: Missing paper info", __FILE__, __LINE__);
	}

	gtk_object_ref (GTK_OBJECT (canvas));
	preview->canvas = canvas;

	if (getenv ("GNOME_PRINT_DEBUG_WIDE"))
		gnome_canvas_set_scroll_region (
			canvas,	-900, -900, 900, 900);
	else
		gnome_canvas_set_scroll_region (
			canvas, 0, 0,
			paper_info ? gnome_paper_pswidth (paper_info) : 21.0 * 72 / 2.54,
			paper_info ? gnome_paper_psheight (paper_info) : 29.7 * 72 / 2.54);

	preview->priv->root = gnome_canvas_item_new (
		gnome_canvas_root (preview->canvas),
		gnome_canvas_group_get_type (),
		"x", 0.0,
		"y", 0.0,
		NULL);

	preview->priv->page = gnome_canvas_item_new (
		gnome_canvas_root (preview->canvas),
		gnome_canvas_group_get_type (),
		"x", 0.0,
		"y", 0.0,
		NULL);

	gtk_signal_connect (GTK_OBJECT (preview->priv->page), "destroy",
			    GTK_SIGNAL_FUNC (clear_val), &preview->priv->page);
	gtk_signal_connect (GTK_OBJECT (preview->priv->root), "destroy",
			    GTK_SIGNAL_FUNC (clear_val), &preview->priv->root);
	/*
	 * Setup base group
	 */

	group = GNOME_CANVAS_GROUP (preview->priv->page);

	gp_gc_set_data (preview->priv->gc, group);

	/*
	 * Setup the affines
	 */

	art_affine_scale (page2root, 1.0, -1.0);
	page2root[5] = paper_info ? gnome_paper_psheight (paper_info) : 29.7 * 72 / 2.54;

	gnome_canvas_item_affine_absolute (preview->priv->page, page2root);
}

/**
 * gnome_print_preview_new:
 * @canvas: Canvas on which we display the print preview
 * @paper_size: A valid name for a paper size
 *
 * Creates a new PrintPreview object that use the @canvas GnomeCanvas 
 * as its rendering backend.
 *
 * Returns: A GnomePrintContext suitable for using with the GNOME print API.
 */
GnomePrintContext *
gnome_print_preview_new (GnomeCanvas *canvas, const char *paper_size)
{
	GnomePrintPreview *preview;
	const GnomePaper *paper_info;
	
	g_return_val_if_fail (canvas != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_CANVAS (canvas), NULL);
	g_return_val_if_fail (paper_size != NULL, NULL);

	paper_info = gnome_paper_with_name (paper_size);

	preview = gtk_type_new (gnome_print_preview_get_type ());
	if (preview == NULL)
		return NULL;

	gnome_print_preview_construct (preview, canvas, paper_info);
	return GNOME_PRINT_CONTEXT (preview);
}

/**
 * gnome_print_preview_get_type:
 *
 * GTK type identification routine for #GnomePrintPreview
 *
 * Returns: The Gtk type for the #GnomePrintPreview object
 */
GtkType
gnome_print_preview_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnomePrintPreview",
			sizeof (GnomePrintPreview),
			sizeof (GnomePrintPreviewClass),
			(GtkClassInitFunc) gpp_class_init,
			(GtkObjectInitFunc) gpp_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		type = gtk_type_unique (gnome_print_context_get_type (), &info);
	}
	return type;
}

