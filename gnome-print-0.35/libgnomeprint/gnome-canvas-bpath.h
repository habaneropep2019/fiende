/* Bpath item type for GnomeCanvas widget
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998,1999 The Free Software Foundation
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Raph Levien <raph@acm.org>
 *          Lauris Kaplinski <lauris@ariman.ee>
 */

#ifndef GNOME_CANVAS_BPATH_H
#define GNOME_CANVAS_BPATH_H

#include <libgnome/gnome-defs.h>
#include <libgnomeui/gnome-canvas.h>
#if 0
#include <libgnomeui/gnome-canvas-util.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_svp_wind.h>
#include <libart_lgpl/art_vpath_dash.h>
#include "gp-path.h"
#endif

BEGIN_GNOME_DECLS


/* Bpath item for the canvas.
 *
 * The following object arguments are available:
 *
 * name			type			read/write	description
 * ------------------------------------------------------------------------------------------
 * bpath		GPPath *		RW		Pointer to an GPPath structure.
 *								This can be created by a call to
 *								gp_path_new() in (gp-path.h).
 * fill_color		string			W		X color specification for fill color,
 *								or NULL pointer for no color (transparent).
 * fill_color_gdk	GdkColor*		RW		Allocated GdkColor for fill.
 * outline_color	string			W		X color specification for outline color,
 *								or NULL pointer for no color (transparent).
 * outline_color_gdk	GdkColor*		RW		Allocated GdkColor for outline.
 * fill_stipple		GdkBitmap*		RW		Stipple pattern for fill
 * outline_stipple	GdkBitmap*		RW		Stipple pattern for outline
 * width_pixels		uint			RW		Width of the outline in pixels.  The outline will
 *								not be scaled when the canvas zoom factor is changed.
 * width_units		double			RW		Width of the outline in canvas units.  The outline
 *								will be scaled when the canvas zoom factor is changed.
 * cap_style		GdkCapStyle		RW		Cap ("endpoint") style for the bpath.
 * join_style		GdkJoinStyle		RW		Join ("vertex") style for the bpath.
 * wind                 ArtWindRule             RW              Winding rule for the bpath.
 * dash			ArtVpathDash		RW		Dashing pattern
 * miterlimit		double			RW		Minimum angle between segments, where miter join
 *								rule is applied.
 */

#define GNOME_TYPE_CANVAS_BPATH            (gnome_canvas_bpath_get_type ())
#define GNOME_CANVAS_BPATH(obj)            (GTK_CHECK_CAST ((obj), GNOME_TYPE_CANVAS_BPATH, GnomeCanvasBpath))
#define GNOME_CANVAS_BPATH_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_BPATH, GnomeCanvasBpathClass))
#define GNOME_IS_CANVAS_BPATH(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_CANVAS_BPATH))
#define GNOME_IS_CANVAS_BPATH_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_BPATH))


typedef struct _GnomeCanvasBpath GnomeCanvasBpath;
typedef struct _GnomeCanvasBpathPriv GnomeCanvasBpathPriv;
typedef struct _GnomeCanvasBpathClass GnomeCanvasBpathClass;

struct _GnomeCanvasBpath {
	GnomeCanvasItem item;

	GnomeCanvasBpathPriv *priv;	/* Private data */
};

struct _GnomeCanvasBpathClass {
	GnomeCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType gnome_canvas_bpath_get_type (void);

END_GNOME_DECLS

#endif
