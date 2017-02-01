#define GNOME_CANVAS_CLIPGROUP_C

/*
 * Clipping group implementation for GnomeCanvas
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * TODO: Implement this in libgnomeui, possibly merge with real group
 *
 * Copyright (C) 1998,1999 The Free Software Foundation
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 */

#include <math.h>
#include <string.h>

#include <libart_lgpl/art_misc.h>
#include <libart_lgpl/art_rect.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_vpath_bpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_rect_svp.h>
#include <libart_lgpl/art_gray_svp.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_vpath_dash.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libart_lgpl/art_svp_point.h>
#include <libart_lgpl/art_svp_ops.h>
#include <libgnomeui/gnome-canvas.h>
#include <libgnomeui/gnome-canvas-util.h>
#include "gnome-canvas-clipgroup.h"

enum {
	ARG_0,
	ARG_PATH,
	ARG_WIND
};

/* Generic clipping stuff */

#define GCG_BUF_WIDTH 128
#define GCG_BUF_HEIGHT 128
#define GCG_BUF_PIXELS (GCG_BUF_WIDTH * GCG_BUF_HEIGHT)
#define GCG_BUF_SIZE (GCG_BUF_WIDTH * GCG_BUF_HEIGHT * 3)

#define noSHOW_SHADOW

static guchar *gcg_buf_new (void);
static void gcg_buf_free (guchar *buf);
static guchar *gcg_mask_new (void);
static void gcg_mask_free (guchar *mask);

static void gnome_canvas_clipgroup_class_init (GnomeCanvasClipgroupClass *class);
static void gnome_canvas_clipgroup_init (GnomeCanvasClipgroup *clipgroup);
static void gnome_canvas_clipgroup_destroy (GtkObject *object);
static void gnome_canvas_clipgroup_set_arg (GtkObject *object, GtkArg *arg, guint arg_id);
static void gnome_canvas_clipgroup_get_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static void gnome_canvas_clipgroup_update (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);

static void gnome_canvas_clipgroup_render (GnomeCanvasItem *item, GnomeCanvasBuf *buf);

static GnomeCanvasGroupClass *parent_class;

GtkType
gnome_canvas_clipgroup_get_type (void)
{
	static GtkType clipgroup_type = 0;

	if (!clipgroup_type) {
		GtkTypeInfo clipgroup_info = {
			"GnomeCanvasClipgroup",
			sizeof (GnomeCanvasClipgroup),
			sizeof (GnomeCanvasClipgroupClass),
			(GtkClassInitFunc) gnome_canvas_clipgroup_class_init,
			(GtkObjectInitFunc) gnome_canvas_clipgroup_init,
			NULL, NULL, NULL
		};

		clipgroup_type = gtk_type_unique (gnome_canvas_group_get_type (), &clipgroup_info);
	}

	return clipgroup_type;
}

static void
gnome_canvas_clipgroup_class_init (GnomeCanvasClipgroupClass *class)
{
	GtkObjectClass *object_class;
	GnomeCanvasItemClass *item_class;

	object_class = (GtkObjectClass *) class;
	item_class = (GnomeCanvasItemClass *) class;

	parent_class = gtk_type_class (gnome_canvas_group_get_type ());

	gtk_object_add_arg_type ("GnomeCanvasClipgroup::path", GTK_TYPE_POINTER, GTK_ARG_READWRITE, ARG_PATH);
	gtk_object_add_arg_type ("GnomeCanvasClipgroup::wind", GTK_TYPE_ENUM, GTK_ARG_READWRITE, ARG_WIND);

	object_class->destroy = gnome_canvas_clipgroup_destroy;
	object_class->set_arg = gnome_canvas_clipgroup_set_arg;
	object_class->get_arg = gnome_canvas_clipgroup_get_arg;

	item_class->update = gnome_canvas_clipgroup_update;
	item_class->render = gnome_canvas_clipgroup_render;
}

static void
gnome_canvas_clipgroup_init (GnomeCanvasClipgroup *clipgroup)
{
	clipgroup->path = NULL;
	clipgroup->wind = ART_WIND_RULE_NONZERO;
	clipgroup->svp = NULL;
}

static void
gnome_canvas_clipgroup_destroy (GtkObject *object)
{
	GnomeCanvasClipgroup *clipgroup;

	clipgroup = GNOME_CANVAS_CLIPGROUP (object);

	if (clipgroup->path) {
		gp_path_unref (clipgroup->path);
		clipgroup->path = NULL;
	}
	
	if (clipgroup->svp) {
		art_svp_free (clipgroup->svp);
		clipgroup->svp = NULL;
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


static void
gnome_canvas_clipgroup_set_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GnomeCanvasItem *item;
	GnomeCanvasClipgroup *cgroup;
	GPPath *gpp;

	item = GNOME_CANVAS_ITEM (object);
	cgroup = GNOME_CANVAS_CLIPGROUP (object);

	switch (arg_id) {
	case ARG_PATH:
		gpp = GTK_VALUE_POINTER (*arg);

		if (cgroup->path) {
			gp_path_unref (cgroup->path);
			cgroup->path = NULL;
		}
		if (gpp != NULL) {
			cgroup->path = gp_path_closed_parts (gpp);
		}

		gnome_canvas_item_request_update (item);
		break;

	case ARG_WIND:
		cgroup->wind = GTK_VALUE_ENUM (*arg);
		gnome_canvas_item_request_update (item);
		break;

	default:
		break;
	}
}

static void
gnome_canvas_clipgroup_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GnomeCanvasClipgroup * cgroup;

	cgroup = GNOME_CANVAS_CLIPGROUP (object);

	switch (arg_id) {
	case ARG_PATH:
		GTK_VALUE_POINTER (*arg) = cgroup->path;
		break;

	case ARG_WIND:
		GTK_VALUE_ENUM (*arg) = cgroup->wind;
		break;

	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

static void
gnome_canvas_clipgroup_update (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	GnomeCanvasClipgroup *clipgroup;
	ArtBpath *bp;
	ArtBpath *bpath;
	ArtVpath *vpath1, *vpath2;
	ArtSVP *svp, *svp1, *svp2;

	clipgroup = GNOME_CANVAS_CLIPGROUP (item);

	if (clipgroup->svp) {
		art_svp_free (clipgroup->svp);
		clipgroup->svp = NULL;
	}

	if (clipgroup->path) {
		bp = gp_path_bpath (clipgroup->path);
		bpath = art_bpath_affine_transform (bp, affine);

		vpath1 = art_bez_path_to_vec (bpath, 0.25);
		art_free (bpath);

		vpath2 = art_vpath_perturb (vpath1);
		art_free (vpath1);

		svp1 = art_svp_from_vpath (vpath2);
		art_free (vpath2);
		
		svp2 = art_svp_uncross (svp1);
		art_svp_free (svp1);

		svp1 = art_svp_rewind_uncrossed (svp2, clipgroup->wind);
		art_svp_free (svp2);

		if (clip_path != NULL) {
			svp = art_svp_intersect (svp1, clip_path);
			art_svp_free (svp1);
		} else {
			svp = svp1;
		}

		clipgroup->svp = svp;
	}

	if (GNOME_CANVAS_ITEM_CLASS (parent_class)->update)
		(GNOME_CANVAS_ITEM_CLASS (parent_class)->update) (item, affine, NULL, flags);

	if (clipgroup->svp) {
		ArtDRect cbox;
		art_drect_svp (&cbox, clipgroup->svp);
		item->x1 = MAX (item->x1, cbox.x0 - 1.0);
		item->y1 = MAX (item->y1, cbox.y0 - 1.0);
		item->x2 = MIN (item->x2, cbox.x1 + 1.0);
		item->y2 = MIN (item->y2, cbox.y1 + 1.0);
	}
}

#define COMPOSEN11(fc,fa,bc) (((255 - (guint) (fa)) * (guint) (bc) + (guint) (fc) * (guint) (fa) + 127) / 255)

static void
gnome_canvas_clipgroup_render (GnomeCanvasItem *item, GnomeCanvasBuf *buf)
{
	GnomeCanvasClipgroup *cg;
	GnomeCanvasBuf lbuf;
	guchar *mask;

	cg = GNOME_CANVAS_CLIPGROUP (item);

	if (cg->svp) {
		gint bw, bh, sw, sh;
		gint x, y;

		if (buf->is_bg) {
			gnome_canvas_buf_ensure_buf (buf);
			buf->is_bg = FALSE;
			buf->is_buf = TRUE;
		}

		bw = buf->rect.x1 - buf->rect.x0;
		bh = buf->rect.y1 - buf->rect.y0;
		if ((bw < 1) || (bh < 1)) return;

		if (bw * bh <= GCG_BUF_PIXELS) {
			/* We can go with single buffer */
			sw = bw;
			sh = bh;
		} else if (bw <= (GCG_BUF_PIXELS >> 3)) {
			/* Go with row buffer */
			sw = bw;
			sh =  GCG_BUF_PIXELS / bw;
		} else if (bh <= (GCG_BUF_PIXELS >> 3)) {
			/* Go with column buffer */
			sw = GCG_BUF_PIXELS / bh;
			sh = bh;
		} else {
			/* Tile buffer */
			sw = GCG_BUF_WIDTH;
			sh = GCG_BUF_HEIGHT;
		}

		/* Set up local buffer */
		lbuf.buf = gcg_buf_new ();
		lbuf.bg_color = buf->bg_color;
		lbuf.is_bg = FALSE;
		lbuf.is_buf = TRUE;
		/* Allocate mask */
		mask = gcg_mask_new ();

		for (y = buf->rect.y0; y < buf->rect.y1; y += sh) {
			for (x = buf->rect.x0; x < buf->rect.x1; x += sw) {
				gint r, xx, yy;
				/* Set up local buffer */
				lbuf.rect.x0 = x;
				lbuf.rect.y0 = y;
				lbuf.rect.x1 = MIN (x + sw, buf->rect.x1);
				lbuf.rect.y1 = MIN (y + sh, buf->rect.y1);
				lbuf.buf_rowstride = 3 * (lbuf.rect.x1 - lbuf.rect.x0);
				/* Copy background */
				for (r = lbuf.rect.y0; r < lbuf.rect.y1; r++) {
					memcpy (lbuf.buf + (r - lbuf.rect.y0) * lbuf.buf_rowstride,
						buf->buf + (r - buf->rect.y0) * buf->buf_rowstride + (x - buf->rect.x0) * 3,
						(lbuf.rect.x1 - lbuf.rect.x0) * 3);
				}
				/* Invoke render method */
				if (((GnomeCanvasItemClass *) parent_class)->render)
					((GnomeCanvasItemClass *) parent_class)->render (item, &lbuf);
				/* Render mask */
				art_gray_svp_aa (cg->svp, lbuf.rect.x0, lbuf.rect.y0, lbuf.rect.x1, lbuf.rect.y1,
						 mask, lbuf.rect.x1 - lbuf.rect.x0);
				/* Combine */
				for (yy = lbuf.rect.y0; yy < lbuf.rect.y1; yy++) {
					guchar *s, *m, *d;
					s = lbuf.buf + (yy - lbuf.rect.y0) * lbuf.buf_rowstride;
					m = mask + (yy - lbuf.rect.y0) * (lbuf.rect.x1 - lbuf.rect.x0);
					d = buf->buf + (yy - buf->rect.y0) * buf->buf_rowstride + (x - buf->rect.x0) * 3;
					for (xx = lbuf.rect.x0; xx < lbuf.rect.x1; xx++) {
#ifndef SHOW_SHADOW
						d[0] = COMPOSEN11 (s[0], m[0], d[0]);
						d[1] = COMPOSEN11 (s[1], m[0], d[1]);
						d[2] = COMPOSEN11 (s[2], m[0], d[2]);
#else
						d[0] = COMPOSEN11 (s[0], m[0] | 0x7f, d[0]);
						d[1] = COMPOSEN11 (s[1], m[0] | 0x7f, d[1]);
						d[2] = COMPOSEN11 (s[2], m[0] | 0x7f, d[2]);
#endif
						s += 3;
						m += 1;
						d += 3;
					}
				}
			}
		}
		/* Free buffers */
		gcg_mask_free (mask);
		gcg_buf_free (lbuf.buf);
	} else {
		if (((GnomeCanvasItemClass *) parent_class)->render)
			((GnomeCanvasItemClass *) parent_class)->render (item, buf);
	}
}

static GSList *gcg_buffers = NULL;
static GSList *gcg_masks = NULL;

static guchar *
gcg_buf_new (void)
{
	guchar *buf;

	if (!gcg_buffers) {
		buf = g_new (guchar, GCG_BUF_SIZE);
	} else {
		buf = (guchar *) gcg_buffers->data;
		gcg_buffers = g_slist_remove (gcg_buffers, buf);
	}

	return buf;
}

static void
gcg_buf_free (guchar *buf)
{
	gcg_buffers = g_slist_prepend (gcg_buffers, buf);
}

static guchar *
gcg_mask_new (void)
{
	guchar *mask;

	if (!gcg_masks) {
		mask = g_new (guchar, GCG_BUF_PIXELS);
	} else {
		mask = (guchar *) gcg_masks->data;
		gcg_masks = g_slist_remove (gcg_masks, mask);
	}

	return mask;
}

static void
gcg_mask_free (guchar *mask)
{
	gcg_masks = g_slist_prepend (gcg_masks, mask);
}

