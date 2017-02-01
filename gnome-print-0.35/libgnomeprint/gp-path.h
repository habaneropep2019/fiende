#ifndef GP_PATH_H
#define GP_PATH_H

/*
 * GPPath
 *
 * (C) 1999-2000 Lauris Kaplinski <lauris@ariman.ee>
 * Released under LGPL
 *
 * This is mostly like GnomeCanvasBpathDef, but with added functionality:
 * - can be constructed from scratch, from existing bpath of from static bpath
 * - Path is always terminated with ART_END
 * - Has closed flag
 * - has concat, split and copy methods
 *
 */

#include <glib.h>
#include <libart_lgpl/art_bpath.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

typedef struct _GPPath GPPath;

/* Constructors */

GPPath * gp_path_new (void);
GPPath * gp_path_new_sized (gint length);
GPPath * gp_path_new_from_bpath (ArtBpath * bpath);
GPPath * gp_path_new_from_static_bpath (ArtBpath * bpath);
GPPath * gp_path_new_from_foreign_bpath (ArtBpath * bpath);

void gp_path_ref (GPPath * path);
void gp_path_finish (GPPath * path);
void gp_path_ensure_space (GPPath * path, gint space);

/*
 * Misc constructors
 * All these return NEW path, not unrefing old
 * Also copy and duplicate force bpath to be private (otherwise you
 * would use ref :)
 */

GPPath * gp_path_copy (GPPath * dst, const GPPath * src);
GPPath * gp_path_duplicate (const GPPath * path);
GPPath * gp_path_concat (const GSList * list);
GSList * gp_path_split (const GPPath * path);
GPPath * gp_path_open_parts (const GPPath * path);
GPPath * gp_path_closed_parts (const GPPath * path);
GPPath * gp_path_close_all (const GPPath * path);

/* Destructor */

void gp_path_unref (GPPath * path);

/* Methods */

/* Sets GPPath to zero length */

void gp_path_reset (GPPath * path);

/* Drawing methods */

void gp_path_moveto (GPPath * path, gdouble x, gdouble y);
void gp_path_lineto (GPPath * path, gdouble x, gdouble y);

/* Does not create new ArtBpath, but simply changes last lineto position */

void gp_path_lineto_moving (GPPath * path, gdouble x, gdouble y);
void gp_path_curveto (GPPath * path, gdouble x0, gdouble y0,gdouble x1, gdouble y1, gdouble x2, gdouble y2);
void gp_path_closepath (GPPath * path);

/* Does not draw new line to startpoint, but moves last lineto */

void gp_path_closepath_current (GPPath * path);

/* Various methods */

ArtBpath * gp_path_bpath (const GPPath * path);
gint gp_path_length (const GPPath * path);
gboolean gp_path_is_empty (const GPPath * path);
gboolean gp_path_has_currentpoint (const GPPath * path);
ArtPoint * gp_path_currentpoint (const GPPath * path, ArtPoint * p);
ArtBpath * gp_path_last_bpath (const GPPath * path);
ArtBpath * gp_path_first_bpath (const GPPath * path);
gboolean gp_path_any_open (const GPPath * path);
gboolean gp_path_all_open (const GPPath * path);
gboolean gp_path_any_closed (const GPPath * path);
gboolean gp_path_all_closed (const GPPath * path);

END_GNOME_DECLS

#endif
