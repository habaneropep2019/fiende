/*
 * gnome-print-ps.c: A Postscript driver for GnomePrint 
 *
 * Authors:
 *   Raph Levien (raph@acm.org)
 *   Lauris Kaplinski <lauris@helixcode.com>
 *
 */

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <gtk/gtk.h>

#include <libart_lgpl/art_affine.h>
#include <libgnomeprint/gp-unicode.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-printer-private.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-print-ps.h>
#include <libgnomeprint/gnome-font.h>
#if 0
#include <libgnomeprint/gnome-font-private.h> /* for glyph_name at moment */
#endif

typedef enum {
  GNOME_PRINT_PS_TYPE_GENERIC_PS
} GnomePrintPsType;

struct _GnomePrintPs
{
  GnomePrintContext pc;

  /* Number of showpage() preceeding current state.  */
  int pageno;

  /* the list of fonts that have already been downloaded */
  int n_fonts, n_fonts_max;
  char **fonts;

  /* the list of fonts assumed to already exist in the printer */
  int n_builtins, n_builtins_max;
  char **builtins;

  /* list of fonts that have been reencoded, and exist int printer */
  int  n_fonts_reencoded, n_fonts_reencoded_max;
  char **fonts_reencoded;

  /* current color */
  double r;
  double g;
  double b;

  /* current font */
  char *current_font;
  double current_font_size;
  const GnomeFont *current_gnome_font;

  /* here follows stuff for printing gnome-text lines */
  GnomeTextFontHandle cur_font;
  int cur_size;
};

struct _GnomePrintPsClass
{
  GnomePrintContextClass parent_class;
};

static void gnome_print_ps_class_init (GnomePrintPsClass *klass);

static void
gnome_print_ps_init (GnomePrintPs *ps);

static void
gnome_print_ps_finalize (GtkObject *object);

static void
gnome_print_ps_reencode_font (GnomePrintContext *pc, const char *fontname);

static GnomePrintContextClass *parent_class = NULL;

GtkType
gnome_print_ps_get_type (void)
{
  static GtkType ps_type = 0;

  if (!ps_type)
    {
      GtkTypeInfo ps_info =
      {
	"GnomePrintPs",
	sizeof (GnomePrintPs),
	sizeof (GnomePrintPsClass),
	(GtkClassInitFunc) gnome_print_ps_class_init,
	(GtkObjectInitFunc) gnome_print_ps_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      ps_type = gtk_type_unique (gnome_print_context_get_type (), &ps_info);
    }

  return ps_type;
}

/* The implementations of the PostScript paint methods, autogenned */

static int
gnome_print_ps_newpath (GnomePrintContext *pc)
{
  return gnome_print_context_fprintf (pc,
                                      "newpath\n");
}

static int
gnome_print_ps_moveto (GnomePrintContext *pc, double x, double y)
{
  return gnome_print_context_fprintf (pc,
                                      "%g %g moveto\n",
                                      x,
                                      y);
}

static int
gnome_print_ps_lineto (GnomePrintContext *pc, double x, double y)
{
  return gnome_print_context_fprintf (pc,
                                      "%g %g lineto\n",
                                      x,
                                      y);
}

static int
gnome_print_ps_curveto (GnomePrintContext *pc,
			double x1, double y1,
			double x2, double y2,
			double x3, double y3)
{
  return gnome_print_context_fprintf (pc,
                                      "%g %g %g %g %g %g curveto\n",
                                      x1,
                                      y1,
                                      x2,
                                      y2,
                                      x3,
                                      y3);
}

static int
gnome_print_ps_closepath (GnomePrintContext *pc)
{
  return gnome_print_context_fprintf (pc,
                                      "closepath\n");
}

static int
gnome_print_ps_setrgbcolor (GnomePrintContext *pc,
			    double r, double g, double b)
{
  GnomePrintPs *ps = GNOME_PRINT_PS (pc);

  /* I think there's a problem:
   * gsave, setcolor, grestore means a setrgbcolor is required;
   * ps->r, etc. are the "last set" color, not the "current color"
   * so you need a color stack to do this optimization.
   */

  ps->r = r; ps->g = g; ps->b = b;

  return gnome_print_context_fprintf (pc,
                                      "%g %g %g setrgbcolor\n",
                                      r,
                                      g,
                                      b);
}

static int
gnome_print_ps_fill (GnomePrintContext *pc, ArtWindRule rule)
{
  return gnome_print_context_fprintf (pc,
				      (rule == ART_WIND_RULE_NONZERO) ? "fill\n" : "eofill\n");
}

static int
gnome_print_ps_setlinewidth (GnomePrintContext *pc, double width)
{
  return gnome_print_context_fprintf (pc,
                                      "%g setlinewidth\n",
                                      width);
}

static int
gnome_print_ps_setmiterlimit (GnomePrintContext *pc, double limit)
{
  return gnome_print_context_fprintf (pc,
                                      "%g setmiterlimit\n",
                                      limit);
}

static int
gnome_print_ps_setlinejoin (GnomePrintContext *pc, int jointype)
{
  return gnome_print_context_fprintf (pc,
                                      "%d setlinejoin\n",
                                      jointype);
}

static int
gnome_print_ps_setlinecap (GnomePrintContext *pc, int captype)
{
  return gnome_print_context_fprintf (pc,
                                      "%d setlinecap\n",
                                      captype);
}

static int
gnome_print_ps_setdash (GnomePrintContext *pc, int n_values, const double *values, double offset)
{
  int i, res;

  res = gnome_print_context_fprintf(pc, "[");
  for (i = 0; i < n_values; i++)
    res += gnome_print_context_fprintf(pc, " %g", values[i]);
  res += gnome_print_context_fprintf(pc, "] %g setdash\n", offset);
  return res;
}

static int
gnome_print_ps_strokepath (GnomePrintContext *pc)
{
  return gnome_print_context_fprintf (pc,
                                      "strokepath\n");
}

static int
gnome_print_ps_stroke (GnomePrintContext *pc)
{
  return gnome_print_context_fprintf (pc,
                                      "stroke\n");
}

static int
gnome_print_ps_setopacity (GnomePrintContext *pc, double opacity)
{
  static gboolean warned = FALSE;
  if (!warned) {
    g_warning ("Unimplemented setopacity");
    warned = TRUE;
  }
  return 0;
}

static int
gnome_print_ps_setfont_raw (GnomePrintContext *pc, const GnomeFontFace * face,
			    double size)
{
  GnomePrintPs *ps;
  char *pfa;
  int i;
  char *fontname;

  g_return_val_if_fail (pc != NULL, -1);

  ps = GNOME_PRINT_PS (pc);

  if (face == NULL)
    return -1;

  fontname = g_strdup (gnome_font_face_get_ps_name (face));
  for (i = 0; i < ps->n_builtins; i++)
    if (!strcmp (fontname, ps->builtins[i]))
      break;

  if (i == ps->n_builtins)
    {
      gtk_object_get (GTK_OBJECT (face), "pfbname", &fontname, NULL);
      for (i = 0; i < ps->n_fonts; i++)
	if (!strcmp (fontname, ps->fonts[i]))
	  break;
      if (i == ps->n_fonts)
	{
	  pfa = gnome_font_face_get_pfa (face);
	  if (pfa == NULL) {
	    g_free (fontname);
	    return -1;
	  }
	  
	  if (gnome_print_context_fprintf (pc, "%s", pfa) < 0) {
	    g_free (fontname);
	    return -1;
	  }
	  
	  if (ps->n_fonts == ps->n_fonts_max)
	    ps->fonts = g_realloc (ps->fonts, sizeof (char *) *
				   (ps->n_fonts_max <<= 1));
	  ps->fonts[ps->n_fonts++] = g_strdup (fontname);
	  g_free (pfa);
	  gnome_print_ps_reencode_font (pc, fontname);
	}
    }
  else
    {
      for (i = 0; i < ps->n_fonts_reencoded; i++)
        if (!strcmp (fontname, ps->fonts_reencoded[i]))
	  break;

      if (i == ps->n_fonts_reencoded)
        {
          if (ps->n_fonts_reencoded == ps->n_fonts_reencoded_max)
	    ps->fonts_reencoded = g_realloc (ps->fonts_reencoded, sizeof (char *) *
				            (ps->n_fonts_reencoded_max <<= 1));
	  ps->fonts_reencoded[ps->n_fonts_reencoded++] = g_strdup (fontname);
          gnome_print_ps_reencode_font (pc, fontname);
	}
    }

  ps->current_font = fontname;
  ps->current_font_size = size;

  return gnome_print_context_fprintf (pc,
				      "/La-%s findfont %g scalefont setfont\n",
				      fontname, size);

}

/* todo: just make this call the _raw variant on the unsized font */
static int
gnome_print_ps_setfont (GnomePrintContext *pc, GnomeFont *font)
{
  GnomePrintPs *ps;
  gint ret;

  ps = GNOME_PRINT_PS (pc);

#if 0
  char *pfa;
  int i;
  const char *fontname;

  g_return_val_if_fail (pc != NULL, -1);

  if (font == NULL)
    return -1;

  fontname = gnome_font_get_ps_name (font);
  for (i = 0; i < ps->n_builtins; i++)
    if (!strcmp (fontname, ps->builtins[i]))
      break;

  if (i == ps->n_builtins)
    {
      fontname = gnome_font_get_glyph_name (font);
      for (i = 0; i < ps->n_fonts; i++)
	if (!strcmp (fontname, ps->fonts[i]))
	  break;
      if (i == ps->n_fonts)
	{
	  pfa = gnome_font_get_pfa (font);
	  if (pfa == NULL)
	    return -1;
	  
	  if (gnome_print_context_fprintf (pc, "%s", pfa) < 0)
	    return -1;
	  
	  if (ps->n_fonts == ps->n_fonts_max)
	    ps->fonts = g_realloc (ps->fonts, sizeof (char *) *
				   (ps->n_fonts_max <<= 1));
	  ps->fonts[ps->n_fonts++] = g_strdup (fontname);
	  g_free (pfa);
	  gnome_print_ps_reencode_font (pc, fontname);
	}
    }
  else
    {
      for (i = 0; i < ps->n_fonts_reencoded; i++)
        if (!strcmp (fontname, ps->fonts_reencoded[i]))
	  break;

      if (i == ps->n_fonts_reencoded)
        {
          if (ps->n_fonts_reencoded == ps->n_fonts_reencoded_max)
	    ps->fonts_reencoded = g_realloc (ps->fonts_reencoded, sizeof (char *) *
				            (ps->n_fonts_reencoded_max <<= 1));
	  ps->fonts_reencoded[ps->n_fonts_reencoded++] = g_strdup (fontname);
          gnome_print_ps_reencode_font (pc, fontname);
	}
    }

  ps->current_font = fontname;
  ps->current_font_size = gnome_font_get_size (font);

#else

  ret = gnome_print_ps_setfont_raw (pc, gnome_font_get_face (font), gnome_font_get_size (font));
#endif

  ps->current_gnome_font = font;
  /* FIXME: should we ref the font or are we guaranteed to have it around
     for long enough?  */

  /* invalidate the settings for gnome-text */
  ps->cur_font = 0;
  ps->cur_size = 0;


  return ret;
#if 0
  return gnome_print_context_fprintf (pc,
				      "/La-%s findfont %g scalefont setfont\n",
				      fontname, gnome_font_get_size (font));
#endif
}

static void
gnome_print_ps_reencode_font (GnomePrintContext *pc, const char *fontname)
{
  gnome_print_context_fprintf (pc,"/%s findfont\ndup length dict begin\n"
			       "{1 index /FID ne {def} {pop pop} ifelse}"
			       "forall\n/Encoding ISOLatin1Encoding def\n"
			       "currentdict\nend\n/La-%s exch definefont pop\n",
			       fontname, fontname);
}

static int
gnome_print_ps_show_sized (GnomePrintContext *pc, const char *text, int bytes)
{
  GnomePrintPs *ps;
  gunichar u;
  const char * p;
  ps = GNOME_PRINT_PS (pc);

  if (gnome_print_context_fprintf (pc, "(") < 0)
    return -1;
  for (p = text; p && p < (text + bytes); p = g_utf8_next_char (p))
    {
      u = g_utf8_get_char (p);
      if (u == '(' || u == ')' || u == '\\') 
	{
	  if (gnome_print_context_fprintf (pc, "\\%c", u) < 0)
	    return -1;
	}
      else if ( u == '-' )
        {
	     /* This is a stupid Postscript bug in which all the chars below 127 are
		the same in the Standard Vector and the ISOLatin encodig BUT this one */
          if (gnome_print_context_fprintf (pc, "\\255") < 0)
	    return -1;
	}
      else if ( u > 31 && u < 127)
        {
          if (gnome_print_context_fprintf (pc, "%c", u) < 0)
	    return -1;
	}
      else if ( (u == 153) || (u == 156) || (u > 255) ) 
	{
          /* this chars are .notdef in isoLatin*/ 
	  if (gnome_print_context_fprintf (pc," ") < 0)
	    return -1;
	}
      else if ( (unsigned char) u > 143 )
        {
          if (gnome_print_context_fprintf (pc, "\\%o", u) < 0)
	    return -1;
	}
      else 
	{
	     /* .notdef char */
	  if (gnome_print_context_fprintf (pc, " ") < 0)
	    return -1;
	}
    }
  return gnome_print_context_fprintf (pc, ") show\n");
}

static int
gnome_print_ps_concat (GnomePrintContext *pc, const double matrix[6])
{
  char str[128];

  art_affine_to_string (str, matrix);
  return gnome_print_context_fprintf (pc,
                                      "%s\n", str);
}

static int
gnome_print_ps_gsave (GnomePrintContext *pc)
{
  return gnome_print_context_fprintf (pc,
                                      "gsave\n");
}

static int
gnome_print_ps_grestore (GnomePrintContext *pc)
{
  return gnome_print_context_fprintf (pc,
                                      "grestore\n");
}

static int
gnome_print_ps_clip (GnomePrintContext *pc, ArtWindRule rule)
{
  return gnome_print_context_fprintf (pc,
                                      (rule == ART_WIND_RULE_NONZERO) ? "clip\n" : "eoclip\n");
}

static int
gnome_print_ps_image (GnomePrintContext *pc, const char *data, int width, int height,
		      int rowstride, int bytes_per_pixel)
{
  int status;
  char linebuf[80];
  int x, y;
  int pos;
  int startline, ix;
  unsigned char b;
  const char tohex[16] = "0123456789abcdef";
  int bytes_per_line;

  bytes_per_line = width * bytes_per_pixel;

  status = gnome_print_context_fprintf (pc, "/buf %d string def\n%d %d 8\n",
					bytes_per_line,
					width, height);
  if (status < 0)
    return status;

  status = gnome_print_context_fprintf (pc, "[ %d 0 0 %d 0 %d ]\n",
					width,
					-height,
					height);
  if (status < 0)
    return status;

  status = gnome_print_context_fprintf (pc, "{ currentfile buf readhexstring pop }\n");
  if (status < 0)
    return status;

  if (bytes_per_pixel == 1)
    status = gnome_print_context_fprintf (pc, "image\n");
  else if (bytes_per_pixel == 3)
    status = gnome_print_context_fprintf (pc, "false %d colorimage\n",
					  bytes_per_pixel);
  if (status < 0)
    return status;

  pos = 0;
  startline = 0;
  for (y = 0; y < height; y++)
    {
      ix = startline;
      for (x = 0; x < bytes_per_line; x++)
	{
	  b = data[ix++];
	  linebuf[pos++] = tohex[b >> 4];
	  linebuf[pos++] = tohex[b & 15];
	  if (pos == 72)
	    {
	      linebuf[pos++] = '\n';
	      if (gnome_print_context_write_file (pc, linebuf, pos) < pos)
		return -1;
	      pos = 0;
	    }
	}
      startline += rowstride;
    }
  if (pos)
    {
      linebuf[pos++] = '\n';
      if (gnome_print_context_write_file (pc, linebuf, pos) < pos)
	return -1;
    }
  return 0;
}

static int
gnome_print_ps_grayimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride)
{
  return  gnome_print_ps_image (pc, data, width, height, rowstride, 1);
}

static int
gnome_print_ps_rgbimage (GnomePrintContext *pc, const char *data, int width, int height,
					int rowstride)
{
  return gnome_print_ps_image (pc, data, width, height, rowstride, 3);
}

static int
gnome_print_ps_textline (GnomePrintContext *pc, GnomeTextLine *line)
{
  int i;
  int attr_idx;
  int cur_size;
  int cur_xscale;
  int last_size;
  double scale_factor;
  GnomeTextFontHandle cur_font, last_font;
  GnomePrintPs *ps = GNOME_PRINT_PS (pc);
  GnomeTextGlyphAttrEl *attrs = line->attrs;
  int x;
  int glyph;
  gboolean open;

  cur_font = ps->cur_font;
  last_font = cur_font;
  cur_size = ps->cur_size;
  last_size = cur_size;

  cur_xscale = 1000;
  scale_factor = cur_size * cur_xscale * 1e-9 * GNOME_TEXT_SCALE;

  open = 0;
  x = 0;
  attr_idx = 0;
  for (i = 0; i < line->n_glyphs; i++)
    {
      while (attrs[attr_idx].glyph_pos == i)
	{
	  switch (attrs[attr_idx].attr)
	    {
	    case GNOME_TEXT_GLYPH_FONT:
	      cur_font = attrs[attr_idx].attr_val;
	      break;
	    case GNOME_TEXT_GLYPH_SIZE:
	      cur_size = attrs[attr_idx].attr_val;
	      scale_factor = cur_size * cur_xscale * 1e-9 * GNOME_TEXT_SCALE;
	      break;
	    default:
	      break;
	    }
	  attr_idx++;
	}
      if (cur_size != last_size || cur_font != last_font)
	{
#ifdef VERBOSE
	  g_print ("cur_size = %d, expands to %g\n",
		   cur_size, ps->current_font_size);
#endif
	  if (open)
	    gnome_print_context_fprintf (pc, ") show\n");
	  gnome_print_ps_setfont_raw (pc, gnome_text_get_font (cur_font),
				      cur_size * 0.001);
	  open = 0;
	  last_size = cur_size;
	  last_font = cur_font;
	}
#ifdef VERBOSE
      g_print ("x = %d, glyph x = %d\n",
	       x, line->glyphs[i].x);
#endif
      if (abs (line->glyphs[i].x - x) > 1)
	{
	  gnome_print_context_fprintf (pc, "%s%g 0 rmoveto\n",
				       open ? ") show " : "",
				       ((line->glyphs[i].x - x) * 1.0 /
					GNOME_TEXT_SCALE));
	  open = 0;
	  x = line->glyphs[i].x;
	}
      glyph = line->glyphs[i].glyph_num;
      if (!open)
	gnome_print_context_fprintf (pc, "(");
      if (glyph >= ' ' && glyph < 0x7f)
	if (glyph == '(' || glyph == ')' || glyph == '\\')
	  gnome_print_context_fprintf (pc, "\\%c", glyph);
	else
	  gnome_print_context_fprintf (pc, "%c", glyph);
      else
	gnome_print_context_fprintf (pc, "\\%03o", glyph);
      open = 1;
      x += floor (gnome_text_get_width (cur_font, glyph) * scale_factor + 0.5);
    }
  if (open)
    gnome_print_context_fprintf (pc, ") show\n");
  ps->cur_font = cur_font;
  ps->cur_size = cur_size;
  return 0;
}

static int
gnome_print_ps_showpage (GnomePrintContext *pc)
{
  GnomePrintPs *ps = GNOME_PRINT_PS (pc);
  ps->pageno++;
  return gnome_print_context_fprintf (pc,
                                      "showpage\n");
}

static int
gnome_print_ps_beginpage (GnomePrintContext *pc, const char *name_of_this_page)
{
  GnomePrintPs *ps = GNOME_PRINT_PS (pc);
  return gnome_print_context_fprintf (pc,
                                      "%%%%Page: %s %d\n",
				      name_of_this_page, ps->pageno + 1);
}

static int
gnome_print_ps_close (GnomePrintContext *pc)
{
  GnomePrintPs *ps = GNOME_PRINT_PS (pc);
  gnome_print_context_fprintf (pc,
			       "%%%%Trailer\n"
			       "%%%%Pages: %d\n"
			       "%%%%EOF\n",
			       ps->pageno);
  return gnome_print_context_close_file (pc);
}

static void
gnome_print_ps_class_init (GnomePrintPsClass *class)
{
  GtkObjectClass *object_class;
  GnomePrintContextClass *pc_class;

  object_class = (GtkObjectClass *)class;
  pc_class = (GnomePrintContextClass *)class;

  parent_class = gtk_type_class (gnome_print_context_get_type ());

  object_class->finalize = gnome_print_ps_finalize;

  /* initialization code, autogenned */
  pc_class->newpath = gnome_print_ps_newpath;
  pc_class->moveto = gnome_print_ps_moveto;
  pc_class->lineto = gnome_print_ps_lineto;
  pc_class->curveto = gnome_print_ps_curveto;
  pc_class->closepath = gnome_print_ps_closepath;
  pc_class->setrgbcolor = gnome_print_ps_setrgbcolor;
  pc_class->fill = gnome_print_ps_fill;
  pc_class->setlinewidth = gnome_print_ps_setlinewidth;
  pc_class->setmiterlimit = gnome_print_ps_setmiterlimit;
  pc_class->setlinejoin = gnome_print_ps_setlinejoin;
  pc_class->setlinecap = gnome_print_ps_setlinecap;
  pc_class->setdash = gnome_print_ps_setdash;
  pc_class->strokepath = gnome_print_ps_strokepath;
  pc_class->stroke = gnome_print_ps_stroke;
  pc_class->setfont = gnome_print_ps_setfont;
  pc_class->show_sized = gnome_print_ps_show_sized;
  pc_class->concat = gnome_print_ps_concat;
  pc_class->gsave = gnome_print_ps_gsave;
  pc_class->grestore = gnome_print_ps_grestore;
  pc_class->clip = gnome_print_ps_clip;
  pc_class->grayimage = gnome_print_ps_grayimage;
  pc_class->rgbimage = gnome_print_ps_rgbimage;
  pc_class->textline = gnome_print_ps_textline;
  pc_class->showpage = gnome_print_ps_showpage;
  pc_class->beginpage = gnome_print_ps_beginpage;
  pc_class->setopacity = gnome_print_ps_setopacity;

  pc_class->close = gnome_print_ps_close;
}

/* These are the PostScript 35, assumed to be in the printer. */
static char *gnome_print_ps_builtins[] = {
  "AvantGarde-Book",
  "AvantGarde-BookOblique",
  "AvantGarde-Demi",
  "AvantGarde-DemiOblique",
  "Bookman-Demi",
  "Bookman-DemiItalic",
  "Bookman-Light",
  "Bookman-LightItalic",
  "Courier",
  "Courier-Bold",
  "Courier-BoldOblique",
  "Courier-Oblique",
  "Helvetica",
  "Helvetica-Bold",
  "Helvetica-BoldOblique",
  "Helvetica-Narrow",
  "Helvetica-Narrow-Bold",
  "Helvetica-Narrow-BoldOblique",
  "Helvetica-Narrow-Oblique",
  "Helvetica-Oblique",
  "NewCenturySchlbk-Bold",
  "NewCenturySchlbk-BoldItalic",
  "NewCenturySchlbk-Italic",
  "NewCenturySchlbk-Roman",
  "Palatino-Bold",
  "Palatino-BoldItalic",
  "Palatino-Italic",
  "Palatino-Roman",
  "Symbol",
  "Times-Bold",
  "Times-BoldItalic",
  "Times-Italic",
  "Times-Roman",
  "ZapfChancery-MediumItalic",
  "ZapfDingbats"
};

static void
gnome_print_ps_init (GnomePrintPs *ps)
{
  int i;

  ps->pageno = 0;

  ps->n_fonts = 0;
  ps->n_fonts_reencoded = 0;
  ps->n_fonts_max = 16;
  ps->n_fonts_reencoded_max = 10;
  ps->fonts = g_new (char *, ps->n_fonts_max);

  ps->n_builtins = sizeof(gnome_print_ps_builtins) / sizeof(char *);
  ps->n_builtins_max = sizeof(gnome_print_ps_builtins) / sizeof(char *);
  ps->builtins = g_new (char *, ps->n_builtins_max);

  ps->fonts_reencoded = g_new (char *, ps->n_fonts_reencoded_max);

  for (i = 0; i < sizeof(gnome_print_ps_builtins) / sizeof(char *); i++)
    ps->builtins[i] = gnome_print_ps_builtins[i];

  ps->r = 0;
  ps->g = 0;
  ps->b = 0;

  ps->current_font = NULL;
  ps->current_font_size = 0;
}

/**
 * gnome_print_ps_new:
 * @printer: Where do we print
 *
 * Creates a new Postscript printing context
 *
 * Returns: a new GnomePrintPs object in which you can issue GnomePrint commands.
 */
GnomePrintPs *
gnome_print_ps_new (GnomePrinter *printer)
{
  GnomePrintPs *ps;
  GnomePrintContext *pc;

  ps = gtk_type_new (gnome_print_ps_get_type ());

  if (!gnome_print_context_open_file (GNOME_PRINT_CONTEXT (ps), printer->filename))
    goto failure;

  pc = GNOME_PRINT_CONTEXT (ps);

  if (gnome_print_context_fprintf (pc,
				   "%%!PS-Adobe-2.0\n"
				   "%%%% Creator: Gnome Print Version %s\n"
				   "%%%% DocumentName: %s\n"
				   "%%%% Author: %s\n"
				   "%%%% Pages: (atend)\n"
				   "%%%% EndComments\n\n\n",
				   VERSION,
				   "Document Name Goes Here",
				   "Author Goes Here") < 0)
    goto failure;
  return ps;

 failure:
  gtk_object_unref (GTK_OBJECT (ps));
  return NULL;
}


static void
gnome_print_ps_finalize (GtkObject *object)
{
  int i;
  GnomePrintPs *ps;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GNOME_IS_PRINT_PS (object));

  ps = GNOME_PRINT_PS (object);

  for (i = 0; i < ps->n_fonts; i++)
    g_free (ps->fonts[i]);

  for (i = 0; i < ps->n_fonts_reencoded; i++)
    g_free (ps->fonts_reencoded[i]);

  g_free (ps->fonts);
  g_free (ps->builtins);
  g_free (ps->fonts_reencoded);

  (* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}

