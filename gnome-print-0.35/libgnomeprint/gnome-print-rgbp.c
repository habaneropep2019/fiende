/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-print-rgbp.c: A preview driver abstract class for GnomePrint
 * that renders into an RGB buffer.
 *
 * This implementation uses the GnomePrintPreview driver.
 *
 *
 * Concept implementation.  Nothing more, nothing less.
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org)
 *
 * (C) 1999 International GNOME Support
 */
#include <config.h>
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>

#define USE_BANDS
#include <libgnomeui/gnome-canvas.h>
#include <libgnomeui/gnome-canvas-image.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-print-rgbp.h>

static GnomePrintPreviewClass *print_rgbp_parent_class;
struct _GnomePrintRGBPPrivate {
	GtkWidget *canvas;

	/* Width and Height in pixels */
	unsigned int width;
	unsigned int height;

	/* Paper Name */
	GnomePaper * paper;

	int dpi;
	GnomeCanvasBuf buf;
};

/**
 * gnome_print_rgbp_construct:
 * @rgbp: The GnomePrintRGBP object to be constructed.
 * @paper_info: GnomePaper object.
 * @dpi: resolution (in dots per inch) for the this RGBP object.
 *
 * Returns: a constructed GnomePrintRGBP object or NULL if there
 * is an error during initialization.
 */
GnomePrintRGBP *
gnome_print_rgbp_construct (GnomePrintRGBP   *rgbp,
			    const GnomePaper *paper_info,
			    int               dpi)
{
	gdouble pw, ph;

	if (paper_info == NULL) {
		g_warning ("file %s: line %d: Missing paper info", __FILE__, __LINE__);
		pw = 21 * 72 / 2.54;
		ph = 29.7 * 72 / 2.54;
	} else {
		pw = gnome_paper_pswidth (paper_info);
		ph = gnome_paper_psheight (paper_info);
	}

	/*
	 * RGB (3 bytes) * dots-per-inch * page-width-in-inches * dpi-height
	 */
	rgbp->priv->dpi = dpi;

	rgbp->priv->width = dpi * (pw / 72.0);
	rgbp->priv->height = dpi * (ph / 72.0);

	/*
	 * The buffer is big enough to hold a band (one inch).
	 */
	rgbp->priv->buf.buf_rowstride = 3 * rgbp->priv->width;
	rgbp->priv->buf.buf = g_malloc (rgbp->priv->buf.buf_rowstride * dpi);

	if (rgbp->priv->buf.buf == NULL)
		return NULL;
	
	/*
	 * Initialize base class
	 */
	rgbp->priv->canvas = gnome_canvas_new_aa ();
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (rgbp->priv->canvas), dpi/72);
	gnome_print_preview_construct (
		GNOME_PRINT_PREVIEW (rgbp),
		GNOME_CANVAS (rgbp->priv->canvas),
		paper_info);
	
	/* Save the paper name for multiple pages */
	rgbp->priv->paper = (GnomePaper *) paper_info;

	/* White background */
	rgbp->priv->buf.bg_color = 0xffffff;
	rgbp->priv->buf.is_bg = 1;
	rgbp->priv->buf.is_buf = 1;

	return rgbp;
}

/**
 * gnome_print_rgbp_new:
 * @paper_size: Initial paper size.
 * @dpi: resolution (in dots per inch) for the this RGBP object.
 *
 * Returns: a new GnomePrintRGBP object or NULL if there is an error
 * during initialization.
 */
GnomePrintContext *
gnome_print_rgbp_new (const char *paper_size, int dpi)
{
	const GnomePaper *paper_info;
	GnomePrintRGBP *rgbp;
	
	g_return_val_if_fail (paper_size != NULL, NULL);
	g_return_val_if_fail (dpi > 0, NULL);
	
	paper_info = gnome_paper_with_name (paper_size);
	if (paper_info == NULL) {
		g_warning ("file %s: line %d: Cannot get info for paper %s", __FILE__, __LINE__, paper_size);
	}

	rgbp = gtk_type_new (gnome_print_rgbp_get_type ());
	if (rgbp == NULL)
		return NULL;

	if (gnome_print_rgbp_construct (rgbp, paper_info, dpi)){
		gtk_object_unref (GTK_OBJECT (rgbp));
		return NULL;
	}
	
	return GNOME_PRINT_CONTEXT (rgbp);
}

static void
rgbp_finalize (GtkObject *object)
{
	GnomePrintRGBP *rgbp = GNOME_PRINT_RGBP (object);

	if (rgbp->priv->buf.buf)
		g_free (rgbp->priv->buf.buf);

	if (rgbp->priv->canvas)
		gtk_object_unref (GTK_OBJECT (rgbp->priv->canvas));

	g_free (rgbp->priv);
	
	GTK_OBJECT_CLASS (print_rgbp_parent_class)->finalize (object);
}

static void
rgbp_init (GnomePrintRGBP *rgbp)
{
	GnomePrintRGBPPrivate *priv;
	
	priv = rgbp->priv = g_new0 (GnomePrintRGBPPrivate, 1);
}

static int
rgbp_print_init (GnomePrintRGBP *rgbp, int dpi)
{
	return 0;
}

static int
rgbp_page_begin (GnomePrintRGBP *rgbp)
{
	return 0;
}

static int
rgbp_page_end (GnomePrintRGBP *rgbp)
{
	return 0;
}

static int
rgbp_print_band (GnomePrintRGBP *rgbp, guchar *rgb_buffer, ArtIRect *boundaries)
{
	return 0;
}

static int
rgbp_showpage (GnomePrintContext *pc)
{
	GnomePrintRGBP *rgbp = GNOME_PRINT_RGBP (pc);
	GnomePrintRGBPClass *rgb_class;
	GnomeCanvasItemClass *item_class;
	GnomeCanvas *canvas = GNOME_CANVAS (rgbp->priv->canvas);
	GnomeCanvasItem *root = GNOME_CANVAS_ITEM (gnome_canvas_root (canvas));
	int dpi, y0;
	double identity [6];

	/*
	 * One band per inch
	 */
	dpi = rgbp->priv->dpi;

	rgb_class = GNOME_PRINT_RGBP_CLASS (GTK_OBJECT (pc)->klass);
	item_class = GNOME_CANVAS_ITEM_CLASS (GTK_OBJECT (root)->klass);

	art_affine_identity (identity);
	item_class->update (root, identity, NULL,
			    GNOME_CANVAS_UPDATE_REQUESTED |
			    GNOME_CANVAS_UPDATE_AFFINE |
			    GNOME_CANVAS_UPDATE_CLIP |
			    GNOME_CANVAS_UPDATE_VISIBILITY);

	for (y0 = 0; y0 < rgbp->priv->height; y0 += dpi){
		int size;
		ArtIRect *rect = &rgbp->priv->buf.rect;
		
		rect->x0 = 0;
		rect->x1 = rgbp->priv->width;
		rect->y0 = y0;
		rect->y1 = y0 + dpi;

		/* Clear the background */
		memset (rgbp->priv->buf.buf, 0xff, dpi * rgbp->priv->buf.buf_rowstride);

		/* Render the band */
		item_class->render (root, &rgbp->priv->buf);

		size = (rect->x1 - rect->x0) * (rect->y1 - rect->y0) * 3;

		rgb_class->print_band (rgbp, rgbp->priv->buf.buf, &rgbp->priv->buf.rect);
	}

	/* We destroy the Canvas, and create a new one,
	   so that we clear all the objects */
	gtk_object_unref (GTK_OBJECT (rgbp->priv->canvas));
	rgbp->priv->canvas = gnome_canvas_new_aa ();
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (rgbp->priv->canvas), dpi/72);
	gnome_print_preview_construct (
		GNOME_PRINT_PREVIEW (rgbp),
		GNOME_CANVAS (rgbp->priv->canvas),
		rgbp->priv->paper);

	rgb_class->page_end (rgbp);
	
	return 1;
}

static int
rgbp_beginpage (GnomePrintContext *rgbp, const char *name_of_this_page)
{
	/* Nothing.  */
	return 0;
}


static void
rgbp_class_init (GnomePrintRGBPClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	GnomePrintContextClass *pc_class = (GnomePrintContextClass *) class;

	print_rgbp_parent_class = gtk_type_class (gnome_print_preview_get_type ());

	object_class->finalize = rgbp_finalize;

	pc_class->showpage = rgbp_showpage;
	pc_class->beginpage = rgbp_beginpage;

	class->print_init = rgbp_print_init;
	class->page_begin = rgbp_page_begin;
	class->print_band = rgbp_print_band;
	class->page_end   = rgbp_page_end;
}

GtkType
gnome_print_rgbp_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnomePrintRGBP",
			sizeof (GnomePrintRGBP),
			sizeof (GnomePrintRGBPClass),
			(GtkClassInitFunc) rgbp_class_init,
			(GtkObjectInitFunc) rgbp_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		type = gtk_type_unique (gnome_print_preview_get_type (), &info);
	}
	return type;
}

