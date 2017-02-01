/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* eel-canvas-rect.h - canvas rect with more-efficient update function

   Copyright (C) 2001 Ximian, Inc.
   Copyright (C) 2001 Eazel, Inc.

   The Eel Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Eel Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Chris Lahey <clahey@ximian.com>
            Darin Adler <darin@eazel.com>
*/

#include <config.h>
#include "eel-canvas-rect.h"

#include "eel-gtk-macros.h"
#include "eel-lib-self-check-functions.h"
#include <libart_lgpl/art_rect_svp.h>
#include <libart_lgpl/art_svp_ops.h>
#include <libart_lgpl/art_svp_vpath.h>
#include <libart_lgpl/art_uta_svp.h>
#include <libgnomeui/gnome-canvas-util.h>

struct EelCanvasRectDetails {
	ArtDRect last_update_rect;
};

static void eel_canvas_rect_initialize       (GtkObject      *object);
static void eel_canvas_rect_initialize_class (GtkObjectClass *object_class);
static void diff_rects                       (ArtDRect        source1,
					      ArtDRect        source2,
					      int            *count,
					      ArtDRect        result[4]);

EEL_DEFINE_CLASS_BOILERPLATE (EelCanvasRect,
			      eel_canvas_rect,
			      GNOME_TYPE_CANVAS_RECT)

static GnomeCanvasItemClass *re_parent_class;

/* FIXME: Give another name (eel_art_drect_make?) and move to eel-art-extensions. */
static ArtDRect
make_drect (double x0, double y0, double x1, double y1)
{
	ArtDRect r;

	r.x0 = x0;
	r.y0 = y0;
	r.x1 = x1;
	r.y1 = y1;
	return r;
}

/* FIXME: Give another name (eel_art_drect_make_empty?) and move to eel-art-extensions. */
static ArtDRect
make_empty_drect (void)
{
	ArtDRect r;

	r.x0 = 0;
	r.y0 = 0;
	r.x1 = 0;
	r.y1 = 0;
	return r;
}

static void
make_rect_vpath (ArtDRect r,
		 ArtVpath vpath[6])
{
	vpath[0].code = ART_MOVETO;
	vpath[0].x = r.x0;
	vpath[0].y = r.y0;
	vpath[1].code = ART_LINETO;
	vpath[1].x = r.x0;
	vpath[1].y = r.y1;
	vpath[2].code = ART_LINETO;
	vpath[2].x = r.x1;
	vpath[2].y = r.y1;
	vpath[3].code = ART_LINETO;
	vpath[3].x = r.x1;
	vpath[3].y = r.y0;
	vpath[4].code = ART_LINETO;
	vpath[4].x = r.x0;
	vpath[4].y = r.y0;
	vpath[5].code = ART_END;
	vpath[5].x = 0;
	vpath[5].y = 0;
}

/**
 * canvas_item_update_svp_no_repaint:
 * @item: the canvas item containing the svp that needs updating.
 * @p_svp: a pointer to the existing svp
 * @new_svp: the new svp
 *
 * Sets the svp to the new value. This
 * function takes responsibility for freeing new_svp. This routine also adds the
 * svp's bbox to the item's.
 *
 * This clone of gnome_canvas_item_update_svp does not request a repaint.
 **/
static void
canvas_item_update_svp_no_repaint (GnomeCanvasItem *item, ArtSVP **p_svp, ArtSVP *new_svp)
{
	ArtDRect bbox;

	*p_svp = new_svp;
	if (new_svp) {
		bbox.x0 = item->x1;
		bbox.y0 = item->y1;
		bbox.x1 = item->x2;
		bbox.y1 = item->y2;
		art_drect_svp_union (&bbox, new_svp);
		item->x1 = bbox.x0;
		item->y1 = bbox.y0;
		item->x2 = bbox.x1;
		item->y2 = bbox.y1;
	}
}

/**
 * canvas_item_update_svp_clip_no_repaint:
 * @item: the canvas item containing the svp that needs updating.
 * @p_svp: a pointer to the existing svp
 * @new_svp: the new svp
 * @clip_svp: a clip path, if non-null
 *
 * Sets the svp to the new value, clipping if necessary.
 * This function takes responsibility for freeing new_svp.
 *
 * This clone of gnome_canvas_item_update_svp_clip does not request a repaint.
 **/
static void
canvas_item_update_svp_clip_no_repaint (GnomeCanvasItem *item, ArtSVP **p_svp, ArtSVP *new_svp,
					ArtSVP *clip_svp)
{
	ArtSVP *clipped_svp;

	if (clip_svp != NULL) {
		clipped_svp = art_svp_intersect (new_svp, clip_svp);
		art_svp_free (new_svp);
	} else {
		clipped_svp = new_svp;
	}

	canvas_item_update_svp_no_repaint (item, p_svp, clipped_svp);
}

static void
canvas_request_update_rect (GnomeCanvas *canvas,
			    double affine[6],
			    ArtSVP *clip_svp,
			    ArtDRect rect)
{
	ArtVpath rect_vpath[6];
	ArtVpath *transformed_vpath;
	ArtSVP *svp, *clipped_svp;
	ArtUta *uta;

	make_rect_vpath (rect, rect_vpath);
	transformed_vpath = art_vpath_affine_transform (rect_vpath, affine);
	svp = art_svp_from_vpath (transformed_vpath);
	art_free (transformed_vpath);
	if (clip_svp != NULL) {
		clipped_svp = art_svp_intersect (svp, clip_svp);
		art_svp_free (svp);
	} else {
		clipped_svp = svp;
	}
	uta = art_uta_from_svp (clipped_svp);
	art_svp_free (svp);
	gnome_canvas_request_redraw_uta (canvas, uta);
}

static void
rect_update (GnomeCanvasItem *item, double affine[6], ArtSVP *clip_path, int flags)
{
	GnomeCanvasRE *re;
	EelCanvasRectDetails *details;
	ArtVpath rect_vpath[6];
	ArtVpath *vpath;
	ArtSVP *stroke_svp;
      	double width;
	ArtDRect update_rect, repaint_rects[4];
	int repaint_rects_count, i;

	if (!item->canvas->aa) {
		EEL_CALL_PARENT (GNOME_CANVAS_ITEM_CLASS, update, (item, affine, clip_path, flags));
		return;
	}

	(* re_parent_class->update) (item, affine, clip_path, flags);
	
	re = GNOME_CANVAS_RE (item);
	details = EEL_CANVAS_RECT (item)->details;
	
	if (!(re->fill_set || re->outline_set)) {
		update_rect = make_empty_drect ();
		vpath = NULL;
	} else {
		update_rect = make_drect (re->x1, re->y1, re->x2, re->y2);
		make_rect_vpath (update_rect, rect_vpath);
		vpath = art_vpath_affine_transform (rect_vpath, affine);
	}
	
	gnome_canvas_item_reset_bounds (item);
	
	if (!re->fill_set) {
		gnome_canvas_item_update_svp (item, &re->fill_svp, NULL);
	} else {
		if (re->fill_svp)
			art_svp_free (re->fill_svp);
		
		canvas_item_update_svp_clip_no_repaint
			(item, &re->fill_svp, art_svp_from_vpath (vpath), clip_path);
		
		/* We called a version of update_svp that doesn't
		 * request a repaint, so now we handle the repainting
		 * separately.
		 */
		diff_rects (update_rect, details->last_update_rect,
			    &repaint_rects_count, repaint_rects);
		for (i = 0; i < repaint_rects_count; i++) {
			canvas_request_update_rect (item->canvas, affine,
						    clip_path, repaint_rects[i]);
		}
	}
	
	if (!re->outline_set) {
		gnome_canvas_item_update_svp (item, &re->outline_svp, NULL);
	} else {
		width = re->width;
		if (!re->width_pixels) {
			width *= item->canvas->pixels_per_unit;
		}
		stroke_svp = art_svp_vpath_stroke (vpath,
						   ART_PATH_STROKE_JOIN_MITER,
						   ART_PATH_STROKE_CAP_BUTT,
						   width, 4, 0.25);
		gnome_canvas_item_update_svp_clip (item, &re->outline_svp,
						   stroke_svp, clip_path);
	}
	
	art_free (vpath);

	details->last_update_rect = update_rect;
}

static void
rect_destroy (GtkObject *object)
{
	EelCanvasRect *rect;
	
	rect = EEL_CANVAS_RECT (object);

	g_free (rect->details);

	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

static void
eel_canvas_rect_initialize_class (GtkObjectClass *object_class)
{
	GnomeCanvasItemClass *item_class;

	/* our ancestor, for the update call that skips a generation */
	re_parent_class = gtk_type_class (GNOME_TYPE_CANVAS_ITEM);

	item_class = GNOME_CANVAS_ITEM_CLASS (object_class);

	item_class->update = rect_update;
	object_class->destroy = rect_destroy;
}

static void
eel_canvas_rect_initialize (GtkObject *object)
{
	EelCanvasRect *rect;
	
	rect = EEL_CANVAS_RECT (object);
	
	rect->details = g_new0 (EelCanvasRectDetails, 1);
}

GnomeCanvasItem *
eel_canvas_rect_new (void)
{
	return GNOME_CANVAS_ITEM (gtk_type_new (EEL_TYPE_CANVAS_RECT));
}

/* FIXME: Give another name (eel_art_drect_hits_drect?) and move to eel-art-extensions. */
static gboolean
rects_intersect (ArtDRect r1, ArtDRect r2)
{
	if (r1.x0 >= r2.x1) {
		return FALSE;
	}
	if (r2.x0 >= r1.x1) {
		return FALSE;
	}
	if (r1.y0 >= r2.y1) {
		return FALSE;
	}
	if (r2.y0 >= r1.y1) {
		return FALSE;
	}
	return TRUE;
}

static void
diff_rects_guts (ArtDRect ra, ArtDRect rb, int *count, ArtDRect result[4])
{
	if (ra.x0 < rb.x0) {
		result[(*count)++] = make_drect (ra.x0, ra.y0, rb.x0, ra.y1);
	}
	if (ra.y0 < rb.y0) {
		result[(*count)++] = make_drect (ra.x0, ra.y0, ra.x1, rb.y0);
	}
	if (ra.x1 < rb.x1) {
		result[(*count)++] = make_drect (ra.x1, rb.y0, rb.x1, rb.y1);
	}
	if (ra.y1 < rb.y1) {
		result[(*count)++] = make_drect (rb.x0, ra.y1, rb.x1, rb.y1);
	}
}

/* FIXME: Give another name (eel_diff_art_drects?) and move to eel-art-extensions. */
static void
diff_rects (ArtDRect r1, ArtDRect r2, int *count, ArtDRect result[4])
{
	g_assert (count != NULL);
	g_assert (result != NULL);

	*count = 0;

	if (rects_intersect (r1, r2)) {
		diff_rects_guts (r1, r2, count, result);
		diff_rects_guts (r2, r1, count, result);
	} else {
		if (!art_drect_empty (&r1)) {
			result[(*count)++] = r1;
		}
		if (!art_drect_empty (&r2)) {
			result[(*count)++] = r2;
		}
	}
}

#ifndef EEL_OMIT_SELF_CHECK

static gboolean
test_rects_intersect (double x01, double y01, double x11, double y11,
		      double x02, double y02, double x12, double y12)
{
	ArtDRect r1, r2;
	
	r1.x0 = x01;
	r1.y0 = y01;
	r1.x1 = x11;
	r1.y1 = y11;
	r2.x0 = x02;
	r2.y0 = y02;
	r2.x1 = x12;
	r2.y1 = y12;
	return rects_intersect (r1, r2);
}

static char *
test_diff_rects (double x01, double y01, double x11, double y11,
		 double x02, double y02, double x12, double y12)
{
	ArtDRect r1, r2, result[4];
	int count, i;
	GString *string;
	char *str;
	
	r1.x0 = x01;
	r1.y0 = y01;
	r1.x1 = x11;
	r1.y1 = y11;
	r2.x0 = x02;
	r2.y0 = y02;
	r2.x1 = x12;
	r2.y1 = y12;
	diff_rects (r1, r2, &count, result);

	if (count < 0 || count > 4) {
		return g_strdup ("ERROR");
	}

	string = g_string_new ("");
	for (i = 0; i < count; i++) {
		g_string_sprintfa (string, "(%.0f,%.0f,%.0f,%.0f) ",
				   result[i].x0,
				   result[i].y0,
				   result[i].x1,
				   result[i].y1);
	}
	if (string->len != 0) {
		g_string_truncate (string, string->len - 1);
	}
	str = string->str;
	g_string_free (string, FALSE);
	return str;
}

void
eel_self_check_canvas_rect (void)
{
	EEL_CHECK_BOOLEAN_RESULT (test_rects_intersect (0,0,0,0, 0,0,0,0), FALSE);
	EEL_CHECK_BOOLEAN_RESULT (test_rects_intersect (0,0,0,0, 0,0,1,1), FALSE);
	EEL_CHECK_BOOLEAN_RESULT (test_rects_intersect (1,1,1,1, 0,0,0,0), FALSE);
	EEL_CHECK_BOOLEAN_RESULT (test_rects_intersect (1,1,1,1, 0,0,1,1), FALSE);
	EEL_CHECK_BOOLEAN_RESULT (test_rects_intersect (0,0,1,1, 0,0,2,2), TRUE);
	EEL_CHECK_BOOLEAN_RESULT (test_rects_intersect (0,0,3,3, 1,1,2,2), TRUE);
	EEL_CHECK_BOOLEAN_RESULT (test_rects_intersect (0,0,5,5, 5,5,10,10), FALSE);
	EEL_CHECK_BOOLEAN_RESULT (test_rects_intersect (0,0,10,10, 5,5,15,15), TRUE);

	EEL_CHECK_STRING_RESULT (test_diff_rects (0,0,0,0, 0,0,0,0), "");
	EEL_CHECK_STRING_RESULT (test_diff_rects (0,0,0,0, 0,0,1,1), "(0,0,1,1)");
	EEL_CHECK_STRING_RESULT (test_diff_rects (1,1,1,1, 0,0,0,0), "");
	EEL_CHECK_STRING_RESULT (test_diff_rects (1,1,1,1, 0,0,1,1), "(0,0,1,1)");
	EEL_CHECK_STRING_RESULT (test_diff_rects (0,0,1,1, 0,0,2,2), "(1,0,2,2) (0,1,2,2)");
	EEL_CHECK_STRING_RESULT (test_diff_rects (0,0,3,3, 1,1,2,2), "(0,0,1,3) (0,0,3,1) (2,0,3,3) (0,2,3,3)");
	EEL_CHECK_STRING_RESULT (test_diff_rects (0,0,5,5, 5,5,10,10), "(0,0,5,5) (5,5,10,10)");
	EEL_CHECK_STRING_RESULT (test_diff_rects (0,0,10,10, 5,5,15,15), "(0,0,5,10) (0,0,10,5) (10,5,15,15) (5,10,15,15)");
}

#endif
