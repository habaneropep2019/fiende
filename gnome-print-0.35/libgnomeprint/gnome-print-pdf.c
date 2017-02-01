/* -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/*
 * gnome-print-pdf.c: A PDF driver for GnomePrint 
 *
 * (C) 2000 Jose M Celorio
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * References :
 * 
 * [1] Portable Document Format Referece Manual, Version 1.3 (March 11, 1999)
 *
 * Authors:
 *   Chema Celorio (chema@celorio.com)
 *
 *
 */


/*

	This driver needs some love. I need to rewrite parts of it.
	This is the stuff I plan to do :

	1. Remove the _moveto _lineto _curveto etc .. functions.
	us gp_gc instead.

	2. We are sucking memory when printing images, althou this is
	not a problem with your normal Image, for large images is.
	We need to modify the gnome_print_encode functions so that we can
	use them "blockified" so we would do something like :

	temp = g_malloc (SOME_CONSTANT_A);
	gnome_print_encode_ascii85_ini (SOME_CONSTANT_A);
	while (offset < size) {
	   gnome_print_encode_ascii85_body (buffer_in, offset, temp)
		 gnome_print_pdf_write (temp, SOME_CONSTANT_A)
		 offset += SOME_CONSTANT_A;
 }
 gnome_print_endode_ascii85_end (); (or flush ? )

 and thus only using SOME_CONSTANT_A memory for the process.

 3. Move the gnome-print images printing to gdk_pixbufs. Ref them
 use them and unref them. For the old functions, create a pixbuf,
 print with it and destroy it.

 4. Finish the Font subseting code for type1 fonst. (I think we can also
 compress the fonts with /Filter .


 5. (minor) Add page name to pdf file (set by begin page)
 6. (minor) Add /Creator ( program name that created the PDF (ie. "gedit", "gnumeric")

 7. Add JPEG Compression. And CCTII (or whatever) compression for images. gdk_pixbuf
 might do them for us.

 8. For non ortogonal scales, we are not honoring the scale factors (see setlinewidth
 for example)

 9. We need to be able to start writing to disk the content of the pages, we need to
 compress them so this can be tricky but fun :-).
 
 Chema
 (Nov 27, 2000)

*/


/* __FUNCTION__ is not defined in Irix. thanks, <drk@sgi.com>*/
#ifndef __GNUC__
  #define __FUNCTION__   ""
#endif
#define debug(section,str) if (FALSE) printf ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str); 
	
#include "config.h"
#include <unistd.h> /* For getpid () */
#include <time.h>   /* For time() */
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>
#include <locale.h>

#include <libart_lgpl/art_affine.h>
#include <libart_lgpl/art_misc.h>
#include <libgnome/gnome-paper.h>
#include <libgnomeprint/gp-unicode.h>
#include <libgnomeprint/gnome-print-pdf.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-printer-private.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-print-encode-private.h>
#include <libgnomeprint/gnome-print-pdf-type1.h>

#ifdef ENABLE_LIBGPA
#include <libgpa/gpa-printer.h>
#include <libgpa/gpa-settings.h>
#endif


#define EOL "\r\n"
#define GNOME_PRINT_PDF_BUFFER_SIZE 1024

#if 1 /* Use a very low number of elements so that we reallocate stuff
				 and test the code for array growing */
#define GNOME_PRINT_NUMBER_OF_ELEMENTS 2
#define GNOME_PRINT_NUMBER_OF_ELEMENTS_GROW 2
#else
#define GNOME_PRINT_NUMBER_OF_ELEMENTS 16
#define GNOME_PRINT_NUMBER_OF_ELEMENTS_GROW 8
#endif

#define GNOME_PRINT_PDF_FONT_UNDEFINED 9999

static GnomePrintContextClass *parent_class = NULL;

typedef struct _GnomePrintDash            GnomePrintDash;
typedef struct _GnomePrintPdfPage         GnomePrintPdfPage;
typedef struct _GnomePrintPdfObject       GnomePrintPdfObject;
typedef struct _GnomePrintPdfImage        GnomePrintPdfImage;
typedef struct _GnomePrintPdfGsave        GnomePrintPdfGsave;
typedef struct _GnomePrintPdfGraphicState GnomePrintPdfGraphicState;

typedef enum {
	PDF_COLOR_MODE_DEVICEGRAY,
	PDF_COLOR_MODE_DEVICERGB,
	PDF_COLOR_MODE_DEVICECMYK,
	PDF_COLOR_MODE_UNDEFINED
}PdfColorModes;

typedef enum {
	PDF_GRAPHIC_MODE_GRAPHICS,
	PDF_GRAPHIC_MODE_TEXT,
	PDF_GRAPHIC_MODE_UNDEFINED,
}PdfGraphicModes;

typedef enum {
	PDF_COLOR_GROUP_FILL,
	PDF_COLOR_GROUP_STROKE,
	PDF_COLOR_GROUP_BOTH
}PdfColorGroup;

typedef enum {
	PDF_IMAGE_GRAYSCALE,
	PDF_IMAGE_RGB,
}PdfImageType;

/* This are really compression/encoding methods */
typedef enum {
	PDF_COMPRESSION_NONE,
	PDF_COMPRESSION_FLATE,
	PDF_COMPRESSION_HEX
}PdfCompressionType;

struct _GnomePrintDash {
  gint   number_values;
  double phase;
  double *values;
};

struct _GnomePrintPdfObject {
	guint number;
	guint offset;
};

struct _GnomePrintPdfImage {
	gchar *data;
	gint data_length;
	gint width;
	gint height;
	gint rowstride;
	gint bytes_per_pixel;
	gint image_number;
	gint object_number;
	gint image_type;
	PdfCompressionType compr_type;
};


struct _GnomePrintPdfGsave {
	gint graphics_mode;
	GnomePrintPdfGraphicState *graphic_state;
	GnomePrintPdfGraphicState *graphic_state_set;
};

struct _GnomePrintPdfPage{
	/* We need to warn when a page
		 in not beeing showpaged !*/
	gint showpaged : 1;

	gint used_color_images : 1;
	gint used_grayscale_images :1;
	
	/* Page number */
	guint  page_number;
	gchar *page_name;
	
	/* object numbers */
	guint object_number_page;
	guint object_number_contents;
	guint object_number_resources;

	/* Contents */
	gchar *stream;
	gint   stream_used;
	gint   stream_allocated;

	/* Resources */
	GList *resources;

	/* Images */
	gint images_max;
	gint images_number;
	GnomePrintPdfImage *images;
};

struct _GnomePrintPdfGraphicState{
	gboolean dirty        : 1;
	gboolean written      : 1;
	gboolean dirty_text   : 1;
	gboolean written_text : 1;

	/* CTM */
	double ctm[6];
	
	/* Current Path */
	GPPath * current_path;

	/* Color */
	gint   color_stroke_mode;
	gint   color_fill_mode;
	double color_stroke[4];
	double color_fill  [4];
	
	/* Line stuff */
	gint linejoin;     /* 0, 1 or 2 */
	gint linecap;      /* 0, 1 or 2 */
	gint flat;         /* 0 <-> 100 */
	double miterlimit; /* greater or equal than 1 */
	double linewidth;  /* 0 or greater */
	
	/* Line dash */
	GnomePrintDash dash;

  /* Font */
	gint   pdf_font_number;
  double font_size;
	double font_character_spacing;
	double font_word_spacing;
	gint text_flag : 1;
	
};

typedef struct {
	char *font_name;
	gint is_basic_14;
} ps_internal_font;

struct _GnomePrintPdf {
	
	GnomePrintContext pc;

#ifdef ENABLE_LIBGPA	
	/* GpaPrinter */
	GpaPrinter *gpa_printer;
	GpaSettings *gpa_settings;
#endif	
	gboolean ascii_format;

	const GnomePaper *paper;

	/* Graphic states */
	GnomePrintPdfGraphicState *graphic_state;
	GnomePrintPdfGraphicState *graphic_state_set;
	gint  graphics_mode;

	/* Offset in the output file */
	guint offset;
	
	/* Objects */
	GList *objects;
	guint object_number_last_used;
	guint object_number_pages;
	guint object_number_catalog;
	guint object_number_current;
	guint object_number_info;
	guint object_number_gstate;

	/* xref location */
	guint xref_location;

	/* Page stuff */
	GList             *pages;
	gint               current_page_number;
	GnomePrintPdfPage *current_page;

  /* Fonts */
  gint  fonts_internal_number;
  ps_internal_font *fonts_internal;
  gint fonts_max;
	gint fonts_number;
	GnomePrintPdfFont* fonts;

	/* gsave/grestore */
	gint gsave_level_number;
	gint gsave_level_max;
	GnomePrintPdfGsave *gsave_stack;
};

struct _GnomePrintPdfClass
{
  GnomePrintContextClass parent_class;
};

static void gnome_print_pdf_class_init    (GnomePrintPdfClass *klass);
static void gnome_print_pdf_init          (GnomePrintPdf *pdf);
static void gnome_print_pdf_finalize      (GtkObject *object);

gint gnome_print_pdf_write         (GnomePrintContext *pc, const char *format, ...);
static gint gnome_print_pdf_graphic_mode_set (GnomePrintPdf *pdf, gint mode);


/* -------------------------- PUBLIC FUNCTIONS ------------------------ */
GnomePrintPdf *
gnome_print_pdf_new_with_paper (GnomePrinter *printer, const gchar *paper_name)
{
	GnomePrintPdf *pdf;
	GnomePrintContext *pc;
	const GnomePaper *paper;
	gint ret = 0;

	pdf = gtk_type_new (gnome_print_pdf_get_type ());

	if (!gnome_print_context_open_file (GNOME_PRINT_CONTEXT (pdf), printer->filename))
		goto failure;

	paper = gnome_paper_with_name (paper_name);
	if (!paper) {
		g_warning ("file %s: line %d: Cannot find paper %s", __FILE__, __LINE__, paper_name);
	}

	pdf->paper = paper;
	
#ifdef ENABLE_LIBGPA
	g_return_val_if_fail (GPA_IS_PRINTER (printer->gpa_printer), NULL);
	/* We take the first settings for now */
	g_return_val_if_fail (GPA_IS_SETTINGS (printer->gpa_settings), NULL);
	
	pdf->gpa_printer  = printer->gpa_printer;
	pdf->gpa_settings = printer->gpa_settings;

	/* We don't need to ref the printer, because the settings should
	 * ref it (and unref it) but I don't think libgpa settings is
	 * refing the printer now. Just be safe now, but FIX gpa-settings
	 * so that it refs the printer. Or what do you think ? Chema
	 */
	gpa_printer_ref (printer->gpa_printer);
	gpa_settings_ref (printer->gpa_settings);

	pdf->ascii_format = gpa_settings_query_options_boolean (pdf->gpa_settings,
																													"AsciiFormat");
#endif
	
	pc = GNOME_PRINT_CONTEXT (pdf);

	/* for now, we don't need to require 1.3. This could change ...*/
	ret += gnome_print_pdf_write (pc,"%cPDF-1.2" EOL, '%');
	
	if ( ret < 0)
		goto failure;

	/* binary file [1] Page 22-23 */
	ret += gnome_print_pdf_write (pc,"%c%c%c%c%c" EOL,
																'%',181,237,174,251);

	return pdf;

	failure:
	
	g_warning ("gnome_print_pdf_new: PDF new failure ..\n");
	gtk_object_unref (GTK_OBJECT (pdf));
	return NULL;
}


GtkType
gnome_print_pdf_get_type (void)
{
  static GtkType pdf_type = 0;

  if (!pdf_type)
    {
      GtkTypeInfo pdf_info =
      {
				"GnomePrintPdf",
				sizeof (GnomePrintPdf),
				sizeof (GnomePrintPdfClass),
				(GtkClassInitFunc)  gnome_print_pdf_class_init,
				(GtkObjectInitFunc) gnome_print_pdf_init,
				/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      pdf_type = gtk_type_unique (gnome_print_context_get_type (), &pdf_info);
    }

  return pdf_type;
}

gint
gnome_print_pdf_write (GnomePrintContext *pc, const char *format, ...)
{
	GnomePrintPdf *pdf;
	va_list arguments;
	gchar *text;
	gchar *oldlocale;
	
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	oldlocale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");
		
	va_start (arguments, format);
	text = g_strdup_vprintf (format, arguments);
	va_end (arguments);

	pdf->offset += gnome_print_context_write_file (pc, text, strlen(text));

	g_free (text);

	setlocale (LC_NUMERIC, oldlocale);
	g_free (oldlocale);			
	
	return 0;
}

gint
gnome_print_pdf_add_bytes_written (GnomePrintPdf *pdf, gint bytes)
{
	g_return_val_if_fail (GNOME_IS_PRINT_PDF (pdf), -1);
	
	pdf->offset += bytes;

	return 0;
}


gint
gnome_print_pdf_object_start (GnomePrintContext *pc, guint object_number)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfObject *object;
	gint ret = 0;
	gint temp_num = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	g_return_val_if_fail (pdf->object_number_current == 0, -1);

	temp_num = pdf->object_number_last_used - object_number;
	object = g_list_nth_data (pdf->objects, temp_num);
	
	g_return_val_if_fail (object != NULL, -1);

	pdf->object_number_current = object_number;

	object->number = object_number;
	object->offset = pdf->offset;

	ret += gnome_print_pdf_write   (pc,
																	"%i 0 obj" EOL
																	"<<" EOL,
																	object_number);
	return ret;
}

gint
gnome_print_pdf_object_end (GnomePrintContext *pc, guint object_number, guint dont_print)
{
	gint ret = 0;
	GnomePrintPdf *pdf;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	g_return_val_if_fail (pdf->object_number_current == object_number, -1);

	pdf->object_number_current = 0;
	
	if (!dont_print)
		ret += gnome_print_pdf_write  (pc,
																	 ">>" EOL
																	 "endobj" EOL,
																	 object_number);
	
	return ret;
}

/* -------------------------- END: PUBLIC FUNCTIONS ------------------------ */
static guint
gnome_print_pdf_object_number (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfObject *object;

	debug (FALSE, "");
	
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	
	object = g_new (GnomePrintPdfObject, 1);

	pdf->objects = g_list_prepend (pdf->objects, object);

	return ++pdf->object_number_last_used;
}

static GnomePrintPdfGraphicState *
gnome_print_pdf_graphic_state_current (GnomePrintPdf *pdf, gint dirtyfy)
{
	GnomePrintPdfGraphicState *gs;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_PDF(pdf), NULL);

	gs = pdf->graphic_state;

	if (dirtyfy)
		gs->dirty = TRUE;
	
	return gs;
	
}
	
static gint
gnome_print_pdf_page_start (GnomePrintContext *pc)
{
	GnomePrintPdfPage *page;
	GnomePrintPdf *pdf;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	g_return_val_if_fail (pdf->current_page == NULL, -1);

	page = g_new (GnomePrintPdfPage, 1);

	pdf->current_page = page;

	page->page_name = NULL;
	page->showpaged = FALSE;
	page->used_color_images = FALSE;
	page->used_grayscale_images = FALSE;
	page->page_number = pdf->current_page_number++;
	page->stream_allocated = GNOME_PRINT_PDF_BUFFER_SIZE;
	page->stream = g_malloc (page->stream_allocated);
	page->stream[0] = 0;
	page->stream_used = 0;

	page->images_max = GNOME_PRINT_NUMBER_OF_ELEMENTS;
	page->images_number = 0;
	page->images = g_new (GnomePrintPdfImage, page->images_max);
	
	return 0;
}

/**
 * gnome_print_pdf_compr_from_string:
 * @str: 
 * 
 * Given the string of a compression method, it returns it's
 * enum value.
 * 
 * Return Value: Enum value of the compression. PDF_COMPRESSION_NONE on error;
 *
 **/
#ifdef ENABLE_LIBGPA
static PdfCompressionType
gnome_print_pdf_compr_from_string (const gchar *str)
{
	g_return_val_if_fail (str != NULL, PDF_COMPRESSION_NONE);

	if (strcmp ("NoCompression", str) == 0)
		return PDF_COMPRESSION_NONE;
	else if (strcmp ("FlateCompression", str) == 0)
		return PDF_COMPRESSION_FLATE;
	else if (strcmp ("HexEncoded", str) == 0)
		return PDF_COMPRESSION_HEX;
	else
		g_warning ("Could not determine compression from %s\n", str);

	return 	PDF_COMPRESSION_NONE;
}
#endif

static gint
gnome_print_pdf_write_compression_filters (GnomePrintContext *pc, PdfCompressionType compr_type)
{
	GnomePrintPdf *pdf;
	gint ret = 0;

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (GNOME_IS_PRINT_PDF (pdf), -1);
	
	if (compr_type == PDF_COMPRESSION_NONE &&	!pdf->ascii_format)
		return ret;
	
	ret += gnome_print_pdf_write  (pc, "/Filter [");

	if (pdf->ascii_format && compr_type != PDF_COMPRESSION_HEX)
		ret += gnome_print_pdf_write  (pc,"/ASCII85Decode ");
	
	if (compr_type == PDF_COMPRESSION_FLATE) 
		ret += gnome_print_pdf_write  (pc, "/FlateDecode ");

	if (compr_type == PDF_COMPRESSION_HEX)
		ret += gnome_print_pdf_write  (pc,"/ASCIIHexDecode ");

	ret += gnome_print_pdf_write  (pc,"]" EOL);

	return ret;
}

static gint
gnome_print_pdf_write_stream (GnomePrintContext *pc,
															gchar *stream,
															gint length,
															PdfCompressionType compr_type)
{
	gint ret = 0;

#ifdef ENABLE_LIBGPA
	GnomePrintPdf *pdf;
	gchar *compressed_stream = NULL;

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (GNOME_IS_PRINT_PDF (pdf), -1);
	
	if (pdf->ascii_format && compr_type != PDF_COMPRESSION_HEX) {
		gint length_needed;
		gint real_length;

		length_needed = gnome_print_encode_ascii85_wcs (length);
		compressed_stream	= g_malloc (length_needed);
		real_length = gnome_print_encode_ascii85 (stream,
																							compressed_stream,
																							length);
		length = real_length;
		stream = compressed_stream;
	} 
	
#endif

	ret += gnome_print_pdf_write  (pc, "/Length %i" EOL, length);
	ret += gnome_print_pdf_write_compression_filters (pc, compr_type);
	ret += gnome_print_pdf_write  (pc, ">>" EOL);
	ret += gnome_print_pdf_write  (pc, "stream" EOL);

	ret += gnome_print_context_write_file (pc,
																				 stream,
																				 length);
	
#ifdef ENABLE_LIBGPA
	if (pdf->ascii_format) {
		g_free (compressed_stream);
	}
#endif

	return ret;
}


static gint
gnome_print_pdf_page_write_contents (GnomePrintContext *pc, GnomePrintPdfPage *page)
{
	GnomePrintPdf *pdf;
	PdfCompressionType compr_type;
#ifdef ENABLE_LIBGPA
	const gchar *compr_string;
#endif
	gchar *compressed_stream = NULL;
	gint ret = 0;
	gint compressed_stream_length;
	gint stream_length;
	gint real_length = 0;
	

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	g_return_val_if_fail (page != NULL, -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (GNOME_IS_PRINT_PDF (pdf), -1);

#ifdef ENABLE_LIBGPA
	compr_string = gpa_settings_query_options (pdf->gpa_settings, "TextCompression");
	compr_type = gnome_print_pdf_compr_from_string (compr_string);
#else
	compr_type = PDF_COMPRESSION_NONE;
#endif

	switch (compr_type) {
	case PDF_COMPRESSION_FLATE:
		stream_length =  page->stream_used;
		compressed_stream_length = gnome_print_encode_deflate_wcs (stream_length);
		compressed_stream	= g_malloc (compressed_stream_length);
		real_length = gnome_print_encode_deflate (page->stream, compressed_stream,
																							stream_length, compressed_stream_length);
		break;
	case PDF_COMPRESSION_NONE:
	default:
		real_length = page->stream_used; 
	}
	

	ret += gnome_print_pdf_object_start (pc, page->object_number_contents);

	if (compr_type != PDF_COMPRESSION_NONE) {
		pdf->offset += gnome_print_pdf_write_stream (pc,
																								 compressed_stream,
																								 real_length,
																								 compr_type);
		ret += gnome_print_pdf_write  (pc, EOL);
	}	else {
		ret += gnome_print_pdf_write  (pc,"/Length %i" EOL, page->stream_used); 
		ret += gnome_print_pdf_write  (pc,">>" EOL);
		ret += gnome_print_pdf_write  (pc,"stream" EOL);                
		ret += gnome_print_pdf_write  (pc, "%s", page->stream);
	}
	
	ret += gnome_print_pdf_write  (pc, "endstream" EOL);
	ret += gnome_print_pdf_write  (pc, "endobj" EOL);
	ret += gnome_print_pdf_object_end (pc, page->object_number_contents, TRUE);

	if (compr_type != PDF_COMPRESSION_NONE)
		g_free (compressed_stream);

	return ret;
}

static gint
gnome_print_pdf_encoding (GnomePrintContext *pc, GnomePrintPdfFont *font_in)
{
	GnomePrintPdf *pdf;
	const GnomeFontFace *face;
	GnomeFont *font;
	gint nglyphs, nfonts;
	gint i, j;
	gint ret = 0;

	debug (FALSE, "");
	
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	g_return_val_if_fail (font_in != NULL, -1);
	g_return_val_if_fail (GNOME_IS_FONT (font_in->gnome_font), -1);
	font = font_in->gnome_font;
	g_return_val_if_fail (GNOME_IS_FONT (font), -1);
	face = (GnomeFontFace *) gnome_font_get_face (font);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -1);

	gnome_print_pdf_object_start (pc, font_in->object_number_encoding);
	ret += gnome_print_pdf_write (pc,"/Type /Encoding" EOL);
	ret += gnome_print_pdf_write (pc,"/Differences [0" EOL);
	
	nglyphs = gnome_font_face_get_num_glyphs (face);
	nfonts = (nglyphs + 255) >> 8;

	for (i = 0; i < nfonts; i++) {
		gint col = 0;
		for (j = 0; j < 256; j++) {
			gint glyph;
			const gchar *name;

			glyph = 256 * i + j;
			if (glyph >= nglyphs)
				glyph = 0;

			name = gnome_font_face_get_glyph_ps_name (face, glyph);
			gnome_print_pdf_write (pc, "/%s",	name);
			col += strlen (name) + 1;
			if (col > 70) {
				gnome_print_pdf_write (pc, EOL);
				col = 0;
			}
		}

	}
	
	ret += gnome_print_pdf_write (pc,"]" EOL);

	gnome_print_pdf_object_end   (pc, font_in->object_number_encoding, FALSE);

	return ret;
}


/* FIXME : fix the '-' character issue. Chema */
static gint
gnome_print_pdf_font_print_metrics (GnomePrintContext *pc, GnomePrintPdfFont *font_in)
{
	GnomePrintPdf *pdf;
	const GnomeFontFace *face;
	const GnomeFont *font;
	gint nglyphs, nfonts;
	gint i, j;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (GNOME_IS_PRINT_PDF (pdf), -1);
	g_return_val_if_fail (font_in != NULL, -1);
	font = font_in->gnome_font;
	g_return_val_if_fail (GNOME_IS_FONT (font), -1);
	face = (GnomeFontFace *) gnome_font_get_face (font);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -1);

	nglyphs = gnome_font_face_get_num_glyphs (face);
	nfonts = (nglyphs + 255) >> 8;
	if (nfonts > 1) {
		static gboolean warned = FALSE;
		if (!warned || TRUE)
			g_warning ("\nCan't encode fonts with more than 1 page of glyphs for now.\n"
								 "Encoding only the first page of glyphs (256). The font in \n"
								 "question is :%s, which has %i glyphs. \n",
								 gnome_font_face_get_name (face),
								 gnome_font_face_get_num_glyphs (face));
		warned = TRUE;
		nfonts = 1;
	}

	if (nglyphs > 256)
		nglyphs = 256;
	
	ret += gnome_print_pdf_write  (pc,
																 "/FirstChar %i" EOL
																 "/LastChar %i" EOL
																 "/Widths [",
																 1,
																 nglyphs - 1);

	
	for (i = 0; i < nfonts; i++) 
	{
		gint col = 0;
		for (j = 1; j < nglyphs; j++) {
			gint glyph;
			ArtPoint point;

			glyph = 256 * i + j;
			if (glyph >= nglyphs)
				glyph = 0;

			gnome_font_face_get_glyph_stdadvance (face, glyph, &point);

			ret += gnome_print_pdf_write  (pc, "%g ",point.x);
			
			col += 1;
			if (col > 15) {
				gnome_print_pdf_write (pc, EOL);
				col = 0;
			}
		}
	}
	
	ret += gnome_print_pdf_write  (pc, "]" EOL);

	return ret;
}


static gint32
gnome_font_face_get_pdf_flags (const GnomeFontFace *face)
{
	/* See the PDF 1.3 standard, section 7.11.2, page 225 */
	gboolean fixed_width;
	gboolean serif;
	gboolean symbolic;
	gboolean script;
	gboolean italic;
	gboolean all_cap;
	gboolean small_cap;
	gboolean force_bold;
	gboolean adobe_roman_set;
	gint32 flags;

	fixed_width = gnome_font_face_is_fixed_width (face);
	serif = TRUE;
	symbolic = FALSE; /* FIXME */
	script = FALSE;  /* FIXME */
	italic = gnome_font_face_is_italic (face);
	all_cap = FALSE;
	small_cap = FALSE;
	force_bold = FALSE;
	adobe_roman_set = TRUE;

	flags = 0;
	flags =
		fixed_width     |
		(serif      << 1) |
		(symbolic   << 2) |
		(script     << 3) |
		/* 5th bit is reserved */
		(adobe_roman_set << 5) |
		(italic     << 6) |
		/* 8th - 16 are reserved */
		(all_cap    << 16) |
		(small_cap  << 17) |
		(force_bold << 18);
	/* 20-32 reserved */

	return flags;
}

static gint
gnome_font_face_get_stemv (const GnomeFontFace *face)
{
	gint stemv;
	gint stemh;

	gnome_print_pdf_type1_get_stems (face, &stemv, &stemh);

	return stemv;
}


static gint
gnome_print_pdf_font_print_descriptor (GnomePrintContext *pc,
																			 GnomePrintPdfFont *font)
{
	const GnomeFontFace *face;
	guint object_number;
	guint pfb_object_number;
	gint ret = 0;
	gint ascent;
	gint descent;
	gint flags;
	gint stemv;
	gint italic_angle;
	gint capheight;
	gint xheight;
	const ArtDRect *rect;
	gdouble val;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (font != NULL, -1);
	g_return_val_if_fail (font->gnome_font != NULL, -1);
	g_return_val_if_fail (GNOME_IS_FONT (font->gnome_font), -1);
	face = gnome_font_get_face (font->gnome_font);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -1);
	g_return_val_if_fail (font->font_name != NULL, -1);

	object_number     = gnome_print_pdf_object_number (pc);
	pfb_object_number = gnome_print_pdf_object_number (pc);
	font->object_number_descriptor = object_number;
	font->object_number_pfb = pfb_object_number;
	
	ret += gnome_print_pdf_object_start (pc, object_number);

	ascent = (gint) gnome_font_face_get_ascender (face);
	descent = (gint) gnome_font_face_get_descender (face);
	flags = gnome_font_face_get_pdf_flags (face);
	stemv = gnome_font_face_get_stemv (face);

	/* GnomeFontFace arguments */
	gtk_object_get (GTK_OBJECT (face), "ItalicAngle", &val, NULL);
	italic_angle = (gint) val;
	gtk_object_get (GTK_OBJECT (face), "CapHeight", &val, NULL);
	capheight = (gint) val;
	gtk_object_get (GTK_OBJECT (face), "XHeight", &val, NULL);
	xheight = (gint) val;
	gtk_object_get (GTK_OBJECT (face), "FontBBox", &rect, NULL);
	
	ret += gnome_print_pdf_write  (pc,
																 "/Type /FontDescriptor" EOL
																 "/Ascent %i" EOL
																 "/CapHeight %i" EOL
																 "/Descent -%i" EOL
																 "/Flags %i" EOL
																 "/FontBBox [%g %g %g %g]" EOL
																 "/FontName /%s" EOL
																 "/ItalicAngle %i" EOL
																 "/StemV %i" EOL
																 "/XHeight %i" EOL
																 "/FontFile %i 0 R" EOL,
																 ascent,
																 capheight,
																 descent,
																 flags,
																 rect->x0, rect->y0, rect->x1, rect->y1,
																 font->font_name,
																 italic_angle,
																 stemv,
																 xheight,
																 pfb_object_number);

	ret += gnome_print_pdf_object_end   (pc, object_number, FALSE);

	return ret;
}

	
static gint
gnome_print_pdf_fonts (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfFont *font;
	gint ret = 0;
	gint n;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	if (pdf->fonts_number < 1)
		return 0;

	for (n=0; n < pdf->fonts_number; n++)	{
		font = &pdf->fonts [n];

		if (!font->is_basic_14) {
			ret += gnome_print_pdf_font_print_descriptor (pc, font);
			/* We are initialy implementing only type1 embeding */
			ret += gnome_print_pdf_font_type1_embed      (pc, font);
		}

		ret += gnome_print_pdf_encoding (pc, font);
		
		ret += gnome_print_pdf_object_start (pc, font->object_number);
		ret += gnome_print_pdf_write  (pc,
																	 "/Type /Font" EOL
																	 "/Subtype /Type1" EOL /* Hardcode tyep1 for now*/
																	 "/Name /F%i" EOL,
																	 font->font_number);
		if (!font->is_basic_14)
			ret += gnome_print_pdf_font_print_metrics (pc, font);

		ret += gnome_print_pdf_write  (pc,
																	 "/Encoding %i 0 R" EOL
																	 "/BaseFont /%s" EOL,
																	 font->object_number_encoding,
																	 font->font_name);
		if (!font->is_basic_14)
			ret += gnome_print_pdf_write  (pc,
																		 "/FontDescriptor %i 0 R" EOL,
																		 font->object_number_descriptor);
		
		ret += gnome_print_pdf_object_end   (pc, font->object_number, FALSE);
	}
	
	return ret;
}

static gint
gnome_print_pdf_error (gint fatal, const char *format, ...)
{
	va_list arguments;
	gchar *text;

	debug (FALSE, "");

	va_start (arguments, format);
	text = g_strdup_vprintf (format, arguments);
	va_end (arguments);
	
	g_warning ("Offending command [[%s]]", text);

	g_free (text);
	
	return -1;
}


static gint
gnome_print_pdf_images (GnomePrintContext *pc, GnomePrintPdfPage *page)
{
	GnomePrintPdfImage *image;
	GnomePrintPdf *pdf;
	const gchar type_gray[] = "DeviceGray";
	const gchar type_rgb [] = "DeviceRGB";
	const gchar *image_type_text;
	gint ret = 0;
	gint n;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	if (page->images_number < 1)
		return 0;

	for (n=0; n < page->images_number; n++) {
		image = &page->images[n];

		switch (image->image_type){
		case PDF_IMAGE_RGB:
			image_type_text = type_rgb;
			break;
		case PDF_IMAGE_GRAYSCALE:
			image_type_text = type_gray;
			break;
		default:
			gnome_print_pdf_error (FALSE, "image, type undefined");
			image_type_text = type_gray;
			break;
		}

		ret += gnome_print_pdf_object_start (pc, image->object_number);
		ret += gnome_print_pdf_write  (pc,
																	 "/Type /XObject" EOL
																	 "/Subtype /Image" EOL
																	 "/Name /Im%i" EOL
																	 "/Width %i" EOL
																	 "/Height %i" EOL
																	 "/BitsPerComponent 8" EOL
																	 "/ColorSpace /%s" EOL,
																	 image->image_number,
																	 image->width,
																	 image->height,
																	 image_type_text);

		pdf->offset += gnome_print_pdf_write_stream (pc,
																								 image->data,
																								 image->data_length,
																								 image->compr_type);
		
		ret += gnome_print_pdf_write  (pc, EOL);
		
		ret += gnome_print_pdf_write  (pc,
																	 "endstream" EOL
																	 "endobj" EOL);

		
		ret += gnome_print_pdf_object_end   (pc, image->object_number, TRUE);
	}

	return ret;
}

static gint
gnome_print_pdf_get_fonts_object_numbers (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfFont *font;
	gint n;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	if (pdf->fonts_number < 1)
		return 0;

	ret += gnome_print_pdf_write  (pc, "/Font <<" EOL);
	
	for (n=0; n < pdf->fonts_number; n++)	{
		font = &pdf->fonts [n];
		if (font->object_number == 0) {
			font->object_number = gnome_print_pdf_object_number (pc);
			font->object_number_encoding = gnome_print_pdf_object_number (pc);
		}
		
		ret += gnome_print_pdf_write  (pc, "/F%i %i 0 R" EOL,
																	 font->font_number,
																	 font->object_number);
	}

	ret += gnome_print_pdf_write  (pc, ">>" EOL);

	return ret;
}

static gint
gnome_print_pdf_get_images_object_numbers (GnomePrintContext *pc, GnomePrintPdfPage *page)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfImage *image;
	
	gint n;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	g_return_val_if_fail (page != NULL, -1);


	if (page->images_number < 1)
		return 0;

	ret += gnome_print_pdf_write  (pc, "/XObject <<" EOL);
	
	for (n=0; n < page->images_number; n++)	{
		image = &page->images [n];
		ret += gnome_print_pdf_write  (pc, "/Im%i %i 0 R" EOL,
																	 image->image_number,
																	 image->object_number);
	}

	ret += gnome_print_pdf_write  (pc, ">>" EOL);

	return ret;
}

static gint
gnome_print_pdf_page_write_resources (GnomePrintContext *pc, GnomePrintPdfPage *page)
{
	GnomePrintPdf *pdf;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (page != NULL, -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf  != NULL, -1);


	ret += gnome_print_pdf_object_start (pc, page->object_number_resources);
	ret += gnome_print_pdf_write  (pc,"/ProcSet [/PDF ");
	if (pdf->fonts_number > 0)
		ret += gnome_print_pdf_write  (pc,"/Text ");
	if (page->used_grayscale_images)
		ret += gnome_print_pdf_write  (pc,"/ImageB ");
	if (page->used_color_images)
		ret += gnome_print_pdf_write  (pc,"/ImageC ");
	ret += gnome_print_pdf_write  (pc,"]" EOL);

	ret += gnome_print_pdf_get_fonts_object_numbers  (pc);
	ret += gnome_print_pdf_get_images_object_numbers (pc, page);
	
	ret += gnome_print_pdf_write  (pc,"/ExtGState <<" EOL);
	if (pdf->object_number_gstate == 0)
		pdf->object_number_gstate = gnome_print_pdf_object_number (pc);
	ret += gnome_print_pdf_write  (pc,"/GS1 %i 0 R" EOL, pdf->object_number_gstate);
	ret += gnome_print_pdf_write  (pc,">>" EOL);
	ret += gnome_print_pdf_object_end   (pc, page->object_number_resources, FALSE);
	
	return ret;
}

static gint
gnome_print_pdf_page_end (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfPage *page;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf  != NULL, -1);
	g_return_val_if_fail (pdf->current_page != NULL, -1);

	gnome_print_pdf_graphic_mode_set (pdf, PDF_GRAPHIC_MODE_GRAPHICS);
		
	page = pdf->current_page;
	/* Ask for object numbers for this page */
	page->object_number_page      = gnome_print_pdf_object_number (pc);
	page->object_number_contents  = gnome_print_pdf_object_number (pc);
	page->object_number_resources = gnome_print_pdf_object_number (pc);

	ret += gnome_print_pdf_images (pc, page);
	ret += gnome_print_pdf_page_write_contents  (pc, page);
	/* After writing the contents, free the memory allocated
		 by the page's stream, we dont need it anymore and it is
     only taking up space */
	g_free (page->stream);
	page->stream = NULL;
	
	ret += gnome_print_pdf_page_write_resources (pc, page);

	if (page->page_number == 1) {
		pdf->object_number_pages = gnome_print_pdf_object_number (pc);
	}

	pdf->pages = g_list_prepend (pdf->pages, page);

	pdf->current_page = NULL;

	pdf->graphic_state->written = FALSE;

	return ret;

}

static void
gnome_print_pdf_graphic_state_free (GnomePrintPdfGraphicState *gs)
{
	debug (FALSE, "");

	gp_path_unref (gs->current_path);
	g_free (gs);
}
	
static gboolean
gnome_print_dash_init (GnomePrintDash *dash)
{
	debug (FALSE, "");

	dash->number_values = 0;
	dash->phase = 0;
	dash->values = 0;
	
	return TRUE;
}

static GnomePrintPdfGraphicState *
gnome_print_pdf_graphic_state_new (gint undefined)
{
	GnomePrintPdfGraphicState * state;
	gint def;

	debug (FALSE, "");

	if (undefined)
		def = 1;
	else
		def = 0;
	
	state = g_new (GnomePrintPdfGraphicState, 1);

	state->dirty   = TRUE;
	state->written = FALSE;

	state->current_path = gp_path_new();

	/* CTM */
	art_affine_identity (state->ctm);
		
	/* Color stuff */
	if (undefined)
		state->color_stroke_mode = PDF_COLOR_MODE_UNDEFINED;
	else
		state->color_stroke_mode = PDF_COLOR_MODE_DEVICEGRAY;
	state->color_stroke [0] = def;
	state->color_stroke [1] = def;
	state->color_stroke [2] = def;
	state->color_stroke [3] = def;
	if (undefined)
		state->color_fill_mode   = PDF_COLOR_MODE_UNDEFINED;
	else
		state->color_fill_mode   = PDF_COLOR_MODE_DEVICEGRAY;
	state->color_fill   [0] = def;
	state->color_fill   [1] = def;
	state->color_fill   [2] = def;
	state->color_fill   [3] = def;

	/* Line stuff */
	state->linejoin   = 0;
	state->linecap    = 0;
	state->miterlimit = 10;
	state->linewidth  = 1;
	gnome_print_dash_init (&state->dash);

	/* Font stuff */
  state->font_size = def;
	state->font_character_spacing = 0;
	state->font_word_spacing = 0;

  state->pdf_font_number = GNOME_PRINT_PDF_FONT_UNDEFINED;
	state->text_flag = FALSE;

	return state;
}

static gint
gnome_print_pdf_showpage (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	g_return_val_if_fail (pdf->current_page != NULL, -1);

	if (pdf->current_page->showpaged) {
		gnome_print_pdf_error (TRUE, "showpage, showpaged used twice for the same page");
		return 0;
	}

	if (pdf->gsave_level_number > 0)
		gnome_print_pdf_error (TRUE, "showpage, with graphic state stack NOT empty");

	pdf->current_page->showpaged = TRUE;

	gnome_print_pdf_page_end   (pc);

	/* We need to clean the Graphic State */
	gnome_print_pdf_graphic_state_free (pdf->graphic_state);
	gnome_print_pdf_graphic_state_free (pdf->graphic_state_set);
	pdf->graphic_state     = gnome_print_pdf_graphic_state_new (FALSE);
	pdf->graphic_state_set = gnome_print_pdf_graphic_state_new (TRUE);
	
	gnome_print_pdf_page_start (pc);
		
	return 0;
}

static gint
gnome_print_pdf_beginpage (GnomePrintContext *pc, const char *name_of_this_page)
{
	GnomePrintPdf *pdf;

	debug (FALSE, "");
	
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (name_of_this_page != NULL, -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	g_return_val_if_fail (pdf->current_page != NULL, -1);
	g_return_val_if_fail (pdf->current_page->page_name == NULL, -1);
	
	pdf->current_page->page_name = g_strdup (name_of_this_page);

	return 0;
}

static gint
gnome_print_pdf_xref (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfObject *nth_object;
	GList *list;
	gint   ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	g_return_val_if_fail (pdf->object_number_current == 0, -1);

	pdf->xref_location = pdf->offset;
	
	ret += gnome_print_pdf_write (pc,
																"xref" EOL
																"0 %i" EOL
																"%010i %05i f" EOL,
																pdf->object_number_last_used+1,
																0,
																65535);
	
	pdf->objects = g_list_reverse (pdf->objects);
	list = pdf->objects;

	for (; list; list = list->next)	{
		nth_object = (GnomePrintPdfObject *) list->data;
		ret +=   gnome_print_pdf_write (pc,
																		"%010i %05i n" EOL,
																		nth_object->offset,
																		0);
	}
	
	return ret;
}

static gchar*
gnome_print_pdf_id_new (GnomePrintPdf *pdf)
{
	gchar *a;
	
	debug (FALSE, "");

	/* FIXME: Improve ID generation */
	a = g_strdup_printf ("%.12d%.2d%.12d%.6ld",
											 (gint) time(NULL),
											 95,
											 pdf->offset,
											 (long) getpid());

	if (strlen (a) != 32) {
		g_warning ("Error while creating pdf_id. [%s]\n", a);
		if (a != NULL)
			g_free (a);
		a = g_strdup ("00ff00ff00ff00ff00ff00ff00ff00ff");

	}

	return a;
}

static gint
gnome_print_pdf_trailer (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	gint ret = 0;
	gchar *id;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	id = gnome_print_pdf_id_new (pdf);

	g_return_val_if_fail (id != NULL, -1);
	
	ret += gnome_print_pdf_write (pc,
																"trailer\r\n"
																"<<\r\n"
																"/Size %i\r\n"
																"/Root %i 0 R\r\n"
																"/Info %i 0 R\r\n"
																"/ID [<%s><%s>]\r\n"
																">>\r\n",
																pdf->object_number_last_used+1,
																pdf->object_number_catalog,
																pdf->object_number_info,
																id,
																id);

	g_free (id);
	
	return ret;
}


static gint
gnome_print_pdf_page (GnomePrintContext *pc, GnomePrintPdfPage *page)
{
	GnomePrintPdf *pdf;
	gint ret = 0;
	
	debug (FALSE, "");

	g_return_val_if_fail (page != NULL, -1);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf  != NULL, -1);

	ret += gnome_print_pdf_object_start (pc, page->object_number_page);
	ret += gnome_print_pdf_write (pc,
																"/Type /Page" EOL
																"/Parent %i 0 R" EOL
																"/Resources %i 0 R" EOL
																"/Contents %i 0 R" EOL,
																pdf->object_number_pages,
																page->object_number_resources,
																page->object_number_contents);
	ret += gnome_print_pdf_object_end   (pc, page->object_number_page, FALSE);

	return ret;
	
}


static gint
gnome_print_pdf_pages (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfPage *nth_page;
	GList *list;
	gint ret = 0;
	gint col;
	gint paper_width, paper_height;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf  != NULL, -1);

	pdf->pages = g_list_reverse (pdf->pages);
	list = pdf->pages;
	for (; list; list = list->next)	{
		nth_page = (GnomePrintPdfPage *) list->data;
		ret += gnome_print_pdf_page (pc, nth_page);
	}

	ret += gnome_print_pdf_object_start (pc, pdf->object_number_pages);
	ret += gnome_print_pdf_write (pc,
																"/Type /Pages" EOL
																"/Kids [");

	col = 0;
	list = pdf->pages;
	for (; list; list = list->next)	{
		nth_page = (GnomePrintPdfPage *) list->data;
		ret += gnome_print_pdf_write (pc,"%i 0 R ", nth_page->object_number_page);
		col++;
		if (col == 10) {
			ret += gnome_print_pdf_write (pc, EOL);
			col = 0;
		}
	}

	paper_width  = pdf->paper ? gnome_paper_pswidth (pdf->paper) : 21 * 72 / 2.54;
	paper_height = pdf->paper ? gnome_paper_psheight (pdf->paper) : 29.7 * 72 / 2.54;
	ret += gnome_print_pdf_write (pc,
																"]" EOL
																"/Count %i" EOL
																"/MediaBox [0 0 %i %i]" EOL,
																g_list_length (pdf->pages),
																paper_width,
																paper_height);

	ret += gnome_print_pdf_object_end (pc, pdf->object_number_pages, FALSE);
	
	return ret;
}

static guint
gnome_print_pdf_catalog (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	
	pdf->object_number_catalog = gnome_print_pdf_object_number (pc);

	ret += gnome_print_pdf_object_start (pc, pdf->object_number_catalog);
	ret += gnome_print_pdf_write (pc,
																"/Type /Catalog" EOL
																"/Pages %i 0 R" EOL,
																pdf->object_number_pages);
	ret += gnome_print_pdf_object_end (pc, pdf->object_number_catalog, FALSE);
	
	return ret;
}

static gchar*
gnome_print_pdf_get_date (void)
{
	time_t clock;
	struct tm *now;
	gchar *date;

#ifdef ADD_TIMEZONE_STAMP
  extern char * tzname[];
	/* TODO : Add :
		 "[+-]"
		 "HH'" Offset from gmt in hours
		 "OO'" Offset from gmt in minutes
	   we need to use tz_time. but I don't
	   know how protable this is. Chema */
	gprint ("Timezone %s\n", tzname[0]);
	gprint ("Timezone *%s*%s*%li*\n", tzname[1], timezone);
#endif	

	debug (FALSE, "");

	clock = time (NULL);
	now = localtime (&clock);

	date = g_strdup_printf ("D:%04d%02d%02d%02d%02d%02d",
													now->tm_year + 1900,
													now->tm_mon + 1,
													now->tm_mday,
													now->tm_hour,
													now->tm_min,
													now->tm_sec);

	return date;
}
	

static guint
gnome_print_pdf_info (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	gchar *date = NULL;
	gchar *producer = NULL;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	pdf->object_number_info = gnome_print_pdf_object_number (pc);

	date     = gnome_print_pdf_get_date ();
	producer = g_strdup_printf ("Gnome Print Ver: %s", VERSION);

	ret += gnome_print_pdf_object_start (pc, pdf->object_number_info);
	ret += gnome_print_pdf_write  (pc,
																 "/CreationDate (%s)" EOL
																 "/Producer (%s)" EOL,
																 date,
																 producer);
	ret += gnome_print_pdf_object_end   (pc, pdf->object_number_info, FALSE);

	g_free (producer);
	g_free (date);
	
	return ret;
}
	
static gint
gnome_print_pdf_halftone_default (GnomePrintContext *pc)
{
	guint object_number_halftone;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	
	object_number_halftone = gnome_print_pdf_object_number (pc);

	ret += gnome_print_pdf_object_start (pc, object_number_halftone);
	ret += gnome_print_pdf_write  (pc,
																 "/Type /Halftone" EOL
																 "/HalftoneType 1" EOL
																 "/HalftoneName (Default)" EOL
																 "/Frequency 60" EOL
																 "/Angle 45" EOL
																 "/SpotFunction /Round" EOL);
	ret += gnome_print_pdf_object_end   (pc, object_number_halftone, FALSE);

	return ret;
}

static gint
gnome_print_pdf_default_GS (GnomePrintContext *pc)
{
	gint ret = 0;
	GnomePrintPdf *pdf;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	ret += gnome_print_pdf_object_start (pc, pdf->object_number_gstate);
	ret += gnome_print_pdf_write (pc,
																"/Type /ExtGState" EOL
																"/SA false" EOL
																"/OP false" EOL
																"/HT /Default" EOL);
	ret += gnome_print_pdf_object_end   (pc, pdf->object_number_gstate, FALSE);
	
	return ret;
}


static gint
gnome_print_pdf_close (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	ret += gnome_print_pdf_halftone_default (pc);
	ret += gnome_print_pdf_default_GS       (pc);
	ret += gnome_print_pdf_fonts            (pc);
	ret += gnome_print_pdf_pages            (pc);
	ret += gnome_print_pdf_catalog          (pc);
	ret += gnome_print_pdf_info             (pc);
	ret += gnome_print_pdf_xref             (pc);
	ret += gnome_print_pdf_trailer          (pc);
	
	gnome_print_pdf_write (pc,
												 "startxref" EOL
												 "%i" EOL
												 "%c%cEOF" EOL,
												 pdf->xref_location,
												 '%',
												 '%');

	return gnome_print_context_close_file (pc);
}

static gint
gnome_print_pdf_write_content (GnomePrintPdf *pdf, const char *format, ...)
{
	GnomePrintContext *pc;
	GnomePrintPdfPage *page;
	va_list arguments;
	gchar *text;
	gint text_length;
	gchar *oldlocale;	

	debug (FALSE, "");
	
	g_return_val_if_fail (pdf->current_page != NULL, -1);
	g_return_val_if_fail (pdf != NULL, -1);
	pc = GNOME_PRINT_CONTEXT (pdf);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);

	oldlocale = g_strdup (setlocale (LC_NUMERIC, NULL));
	setlocale (LC_NUMERIC, "C");

	va_start (arguments, format);
	text = g_strdup_vprintf (format, arguments);
	va_end (arguments);

	setlocale (LC_NUMERIC, oldlocale);
	g_free (oldlocale);

	page = pdf->current_page;

	text_length = strlen (text);

	if (page->stream_used + text_length + 2 > page->stream_allocated ) {
		page->stream_allocated += GNOME_PRINT_PDF_BUFFER_SIZE;
		page->stream = g_realloc (page->stream, page->stream_allocated);
	}

	memcpy (page->stream + page->stream_used,
					text,
					text_length * sizeof (gchar));

	page->stream_used += text_length;
	page->stream [page->stream_used] = 0;

	g_free (text);
	
	return 0;
}

static gint
gnome_print_pdf_moveto (GnomePrintContext *pc, double x, double y)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;
	ArtPoint point;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	gs = gnome_print_pdf_graphic_state_current (pdf, FALSE);

	point.x = x;
	point.y = y;
	art_affine_point (&point, &point, gs->ctm);	
	gp_path_moveto (gs->current_path, point.x, point.y);

	return 0;
	
}

static gint
gnome_print_pdf_setopacity (GnomePrintContext *pc, double opacity)
{
  static gboolean warned = FALSE;

	debug (FALSE, "");

	if (!warned) {
    g_warning ("Unimplemented setopacity");
    warned = TRUE;
  }
  return 0;
}

static gint
gnome_print_pdf_setrgbcolor (GnomePrintContext *pc,
														 double r, double g, double b)
{
  GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	
	gs = gnome_print_pdf_graphic_state_current (pdf, TRUE);

	gs->color_fill_mode   = PDF_COLOR_MODE_DEVICERGB;
	gs->color_fill [0] = r;
	gs->color_fill [1] = g;
	gs->color_fill [2] = b;
	
	gs->color_stroke_mode = PDF_COLOR_MODE_DEVICERGB;
	gs->color_stroke [0] = r;
	gs->color_stroke [1] = g;
	gs->color_stroke [2] = b;

	return 0;
}

static gint
gnome_print_pdf_graphic_mode_set (GnomePrintPdf *pdf, gint mode)
{
	gint ret = 0;

	debug (FALSE, "");

	if (pdf->graphics_mode == mode)
		return 0;

	switch (mode) {
	case PDF_GRAPHIC_MODE_GRAPHICS:
		if (pdf->graphics_mode == PDF_GRAPHIC_MODE_TEXT)
			ret += gnome_print_pdf_write_content (pdf, "ET" EOL);
		break;
	case PDF_GRAPHIC_MODE_TEXT:
		ret += gnome_print_pdf_write_content (pdf, "BT" EOL);
		break;
	case PDF_GRAPHIC_MODE_UNDEFINED:
		gnome_print_pdf_error (FALSE, "GRAPHIC_MODE undefined\n");
		ret = -1;
		break;
	default:
		gnome_print_pdf_error (FALSE, "mem-problems\n");
		ret = -1;
		g_assert_not_reached ();
	}

	pdf->graphics_mode = mode;

	return ret;
}			

static gint
gnome_print_pdf_setlinewidth (GnomePrintContext *pc, double width)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	if (width < 0)
		return gnome_print_pdf_error (FALSE, "setlinewidth, invalid parameter range %g", width);
	
	gs = gnome_print_pdf_graphic_state_current (pdf, TRUE);

	/*FIXME ! This is broken !!!!!!!!!!! */
	gs->linewidth = (fabs (width * gs->ctm[0]) +
									 fabs (width * gs->ctm[1]) +
									 fabs (width * gs->ctm[2]) +
									 fabs (width * gs->ctm[3])) / 2;	
	return 0;
}

static gint
gnome_print_pdf_setmiterlimit (GnomePrintContext *pc, double miterlimit)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	if (miterlimit < 1)
		return gnome_print_pdf_error (FALSE, "setmiterlimit, invalid parameter range %g", miterlimit);
	
	gs = gnome_print_pdf_graphic_state_current (pdf, TRUE);

	gs->miterlimit = miterlimit;

	return 0;
}

static gint
gnome_print_pdf_setlinecap (GnomePrintContext *pc, gint linecap)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	if (linecap < 0 || linecap > 2)
		return gnome_print_pdf_error (FALSE, "setlinecap, invalid parameter range %i", linecap);

	gs = gnome_print_pdf_graphic_state_current (pdf, TRUE);

	gs->linecap = linecap;
	return 0;
}

static gint
gnome_print_pdf_setlinejoin (GnomePrintContext *pc, gint linejoin)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	if (linejoin < 0 || linejoin > 2)
		return gnome_print_pdf_error (FALSE, "setlinejoin, invalid parameter range %i", linejoin);

	gs = gnome_print_pdf_graphic_state_current (pdf, TRUE);

	gs->linejoin  = linejoin;

	return 0;
}

static gboolean
gnome_print_dash_compare (GnomePrintDash *dash1, GnomePrintDash *dash2)
{
	gint n;
	
	if (dash1->number_values != dash2->number_values)
		return FALSE;
	
	if (dash1->phase != dash2->phase)
		return FALSE;

	for (n=0; n < dash1->number_values; n++)
		if (dash1->values[n] != dash2->values[n])
			return FALSE;

	return TRUE;
}

static void
gnome_print_dash_copy (GnomePrintDash *dash_from, GnomePrintDash *dash_to)
{
	dash_to->number_values = dash_from->number_values;
	dash_to->phase         = dash_from->phase;

	if (dash_to->values == NULL)
		dash_to->values = g_new (gdouble, dash_from->number_values);
	
if(FALSE)	memcpy (dash_to->values,
					dash_from->values,
					dash_to->number_values * sizeof (gdouble));
}

static gint
gnome_print_pdf_setdash (GnomePrintContext *pc, gint number_values, const double *values, double offset)
{
	GnomePrintPdf *pdf;
	GnomePrintDash *dash;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	dash = &pdf->graphic_state->dash;

	g_free (dash->values);

	dash->phase = offset;
	dash->values = g_new (gdouble, number_values);
	dash->number_values = number_values;

	memcpy (dash->values, values,
					number_values * sizeof (gdouble));

	return 0;
}

static gint
gnome_print_pdf_graphic_state_set_color (GnomePrintPdf *pdf, gint color_group)
{
	GnomePrintPdfGraphicState *gs;
	GnomePrintPdfGraphicState *gs_set;

	debug (FALSE, "");

	g_return_val_if_fail (pdf != NULL, -1);

	gs     = pdf->graphic_state;
	gs_set = pdf->graphic_state_set;

	if ((color_group == PDF_COLOR_GROUP_STROKE ||
			 color_group == PDF_COLOR_GROUP_BOTH) &&
			(gs->color_stroke_mode != gs_set->color_stroke_mode ||
			gs->color_stroke [0] != gs_set->color_stroke [0]||
			gs->color_stroke [1] != gs_set->color_stroke [1]||
			gs->color_stroke [2] != gs_set->color_stroke [2]||
			gs->color_stroke [3] != gs_set->color_stroke [3])) {
		switch (gs->color_stroke_mode){
		case PDF_COLOR_MODE_DEVICEGRAY:
			gnome_print_pdf_write_content (pdf, "%.3g G" EOL,
																		 gs->color_stroke[0]);
			break;
		case PDF_COLOR_MODE_DEVICERGB:
			gnome_print_pdf_write_content (pdf, "%.3g %.3g %.3g RG" EOL,
																		 gs->color_stroke[0],
																		 gs->color_stroke[1],
																		 gs->color_stroke[2]);
			break;
		case PDF_COLOR_MODE_DEVICECMYK:
			gnome_print_pdf_write_content (pdf, "%.3g %.3g %.3g %.3g K" EOL,
																		 gs->color_stroke[0],
																		 gs->color_stroke[1],
																		 gs->color_stroke[2],
																		 gs->color_stroke[3]);
			break;
		}
		gs_set->color_stroke_mode = gs->color_stroke_mode;
		gs_set->color_stroke[0]   = gs->color_stroke[0];
		gs_set->color_stroke[1]   = gs->color_stroke[1];
		gs_set->color_stroke[2]   = gs->color_stroke[2];
		gs_set->color_stroke[3]   = gs->color_stroke[3];
	}

	if ((color_group == PDF_COLOR_GROUP_FILL ||
			 color_group == PDF_COLOR_GROUP_BOTH) &&
			(gs->color_fill_mode != gs_set->color_fill_mode ||
			gs->color_fill [0] != gs_set->color_fill [0]||
			gs->color_fill [1] != gs_set->color_fill [1]||
			gs->color_fill [2] != gs_set->color_fill [2]||
			gs->color_fill [3] != gs_set->color_fill [3])) {
		switch (gs->color_fill_mode){
		case PDF_COLOR_MODE_DEVICEGRAY:
			gnome_print_pdf_write_content (pdf, "%.3g g" EOL,
																		 gs->color_fill[0]);
			break;
		case PDF_COLOR_MODE_DEVICERGB:
			gnome_print_pdf_write_content (pdf, "%.3g %.3g %.3g rg" EOL,
																		 gs->color_fill[0],
																		 gs->color_fill[1],
																		 gs->color_fill[2]);
			break;
		case PDF_COLOR_MODE_DEVICECMYK:
			gnome_print_pdf_write_content (pdf, "%.3g %.3g %.3g %.3g K" EOL,
																		 gs->color_fill[0],
																		 gs->color_fill[1],
																		 gs->color_fill[2],
																		 gs->color_fill[3]);
			break;
		}
		gs_set->color_fill_mode  = gs->color_fill_mode;
		gs_set->color_fill [0]  = gs->color_fill [0];
		gs_set->color_fill [1]  = gs->color_fill [1];
		gs_set->color_fill [2]  = gs->color_fill [2];
		gs_set->color_fill [3]  = gs->color_fill [3];
	}

	return 0;
}

static gint
gnome_print_pdf_write_gs (GnomePrintPdf *pdf)
{
	static gint printed = FALSE;
	gint ret = 0;

	debug (FALSE, "");

	if (printed)
		return 0;

	printed = TRUE;

	ret += gnome_print_pdf_write_content (pdf,"/GS1 gs" EOL);

	return ret;
}

static GnomePrintPdfGraphicState *
gnome_print_pdf_graphic_state_text_set (GnomePrintPdf *pdf)
{
	GnomePrintPdfGraphicState *gs;
	GnomePrintPdfGraphicState *gs_set;

	debug (FALSE, "");

	g_return_val_if_fail (pdf != NULL, NULL);

	gs     = pdf->graphic_state;
	gs_set = pdf->graphic_state_set;

#ifdef FIX_ME_FIX_ME_FIX_ME
	if (!gs->dirty)
		return 0;
#endif

	gnome_print_pdf_graphic_state_set_color (pdf, PDF_COLOR_GROUP_FILL);

	gnome_print_pdf_write_gs (pdf);

	/* Linecap */
	if (gs->font_character_spacing != gs_set->font_character_spacing) {
		gnome_print_pdf_write_content (pdf, "%g Tc" EOL, gs->font_character_spacing);
		gs_set->font_character_spacing = gs->font_character_spacing;
	}
	
	return gs;
}


static gint
gnome_print_pdf_graphic_state_set_font (GnomePrintPdf *pdf)
{
	GnomePrintPdfGraphicState *gs;
	GnomePrintPdfGraphicState *gs_set;
	gint ret = 0;

	debug (FALSE, "");
	
	g_return_val_if_fail (GNOME_IS_PRINT_PDF(pdf), -1);

	gs     = pdf->graphic_state;
	gs_set = pdf->graphic_state_set;

	if (gs->pdf_font_number != gs_set->pdf_font_number) {
		ret += gnome_print_pdf_write_content (pdf,
																					"/F%i 1 Tf" EOL,
																					pdf->fonts[gs->pdf_font_number].font_number);
		gs_set->pdf_font_number = gs->pdf_font_number;
	}
	
	return ret;
}


static GnomePrintPdfGraphicState *
gnome_print_pdf_graphic_state_set (GnomePrintPdf *pdf)
{
	GnomePrintPdfGraphicState *gs;
	GnomePrintPdfGraphicState *gs_set;
	gint changed = FALSE;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_PDF(pdf), NULL);

	gs     = pdf->graphic_state;
	gs_set = pdf->graphic_state_set;

	if (!gs->dirty)
		return 0;

	gnome_print_pdf_graphic_state_set_color (pdf, PDF_COLOR_GROUP_BOTH);
	
	/* Linecap */
	if (gs->linecap != gs_set->linecap || !gs->written) {
		gnome_print_pdf_write_content (pdf, "%i J ", gs->linecap);
		gs_set->linecap    = gs->linecap;
		changed = TRUE;
	}

	/* Line join */
	if (gs->linejoin != gs_set->linejoin || !gs->written)	{
		gnome_print_pdf_write_content (pdf, "%i j ", gs->linejoin);
		gs_set->linejoin = gs->linejoin;
		changed = TRUE;
	}

	/* Line width */
	if (gs->linewidth != gs_set->linewidth || !gs->written) {
		gnome_print_pdf_write_content (pdf, "%g w ", gs->linewidth);
		gs_set->linewidth  = gs->linewidth;
		changed = TRUE;
	}
	
	/* Miterlimit */
	if (gs->miterlimit != gs_set->miterlimit || !gs->written)	{
		gnome_print_pdf_write_content (pdf, "%g M ", gs->miterlimit);
		gs_set->miterlimit = gs->miterlimit;
		changed = TRUE;
	}

	/* Dash */
	if (!gnome_print_dash_compare (&gs->dash, &gs_set->dash) || !gs->written)	{
		gint n;
		gnome_print_pdf_write_content (pdf, "[");
		for (n = 0; n < gs->dash.number_values; n++)
			gnome_print_pdf_write_content (pdf, " %g", gs->dash.values[n]);
		gnome_print_pdf_write_content (pdf, "]%g d", gs->dash.phase);
		gnome_print_dash_copy (&gs->dash, &gs_set->dash);
		changed = TRUE;
	}

	if (changed) 
		gnome_print_pdf_write_content (pdf, EOL);

	if (!gs->written)
		gnome_print_pdf_write_gs (pdf);
	
	if (!gs->written)
		gnome_print_pdf_write_content (pdf, "1 i " EOL);
	
	gs->written = TRUE;

	return gs;
}

static gint
gnome_print_pdf_path_print (GnomePrintPdf *pdf, GPPath *path_incoming)
{
	GnomePrintPdfGraphicState *gs;
	ArtBpath *path;;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_PDF(pdf), -1);
	
	gs = gnome_print_pdf_graphic_state_current (pdf, FALSE);

	path = gp_path_bpath (path_incoming);
	
	for (; path->code != ART_END; path++)
		switch (path->code) {
		case ART_MOVETO_OPEN:
			gnome_print_pdf_write_content (pdf, "%g %g m" EOL, path->x3, path->y3);
			break;
		case ART_MOVETO:
			gnome_print_pdf_write_content (pdf, "%g %g m" EOL, path->x3, path->y3);
			break;
		case ART_LINETO:
			gnome_print_pdf_write_content (pdf, "%g %g l" EOL, path->x3, path->y3);
			break;
		case ART_CURVETO:
			gnome_print_pdf_write_content (pdf, "%g %g %g %g %g %g c" EOL,
																		 path->x1, path->y1,
																		 path->x2, path->y2,
																		 path->x3, path->y3);
			break;
		default:
			gnome_print_pdf_error (TRUE, "the path contains an unknown type point");
			return -1;
		}
	
	gp_path_reset (path_incoming);
	
	return 0;
}

#if 0
static gint
gnome_print_pdf_dump_gmode (GnomePrintPdf *pdf)
{
	gint ret = 0;

	debug (FALSE, "");

	switch (pdf->graphics_mode){
	case PDF_GRAPHIC_MODE_GRAPHICS:
		gprint ("Graphics\n");
		ret += gnome_print_pdf_write_content (pdf,"Graphics" EOL);
		break;
	case PDF_GRAPHIC_MODE_TEXT:
		gprint ("Text\n");
		ret += gnome_print_pdf_write_content (pdf,"Text" EOL);
		break;
	case PDF_GRAPHIC_MODE_UNDEFINED:
		gprint ("Undefined\n");
		ret += gnome_print_pdf_write_content (pdf,"Undefined" EOL);
		break;
	default:
		gprint ("Other\n");
		ret += gnome_print_pdf_write_content (pdf,"Other" EOL);
		break;
	}
	return ret;
}
#endif

#if 0
static gint
gnome_print_pdf_ctm_is_identity (double matrix [6])
{
	double identity [6];

	art_affine_identity (identity);

	if ( (identity [0] != matrix [0]) ||
			 (identity [1] != matrix [1]) ||
			 (identity [2] != matrix [2]) ||
			 (identity [3] != matrix [3]) ||
			 (identity [4] != matrix [4]) ||
			 (identity [5] != matrix [5]))
		return FALSE;
	
	return TRUE;
}
#endif

static GnomePrintPdfGraphicState *
gnome_print_pdf_graphic_state_duplicate (GnomePrintPdfGraphicState *gs_in)
{
	GnomePrintPdfGraphicState *gs_out;

	g_return_val_if_fail (gs_in != NULL, NULL);
	
	debug (FALSE, "");
	
	gs_out = g_new (GnomePrintPdfGraphicState, 1);
	
	memcpy (gs_out,
					gs_in,
					sizeof (GnomePrintPdfGraphicState));

	gs_out->current_path = gp_path_duplicate (gs_in->current_path);

	return gs_out;
}

static gint
gnome_print_pdf_gsave (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs_push_me;
	GnomePrintPdfGraphicState *gs_push_me_set;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	/* This is messy, but I don't want to play arround with it before
		 Gnome 1.4 is out. I don't think there is anything wrong. But since
		 we are setting the mode to graphics before gsave we are always going
	   to do gsaves and grestores in GRAPHICS mode, for that reason we don't
	   need to save the graphics_mode, we know it to be MODE_GRAPHICS. Just clean
	   it after 1.4 Chema. */
	gnome_print_pdf_graphic_mode_set (pdf, PDF_GRAPHIC_MODE_GRAPHICS);
	
	gs_push_me     = gnome_print_pdf_graphic_state_duplicate (pdf->graphic_state);
	gs_push_me_set = gnome_print_pdf_graphic_state_duplicate (pdf->graphic_state_set);

	pdf->gsave_stack [pdf->gsave_level_number].graphics_mode     = pdf->graphics_mode;
	pdf->gsave_stack [pdf->gsave_level_number].graphic_state     = gs_push_me;
	pdf->gsave_stack [pdf->gsave_level_number].graphic_state_set = gs_push_me_set;

	pdf->gsave_level_number++;

	if (pdf->gsave_level_number == pdf->gsave_level_max) {
		pdf->gsave_stack = g_realloc (pdf->gsave_stack, sizeof (GnomePrintPdfGsave) *
																	(pdf->gsave_level_max += GNOME_PRINT_NUMBER_OF_ELEMENTS_GROW));
	}
	
	ret += gnome_print_pdf_write_content (pdf, "q" EOL);

	return ret;
}

static gint
gnome_print_pdf_grestore (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	gint ret = 0;
	gint mode;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	/* * * * * * * * * * */
#if 0	
	mode = pdf->gsave_stack [pdf->gsave_level_number].graphics_mode;
	gnome_print_pdf_graphic_mode_set (pdf, PDF_GRAPHIC_MODE_GRAPHICS);
#endif	
	/* * * * * * * * * * */
	
	pdf->gsave_level_number--;

	if (pdf->gsave_level_number < 0) {
		gnome_print_pdf_error (TRUE, "grestore, graphic state stack empty");
		return 0;
	}

	gnome_print_pdf_graphic_state_free (pdf->graphic_state_set);
	gnome_print_pdf_graphic_state_free (pdf->graphic_state);
	pdf->graphic_state_set = pdf->gsave_stack [pdf->gsave_level_number].graphic_state_set;
	pdf->graphic_state     = pdf->gsave_stack [pdf->gsave_level_number].graphic_state;

	/* * * * * * * * * * */
	mode = pdf->gsave_stack [pdf->gsave_level_number].graphics_mode;
	gnome_print_pdf_graphic_mode_set (pdf, mode);
	/* * * * * * * * * * */

	ret += gnome_print_pdf_write_content (pdf, "Q" EOL);

	return ret;
}


static gdouble
gp_zero_clean (gdouble cleanme)
{
	if (abs(cleanme) > 0.00000001) {
		return cleanme;
	}
	return 0.0;
}


static gboolean
gnome_print_pdf_add_glyph_to_font (GnomePrintPdfFont *pdf_font, const gchar *glyph_name)
{
	gint n;
	
	g_return_val_if_fail (pdf_font != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_FONT (pdf_font->gnome_font), FALSE);
	g_return_val_if_fail (glyph_name != NULL, FALSE);

	for (n = 0; n < pdf_font->glyphs_num; n++) {
		if (!strcmp (glyph_name, pdf_font->glyphs[n].name))
			break;
	}

	if (n == pdf_font->glyphs_num) {
		if (pdf_font->glyphs_num == pdf_font->glyphs_max)
			pdf_font->glyphs = g_realloc (pdf_font->glyphs, sizeof (T1Glyph) *
																		(pdf_font->glyphs_max +=
																		 GNOME_PRINT_NUMBER_OF_ELEMENTS_GROW));
		pdf_font->glyphs[pdf_font->glyphs_num++].name = g_strdup (glyph_name);
	}
	
	return TRUE;
}
	

/*
#define METHOD_ONE
*/

static gint
gnome_print_pdf_show_sized (GnomePrintContext *pc, const char *text, int bytes)
{
	GnomePrintPdfGraphicState *gs;
	GnomePrintPdfFont *pdf_font;
	GnomePrintPdf *pdf;
	ArtPoint point;
	gint ret = 0;
	double val_1, val_2, val_3, val_4;
	const char *p;
	const GnomeFontFace * face;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (GNOME_IS_PRINT_PDF (pc), -1);
	g_return_val_if_fail (pdf->fonts != NULL, -1);

	gs = pdf->graphic_state;
	
	if (!gp_path_has_currentpoint (gs->current_path)) {
		gnome_print_pdf_error (FALSE, "show, currentpoint not defined.");
		return -1;
	}

	if (gs->pdf_font_number == GNOME_PRINT_PDF_FONT_UNDEFINED ||
			gs->font_size == 0) {
		gnome_print_pdf_error (FALSE, "show, fontname or fontsize not defined.");
		return -1;
	}

	val_1 = gp_zero_clean (gs->font_size * gs->ctm[0]);
	val_2 = gp_zero_clean (gs->font_size * gs->ctm[1]);
	val_3 = gp_zero_clean (gs->font_size * gs->ctm[2]);
	val_4 = gp_zero_clean (gs->font_size * gs->ctm[3]);

	gp_path_currentpoint (gs->current_path, &point);

	ret += gnome_print_pdf_graphic_mode_set (pdf, PDF_GRAPHIC_MODE_TEXT);

	ret += gnome_print_pdf_graphic_state_set_font (pdf);

	pdf_font = &pdf->fonts[gs->pdf_font_number];
	g_return_val_if_fail (GNOME_IS_FONT (pdf_font->gnome_font), -1);
	face = gnome_font_get_face (pdf_font->gnome_font);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -1);

	ret += gnome_print_pdf_write_content (pdf,
																				"%f %f %f %f %f %f Tm" EOL,
																				val_1,
																				val_2,
																				val_3,
																				val_4,
																				point.x,
																				point.y);
	gnome_print_pdf_graphic_state_text_set (pdf);

	/* I don't like the fact that we are writing one letter at time */
  if (gnome_print_pdf_write_content (pdf, "(") < 0)
    return -1;

#ifdef METHOD_ONE
  if (gnome_print_pdf_write_content (pdf, "\\376\\377") < 0)
    return -1;
#endif
	
  for (p = text; p && p < (text + bytes); p = g_utf8_next_char (p))
	{
		const gchar *glyph_name;
		gunichar u;
		gint g, glyph, page;

    u = g_utf8_get_char (p);
		g = gnome_font_face_lookup_default (face, u);
		glyph_name = gnome_font_face_get_glyph_ps_name (face, g);
		gnome_print_pdf_add_glyph_to_font (pdf_font, glyph_name);
		
		glyph = g & 0xff;
		page = (g >> 8) & 0xff;

#ifdef METHOD_ONE
		if (gnome_print_pdf_write_content (pdf,"\\%03o\\%03o", page, glyph) < 0)
			return -1;
#else
		if (page != 0) {
			static gboolean warned = FALSE;
			if (!warned)
				g_warning ("The Gnome PDF writer can't print characters outside "
									 "the main page, (page 0).");
			warned = TRUE;
		}
		if (gnome_print_pdf_write_content (pdf,"\\%03o", glyph) < 0)
			return -1;
#endif	
	}

	gnome_print_pdf_write_content (pdf, ")Tj" EOL);

  return ret;
}

static gint
gnome_print_pdf_clip (GnomePrintContext *pc, ArtWindRule rule)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;
	gint ret = 0;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	gnome_print_pdf_graphic_mode_set (pdf, PDF_GRAPHIC_MODE_GRAPHICS);

	gs = gnome_print_pdf_graphic_state_set (pdf);

	if (gp_path_length (gs->current_path) < 2) {
		gnome_print_pdf_error (FALSE, "Trying to clip with an empty path.");
		gp_path_reset (gs->current_path);
		return -1;
	}

	ret += gnome_print_pdf_path_print (pdf, gs->current_path);
	if (rule == ART_WIND_RULE_NONZERO)
		ret += gnome_print_pdf_write_content (pdf, "W n" EOL);
	else
		ret += gnome_print_pdf_write_content (pdf, "W* n" EOL);

	gp_path_reset (gs->current_path);
		
  return ret;
}


static gint
gnome_print_pdf_stroke (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;
	gint ret = 0;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	gnome_print_pdf_graphic_mode_set (pdf, PDF_GRAPHIC_MODE_GRAPHICS);
	
	gs = gnome_print_pdf_graphic_state_set (pdf);

	if (gp_path_length (gs->current_path) < 2) {
		gnome_print_pdf_error (FALSE, "Trying to stroke an empty path");
		gp_path_reset (gs->current_path);
		return -1;
	}

	ret += gnome_print_pdf_path_print (pdf, gs->current_path);
	ret += gnome_print_pdf_write_content (pdf, "S" EOL);

  return ret;
}

static gint
gnome_print_pdf_fill (GnomePrintContext *pc, ArtWindRule rule)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;
	gint ret = 0;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	gs = gnome_print_pdf_graphic_state_set (pdf);

	if (gp_path_length (gs->current_path) < 2) {
		gnome_print_pdf_error (FALSE, "Trying to fill an empty path");
		gp_path_reset (gs->current_path);
		return -1;
	}

	ret += gnome_print_pdf_path_print (pdf, gs->current_path);
	if (rule == ART_WIND_RULE_NONZERO)
		ret += gnome_print_pdf_write_content (pdf, "f" EOL);
	else
		ret += gnome_print_pdf_write_content (pdf, "f*" EOL);

	gp_path_reset (gs->current_path);

  return ret;
}

static gint
gnome_print_pdf_lineto (GnomePrintContext *pc, double x, double y)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;
	ArtPoint point;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	gs = gnome_print_pdf_graphic_state_current (pdf, FALSE);

	point.x = x;
	point.y = y;
	art_affine_point (&point, &point, gs->ctm);

	gp_path_lineto (gs->current_path, point.x, point.y);
	
	return 0;
}

static gint
gnome_print_pdf_curveto (GnomePrintContext *pc,
												 double x0, double y0,
												 double x1, double y1,
												 double x2, double y2)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;
	ArtPoint point_0;
	ArtPoint point_1;
	ArtPoint point_2;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	gs = gnome_print_pdf_graphic_state_current (pdf, FALSE);

	/* IS there a libart function for this ??? Chema */
	point_0.x = x0;
	point_0.y = y0;
	point_1.x = x1;
	point_1.y = y1;
	point_2.x = x2;
	point_2.y = y2;

	art_affine_point (&point_0, &point_0, gs->ctm);
	art_affine_point (&point_1, &point_1, gs->ctm);
	art_affine_point (&point_2, &point_2, gs->ctm);

	gp_path_curveto (gs->current_path,
									 point_0.x, point_0.y,
									 point_1.x, point_1.y,
									 point_2.x, point_2.y); 
	
	return 0;	
}

static gboolean
gnome_print_pdf_font_insert (GnomePrintPdf *pdf,
														 GnomeFont *gnome_font,
														 const gchar *font_name,
														 gboolean is_basic_14)
{
	GnomePrintPdfFont *font;

	debug (FALSE, "");
	
	g_return_val_if_fail (GNOME_IS_PRINT_PDF (pdf), FALSE);
	g_return_val_if_fail (GNOME_IS_FONT (gnome_font), FALSE);
	g_return_val_if_fail (font_name != NULL, FALSE);
	
	if (pdf->fonts_number == pdf->fonts_max)
		pdf->fonts = g_realloc (pdf->fonts, sizeof (GnomePrintPdfFont) *
													 (pdf->fonts_max += GNOME_PRINT_NUMBER_OF_ELEMENTS_GROW));

	font = &pdf->fonts[pdf->fonts_number++];
	font->font_number   = pdf->fonts_number;
	font->gnome_font    = gnome_font;
	font->font_name     = g_strdup (font_name);
	font->object_number = 0;
	font->object_number_descriptor = 5890990;
	font->object_number_pfb        = 2931915;
	font->is_basic_14   = is_basic_14;

	font->glyphs_max    = GNOME_PRINT_NUMBER_OF_ELEMENTS;
	font->glyphs_num    = 0;
	font->glyphs        = g_new (T1Glyph, font->glyphs_max);

	gtk_object_ref (GTK_OBJECT(gnome_font));
	
	return TRUE;
}

static gint
gnome_print_pdf_get_font_number (GnomePrintContext *pc,
																 GnomeFont *gnome_font,
																 gboolean is_basic_14)
{
	GnomePrintPdf *pdf;
	gint n;
	const gchar *font_name;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	g_return_val_if_fail (GNOME_IS_FONT (gnome_font), -1);

	font_name = gnome_font_get_ps_name (gnome_font);

	for (n = 0; n < pdf->fonts_number; n++)
		if (!strcmp (font_name, pdf->fonts[n].font_name))
			break;

	if (n != pdf->fonts_number)
		return n;

	gnome_print_pdf_font_insert (pdf, gnome_font, font_name, is_basic_14);

	return pdf->fonts_number-1;
}


static gint
gnome_print_pdf_setfont (GnomePrintContext *pc, GnomeFont *font)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;
  const char *fontname;
	gint n;
	gint is_basic_14;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
  pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

  if (font == NULL)
    return -1;

  fontname = gnome_font_get_ps_name (font);

  for (n = 0; n < pdf->fonts_internal_number; n++)
    if (!strcmp (fontname, pdf->fonts_internal[n].font_name))
      break;

	if (n == pdf->fonts_internal_number) {
		is_basic_14 = FALSE;
	}	else {
		is_basic_14 = pdf->fonts_internal[n].is_basic_14;
	}

	gs = pdf->graphic_state;
	gs->font_size = gnome_font_get_size (font);
	gs->pdf_font_number = gnome_print_pdf_get_font_number (pc, font, is_basic_14);

	return 0;
}

static gint
gnome_print_pdf_newpath (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;
	GnomePrintPdfGraphicState *gs;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	gs = gnome_print_pdf_graphic_state_current (pdf, FALSE);
	
	if (gp_path_length (gs->current_path) > 1) 
		g_warning ("Path was disposed without using it [newpath]\n");
	
	gp_path_reset (gs->current_path);

  return 0;
}

static gint
gnome_print_pdf_strokepath (GnomePrintContext *pc)
{
  static gboolean warned = FALSE;

	debug (FALSE, "");

	if (warned)
		return 0;
	
	g_warning ("Strokepath is not supported. It migth get deprecated in the future\n");
	warned = TRUE;
	
  return 0;
}

static gint
gnome_print_pdf_textline (GnomePrintContext *pc, GnomeTextLine *line)
{
	static gint warned = FALSE;

	debug (FALSE, "");

	if (warned)
		return 0;

	g_warning ("Pdf_textline not supported.\n");
	warned = TRUE;
	
	return 0;
}

#if 0
static gint
gnome_print_pdf_concat_optimize (GnomePrintContext *pc,
																 const double incoming_matrix[6])
{
	gint ret = 0;
	double matrix[6];

	matrix [0] = incoming_matrix [0];
	matrix [1] = incoming_matrix [1];
	matrix [2] = incoming_matrix [2];
	matrix [3] = incoming_matrix [3];
	matrix [4] = incoming_matrix [4];
	matrix [5] = incoming_matrix [5];
	
	gnome_print_pdf_matrix_clean (matrix);
	ret += gnome_print_context_fprintf (pc,
                                      "[ %g %g %g %g %g %g ] cm" EOL,
                                      matrix [0], matrix [1],
                                      matrix [2], matrix [3],
                                      matrix [4], matrix [5]);
	return 0;
}
#endif

static gint
gnome_print_pdf_concat (GnomePrintContext *pc, const double matrix[6])
{
	GnomePrintPdf * pdf;
	GnomePrintPdfGraphicState *gs;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	gs = gnome_print_pdf_graphic_state_current (pdf, FALSE);

	art_affine_multiply (gs->ctm, matrix, gs->ctm);

	return 0;
}


static gint
gnome_print_pdf_closepath (GnomePrintContext *pc)
{
	GnomePrintPdf *pdf;

	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);

	gp_path_closepath (pdf->graphic_state->current_path);

  return 0;
}



static gint
gnome_print_pdf_image_load (GnomePrintPdf *pdf, gchar *data, gint width, gint height,
														gint rowstride, gint bytes_per_pixel, gint image_type,
														gint data_length, PdfCompressionType compr_type)
{
	GnomePrintPdfImage *image;
	GnomePrintPdfPage *page;

	g_return_val_if_fail (GNOME_IS_PRINT_PDF(pdf), -1);
	
	page = pdf->current_page;
	
  if (page->images_number >= page->images_max)
		page->images = g_realloc (page->images,
															sizeof (GnomePrintPdfImage) *
															(page->images_max += GNOME_PRINT_NUMBER_OF_ELEMENTS_GROW));

	image = &page->images[page->images_number++];
	image->data = g_malloc (data_length+1);
	memcpy (image->data, data, data_length);
	image->data_length = data_length;
	image->width = width;
	image->height = height;
	image->rowstride = rowstride;
	image->bytes_per_pixel = bytes_per_pixel;
	image->image_number = page->images_number;
	image->object_number = gnome_print_pdf_object_number (GNOME_PRINT_CONTEXT(pdf));
	image->image_type = image_type;
	image->compr_type = compr_type;

	return image->image_number;
}

static gint
gnome_print_pdf_image_compressed (GnomePrintContext *pc,
																	const char *data,
																	gint width,
																	gint height,
																	gint rowstride,
																	gint bytes_per_pixel,
																	gint image_type)
{
	PdfCompressionType compr_type;
#ifdef ENABLE_LIBGPA
	const gchar *compr_string;
#endif
	GnomePrintPdfGraphicState *gs;
	GnomePrintPdf *pdf;
	gchar *image_stream = NULL;
	gint ret = 0;
	gint image_number;
	gint image_stream_size;

	g_return_val_if_fail (GNOME_IS_PRINT_PDF(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	
	debug (FALSE, "");

#ifdef ENABLE_LIBGPA
	if (image_type == PDF_IMAGE_GRAYSCALE)
		compr_string = gpa_settings_query_options (pdf->gpa_settings, "GrayscaleImagesCompression");
	else
		compr_string = gpa_settings_query_options (pdf->gpa_settings, "ColorImagesCompression");
	compr_type = gnome_print_pdf_compr_from_string (compr_string);
#else
	compr_type = PDF_COMPRESSION_HEX;
#endif
	
	gs = pdf->graphic_state;

	gnome_print_pdf_graphic_mode_set (pdf, PDF_GRAPHIC_MODE_GRAPHICS);
	
	ret += gnome_print_pdf_write_content (pdf, "%g %g %g %g %g %g cm" EOL,
																				gs->ctm[0], gs->ctm[1],
																				gs->ctm[2],	gs->ctm[3],
																				gs->ctm[4], gs->ctm[5]);
	ret += gnome_print_pdf_write_content (pdf, "0 0 m" EOL);

	/* 1. Get the required size (after compression) */
	switch (compr_type) {
	case PDF_COMPRESSION_FLATE:
		image_stream_size = gnome_print_encode_deflate_wcs (width * height * bytes_per_pixel);
		break;
	case PDF_COMPRESSION_HEX:
		image_stream_size = gnome_print_encode_hex_wcs (width * height * bytes_per_pixel);
		break;
	case PDF_COMPRESSION_NONE:
		image_stream_size = width * height * bytes_per_pixel;
		g_print ("SIze %i\n", image_stream_size);
		break;
	default:
		g_warning ("Compression Method undetermined. [%i]", compr_type);
		return -1;
	}

	if (compr_type != PDF_COMPRESSION_NONE) {
		/* 2. Allocate the buffer */
		image_stream      = g_new (gchar, image_stream_size);

		/* 3. Compress the image */
		switch (compr_type) {
		case PDF_COMPRESSION_FLATE:
			image_stream_size = gnome_print_encode_deflate (data, image_stream,
																											width * height * bytes_per_pixel,
																											image_stream_size);
			break;
		case PDF_COMPRESSION_HEX:
			image_stream_size = gnome_print_encode_hex (data, image_stream,
																									width * height * bytes_per_pixel);
			break;
		case PDF_COMPRESSION_NONE:
		default:
			g_warning ("Compression Method undetermined. [%i]", compr_type);
			return -1;
		}
	} else {
		image_stream = (gchar *)data;
		g_print ("Compression none ..copy data\n");
	}

	/* 4. Load it */
	image_number = gnome_print_pdf_image_load (pdf, image_stream,
																						 width, height,
																						 rowstride, bytes_per_pixel,
																						 image_type, image_stream_size,
																						 compr_type);

	ret += gnome_print_pdf_write_content (pdf, "/Im%i Do" EOL, image_number);
	
  return 0;
}

static gint
gnome_print_pdf_grayimage (GnomePrintContext *pc, const char *data, gint width, gint height, gint rowstride)
{
	GnomePrintPdf *pdf;
	gint ret = 0;

	g_return_val_if_fail (GNOME_IS_PRINT_PDF(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	
	debug (FALSE, "");

	pdf->current_page->used_grayscale_images = TRUE;
	
  ret += gnome_print_pdf_image_compressed (pc,
																					 data,
																					 width,
																					 height,
																					 rowstride,
																					 1,
																					 PDF_IMAGE_GRAYSCALE);

	return ret;
}

static gint
gnome_print_pdf_rgbimage (GnomePrintContext *pc,
													const char *data,
													gint width,
													gint height,
													gint rowstride)
{
	GnomePrintPdf *pdf;
	gint ret = 0;

	g_return_val_if_fail (GNOME_IS_PRINT_PDF(pc), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (pdf != NULL, -1);
	
	debug (FALSE, "");

	pdf->current_page->used_color_images = TRUE;
	
	ret += gnome_print_pdf_image_compressed (pc,
																					 data,
																					 width,
																					 height,
																					 rowstride,
																					 3,
																					 PDF_IMAGE_RGB);
	
  return ret;
}

static gint
gnome_print_pdf_page_free (GnomePrintPdfPage *page)
{
	debug (FALSE, "");

	g_return_val_if_fail (page != NULL, -1);
		
	g_free (page->images);
	g_free (page->page_name);
	g_free (page);

	return 0;
}

/* These are the PostScript 35, assumed to be in the printer. */
static const ps_internal_font gnome_print_pdf_internal_fonts[] = {
	{  "AvantGarde-Book",              FALSE},
	{  "AvantGarde-BookOblique",       FALSE},
	{  "AvantGarde-Demi",              FALSE},
	{  "AvantGarde-DemiOblique",       FALSE},
	{  "Bookman-Demi",                 FALSE},
	{  "Bookman-DemiItalic",           FALSE},
	{  "Bookman-Light",                FALSE},
	{  "Bookman-LightItalic",          FALSE},
	{  "Courier",                      TRUE},
	{  "Courier-Bold",                 TRUE},
	{  "Courier-BoldOblique",          TRUE},
	{  "Courier-Oblique",              TRUE},
	{  "Helvetica",                    TRUE},
	{  "Helvetica-Bold",               TRUE},
	{  "Helvetica-BoldOblique",        TRUE},
	{  "Helvetica-Narrow",             FALSE},
	{  "Helvetica-Narrow-Bold",        FALSE},
	{  "Helvetica-Narrow-BoldOblique", FALSE},
	{  "Helvetica-Narrow-Oblique",     FALSE},
	{  "Helvetica-Oblique",            TRUE},
	{  "NewCenturySchlbk-Bold",        FALSE},
	{  "NewCenturySchlbk-BoldItalic",  FALSE},
	{  "NewCenturySchlbk-Italic",      FALSE},
	{  "NewCenturySchlbk-Roman",       FALSE},
	{  "Palatino-Bold",                FALSE},
	{  "Palatino-BoldItalic",          FALSE},
	{  "Palatino-Italic",              FALSE},
	{  "Palatino-Roman",               FALSE},
	{  "Symbol",                       TRUE},
	{  "Times-Bold",                   TRUE},
	{  "Times-BoldItalic",             TRUE},
	{  "Times-Italic",                 TRUE},
	{  "Times-Roman",                  TRUE},
	{  "ZapfChancery-MediumItalic",    FALSE},
	{  "ZapfDingbats",                 TRUE}
};

static gboolean
gnome_print_pdf_free_fonts (GnomePrintPdf *pdf)
{
	GnomePrintPdfFont *font;
	gint n,i;
	debug (FALSE, "");

	g_return_val_if_fail (pdf != NULL, FALSE);
	
	for (n = 0; n < pdf->fonts_number; n++) {
		font = (GnomePrintPdfFont *) &pdf->fonts[n];
		g_free (font->font_name);
		for (i = 0; i < font->glyphs_num; i++) {
			gchar *name;
			name = font->glyphs[i].name;
			g_free (name);
		}
		g_free (font->glyphs);
		gtk_object_unref (GTK_OBJECT (font->gnome_font));
	}

	return TRUE;
}
					 

static gboolean
gnome_print_pdf_free_objects (GnomePrintPdf *pdf)
{
	GList *list;
	
	debug (FALSE, "");

	g_return_val_if_fail (pdf != NULL, FALSE);
		
	for (list = pdf->objects; list != NULL; list = list->next) {
		GnomePrintPdfObject *object;
		object = (GnomePrintPdfObject *) list->data;
		g_free (object);
	}

	return TRUE;
}

static gboolean
gnome_print_pdf_free_pages (GnomePrintPdf *pdf, gboolean *showpaged_all)
{
	GnomePrintPdfPage *page;
  GList *list;

	debug (FALSE, "");

	g_return_val_if_fail (pdf != NULL, FALSE);

	for (list = pdf->pages; list != NULL; list = list->next) {
		page = (GnomePrintPdfPage *) list->data;
		if (!page->showpaged)
			showpaged_all = FALSE;
		gnome_print_pdf_page_free (page);
	}
	gnome_print_pdf_page_free (pdf->current_page);
	g_free (pdf->current_page->stream);

	return TRUE;
}

static void
gnome_print_pdf_finalize (GtkObject *object)
{
	GnomePrintPdf *pdf;
	gint showpaged_all;

	debug (FALSE, "");

  g_return_if_fail (object != NULL);
  g_return_if_fail (GNOME_IS_PRINT_PDF (object));
  pdf = GNOME_PRINT_PDF (object);

	if (pdf->gsave_level_number != 0)
		g_warning ("gsave unmatched. Should end with an empty stack");

	gnome_print_pdf_graphic_state_free (pdf->graphic_state);
	gnome_print_pdf_graphic_state_free (pdf->graphic_state_set);
	gnome_print_pdf_free_objects (pdf);

	if (g_list_length (pdf->pages) == 0)
		showpaged_all = FALSE;
	else
		showpaged_all = TRUE;

	gnome_print_pdf_free_pages (pdf, &showpaged_all);

	if (!showpaged_all && (g_list_length(pdf->pages)>0))
		g_warning ("The application didn't called \"showpage\" for\n"
							 "one or more pages. Please report this bug for the\n"
							 "program you are using to print. Some **CRITICAL**\n"
							 "messages are normal because of this bug.\n"
							 "This is not a gnome-print bug.\n\n");

	g_list_free (pdf->objects);
	g_list_free (pdf->pages);

	gnome_print_pdf_free_fonts (pdf);

	g_free (pdf->gsave_stack);
	g_free (pdf->fonts);
	g_free (pdf->fonts_internal);

  (* GTK_OBJECT_CLASS (parent_class)->finalize) (object);

#ifdef ENABLE_LIBGPA
	gpa_printer_unref (pdf->gpa_printer);
	gpa_settings_unref (pdf->gpa_settings);
#endif

}

static void
gnome_print_pdf_init (GnomePrintPdf *pdf)
{
	GnomePrintContext *pc = GNOME_PRINT_CONTEXT (pdf);
	gint n;
	
	debug (FALSE, "");

	pdf->current_page = NULL;
	pdf->pages = NULL;
	pdf->current_page_number = 1;

	/* Object numbers */
	pdf->objects = NULL;
	pdf->object_number_last_used = 0;
	pdf->object_number_current = 0;
	pdf->object_number_gstate = 0;
	pdf->offset = 0;

	pdf->graphics_mode = PDF_GRAPHIC_MODE_GRAPHICS;

	/* Fonts */
	pdf->fonts_internal_number = sizeof(gnome_print_pdf_internal_fonts) /
		                           sizeof(ps_internal_font);
  pdf->fonts_internal = g_new (ps_internal_font, pdf->fonts_internal_number);
	for (n = 0; n < pdf->fonts_internal_number; n++) {
    pdf->fonts_internal[n].font_name = gnome_print_pdf_internal_fonts[n].font_name;
    pdf->fonts_internal[n].is_basic_14 = gnome_print_pdf_internal_fonts[n].is_basic_14;
	}
	
  pdf->fonts_max = GNOME_PRINT_NUMBER_OF_ELEMENTS;
	pdf->fonts_number = 0;
  pdf->fonts = g_new (GnomePrintPdfFont, pdf->fonts_max);

	/* Start Page */
	gnome_print_pdf_page_start (pc);

	/* Allocate and set defaults for the current graphic state */
	pdf->graphic_state     = gnome_print_pdf_graphic_state_new (FALSE);
	pdf->graphic_state_set = gnome_print_pdf_graphic_state_new (TRUE);

	/* gsave/grestore */
	pdf->gsave_level_max = GNOME_PRINT_NUMBER_OF_ELEMENTS;
	pdf->gsave_level_number = 0;
	pdf->gsave_stack = g_new (GnomePrintPdfGsave, pdf->gsave_level_max);
	
}

static void
gnome_print_pdf_class_init (GnomePrintPdfClass *class)
{
  GtkObjectClass *object_class;
  GnomePrintContextClass *pc_class;

  object_class = (GtkObjectClass *)class;
  pc_class     = (GnomePrintContextClass *)class;

	parent_class = gtk_type_class (gnome_print_context_get_type ());
	
  object_class->finalize = gnome_print_pdf_finalize;

  pc_class->newpath         = gnome_print_pdf_newpath;
  pc_class->moveto          = gnome_print_pdf_moveto;
  pc_class->lineto          = gnome_print_pdf_lineto;
  pc_class->curveto         = gnome_print_pdf_curveto;
  pc_class->closepath       = gnome_print_pdf_closepath;
  pc_class->setrgbcolor     = gnome_print_pdf_setrgbcolor;
  pc_class->fill            = gnome_print_pdf_fill;
  pc_class->setlinewidth    = gnome_print_pdf_setlinewidth;
  pc_class->setmiterlimit   = gnome_print_pdf_setmiterlimit;
  pc_class->setlinejoin     = gnome_print_pdf_setlinejoin;
  pc_class->setlinecap      = gnome_print_pdf_setlinecap;
  pc_class->setdash         = gnome_print_pdf_setdash;
  pc_class->strokepath      = gnome_print_pdf_strokepath;
  pc_class->stroke          = gnome_print_pdf_stroke;
  pc_class->setfont         = gnome_print_pdf_setfont;
  pc_class->show_sized      = gnome_print_pdf_show_sized;
  pc_class->concat          = gnome_print_pdf_concat;
  pc_class->gsave           = gnome_print_pdf_gsave;
  pc_class->grestore        = gnome_print_pdf_grestore;
  pc_class->clip            = gnome_print_pdf_clip;
  pc_class->grayimage       = gnome_print_pdf_grayimage;
  pc_class->rgbimage        = gnome_print_pdf_rgbimage;
  pc_class->textline        = gnome_print_pdf_textline;
  pc_class->showpage        = gnome_print_pdf_showpage;
  pc_class->beginpage       = gnome_print_pdf_beginpage;
  pc_class->setopacity      = gnome_print_pdf_setopacity;

  pc_class->close           = gnome_print_pdf_close;
}
