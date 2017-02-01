#ifndef _GNOME_GLYPHLIST_PRIVATE_H_
#define _GNOME_GLYPHLIST_PRIVATE_H_

#include <libgnomeprint/gnome-glyphlist.h>

BEGIN_GNOME_DECLS

/*
 * We are dealing with lists at moment - although arrays are probably better
 */

#define GGL_GLYPH_BLOCK_SIZE 32
#define GGL_RULE_BLOCK_SIZE 4

typedef struct _GGLRule GGLRule;

struct _GnomeGlyphList {
	GtkObject object;
	gint * glyphs;
	gint g_length, g_size;
	GGLRule * rules;
	gint r_length, r_size;
};

struct _GnomeGlyphListClass {
	GtkObjectClass parent_class;
};

typedef enum {
	GGL_POSITION,     /* Glyph position for following rules */
	/* Absolute positioning */
	GGL_MOVETOX,      /* Set pen to absolute X */
	GGL_MOVETOY,      /* Set pen to absolute Y */
	GGL_RMOVETOX,     /* Advance pen X by distance */
	GGL_RMOVETOY,     /* Advance pen Y by distance */
	/* Stack manipulation */
	GGL_PUSHCP,       /* Push pen position to stack */
	GGL_POPCP,        /* Pop pen position from stack */
	/* Typesetting rules */
	GGL_ADVANCE,      /* Forces glyph to advance pen by stdadvance */
	GGL_LETTERSPACE,  /* Forces given point distance to be added for every advance */
	GGL_KERNING,      /* Forces given % of interglyph kerning to be added */
	GGL_FONT,         /* Font to use for following glyphs */
	GGL_COLOR         /* Color to use for following glyphs */
} GGLInfoType;

struct _GGLRule {
	guint8 code;
	union {
		gint ival;
		gdouble dval;
		gboolean bval;
		GnomeFont * font;
		guint32 color;
	} value;
};

END_GNOME_DECLS

#endif
