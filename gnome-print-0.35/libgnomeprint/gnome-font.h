#ifndef __GNOME_FONT_H__
#define __GNOME_FONT_H__

/*
 * GnomeFont - basic user visible handle to scaled typeface
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
 * The following arguments are available:
 *
 * name		       type		read/write	description
 * --------------------------------------------------------------------------------
 * ascender            double           R               maximum ascender for the face
 * descender           double           R               maximum descender for the face
 * underline_position  double           R               underline position for the face
 * underline_thickness double           R               underline thickness for the face
 *
 */

#define GNOME_TYPE_FONT		 (gnome_font_get_type ())
#define GNOME_FONT(obj)		 (GTK_CHECK_CAST ((obj), GNOME_TYPE_FONT, GnomeFont))
#define GNOME_FONT_CLASS(klass)	 (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_FONT, GnomeFontClass))
#define GNOME_IS_FONT(obj)	 (GTK_CHECK_TYPE ((obj), GNOME_TYPE_FONT))
#define GNOME_IS_FONT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_FONT))

typedef struct _GnomeFont		GnomeFont;
typedef struct _GnomeFontPrivate	GnomeFontPrivate;
typedef struct _GnomeFontClass		GnomeFontClass;
typedef struct _GnomeFontClassPrivate	GnomeFontClassPrivate;

#include <gdk/gdk.h>
#include <libgnomeprint/gnome-font-face.h>

BEGIN_GNOME_DECLS

/* The one and only Gtk+ type */

GtkType gnome_font_get_type (void);

#define gnome_font_ref(f) gtk_object_ref (GTK_OBJECT (f))
#define gnome_font_unref(f) gtk_object_unref (GTK_OBJECT (f))

/*
 * Methods
 *
 * Look into gnome-font-face for explanations
 */

/* Naming */

const gchar * gnome_font_get_name (const GnomeFont * font);
const gchar * gnome_font_get_family_name (const GnomeFont * font);
const gchar * gnome_font_get_species_name (const GnomeFont * font);
const gchar * gnome_font_get_ps_name (const GnomeFont * font);

/* Unicode -> glyph translation */

gint gnome_font_lookup_default (const GnomeFont * font, gint unicode);

/*
 * Metrics
 *
 * Note that GnomeFont metrics are given in typographic points
 */

ArtPoint * gnome_font_get_glyph_stdadvance (const GnomeFont * font, gint glyph, ArtPoint * advance);
ArtDRect * gnome_font_get_glyph_stdbbox (const GnomeFont * font, gint glyph, ArtDRect * bbox);
const ArtBpath * gnome_font_get_glyph_stdoutline (const GnomeFont * font, gint glyph);

/*
 * Backward compatibility and convenience methods
 *
 * NB! Those usually do not scale for international fonts, so use with
 * caution.
 */

gdouble gnome_font_get_ascender (const GnomeFont * font);
gdouble gnome_font_get_descender (const GnomeFont * font);
gdouble gnome_font_get_underline_position (const GnomeFont * font);
gdouble gnome_font_get_underline_thickness (const GnomeFont * font);

gdouble gnome_font_get_glyph_width (const GnomeFont * font, gint glyph);
gdouble gnome_font_get_glyph_kerning (const GnomeFont * font, gint glyph1, gint glyph2);

GnomeFontWeight gnome_font_get_weight_code (const GnomeFont * font);
gboolean gnome_font_is_italic (const GnomeFont * font);

gchar * gnome_font_get_pfa (const GnomeFont * font);

const GnomeFontFace * gnome_font_get_face (const GnomeFont * font);
gdouble gnome_font_get_size (const GnomeFont * font);

/*
 * Font fetching
 */

GnomeFont * gnome_font_new (const char * name, double size);

GnomeFont * gnome_font_new_closest (const char *family_name,
				   GnomeFontWeight weight,
				   gboolean italic,
				   double size);

GnomeFont *gnome_font_new_from_full_name (const char *string);

/*
 * Font browsing
 */

/* Return a list of fonts, as a g_list of strings */

GList * gnome_font_list (void);
void gnome_font_list_free (GList *fontlist);

/* Return a list of font families, as a g_list of newly allocated strings */

GList * gnome_font_family_list (void);
void gnome_font_family_list_free (GList *fontlist);

/*
 * Compatibility
 */

/* typedef struct
 * {
 *  GnomeFontUnsized *unsized_font;   font before scaling
 *  GnomeFont *gnome_font;            scaled font
 *  double scale;                     scaling factor requested by a view
 * 
 *  char *x_font_name;                x name that got us gdk_font
 *  GdkFont *gdk_font;                used for drawing
 * } GnomeDisplayFont;
 */

typedef struct _GnomeRFont GnomeDisplayFont;

GnomeDisplayFont * gnome_font_get_display_font (GnomeFont *font);
const char * gnome_font_weight_to_string (GnomeFontWeight gfw);
GnomeFontWeight string_to_gnome_font_weight (const char *weight);
GnomeDisplayFont * gnome_get_display_font (const char * family,
					   GnomeFontWeight weight,
					   gboolean italic,
					   double points,
					   double scale);
int gnome_display_font_height (GnomeDisplayFont * gdf);

/*
 * These are for attribute reading
 *
 * If writing fresh code, use rfont methods directly
 */

const GnomeFontFace * gnome_display_font_get_face (const GnomeDisplayFont * gdf);
const GnomeFont * gnome_display_font_get_font (const GnomeDisplayFont * gdf);
gdouble gnome_display_font_get_scale (const GnomeDisplayFont * gdf);
const gchar * gnome_display_font_get_x_font_name (const GnomeDisplayFont * gdf);
GdkFont * gnome_display_font_get_gdk_font (const GnomeDisplayFont * gdf);

void gnome_display_font_ref (GnomeDisplayFont * gdf);
void gnome_display_font_unref (GnomeDisplayFont * gdf);

/* Misc */

char * gnome_font_get_full_name (const GnomeFont *font);

/*
 * We keep these at moment, but in future better go with Pango/glyphlists
 */

double gnome_font_get_width_utf8 (const GnomeFont *font, const char *s);
double gnome_font_get_width_utf8_sized (const GnomeFont *font, const char *s, int n);
/*
 * WARNING WARNING WARNING
 *
 * These functions do expect string to be 8-but iso-8859-1!
 * NOT utf-8 as most other gnome-print methods!
 * So they are COMPLETELY unusable for non-latin scripts!
 *
 * WARNING WARNING WARNING
 *
 */
double gnome_font_get_width_string (const GnomeFont *font, const char *s);
double gnome_font_get_width_string_n (const GnomeFont *font, const char *s, int n);

END_GNOME_DECLS

#endif /* __GNOME_FONT_H__ */



