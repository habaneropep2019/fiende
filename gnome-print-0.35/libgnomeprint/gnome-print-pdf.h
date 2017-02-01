/* -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2 -*- */
#ifndef __GNOME_PRINT_PDF_H__
#define __GNOME_PRINT_PDF_H__

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gp-path.h>

BEGIN_GNOME_DECLS

#define GNOME_TYPE_PRINT_PDF						(gnome_print_pdf_get_type ())
#define GNOME_PRINT_PDF(obj)						(GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_PDF, GnomePrintPdf))
#define GNOME_PRINT_PDF_CLASS(klass)		(GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_PDF, GnomePrintPdfClass))
#define GNOME_IS_PRINT_PDF(obj)	 	 			(GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_PDF))
#define GNOME_IS_PRINT_PDF_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_PDF))

typedef struct _GnomePrintPdf       GnomePrintPdf;
typedef struct _GnomePrintPdfClass  GnomePrintPdfClass;
typedef struct _GnomePrintPdfFont         GnomePrintPdfFont;
typedef struct _T1Glyph T1Glyph;

struct _GnomePrintPdfFont {
	GnomeFont *gnome_font;
	T1Glyph *glyphs;
	gint glyphs_num;
	gint glyphs_max;

	gint       font_number;
	gchar     *font_name;
	gboolean   is_basic_14;

	guint      object_number;
	guint      object_number_descriptor;
	guint      object_number_encoding;
	guint      object_number_pfb;
};


struct _T1Glyph {
	gchar *name;
	guchar *crypted;
	gint crypted_size;
	guchar *uncrypted;
	gint uncrypted_size;
};



GtkType 	      gnome_print_pdf_get_type (void);
GnomePrintPdf*	gnome_print_pdf_new_with_paper (GnomePrinter *printer,
																								const gchar *paper);

gint gnome_print_pdf_object_end (GnomePrintContext *pc,
																 guint object_number,
																 guint dont_print);
gint gnome_print_pdf_object_start (GnomePrintContext *pc,
																	 guint object_number);

gint gnome_print_pdf_add_bytes_written (GnomePrintPdf *pdf, gint bytes);

gint gnome_print_pdf_write (GnomePrintContext *pc,
														const char *format, ...);


END_GNOME_DECLS

#endif /* __GNOME_PRINT_PDF_H__ */
