/* 
 * gnome-print-multipage.c
 * Copyright (C) 2000  Helix Code, Inc.
 * Author: Chris Lahey <clahey@helixcode.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <libart_lgpl/art_affine.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-print-multipage.h>
#include <libgnomeprint/gnome-font.h>

typedef enum {
  GNOME_PRINT_MULTIPAGE_TYPE_GENERIC_MULTIPAGE
} GnomePrintMultipageType;

struct _GnomePrintMultipage
{
  GnomePrintContext pc;

  GnomePrintContext *subpc;
  GList *affines; /* Of type double[6] */
  GList *subpage;
};

struct _GnomePrintMultipageClass
{
  GnomePrintContextClass parent_class;
};

static void gnome_print_multipage_class_init (GnomePrintMultipageClass *klass);

static void gnome_print_multipage_init (GnomePrintMultipage *multipage);

static void gnome_print_multipage_finalize (GtkObject *object);

static GList *gnome_print_multipage_affine_list_duplicate(GList *affines);

static GnomePrintContextClass *parent_class = NULL;

GtkType
gnome_print_multipage_get_type (void)
{
  static GtkType multipage_type = 0;

  if (!multipage_type)
    {
      GtkTypeInfo multipage_info =
      {
	"GnomePrintMultipage",
	sizeof (GnomePrintMultipage),
	sizeof (GnomePrintMultipageClass),
	(GtkClassInitFunc) gnome_print_multipage_class_init,
	(GtkObjectInitFunc) gnome_print_multipage_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      multipage_type = gtk_type_unique (gnome_print_context_get_type (), &multipage_info);
    }

  return multipage_type;
}

/* The implementations of the PostScript paint methods, autogenned */

static int
gnome_print_multipage_newpath (GnomePrintContext *pc)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_newpath (multipage->subpc);
}

static int
gnome_print_multipage_moveto (GnomePrintContext *pc, double x, double y)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_moveto (multipage->subpc, x, y);
}

static int
gnome_print_multipage_lineto (GnomePrintContext *pc, double x, double y)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_lineto (multipage->subpc, x, y);
}

static int
gnome_print_multipage_curveto (GnomePrintContext *pc,
			double x1, double y1,
			double x2, double y2,
			double x3, double y3)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_curveto (multipage->subpc, 
			      x1, y1,
			      x2, y2,
			      x3, y3);
}

static int
gnome_print_multipage_closepath (GnomePrintContext *pc)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_closepath (multipage->subpc);
}

static int
gnome_print_multipage_setrgbcolor (GnomePrintContext *pc,
			    double r, double g, double b)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_setrgbcolor (multipage->subpc,
				  r, g, b);
}

static int
gnome_print_multipage_fill (GnomePrintContext *pc, ArtWindRule rule)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  if (rule == ART_WIND_RULE_NONZERO) {
	  return gnome_print_fill (multipage->subpc);
  } else {
	  return gnome_print_eofill (multipage->subpc);
  }
}

static int
gnome_print_multipage_setlinewidth (GnomePrintContext *pc, double width)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_setlinewidth (multipage->subpc, width);
}

static int
gnome_print_multipage_setmiterlimit (GnomePrintContext *pc, double limit)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_setmiterlimit (multipage->subpc, limit);
}

static int
gnome_print_multipage_setlinejoin (GnomePrintContext *pc, int jointype)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_setlinejoin (multipage->subpc, jointype);
}

static int
gnome_print_multipage_setlinecap (GnomePrintContext *pc, int captype)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_setlinecap (multipage->subpc, captype);
}

static int
gnome_print_multipage_setdash (GnomePrintContext *pc, int n_values, const double *values, double offset)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_setdash (multipage->subpc, n_values, values, offset);
}

static int
gnome_print_multipage_strokepath (GnomePrintContext *pc)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_strokepath (multipage->subpc);
}

static int
gnome_print_multipage_stroke (GnomePrintContext *pc)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_stroke (multipage->subpc);
}

/* todo: just make this call the _raw variant on the unsized font */
static int
gnome_print_multipage_setfont (GnomePrintContext *pc, GnomeFont *font)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_setfont (multipage->subpc, font);
}

static int
gnome_print_multipage_show_sized (GnomePrintContext *pc, const char *text, int bytes)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE (pc);
  return gnome_print_show_sized (multipage->subpc, text, bytes);
}

static int
gnome_print_multipage_concat (GnomePrintContext *pc, const double matrix[6])
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_concat (multipage->subpc, matrix);
}

static int
gnome_print_multipage_gsave (GnomePrintContext *pc)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_gsave (multipage->subpc);
}

static int
gnome_print_multipage_grestore (GnomePrintContext *pc)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_grestore (multipage->subpc);
}

static int
gnome_print_multipage_clip (GnomePrintContext *pc, ArtWindRule rule)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  if (rule == ART_WIND_RULE_NONZERO) {
	  return gnome_print_clip (multipage->subpc);
  } else {
	  return gnome_print_eoclip (multipage->subpc);
  }
}

static int
gnome_print_multipage_grayimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_grayimage (multipage->subpc, data, width, height, rowstride);
}

static int
gnome_print_multipage_rgbimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_rgbimage (multipage->subpc, data, width, height, rowstride);
}

static int
gnome_print_multipage_textline (GnomePrintContext *pc, GnomeTextLine *line)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  return gnome_print_textline (multipage->subpc, line);
}

static int
gnome_print_multipage_showpage (GnomePrintContext *pc)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  int error_code;

  error_code = gnome_print_grestore(multipage->subpc);
  if ( error_code ) return error_code;

  multipage->subpage = multipage->subpage->next;
  if ( multipage->subpage == NULL ) 
    {
      multipage->subpage = multipage->affines;
      error_code = gnome_print_showpage (multipage->subpc);
      if ( error_code ) return error_code;
    }
  error_code = gnome_print_gsave(multipage->subpc);
  if ( error_code ) return error_code;
  error_code = gnome_print_concat(multipage->subpc, multipage->subpage->data);
  if ( error_code ) return error_code;
  return 0;
}

static int
gnome_print_multipage_beginpage (GnomePrintContext *pc, const char *name_of_this_page)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE (pc);
  return gnome_print_beginpage (multipage->subpc, name_of_this_page);
}

static int
gnome_print_multipage_close (GnomePrintContext *pc)
{
  GnomePrintMultipage *multipage = GNOME_PRINT_MULTIPAGE(pc);
  if (multipage->affines != multipage->subpage)
    gnome_print_showpage(multipage->subpc);
  return gnome_print_context_close (multipage->subpc);
}

static void
gnome_print_multipage_class_init (GnomePrintMultipageClass *class)
{
  GtkObjectClass *object_class;
  GnomePrintContextClass *pc_class;

  object_class = (GtkObjectClass *)class;
  pc_class = (GnomePrintContextClass *)class;

  parent_class = gtk_type_class (gnome_print_context_get_type ());

  object_class->finalize = gnome_print_multipage_finalize;

  /* initialization code, autogenned */
  pc_class->newpath = gnome_print_multipage_newpath;
  pc_class->moveto = gnome_print_multipage_moveto;
  pc_class->lineto = gnome_print_multipage_lineto;
  pc_class->curveto = gnome_print_multipage_curveto;
  pc_class->closepath = gnome_print_multipage_closepath;
  pc_class->setrgbcolor = gnome_print_multipage_setrgbcolor;
  pc_class->fill = gnome_print_multipage_fill;
  pc_class->setlinewidth = gnome_print_multipage_setlinewidth;
  pc_class->setmiterlimit = gnome_print_multipage_setmiterlimit;
  pc_class->setlinejoin = gnome_print_multipage_setlinejoin;
  pc_class->setlinecap = gnome_print_multipage_setlinecap;
  pc_class->setdash = gnome_print_multipage_setdash;
  pc_class->strokepath = gnome_print_multipage_strokepath;
  pc_class->stroke = gnome_print_multipage_stroke;
  pc_class->setfont = gnome_print_multipage_setfont;
  pc_class->show_sized = gnome_print_multipage_show_sized;
  pc_class->concat = gnome_print_multipage_concat;
  pc_class->gsave = gnome_print_multipage_gsave;
  pc_class->grestore = gnome_print_multipage_grestore;
  pc_class->clip = gnome_print_multipage_clip;
  pc_class->grayimage = gnome_print_multipage_grayimage;
  pc_class->rgbimage = gnome_print_multipage_rgbimage;
  pc_class->textline = gnome_print_multipage_textline;
  pc_class->showpage = gnome_print_multipage_showpage;
  pc_class->beginpage = gnome_print_multipage_beginpage;

  pc_class->close = gnome_print_multipage_close;
}

static void
gnome_print_multipage_init (GnomePrintMultipage *multipage)
{
  multipage->affines = NULL;
  multipage->subpage = NULL;
  multipage->subpc = NULL;
}

static GList *
gnome_print_multipage_affine_list_duplicate(GList *affines)
{
	GList *list = NULL;

	while (affines) {
		gdouble *affine;
		affine = g_new (gdouble, 6);
		memcpy (affine, affines->data, sizeof (gdouble) * 6);
		list = g_list_prepend (list,affine);
		affines = affines->next;
	}

	list = g_list_reverse (list);

	return list;
}

/**
 * gnome_print_multipage_new:
 * @subpc: Where do we print
 * @affines: List of positions for pages.  There must be at least one item in this list.
 *
 * Creates a new Postscript printing context
 *
 * Returns: a new GnomePrintMultipage object in which you can issue GnomePrint commands.
 */
GnomePrintMultipage *
gnome_print_multipage_new (GnomePrintContext *subpc, GList *affines /* Of type double[6] */)
{
  GnomePrintMultipage *multipage;
  gint error_code;

  g_return_val_if_fail(subpc != NULL, NULL);
  g_return_val_if_fail(GNOME_IS_PRINT_CONTEXT(subpc), NULL);
  g_return_val_if_fail(affines != NULL, NULL);

  multipage = gtk_type_new (gnome_print_multipage_get_type ());

  multipage->subpc = subpc;
  multipage->affines = gnome_print_multipage_affine_list_duplicate(affines);
  multipage->subpage = multipage->affines;
  gtk_object_ref(GTK_OBJECT(subpc));

  error_code = gnome_print_gsave(multipage->subpc);
  if ( error_code ) {
    gtk_object_unref(GTK_OBJECT(multipage));
    return NULL;
  }
  error_code = gnome_print_concat(multipage->subpc, multipage->subpage->data);
  if ( error_code ) {
    gtk_object_unref(GTK_OBJECT(multipage));
    return NULL;
  }
  
  return multipage;
}


/**
 * gnome_print_multipage_new_from_sizes:
 * @subpc: Where do we print
 * @paper_width: Width of paper to print on.
 * @paper_height: Height of paper to print on.
 * @page_width: Width of page to print.
 * @page_height: Height of page to print.
 *
 * Creates a new Postscript printing context
 *
 * Returns: a new GnomePrintMultipage object in which you can issue GnomePrint commands.
 */
GnomePrintMultipage *
gnome_print_multipage_new_from_sizes (GnomePrintContext *subpc, gdouble paper_width, gdouble paper_height, gdouble page_width, gdouble page_height)
{
  GnomePrintMultipage *multipage;
  gint same_count, opposite_count;
  gdouble start_affine[6];
  gdouble x_affine[6];
  gdouble y_affine[6];
  gdouble current_affine[6];
  int x_count;
  int y_count;
  int x;
  int y;
  gint error_code;

  g_return_val_if_fail(subpc != NULL, NULL);

  same_count = ((int)(paper_width / page_width)) * ((int)(paper_height / page_height));
  opposite_count = ((int)(paper_width / page_height)) * ((int)(paper_height / page_width));
  
  if (same_count >= opposite_count) {
    art_affine_translate(start_affine, 0, paper_height - page_height);
    art_affine_translate(x_affine, page_width, 0);
    art_affine_translate(y_affine, 0, -page_height);
    x_count = ((int)(paper_width / page_width));
    y_count = ((int)(paper_height / page_height));
  } else {
    gdouble translation[6];
    art_affine_rotate(start_affine, -90);
    art_affine_translate(translation, paper_width - page_height, paper_height);
    art_affine_multiply(start_affine, start_affine, translation);
    art_affine_translate(x_affine, 0, -page_width);
    art_affine_translate(y_affine, -page_height, 0);
    x_count = ((int)(paper_width / page_height));
    y_count = ((int)(paper_height / page_width));
  }

  multipage = gtk_type_new (gnome_print_multipage_get_type ());

  multipage->subpc = subpc;
  for ( x = 0; x < x_count; x++ )
    {
      memcpy(current_affine, start_affine, 6 * sizeof(gdouble));
      for ( y = 0; y < y_count; y++ ) 
	{
	  gdouble *affine;
	  affine = g_new(gdouble, 6);
	  memcpy(affine, current_affine, 6 * sizeof(gdouble));
	  multipage->affines = g_list_append(multipage->affines, affine);
	  art_affine_multiply(current_affine, current_affine, x_affine);
	}
      art_affine_multiply(start_affine, start_affine, y_affine);
  }
  multipage->subpage = multipage->affines;

  gtk_object_ref(GTK_OBJECT(subpc));

  error_code = gnome_print_gsave(multipage->subpc);
  if ( error_code ) {
    gtk_object_unref(GTK_OBJECT(multipage));
    return NULL;
  }
  error_code = gnome_print_concat(multipage->subpc, multipage->subpage->data);
  if ( error_code ) {
    gtk_object_unref(GTK_OBJECT(multipage));
    return NULL;
  }
  
  return multipage;
}

static void
gnome_print_multipage_finalize (GtkObject *object)
{
  GnomePrintMultipage *multipage;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GNOME_IS_PRINT_MULTIPAGE (object));

  multipage = GNOME_PRINT_MULTIPAGE(object);
  
  gtk_object_unref(GTK_OBJECT(multipage->subpc));

  g_list_foreach(multipage->affines, (GFunc) g_free, NULL);
  g_list_free(multipage->affines);

  (* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}
