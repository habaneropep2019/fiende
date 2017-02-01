#ifndef __GNOME_RFONT_H__
#define __GNOME_RFONT_H__

/*
 * GnomeRFont - Raster font for gnome
 *
 * Authors:
 *   Lauris Kaplinski <lauris@helixcode.com>
 *
 * Copyright (C) 1999-2000 Helix Code, Inc.
 *
 */

#define GNOME_TYPE_RFONT		(gnome_rfont_get_type ())
#define GNOME_RFONT(obj)		(GTK_CHECK_CAST ((obj), GNOME_TYPE_RFONT, GnomeRFont))
#define GNOME_RFONT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_RFONT, GnomeRFontClass))
#define GNOME_IS_RFONT(obj)		(GTK_CHECK_TYPE ((obj), GNOME_TYPE_RFONT))
#define GNOME_IS_RFONT_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_RFONT))

typedef struct _GnomeRFont		GnomeRFont;
typedef struct _GnomeRFontClass		GnomeRFontClass;

#include <libart_lgpl/art_svp.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-pgl.h>

BEGIN_GNOME_DECLS

/* The one and only Gtk+ type */

GtkType gnome_rfont_get_type (void);

/* Methods */

/*
 * Creates new RFont from given font and font->raster affine matrix
 * Matrix can be 2x2, although if read, all 2x3 values are retrieved
 * RFont is referenced, so you have to unref it somewhere
 */

GnomeRFont * gnome_font_get_rfont (GnomeFont * font, gdouble * transform);

#define gnome_rfont_ref(o) gtk_object_ref (GTK_OBJECT (o))
#define gnome_rfont_unref(o) gtk_object_unref (GTK_OBJECT (o))

/*
 * Attributes
 */

const GnomeFont * gnome_rfont_get_font (const GnomeRFont * rfont);
const GnomeFontFace * gnome_rfont_get_face (const GnomeRFont * rfont);
gdouble * gnome_rfont_get_matrix (const GnomeRFont * rfont, gdouble * matrix);

/*
 * Typeface attributes
 *
 * stdadvance is basically space width transformed to device coordinates
 */

ArtPoint * gnome_rfont_get_stdadvance (const GnomeRFont * rfont, ArtPoint * advance);

/*
 * Glyph Attributes
 */

ArtPoint * gnome_rfont_get_glyph_stdadvance (const GnomeRFont * rfont, gint glyph, ArtPoint * advance);
ArtDRect * gnome_rfont_get_glyph_stdbbox (const GnomeRFont * rfont, gint glyph, ArtDRect * bbox);

/*
 * I am not sure, whether these should be stdoutline and stdsvp
 */

const ArtBpath * gnome_rfont_get_glyph_bpath (const GnomeRFont * rfont, gint glyph);
const ArtSVP * gnome_rfont_get_glyph_svp (const GnomeRFont * rfont, gint glyph);

/*
 * Rendering flags
 *
 * Look into gnome-pgl.h for more meaningful definitions
 */

#define GNOME_RFONT_RENDER_DEFAULT 0

void gnome_rfont_render_glyph_rgba8 (const GnomeRFont * rfont, gint glyph,
				     guint32 rgba,
				     gdouble x, gdouble y,
				     guchar * buf,
				     gint width, gint height, gint rowstride,
				     guint flags);

void gnome_rfont_render_glyph_rgb8 (const GnomeRFont * rfont, gint glyph,
				    guint32 rgba,
				    gdouble x, gdouble y,
				    guchar * buf,
				    gint width, gint height, gint rowstride,
				    guint flags);

void gnome_rfont_render_pgl_rgb8 (const GnomePosGlyphList * pgl,
				    gdouble x, gdouble y,
				    guchar * buf,
				    gint width, gint height, gint rowstride,
				    guint flags);

/*
 * WARNING - THIS IS EXPERIMENTAL
 */

void gnome_rfont_render_glyph_gdk_drawable (const GnomeRFont * rfont, gint glyph,
				guint32 rgba,
				gdouble x, gdouble y,
				GdkDrawable * drawable,
				guint32 background,
				guint flags);

void gnome_rfont_render_pgl_gdk_drawable (const GnomePosGlyphList * pgl,
				gdouble x, gdouble y,
				GdkDrawable * drawable,
				guint32 background,
				guint flags);

END_GNOME_DECLS

#endif /* __GNOME_RFONT_H__ */


