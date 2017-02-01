/* -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/*
 * gnome-print-pdf-type1.c: Type 1 font handling for PDF
 *
 * Authors:
 *   Chema Celorio (chema@celorio.com)
 *
 * (C) 2000 Jose M Celorio1528
 *
 * References :
 * 
 * [1] Portable Document Format Referece Manual, Version 1.3 (March 11, 1999)
 * [2] Adobe Type 1 Font Format 
 *
 */


/* __FUNCTION__ is not defined in Irix. thanks, <drk@sgi.com>*/
#ifndef __GNUC__
  #define __FUNCTION__   ""
#endif
#define debug(section,str) if (FALSE) printf ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str); 
	
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdlib.h> /* For atoi */
#include <glib.h>
#include <ctype.h>
#include <stdio.h>

#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-pdf-type1.h>
#if 0
#include <libgnomeprint/gnome-font-private.h>
#endif
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/text-utils.h>

#define EOL "\r\n"
#if 1 /* Use a very low number of elements so that we reallocate stuff
				 and test the code for array growing */
#define GNOME_PRINT_NUMBER_OF_ELEMENTS 2
#define GNOME_PRINT_NUMBER_OF_ELEMENTS_GROW 2
#else
#define GNOME_PRINT_NUMBER_OF_ELEMENTS 16
#define GNOME_PRINT_NUMBER_OF_ELEMENTS_GROW 8
#endif

typedef struct _GnomePrintPdfSubFont GnomePrintPdfSubFont;

struct _GnomePrintPdfSubFont {
	GnomePrintPdfFont *pdf_font;

	gboolean *subs;
	gint subs_num;
	
	gchar *delim_readstring;
	gchar *delim_def;
	gchar *delim_put;

	guchar *decrypted;
	guchar *header;
	guchar *subrutines;
	guchar *charstrings;
	guchar *footer;

	gint decrypted_size;
	gint header_size;
	gint subrutines_size;
	gint charstrings_size;
	gint footer_size;
};

/**
 * text_utils_search_real:
 * @buffer: buffer to do the searching in
 * @buffer_length: length of the buffer
 * @search_text: text to search for
 * @search_text_length: lenght of the search text
 * @case_sensitive: TRUE if the search is case sensitive, FALSE otherwise
 * 
 * Searches for "search_text" in "buffer" and returns the location where it is found
 * it can search buffers that contain ascii ceros. That is why you need to provide
 * buffer & search text length
 *
 * Return Value: 
 **/
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


/**
 * gnome_print_pdf_type1_get_length: 
 * @pfb: pfb buffer
 * 
 * returns the numeric value of 2 bytes starting at pfb
 *
 * Return Value: 
 **/
static gint
gp_t1_get_length (guchar *pfb)
{
	guchar msb;
	guchar lsb;
	gint resp;

  lsb = pfb [0];
	msb = pfb [1];

	resp = (msb * 256) + lsb;

	return resp;
}

#define PDF_FONT_TAG_1 "%!"
#define PDF_FONT_TAG_2 "currentfile eexec"
#define PDF_FONT_TAG_3 "0000000000"

/**
 * gnome_print_pdf_type1_determine_lengths:
 * @pfb: font's pfb buffer
 * @pfb_size: size of pfb
 * @length1: length of the 1st section
 * @length2: 2nd
 * @length3: 3rd
 * 
 * given a pdf, it determines the lengths of the 1st,2nd and 3rd sections by string
 * mathcing. It is use to verify that the lengths of the font file specified on
 * the block identifiers are correct.
 * 
 * Return Value: TRUE on success, FALSE on error
 **/
static gboolean
gp_t1_determine_lengths (guchar *pfb, gint pfb_size, gint *length1, gint *length2, gint *length3)
{
	gint pos1;
	gint pos2;
	gint pos3;
	gint temp;

	debug (FALSE, "");

	
	g_return_val_if_fail (pfb != NULL, FALSE);

	/* Search for %! */
	pos1 = text_utils_search_real (pfb, pfb_size,
																 PDF_FONT_TAG_1, strlen (PDF_FONT_TAG_1),
																 TRUE);
	if (pos1 == -1) {
		g_warning ("Could not find %s\n", PDF_FONT_TAG_1);
		return FALSE;
	}

	/* Search for "currentfile eexec"*/
	pos2 = text_utils_search_real (pfb, pfb_size,
																 PDF_FONT_TAG_2, strlen (PDF_FONT_TAG_2),
																 TRUE);
	if (pos2 == -1) {
		g_warning ("Could not find %s\n", PDF_FONT_TAG_2);
		return FALSE;
	}

	/* Search for 00000 */
	pos3 = text_utils_search_real (pfb, pfb_size,
																 PDF_FONT_TAG_3, strlen (PDF_FONT_TAG_3),
																 TRUE);

	if (pos3 == -1) {
		g_warning ("Could not find %s\n", PDF_FONT_TAG_3);
		return FALSE;
	}

	temp = pos2 + strlen (PDF_FONT_TAG_2) - pos1 + 1;

	*length1 = temp;
	*length2 = pos3 - pos2 - strlen (PDF_FONT_TAG_2) - 1 - 12;
                       /*12 are the 2 x 6 bytes block init */
	*length3 = 0;

	debug (FALSE, "end");

	
	return TRUE;
}

/**
 * gnome_print_pdf_type1_get_lengths: 
 * @pfb: pfb buffer
 * @length1: returns the length of the 1st section
 * @length2: 2nd section
 * @length3: 3rd section
 * 
 * reads the lengths of the 3 sections of the pfb based on the blocks identifiers
 * it also performs basic checking of the blocks identiiers.
 *
 * Return Value: TRUE on success, FALSE otherwise
 **/
	
static gboolean
gp_t1_get_lengths (guchar *pfb, gint pfb_size, gint *len1, gint *len2, gint *len3)
{
	gint length1;
	gint length2;
	gint length3;

	debug (FALSE, "");
	
	g_return_val_if_fail (pfb != NULL, FALSE);
	
	if ((pfb [0] != 0x80) ||
			(pfb [1] != 0x01)) {
		g_warning ("Expected 0x80,0x01 at the begining of the pfb (1)\n");
		return FALSE;
	}
	if ((pfb [4] != 0x00) ||
			(pfb [5] != 0x00)) {
		g_warning ("Expected 0x00,0x00 at the begining of the pfb (2)\n");
		return FALSE;
	}

	*len1 = gp_t1_get_length (pfb + 2);

	if ((pfb [ *len1 + 6]     != 0x80) ||
			(pfb [ *len1 + 6 + 1] != 0x02)) {
		g_warning ("Expected 0x80,0x02 at the midle of the pfb (3)\n");
		return FALSE;
	}
	if ((pfb [ *len1 + 6 + 4] != 0x00) ||
			(pfb [ *len1 + 6 + 5] != 0x00)) {
		g_warning ("Expected 0x00,0x00 at the middle of the pfb (4)\n");
		return FALSE;
	}

	*len2 = gp_t1_get_length (pfb + *len1 + 2 + 6);
	*len3 = 0;

	if (!gp_t1_determine_lengths (pfb, pfb_size, &length1, &length2, &length3)) {
		g_warning ("Could not determine lengths from font file");
		return -1;
	}
	if ((*len1 != length1) ||
			(*len2 != length2) ||
			(*len3 != length3) ) {
		g_warning ("The lengths of the font do not match [%i,%i,%i] [%i,%i,%i]",
							 *len1, *len2, *len3, length1, length2, length3);
		return -1;
	}
	
	
	debug (FALSE, "end");
	
	return TRUE;
}


/**
 * gnome_print_pdf_type1_read_pfb:
 * @file_name: full path of the .pfb file
 * @pfb_size: return the length read here
 * 
 * reads a pfb from filename 
 * 
 * Return Value: pointer to the malloced buffer, caller must free it
 *               NULL on error
 **/
static guchar*
gp_t1_read_pfb (const gchar* file_name, gint *pfb_size_)
{
	FILE *f;
	gint pfb_size_max;
	gint bytes_read;
	gint pfb_size;
	guchar *pfb;

	*pfb_size_ = 0;
	pfb_size = 0;
	
	g_return_val_if_fail (file_name != NULL, NULL);
	
	f = fopen (file_name, "r");
  if (f == NULL)
    {
      g_warning ("Couldn't open font file %s\n", file_name);
      return NULL;
    }

  pfb_size = 0;
  pfb_size_max = 32768;
  pfb = g_new (guchar, pfb_size_max);
  while (1)
    {
      bytes_read = fread (pfb + pfb_size, 1, pfb_size_max - pfb_size, f);
      if (bytes_read == 0)
				break;
      pfb_size += bytes_read;
      pfb = g_realloc (pfb, pfb_size_max <<= 1);
    }

	*pfb_size_ = pfb_size;

	if (0 != fclose (f))
		g_warning ("Could not close %s", file_name);
	
	return pfb;
}


/**
 * gnome_print_pdf_font_parse:
 * @file_name: full path of the pfb of the font
 * @body_: return the cleaned structure here, the caller must free it later
 *         by cleaned I mean that it does not contain block identifiers and
 *         has the trailing ceros removed. So it is only section 1 & 2.
 * @length: the length of the block
 * @length1: length of the 1st section of the font
 * @length2: "" 2nd ""
 * @length3: "" 3rd "", which will always be 0 for now.
 * @body_length: The length of @body
 * 
 * Parses the type1 font pointed by *file_name and cleans it by removing
 * the block identifiers and the 3rd secciont (section of 512 0's). It also
 * calculates the lengths of the sections of the font. See [2].
 *
 * Return Value: TRUE on success, FALSE otherwise
 **/
static gboolean
gp_t1_font_parse (const gchar *file_name,
									gchar **body_,
									gint *length, gint *len1, gint *len2, gint *len3,
									gint *body_length)
{
	guchar *pfb;
	guchar *pfb_clean;
	gint pfb_size;

	debug (FALSE, "");

	*length = 0;
	*len1 = 0;
	*len2 = 0;
	*len3 = 0;

	pfb = gp_t1_read_pfb (file_name, &pfb_size);

	if ((pfb == NULL) || (!pfb_size)) {
		g_warning ("Could not read the font in \"%s\"\n", file_name);
		return FALSE;
	}

	if (!gp_t1_get_lengths (pfb, pfb_size, len1, len2, len3)) {
		g_warning ("Could not get lengths from font file");
		return FALSE;
	}

	*length = *len1 + *len2 + strlen (EOL);
	*len3 = 0;
	
	*body_length = *length;

  pfb_clean = g_new (guchar, *len1 + *len2 + 1);

	/* The block identifiers are 6 bytes long */
	memcpy (pfb_clean, pfb + 6, *len1);
	memcpy (pfb_clean + *len1, pfb + 6 + *len1 + 6, *len2);
	
	*body_ = pfb_clean;
	*body_length = *len1 + *len2;

	g_free (pfb);

	debug (FALSE, "end");
	
	return TRUE;
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

#if FEAUTRE_NOT_IMPLEMENTED
/**
 * charstring_decrypt:
 * @plaintext: encrypted charstring
 * @plaintext_size: encrypted charstring size
 * @ciphertext: unencrypted charstring
 * @ciphertext_size: unencrypted charstring size
 * 
 * Unencrypt a charstring
 * See [2].
 *
 **/
static void
charstring_decrypt (gchar *plaintext, gint plaintext_size,
										gchar *ciphertext, gint ciphertext_size)
{
  int i;
	unsigned short r;
  unsigned char cipher;
  unsigned char plain;

  if (plaintext_size < ciphertext_size - 4)
    {
      printf ("not enough space allocated for charstring decryption\n");
      return;
    }

	/* Initial key */
  r = 4330;

  for (i = 0; i < ciphertext_size; i++)
    {
      cipher = ciphertext[i];
      plain = (cipher ^ (r>>8));
      r = (cipher + r) * EEXEC_C1 + EEXEC_C2;
      if (i >= 4)
				plaintext[i - 4] = plain;
    }
  plaintext_size = ciphertext_size - 4;
}

#if FEAUTRE_NOT_IMPLEMENTED
/**
 * print_glyph_code:
 * @plaintext: 
 * @plaintext_size: 
 * 
 * Dump to the console the glyph_code, courtesy of Raph Levien, taken
 * from gt1-parset.c
 *
 **/
static void
print_glyph_code (guchar *plaintext, gint plaintext_size)
{
  int i;
  int byte, byte1, byte2, byte3, byte4;

  for (i = 0; i < plaintext_size; i++)
    {
      byte = ((unsigned char *)plaintext)[i];
      if (byte >= 32 && byte <= 246)
				printf (" %d", byte - 139);
      else if (byte >= 247 && byte <= 250)
			{
				byte1 = ((unsigned char *)plaintext)[++i];
				printf (" %d", ((byte - 247) << 8) + byte1 + 108);
			}
      else if (byte >= 251 && byte <= 254)
			{
				byte1 = ((unsigned char *)plaintext)[++i];
				printf (" %d", -((byte - 251) << 8) - byte1 - 108);
			}
      else if (byte == 255)
			{
				byte1 = ((unsigned char *)plaintext)[++i];
				byte2 = ((unsigned char *)plaintext)[++i];
				byte3 = ((unsigned char *)plaintext)[++i];
				byte4 = ((unsigned char *)plaintext)[++i];
				/* warning: this _must_ be a 32 bit int - alpha portability
					 issue! */
				printf (" %d", (byte1 << 24) + (byte2 << 16) + (byte3 << 8) + byte4);
			}
      else if (byte == 12)
			{
				byte1 = ((unsigned char *)plaintext)[++i];
				if (byte1 == 6)
					printf (" seac");
				else if (byte1 == 7)
					printf (" sbw");
				else if (byte1 == 0)
					printf (" dotsection");
				else if (byte1 == 2)
					printf (" hstem3");
				else if (byte1 == 1)
					printf (" vstem3");
				else if (byte1 == 12)
					printf (" div");
				else if (byte1 == 16)
					printf (" callothersubr");
				else if (byte1 == 17)
					printf (" pop");
				else if (byte1 == 33)
					printf (" setcurrentpoint");
				else
					printf (" esc%d", byte1);
			}
      else if (byte == 14)
				printf (" endchar");
      else if (byte == 13)
				printf (" hsbw");
      else if (byte == 9)
				printf (" closepath");
      else if (byte == 6)
				printf (" hlineto");
      else if (byte == 22)
				printf (" hmoveto");
      else if (byte == 31)
				printf (" hvcurveto");
      else if (byte == 5)
				printf (" rlineto");
      else if (byte == 21)
				printf (" rmoveto");
      else if (byte == 8)
				printf (" rrcurveto");
      else if (byte == 30)
				printf (" vhcurveto");
      else if (byte == 7)
				printf (" vlineto");
      else if (byte == 4)
				printf (" vmoveto");
      else if (byte == 1)
				printf (" hstem");
      else if (byte == 3)
				printf (" vstem");
      else if (byte == 10)
				printf (" callsubr");
      else if (byte == 11)
				printf (" return");
      else
				printf (" com%d", byte);
    }
  printf ("\n");
}
#endif

/**
 * gp_t1_read_glyph:
 * @buffer: buffer to read from 
 * @glyph: the already allocated T1Glyph struct, whichi is missing all but
 * the glyph name.
 * 
 * Read the rest of the glyph infrmation
 *
 * Returns: FALSE on error, TRUE otherwise
 **/
static gint
gp_t1_read_glyph (gchar *buffer, gint buffer_size, T1Glyph *glyph)
{
	gchar *search_for;
	gchar *size_str;
	gint pos;
	gint len;
	gint n;

	g_return_val_if_fail (buffer != NULL, FALSE);
	g_return_val_if_fail (glyph != NULL, FALSE);
		
	/* Search in buffer for the glyph */
	len = strlen (glyph->name) + 2;
	search_for = g_strdup_printf ("/%s ", glyph->name);
	pos = text_utils_search_real (buffer, buffer_size,
																search_for, len,
																TRUE);
	g_free (search_for);
	buffer += pos + len;
	
	/* Read the size of the crypted portion */
	for (n = 1; n < 10; n++)
		if ( buffer [n] == ' ')
			break;
	size_str = g_strndup (buffer, n);
	glyph->crypted_size = atoi (size_str);
	g_free (size_str);
	buffer += n;

	/* Skip the start char tag */
	/* TODO, verify that the start tag is the same in all chars */
	for (n = 1; n < 10; n++) {
#if 0
		g_print ("%c", buffer [n]);
#endif	
		if ( buffer [n] == ' ')
			break;
	}
#if 0	
	g_print ("\n");
#endif	
	if (n == 9) {
		g_warning ("Error while reading glyph");
		return FALSE;
	}

	/* Copy to the structure the crypted & uncrypted steams */
	glyph->crypted = g_new (guchar, glyph->crypted_size);
	memcpy (glyph->crypted, buffer+n+1, glyph->crypted_size);
	glyph->uncrypted_size = glyph->crypted_size - 4;
	if (glyph->uncrypted_size > 0) {
		glyph->uncrypted = g_new (guchar, glyph->uncrypted_size);
		charstring_decrypt (glyph->uncrypted, glyph->uncrypted_size,
												glyph->crypted, glyph->crypted_size);
	} else {
		glyph->uncrypted = NULL;
		glyph->uncrypted_size = 0;
	}

#if 0	
	g_print ("Size *%s*%i*%i*\n",
					 glyph->name,
					 glyph->crypted_size,
					 glyph->uncrypted_size);

	print_glyph_code (glyph->uncrypted, glyph->uncrypted_size);
#endif	

	return TRUE;
}



/**
 * gp_t1_create_subs_array:
 * @font: 
 * @buffer: 
 * 
 * 
 * 
 * Return Value: 
 **/
static gint
gp_t1_create_subs_array (GnomePrintPdfSubFont *sub_font, const gchar *buffer)
{
	gchar temp [16];
	gint n;
	
	debug (FALSE, "");

	g_return_val_if_fail (sub_font != NULL, -1);

	/* Get the length of the subrs array */
	for (n = 0; n < 6; n++) {
		if (buffer [n + 1] == ' ')
			break;
		temp [n] = buffer [n + 1];
	}
	temp [n] = 0;

	/* Create the array now, and tag all the subrutines as false */
	sub_font->subs_num = atoi (temp);
	sub_font->subs = g_new (gboolean, sub_font->subs_num);
	for (n = 0; n < sub_font->subs_num; n++) {
		sub_font->subs[n] = FALSE;
	}

	return sub_font->subs_num;
}

/**
 * gp_t1_load_subs_from_glyph:
 * @glyph: The glyph that we need to analize
 * @subs: the subs structure that we need to flag as TRUE for the subs used
 *        for @glyph
 * 
 * See the print_glyph_code function to see what this is all about
 * 
 * Return Value: TRUE on success, FALSE otherwise
 **/
static gboolean
gp_t1_load_subs_from_glyph (T1Glyph *glyph, GnomePrintPdfSubFont *sub_font)
{
	gchar *plaintext;
	gint plaintext_size;
  gint byte;
  gint i;
	gint sub_num = 0;

	debug (FALSE, "");

	g_return_val_if_fail (glyph != NULL, FALSE);
	g_return_val_if_fail (sub_font != NULL, FALSE);

	plaintext = glyph->uncrypted;
	plaintext_size = glyph->uncrypted_size;

  for (i = 0; i < plaintext_size; i++)
	{
		byte = ((unsigned char *)plaintext)[i];
		if (byte >= 32 && byte <= 246)
			sub_num = byte - 139;
		else if (byte >= 247 && byte <= 250)
			i++;
		else if (byte >= 251 && byte <= 254)
			i++;
		else if (byte == 255)
			i += 4;
		else if (byte == 12)
			i++;
		else if (byte == 10) {
			sub_font->subs [sub_num] = TRUE;
			if ((sub_num < 0) ||
					(sub_num > sub_font->subs_num)) {
				g_warning ("Invalid subrutine number (%i)\n", sub_num);
				g_print ("Dumping glyph code\n-gc-->");
				print_glyph_code (plaintext, plaintext_size);
				g_print ("<--gc-\n");
				return FALSE;
			}
#if 0
			g_print ("callsubr [%i]\n", sub_num);
#endif	
		}
	}
			
	return TRUE;
}
#endif

#define PDF_INITIAL_BUFFER_SIZE 1024
#if FEAUTRE_NOT_IMPLEMENTED
static gboolean
gp_t1_write_buffer (guchar **buffer_, gint *offset_, gint *max_, const char *format, ...)
{
	va_list arguments;
	guchar *buffer;
	gchar *text;
	gint text_length;
	gint offset;
	gint max;

	debug (FALSE, "");

	buffer = *buffer_;
	offset = *offset_;
	max = *max_;

	g_print ("write buffer\n");
	
	g_return_val_if_fail (buffer != NULL, FALSE);
	
	va_start (arguments, format);
	text = g_strdup_vprintf (format, arguments);
	va_end (arguments);

	g_print ("text *%s*\n", text);
	
	text_length = strlen (text);

	if (offset + text_length + 2 > max ) {
		g_print ("realloc\n");
		max += PDF_INITIAL_BUFFER_SIZE;
		buffer = g_realloc (buffer, max);
	}

	memcpy (buffer + offset,
					text,
					text_length * sizeof (gchar));

	offset += text_length;

	g_free (text);
	
	*offset_ = offset;
	*buffer_ = buffer;
	*max_ = max;

	return 0;
}

static gboolean
gp_t1_get_subrutine (GnomePrintPdfSubFont *sub_font,
										 gint num,
										 guchar **buffer_,
										 gint *buffer_size_)
{
	guchar *buffer;
	gint buffer_size;
	
	g_return_val_if_fail (sub_font != NULL, FALSE);

	buffer = NULL;
	buffer_size = 0;
	
	*buffer_ = NULL;
	*buffer_size_ = 0;
	
	return TRUE;
}

static gboolean
gp_t1_create_subrutines (GnomePrintPdfSubFont *sub_font)
{
	guchar *buffer;
	guchar *subrutine;
	gint subrutine_length;
	gint offset = 0;
	gint max;
	gint n;

	g_return_val_if_fail (sub_font != NULL, FALSE);
	g_return_val_if_fail (sub_font->delim_readstring != NULL, FALSE);
	g_return_val_if_fail (sub_font->delim_put != NULL, FALSE);
	
	max = PDF_INITIAL_BUFFER_SIZE;
	buffer = g_new (guchar, max);
	gp_t1_write_buffer (&buffer, &offset, &max, "/Subrs %i array\n", sub_font->subs_num);

	for (n = 0; n < sub_font->subs_num; n++) {
		if (sub_font->subs[n]) {
			if (!gp_t1_get_subrutine (sub_font, n, &subrutine, &subrutine_length))
				return FALSE;
			gp_t1_write_buffer (&buffer, &offset, &max, "dup %i %i %s %s%s\n",
													n,
													subrutine_length,
													sub_font->delim_readstring,
													"Algo",
													sub_font->delim_put);
		}	else {
			gp_t1_write_buffer (&buffer, &offset, &max, "dup %i 1 %s $ %s\n",
													n,
													sub_font->delim_readstring,
													sub_font->delim_put);
		}
	}
		
	/*
		/Subrs 223 array
		dup 0 15 RD ¸Á8{{ÜŠÿÉçÐ›’ noaccess put
	*/

	g_print ("\n-->");
	for (n=0;n<offset;n++)
		g_print ("%c", buffer [n]);
	g_print ("<--\n");


	return TRUE;
}


#define UNIQUE_ID_TAG "/UniqueID "
static gboolean
gp_t1_clean_header (GnomePrintPdfSubFont *sub_font, gint in_size)
{
	guchar *out;
	gint out_size;
	gint pos;
	
	debug (FALSE, "");
	
	g_return_val_if_fail (sub_font != NULL, FALSE);
	
	out = g_new (guchar, in_size);
	memcpy (out, sub_font->decrypted, in_size);
	out_size = in_size;

	/* Replace "/UniqueID" with "/XniqueId" */
	/* This is a Hacky solution, but I don't want to implement a
		 mini-postrcript parser*/
	pos = text_utils_search_real (out, out_size,
																UNIQUE_ID_TAG, strlen (UNIQUE_ID_TAG),
																TRUE);
	if (pos > 0) 
		out [pos+1] = 'X';

#if 0
	{
		gint n
			g_print ("\n--- out --->");
		for (n = 0; n < out_size; n++)
			g_print ("%c", out [n]);
		g_print ("<--- out ---\n");
	}
#endif

	sub_font->header = out;
	sub_font->header_size = out_size;
	
	return TRUE;
}

#define PDF_SUB_TAG "/Subrs"
#define PDF_ARRAY_TAG "array"
#define PDF_TAG_DUP_0 "dup 0 "


static gboolean
gp_t1_get_delimiters_method_two (guchar *buffer, gint buffer_length,
																 gchar **delim_readstring_,
																 gchar **delim_def_,
																 gchar **delim_put_)
{
	gchar *token;
	gchar *delim_readstring = NULL;
	gchar *delim_def = NULL;
	gchar *delim_put = NULL;
	gint pos;
	gint subrutine_size;
	gint *offset;
	gboolean ret;
																 
	pos = text_utils_search_real (buffer, buffer_length, PDF_SUB_TAG,
																strlen (PDF_SUB_TAG), TRUE);

	g_return_val_if_fail (pos > 0, FALSE);

	buffer += pos;
	buffer_length -= pos;
	offset = g_new (gint, 1);
	*offset = 0;
	
	ret = tu_token_next_verify (buffer, offset, PDF_SUB_TAG);
	g_return_val_if_fail (ret, FALSE);
	
	token = tu_token_next_dup (buffer, buffer_length, offset);
	g_return_val_if_fail (token != NULL, FALSE);
#if 0	
	g_print ("Array size is %i\n", atoi (token));
#endif	
	g_free (token);
	
	ret = tu_token_next_verify (buffer, offset, "array");
	g_return_val_if_fail (ret, FALSE);
	ret = tu_token_next_verify (buffer, offset, "dup");
	g_return_val_if_fail (ret, FALSE);
	ret = tu_token_next_verify (buffer, offset, "0");
	g_return_val_if_fail (ret, FALSE);
	token = tu_token_next_dup (buffer, buffer_length, offset);
	g_return_val_if_fail (token != NULL, FALSE);
	subrutine_size = atoi (token);
	g_free (token);

#if 0	
	g_print ("This subrutine size is %i\n", subrutine_size);
#endif	

	delim_readstring = tu_token_next_dup (buffer, buffer_length, offset);
	g_return_val_if_fail (delim_readstring != NULL, FALSE);
	*offset += subrutine_size + 1;

	delim_put = tu_token_next_dup_till_newline (buffer, buffer_length, offset);
	g_return_val_if_fail (delim_put != NULL, FALSE);

	*delim_readstring_ = delim_readstring;
	*delim_put_ = delim_put;
	*delim_def_ = delim_def;

	g_free (offset);
	
	return TRUE;
}

static gchar *
gp_t1_get_previous_label (guchar *buffer, gint pos)
{
	gint n;
	
	g_return_val_if_fail (buffer != NULL, NULL);
	
	if ((pos > 0) && (buffer [pos - 1] == ' ')) {
		for (n=0; n < pos; n++)
			if (buffer [pos - n] == '/')
				break;
		if (n != pos)
			return g_strndup (buffer + pos - n + 1, n - 2);
	}
	return NULL;
}

#define PDF_DELIM_READSTRING_TAG "{string currentfile exch readstring pop} executeonly"
#define PDF_DELIM_DEF_TAG        "{noaccess def} executeonly"
#define PDF_DELIM_PUT_TAG        "{noaccess put} executeonly"

static gboolean
gp_t1_get_delimiters_method_one (guchar *buffer, gint in_size,
																 gchar **delim_readstring_,
																 gchar **delim_def_,
																 gchar **delim_put_)
{
	gchar *delim_readstring = NULL;
	gchar *delim_def = NULL;
	gchar *delim_put = NULL;
	gint pos1, pos2, pos3;
																 
	/* Get the delimiters by 2 methods, and then compare the results */
	/* Method 1 */
	pos1 = text_utils_search_real (buffer, in_size, PDF_DELIM_READSTRING_TAG,
																 strlen (PDF_DELIM_READSTRING_TAG), TRUE);
	pos2 = text_utils_search_real (buffer, in_size, PDF_DELIM_DEF_TAG,
																 strlen (PDF_DELIM_DEF_TAG), TRUE);
	pos3 = text_utils_search_real (buffer, in_size, PDF_DELIM_PUT_TAG,
																 strlen (PDF_DELIM_PUT_TAG), TRUE);

	delim_readstring = gp_t1_get_previous_label (buffer, pos1);
	delim_def = gp_t1_get_previous_label (buffer, pos2);
	delim_put = gp_t1_get_previous_label (buffer, pos3);

	*delim_readstring_ = delim_readstring;
	*delim_put_ = delim_put;
	*delim_def_ = delim_def;
	
	return TRUE;
}

static gboolean
gp_t1_get_delimiters (GnomePrintPdfSubFont *sub_font, gint header_size)
{
	guchar *buffer;
	gchar *delim_readstring_1 = NULL;
	gchar *delim_def_1 = NULL;
	gchar *delim_put_1  = NULL;
	gchar *delim_readstring_2  = NULL;
	gchar *delim_def_2 = NULL;
	gchar *delim_put_2 = NULL;
	
	debug (FALSE, "");

	g_return_val_if_fail (sub_font != NULL, FALSE);

	buffer = sub_font->decrypted;

	gp_t1_get_delimiters_method_one (buffer, header_size,
																	 &delim_readstring_1,
																	 &delim_def_1,
																	 &delim_put_1);
	
#if 0
	g_print ("[%s*%s*%s]\n", delim_readstring_1, delim_def_1, delim_put_1);
#endif	

	gp_t1_get_delimiters_method_two (buffer, sub_font->decrypted_size,
																	 &delim_readstring_2,
																	 &delim_def_2,
																	 &delim_put_2);
	
#if 0
	g_print ("[%s*%s*%s]\n", delim_readstring_2, delim_def_2, delim_put_2);
#endif	

	g_return_val_if_fail (delim_readstring_1 != NULL, FALSE);
	g_return_val_if_fail (delim_readstring_2 != NULL, FALSE);

	if (delim_def_1 == NULL) {
		g_return_val_if_fail (delim_def_2 != NULL, FALSE);
		delim_def_1 = g_strdup (delim_def_2);
	}
	if (delim_def_2 == NULL) {
		g_return_val_if_fail (delim_def_1 != NULL, FALSE);
		delim_def_2 = g_strdup (delim_def_1);
	}
	if (delim_put_1 == NULL) {
		g_return_val_if_fail (delim_put_2 != NULL, FALSE);
		delim_put_1 = g_strdup (delim_put_2);
	}
	if (delim_put_2 == NULL) {
		g_return_val_if_fail (delim_put_1 != NULL, FALSE);
		delim_put_2 = g_strdup (delim_put_1);
	}

	if ((strcmp (delim_readstring_1, delim_readstring_2) != 0) ||
			(strcmp (delim_def_1, delim_def_2) != 0) ||
			(strcmp (delim_put_2, delim_put_2) != 0)) {
		g_warning ("The delimeters gathered by different methods, do not match\n");
		return FALSE;
	}

	g_print ("Delimeters [%s*%s*%s]\n", delim_readstring_1, delim_def_1, delim_put_1);

	g_free (delim_readstring_1);
	g_free (delim_def_1);
	g_free (delim_put_1);

	sub_font->delim_readstring = delim_readstring_2;
	sub_font->delim_def = delim_def_2;
	sub_font->delim_put = delim_put_2;
	
	return TRUE;
}

#define PDF_SUB_TAG "/Subrs"
#define PDF_CHARSTRING_TAG "/CharString"
/**
 * gnome_print_pdf_font_create_subfont:
 * @font: PdfFont structure that contains the arrays of glyphs to include
 * @body: body of the font (which is encrypted)
 * @body_length: length of the body
 * @length1: Length of the 1st section of the font, see [2]
 * @length2: ditto
 * @subfont_out: retrn the encrypted subfont here
 * 
 * Create a subfont from the already loaded "body", the PdfFont contains the arrays
 * of glyphs that we need to include
 *
 * Return Value: 
 **/
static gboolean
gnome_print_pdf_font_create_subfont (GnomePrintPdfFont *font,
																		 const gchar *body, gint body_length,
																		 gint length1, gint length2,
																		 gchar **subfont_out)
{
	GnomePrintPdfSubFont *sub_font;
	const gchar *crypted;
	gchar *buffer;
	gint buffer_size;
	gint n;
	gint pos1, pos2;

	g_return_val_if_fail (font != NULL, FALSE);
	
	sub_font = g_new (GnomePrintPdfSubFont, 1);
	sub_font->pdf_font = font;
	sub_font->delim_readstring = NULL;
	sub_font->delim_put = NULL;
	sub_font->delim_def = NULL;
	sub_font->decrypted = NULL;
	sub_font->header = NULL;
	sub_font->subrutines = NULL;
	sub_font->charstrings = NULL;
	sub_font->footer = NULL;
		
	crypted = body + length1;
	sub_font->decrypted = g_new (gchar, length2);
	sub_font->decrypted_size = decrypt_eexec (sub_font->decrypted, crypted, length2);
	
	pos1 = text_utils_search_real (sub_font->decrypted, sub_font->decrypted_size,
																 PDF_SUB_TAG, strlen (PDF_SUB_TAG), TRUE);
	pos2 = text_utils_search_real (sub_font->decrypted, sub_font->decrypted_size,
																 PDF_CHARSTRING_TAG, strlen (PDF_CHARSTRING_TAG), TRUE);

	if ((pos1 == -1) || (pos2 == -1)) {
		g_warning ("Tags not found inside the font\n");
		return FALSE;
	}

	/* Get the delimiters */
	if (!gp_t1_get_delimiters (sub_font, pos1)) {
		g_warning ("There was a problem in getting the delimiters from the font");
		return FALSE;
	}

	/* Create the subrutines array, which will hold a bank of gbooleans if the
		 subrutine needs to be added or not */
	buffer = sub_font->decrypted + pos1 + strlen (PDF_SUB_TAG);
	gp_t1_create_subs_array (sub_font, buffer);
	
	/* Read the glyphs */
	buffer = sub_font->decrypted + pos2;
	buffer_size = sub_font->decrypted_size - pos2;
	for (n = 0; n < font->glyphs_num; n++) {
		T1Glyph *glyph;
		glyph = &font->glyphs[n];
		gp_t1_read_glyph (buffer, buffer_size, glyph);
		gp_t1_load_subs_from_glyph (glyph, sub_font);
	}

	/* Now that we gathered the info from the font & glyphs, we need to create
		 the subfont_out buffer which will be composed of :

		 1. The Header of the original section without the "uniqueid"
		 identifier, since this font is different than the original. This is needed
		 because there can be cases in which a particular system (say a spooler)
		 can save the font using this Identifier for later use, so the next time
		 the font is downloaded, it looks for this ID and loads the previously
		 loaded font. This might be an exageration but I rather play it safe here.

		 2. The middle section of the font, We need to compose this section from
		 the header, the subrutines (which we only need to include the subrutines
		 that the chars are actually refering to) and the charstrings themselves.
		 After this section is composed, we need to crypt it again */

	if (!gp_t1_clean_header (sub_font, pos1)) {
		g_warning ("Can't create the header\n");
		return FALSE;
	}
	if (!gp_t1_create_subrutines (sub_font)) {
		g_warning ("Can't create the header\n");
		return FALSE;
	}
	
	
	*subfont_out = NULL;

#if 0	
	g_free (decrypted);
#endif	
	
	return TRUE;
}
#endif

/**
 * gnome_print_pdf_font_type1_embed:
 * @pc: Print Context to embed in
 * @font: Font to embed
 * 
 * Embeds in the print context a the type1 font
 *
 * Return Value: 0 on success, -1 on error
 **/
gint
gnome_print_pdf_font_type1_embed (GnomePrintContext *pc,
																	GnomePrintPdfFont *font)
{
	GnomePrintPdf *pdf;
	const GnomeFontFace *face;
	gchar *file_name;
	gchar *body;
#if 0 /* kill warning */
	gchar *subfont;
#endif	
	guint object_number;
	gint length;
	gint length1;
	gint length2;
	gint length3;
	gint ret = 0;
	gint written;
	gint body_length;
	gboolean ascii85 = FALSE;
	
	debug (FALSE, "");

	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (pc), -1);
	g_return_val_if_fail (font != NULL, -1);
	face = gnome_font_get_face (font->gnome_font);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -1);
	pdf = GNOME_PRINT_PDF (pc);
	g_return_val_if_fail (GNOME_IS_PRINT_PDF (pdf), -1);

	gtk_object_get (GTK_OBJECT (face), "pfb", &file_name, NULL);

	if (!file_name) return -1;
	if (!gp_t1_font_parse (file_name,
												 &body,
												 &length,
												 &length1,
												 &length2,
												 &length3,
												 &body_length)) {
		g_free (file_name);
		return -1;
	} else {
		g_free (file_name);
	}

#if 0	/* Disable subfont creation, we need to stabilize this for
			 Gnome 1.4. Chema */
	gnome_print_pdf_font_create_subfont (font, body, body_length,
																			 length1, length2, &subfont);
#endif	

	if ((length == 0) ||
			(length1 == 0)){
		g_warning ("Could not embed the font\n");
		return -1;
	}
	
	object_number = font->object_number_pfb;

	ret += gnome_print_pdf_object_start (pc, object_number);
	if (ascii85)
		ret += gnome_print_pdf_write  (pc,"/Filter /ASCII85Decode" EOL);
	ret += gnome_print_pdf_write  (pc,
																 "/Length %i" EOL
																 "/Length1 %i" EOL
																 "/Length2 %i" EOL
																 "/Length3 %i" EOL,
																 length,
																 length1,
																 length2,
																 length3);
	ret += gnome_print_pdf_write  (pc,
																 ">>" EOL
																 "stream" EOL);
	written = gnome_print_context_write_file (pc, body, body_length);
	gnome_print_pdf_add_bytes_written (pdf, written);
	ret += written;
	ret += gnome_print_pdf_write  (pc, EOL "endstream" EOL);
	ret += gnome_print_pdf_write  (pc, EOL "endobj" EOL);
	ret += gnome_print_pdf_object_end   (pc, object_number, TRUE);

	g_free (body);

	return ret;
}

/**
 * gp_t1_get_body_from_pfb:
 * @file_name: full path of the font
 * @body: return the body of the font here
 * @body_length: which has is of this length
 * 
 * get the body of the font (section 2 out of 3) unencrypted
 * 
 * Return Value: TRUE on success, FALSE otherwise
 **/
static gboolean
gp_t1_get_body_from_pfb (const gchar* file_name, gchar **body, gint * body_length)
{
	gchar *pfb;
	gchar *decrypted;
	const gchar *crypted;
	gint length;
	gint length1;
	gint length2;
	gint length3;
	gint pfb_length;
	gint size;

	*body = NULL;
	if (!gp_t1_font_parse (file_name,
												&pfb,
												&length,
												&length1,
												&length2,
												&length3,
												&pfb_length))
		return FALSE;


	crypted = pfb + length1;
	
	decrypted = g_new (gchar, length2);
	
	size = decrypt_eexec (decrypted, crypted, length2);

	g_free (pfb);

	*body_length = size;
	*body = decrypted;
	
	return TRUE;
}


/**
 * gp_t1_get_number_from_brackets:
 * @buffer: buffer pointing to a bracked value
 * @number: return the number here
 * 
 * given a pointer to a buffer like "[123]" returns 123 (a gint)
 * 
 * Return Value: TRUE on success, FALSE on error
 **/
static gboolean
gp_t1_get_number_from_brackets (gchar *buffer, gint *number)
{
	gint n;
	gchar *num;

	g_return_val_if_fail (buffer != NULL, FALSE);
	g_return_val_if_fail (buffer [0] == '[', FALSE);
	
	*number = 0;
	/* Limit the size for buffer overflows */
	num = g_new (gchar, 15);
	for (n = 0; n < 15; n++) {
		if (buffer [n+1] == ']')
			break;
		num [n] = buffer [n+1];
	}
	num [n] = 0;
	
	if (n > 13) {
		g_free (num);
		return FALSE;
	}

	*number = atoi (num);

	return TRUE;
}
		
#define GP_STEMV_TAG "/StdHW"
#define GP_STEMH_TAG "/StdVW"

/* TODO: Save the steam for each font in something like gnome_config,
	 so that we don't have to parse the font everytime we want to get
	 it's stems. */
/**
 * gnome_print_pdf_type1_get_stems:
 * @face: Font Face to get stems from 
 * @stemv: return the stemv here
 * @stemh: return the stemh here
 * 
 * This function parses the type1 font, and searches for the
 * "/StdHW [39] def" & "/StdVW [96] def" field, returns the numbers
 * inside.
 *
 * Return Value: TRUE on succes, FALSE otherwise
 **/
gint
gnome_print_pdf_type1_get_stems (const GnomeFontFace *face, gint *stemv, gint*stemh)
{
	gchar *file_name;
	gchar *body;
	gint body_size;
	gint pos;
	gint number;

	*stemv = 96; /* Use a default value */
	*stemh = 0; /* Ditto */
	
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), FALSE);

	gtk_object_get (GTK_OBJECT (face), "pfb", &file_name, NULL);
	if (!file_name) return FALSE;

	if (!gp_t1_get_body_from_pfb (file_name, &body, &body_size)) {
		g_warning ("Cant get body from pfb");
		g_free (file_name);
		return FALSE;
	} else {
		g_free (file_name);
	}

	/* Get the stemv */
	pos = text_utils_search_real (body, body_size,
																GP_STEMV_TAG, strlen (GP_STEMV_TAG),
																TRUE);
	if (pos == -1) {
		g_warning ("Could not find the \"%s\" tag", GP_STEMV_TAG);
		return FALSE;
	}
	if (!gp_t1_get_number_from_brackets (body + pos + strlen (GP_STEMV_TAG) + 1,
																			 &number)) {
		g_warning ("Could not get number from brackets");
		return FALSE;
	}
	*stemv = number;
			
	/* Get the stemh */
	pos = text_utils_search_real (body, body_size,
																GP_STEMH_TAG, strlen (GP_STEMH_TAG),
																TRUE);
	if (pos == -1) {
		g_warning ("Could not find the \"%s\" tag", GP_STEMH_TAG);
		return FALSE;
	}
	if (!gp_t1_get_number_from_brackets (body + pos + strlen (GP_STEMH_TAG) + 1,
																			 &number)) {
		g_warning ("Could not get number from brackets");
		return FALSE;
	}
	*stemh = number;

	g_free (body);
	
	return TRUE;
}
