#ifndef _GNOME_PRINT_PRIVATE_H_
#define _GNOME_PRINT_PRIVATE_H_

#include <stdio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgnome/gnome-defs.h>
#include <libgnomeprint/gp-gc.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-text.h>
#include <libgnomeprint/gnome-glyphlist.h>
#include <libgnomeprint/gnome-print.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>
#include <libart_lgpl/art_svp.h>
#include <libart_lgpl/art_svp_wind.h>

BEGIN_GNOME_DECLS

#if 0
typedef enum {
  GNOME_PRINT_CONTEXT_TYPE_GENERIC_PS
} GnomePrintContextType;
#endif

typedef enum {
	GNOME_PRINT_OUTPUT_NULL,
	GNOME_PRINT_OUTPUT_FILE,
	GNOME_PRINT_OUTPUT_PIPE,
	GNOME_PRINT_OUTPUT_PROGRAM
} GnomePrintOutputType;

struct _GnomePrintContext
{
	GtkObject object;

	GPGC *gc;
	gint level;
	gboolean has_page;

	GnomePrintOutputType output;
	gchar *command;
	gchar *filename;
	FILE *f;
};

/* The method defs, autogenned */
struct _GnomePrintContextClass
{
  GtkObjectClass parent_class;

  int (* newpath)          (GnomePrintContext *pc);

  int (* moveto)           (GnomePrintContext *pc, double x, double y);

  int (* lineto)           (GnomePrintContext *pc, double x, double y);

  int (* curveto)          (GnomePrintContext *pc, double x1, double y1, double x2, double y2, double x3, double y3);

  int (* closepath)        (GnomePrintContext *pc);

  int (* setrgbcolor)      (GnomePrintContext *pc, double r, double g, double b);

  int (* fill)             (GnomePrintContext *pc, ArtWindRule rule);
#if 0
  int (* eofill)           (GnomePrintContext *pc);
#endif
  int (* setlinewidth)     (GnomePrintContext *pc, double width);

  int (* setmiterlimit)    (GnomePrintContext *pc, double limit);

  int (* setlinejoin)      (GnomePrintContext *pc, int jointype);

  int (* setlinecap)       (GnomePrintContext *pc, int captype);

  int (* setdash)          (GnomePrintContext *pc, int n_values, const double *values, double offset);

  int (* strokepath)       (GnomePrintContext *pc);

  int (* stroke)           (GnomePrintContext *pc);

  int (* setfont)          (GnomePrintContext *pc, GnomeFont *font);

  int (* show_sized)       (GnomePrintContext *pc, const char *text, int bytes);

  int (* concat)           (GnomePrintContext *pc, const double matrix[6]);

  int (* gsave)            (GnomePrintContext *pc);

  int (* grestore)         (GnomePrintContext *pc);

  int (* clip)             (GnomePrintContext *pc, ArtWindRule rule);
#if 0
  int (* eoclip)           (GnomePrintContext *pc);
#endif
  int (* grayimage)        (GnomePrintContext *pc, const char *data, int width, int height, int rowstride);

  int (* rgbimage)         (GnomePrintContext *pc, const char *data, int width, int height, int rowstride);

  int (* textline)	   (GnomePrintContext *pc, GnomeTextLine *line);

  int (* beginpage)        (GnomePrintContext *pc, const char *name_of_this_page);

  int (* showpage)         (GnomePrintContext *pc);

  int (* close)            (GnomePrintContext *pc);

  int (* setopacity)       (GnomePrintContext *pc, double opacity);

  int (* rgbaimage)         (GnomePrintContext *pc, const char *data, int width, int height, int rowstride);

  int (* glyphlist) (GnomePrintContext * pc, GnomeGlyphList * glyphlist);
};

/* These are functions for writing bytes to the printer - generally to a
   file or piped to lpr. */

int gnome_print_context_open_file (GnomePrintContext *pc, const char *filename);
int gnome_print_context_write_file (GnomePrintContext *pc, const void *buf, size_t size);
int gnome_print_context_fprintf (GnomePrintContext *pc, const char *fmt, ...);
int gnome_print_context_close_file (GnomePrintContext *pc);

END_GNOME_DECLS

#endif


