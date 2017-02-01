#ifndef __GNOME_CANVAS_CLIPGROUP_H__
#define __GNOME_CANVAS_CLIPGROUP_H__

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

#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libart_lgpl/art_vpath_dash.h>
#include <libgnomeui/gnome-canvas.h>
#include "gp-path.h"

#define GNOME_TYPE_CANVAS_CLIPGROUP            (gnome_canvas_clipgroup_get_type ())
#define GNOME_CANVAS_CLIPGROUP(obj)            (GTK_CHECK_CAST ((obj), GNOME_TYPE_CANVAS_CLIPGROUP, GnomeCanvasClipgroup))
#define GNOME_CANVAS_CLIPGROUP_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_CLIPGROUP, GnomeCanvasClipgroupClass))
#define GNOME_IS_CANVAS_CLIPGROUP(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_CANVAS_CLIPGROUP))
#define GNOME_IS_CANVAS_CLIPGROUP_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_CLIPGROUP))


typedef struct _GnomeCanvasClipgroup GnomeCanvasClipgroup;
typedef struct _GnomeCanvasClipgroupClass GnomeCanvasClipgroupClass;

struct _GnomeCanvasClipgroup {
	GnomeCanvasGroup group;

	GPPath *path;
	ArtWindRule wind;

	ArtSVP *svp;
};

struct _GnomeCanvasClipgroupClass {
	GnomeCanvasGroupClass parent_class;
};

GtkType gnome_canvas_clipgroup_get_type (void);

END_GNOME_DECLS

#endif
