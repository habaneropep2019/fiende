#ifndef __GNOME_PRINT_H__
#define __GNOME_PRINT_H__

#include <stdio.h> /* for FILE */
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgnome/gnome-defs.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-text.h>
#include <libgnomeprint/gnome-glyphlist.h>
#include <libart_lgpl/art_vpath.h>
#include <libart_lgpl/art_bpath.h>

BEGIN_GNOME_DECLS

/*
 * These are currently defined values for gnome_print_xxx results
 * feel free to add more, if those make sense
 */

typedef enum {
	GNOME_PRINT_OK = 0,
	GNOME_PRINT_ERROR_UNKNOWN = -1,
	GNOME_PRINT_ERROR_BADVALUE = -2,
	GNOME_PRINT_ERROR_NOCURRENTPOINT = -3,
	GNOME_PRINT_ERROR_NOCURRENTPATH = -4,
	GNOME_PRINT_ERROR_TEXTCORRUPT = -5,
}GnomePrintReturnCode;

#define GNOME_TYPE_PRINT_CONTEXT         (gnome_print_context_get_type ())
#define GNOME_PRINT_CONTEXT(obj)         (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_CONTEXT, GnomePrintContext))
#define GNOME_PRINT_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_CONTEXT, GnomePrintContextClass))
#define GNOME_IS_PRINT_CONTEXT(obj)      (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_CONTEXT))
#define GNOME_IS_PRINT_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_CONTEXT))

typedef struct _GnomePrintContext       GnomePrintContext;
typedef struct _GnomePrintContextClass  GnomePrintContextClass;

/*
 * INTERNALS ARE PRIVATE UNTIL STABILIZED
 */

GtkType gnome_print_context_get_type (void);

/*
 * These generate default context for given printer and do not allow
 * setting any driver-specific parameters.
 * The real constructor method will be handled by printmanager
 */

GnomePrintContext *gnome_print_context_new (GnomePrinter *printer);
GnomePrintContext *gnome_print_context_new_with_paper_size (GnomePrinter *, const char *paper_size);

int gnome_print_context_close (GnomePrintContext *pc);

/* Path manipulation */

int gnome_print_newpath (GnomePrintContext *pc);
int gnome_print_moveto (GnomePrintContext *pc, double x, double y);
int gnome_print_lineto (GnomePrintContext *pc, double x, double y);
int gnome_print_curveto (GnomePrintContext *pc, double x1, double y1, double x2, double y2, double x3, double y3);
int gnome_print_closepath (GnomePrintContext *pc);
int gnome_print_strokepath (GnomePrintContext *pc);
void gnome_print_vpath (GnomePrintContext * gpc, ArtVpath * vpath, gboolean append);
void gnome_print_bpath (GnomePrintContext * gpc, ArtBpath * bpath, gboolean append);

/* Graphic state manipulation */

int gnome_print_setrgbcolor (GnomePrintContext *pc, double r, double g, double b);
int gnome_print_setopacity (GnomePrintContext *pc, double opacity);
int gnome_print_setlinewidth (GnomePrintContext *pc, double width);
int gnome_print_setmiterlimit (GnomePrintContext *pc, double limit);
int gnome_print_setlinejoin (GnomePrintContext *pc, int jointype);
int gnome_print_setlinecap (GnomePrintContext *pc, int captype);
int gnome_print_setdash (GnomePrintContext *pc, int n_values, const double *values, double offset);
int gnome_print_setfont (GnomePrintContext *pc, GnomeFont *font);
int gnome_print_clip (GnomePrintContext *pc);
int gnome_print_eoclip (GnomePrintContext *pc);

/* CTM manipulation */

int gnome_print_concat (GnomePrintContext *pc, const double matrix[6]);
int gnome_print_scale (GnomePrintContext *pc, double sx, double sy);
int gnome_print_rotate (GnomePrintContext *pc, double theta_in_degrees);
int gnome_print_translate (GnomePrintContext *pc, double x, double y);

/* Stack */

int gnome_print_gsave (GnomePrintContext *pc);
int gnome_print_grestore (GnomePrintContext *pc);

/* Painting */

int gnome_print_fill (GnomePrintContext *pc);
int gnome_print_eofill (GnomePrintContext *pc);
int gnome_print_stroke (GnomePrintContext *pc);

/* Text drawing */

int gnome_print_show (GnomePrintContext *pc, char const *text);
int gnome_print_show_sized (GnomePrintContext *pc, const char *text, int bytes);
/* WARNING: show_ucs4 is DEPRECATED */
/* WARNING: it uses BIG ENDIAN integers */
int gnome_print_show_ucs4 (GnomePrintContext *pc, guint32 *buf, gint length);
int gnome_print_textline (GnomePrintContext *pc, GnomeTextLine *line);
int gnome_print_glyphlist (GnomePrintContext *pc, GnomeGlyphList * glyphlist);

/* Images */

int gnome_print_grayimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride);
int gnome_print_rgbimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride);
int gnome_print_rgbaimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride);
int gnome_print_pixbuf (GnomePrintContext *pc, GdkPixbuf *pixbuf);

/* General */

int gnome_print_beginpage (GnomePrintContext *pc, const char *name_of_this_page);
int gnome_print_showpage (GnomePrintContext *pc);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_H__ */

