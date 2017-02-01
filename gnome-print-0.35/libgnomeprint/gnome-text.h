#ifndef __GNOME_TEXT_H__
#define __GNOME_TEXT_H__

BEGIN_GNOME_DECLS

/* A first cut at an interface for text handling */

/* A font family code is a handle gotten from gnome-font. gnome-font
   will also have an interface for querying a font family given the
   handle. */

typedef guint32 G_Ucs4;

typedef struct _GnomeTextLayout GnomeTextLayout;
typedef struct _GnomeTextBreak GnomeTextBreak;
typedef struct _GnomeTextAttrEl GnomeTextAttrEl;
typedef struct _GnomeTextGlyphAttrEl GnomeTextGlyphAttrEl;
typedef struct _GnomeTextGlyph GnomeTextGlyph;
typedef struct _GnomeTextLine GnomeTextLine;

typedef int GnomeTextFontListHandle;
typedef int GnomeTextFontFamilyHandle;
typedef int GnomeTextFontHandle;

typedef enum {
  GNOME_TEXT_END,

  GNOME_TEXT_FONT_LIST, /* A GnomeTextFontList handle */
  GNOME_TEXT_SIZE, /* point size * 1000 */
  GNOME_TEXT_XSCALE, /* 800 = condensed, 1000 = normal, 1300 = extended */
  GNOME_TEXT_OBLIQUING_UGH, /* tangent of angle * 1000 */
  GNOME_TEXT_WEIGHT, /* GNOME_TEXT_BOOK and GNOME_TEXT_BOLD are values */
  GNOME_TEXT_ITALICS, /* boolean */
  GNOME_TEXT_KERNING, /* boolean, inverted */
  GNOME_TEXT_LIGATURES, /* GNOME_TEXT_LIG_NONE, _NORMAL, _MAX */
  GNOME_TEXT_TRACKING, /* extra letterspace in .001 em units */
  GNOME_TEXT_SMALL_CAPS, /* boolean */
  GNOME_TEXT_GLYPH_ALTERNATE, /* integer, font specific */
  GNOME_TEXT_UNDERLINE_UGH, /* boolean, or maybe 2 for double underline */
  GNOME_TEXT_STRIKETHROUGH, /* boolean */
  GNOME_TEXT_RISE, /* extra height from baseline in .001 em units */
  GNOME_TEXT_COLOR, /* rgba */
  /* language code */
  /* random multiple master attributes, etc., here */
  /* probably hyphenation preferences need to get added too */
  GNOME_TEXT_LAST
} GnomeTextAttr;

typedef enum {
  GNOME_TEXT_GLYPH_FONT, /* A GnomeTextFont handle */
  GNOME_TEXT_GLYPH_SIZE, /* point size * 1000 */
  GNOME_TEXT_GLYPH_XSCALE, /* 800 = condensed, 1000 = normal, 1300 = extended */
  GNOME_TEXT_GLYPH_OBLIQUING, /* tangent of angle * 1000 */
  GNOME_TEXT_GLYPH_UNDERLINE, /* boolean, or maybe 2 for double underline */
  GNOME_TEXT_GLYPH_STRIKETHROUGH, /* boolean */
  GNOME_TEXT_GLYPH_RISE, /* extra height from baseline in .001 em units */
  GNOME_TEXT_GLYPH_COLOR, /* rgba */
  /* random multiple master attributes, etc., here */

  GNOME_TEXT_GLYPH_END

} GnomeTextGlyphAttr;

typedef int GnomeTextGlyphAttrState[GNOME_TEXT_GLYPH_END];

extern const GnomeTextGlyphAttrState gnome_text_default_glyph_state;

struct _GnomeTextAttrEl {
  int char_pos; /* offset in (possibly wide) characters from start of string */
  GnomeTextAttr attr;
  int attr_val;
};

struct _GnomeTextGlyphAttrEl {
  int glyph_pos; /* offset in glyphs from start of string */
  GnomeTextGlyphAttr attr;
  int attr_val;
};

/* so the standard interface of attributed text into gnome_text is:

func (G_Ucs4 *chars, GnomeTextAttrEl *attrs);

maybe n_chars too, maybe null terminated.

The chars are UCS4 encoded. Thus, ISO-8859-1 characters can just get
zero-extended. This may seem wasteful to you now, but it will make
full Unicode a lot easier later down the road. Trust me.

Note added 4 Jan 1999: on the advice of Chris Lahey, I'm going with
UTF-8 instead of UCS4 as the standard encoding.

*/

/* Next up is an interface that returns a list of breaks given an
   attributed text stream. These are in essentially the same format
   as libhnj.

   In addition, there is an (optional) glyph list at the end of the
   first line, and an (optional) glyph list at the beginning of the
   second. The glyph positions from which to take glyphs from the
   unbroken line are also given.

   Thus, a line delimited by two breaks is reconstructed as follows:
   the glyphs in the beginning of the first break, the glyphs from the
   begin position given in the first break to the end position in the
   second break, and the glyphs in the end of the second break. */

typedef enum {
  GNOME_TEXT_BREAK_ISSPACE = 1,
  GNOME_TEXT_BREAK_ISHYPHEN = 2
} GnomeTextBreakFlags;

struct _GnomeTextBreak {
  int x0;
  int x1;
  int penalty;
  GnomeTextBreakFlags flags;
  GnomeTextGlyph *end_glyphs;
  int end_glyph_pos;
  GnomeTextGlyph *begin_glyphs;
  int begin_glyph_pos;
};

#define GNOME_TEXT_SCALE 50

/* The x values are in GNOME_TEXT_SCALE * 0.001 of a user space
coordinate units. To be concrete, if the user space coordinate is a
point, and GNOME_TEXT_SCALE is 50, then a single x value is 50 * 0.001
/ 72 inch, or about 0.000694 inches.

It will probably store an x value for each character - but rough.
In particular, they will change with line breaks, and also positioning
with ligatures is approximate at best. And this is for English-like
languages!

Finally, there must be a data structure somewhere that maps breaks
back to character positions. Searching in the x value for
each character works, but may (or may not be) extra computation.

Anyway, the GnomeTextBreak values are run through the justification
engine, and a series of line breaks are returned (see libhnj for
more discussion).

The line breaks, x value list, and original attributed text are then
run through a second pass of the high level gnome_text interface,
resulting in an attributed glyph list.

Attributed glyph lists are very much like attributed text, except:
1. a font handle is used instead of a font family handle
2. instead of UCS4 character numbers, glyph numbers are given,
   the encoding of which are specific to the font.
3. an x value is given for each glyph, and all x spacing attributes
   are gone
4. ligature codes are gone because this step takes care of ligation

Thus, the following attributes remain:

_FONT, // not present in attributed text
_SIZE,
_XSCALE,
_OBLIQUING_UGH,
_UNDERLINE_UGH,
_STRIKETHROUGH,
_RISE,
_COLOR

*/

struct _GnomeTextGlyph {
  int glyph_num;
  int x;
};

typedef enum {
  GNOME_TEXT_ALIGN_LEFT,
  GNOME_TEXT_ALIGN_CENTER,
  GNOME_TEXT_ALIGN_RIGHT,
  GNOME_TEXT_ALIGN_JUST
} GnomeTextAlign;

/* The GnomeTextLayout data structure is an "object" that stays with
   the text as it goes through the different phases.

   Should this structure be renamed simply GnomeText?
*/
struct _GnomeTextLayout {
  GnomeTextGlyphAttrEl *attrs;

  GnomeTextBreak *breaks;
  int n_breaks;

  GnomeTextGlyph *glyphs;
  int n_glyphs;

  int set_width;
  int max_neg_space;
  GnomeTextAlign align;

  int *breaks_chosen; /* one per line */
  int n_lines;
};

struct _GnomeTextLine {
  GnomeTextGlyphAttrEl *attrs;

  GnomeTextGlyph *glyphs;
  int n_glyphs;
};

/* This takes the utf-8 encoded text, and does the first phase in
   laying it out. This phase is the one that's completely independent
   of justification or line formatting. */
GnomeTextLayout *
gnome_text_layout_new (const guchar *text, GnomeTextAttrEl *attrs);

void
gnome_text_layout_free (GnomeTextLayout *layout);

void
gnome_text_unicode_to_glyph (GnomeTextFontListHandle fontlist,
			     int unicode,
			     GnomeFontWeight weight,
			     gboolean italic,
			     GnomeTextFontHandle *p_font,
			     int *p_glyph);

char *
gnome_text_get_font_name (GnomeTextFontHandle font);

GnomeFontUnsized *
gnome_text_get_font (GnomeTextFontHandle font);

int
gnome_text_get_width (GnomeTextFontHandle font, int glyph);

/* The argument is given as a comma-separated list of font families
   (thus, font families are not allowed to contain commas). The return
   value is a GnomeTextFontListHandle, suitable for use as a
   GNOME_TEXT_FONT_LIST attribute in a GnomeTextLayout. */
GnomeTextFontListHandle
gnome_text_intern_font_list (const char *fontlist_name);

/* Justify the layout using a simple, greedy algorithm. */
void
gnome_text_hs_just (GnomeTextLayout *layout);

/* This function creates a single text line from the layout - it can
   be used in cases when it is known that the text will fit in a
   single line. */
GnomeTextLine *
gnome_text_line_from_layout (GnomeTextLayout *layout);

GnomeTextLine **
gnome_text_lines_from_layout (GnomeTextLayout *layout);

void
gnome_text_line_free (GnomeTextLine *line);

END_GNOME_DECLS

#endif
