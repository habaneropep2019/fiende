#ifndef __GNOME_FONT_FACE_H__
#define __GNOME_FONT_FACE_H__

/*
 * GnomeFontFace - An unscaled typeface object for gnome-print
 *
 * Authors:
 *   Jody Goldberg <jody@helixcode.com>
 *   Miguel de Icaza <miguel@helixcode.com>
 *   Lauris Kaplinski <lauris@helixcode.com>
 *   Christopher James Lahey <clahey@helixcode.com>
 *   Michael Meeks <michael@helixcode.com>
 *   Morten Welinder <terra@diku.dk>
 *
 * Copyright (C) 1999-2000 Helix Code, Inc. and authors
 *
 */

#define GNOME_TYPE_FONT_FACE		(gnome_font_face_get_type ())
#define GNOME_FONT_FACE(obj)		(GTK_CHECK_CAST ((obj), GNOME_TYPE_FONT_FACE, GnomeFontFace))
#define GNOME_FONT_FACE_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_FONT_FACE, GnomeFontFaceClass))
#define GNOME_IS_FONT_FACE(obj)		(GTK_CHECK_TYPE ((obj), GNOME_TYPE_FONT_FACE))
#define GNOME_IS_FONT_FACE_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_FONT_FACE))

typedef struct _GnomeFontFace		GnomeFontFace;
typedef struct _GnomeFontFacePrivate	GnomeFontFacePrivate;
typedef struct _GnomeFontFaceClass	GnomeFontFaceClass;

/* Various backward compatibility typedefs */
typedef struct _GnomeFontFace		GnomeFontUnsized;
typedef struct _GnomeFontFace		GnomeFontMap;

/*
 * These numbers are very loosely adapted from the Univers numbering
 * convention. I've had to insert some to accomodate all the
 * distinctions in the Adobe font catalog. Also, I've offset the
 * numbering so that the default is 0.
 */

typedef enum {
  GNOME_FONT_LIGHTEST = -3,
  GNOME_FONT_EXTRA_LIGHT = -3,
  GNOME_FONT_THIN = -2,
  GNOME_FONT_LIGHT = -1,
  GNOME_FONT_BOOK = 0, /* also known as "regular" or "roman" */
  /* gap here so that if book is missing, light wins over medium */
  GNOME_FONT_MEDIUM = 2,
  GNOME_FONT_SEMI = 3, /* also known as "demi" */
  GNOME_FONT_BOLD = 4,
  /* gap here so that if bold is missing, semi wins over heavy */
  GNOME_FONT_HEAVY = 6,
  GNOME_FONT_EXTRABOLD = 7,
  GNOME_FONT_BLACK = 8,
  GNOME_FONT_EXTRABLACK = 9, /* also known as "ultra" */
  GNOME_FONT_HEAVIEST = 9
} GnomeFontWeight;

#define GNOME_FONT_NUM_WEIGHTS (GNOME_FONT_HEAVIEST + 1 - GNOME_FONT_LIGHTEST)

#include <gtk/gtkobject.h>
#include <libgnome/gnome-defs.h>
#include <libart_lgpl/art_rect.h>
#include <libart_lgpl/art_bpath.h>
#include <libgnomeprint/gnome-font.h>

BEGIN_GNOME_DECLS

/* The one and only Gtk+ type */

GtkType gnome_font_face_get_type (void);

#define gnome_font_face_ref(f) gtk_object_ref (GTK_OBJECT (f))
#define gnome_font_face_unref(f) gtk_object_unref (GTK_OBJECT (f))

/* Methods */

/*
 * Finding
 */

GnomeFontFace * gnome_font_face_new (const gchar * name);

/*
 * Naming
 *
 * gnome_font_face_get_name () should return one "true" name for font, that
 * does not have to be its PostScript name.
 * In future those names can be possibly localized (except ps_name)
 */

const gchar * gnome_font_face_get_name (const GnomeFontFace * face);
const gchar * gnome_font_face_get_family_name (const GnomeFontFace * face);
const gchar * gnome_font_face_get_species_name (const GnomeFontFace * face);
const gchar * gnome_font_face_get_ps_name (const GnomeFontFace * face);

/*
 * General information
 *
 */

gint gnome_font_face_get_num_glyphs (const GnomeFontFace * face);

/*
 * Get glyph number from unicode char
 *
 * In future there can probably be several different unicode to glyph
 * character mappings per font (for different languages), current function
 * uses the default one :)
 */

gint gnome_font_face_lookup_default (const GnomeFontFace * face, gint unicode);

/*
 * Metrics
 *
 * Currently those return standard values for left to right, horizontal script
 * The prefix std means, that there (a) will hopefully be methods to extract
 * different metric values and (b) for given face one text direction can
 * be defined as "default"
 * All face metrics are given in 0.001 em units
 */

const ArtDRect * gnome_font_face_get_stdbbox (const GnomeFontFace * face);

ArtPoint * gnome_font_face_get_glyph_stdadvance (const GnomeFontFace * face, gint glyph, ArtPoint * advance);
ArtDRect * gnome_font_face_get_glyph_stdbbox (const GnomeFontFace * face, gint glyph, ArtDRect * bbox);
const ArtBpath * gnome_font_face_get_glyph_stdoutline (const GnomeFontFace * face, gint glyph);

/*
 * Create font
 *
 * Those allow one to get sized font object from given typeface. Theoretically
 * GnomeFont should be optimized for certain resolution (resx, resy). If that
 * resolution is not known, get_font_default should give reasonably well-
 * looking font for most occasions
 */

GnomeFont * gnome_font_face_get_font (const GnomeFontFace * face, gdouble size, gdouble xres, gdouble yres);
GnomeFont * gnome_font_face_get_font_default (const GnomeFontFace * face, gdouble size);

/*
 * Various backward-compatibility and conveninence methods
 *
 * NB! Those usually do not scale for international fonts, so use with
 * caution.
 */

gdouble gnome_font_face_get_ascender (const GnomeFontFace * face);
gdouble gnome_font_face_get_descender (const GnomeFontFace * face);
gdouble gnome_font_face_get_underline_position (const GnomeFontFace * face);
gdouble gnome_font_face_get_underline_thickness (const GnomeFontFace * face);

gdouble gnome_font_face_get_glyph_width (const GnomeFontFace * face, gint glyph);
gdouble gnome_font_face_get_glyph_kerning (const GnomeFontFace * face, gint glyph1, gint glyph2);
const gchar * gnome_font_face_get_glyph_ps_name (const GnomeFontFace * face, gint glyph);

GnomeFontWeight gnome_font_face_get_weight_code (const GnomeFontFace * face);
gboolean gnome_font_face_is_italic (const GnomeFontFace * face);
gboolean gnome_font_face_is_fixed_width (const GnomeFontFace * face);


gchar * gnome_font_face_get_pfa (const GnomeFontFace * face);

/*
 * Give (possibly localized) demonstration text for given face
 * Most usually this tells about quick fox and lazy dog...
 */

const gchar * gnome_font_face_get_sample (const GnomeFontFace * face);

/* Find the closest face matching the family name, weight, and italic */

GnomeFontFace * gnome_font_unsized_closest (const char *family_name,
					    GnomeFontWeight weight,
					    gboolean italic);

END_GNOME_DECLS

#endif /* __GNOME_FONT_FACE_H__ */



