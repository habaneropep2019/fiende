/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <math.h>
#include <libart_lgpl/art_affine.h>
#include <gnome.h>

#include <libgnomeprint/gnome-print-pdf.h>
#include <libgnomeprint/gnome-print-ps.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-printer-dialog.h>

static int pdf2font;
static int font2head;
static int font2body;
static int body2body;

static struct poptOption options [] = {
	{ "pdf2font", '\0', 0, &pdf2font, 0,
	  N_("extracts the first font from output.pdf "
	     "and places it in font.dump."), NULL },
	{ "font2head", '\0', 0, &font2head, 0,
	  N_("extracts the font header from font.dump"), NULL },
	{ "font2body", '\0', 0, &font2body, 0,
	  N_("extracts the font body "), NULL },
	{ "body2body", '\0', 0, &body2body, 0,
	  N_("decrypts the font body "), NULL },
	{NULL, '\0', 0, NULL, 0}	
};


static gint
text_utils_search_real (const gchar *buffer, gint buffer_length,
			const gchar *search_text, gint search_text_length,
			gboolean case_sensitive)
{
	gint p1;
	gint p2 = 0;
	gint case_sensitive_mask;

	case_sensitive_mask = case_sensitive?0:32;
	
	for (p1=0; p1 < buffer_length; p1 ++)
	{
		if ((buffer[p1]     |case_sensitive_mask)==
		    (search_text[p2]|case_sensitive_mask))
		{
			p2++;
			if (p2==search_text_length)
				return p1 - search_text_length + 1;
		}
		else
			p2 = 0;
	}
	
	return -1;
}


#define EEXEC_C1 ((unsigned short)52845)
#define EEXEC_C2 ((unsigned short)22719)
/**
 * decrypt_eexec:
 * @plaintext: unencrypted text, already malloced by the caller
 * @ciphertext: crypted text
 * @ciphertext_size: size of crypted buffer
 *
 * Unencrypt the midle section of the font
 * See : [2], pag 63
 * 
 * Return Value: size of unencrypted text
 **/
static int
decrypt_eexec (char *plaintext, const char *ciphertext, int ciphertext_size)
{
  int i;
  unsigned short r;
  unsigned char cipher;
  unsigned char plain;

  r = 55665;  /* initial key */

  for (i = 0; i < ciphertext_size; i++)
    {
      cipher = ciphertext[i];
      plain = (cipher ^ (r>>8));
      r = (cipher + r) * EEXEC_C1 + EEXEC_C2;
      if (i >= 4)
	plaintext[i - 4] = plain;
    }
  return ciphertext_size - 4;
}


#define TEST_TAG_1  "%!FontType1-1"
#define TEST_TAG_1B "%!PS-AdobeFon" /* Has to be same size as 1 */
#define TEST_TAG_2 "endstream"
#define TEST_TAG_3 "eexec"

static gint
do_body2body (const gchar* in_name, const gchar *out_name)
{
	FILE *f;
	gchar *in;
	gchar *out;
	gint in_size;
	gint in_size_max;
	gint bytes_read;
	gint size;
	
	g_print ("Extracting from %s into:  %s\n",
		 in_name, out_name);

	f = fopen (in_name, "r");
	if (f == NULL)
	{
		g_warning ("Couldn't open file %s\n", in_name);
		return FALSE;
	}

	in_size = 0;
	in_size_max = 32768;
	in = g_new (char, in_size_max);
	while (1)
	{
		bytes_read = fread (in + in_size, 1,
				    in_size_max - in_size, f);
		if (bytes_read == 0)
			break;
		in_size += bytes_read;
		in = g_realloc (in, in_size_max <<= 1);
	}
	
	if (!in_size) {
		g_warning ("Could not read form \"%s\"\n",
			   in_name);
		return FALSE;
	}

	g_print ("in_size %i\n", in_size);

	out = g_new (char, in_size * 2);
	size = decrypt_eexec (out, in, in_size);

	f = fopen (out_name, "w");
	if (f == NULL) {
		g_warning ("Could not open file %s writing\n",
			   out_name);
		return FALSE;
	}
	
	if (fwrite (out, 1, size, f) == EOF) {
		g_warning ("Could not write to file");
		return FALSE;
	}

	if (fclose (f) != 0) {
		g_warning ("Could not close the file\n");
		return FALSE;
	}

	g_free (out);
	
	return TRUE;
}

static gint
do_font2body (const gchar* in_name, const gchar *out_name)
{
	FILE *f;
	gchar *in;
	gchar *out;
	gchar *start;
	gint in_size;
	gint in_size_max;
	gint bytes_read;
	gint p1;
	gint size;
	
	g_print ("Extracting from %s into:  %s\n",
		 in_name, out_name);

	f = fopen (in_name, "r");
	if (f == NULL)
	{
		g_warning ("Couldn't open file %s\n", in_name);
		return FALSE;
	}

	in_size = 0;
	in_size_max = 32768;
	in = g_new (char, in_size_max);
	while (1)
	{
		bytes_read = fread (in + in_size, 1,
				    in_size_max - in_size, f);
		if (bytes_read == 0)
			break;
		in_size += bytes_read;
 		in = g_realloc (in, in_size_max <<= 1);
	}
	
	if (!in_size) {
		g_warning ("Could not read form \"%s\"\n",
			   in_name);
		return FALSE;
	}

	g_print ("in_size %i\n", in_size);

	p1 = text_utils_search_real (in, in_size,
				     TEST_TAG_3, strlen (TEST_TAG_3),
				     TRUE);

	size = in_size - p1 - strlen (TEST_TAG_3) - 2 /* For EOL */ - 1;
	start = in + p1 + strlen (TEST_TAG_3) + 1;

	out = g_new (gchar, size);
	memcpy (out, start, size);
	g_free (in);
	
	f = fopen (out_name, "w");
	if (f == NULL) {
		g_warning ("Could not open file %s writing\n",
			   out_name);
		return FALSE;
	}
	
	if (fwrite (out, 1, size, f) == EOF) {
		g_warning ("Could not write to file");
		return FALSE;
	}

	if (fclose (f) != 0) {
		g_warning ("Could not close the file\n");
		return FALSE;
	}

	g_free (out);

	return TRUE;
}

static gint
do_font2head (const gchar* font_name, const gchar *head_out)
{
	FILE *f;
	gchar *font;
	gchar *head;
	gint font_size;
	gint font_size_max;
	gint bytes_read;
	gint p1;

	
	g_print ("Extracting head from font. font :  %s into:  %s\n",
		 font_name, head_out);

	f = fopen (font_name, "r");
	if (f == NULL)
	{
		g_warning ("Couldn't open file %s\n", font_name);
		return FALSE;
	}

	font_size = 0;
	font_size_max = 32768;
	font = g_new (char, font_size_max);
	while (1)
	{
		bytes_read = fread (font + font_size, 1,
				    font_size_max - font_size, f);
		if (bytes_read == 0)
			break;
		font_size += bytes_read;
		font = g_realloc (font, font_size_max <<= 1);
	}
	
	if (!font_size) {
		g_warning ("Could not read the font form \"%s\"\n",
			   font_name);
		return FALSE;
	}

	g_print ("font_size %i\n", font_size);

	p1 = text_utils_search_real (font, font_size,
				     TEST_TAG_3, strlen (TEST_TAG_3),
				     TRUE);

	p1 += strlen (TEST_TAG_3);

	head = g_new (gchar, p1);
	memcpy (head, font, p1);
	g_free (font);
	
	f = fopen (head_out, "w");
	if (f == NULL) {
		g_warning ("Could not open file for writing\n");
		return FALSE;
	}
	
	if (fwrite (head, 1, p1, f) == EOF) {
		g_warning ("Could not write the file");
		return FALSE;
	}

	if (fclose (f) != 0) {
		g_warning ("Could not close the file\n");
		return FALSE;
	}

	return TRUE;
}

static gint
do_pdf2font (const gchar* pdf_name, const gchar *font_out)
{
	gchar *pdf;
	gchar *font;
	FILE *f;
	gint bytes_read;
	gint pdf_size;
	gint pdf_size_max;
	gint p1;
	gint p2;
	gint start;
	gint size;
	
	
	g_print ("Extracting font from %s and into %s\n",
		 pdf_name, font_out);

	f = fopen (pdf_name, "r");
	if (f == NULL)
	{
		g_warning ("Couldn't open font file %s\n", pdf_name);
		return FALSE;
	}

	pdf_size = 0;
	pdf_size_max = 32768;
	pdf = g_new (char, pdf_size_max);
	while (1)
	{
		bytes_read = fread (pdf + pdf_size, 1, pdf_size_max - pdf_size, f);
		if (bytes_read == 0)
			break;
		pdf_size += bytes_read;
		pdf = g_realloc (pdf, pdf_size_max <<= 1);
	}
	
	if (!pdf_size) {
		g_warning ("Could not read the font in \"%s\"\n", pdf_name);
		return FALSE;
	}

	g_print ("pdf_size %i\n", pdf_size);

	p1 = text_utils_search_real (pdf, pdf_size,
				     TEST_TAG_1, strlen (TEST_TAG_1),
				     TRUE);

	g_print ("p1 %i:%X\n", p1, p1);
	if (p1 == -1) {
		g_print ("2nd choice\n");
		p1 = text_utils_search_real (pdf, pdf_size,
					     TEST_TAG_1B, strlen (TEST_TAG_1B),
					     TRUE);
	}
	
	if (p1 == -1) {
		g_warning ("not found\n");
		return FALSE;
	}

	p2 = text_utils_search_real (pdf + p1, pdf_size,
				     TEST_TAG_2, strlen (TEST_TAG_2),
				     TRUE);

	if (p1 == -1) {
		g_warning ("not found\n");
		return FALSE;
	}
	

	g_print ("p2 %i:%X\n", p2, p2);

	start = p1;
	size  = p2/* EOL*/;

	g_print ("Size %i\n", size);
	
	font = g_new (gchar, size);
	memcpy (font, pdf + start, size);
	g_free (pdf);
	

	f = fopen (font_out, "w");
	if (f == NULL) {
		g_warning ("Could not open file for writing\n");
		return FALSE;
	}
	
	if (fwrite (font, 1, size, f) == EOF) {
		g_warning ("Could not write the file");
		return FALSE;
	}

	if (fclose (f) != 0) {
		g_warning ("Could not close the file\n");
		return FALSE;
	}
		 
	return TRUE;
}

int
main (int argc, char ** argv)
{
	gnome_init_with_popt_table ("testembed", "0.1", argc, argv, options, 0, NULL);

	if (!pdf2font &&
	    !font2head)
		g_print ("No arguments\n");
	
	if (pdf2font)
		do_pdf2font  ("output.pdf", "font.dump");
	if (font2head)
		do_font2head ("font.dump", "head.dump");
	if (font2body)
		do_font2body ("font.dump", "body.dump");
	if (body2body)
		do_body2body ("body.dump", "body.clean");
		
	return 0;
}
