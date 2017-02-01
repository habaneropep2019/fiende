#define __GF_TTF_C__

/*                                                            
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
 * Authors :
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 */

/*
 * fixme: We should relly do some parsing here
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <freetype/freetype.h>

#include "gf-ttf.h"

static gchar *gf_ttf_weight_from_style (const gchar *style);
static gdouble gf_ttf_italic_from_style (const gchar *style);

extern FT_Library ft_library;

GFTTF *
gf_ttf_open (const gchar *name, gint face, const guchar *familyname, const guchar *stylename)
{
	GFTTF * ttf;
	gint fh;
	struct stat s;
	guchar buf[32];
	guint32 scaler;
	FT_Face ft_face;
	FT_Error ft_result;
	gint len, i;

	g_return_val_if_fail (name != NULL, NULL);

	if (stat (name, &s) != 0) return NULL;
	/* I think no font can be < 8192 bytes */
	if (s.st_size < 8192) return NULL;

	fh = open (name, O_RDONLY);
	if (fh < 0) return NULL;
	len = read (fh, buf, 32);
	close (fh);
	if (len != 32) return NULL;

	/* Read scaler type */
	/* fixme: More checks */
	scaler = GUINT32_FROM_BE (* (guint32 *) buf);

	if ((scaler != 0x74727565) && (scaler != 0x00010000) && (scaler != 0x74746366)) {
		/* Not a valid scaler type */
		return NULL;
	}

	ft_result = FT_New_Face (ft_library, name, face, &ft_face);
	if (ft_result != FT_Err_Ok) return NULL;
	ft_result = FT_Select_Charmap (ft_face, ft_encoding_unicode);
	if (ft_result != FT_Err_Ok) {
		FT_Done_Face (ft_face);
		return NULL;
	}

	/* Replace names, if needed */
	if (ft_face->family_name) familyname = ft_face->family_name;
	if (ft_face->style_name) stylename = ft_face->style_name;

	if (!FT_IS_SCALABLE (ft_face) ||
	    !FT_IS_SFNT (ft_face) ||
	    !familyname) {
		FT_Done_Face (ft_face);
		return NULL;
	}

	/* Hope it is valid TrueType file */

	ttf = g_new0 (GFTTF, 1);
	ttf->filename = g_strdup (name);
	ttf->num_faces = ft_face->num_faces;

	/* Fill GlobalFontInfo */

	ttf->gfi.afmVersion = NULL;
	if (stylename) {
		ttf->gfi.fontName = g_strdup_printf ("%s-%s", familyname, stylename);
		ttf->gfi.fullName = g_strdup_printf ("%s %s", familyname, stylename);
	} else {
		ttf->gfi.fontName = g_strdup (familyname);
		ttf->gfi.fullName = g_strdup (familyname);
	}
	for (i = 0; ttf->gfi.fontName[i]; i++) {
		if (ttf->gfi.fontName[i] == ' ') ttf->gfi.fontName[i] = '-';
	}
	ttf->gfi.familyName = g_strdup (familyname);
	ttf->gfi.isFixedPitch = FT_IS_FIXED_WIDTH (ft_face);
	ttf->gfi.fontBBox.llx = (gdouble) ft_face->bbox.xMin * 1000.0 / ft_face->units_per_EM;
	ttf->gfi.fontBBox.lly = (gdouble) ft_face->bbox.yMin * 1000.0 / ft_face->units_per_EM;
	ttf->gfi.fontBBox.urx = (gdouble) ft_face->bbox.xMax * 1000.0 / ft_face->units_per_EM;
	ttf->gfi.fontBBox.ury = (gdouble) ft_face->bbox.yMax * 1000.0 / ft_face->units_per_EM;

	ttf->gfi.underlinePosition = (gdouble) ft_face->underline_position * 1000.0 / ft_face->units_per_EM;
	ttf->gfi.underlineThickness = (gdouble) ft_face->underline_thickness * 1000.0 / ft_face->units_per_EM;
	ttf->gfi.version = g_strdup ("1.0");
	ttf->gfi.notice = NULL;
	ttf->gfi.encodingScheme = NULL;
	ttf->gfi.capHeight = 1000.0;
	ttf->gfi.xHeight = 500.0;
	ttf->gfi.ascender = (gdouble) ft_face->ascender * 1000.0 / ft_face->units_per_EM;
	ttf->gfi.descender = (gdouble) -ft_face->descender * 1000.0 / ft_face->units_per_EM;

	if (stylename) {
		ttf->gfi.weight = gf_ttf_weight_from_style (stylename);
		ttf->gfi.italicAngle = gf_ttf_italic_from_style (stylename);
	} else {
		if (ft_face->style_flags & FT_STYLE_FLAG_BOLD) {
			ttf->gfi.weight = g_strdup ("Bold");
		} else {
			ttf->gfi.weight = g_strdup ("Book");
		}
		if (ft_face->style_flags & FT_STYLE_FLAG_ITALIC) {
			ttf->gfi.italicAngle = -10.0;
		} else {
			ttf->gfi.italicAngle = 0.0;
		}
	}

	FT_Done_Face (ft_face);

	if ((!ttf->gfi.fontName) || (!ttf->gfi.fullName) || (!ttf->gfi.familyName)) {
		gf_ttf_close (ttf);
		return NULL;
	}

	return ttf;
}

void
gf_ttf_close (GFTTF * ttf)
{
	g_return_if_fail (ttf != NULL);

	if (ttf->filename) g_free (ttf->filename);
	if (ttf->gfi.afmVersion) g_free (ttf->gfi.afmVersion);
	if (ttf->gfi.fontName) g_free (ttf->gfi.fontName);
	if (ttf->gfi.fullName) g_free (ttf->gfi.fullName);
	if (ttf->gfi.familyName) g_free (ttf->gfi.familyName);
	if (ttf->gfi.weight) g_free (ttf->gfi.weight);
	if (ttf->gfi.version) g_free (ttf->gfi.version);
	if (ttf->gfi.notice) g_free (ttf->gfi.notice);
	if (ttf->gfi.encodingScheme) g_free (ttf->gfi.encodingScheme);
	g_free (ttf);
}

static gchar *
gf_ttf_weight_from_style (const gchar *style)
{
	gchar *weight;
	gchar *b;

	b = g_strdup (style);
	g_strdown (b);

	weight = NULL;

	if (strstr (b, "extralight")) {
		weight = g_strdup ("ExtraLight");
	} else if (strstr (b, "extra light")) {
		weight = g_strdup ("ExtraLight");
	} else if (strstr (b, "thin")) {
		weight = g_strdup ("Thin");
	} else if (strstr (b, "light")) {
		weight = g_strdup ("Light");
	} else if (strstr (b, "book")) {
		weight = g_strdup ("Book");
	} else if (strstr (b, "roman")) {
		weight = g_strdup ("Roman");
	} else if (strstr (b, "regular")) {
		weight = g_strdup ("Regular");
	} else if (strstr (b, "medium")) {
		weight = g_strdup ("Medium");
	} else if (strstr (b, "semi")) {
		weight = g_strdup ("Demi");
	} else if (strstr (b, "semibold")) {
		weight = g_strdup ("Demi");
	} else if (strstr (b, "demi")) {
		weight = g_strdup ("Demi");
	} else if (strstr (b, "demibold")) {
		weight = g_strdup ("Demi");
	} else if (strstr (b, "bold")) {
		weight = g_strdup ("Bold");
	} else if (strstr (b, "heavy")) {
		weight = g_strdup ("Heavy");
	} else if (strstr (b, "extrablack")) {
		weight = g_strdup ("ExtraBlack");
	} else if (strstr (b, "extra black")) {
		weight = g_strdup ("ExtraBlack");
	} else if (strstr (b, "heavy")) {
		weight = g_strdup ("Heavy");
	} else {
		weight = g_strdup ("Book");
	}

	g_free (b);

	return weight;
}

static gdouble
gf_ttf_italic_from_style (const gchar *style)
{
	gdouble italic;
	gchar *b;

	b = g_strdup (style);
	g_strdown (b);

	if (strstr (b, "italic")) {
		italic = -20.0;
	} else if (strstr (b, "oblique")) {
		italic = -20.0;
	} else {
		italic = 0.0;
	}

	g_free (b);

	return italic;
}
