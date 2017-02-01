#ifndef _GNOME_PGL_PRIVATE_H_
#define _GNOME_PGL_PRIVATE_H_

#include <libgnomeprint/gnome-rfont.h>
#include <libgnomeprint/gnome-glyphlist.h>

BEGIN_GNOME_DECLS

typedef struct _GnomePosGlyph GnomePosGlyph;
typedef struct _GnomePosString GnomePosString;

/*
 * Positioned Glyph
 */

struct _GnomePosGlyph {
	gint glyph;
	gdouble x, y;
};

struct _GnomePosString {
	gint start, length;
	GnomeRFont * rfont;
	guint32 color;
};

struct _GnomePosGlyphList {
	GnomePosGlyph * glyphs;
	GnomePosString * strings;
	gint num_strings;
};

END_GNOME_DECLS

#endif
