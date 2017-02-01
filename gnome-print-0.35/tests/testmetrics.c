#include <math.h>
#include <libart_lgpl/art_affine.h>
#include <gnome.h>

#include "libgnomeprint/gnome-printer.h"
#include "libgnomeprint/gnome-printer-dialog.h"

#include "libgnomeprint/gnome-print-rbuf.h"
#include "libgnomeprint/gnome-print-frgba.h"
#include "libgnomeprint/gnome-print-preview.h"

#include "libgnomeprint/gnome-font-dialog.h"
#include "libgnomeprint/gnome-font-face.h"

static struct poptOption options [] = {
	{NULL}
};

#define FILE_NAME "FontMetrics"

typedef struct {
	char *font_name;
	gint is_basic_14;
} ps_internal_font;

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
	{  "ZapfDingbats",                 TRUE},
	
};

static gboolean
print_metrics (const gchar * font_name, FILE *file_pointer)
{
	GnomeFontFace *font_face;
	ArtPoint point;
	gchar *string;
	const gchar *name;
	gint first_char;
	gint last_char;
	gint m;
	gint glyph;
	
	fputs ("\nPrinting Metrics for font :", file_pointer);
	fputs (font_name, file_pointer);
	fputs ("\n", file_pointer);

	font_face = gnome_font_face_new (font_name);
	g_return_val_if_fail (font_face != NULL, FALSE);

	first_char = 32;
	last_char  = 126+1;

	for ( m = first_char; m < last_char ; m++)
	{
		glyph = gnome_font_face_lookup_default (font_face, m);
		gnome_font_face_get_glyph_stdadvance (font_face, glyph, &point);
		name = gnome_font_face_get_glyph_ps_name (font_face, glyph);
		string = g_strdup_printf ("%i\t%c\t%g\t%s\n", m, m, point.x,name);
		fputs (string, file_pointer);
		g_free (string);
	}
	
	gtk_object_unref (GTK_OBJECT(font_face));

	return TRUE;
}

static gboolean
do_deluxe (void)
{
	FILE  *file_pointer;
	gint max_fonts;
	gint n;

	max_fonts = sizeof(gnome_print_pdf_internal_fonts) /sizeof(ps_internal_font);

	file_pointer = fopen (FILE_NAME, "w");

	if (file_pointer == NULL) {
		g_print ("could not open file\n");
		return FALSE;
	}
		
	for (n = 0; n < max_fonts; n++)
		if (!print_metrics (gnome_print_pdf_internal_fonts[n].font_name, file_pointer))
			return FALSE;

	if (fclose (file_pointer) != 0)
		g_print ("there was an error while trying to cloe the file\n");
	
	return TRUE;
}

	
int
main (int argc, char ** argv)
{
	gnome_init_with_popt_table ("FontsDeluxe", "0.1", argc, argv, options, 0, NULL);

	do_deluxe ();

	g_print ("Metrics writen to %s\n", FILE_NAME);
	    
	return 0;
}

#if 0
static gint
latin_to_utf8 (gchar * text, gchar * utext, gint ulength)
{
	static gboolean uinit = FALSE;
	static unicode_iconv_t uiconv = (unicode_iconv_t) -1;
	const gchar * ibuf;
	gchar * obuf;
	gint ilen, olen, cnum;

	if (!uinit) {
		unicode_init ();
		uiconv = unicode_iconv_open ("UTF-8", "iso-8859-1");
		g_return_val_if_fail (uiconv != (unicode_iconv_t) -1, -1);
		uinit = TRUE;
	}

	ilen = strlen (text);
	ibuf = text;
	olen = ulength;
	obuf = utext;

	cnum = unicode_iconv (uiconv, &ibuf, &ilen, &obuf, &olen);

	g_return_val_if_fail (cnum >= 0, -1);

	return obuf - utext;
}
#endif

