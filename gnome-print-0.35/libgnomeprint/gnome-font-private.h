#ifndef __GNOME_FONT_PRIVATE_H__
#define __GNOME_FONT_PRIVATE_H__

/*
 * Private data definitions and forward declarations for gnome fonts
 *
 * Authors:
 *   Jody Goldberg <jody@helixcode.com>
 *   Miguel de Icaza <miguel@helixcode.com>
 *   Lauris Kaplinski <lauris@helixcode.com>
 *   Christopher James Lahey <clahey@helixcode.com>
 *   Michael Meeks <michael@helixcode.com>
 *   Morten Welinder <terra@diku.dk>
 *
 * Distributed under GNU Lesser Public License
 *
 * Copyright (C) 1999-2001 Ximian Code, Inc. and authors
 *
 */

#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

#include <freetype/freetype.h>
#include <libgnomeprint/gp-character-block.h>
#include <libgnomeprint/gp-fontmap.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-rfont.h>

typedef struct _GFFGlyphInfo GFFGlyphInfo;

struct _GnomeFontFace {
	GtkObject object;

	/* Pointer to our fontmap entry */
	GPFontEntry *entry;

	/* Glyph storage */
	gint num_glyphs;
	GFFGlyphInfo *glyphs;

	/* FT -> PostScript scaling coefficent */
	gdouble ft2ps;

	/* Face bounding box */
	ArtDRect bbox;

	/* FreeType stuff */
	FT_Face ft_face;
};

struct _GnomeFontFaceClass {
	GtkObjectClass parent_class;
};

struct _GFFGlyphInfo {
	guint metrics : 1;
	ArtPoint advance;
	ArtDRect bbox;
	ArtBpath *bpath;
};

typedef struct _GFPSObject GFPSObject;

struct _GFPSObject {
	/* Our face (NB referenced) */
	GnomeFontFace *face;
	/* Resident face name, if any */
	guchar *residentname;
	/* Name of final font (i.e. findfont argument) */
	guchar *encodedname;
	/* 1 or 2 byte encoding
	 * This will be set according to total number of
	 * glyphs in font, as we cannot know the number
	 * of glyphs used initially */
	gint encodedbytes;
	/* Number of glyphs in face */
	gint num_glyphs;
	/* Used glyph bit array */
	guint32 *glyphs;
	/* Buffer values */
	gint bufsize;
	gint length;
	guchar *buf;
};

GFPSObject *gnome_font_face_pso_new (GnomeFontFace *face, guchar *residentname);
/* Mark glyph as used in internal bit array */
void gnome_font_face_pso_mark_glyph (GFPSObject *pso, gint glyph);
void gnome_font_face_pso_free (GFPSObject *pso);
void gnome_font_face_pso_ensure_buffer (GFPSObject *pso);

struct _GnomeFont {
	GtkObject object;

	/* Our face entry */
	GnomeFontFace *face;

	/* Size in typographic units */
	gdouble size;

	/* Cache of glyph outlines */
	GHashTable *outlines;
};

struct _GnomeFontClass {
	GtkObjectClass parent_class;
};

GnomeFont *gnome_font_face_get_font_full (GnomeFontFace * face, gdouble size, gdouble * affine);

/*
 * Returns PostScript name for glyph
 */

const gchar * gnome_font_face_get_glyph_ps_name (const GnomeFontFace *face, gint glyph);
const gchar * gnome_font_unsized_get_glyph_name (const GnomeFontFace *face);

ArtPoint * gnome_rfont_get_glyph_stdkerning (const GnomeRFont * rfont, gint glyph0, gint glyph1, ArtPoint * kerning);

END_GNOME_DECLS

#endif

