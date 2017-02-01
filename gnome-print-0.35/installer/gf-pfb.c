#define _GF_PFB_C_

/*
 * fixme: We should relly do some parsing here
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <freetype/freetype.h>
#include "gf-pfb.h"

#define noVERBOSE

static guchar * gf_pfb_search_def_name (const guchar * b, gint length, const guchar * name);
static guchar * gf_pfb_search_def_string (const guchar * b, gint length, const guchar * name);
static gdouble gf_pfb_search_def_float (const guchar * b, gint length, const guchar * name);
static gint gf_pfb_search_def_int (const guchar * b, gint length, const guchar * name);
static gboolean gf_pfb_search_def_boolean (const guchar * b, gint length, const guchar * name);
static void gf_pfb_search_def_bbox (const guchar * b, gint length, const guchar * name, BBox * bbox);
static const guchar * gf_pfb_search_def (const guchar * b, gint length, const gchar * name);
static const guchar * gf_pfb_next_token (const guchar * b, const guchar * end);

extern FT_Library ft_library;

GFPFB *
gf_pfb_open (const gchar * name)
{
	GFPFB * pfb;
	gint fh;
	struct stat s;
	gint size;
	guchar *buf;
	guchar *f;
	gint length;
	gboolean ascii;
	FT_Face ft_face;
	FT_Error ft_result;
#ifdef VERBOSE
	gint i;
#endif

	g_return_val_if_fail (name != NULL, NULL);

	if (stat (name, &s) != 0) return NULL;
	/* I think no font can be < 4096 bytes */
	if (s.st_size < 8192) return NULL;
	/* We cannot use more than 64K anyways */
	size = MIN (s.st_size, 65536 + 6);

	fh = open (name, O_RDONLY);
	if (fh < 0) return NULL;

	buf = mmap (NULL, size, PROT_READ, MAP_SHARED, fh, 0);

	close (fh);

	if (!buf) return NULL;
	if (buf == (gpointer) -1) return NULL;

	if ((buf[0] == 0x80) && (buf[1] == 0x01)) {
		/* Seems to be pfb font */
		/* Well, we probably mess with some bytes */
		if (strncmp (buf + 6, "%!PS-AdobeFont-1.", 17)) {
			munmap (buf, size);
			return NULL;
		}
		f = buf + 6;
		length = 0x100 * buf[3] + buf[2];
		ascii = FALSE;
	} else {
		/* We do not support pfa fonts, because certain version of FT2 may crash on some */
		munmap (buf, size);
		return NULL;
	}

	ft_result = FT_New_Face (ft_library, name, 0, &ft_face);
	if (ft_result != FT_Err_Ok) return NULL;
	if (!FT_IS_SCALABLE (ft_face) ||
	    !ft_face->family_name) {
		FT_Done_Face (ft_face);
		return NULL;
	}
#ifdef VERBOSE
	g_print ("--- Face information for %s ---\n", name);
	g_print ("    Family name: %s\n", ft_face->family_name);
	g_print ("    Style name : %s\n", ft_face->family_name);
	g_print ("    Num glyphs : %d\n", (gint) ft_face->num_glyphs);

	ft_result = FT_Select_Charmap (ft_face, ft_encoding_unicode);
	g_print ("ccaron %x glyph code is %d\n", (gint) 0x010D, (gint) FT_Get_Char_Index (ft_face, 0x010D));
	for (i = 0; i < ft_face->num_glyphs; i++) {
		gchar c[256];
		FT_Get_Glyph_Name (ft_face, i, c, 256);
		if (!strcmp (c, "ccaron")) {
			g_print ("But glyph %x (%d) name is ccaron\n", i, i);
		}
	}
#endif
	FT_Done_Face (ft_face);

	pfb = g_new0 (GFPFB, 1);
	pfb->ascii = ascii;
	pfb->filename = g_strdup (name);

	/* Fill GlobalFontInfo */

	pfb->gfi.afmVersion = NULL;
	pfb->gfi.fontName = gf_pfb_search_def_name (f, length, "FontName");
	pfb->gfi.fullName = gf_pfb_search_def_string (f, length, "FullName");
	pfb->gfi.familyName = gf_pfb_search_def_string (f, length, "FamilyName");
	pfb->gfi.weight = gf_pfb_search_def_string (f, length, "Weight");
	pfb->gfi.italicAngle = gf_pfb_search_def_float (f, length, "ItalicAngle");
	pfb->gfi.isFixedPitch = gf_pfb_search_def_boolean (f, length, "isFixedPitch");
	gf_pfb_search_def_bbox (f, length, "FontBBox", &pfb->gfi.fontBBox);
	pfb->gfi.underlinePosition = gf_pfb_search_def_int (f, length, "UnderlinePosition");
	pfb->gfi.underlineThickness = gf_pfb_search_def_int (f, length, "UnderlineThickness");
	pfb->gfi.version = gf_pfb_search_def_string (f, length, "version");
	pfb->gfi.notice = gf_pfb_search_def_string (f, length, "Notice");
	pfb->gfi.encodingScheme = gf_pfb_search_def_string (f, length, "EncodingScheme");
	pfb->gfi.capHeight = gf_pfb_search_def_int (f, length, "CapHeight");
	pfb->gfi.xHeight = gf_pfb_search_def_int (f, length, "XHeight");
	pfb->gfi.ascender = gf_pfb_search_def_int (f, length, "Ascender");
	pfb->gfi.descender = gf_pfb_search_def_int (f, length, "Descender");

	munmap (buf, size);

	if ((!pfb->gfi.fontName) ||
	    (!pfb->gfi.fullName) ||
	    (!pfb->gfi.familyName)) {
		gf_pfb_close (pfb);
		return NULL;
	}

	return pfb;
}

void
gf_pfb_close (GFPFB * pfb)
{
	g_return_if_fail (pfb != NULL);

	if (pfb->filename) g_free (pfb->filename);
	if (pfb->gfi.afmVersion) g_free (pfb->gfi.afmVersion);
	if (pfb->gfi.fontName) g_free (pfb->gfi.fontName);
	if (pfb->gfi.fullName) g_free (pfb->gfi.fullName);
	if (pfb->gfi.familyName) g_free (pfb->gfi.familyName);
	if (pfb->gfi.weight) g_free (pfb->gfi.weight);
	if (pfb->gfi.version) g_free (pfb->gfi.version);
	if (pfb->gfi.notice) g_free (pfb->gfi.notice);
	if (pfb->gfi.encodingScheme) g_free (pfb->gfi.encodingScheme);
	g_free (pfb);
}

/* Checks StartFontMetrics string */
gboolean
gf_afm_check (const guchar *name)
{
	gint fh;
	struct stat s;
	guchar *buf;
	gboolean result;

	g_return_val_if_fail (name != NULL, FALSE);

	if (stat (name, &s) != 0) return FALSE;
	/* I think no afm can be < 256 bytes */
	if (s.st_size < 256) return FALSE;

	fh = open (name, O_RDONLY);
	if (fh < 0) return FALSE;

	buf = mmap (NULL, 256, PROT_READ, MAP_SHARED, fh, 0);

	close (fh);

	if (!buf) return FALSE;
	if (buf == (gpointer) -1) return FALSE;

	result = !strncmp (buf, "StartFontMetrics", 16);

	munmap (buf, 256);

	return result;
}

static guchar *
gf_pfb_search_def_name (const guchar * b, gint length, const guchar * name)
{
	const guchar * d, * n, * e, * end;
	guchar * new;

	end = b + length;

	d = gf_pfb_search_def (b, length, name);
	if (!d) return NULL;

	n = gf_pfb_next_token (d + strlen (name), end);
	if (!n) return NULL;

	if (*n++ != '/') return NULL;
	if (!isalpha (*n)) return NULL;

	for (e = n + 1; e < end; e++) {
		if (!isalnum (*e) && (*e != '-')) break;
	}

	new = g_new (guchar, e - n + 1);
	strncpy (new, n, e - n);
	*(new + (e - n)) = '\0';

	return new;
}

static guchar *
gf_pfb_search_def_string (const guchar * b, gint length, const guchar * name)
{
	const guchar *d, *n, *e, *end;
	gint level;
	guchar * new;

	end = b + length;

	d = gf_pfb_search_def (b, length, name);
	if (!d) return NULL;

	n = gf_pfb_next_token (d + strlen (name), end);
	if (!n) return NULL;

	if (*n++ != '(') return NULL;
	if (*n == ')') return NULL; /* fixme: maybe g_strdup ("") */
	if (iscntrl (*n)) return NULL;

	level = 1;
	for (e = n; e < end; e++) {
		if (*e == '(') {
			level += 1;
		} else if (*e == ')') {
			level -= 1;
			if (level < 1) break;
		} else if (iscntrl (*e)) {
			return NULL;
		}
	}

	new = g_new (guchar, e - n + 1);
	strncpy (new, n, e - n);
	*(new + (e - n)) = '\0';

	return new;
}

static gdouble
gf_pfb_search_def_float (const guchar * b, gint length, const guchar * name)
{
	const guchar *d, *n, *end;
	guchar c[32];

	end = b + length;

	d = gf_pfb_search_def (b, length, name);
	if (!d) return 0.0;

	n = gf_pfb_next_token (d + strlen (name), end);
	if (!n) return 0.0;

	n += 1;

	memcpy (c, n, MIN (end - n, 32));
	c[31] = 0;

	return atof (c);
}

static gint
gf_pfb_search_def_int (const guchar * b, gint length, const guchar * name)
{
	return 0;
}

static gboolean
gf_pfb_search_def_boolean (const guchar * b, gint length, const guchar * name)
{
	return FALSE;
}

static void
gf_pfb_search_def_bbox (const guchar * b, gint length, const guchar * name, BBox * bbox)
{
	bbox->llx = bbox->lly = 0;
	bbox->urx = bbox->ury = 1000;
}

static const guchar *
gf_pfb_search_def (const guchar * b, gint length, const gchar * name)
{
	const guchar * p, * end;
	gint nlen;

	nlen = strlen (name);

	end = b + length - nlen - 3; /* NB! We really want (len - 3) here */

	for (p = b; p < end; p++) {
		if (*p == '/') {
			if (*(p + 1) == '/') {
				p++;
			} else {
				if (strncmp (name, p + 1, nlen) == 0) {
					if (isspace(* (p + 1 + nlen))) return p + 1;
				}
			}
		}
	}

	return NULL;
}

static const guchar *
gf_pfb_next_token (const guchar * b, const guchar * end)
{
	const guchar * p;

	for (p = b; p < end; p++) {
		if (!isspace (*p)) return p;
	}

	return NULL;
}

#if 0
guchar *
gf_pfb_strdup (const guchar * token)
{
	guchar * p, * q;
	guchar * str;

	/* Fixme: This crashes for malformed pfb files */

	p = strchr (token, '(') + 1;
	q = strchr (p, ')');

	str = g_new0 (guchar, q - p + 1);
	memcpy (str, p, q - p);

	return str;
}
	
guchar *
gf_pfb_namedup (const guchar * token)
{
	guchar * p, * q;
	guchar * str;

	/* Fixme: This crashes for malformed pfb files */

	p = strchr (token, '/') + 1;
	q = p;
	while (*q > ' ') q++;

	str = g_new0 (guchar, q - p + 1);
	memcpy (str, p, q - p);

	return str;
}
	
const guchar *
gf_pfb_search (const guchar * buf, gint length, const guchar * text)
{
	gint textlen;
	const guchar * p;
	gint i;
	
	textlen = strlen (text);

	for (p = buf; p <= buf + length - textlen; p++) {
		if (*p == *text) {
			for (i = 1; i < textlen; i++) {
				if (p[i] != text[i]) break;
			}
			if (i == textlen) return p;
		}
	}

	return NULL;
}

const guchar *
gf_pfb_lower_search (const guchar * buf, gint length, const guchar * text) {
	gint textlen;
	const guchar * p;
	gint i;
	
	textlen = strlen (text);

	for (p = buf; p <= buf + length - textlen; p++) {
		if (tolower (*p) == *text) {
			for (i = 1; i < textlen; i++) {
				if (tolower (p[i]) != text[i]) break;
			}
			if (i == textlen) return p;
		}
	}

	return NULL;
}
#endif
