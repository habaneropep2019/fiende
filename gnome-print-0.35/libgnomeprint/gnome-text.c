#include <gtk/gtk.h>
#include <string.h>
#include <math.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-font-private.h>
#include <libgnomeprint/gnome-text.h>

typedef struct _GnomeFontKernPair	GnomeFontKernPair;
typedef struct _GnomeFontLigList	GnomeFontLigList;

struct _GnomeFontKernPair {
	int glyph1;
	int glyph2;
	int x_amt;
};

struct _GnomeFontLigList {
	GnomeFontLigList *next;
	int succ, lig;
};

typedef struct _GnomeTextFontFamily GnomeTextFontFamily;
typedef struct _GnomeTextFontList GnomeTextFontList;

struct _GnomeTextFontList {
  int n_families;
  GnomeTextFontFamilyHandle *families;
};

struct _GnomeTextFontFamily {
  gchar *name;
  GnomeTextFontHandle fonts[2 * GNOME_FONT_NUM_WEIGHTS];
  /* -1 means the corresponding font does not exist. 0 means it
     hasn't been determined yet.

     It's stored as weight-major, i.e. the index is:
     2 * (weight - GNOME_FONT_LIGHTEST) + italic
  */
};

static GHashTable *font_list_hash = NULL;
static int font_list_index = 0;
static GnomeTextFontList **font_list_tab;
static int n_font_list_tab_max;

static GHashTable *font_family_hash = NULL;
static int font_family_index = 0;
static GnomeTextFontFamily **font_family_tab;
static int n_font_family_tab_max;

static GHashTable *font_hash = NULL;
static int font_index = 0;
static GnomeFontUnsized **font_tab;
static int n_font_tab_max;

const GnomeTextGlyphAttrState gnome_text_default_glyph_state =
{ 0, 0, 0, 1000, 0, 0, 0, 0x000000ff };

static void
gnome_text_add_glyph_attr (int glyph_pos,
			   GnomeTextGlyphAttr attr,
			   int attr_val,
			   GnomeTextGlyphAttrEl **p_attrs,
			   int *p_n_attrs,
			   int *p_n_attrs_max)
{
  GnomeTextGlyphAttrEl *el;

  if (*p_n_attrs == *p_n_attrs_max)
    *p_attrs = g_renew (GnomeTextGlyphAttrEl, *p_attrs,
			(*p_n_attrs_max) <<= 1);
  el = *p_attrs + *p_n_attrs;
  (*p_n_attrs)++;
  el->glyph_pos = glyph_pos;
  el->attr = attr;
  el->attr_val = attr_val;
}

/* This takes the utf-8 encoded text, and does the first phase in
   laying it out. This phase is the one that's completely independent
   of justification or line formatting.

   The proper design of this routine is quite challenging. It takes on
   an intimidating amount of functionality, including utf-8 decoding,
   Unicode handling, language-dependent hyphenation, kerning, and
   ligatures. Because of the Unicode handling, it's necessary to do
   dispatching to some language-specific handlers. Yet, efficiency
   remains a major concern. It's important for the common case to fall
   through a straight code path without much overhead for table lookup
   and dispatching.

   Many of the variables in here should probably become elements of
   a structure. This way, the structure can get handed around to
   language-specific handlers.

   In any case: first api, then stability, finally speed.

*/


GnomeTextLayout *
gnome_text_layout_new (const guchar *text, GnomeTextAttrEl *attrs)
{
  GnomeTextLayout *layout;
  int byte_idx;
  int char_idx;
  int attr_idx;
  int code;
  GnomeTextGlyph *glyphs;
  int n_glyphs, n_glyphs_max;
  guchar c;
  GnomeTextFontListHandle cur_fontlist;
  GnomeTextFontHandle cur_font, new_font;
  GnomeFontWeight cur_weight;
  gboolean cur_italic;
  gint cur_size, cur_xscale;
  int cur_track;
  double scale_factor;
  double track_val;
  int glyph;
  GnomeTextGlyphAttrEl *g_attrs;
  int n_attrs, n_attrs_max;
  int x;
  int last_glyph;
  GnomeFontUnsized *unsized;
  GnomeFontLigList *lig;
  GnomeTextBreak *breaks;
  int n_breaks, n_breaks_max;

  layout = g_new (GnomeTextLayout, 1);

  n_glyphs_max = 16;
  glyphs = g_new (GnomeTextGlyph, n_glyphs_max);
  n_glyphs = 0;

  n_attrs_max = 16;
  g_attrs = g_new (GnomeTextGlyphAttrEl, n_attrs_max);
  n_attrs = 0;

  n_breaks_max = 16;
  breaks = g_new (GnomeTextBreak, n_breaks_max);
  n_breaks = 0;

  cur_fontlist = 0;
  cur_weight = GNOME_FONT_BOOK;
  cur_italic = 0;
  cur_size = 0;
  scale_factor = 0;
  cur_xscale = 1000;
  scale_factor = 0;
  cur_track = 0;
  track_val = 0;
  last_glyph = -1;
  unsized = NULL;

  x = 0;
  cur_font = 0;

  attr_idx = 0;
  char_idx = 0;
  for (byte_idx = 0; (c = text[byte_idx]) != 0; char_idx++)
    {
      /* apply attributes */
      while (attrs[attr_idx].char_pos == char_idx)
	{
	  switch (attrs[attr_idx].attr)
	    {
	    case GNOME_TEXT_FONT_LIST:
	      cur_fontlist = attrs[attr_idx].attr_val;
	      last_glyph = -1;
	      break;
	    case GNOME_TEXT_WEIGHT:
	      cur_weight = attrs[attr_idx].attr_val;
	      last_glyph = -1;
	      break;
	    case GNOME_TEXT_ITALICS:
	      cur_italic = attrs[attr_idx].attr_val;
	      last_glyph = -1;
	      break;
	    case GNOME_TEXT_SIZE:
	      cur_size = attrs[attr_idx].attr_val;
	      scale_factor = cur_size * cur_xscale * 1e-9 * GNOME_TEXT_SCALE;
	      track_val = cur_track * scale_factor;
	      gnome_text_add_glyph_attr (n_glyphs,
					 GNOME_TEXT_GLYPH_SIZE,
					 cur_size,
					 &g_attrs,
					 &n_attrs,
					 &n_attrs_max);
	      last_glyph = -1;
	      break;
	    case GNOME_TEXT_XSCALE:
	      cur_xscale = attrs[attr_idx].attr_val;
	      scale_factor = cur_size * cur_xscale * 1e-9 * GNOME_TEXT_SCALE;
	      track_val = cur_track * scale_factor;
	      gnome_text_add_glyph_attr (n_glyphs,
					 GNOME_TEXT_GLYPH_XSCALE,
					 cur_xscale,
					 &g_attrs,
					 &n_attrs,
					 &n_attrs_max);
	      last_glyph = -1;
	      break;
	    case GNOME_TEXT_TRACKING:
	      cur_track = attrs[attr_idx].attr_val;
	      track_val = cur_track * scale_factor;
	      break;
	    default:
	      break;
	    }
	  attr_idx++;
	}

      /* This is the UTF-8 decoder. */
      if (c & 0x80)
	{
	  if ((c & 0xe0) == 0xe0)
	    {
	      if ((c & 0xf0) == 0xf0)
		{
		  /* 4-byte code */
		  code = ((c & 0x07) << 18) |
		    ((text[byte_idx + 1] & 0x3f) << 12) |
		    ((text[byte_idx + 2] & 0x3f) << 6) |
		    (text[byte_idx + 3] & 0x3f);
		  /* an alternative "raphcode" sequence would be
		     ((c << 18) | (text[byte_idx + 1] << 12) |
		      (text[byte_idx + 2] << 6) | text[byte_idx + 3) -
		      0x3c82080 */
		  byte_idx += 4;
		}
	      else
		{
		  /* 3-byte code */
		  code = ((c & 0x0f) << 12) |
		    ((text[byte_idx + 1] & 0x3f) << 6) |
		    (text[byte_idx + 2] & 0x3f);
		  /* alternately,
		     ((c << 12) | (text[byte_idx + 1] << 6) |
		     text[byte_idx + 2]) - 0xe2080 */
		  byte_idx += 3;
		}
	    }
	  else
	    {
	      /* 2-byte code */
	      code = ((c & 0x1f) << 6) |
		(text[byte_idx + 1] & 0x3f);
	      /* alternately,
		 ((c << 6) | text[byte_idx + 1]) - 0x3080 */
	      byte_idx += 2;
	    }
	}
      else
	{
	  /* ASCII */
	  code = c;
	  byte_idx++;
	}

      /* the unicode value of the character is now in code */
      if (code == ' ')
	{
	  /* handle spaces (note: there are other breaking spaces besides
	     0x20 that need to get dealt with) */

	  if (n_breaks == n_breaks_max)
	    breaks = g_renew (GnomeTextBreak, breaks, n_breaks_max <<= 1);

	  breaks[n_breaks].x0 = x;
	  x += floor (250 * scale_factor + 0.5);
	  /* maybe should look up width in font instead of assuming 250? */
	  breaks[n_breaks].x1 = x;
	  breaks[n_breaks].penalty = 0;
	  breaks[n_breaks].flags = GNOME_TEXT_BREAK_ISSPACE;
	  breaks[n_breaks].end_glyphs = NULL;
	  breaks[n_breaks].end_glyph_pos = n_glyphs;
	  breaks[n_breaks].begin_glyphs = NULL;
	  breaks[n_breaks].begin_glyph_pos = n_glyphs;
	  n_breaks++;

	  last_glyph = -1;
	}
      else
	{
	  gnome_text_unicode_to_glyph (cur_fontlist, code,
				       cur_weight, cur_italic,
				       &new_font, &glyph);
	  if (new_font >= 0)
	    {
	      if (new_font != cur_font)
		{
#ifdef VERBOSE
		  g_print ("adding font attribute, attr_idx = %d, byte_idx = %d, char_idx = %d, n_glyphs = %d\n",
			   attr_idx, byte_idx, char_idx, n_glyphs);
#endif
		  gnome_text_add_glyph_attr (n_glyphs,
					     GNOME_TEXT_GLYPH_FONT,
					     new_font,
					     &g_attrs,
					     &n_attrs,
					     &n_attrs_max);
		  cur_font = new_font;
		  unsized = font_tab[cur_font];
		}

	      if (last_glyph >= 0)
		{

#if 0
		  for (lig = unsized->private->ligs[last_glyph];
		       lig != NULL;
		       lig = lig->next)
		    if (lig->succ == glyph)
		      {
			break;
		      }
#else
		  lig = NULL;
#endif

		  if (lig == NULL)
		    x += floor (gnome_font_face_get_glyph_kerning (unsized,
							 last_glyph, glyph) *
				scale_factor + 0.5);
		}
	      else
		lig = NULL;

	      if (lig == NULL)
		{
		  last_glyph = glyph;

		  if (n_glyphs_max == n_glyphs)
		    glyphs = g_renew (GnomeTextGlyph, glyphs,
				      n_glyphs_max <<= 1);
		  glyphs[n_glyphs].glyph_num = glyph;
		  glyphs[n_glyphs].x = x;

		  x += floor (gnome_font_face_get_glyph_width (unsized, glyph) *
			      scale_factor + 0.5);

		  x += track_val;

#ifdef VERBOSE
		  g_print ("glyph %d, unsized width %d, scaled %f, x %d\n",
			   glyph, gnome_font_unsized_get_width (font_tab[cur_font],
								glyph), gnome_font_unsized_get_width (font_tab[cur_font],
												      glyph) * scale_factor +
			   0.5, x);
#endif

		  n_glyphs++;
		}
	      else
		{
		  /* found a ligature - replace the last glyph in the glyph
		     table with the ligated glyph */
		  glyph = lig->lig;
		  glyphs[n_glyphs - 1].glyph_num = glyph;
		  x += floor ((gnome_font_face_get_glyph_width (unsized, glyph) - 
			       gnome_font_face_get_glyph_width (unsized,
							     last_glyph)) *
			      scale_factor + 0.5);
		  last_glyph = glyph;
		}
	    }
	}

    }

  /* a terminal break to indicate end of paragraph */
  if (n_breaks == n_breaks_max)
    breaks = g_renew (GnomeTextBreak, breaks, n_breaks_max <<= 1);

  breaks[n_breaks].x0 = x;
  breaks[n_breaks].x1 = x;
  breaks[n_breaks].penalty = 0;
  breaks[n_breaks].flags = 0;
  breaks[n_breaks].end_glyphs = NULL;
  breaks[n_breaks].end_glyph_pos = n_glyphs;
  breaks[n_breaks].begin_glyphs = NULL;
  breaks[n_breaks].begin_glyph_pos = n_glyphs;
  n_breaks++;

  gnome_text_add_glyph_attr (n_glyphs, GNOME_TEXT_GLYPH_END, 0,
			     &g_attrs, &n_attrs, &n_attrs_max);

  layout->breaks = breaks;
  layout->n_breaks = n_breaks;
  layout->glyphs = glyphs;
  layout->n_glyphs = n_glyphs;
  layout->attrs = g_attrs;

  /* some basic initializations, just to make sure the layout is in a
     consistent state. */
  layout->max_neg_space = 0;
  layout->align = GNOME_TEXT_ALIGN_LEFT;
  layout->breaks_chosen = NULL;
  layout->n_lines = 0;

  return layout;
}

void
gnome_text_layout_free (GnomeTextLayout *layout)
{

  if (layout->attrs)
    g_free (layout->attrs);

  if (layout->glyphs)
    g_free (layout->glyphs);

  if (layout->breaks)
    g_free (layout->breaks);

  g_free (layout);
}

/* The argument is an unsized font. The return value is a
   GnomeFontHandle, suitable for a glyph attribute. The
   GnomeFontHandle indexes the unsized font. */
static GnomeTextFontHandle
gnome_text_intern_font (GnomeFontUnsized *unsized)
{
  gpointer val;

  if (font_hash == NULL)
    {
      font_hash = g_hash_table_new (g_str_hash, g_str_equal);
      n_font_tab_max = 16;
      font_tab = g_new (GnomeFontUnsized *, n_font_tab_max);
    }

  val = g_hash_table_lookup (font_hash, gnome_font_face_get_ps_name (unsized));

  if (val != NULL)
    return GPOINTER_TO_INT (val);

  font_index++;

  if (font_index == n_font_tab_max)
    font_tab = g_renew (GnomeFontUnsized *, font_tab, n_font_tab_max <<= 1);

  font_tab[font_index] = unsized;

  g_hash_table_insert (font_hash, (gpointer) gnome_font_face_get_ps_name (unsized),
		       GINT_TO_POINTER (font_index));
  return font_index;
}

/* The argument is a font family. The return value is a
   GnomeFontFamilyHandle, suitable for a glyph attribute. */
static GnomeTextFontHandle
gnome_text_intern_font_family (const char *family_name)
{
  gpointer val;
  GnomeTextFontFamily *family;
  int i;

  if (font_family_hash == NULL)
    {
      font_family_hash = g_hash_table_new (g_str_hash, g_str_equal);
      n_font_family_tab_max = 16;
      font_family_tab = g_new (GnomeTextFontFamily *, n_font_family_tab_max);
    }

  val = g_hash_table_lookup (font_family_hash, family_name);

  if (val != NULL)
    return GPOINTER_TO_INT (val);

  font_family_index++;

  family = g_new (GnomeTextFontFamily, 1);
  family->name = g_strdup (family_name);
  for (i = 0; i < 2 * GNOME_FONT_NUM_WEIGHTS; i++)
    family->fonts[i] = 0;

  if (font_family_index == n_font_family_tab_max)
    font_family_tab = g_renew (GnomeTextFontFamily *, font_family_tab,
			       n_font_family_tab_max <<= 1);

  font_family_tab[font_family_index] = family;

  g_hash_table_insert (font_family_hash, family->name,
		       GINT_TO_POINTER (font_family_index));
  return font_family_index;
}

/* The argument is given as a comma-separated list of font families
   (thus, font families are not allowed to contain commas). The return
   value is a GnomeTextFontListHandle, suitable for use as a
   GNOME_TEXT_FONT_LIST attribute in a GnomeTextLayout. */
GnomeTextFontListHandle
gnome_text_intern_font_list (const char *fontlist_name)
{
  gpointer val;
  GnomeTextFontList *fontlist;
  int i, n_families;
  int j, j_start;

  if (font_list_hash == NULL)
    {
      font_list_hash = g_hash_table_new (g_str_hash, g_str_equal);
      n_font_list_tab_max = 16;
      font_list_tab = g_new (GnomeTextFontList *, n_font_list_tab_max);
    }

  val = g_hash_table_lookup (font_list_hash, fontlist_name);
  if (val != NULL)
    return GPOINTER_TO_INT (val);

  font_list_index++;

  if (font_list_index == n_font_list_tab_max)
    font_list_tab = g_renew (GnomeTextFontList *, font_list_tab,
			     n_font_list_tab_max <<= 1);

  fontlist = g_new (GnomeTextFontList, 1);
  n_families = 1;
  for (j = 0; fontlist_name[j] != '\0'; j++)
    if (fontlist_name[j] == ',')
      n_families++;
  fontlist->n_families = n_families;
  fontlist->families = g_new (GnomeTextFontHandle, n_families);
  j_start = 0;
  for (i = 0; i < n_families; i++)
    {
      gchar *family;

      for (j = j_start; fontlist_name[j] != '\0' &&
	     fontlist_name[j] != ','; j++);
      family = g_new (gchar, j + 1 - j_start);
      memcpy (family, fontlist_name + j_start, j - j_start);
      family[j - j_start] = '\0';
      fontlist->families[i] = gnome_text_intern_font_family (family);
      g_free (family);
      j_start = j + 1;
    }

  font_list_tab[font_list_index] = fontlist;

  g_hash_table_insert (font_list_hash, g_strdup (fontlist_name),
		       GINT_TO_POINTER (font_list_index));
  return font_list_index;
}

void
gnome_text_unicode_to_glyph (GnomeTextFontListHandle fontlist,
			     int unicode,
			     GnomeFontWeight weight,
			     gboolean italic,
			     GnomeTextFontHandle *p_font,
			     int *p_glyph)
{
  GnomeTextFontList *flist;
  GnomeTextFontFamily *family;
  GnomeFontUnsized *unsized;
  GnomeTextFontHandle fhandle;
  int i;
  int idx;
  int glyph;

  idx = 2 * (weight - GNOME_FONT_LIGHTEST) + italic;
  flist = font_list_tab[fontlist];
  for (i = 0; i < flist->n_families; i++)
    {
      family = font_family_tab[flist->families[i]];
      fhandle = family->fonts[idx];
      if (fhandle == 0)
	{
	  unsized = gnome_font_unsized_closest (family->name, weight, italic);
	  if (unsized == NULL)
	    fhandle = -1;
	  else
	    fhandle = gnome_text_intern_font (unsized);

	  family->fonts[idx] = fhandle;
	}
      else
	{
	  unsized = font_tab[fhandle];
	}
      if (unsized != NULL)
	{
	  glyph = gnome_font_face_lookup_default (unsized, unicode);
	  if (glyph != -1)
	    {
	      *p_font = fhandle;
	      *p_glyph = glyph;
	      return;
	    }
	}
    }
  *p_font = -1;
  *p_glyph = -1;
}

char *
gnome_text_get_font_name (GnomeTextFontHandle font)
{
  return (char *) gnome_font_face_get_ps_name (font_tab[font]);
}

GnomeFontUnsized *
gnome_text_get_font (GnomeTextFontHandle font)
{
  return font_tab[font];
}

int
gnome_text_get_width (GnomeTextFontHandle font, int glyph)
{
  return gnome_font_face_get_glyph_width (font_tab[font], glyph);
}

/* A simple, high speed justification algorithm. Uses the greedy
   approach.

   */
void
gnome_text_hs_just (GnomeTextLayout *layout)
{
  int set_width = layout->set_width;
  int max_neg_space = layout->max_neg_space;
  int break_in_idx;
  int result_idx;
  int x;
  int total_space; /* total space seen so far */
  int best_penalty;
  int best_idx;
  int space_err;
  int penalty;
  GnomeTextBreak *breaks = layout->breaks;
  int n_breaks = layout->n_breaks;
  int *result;

  result = g_new (int, n_breaks);

  break_in_idx = 0;
  result_idx = 0;
  x = 0;
  while (break_in_idx != n_breaks)
    {
      total_space = 0;

      /* Calculate penalty for first possible break. */
      space_err = breaks[break_in_idx].x0 - (x + set_width);
      best_penalty = space_err * space_err + breaks[break_in_idx].penalty;
      best_idx = break_in_idx;

      /* Now, keep trying to find a better break until either all
	 breaks are exhausted, or the maximum negative space
	 constraint is violated, or the distance penalty is larger
	 than the best total penalty so far. */

      if (breaks[break_in_idx].flags & GNOME_TEXT_BREAK_ISSPACE)
	total_space += breaks[break_in_idx].x1 - breaks[break_in_idx].x0;
      break_in_idx++;

      while (break_in_idx < n_breaks &&
	     breaks[break_in_idx].x0 <=
	     x + set_width + ((total_space * max_neg_space + 0x80) >> 8))
	{
#ifdef VERBOSE
	  g_print ("line width here %d, set %d, total space %d\n",
		   breaks[break_in_idx].x0 - x, set_width,
		   total_space);
#endif
	  /* Calculate penalty of this break. */
	  space_err = breaks[break_in_idx].x0 - (x + set_width);
	  penalty = space_err * space_err;
	  if (penalty >= best_penalty)
	    break;
	  penalty += breaks[break_in_idx].penalty;
	  if (penalty < best_penalty)
	    {
	      best_penalty = penalty;
	      best_idx = break_in_idx;
	    }

	  if (breaks[break_in_idx].flags & GNOME_TEXT_BREAK_ISSPACE)
	    total_space += breaks[break_in_idx].x1 - breaks[break_in_idx].x0;
	  break_in_idx++;
	}

      result[result_idx++] = best_idx;
      x = breaks[best_idx].x1;
      break_in_idx = best_idx + 1;
    }

  layout->breaks_chosen = result;
  layout->n_lines = result_idx;
}

/* This function creates a single text line from the layout - it can
   be used in cases when it is known that the text will fit in a
   single line. */
GnomeTextLine *
gnome_text_line_from_layout (GnomeTextLayout *layout)
{
  int i, n_attrs;
  GnomeTextGlyphAttrEl *new_attrs;
  GnomeTextGlyph *new_glyphs;
  GnomeTextLine *line;

  for (i = 0; layout->attrs[i].attr != GNOME_TEXT_GLYPH_END; i++);
  n_attrs = i + 1;
  new_attrs = g_new (GnomeTextGlyphAttrEl, n_attrs);
  for (i = 0; i < n_attrs; i++)
    new_attrs[i] = layout->attrs[i];
  new_glyphs = g_new (GnomeTextGlyph, layout->n_glyphs);
  for (i = 0; i < layout->n_glyphs; i++)
    new_glyphs[i] = layout->glyphs[i];

  line = g_new (GnomeTextLine, 1);
  line->attrs = new_attrs;
  line->glyphs = new_glyphs;
  line->n_glyphs = layout->n_glyphs;
  return line;
}

void
gnome_text_line_free (GnomeTextLine *line)
{
  g_free (line->attrs);
  g_free (line->glyphs);
  g_free (line);
}

typedef struct _GnomeTextSpace GnomeTextSpace;

struct _GnomeTextSpace {
  int glyph_pos;
  int space;
};

/* This function creates a flat array of lines from the layout. */
GnomeTextLine **
gnome_text_lines_from_layout (GnomeTextLayout *layout)
{
  int i;
  GnomeTextGlyphAttrEl *new_attrs;
  int n_attrs, n_attrs_max;
  GnomeTextGlyph *new_glyphs;
  GnomeTextLine *line;
  GnomeTextLine **lines;
  GnomeTextGlyphAttrState attr_state;
  int line_idx;
  int glyph_idx, glyph_start;
  int attr_idx;
  int break_idx;
  int n_glyphs;
  GnomeTextBreak *begin_break, *end_break;
  int x_offset;
  GnomeTextGlyphAttrEl *attrs;
  int padding;
  double space_expand;
  int tmp_pos;
  GnomeTextSpace *spaces;
  int space_idx, n_spaces_max;

  memcpy (attr_state, gnome_text_default_glyph_state, sizeof (attr_state));

  lines = g_new (GnomeTextLine *, layout->n_lines + 1);

  if (layout->align == GNOME_TEXT_ALIGN_JUST)
    {
      n_spaces_max = 32;
      spaces = g_new (GnomeTextSpace, n_spaces_max);
    }
  else
    {
      spaces = NULL;
      n_spaces_max = 0;
    }

  break_idx = 0;
  begin_break = NULL;
  glyph_idx = 0;
  attr_idx = 0;
  attrs = layout->attrs;
  for (line_idx = 0; line_idx < layout->n_lines; line_idx++)
    {
      while (attrs[attr_idx].glyph_pos <= glyph_idx)
	{
	  attr_state[attrs[attr_idx].attr] = attrs[attr_idx].attr_val;
	  attr_idx++;
	}
      end_break = &layout->breaks[layout->breaks_chosen[line_idx]];
      if (begin_break)
	{
	  glyph_start = begin_break->begin_glyph_pos;
	  x_offset = -layout->glyphs[glyph_start].x;
	}
      else
	{
	  glyph_start = 0;
	  x_offset = 0;
	}
      n_glyphs = end_break->end_glyph_pos - glyph_start;
      space_expand = 2.0;
      if (layout->align != GNOME_TEXT_ALIGN_LEFT)
	{
	  padding = layout->set_width - (end_break->x0 + x_offset);
	  if (layout->align == GNOME_TEXT_ALIGN_CENTER)
	    x_offset += padding >> 1;
	  else if (layout->align == GNOME_TEXT_ALIGN_RIGHT)
	    x_offset += padding;
	  else if (layout->align == GNOME_TEXT_ALIGN_JUST)
	    {
	      int space, total_space;

	      /* find all the spaces */
	      total_space = 0;
	      space_idx = 0;
	      for (; break_idx < layout->breaks_chosen[line_idx]; break_idx++)
		{
		  if (layout->breaks[break_idx].flags &
		      GNOME_TEXT_BREAK_ISSPACE)
		    {
		      tmp_pos = layout->breaks[break_idx].begin_glyph_pos;
		      if (space_idx == n_spaces_max)
			spaces = g_renew (GnomeTextSpace, spaces,
					  n_spaces_max <<= 1);
		      spaces[space_idx].glyph_pos = tmp_pos;
		      space = layout->breaks[break_idx].x1 -
			layout->breaks[break_idx].x0;
		      spaces[space_idx].space = space;
		      space_idx++;
		      total_space += space;
		    }
		}
	      if (space_idx == n_spaces_max)
		spaces = g_renew (GnomeTextSpace, spaces, n_spaces_max <<= 1);
	      spaces[space_idx].glyph_pos = end_break->end_glyph_pos;
	      spaces[space_idx].space = 0;
	      if (padding < 0 || line_idx != layout->n_lines - 1)
		space_expand = padding * 1.0 / total_space;
	      else
		space_expand = 0;
	      break_idx++; /* skip the line breaking space */
	    }
	}

      /* todo: add in the glyph diff lists */
      n_attrs_max = 4;
      new_attrs = g_new (GnomeTextGlyphAttrEl, n_attrs_max);
      n_attrs = 0;
      for (i = 0; i < GNOME_TEXT_GLYPH_END; i++)
	if (attr_state[i] != gnome_text_default_glyph_state[i])
	  {
	    gnome_text_add_glyph_attr (0, i, attr_state[i],
				       &new_attrs, &n_attrs, &n_attrs_max);
	  }

      new_glyphs = g_new (GnomeTextGlyph, n_glyphs);
      if (layout->align == GNOME_TEXT_ALIGN_JUST)
	{
	  space_idx = 0;
	  for (i = 0; i < n_glyphs; i++)
	    {
	      /* copy attributes over from layout to new line */
	      while (attrs[attr_idx].glyph_pos <= glyph_start + i)
		{
		  attr_state[attrs[attr_idx].attr] = attrs[attr_idx].attr_val;
		  gnome_text_add_glyph_attr (i, attrs[attr_idx].attr,
					     attrs[attr_idx].attr_val,
					     &new_attrs, &n_attrs,
					     &n_attrs_max);
		  attr_idx++;
		}

	      new_glyphs[i].glyph_num = layout->glyphs[glyph_start + i].glyph_num;

	      if (glyph_start + i == spaces[space_idx].glyph_pos)
		{
		  x_offset += floor (spaces[space_idx].space * space_expand +
				     0.5);
		  space_idx++;
		}
	      new_glyphs[i].x = layout->glyphs[glyph_start + i].x + x_offset;
	    }
	}
      else
	{
	  for (i = 0; i < n_glyphs; i++)
	    {
	      /* copy attributes over from layout to new line */
	      while (attrs[attr_idx].glyph_pos <= glyph_start + i)
		{
		  attr_state[attrs[attr_idx].attr] = attrs[attr_idx].attr_val;
		  gnome_text_add_glyph_attr (i, attrs[attr_idx].attr,
					     attrs[attr_idx].attr_val,
					     &new_attrs, &n_attrs,
					     &n_attrs_max);
		  attr_idx++;
		}

	      new_glyphs[i].glyph_num = layout->glyphs[glyph_start + i].glyph_num;
	      new_glyphs[i].x = layout->glyphs[glyph_start + i].x + x_offset;
	    }
	}

      gnome_text_add_glyph_attr (n_glyphs, GNOME_TEXT_GLYPH_END, 0,
				 &new_attrs, &n_attrs, &n_attrs_max);

      line = g_new (GnomeTextLine, 1);
      line->attrs = new_attrs;
      line->glyphs = new_glyphs;
      line->n_glyphs = n_glyphs;
      lines[line_idx] = line;
      begin_break = end_break; /* begin break of next line = end break
				  of this line */
    }

  lines[line_idx] = NULL;

  if (spaces != NULL)
    g_free (spaces);

  return lines;
}

