/* -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2 -*- */
#ifndef __GNOME_PRINT_PDF_TYPE1_H__
#define __GNOME_PRINT_PDF_TYPE1_H__

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-pdf.h>

BEGIN_GNOME_DECLS

gint gnome_print_pdf_font_type1_embed (GnomePrintContext *pc,
																			 GnomePrintPdfFont *font);
gint gnome_print_pdf_type1_get_stems (const GnomeFontFace *face,
																			gint *stemv, gint*stemh);

END_GNOME_DECLS

#endif /*  __GNOME_PRINT_PDF_TYPE1_H__ */
