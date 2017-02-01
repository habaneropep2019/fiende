/*
 * gnome-print-pixbuf.c: A pixbuf driver for GnomePrint.
 *
 * This implementation uses the GnomePrintRBuf driver to render separate pages
 * into memory. When "showpage" is encountered, signal "showpixbuf" is emitted,
 * and page cleared
 *
 * Authors:
 *   Miguel de Icaza (miguel@gnu.org)
 *   Lauris Kaplinski (lauris@ariman.ee)
 *
 * (C) 1999 International GNOME Support
 */

#include <config.h>
#include <math.h>
#include <stdio.h>
#include <libart_lgpl/art_affine.h>
#include <libgnomeprint/gnome-print-pixbuf.h>
#include <libgnomeprint/gnome-print-rbuf.h>
#include <libgnomeprint/gnome-print-rbuf-private.h>

enum {
	SHOWPIXBUF,
	LAST_SIGNAL
};

typedef struct _GnomePrintPixbufPrivate	GnomePrintPixbufPrivate;

struct _GnomePrintPixbuf {
	GnomePrintRBuf rbuf;

	GnomePrintPixbufPrivate * private;
};

struct _GnomePrintPixbufClass {
	GnomePrintRBufClass parent_class;

	void (* showpixbuf) (GnomePrintPixbuf * gpb, GdkPixbuf * pixbuf, gint pagenum);
};

struct _GnomePrintPixbufPrivate {
	GdkPixbuf * pixbuf;

#if 0
	gint width, height;
#endif
	gdouble page2buf[6];
	gint pagenum;

	ArtDRect bbox;
	gdouble xdpi, ydpi;
	gboolean alpha;
};

static void gpix_class_init (GnomePrintPixbufClass * klass);
static void gpix_init (GnomePrintPixbuf * pixbuf);
static void gpix_destroy (GtkObject * object);

static gint gpix_close (GnomePrintContext * pc);
static gint gpix_showpage (GnomePrintContext * pc);

static void gpix_private_clear_pixbuf (GnomePrintPixbuf * gpb);
static void gpix_private_clip_viewport (GnomePrintPixbuf * gpb);

static GnomePrintRBufClass * parent_class;
static guint gpix_signals[LAST_SIGNAL] = {0};

GtkType
gnome_print_pixbuf_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnomePrintPixbuf",
			sizeof (GnomePrintPixbuf),
			sizeof (GnomePrintPixbufClass),
			(GtkClassInitFunc) gpix_class_init,
			(GtkObjectInitFunc) gpix_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};
		
		type = gtk_type_unique (gnome_print_rbuf_get_type (), &info);
	}
	return type;
}

static void
gpix_class_init (GnomePrintPixbufClass * klass)
{
	GtkObjectClass * object_class;
	GnomePrintContextClass * print_class;

	object_class = (GtkObjectClass *) klass;
	print_class = (GnomePrintContextClass *) klass;

	parent_class = gtk_type_class (gnome_print_rbuf_get_type ());

	gpix_signals[SHOWPIXBUF] = gtk_signal_new ("showpixbuf",
		GTK_RUN_FIRST,
		object_class->type,
		GTK_SIGNAL_OFFSET (GnomePrintPixbufClass, showpixbuf),
		gtk_marshal_NONE__POINTER_INT,
		GTK_TYPE_NONE, 2, GTK_TYPE_POINTER, GTK_TYPE_INT);
	gtk_object_class_add_signals (object_class,
		gpix_signals,
		LAST_SIGNAL);

	object_class->destroy = gpix_destroy;

	print_class->showpage = gpix_showpage;
	print_class->close = gpix_close;
}

static void
gpix_init (GnomePrintPixbuf * pixbuf)
{
	pixbuf->private = g_new (GnomePrintPixbufPrivate, 1);

	pixbuf->private->pixbuf = NULL;
}

static void
gpix_destroy (GtkObject * object)
{
	GnomePrintPixbuf * gpb;

	gpb = (GnomePrintPixbuf *) object;

	if (gpb->private) {
		if (gpb->private->pixbuf) gdk_pixbuf_unref (gpb->private->pixbuf);

		g_free (gpb->private);
		gpb->private = NULL;
	}

	if (((GtkObjectClass *) parent_class)->destroy)
		(* ((GtkObjectClass *) parent_class)->destroy) (object);
}

static int
gpix_close (GnomePrintContext *pc)
{
	GnomePrintPixbuf * gpb;
	GnomePrintPixbufPrivate * priv;
	gint ret;

	gpb = (GnomePrintPixbuf *) pc;
	priv = gpb->private;

	/* Destroyed object */
	g_return_val_if_fail (priv != NULL, -1);

	ret = 0;

	if (((GnomePrintContextClass *) parent_class)->close)
		ret = (* ((GnomePrintContextClass *) parent_class)->close) (pc);

	if (priv->pixbuf) {
		gdk_pixbuf_unref (priv->pixbuf);
		priv->pixbuf = NULL;
	}

	return ret;
}

static int
gpix_showpage (GnomePrintContext * pc)
{
	GnomePrintPixbuf * gpb;
	GnomePrintPixbufPrivate * priv;
	GdkPixbuf * pixbuf;
	gint ret;

	gpb = (GnomePrintPixbuf *) pc;
	priv = gpb->private;

	/* Destroyed object */
	g_return_val_if_fail (priv != NULL, -1);

	/* Closed context */
	g_assert (priv->pixbuf != NULL);

	ret = 0;

	if (((GnomePrintContextClass *) parent_class)->showpage)
		ret = (* ((GnomePrintContextClass *) parent_class)->showpage) (pc);

	/* This silly - I have to modify RBuf, to accept new buffer, instead */

	pixbuf = gdk_pixbuf_copy (priv->pixbuf);

	gtk_signal_emit (GTK_OBJECT (pc),
		gpix_signals[SHOWPIXBUF],
		pixbuf,
		priv->pagenum);

	/* Unref pixbuf */

	gdk_pixbuf_unref (pixbuf);

	priv->pagenum++;

	gpix_private_clear_pixbuf (gpb);
	gpix_private_clip_viewport (gpb);

	return ret;
}

GnomePrintContext *
gnome_print_pixbuf_construct (GnomePrintPixbuf * gpb,
	gdouble x0, gdouble y0,
	gdouble x1, gdouble y1,
	gdouble xdpi, gdouble ydpi,
	gboolean alpha)
{
	GnomePrintPixbufPrivate * priv;
	gint width, height;
	gdouble translate[6], scale[6];

	g_return_val_if_fail (gpb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_PIXBUF (gpb), NULL);
	g_return_val_if_fail (x1 > x0, NULL);
	g_return_val_if_fail (y1 > y0, NULL);
	g_return_val_if_fail (xdpi > 0.0, NULL);
	g_return_val_if_fail (ydpi > 0.0, NULL);

	priv = gpb->private;

	g_assert (priv != NULL);

	priv->bbox.x0 = x0;
	priv->bbox.y0 = y0;
	priv->bbox.x1 = x1;
	priv->bbox.y1 = y1;
	priv->xdpi = xdpi;
	priv->ydpi = ydpi;
	priv->alpha = alpha;

	priv->pagenum = 0;

	/* fixme: 72.0 or 72.some_fraction? */
	/* Rounde to next integer */
	/* fixme: We'll subtract a tiny value, to get over possible */
	/* fixme: rounding error if box really should be an integral */
	width = ceil ((x1 - x0) * xdpi / 72.0 - 1e-6);
	height = ceil ((y1 - y0) * ydpi / 72.0 - 1e-6);

	priv->pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
		alpha, 8,
		width,
		height);

	g_return_val_if_fail (priv->pixbuf != NULL, NULL);

	gpix_private_clear_pixbuf (gpb);

	/* Buffer starts from top left corner */
	art_affine_translate (translate, -x0, -y1);

	/* Scaling */
	art_affine_scale (scale, xdpi / 72.0, -ydpi / 72.0);

	/* Composite page-to-buffer transformation */
	art_affine_multiply (priv->page2buf, translate, scale);

	if (gnome_print_rbuf_construct (GNOME_PRINT_RBUF (gpb),
			gdk_pixbuf_get_pixels (gpb->private->pixbuf),
			width, height,
			gdk_pixbuf_get_rowstride (gpb->private->pixbuf),
			priv->page2buf,
			alpha)) {

		/* Rbuf constructing succeeded */

		gpix_private_clip_viewport (gpb);

		return GNOME_PRINT_CONTEXT (gpb);
	} else {
		return NULL;
	}
}

GnomePrintContext *
gnome_print_pixbuf_new (
	gdouble x0, gdouble y0,
	gdouble x1, gdouble y1,
	gdouble xdpi, gdouble ydpi,
	gboolean alpha)
{
	GnomePrintPixbuf *gpb;

	g_return_val_if_fail (x1 > x0, NULL);
	g_return_val_if_fail (y1 > y0, NULL);
	g_return_val_if_fail (xdpi > 0.0, NULL);
	g_return_val_if_fail (ydpi > 0.0, NULL);

	gpb = gtk_type_new (GNOME_TYPE_PRINT_PIXBUF);

	if (!gnome_print_pixbuf_construct (gpb, x0, y0, x1, y1, xdpi, ydpi, alpha)) {
		gtk_object_unref (GTK_OBJECT (gpb));
		return NULL;
	} else {
		return GNOME_PRINT_CONTEXT (gpb);
	}
}

GdkPixbuf *
gnome_print_pixbuf_get_pixbuf (GnomePrintPixbuf * gpb)
{
	g_return_val_if_fail (gpb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PRINT_PIXBUF (gpb), NULL);
	g_return_val_if_fail (gpb->private != NULL, NULL);

	return gpb->private->pixbuf;
}

gint
gnome_print_pixbuf_get_pagenum (GnomePrintPixbuf * gpb)
{
	g_return_val_if_fail (gpb != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_PIXBUF (gpb), -1);
	g_return_val_if_fail (gpb->private != NULL, -1);

	return gpb->private->pagenum;
}

/* Helpers */

static void
gpix_private_clear_pixbuf (GnomePrintPixbuf * gpb)
{
	GnomePrintPixbufPrivate * priv;
	gint x, y, width, height, rowstride;
	guchar * pixels, * p;

	g_assert (gpb != NULL);

	priv = gpb->private;

	g_assert (priv != NULL);
	g_assert (priv->pixbuf != NULL);

	width = gdk_pixbuf_get_width (priv->pixbuf);
	height = gdk_pixbuf_get_height (priv->pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (priv->pixbuf);
	pixels = gdk_pixbuf_get_pixels (priv->pixbuf);

	if (priv->alpha) {
		for (y = 0; y < height; y++) {
			p = pixels + y * rowstride;
			for (x = 0; x < width; x++) {
				*p++ = 0xff;
				*p++ = 0xff;
				*p++ = 0xff;
				*p++ = 0x00;
			}
		}
	} else {
		for (y = 0; y < height; y++) {
			p = pixels + y * rowstride;
			for (x = 0; x < width; x++) {
				*p++ = 0xff;
				*p++ = 0xff;
				*p++ = 0xff;
			}
		}
	}
}

static void
gpix_private_clip_viewport (GnomePrintPixbuf * gpb)
{
	GnomePrintContext * pc;
	GnomePrintPixbufPrivate * priv;

	g_assert (gpb != NULL);

	priv = gpb->private;

	g_assert (priv != NULL);

	pc = GNOME_PRINT_CONTEXT (gpb);

	gnome_print_newpath (pc);
	gnome_print_moveto (pc, priv->bbox.x0, priv->bbox.y0);
	gnome_print_lineto (pc, priv->bbox.x1, priv->bbox.y0);
	gnome_print_lineto (pc, priv->bbox.x1, priv->bbox.y1);
	gnome_print_lineto (pc, priv->bbox.x0, priv->bbox.y1);
	gnome_print_closepath (pc);
	gnome_print_clip (pc);
	gnome_print_newpath (pc);
}
