#define __PARSE_TT_C__

/*
 * TrueType to Type1 converter
 *
 * Authors:
 *   Akira TAGOH <tagoh@redhat.com>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Please notice, that the actual code has very long copyright
 * owners list, and has advertisement clause. Read accompanying
 * source file for more information.
 *
 * Copyright (C) 2001 Akira Tagoh
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

/*
 * the most of TrueType to Type1 conversion code is based on ttf2pt1.
 * the copyright notice is here:
 *
 *  The following copyright notice applies to all the files provided
 *  in this distribution unless explicitly noted otherwise
 *  (the most notable exception being t1asm.c).
 *  
 *    Copyright (c) 1997-2001 by the AUTHORS:
 *    Andrew Weeks <ccsaw@bath.ac.uk> 
 *    Frank M. Siegert <fms@this.net> 
 *    Mark Heath <mheath@netspace.net.au> 
 *    Thomas Henlich <thenlich@rcs.urz.tu-dresden.de>
 *    Sergey Babkin <babkin@bellatlantic.net>, <sab123@hotmail.com>
 *    Turgut Uyar <uyar@cs.itu.edu.tr>
 *    Rihardas Hepas <rch@WriteMe.Com>
 *    Szalay Tamas <tomek@elender.hu>
 *    Johan Vromans <jvromans@squirrel.nl>
 *    Petr Titera <P.Titera@sh.cvut.cz>
 *    Lei Wang <lwang@amath8.amt.ac.cn>
 *    Chen Xiangyang <chenxy@sun.ihep.ac.cn>
 *    Zvezdan Petkovic <z.petkovic@computer.org>
 *   All rights reserved.
 *  
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. All advertising materials mentioning features or use of this software
 *      must display the following acknowledgement:
 *        This product includes software developed by the TTF2PT1 Project
 *        and its contributors.
 *  
 *   THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 *   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 *   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *   SUCH DAMAGE.
 * 
 * For the approximate list of the AUTHORS' responsibilities see the
 * project history.
 * 
 * Other contributions to the project are:
 * 
 * Turgut Uyar <uyar@cs.itu.edu.tr>
 *  The Unicode translation table for the Turkish language.
 * 
 * Rihardas Hepas <rch@WriteMe.Com>
 *  The Unicode translation table for the Baltic languages.
 * 
 * Szalay Tamas <tomek@elender.hu>
 *  The Unicode translation table for the Central European languages.
 *  
 * Johan Vromans <jvromans@squirrel.nl>
 *  The RPM file.
 * 
 * Petr Titera <P.Titera@sh.cvut.cz>
 *  The Unicode map format with names, the forced Unicode option.
 * 
 * Frank M. Siegert <frank@this.net>
 *  Port to Windows
 * 
 * Lei Wang <lwang@amath8.amt.ac.cn>
 * Chen Xiangyang <chenxy@sun.ihep.ac.cn>
 *  Translation maps for Chinese fonts.
 * 
 * Zvezdan Petkovic <z.petkovic@computer.org>
 *  The Unicode translation tables for the Cyrillic alphabet.
 * 
 * I. Lee Hetherington <ilh@lcs.mit.edu>
 *  The Type1 assembler (from the package 't1utils'), its full copyright
 *  notice:
 *   Copyright (c) 1992 by I. Lee Hetherington, all rights reserved.
 *   Permission is hereby granted to use, modify, and distribute this program
 *   for any purpose provided this copyright notice and the one below remain
 *   intact.
 *  
 */
#include <config.h>

#include <parseTT.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include "gp-ps-unicode.h"
#include "gp-unicode.h"
#if 0
#include "gt1-parset1.h"
#endif

#define FBIGVAL	(1e20)
#define FEPS	(100000./FBIGVAL)

#define MAX_STEMDEPTH 128
#define MAX_STEMS 2000
#define MAXBLUEWIDTH (24)
#define MAXHYST (2000)
#define HYSTBASE 500
#define NSTEMGRP 50					/* maximal number of the substituted stem groups */
#define MAXLEGALWIDTH 10000
#define LINESIZE 256

#define TYPE1_MARKER 128
#define TYPE1_ASCII 1
#define TYPE1_BINARY 2
#define TYPE1_DONE 3

#define GEF_FLOAT 0x02				/* entry contains floating point data */
#define GF_FLOAT 0x0002				/* thys glyph contains floating point entries */

/* front end */
#define CVDIR_FUP 0x02				/* goes over the line connecting the ends */
#define CVDIR_FEQUAL 0x01			/* coincides with the line connecting the ends */
#define CVDIR_FDOWN 0x00			/* goes under the line connecting the ends */
#define CVDIR_FRONT 0x0F			/* mask of all front directions */
/* rear end */
#define CVDIR_RSAME 0x30			/* is the same as for the front end */
#define CVDIR_RUP 0x20				/* goes over the line connecting the ends */
#define CVDIR_REQUAL 0x10			/* coincides with the line connecting the ends */
#define CVDIR_RDOWN 0x00			/* goes under the line connecting the ends */
#define CVDIR_REAR 0xF0				/* mask of all rear directions */

#define GE_HSBW	'B'
#define GE_MOVE 'M'
#define GE_LINE 'L'
#define GE_CURVE 'C'
#define GE_PATH 'P'

/* ordering of ST_END, ST_FLAT, ST_ZONE is IMPORTANT for sorting */
#define ST_END 0x01					/* end of line, lowest priority */
#define ST_FLAT 0x02				/* stem is defined by a flat line, not a curve */
#define ST_ZONE 0x04				/* pseudo-stem, the limit of a blue zone */
#define ST_UP 0x08					/* the black area is to up or right from value */
#define ST_3 0x20					/* first stem of [hv]stem3 */
#define ST_BLUE 0x40				/* stem is in blue zone */
#define ST_TOPZONE 0x80				/* 1 - top zone, 0 - bottom zone */
#define ST_VERT 0x100				/* vertical stem (used in substitutions) */

#define setbasestem(from, to)	(xbstem[0] = from, xbstem[1] = to, xblast = 1)
#define isbaseempty()			(xblast <= 0)
#define iscale(val)				(gint)(val > 0 ? scale_factor * val + 0.5 : scale_factor * val - 0.5)

struct glyph_face {
	struct glyph_face *next;
	gchar *filename;
	gint face;
	struct glyph *glyph;
	gint num_glyphs;

	gchar *copyright;
	gchar *family_name;
	gchar *style_name;
	gchar *full_name;
	gchar *version;
	gchar *ps_name;

	gdouble italic_angle;
	gshort underline_position;
	gshort underline_thickness;
	gshort is_fixed_pitch;
	gshort ascender;
	gshort descender;
	gushort units_per_em;
	gshort bbox[4];

	gint force_bold;
};

struct glyph {
	struct glyph *next;
	struct gentry *entries;
	struct gentry *lastentry;
	struct gentry *path;
	struct stem *hstems;			/* global horiz. and vert. stems */
	struct stem *vstems;
	struct stembounds *sbstems;		/* substituted stems for all the groups */
	gint char_code;				/* Encoding of glyph */
	gchar *name;					/* Postscript name of glyph */
	gint xMin, yMin, xMax, yMax;	/* values from TTF dictionary */
	gint lsb;						/* left sidebearing */
	gint ttf_pathlen;				/* total length of TTF paths */
	gint width;
	gint flags;
	gint scaledwidth;
	gint oldwidth;					/* actually also scaled */
	gint nhs, nvs;					/* numbers of stems */
	gshort *nsbs;					/* indexes of the group ends in the common array */
	gint nsg;						/* actual number of the stem groups */
	gint firstsubr;					/* first substituted stems subroutine number */
	gint rymin, rymax;				/* real values */
	gchar flatymin, flatymax;
};

#define bkwd cntr[0]
#define frwd cntr[1]
#define ipoints points.i.val
#define fpoints points.f.val
#define ixn ipoints[0]
#define iyn ipoints[1]
#define fxn fpoints[0]
#define fyn fpoints[1]
#define ix1 ixn[0]
#define ix2 ixn[1]
#define ix3 ixn[2]
#define iy1 iyn[0]
#define iy2 iyn[1]
#define iy3 iyn[2]
#define fx1 fxn[0]
#define fx2 fxn[1]
#define fx3 fxn[2]
#define fy1 fyn[0]
#define fy2 fyn[1]
#define fy3 fyn[2]

struct gentry {
	struct gentry *next;
	struct gentry *prev;
	struct gentry *cntr[2];			/* double-linked circular list */
	union {
		struct {
			int val[2][3];			/* integer values */
		} i;
		struct {
			double val[2][3];		/* floating values */
		} f;
	} points;
	gchar flags;
	guchar dir;						/* used to temporarily store the values for the directions of the ends of curves */
	gchar stemid;					/* connection to the substituted stem group */
	gchar type;
};

struct stem {
	gshort value;					/* value of X or Y coordinate */
	gshort origin;					/* point of origin for curve stems */
	struct gentry *ge;				/* entry that has (value, origin) as its first dot */
									/* also for all the stems the couple (value, origin)
									   is used to determine whether a stem is relevant for a
									   line, it's considered revelant if this tuple is
									   equal to any of the ends of the line.
									   ge is also used to resolve ambiguity if there is more than
									   one line going through certain pointi, it is used to
									   distinguish these lines.
									*/
	gshort from, to;				/* values of other coordinate between which this stem is valid */
	gshort flags;
};

struct stembounds {
	gshort low;						/* low bound */
	gshort high;					/* high bound */
	gchar isvert;					/* 1 - vertical, 0 - horizontal */
	gchar already;					/* temp. flag: is already included */
};

struct command {
	gchar *name;
	gint one, two;
};

struct ttf_dir_entry {
	gchar tag[4];
	gulong checksum;
	gulong offset;
	gulong length;
};

struct ttf_directory {
	gulong sfntVersion;
	gushort numTables;
	gushort searchRange;
	gushort entrySelector;
	gushort rangeShift;
	struct ttf_dir_entry list;
};

struct ttf_name_rec {
	gushort platformID;
	gushort encodingID;
	gushort languageID;
	gushort nameID;
	gushort stringLength;
	gushort stringOffset;
};

struct ttf_name {
	gushort format;
	gushort numberOfNameRecords;
	gushort offset;
	struct ttf_name_rec nameRecords;
};

static gint outl_moveto (FT_Vector *to, void *unused);
static gint outl_lineto (FT_Vector *to, void *unused);
static gint outl_conicto (FT_Vector *control1, FT_Vector *to, void *unused);
static gint outl_cubicto (FT_Vector *control1, FT_Vector *control2, FT_Vector *to, void *unused);
static void addgeafter (struct gentry *oge, struct gentry *nge);
static struct gentry *newgentry (gint flags);
static gdouble fclosegap (struct gentry *from, struct gentry *to, gint axis, gdouble gap, gdouble *ret);
static struct gentry *freethisge (struct gentry *ge);
static gint isign (gint x);
static gint fsign (gdouble x);
static gint fsqequation (gdouble a, gdouble b, gdouble c, gdouble *res, gdouble min, gdouble max);
static gint fiszigzag (struct gentry *ge);
static void fdelsmall (struct glyph *g, gdouble minlen);
static void fnormalizec (struct glyph *g);
static gint fcrossrays (struct gentry *ge1, struct gentry *ge2, gdouble *max1, gdouble *max2);
static gint fgetcvdir (struct gentry *ge);
static gint fckjoinedcv (struct glyph *g, gdouble t, struct gentry *nge, struct gentry *old1, struct gentry *old2, gdouble k);
static void fixendpath (struct gentry *ge);
static gint iround (gdouble val);
static void fixcvdir (struct gentry *ge, gint dir);
static void fixcvends (struct gentry *ge);
static gdouble fcvval (struct gentry *ge, gint axis, gdouble t);
static int bestblue (gshort *zhyst, gshort *physt, gshort *ozhyst, gint *bluetab);
static int addbluestems (struct stem *s, gint n);
static void sortstems (struct stem *s, gint n);
static void markbluestems (struct stem *s, gint nold);
static int joinmainstems (struct stem *s, gint nold, gint useblues);
static void joinsubstems (struct stem *s, gshort *pairs, gint nold, gint useblues);
static void uniformstems (struct stem *s, gshort *pairs, gint ns);
static void groupsubstems (struct glyph *g, struct stem *hs, gshort *hpairs, gint nhs, struct stem *vs, gshort *vpairs, gint nvs);
static gint besthyst (gint *hyst, gint base, gint *best, gint nbest, gint width, gint *bestindp);
static int stemoverlap (struct stem *s1, struct stem *s2);
static gint subfrombase (gint from, gint to);
static int gssentry (struct gentry *ge, struct stem *hs, gshort *hpairs, gint nhs, struct stem *vs, gshort *vpairs, gint nvs, struct stembounds *s, gshort *egp, gint *nextvsi, gint *nexthsi);
static int findstemat (gint value, gint origin, struct gentry *ge, struct stem *sp, gshort *pairs, gint ns, gint prevbest);
static void getline (gchar **src);
static gchar *eexec_start (gchar *line);
static gchar *eexec_string (gchar *string);
static const gchar *eexec_byte (guchar c);
static guchar eencrypt (guchar plain);
static const gchar *output_byte (guchar c);
static gchar *eexec_end (void);
static gchar *parse_charstring (gchar **src);
static void charstring_start (void);
static guchar cencrypt (guchar plain);
static gint is_integer (gchar *string);
static void charstring_int (gint num);
static void charstring_byte (gint v);
static gchar *charstring_end (void);
static gint command_compare (const void *key, const void *item);
#if 0
static gint unilist_compare (const void *key, const void *item);
#endif
void gclosepath (struct glyph *g);
void reversepaths (struct gentry *from, struct gentry *to);
void fclosepaths (struct glyph *g);
void ffixquadrants (struct glyph *g);
void fsplitzigzags (struct glyph *g);
void fforceconcise (struct glyph *g);
void fstraighten (struct glyph *g);
void fg_rmoveto (struct glyph *g, gdouble x, gdouble y);
void fg_rlineto (struct glyph *g, gdouble x, gdouble y);
void fg_rrcurveto (struct glyph *g, gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble x3, gdouble y3);
void pathtoint (struct glyph *g);
void flattencurves (struct glyph *g);
void findblues (struct glyph_face *gf);
void buildstems (struct glyph *g);
void stemstatistics (struct glyph *glyph);
void docorrectwidth(struct glyph_face *gf);
gint print_glyph_subs (gchar **retval, struct glyph *glyph, gint startid);
gchar *print_glyph (struct glyph *glyph);
gchar *rmoveto (gint dx, gint dy);
gchar *rlineto (gint dx, gint dy);
gchar *rrcurveto (gint dx1, gint dy1, gint dx2, gint dy2, gint dx3, gint dy3);
void ft_get_font_information (FT_Face face, gchar **copyright, gchar **family_name, gchar **style_name, gchar **full_name, gchar **version, gchar **ps_name);
gboolean ttf_get_font_information (const gchar *filename, gchar **copyright, gchar **family_name, gchar **style_name, gchar **full_name, gchar **version, gchar **ps_name);

void ttf_gentry_free (struct gentry *ge);
gchar *ttf_printf (gchar *src, const gchar *format, ...);
gchar *ttf_type1_dump (gchar *src);

struct glyph_face *glyph_cache = NULL;
static FT_Outline_Funcs ft_outline_funcs = {
	outl_moveto,
	outl_lineto,
	outl_conicto,
	outl_cubicto,
	0,
	0
};
static struct command command_table[] = {
	{"callothersubr", 12, 16},
	{"callsubr", 10, -1},
	{"closepath", 9, -1},
	{"div", 12, 12},
	{"dotsection", 12, 0},
	{"endchar", 14, -1},
	{"hlineto", 6, -1},
	{"hmoveto", 22, -1},
	{"hsbw", 13, -1},
	{"hstem", 1, -1},
	{"hstem3", 12, 2},
	{"hvcurveto", 31, -1},
	{"pop", 12, 17},
	{"return", 11, -1},
	{"rlineto", 5, -1},
	{"rmoveto", 21, -1},
	{"rrcurveto", 8, -1},
	{"sbw", 12, 7},
	{"seac", 12, 6},
	{"setcurrentpoint", 12, 33},
	{"vhcurveto", 30, -1},
	{"vlineto", 7, -1},
	{"vmoveto", 4, -1},
	{"vstem", 3, -1},
	{"vstem3", 12, 1},
};

static struct glyph *curg;
static gdouble scale_factor;
static gdouble lastx, lasty;
static gshort xbstem[MAX_STEMS * 2];
static gint xblast = -1;
static gint gssentry_lastgrp = 0;	/* reset to 0 for each new glyph */
static gchar cs_start[10];
static guchar charstring_buf[65535];
static guchar *charstring_bp;
static gchar line[LINESIZE];
static gint lenIV = 4;
static guint16 er, cr;
static guint16 c1 = 52845, c2 = 22719;
#if 0
static gint ttf_ref = 0;
static guint *unilist;
static guint *uniblist;
#endif
#if 0
static gint nunilist;
#endif
#if 0
static gchar *ttf_string = NULL;
#endif
static int hexcol = 0;

FT_Library ftlib = NULL;
FT_Error fterr = FT_Err_Ok;
gdouble italic_angle;
gint bbox[4];
gint bluevalues[14];
gint nblues;
gint notherb;
gint otherblues[10];
gint stdhw, stdvw;					/* dominant stems widths */
gint stemsnaph[12], stemsnapv[12];	/* most typical stem width */
gint active = 0;
gint start_charstring = 0;
gint in_eexec = 0;

/* We do metrics and info parsing directly from GnomeFontFace (lauris) */
/* It is better to have it there, as we share the same code for Type1 as well */
#if 0
gint
parseTT (const char *path, gint fn, Font_Info **fi, FLAGS flags)
{
	FT_Face face;
	FT_UInt i, glyph_index;
	size_t len;

	fterr = FT_Err_Ok;

	(*fi) = (Font_Info *) calloc (MAX_NAME, sizeof (Font_Info));
	if ((*fi) == NULL) {
		fterr = FT_Err_Out_Of_Memory;
		return fterr;
	}
	if (flags & P_G) {
		(*fi)->gfi = (GlobalFontInfo *) calloc (1, sizeof (GlobalFontInfo));
		if ((*fi)->gfi == NULL) {
			fterr = FT_Err_Out_Of_Memory;
			return fterr;
		}
	}

	if (ftlib == NULL) ttf_cache_init ();
	if ((fterr = FT_New_Face (ftlib, path, fn, &face)) == FT_Err_Unknown_File_Format) {
		g_warning ("(%s)Unknown file format. -- '%s'\n", G_GNUC_FUNCTION, path);

		return fterr;
	} else if (fterr) {
		g_warning ("(%s)FreeType error. -- %d\n", G_GNUC_FUNCTION, fterr);

		return fterr;
	}
	if (!ttf_get_font_information (path, &(*fi)->gfi->notice, &(*fi)->gfi->familyName, &(*fi)->gfi->weight, &(*fi)->gfi->fullName, &(*fi)->gfi->version, &(*fi)->gfi->fontName)) {
		ft_get_font_information (face, &(*fi)->gfi->notice, &(*fi)->gfi->familyName, &(*fi)->gfi->weight, &(*fi)->gfi->fullName, &(*fi)->gfi->version, &(*fi)->gfi->fontName);
	}
	(*fi)->gfi->afmVersion = g_strdup ("0.0"); /* FIXME: ttf doesn't have this parameter. */
	(*fi)->gfi->italicAngle = 0.0; /* FIXME: ttf doesn't have this parameter. */
	(*fi)->gfi->isFixedPitch = FALSE; /* FIXME: ttf doesn't have this parameter. */
	(*fi)->gfi->fontBBox.llx = face->bbox.xMin;
	(*fi)->gfi->fontBBox.lly = face->bbox.yMin;
	(*fi)->gfi->fontBBox.urx = face->bbox.xMax;
	(*fi)->gfi->fontBBox.ury = face->bbox.yMax;
	(*fi)->gfi->underlinePosition = face->underline_position;
	(*fi)->gfi->underlineThickness = face->underline_thickness;
	(*fi)->gfi->encodingScheme = NULL; /* FIXME: ttf doesn't have this parameter. */
	(*fi)->gfi->capHeight = face->height;
	(*fi)->gfi->xHeight = face->height; /* FIXME: ??? */
	(*fi)->gfi->ascender = face->ascender;
	(*fi)->gfi->descender = face->descender;
	(*fi)->numOfChars = face->num_glyphs; /* FIXME: we don't load all of glyph */

	/* always load the ASCII characters */
	for (i = 0; i < 256; i++) {
		glyph_index = FT_Get_Char_Index (face, i);
		gp_glyph_index_insert (glyph_index, i);
		gp_unicode_index_insert (i, glyph_index);
		ttf_load_glyph (face, path, fn, glyph_index);
	}
	if (ttf_has_additional_string ()) ttf_set_additional_list ();
	if ((len = ttf_get_additional_list_length ()) > 0) {
		gint c;

		for (c = 0; c < len; c++) {
			glyph_index = FT_Get_Char_Index (face, uniblist[c]);
			gp_glyph_index_insert (glyph_index, uniblist[c]);
			gp_unicode_index_insert (uniblist[c], glyph_index);
			ttf_load_glyph (face, path, fn, glyph_index);
		}
	}

	if (flags & (P_M ^ P_W)) {
		(*fi)->cmi = (CharMetricInfo *)calloc ((*fi)->numOfChars, sizeof (CharMetricInfo));
		if ((*fi)->cmi == NULL) {
			fterr = FT_Err_Out_Of_Memory;
			return fterr;
		}
		for (i = 0; i < (*fi)->numOfChars; i++) {
			memset (&(*fi)->cmi[i], 0, sizeof (CharMetricInfo));
		}
			
		/* always load the ASCII characters */
		for (i = 0; i < 256; i++) {
			glyph_index = FT_Get_Char_Index (face, i);
			gp_glyph_index_insert (glyph_index, i);
			gp_unicode_index_insert (i, glyph_index);
			ttf_load_metric (face, fi, glyph_index);
		}

		if ((len = ttf_get_additional_list_length ()) > 0) {
			gint c;

			for (c = 0; c < len; c++) {
				glyph_index = FT_Get_Char_Index (face, uniblist[c]);
				gp_glyph_index_insert (glyph_index, uniblist[c]);
				gp_unicode_index_insert (uniblist[c], glyph_index);
				ttf_load_metric (face, fi, glyph_index);
			}
		}
	}
	else {
		if (flags & P_W) {
			/* width */
			/* FIXME: doesn't support flag P_W for freetype (yet?) */
			fterr = FT_Err_Out_Of_Memory;
			return fterr;
		}
	}

	if (flags & P_T) {
		/* kerning tracks */
		/* FIXME: doesn't support flag P_T for freetype (yet?) */
		(*fi)->numOfTracks = 0;
		(*fi)->tkd = NULL;
	}
	if (flags & P_P) {
		/* kerning pairs */
		/* FIXME: doesn't support flag P_P for freetype (yet?) */
		(*fi)->numOfPairs = 0;
		(*fi)->pkd = NULL;
	}
	if (flags & P_C) {
		/* FIXME: doesn't support flag P_C for freetype (yet?) */
		(*fi)->numOfComps = 0;
		(*fi)->ccd = NULL;
	}
	FT_Done_Face (face);

	return fterr;
}
#endif

#if 0
void
ttf_cache_init (void)
{
	/* Initialize freetype2 library */
	if (ttf_ref == 0) {
		glyph_cache = NULL;
		fterr = FT_Init_FreeType (&ftlib);
		if (fterr) g_error ("an error occurred during library initialization.\n");
	}
	ttf_ref++;
}
#endif

#if 0
void
ttf_cache_destroy (void)
{
	struct glyph_face *g = glyph_cache;
	struct glyph *gl, *glb;

	if ((--ttf_ref) <= 0) {
		while (g != NULL) {
			gl = g->glyph;
			while (gl != NULL) {
				ttf_gentry_free (gl->entries);
				ttf_gentry_free (gl->lastentry);
				ttf_gentry_free (gl->path);
				if (gl->name) g_free (gl->name);
				glb = gl->next;
				g_free (gl);
				gl = glb;
			}
			if (g->filename) g_free (g->filename);
			if (g->copyright) g_free (g->copyright);
			if (g->family_name) g_free (g->family_name);
			if (g->style_name) g_free (g->style_name);
			if (g->full_name) g_free (g->full_name);
			if (g->version) g_free (g->version);
			if (g->ps_name) g_free (g->ps_name);

			g = g->next;
		}
		glyph_cache = NULL;
		if (unilist != NULL) {
			g_free (unilist);
			unilist = NULL;
		}
		if (uniblist != NULL) {
			g_free (uniblist);
			uniblist = NULL;
		}

		FT_Done_FreeType (ftlib);
		ftlib = NULL;
		ttf_ref = 0;
	}
}
#endif

#if 0
void
ttf_gentry_free (struct gentry *ge)
{
	struct gentry *g;

	while (ge != NULL) {
		g = ge->next;
		g_free (ge);
		ge = g;
	}
}
#endif

#if 0
gint
ttf_load_metric (FT_Face face, Font_Info **fi, guint32 glyph_index)
{
	gchar *name;
	FT_UInt unicode = gp_unicode_from_glyph_index (glyph_index);

	name = gp_ps_from_unicode (unicode);
	if (glyph_index > (*fi)->numOfChars) {
		g_warning ("no keep memory\n");
	}
	if (glyph_index != 0) {
		FT_Glyph glyph;
		FT_BBox bbox;

		if (name != NULL) (*fi)->cmi[glyph_index].name = g_strdup (name);
		else (*fi)->cmi[glyph_index].name = g_strdup (".notdef");
		if ((fterr = FT_Set_Pixel_Sizes (face, 8, 8)) != FT_Err_Ok) {
			g_warning ("can't set the pixcel size -- 0x%X\n", fterr);
			return fterr;
		}
		if ((fterr = FT_Load_Glyph (face, glyph_index, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP)) != FT_Err_Ok) {
			g_warning ("can't load a glyph -- 0x%X\n", fterr);
			return fterr;
		}
		if ((fterr = FT_Get_Glyph (face->glyph, &glyph)) != FT_Err_Ok) {
			g_warning ("can't get a glyph -- 0x%X\n", fterr);
			return fterr;
		}
		FT_Glyph_Get_CBox (glyph, ft_glyph_bbox_unscaled, &bbox);
		(*fi)->cmi[glyph_index].code = unicode;
		(*fi)->cmi[glyph_index].wx = bbox.xMax + face->glyph->advance.x; /* bbox.xMax * (x0 - x1); */
		(*fi)->cmi[glyph_index].wy = 0;
		(*fi)->cmi[glyph_index].charBBox.llx = bbox.xMin;
		(*fi)->cmi[glyph_index].charBBox.lly = bbox.yMin;
		(*fi)->cmi[glyph_index].charBBox.urx = bbox.xMax;
		(*fi)->cmi[glyph_index].charBBox.ury = bbox.yMax;
		(*fi)->cmi[glyph_index].ligs = NULL;
	} else {
		(*fi)->cmi[glyph_index].code = 0;
		(*fi)->cmi[glyph_index].wx = 0;
		(*fi)->cmi[glyph_index].wy = 0;
		(*fi)->cmi[glyph_index].charBBox.llx = 0;
		(*fi)->cmi[glyph_index].charBBox.lly = 0;
		(*fi)->cmi[glyph_index].charBBox.urx = 0;
		(*fi)->cmi[glyph_index].charBBox.ury = 0;
		(*fi)->cmi[glyph_index].ligs = NULL;
	}

	return FT_Err_Ok;
}
#endif

#if 0
gint
ttf_load_glyph (FT_Face face, const gchar *filename, gint fn, guint32 glyph_index)
{
	FT_Glyph_Metrics *met;
	FT_BBox bbox;
	FT_Glyph glyph;
	FT_Outline *outline;
	struct glyph_face **g, *g_prev = NULL;
	struct glyph *gl = NULL, *gl_prev = NULL;
	gchar *name, *names[3], *str;
	guint32 unicode = gp_unicode_from_glyph_index (glyph_index);
	int i, j;
	gboolean initialized = FALSE;

	g = &glyph_cache;

	for (i = 0; i < face->num_charmaps; i++) {
		if (face->charmaps[i]->platform_id == 3) {
			if ((fterr = FT_Set_Charmap (face, face->charmaps[i])) != FT_Err_Ok) {
				g_warning ("(%s)Cannot set charmap in freetype\n", G_GNUC_FUNCTION);
				return fterr;
			}
			break;
		}
	}
	while ((*g) != NULL) {
		if (!strcmp ((*g)->filename, filename) && fn == (*g)->face) break;
		g_prev = (*g);
		(*g) = (*g)->next;
	}
	if ((*g) == NULL) {
		(*g) = g_prev;
		(*g) = g_new0 (struct glyph_face, 1);
		(*g)->filename = g_strdup (filename);
		(*g)->face = fn;
		(*g)->next = NULL;
		(*g)->glyph = NULL;
	} else {
		initialized = TRUE;
	}

	if (!initialized) {
		(*g)->num_glyphs = face->num_glyphs;
		(*g)->italic_angle = 0.0;
		(*g)->underline_position = face->underline_position;
		(*g)->underline_thickness = face->underline_thickness;
		(*g)->is_fixed_pitch = FT_IS_FIXED_WIDTH (face);
		(*g)->ascender = face->ascender;
		(*g)->descender = face->descender;
		(*g)->units_per_em = face->units_per_EM;
		(*g)->bbox[0] = face->bbox.xMin;
		(*g)->bbox[1] = face->bbox.yMin;
		(*g)->bbox[2] = face->bbox.xMax;
		(*g)->bbox[3] = face->bbox.yMax;

		if (!ttf_get_font_information (filename, &(*g)->copyright, &(*g)->family_name, &(*g)->style_name, &(*g)->full_name, &(*g)->version, &(*g)->ps_name)) {
			ft_get_font_information (face, &(*g)->copyright, &(*g)->family_name, &(*g)->style_name, &(*g)->full_name, &(*g)->version, &(*g)->ps_name);
		}
		(*g)->force_bold = 0;

		names[0] = (*g)->style_name;
		names[1] = (*g)->full_name;
		names[2] = (*g)->ps_name;

		for (i = 0; i < sizeof (names) / sizeof (names[0]); i++) {
			str = names[i];
			for (j = 0; str[j] != 0; j++) {
				if (((str[j] == 'B' || str[j] == 'b') &&
					 (j == 0 || !isalpha (str[j - 1]))) &&
					!strncmp ("old", &str[j + 1], 3) &&
					!islower (str[j+4])) {
					(*g)->force_bold = 1;
					break;
				}
			}
		}
	}

	if ((*g)->glyph == NULL) {
		(*g)->glyph = g_new0 (struct glyph, 1);
		gl = (*g)->glyph;
		gl->next = NULL;
	} else {
		for (gl = (*g)->glyph, gl_prev = gl; gl != NULL; gl_prev = gl, gl = gl->next) {
			if (gl->char_code == unicode) return FT_Err_Ok;
		}
		gl = gl_prev;
		gl->next = g_new0 (struct glyph, 1);
		gl = gl->next;
		gl->next = NULL;
	}

	name = gp_ps_from_unicode (unicode);
	if (name != NULL) gl->name = g_strdup (name);
	else gl->name = g_strdup_printf ("_%d", unicode);
	if ((fterr = FT_Load_Glyph (face, glyph_index, FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE)) != FT_Err_Ok) {
			g_warning ("Can't load glyph -- %d\n", unicode);
			return -1;
	}
	met = &face->glyph->metrics;
	if (FT_HAS_HORIZONTAL (face)) {
		gl->width = met->horiAdvance;
		gl->lsb = met->horiBearingX;
	} else {
		gl->width = met->width;
		gl->lsb = 0;
	}

	if ((fterr = FT_Get_Glyph (face->glyph, &glyph)) != FT_Err_Ok) {
		g_warning ("Can't access glyph -- %d\n", unicode);
		return -1;
	}
	FT_Glyph_Get_CBox (glyph, ft_glyph_bbox_unscaled, &bbox);
	gl->entries = NULL;
	gl->lastentry = NULL;
	gl->path = NULL;
	gl->hstems = NULL;
	gl->vstems = NULL;
	gl->sbstems = NULL;
	gl->char_code = unicode;
	gl->xMin = bbox.xMin;
	gl->yMin = bbox.yMin;
	gl->xMax = bbox.xMax;
	gl->yMax = bbox.yMax;
	gl->ttf_pathlen = face->glyph->outline.n_points;
	gl->flags = 0;

	scale_factor = 1000.0 / (gdouble)(*g)->units_per_em;
	gl->scaledwidth = iscale (gl->width);
	gl->oldwidth = 0;
	gl->nhs = 0;
	gl->nvs = 0;
	gl->nsbs = NULL;
	gl->nsg = 0;
	gl->firstsubr = 0;
	gl->rymin = 0;
	gl->rymax = 0;
	gl->flatymin = 0;
	gl->flatymax = 0;

	if (gl->ttf_pathlen != 0) {
		curg = gl;
		outline = &face->glyph->outline;
		if ((fterr = FT_Outline_Decompose (outline, &ft_outline_funcs, NULL)) != FT_Err_Ok) {
			g_warning ("Can't decompose outline of glyph -- %d\n", unicode);
			return -1;
		}
		if (curg->lastentry) gclosepath (curg);
		if (outline->flags & ft_outline_reverse_fill)
			reversepaths (curg->entries, NULL);

		gl->lastentry = NULL;
		fclosepaths (gl);

		/* float processing */
		ffixquadrants (gl);
		fsplitzigzags (gl);
		fforceconcise (gl);
		fstraighten (gl);

		pathtoint (gl);

		/* int processing */
/*		smoothjoints (gl); */
		flattencurves (gl);
	} else {
		gl->flags &= ~GF_FLOAT;
	}

	return FT_Err_Ok;
}
#endif

#if 0
gboolean
ttf_has_additional_string (void)
{
	if (ttf_string != NULL) return TRUE;

	return FALSE;
}
#endif

#if 0
gint
ttf_get_additional_list_length (void)
{
	return nunilist;
}
#endif

#if 0
guint
ttf_get_additional_list_value (guint index)
{
	g_return_val_if_fail (index <= nunilist, 0);

	return uniblist[index];
}
#endif

#if 0
gint
ttf_set_additional_string (gchar *string)
{
	size_t len;
	gchar *buf;
	gint retval = 0;

	if (string == NULL || strlen (string) == 0) return 0;
	if (ttf_string != NULL) {
		len = strlen (ttf_string);
		ttf_string = g_renew (gchar, ttf_string, sizeof (gchar) * len + strlen (string) + 1);
		ttf_string[len] = 0;
		strcat (ttf_string, string);
		g_free (string);
	} else {
		ttf_string = string;
	}
	buf = g_strdup (ttf_string);
	retval = ttf_set_additional_list ();
	ttf_string = buf;

	return retval;
}
#endif

#if 0
gint
ttf_set_additional_list (void)
{
	gchar *p;
	size_t len = strlen (ttf_string);
	gint cnt = ttf_get_additional_list_length ();
	gint update = 0;

	unilist = g_renew (guint, unilist, cnt + len + 1);
	uniblist = g_renew (guint, uniblist, cnt + len + 1);
	for (p = ttf_string; p && p < (ttf_string + len); p = g_utf8_next_char (p)) {
		guint unival;

		unival = g_utf8_get_char (p);
		if (unival < 256) continue;
		if (cnt != 0) {
			gint *l;

			l = (gint *)bsearch ((void *)&unival, (void *)unilist, cnt, sizeof (gint), unilist_compare);
			if (!l) {
				if (unilist[cnt - 1] > unival) {
					gint i = cnt - 1;

					while (unilist[i] > unival && i >= 0) {
						unilist[i + 1] = unilist[i];
						unilist[i] = unival;
						i--;
					}
				} else {
					unilist[cnt] = unival;
				}
				uniblist[cnt++] = unival;
				update++;
			}
		} else {
			unilist[cnt] = unival;
			uniblist[cnt++] = unival;
			update++;
		}
	}
	nunilist = cnt;
	g_free (ttf_string);
	ttf_string = NULL;

	return update;
}
#endif

gchar *
ttf_printf (gchar *src, const gchar *format, ...)
{
	gchar buffer[65536];
	gchar *retval;
	va_list args;

	va_start (args, format);
	g_vsnprintf (buffer, 65535, format, args);
	if (src != NULL) {
		retval = g_new0 (gchar, strlen (src) + strlen (buffer) + 1);
		strcpy (retval, src);
		strcat (retval, buffer);
		g_free (src);
	} else {
		retval = g_strdup (buffer);
	}
	va_end (args);

	return retval;
}

static void
ttf_glyph_face_free (struct glyph_face *g)
{
	g_warning ("Implement tt_glyph_face_free");
}

/*
 * So this is the main thing
 *
 * I'll make it one-shot, and decide later, where and how to cache, if needed (Lauris)
 *
 * I also do not like return type (guchar *), but OK (Lauris)
 */

guchar *
ttf2pfa (FT_Face ft_face, const guchar *embeddedname, guint32 *glyphmask)
{
	struct glyph_face *g;
	gint i;
	/* fixme: Clean this up (Lauris) */
	struct glyph *gi;
	gchar *retval, *buf, *data, fixbuf[65536];
	gint nchars = 0, subid, j;

	/* Step 0 - load glyph face */
	g = g_new0 (struct glyph_face, 1);
	/* fixme: Double check these */
	g->filename = g_strdup ("nofile");
	g->face = ft_face->face_index;
	g->next = NULL;
	g->glyph = NULL;

	g->num_glyphs = ft_face->num_glyphs;
	g->italic_angle = 0.0;
	g->underline_position = ft_face->underline_position;
	g->underline_thickness = ft_face->underline_thickness;
	g->is_fixed_pitch = FT_IS_FIXED_WIDTH (ft_face);
	g->ascender = ft_face->ascender;
	g->descender = ft_face->descender;
	g->units_per_em = ft_face->units_per_EM;
	g->bbox[0] = ft_face->bbox.xMin;
	g->bbox[1] = ft_face->bbox.yMin;
	g->bbox[2] = ft_face->bbox.xMax;
	g->bbox[3] = ft_face->bbox.yMax;

	/* Font information */
	ft_get_font_information (ft_face, &g->copyright, &g->family_name, &g->style_name, &g->full_name, &g->version, &g->ps_name);

	/* fixme: If needed, this should be set by GnomeFontFace */
	g->force_bold = FALSE;

	/* Step 1 - populate used glyphs */
	for (i = 0; i < ft_face->num_glyphs; i++) {
		if (!i || (glyphmask[i >> 5] & (1 << (i & 0x1f)))) {
			struct glyph *gl;
			FT_Error ft_result;
			FT_Glyph glyph;
			FT_BBox bbox;
			FT_Outline *outline;
			/* Glyph is marked */
			gl = g_new0 (struct glyph, 1);
			gl->next = g->glyph;
			g->glyph = gl;
			/* fixme: Does this work or do we need unicode somewhere? (Lauris) */
			gl->name = (i) ? g_strdup_printf ("_%d", i) : g_strdup (".notdef");
			ft_result = FT_Load_Glyph (ft_face, i, FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
			if (ft_result != FT_Err_Ok) {
				g_warning ("file %s: Line %d: Cannot load glyph %d", __FILE__, __LINE__, i);
				ttf_glyph_face_free (g);
				return NULL;
			}
			/* fixme: (Lauris) */
			if (FT_HAS_HORIZONTAL (ft_face)) {
				gl->width = ft_face->glyph->metrics.horiAdvance;
				gl->lsb = ft_face->glyph->metrics.horiBearingX;
			} else {
				gl->width = ft_face->glyph->metrics.width;
				gl->lsb = 0;
			}
			ft_result = FT_Get_Glyph (ft_face->glyph, &glyph);
			if (ft_result != FT_Err_Ok) {
				g_warning ("file %s: Line %d: Cannot get glyph %d", __FILE__, __LINE__, i);
				ttf_glyph_face_free (g);
				return NULL;
			}
			FT_Glyph_Get_CBox (glyph, ft_glyph_bbox_unscaled, &bbox);
			gl->entries = NULL;
			gl->lastentry = NULL;
			gl->path = NULL;
			gl->hstems = NULL;
			gl->vstems = NULL;
			gl->sbstems = NULL;
			/* fixme: Here we are messing again */
			gl->char_code = i;
			gl->xMin = bbox.xMin;
			gl->yMin = bbox.yMin;
			gl->xMax = bbox.xMax;
			gl->yMax = bbox.yMax;
			gl->ttf_pathlen = ft_face->glyph->outline.n_points;
			gl->flags = 0;

			scale_factor = 1000.0 / (gdouble) g->units_per_em;
			gl->scaledwidth = iscale (gl->width);
			gl->oldwidth = 0;
			gl->nhs = 0;
			gl->nvs = 0;
			gl->nsbs = NULL;
			gl->nsg = 0;
			gl->firstsubr = 0;
			gl->rymin = 0;
			gl->rymax = 0;
			gl->flatymin = 0;
			gl->flatymax = 0;

			if (gl->ttf_pathlen != 0) {
				curg = gl;
				outline = &ft_face->glyph->outline;
				ft_result = FT_Outline_Decompose (outline, &ft_outline_funcs, NULL);
				if (ft_result != FT_Err_Ok) {
					g_warning ("file %s: Line %d: Cannot decompose outline %d", __FILE__, __LINE__, i);
					ttf_glyph_face_free (g);
					return NULL;
				}
				if (curg->lastentry) gclosepath (curg);
				if (outline->flags & ft_outline_reverse_fill) {
					reversepaths (curg->entries, NULL);
				}
				
				gl->lastentry = NULL;
				fclosepaths (gl);
				
				/* float processing */
				ffixquadrants (gl);
				fsplitzigzags (gl);
				fforceconcise (gl);
				fstraighten (gl);

				pathtoint (gl);

				/* int processing */
				/* smoothjoints (gl); */
				flattencurves (gl);
			} else {
				gl->flags &= ~GF_FLOAT;
			}
		}
	}

	/* According to my best knowledge, we should now have identical structure to glyph cache */

	italic_angle = g->italic_angle;
	if (italic_angle > 45.0 || italic_angle < -45.0) italic_angle = 0.0;

	findblues (g);
	for (gi = g->glyph; gi != NULL; gi = gi->next) {
		buildstems (gi);
	}
	stemstatistics (g->glyph);
	docorrectwidth (g);

	retval = g_strdup_printf ("%%!PS-AdobeFont-1.0: %s\n" \
				  "12 dict begin\n" \
				  "/FontInfo 9 dict dup begin\n" \
				  "/version (%s) readonly def\n" \
				  "/Notice (%s) readonly def\n" \
				  "/FullName (%s) readonly def\n" \
				  "/FamilyName (%s) readonly def\n" \
				  "/Weight (%s) readonly def\n" \
				  "/ItalicAngle %f def\n" \
				  "/isFixedPitch %s def\n" \
				  "/UnderlinePosition %d def\n" \
				  "/UnderlineThickness %hd def\n" \
				  "end readonly def\n" \
				  "/FontName /%s def\n" \
				  "/PaintType 0 def\n" \
				  "/StrokeWidth 0 def\n" \
				  "/FontType 1 def\n" \
				  "/FontMatrix [0.001 0 0 0.001 0 0] def\n" \
				  "/FontBBox {%d %d %d %d} readonly def\n" \
				  "/Encoding 256 array\n",
				  embeddedname,
				  "0.0",
				  g->copyright, g->full_name, g->family_name,
				  "Book", /* fixme: */
				  g->italic_angle,
				  g->is_fixed_pitch ? "true" : "false",
				  iscale (g->underline_position),
				  iscale (g->underline_thickness),
				  embeddedname,
				  bbox[0], bbox[1], bbox[2], bbox[3]);

#if 0
	/* determine number of elements for metrics table */
	for (i = 0, j = 0; i < 256; i++) {
		g_snprintf (&fixbuf[j], 65535 - j, "dup %d /%s put\n", i, gp_ps_from_unicode (i));
		j = strlen (fixbuf);
		if (j >= 65535) {
			g_warning ("(%s)Buffer overflow\n", G_GNUC_FUNCTION);
			break;
		}
	}
#else
	g_snprintf (fixbuf, 65536, "0 1 255 {1 index exch /.notdef put} for\n");
#endif
	retval = ttf_printf (retval,
			     "%s" \
			     "readonly def\n" \
			     "currentdict end\n" \
			     , fixbuf);

	data = g_strdup_printf ("currentfile eexec\n" \
				"dup /Private 16 dict dup begin\n" \
				"/RD{string currentfile exch readstring pop}executeonly def\n" \
				"/ND{noaccess def}executeonly def\n" \
				"/NP{noaccess put}executeonly def\n" \
				"/ForceBold %s def\n" \
				"/BlueValues [ " \
				, g->force_bold ? "true" : "false");
	fixbuf[0] = '\0';
	for (i = 0, j = 0; i < nblues; i++) {
		g_snprintf (&fixbuf[j], 65535 - j, "%d ", bluevalues[i]);
		j = strlen (fixbuf);
		if (j >= 65535) {
			g_warning ("(%s)Buffer overflow\n", G_GNUC_FUNCTION);
			break;
		}
	}
	data = ttf_printf (data,
			   "%s] def\n" \
			   "/OtherBlues [ " \
			   , fixbuf);
	for (i = 0, j = 0; i < notherb; i++) {
		g_snprintf (&fixbuf[j], 65535 - j, "%d ", otherblues[i]);
		j = strlen (fixbuf);
		if (j >= 65535) {
			g_warning ("(%s)Buffer overflow\n", G_GNUC_FUNCTION);
			break;
		}
	}
	data = ttf_printf (data, "%s] def\n", fixbuf);
	if (stdhw != 0) data = ttf_printf (data, "/StdHW [ %d ] def\n", stdhw);
	if (stdvw != 0) data = ttf_printf (data, "/StdVW [ %d ] def\n", stdvw);
	data = ttf_printf (data, "/StemSnapH [ ");
	for (i = 0, j = 0; i < 12 && stemsnaph[i] != 0; i++) {
		g_snprintf (&fixbuf[j], 65535 - j, "%d ", stemsnaph[i]);
		j = strlen (fixbuf);
		if (j >= 65535) {
			g_warning ("(%s)Buffer overflow\n", G_GNUC_FUNCTION);
			break;
		}
	}
	data = ttf_printf (data,
					   "%s] def\n" \
					   "/StemSnapV [ " \
					   , fixbuf);
	for (i = 0, j = 0; i < 12 && stemsnapv[i] != 0; i++) {
		g_snprintf (&fixbuf[j], 65535 - j, "%d ", stemsnapv[i]);
		j = strlen (fixbuf);
		if (j >= 65535) {
			g_warning ("(%s)Buffer overflow\n", G_GNUC_FUNCTION);
			break;
		}
	}
	data = ttf_printf (data,
					   "%s] def\n" \
					   "/MinFeature {16 16} def\n" \
					   "/password 5839 def\n" \
					   , fixbuf);

	subid = 5;
	for (gi = g->glyph; gi != NULL; gi = gi->next) {
		subid += gi->nsg;
	}

	data = ttf_printf (data,
					   "/Subrs %d array\n" \
					   "dup 0 {\n" \
					   "\t3 0 callothersubr pop pop setcurrentpoint return\n" \
					   "} NP\n" \
					   "dup 1 {\n" \
					   "\t0 1 callothersubr return\n" \
					   "} NP\n" \
					   "dup 2 {\n" \
					   "\t0 2 callothersubr return\n" \
					   "} NP\n" \
					   "dup 3 {\n" \
					   "\treturn\n" \
					   "} NP\n" \
					   "dup 4 {\n" \
					   "\t1 3 callothersubr pop callsubr return\n" \
					   "} NP\n" \
					   , subid);

	subid = 5;
	for (gi = g->glyph; gi != NULL; gi = gi->next) {
		subid += print_glyph_subs (&data, gi, subid);
		nchars++;
	}
	data = ttf_printf (data,
					   "ND\n" \
					   "2 index /CharStrings %d dict dup begin\n" \
					   , nchars);
	for (gi = g->glyph; gi != NULL; gi = gi->next) {
		buf = print_glyph (gi);
		data = ttf_printf (data, "%s", buf);
		g_free (buf);
	}

	strcpy (fixbuf,
			"end\n" \
			"end\n" \
			"readonly put\n" \
			"noaccess put\n" \
			"dup /FontName get exch definefont pop\n" \
			"mark currentfile closefile\n");

	j = strlen (data);
	data = (gchar *)g_realloc (data, sizeof (gchar) * (strlen (data) + strlen (fixbuf) + 1));
	data[j] = 0;
	strcat (data, fixbuf);

	buf = ttf_type1_dump (data);
	g_free (data);
	j = strlen (retval);
	retval = (gchar *)g_realloc (retval, sizeof (gchar) * (strlen (retval) + strlen (buf) + 14));
	retval[j] = 0;
	strcat (retval, buf);
	strcat (retval, "cleartomark\n");
	g_free (buf);

	return retval;
}

#if 0
gchar *
ttf2pfa (const char *filename, gint fn)
{
	struct glyph_face *g = glyph_cache;
	struct glyph *gi;
	gchar *retval, *buf, *data, fixbuf[65536];
	gint nchars = 0, subid, i, j;

	g_return_val_if_fail (glyph_cache != NULL, NULL);

	while (g != NULL) {
		if (!strcmp (g->filename, filename) && fn == g->face) break;
		g = g->next;
	}

	g_return_val_if_fail (g != NULL, NULL);
	g_return_val_if_fail (g->glyph != NULL, NULL);

	italic_angle = g->italic_angle;
	if (italic_angle > 45.0 || italic_angle < -45.0) italic_angle = 0.0;

	findblues (g);
	for (gi = g->glyph; gi != NULL; gi = gi->next) {
		buildstems (gi);
	}
	stemstatistics (g->glyph);
	docorrectwidth (g);

	retval = g_strdup_printf ("%%!PS-AdobeFont-1.0: %s %s\n" \
							  "12 dict begin\n" \
							  "/FontInfo 9 dict dup begin\n" \
							  "/version (%s) readonly def\n" \
							  "/Notice (%s) readonly def\n" \
							  "/FullName (%s) readonly def\n" \
							  "/FamilyName (%s) readonly def\n" \
							  "/Weight (%s) readonly def\n" \
							  "/ItalicAngle %f def\n" \
							  "/isFixedPitch %s def\n" \
							  "/UnderlinePosition %d def\n" \
							  "/UnderlineThickness %hd def\n" \
							  "end readonly def\n" \
							  "/FontName /%s def\n" \
							  "/PaintType 0 def\n" \
							  "/StrokeWidth 0 def\n" \
							  "/FontType 1 def\n" \
							  "/FontMatrix [0.001 0 0 0.001 0 0] def\n" \
							  "/FontBBox {%d %d %d %d} readonly def\n" \
							  "/Encoding 256 array\n" \
							  , g->ps_name, g->copyright
							  , g->version
							  , g->copyright
							  , g->full_name
							  , g->family_name
							  , g->style_name
							  , italic_angle
							  , g->is_fixed_pitch ? "true" : "false"
							  , iscale (g->underline_position)
							  , iscale (g->underline_thickness)
							  , g->ps_name
							  , bbox[0], bbox[1], bbox[2], bbox[3]);

	/* determine number of elements for metrics table */
	for (i = 0, j = 0; i < 256; i++) {
		g_snprintf (&fixbuf[j], 65535 - j, "dup %d /%s put\n", i, gp_ps_from_unicode (i));
		j = strlen (fixbuf);
		if (j >= 65535) {
			g_warning ("(%s)Buffer overflow\n", G_GNUC_FUNCTION);
			break;
		}
	}
	retval = ttf_printf (retval,
						 "%s" \
						 "readonly def\n" \
						 "currentdict end\n" \
						 , fixbuf);

	data = g_strdup_printf ("currentfile eexec\n" \
							"dup /Private 16 dict dup begin\n" \
							"/RD{string currentfile exch readstring pop}executeonly def\n" \
							"/ND{noaccess def}executeonly def\n" \
							"/NP{noaccess put}executeonly def\n" \
							"/ForceBold %s def\n" \
							"/BlueValues [ " \
							, g->force_bold ? "true" : "false");
	for (i = 0, j = 0; i < nblues; i++) {
		g_snprintf (&fixbuf[j], 65535 - j, "%d ", bluevalues[i]);
		j = strlen (fixbuf);
		if (j >= 65535) {
			g_warning ("(%s)Buffer overflow\n", G_GNUC_FUNCTION);
			break;
		}
	}
	data = ttf_printf (data,
					   "%s] def\n" \
					   "/OtherBlues [ " \
					   , fixbuf);
	for (i = 0, j = 0; i < notherb; i++) {
		g_snprintf (&fixbuf[j], 65535 - j, "%d ", otherblues[i]);
		j = strlen (fixbuf);
		if (j >= 65535) {
			g_warning ("(%s)Buffer overflow\n", G_GNUC_FUNCTION);
			break;
		}
	}
	data = ttf_printf (data, "%s] def\n", fixbuf);
	if (stdhw != 0) data = ttf_printf (data, "/StdHW [ %d ] def\n", stdhw);
	if (stdvw != 0) data = ttf_printf (data, "/StdVW [ %d ] def\n", stdvw);
	data = ttf_printf (data, "/StemSnapH [ ");
	for (i = 0, j = 0; i < 12 && stemsnaph[i] != 0; i++) {
		g_snprintf (&fixbuf[j], 65535 - j, "%d ", stemsnaph[i]);
		j = strlen (fixbuf);
		if (j >= 65535) {
			g_warning ("(%s)Buffer overflow\n", G_GNUC_FUNCTION);
			break;
		}
	}
	data = ttf_printf (data,
					   "%s] def\n" \
					   "/StemSnapV [ " \
					   , fixbuf);
	for (i = 0, j = 0; i < 12 && stemsnapv[i] != 0; i++) {
		g_snprintf (&fixbuf[j], 65535 - j, "%d ", stemsnapv[i]);
		j = strlen (fixbuf);
		if (j >= 65535) {
			g_warning ("(%s)Buffer overflow\n", G_GNUC_FUNCTION);
			break;
		}
	}
	data = ttf_printf (data,
					   "%s] def\n" \
					   "/MinFeature {16 16} def\n" \
					   "/password 5839 def\n" \
					   , fixbuf);

	subid = 5;
	for (gi = g->glyph; gi != NULL; gi = gi->next) {
		subid += gi->nsg;
	}

	data = ttf_printf (data,
					   "/Subrs %d array\n" \
					   "dup 0 {\n" \
					   "\t3 0 callothersubr pop pop setcurrentpoint return\n" \
					   "} NP\n" \
					   "dup 1 {\n" \
					   "\t0 1 callothersubr return\n" \
					   "} NP\n" \
					   "dup 2 {\n" \
					   "\t0 2 callothersubr return\n" \
					   "} NP\n" \
					   "dup 3 {\n" \
					   "\treturn\n" \
					   "} NP\n" \
					   "dup 4 {\n" \
					   "\t1 3 callothersubr pop callsubr return\n" \
					   "} NP\n" \
					   , subid);

	subid = 5;
	for (gi = g->glyph; gi != NULL; gi = gi->next) {
		subid += print_glyph_subs (&data, gi, subid);
		nchars++;
	}
	data = ttf_printf (data,
					   "ND\n" \
					   "2 index /CharStrings %d dict dup begin\n" \
					   , nchars);
	for (gi = g->glyph; gi != NULL; gi = gi->next) {
		buf = print_glyph (gi);
		data = ttf_printf (data, "%s", buf);
		g_free (buf);
	}

	strcpy (fixbuf,
			"end\n" \
			"end\n" \
			"readonly put\n" \
			"noaccess put\n" \
			"dup /FontName get exch definefont pop\n" \
			"mark currentfile closefile\n");

	j = strlen (data);
	data = (gchar *)g_realloc (data, sizeof (gchar) * (strlen (data) + strlen (fixbuf) + 1));
	data[j] = 0;
	strcat (data, fixbuf);

	buf = ttf_type1_dump (data);
	g_free (data);
	j = strlen (retval);
	retval = (gchar *)g_realloc (retval, sizeof (gchar) * (strlen (retval) + strlen (buf) + 14));
	retval[j] = 0;
	strcat (retval, buf);
	strcat (retval, "cleartomark\n");
	g_free (buf);

	return retval;
}
#endif

gchar *
ttf_type1_dump (gchar *src)
{
	gchar *p, *q, *r;
	gchar *retval = NULL;
	gchar *buf, *string;
	gint cnt = 0;

	active = 0;
	lenIV = 4;
	c1 = 52845;
	c2 = 22719;

	while (*(src + cnt) != 0) {
		string = src + cnt;
		getline (&string);
		cnt = string - src;
		if (!strcmp (line, "currentfile eexec\n")) {
			if ((buf = eexec_start (line)) != NULL) {
				retval = ttf_printf (retval, "%s", buf);
				g_free (buf);
			}

			continue;
		} else if (strstr (line, "/Subrs") && isspace (line[6])) {
			active = 1;
		} else if ((p = strstr (line, "/lenIV"))) {
			sscanf (p, "%*s %d", &lenIV);
		} else if ((p = strstr (line, "string currentfile"))) {
			*p = '\0';
			q = strrchr (line, '/');
			if (q) {
				r = cs_start;
				q++;
				while (!isspace (*q) && *q != '{') *r++ = *q++;
				*r = '\0';
			}
			*p = 's';
		}
		/* output line data */
		if ((buf = eexec_string (line)) != NULL) {
			retval = ttf_printf (retval, "%s", buf);
			g_free (buf);
		}
		if ((p = strstr (line, "currentfile closefile"))) {
			if ((buf = eexec_end ()) != NULL) {
				retval = ttf_printf (retval, "%s", buf);
				g_free (buf);
			}
		}
		if (start_charstring) {
			if (!cs_start[0]) {
				g_warning ("couldn't find charstring start command\n");
				return NULL;
			}
			string = src + cnt;
			if ((buf = parse_charstring (&string)) != NULL) {
				retval = ttf_printf (retval, "%s", buf);
				g_free (buf);
			}
			cnt = string - src;
		}
	}

	return retval;
}

static gint
outl_moveto (FT_Vector *to, void *unused)
{
	gdouble tox, toy;

	tox = scale_factor * (gdouble)to->x;
	toy = scale_factor * (gdouble)to->y;
	if (curg->lastentry) {
		gclosepath (curg);
	}
	fg_rmoveto (curg, tox, toy);
	lastx = tox;
	lasty = toy;

	return 0;
}

static gint
outl_lineto (FT_Vector *to, void *unused)
{
	gdouble tox, toy;

	tox = scale_factor * (gdouble)to->x;
	toy = scale_factor * (gdouble)to->y;
	fg_rlineto (curg, tox, toy);
	lastx = tox;
	lasty = toy;

	return 0;
}

static gint
outl_conicto (FT_Vector *control1, FT_Vector *to, void *unused)
{
	gdouble c1x, c1y, tox, toy;

	c1x = scale_factor * (gdouble)control1->x;
	c1y = scale_factor * (gdouble)control1->y;
	tox = scale_factor * (gdouble)to->x;
	toy = scale_factor * (gdouble)to->y;
	fg_rrcurveto (curg,
				  (lastx + 2.0 * c1x) / 3.0,
				  (lasty + 2.0 * c1y) / 3.0,
				  (2.0 * c1x + tox) / 3.0,
				  (2.0 * c1y + toy) / 3.0,
				  tox,
				  toy);
	lastx = tox;
	lasty = toy;

	return 0;
}

static gint
outl_cubicto (FT_Vector *control1, FT_Vector *control2, FT_Vector *to, void *unused)
{
	gdouble c1x, c1y, c2x, c2y, tox, toy;

	c1x = scale_factor * (gdouble)control1->x;
	c1y = scale_factor * (gdouble)control1->y;
	c2x = scale_factor * (gdouble)control2->x;
	c2y = scale_factor * (gdouble)control2->y;
	tox = scale_factor * (gdouble)to->x;
	toy = scale_factor * (gdouble)to->y;
	fg_rrcurveto (curg, c1x, c1y, c2x, c2y, tox, toy);
	lastx = tox;
	lasty = toy;

	return 0;
}

void
reversepaths (struct gentry *from, struct gentry *to)
{
	struct gentry *ge, *nge, *pge, *cur, *next;
	gint i, n, ilast[2];
	gdouble flast[2], f;

	for (ge = from; ge != 0 && ge != to; ge = ge->next) {
		if (ge->type == GE_LINE || ge->type == GE_CURVE) {
			/* cut out the path itself */
			pge = ge->prev; /* GE_MOVE */
			if (pge == 0) {
				g_warning ("No MOVE before line\n");
				return;
			}
			nge = ge->bkwd->next; /* GE_PATH */
			pge->next = nge;
			nge->prev = pge;
			ge->bkwd->next = 0; /* mark end of chain */

			/* remember the starting point */
			if (ge->flags & GEF_FLOAT) {
				flast[0] = pge->fx3;
				flast[1] = pge->fy3;
			} else {
				ilast[0] = pge->ix3;
				ilast[1] = pge->iy3;
			}

			/* then reinsert them in backwards order */
			for (cur = ge; cur != 0; cur = next) {
				next = cur->next;
				if (cur->flags & GEF_FLOAT) {
					for (i = 0; i < 2; i++) {
						/* reverse the direction of path element */
						f = cur->fpoints[i][0];
						cur->fpoints[i][0] = cur->fpoints[i][1];
						cur->fpoints[i][1] = f;
						f = flast[i];
						flast[i] = cur->fpoints[i][2];
						cur->fpoints[i][2] = f;
					}
				} else {
					for (i = 0; i < 2; i++) {
						/* reverse the direction of path element */
						n = cur->ipoints[i][0];
						cur->ipoints[i][0] = cur->ipoints[i][1];
						cur->ipoints[i][1] = n;
						n = ilast[i];
						ilast[i] = cur->ipoints[i][2];
						cur->ipoints[i][2] = n;
					}
				}
				addgeafter (pge, cur);
			}

			/* restore the starting point */
			if (ge->flags & GEF_FLOAT) {
				pge->fx3 = flast[0];
				pge->fy3 = flast[1];
			} else {
				pge->ix3 = ilast[0];
				pge->iy3 = ilast[1];
			}
			ge = nge;
		}
	}
}

static void
addgeafter (struct gentry *oge, struct gentry *nge)
{
	if (oge->type == GE_MOVE) {
		/* insert before next */
		if (oge->next->type == GE_PATH) {
			/* first and only gentry in path */
			nge->frwd = nge->bkwd = nge;
		} else {
			nge->frwd = oge->next;
			nge->bkwd = oge->next->bkwd;
			oge->next->bkwd->frwd = nge;
			oge->next->bkwd = nge;
		}
	} else {
		nge->frwd = oge->frwd;
		nge->bkwd = oge;
		oge->frwd->bkwd = nge;
		oge->frwd = nge;
	}

	nge->next = oge->next;
	nge->prev = oge;
	oge->next->prev = nge;
	oge->next = nge;

	if (nge->frwd->prev->type == GE_MOVE) {
		/* fix up the GE_MOVE entry */
		if (nge->flags & GEF_FLOAT) {
			nge->frwd->prev->fx3 = nge->fx3;
			nge->frwd->prev->fy3 = nge->fy3;
		} else {
			nge->frwd->prev->ix3 = nge->ix3;
			nge->frwd->prev->iy3 = nge->iy3;
		}
	}
}

void
gclosepath (struct glyph *g)
{
	struct gentry *oge, *nge;

	oge = g->lastentry;
	if (g->path == 0) {
		if (oge != 0) {
			if (oge->type == GE_MOVE) {
				g->lastentry = oge->prev;
				if (oge->prev == 0) g->entries = 0;
			}
		}
		return;
	}

	nge = newgentry (oge->flags & GEF_FLOAT);
	nge->type = GE_PATH;

	g->path = 0;

	oge->next = nge;
	nge->prev = oge;
	g->lastentry = nge;
}

static struct gentry *
newgentry (gint flags)
{
	struct gentry *ge;

	ge = g_new0 (struct gentry, 1);

	ge->stemid = -1;
	ge->flags = flags;

	return ge;
}

void
fg_rmoveto (struct glyph *g, gdouble x, gdouble y)
{
	struct gentry *oge;

	if ((oge = g->lastentry) != 0) {
		if (oge->type == GE_MOVE) { /* just eat up the first move */
			oge->fx3 = x;
			oge->fy3 = y;
		} else if (oge->type != GE_LINE && oge->type != GE_CURVE) {
			struct gentry *nge;

			nge = newgentry (GEF_FLOAT);
			nge->type = GE_MOVE;
			nge->fx3 = x;
			nge->fy3 = y;

			oge->next = nge;
			nge->prev = oge;
			g->lastentry = nge;
		}
	} else {
		struct gentry *nge;

		nge = newgentry (GEF_FLOAT);
		nge->type = GE_MOVE;
		nge->fx3 = x;
		nge->fy3 = y;
		nge->bkwd = (struct gentry *)&g->entries;
		g->entries = g->lastentry = nge;
	}
}

void
fg_rlineto (struct glyph *g, gdouble x, gdouble y)
{
	struct gentry *oge, *nge;

	nge = newgentry (GEF_FLOAT);
	nge->type = GE_LINE;
	nge->fx3 = x;
	nge->fy3 = y;

	if ((oge = g->lastentry) != 0) {
		if (x == oge->fx3 && y == oge->fy3) { /* emtpy line */
			/* ignore it or we will get in troubles later */
			g_free (nge);
			return;
		}
		if (g->path == 0) {
			g->path = nge;
			nge->bkwd = nge->frwd = nge;
		} else {
			oge->frwd = nge;
			nge->bkwd = oge;
			g->path->bkwd = nge;
			nge->frwd = g->path;
		}

		oge->next = nge;
		nge->prev = oge;
		g->lastentry = nge;
	} else {
		g_free (nge);
	}
}

void
fg_rrcurveto (struct glyph *g, gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble x3, gdouble y3)
{
	struct gentry *oge, *nge;

	oge = g->lastentry;
	if (oge && oge->fx3 == x1 && x1 == x2 && x2 == x3) {
		/* check if it's actually a line */
		fg_rlineto (g, x1, y3);
	} else if (oge && oge->fy3 == y1 && y1 == y2 && y2 == y3)
		fg_rlineto (g, x3, y1);
	else {
		nge = newgentry (GEF_FLOAT);
		nge->type = GE_CURVE;
		nge->fx1 = x1;
		nge->fy1 = y1;
		nge->fx2 = x2;
		nge->fy2 = y2;
		nge->fx3 = x3;
		nge->fy3 = y3;

		if (oge != 0) {
			if (x3 == oge->fx3 && y3 == oge->fy3) {
				g_free (nge);
				return;
			}
			if (g->path == 0) {
				g->path = nge;
				nge->bkwd = nge->frwd = nge;
			} else {
				oge->frwd = nge;
				nge->bkwd = oge;
				g->path->bkwd = nge;
				nge->frwd = g->path;
			}

			oge->next = nge;
			nge->prev = oge;
			g->lastentry = nge;
		} else {
			g_free (nge);
		}
	}
}

void
fclosepaths (struct glyph *g)
{
	struct gentry *ge, *fge, *xge, *nge;
	int i;

	for (xge = g->entries; xge != 0; xge = xge->next) {
		if (xge->type != GE_PATH) continue;

		ge = xge->prev;
		if (ge == 0 || (ge->type != GE_LINE && ge->type != GE_CURVE)) {
			g_warning ("glyph got empty path -- %d\n", g->char_code);
			return;
		}
		fge = ge->frwd;
		if (fge->prev == 0 || fge->prev->type != GE_MOVE) {
			g_warning ("glyph got strange beginning of path -- %d\n", g->char_code);
			return;
		}
		fge = fge->prev;
		if (fge->fx3 != ge->fx3 || fge->fy3 != ge->fy3) {
			nge = newgentry (GEF_FLOAT);
			(*nge) = (*ge);
			nge->fx3 = fge->fx3;
			nge->fy3 = fge->fy3;
			nge->type = GE_LINE;

			addgeafter (ge, nge);
			if (fabs (ge->fx3 - fge->fx3) <= 2 && fabs (ge->fy3 - fge->fy3) <= 2) {
				/* small change, try to get rid of the new entry */
				gdouble df[2];

				for (i = 0; i < 2; i++) {
					df[i] = ge->fpoints[i][2] - fge->fpoints[i][2];
					df[i] = fclosegap (nge, nge, i, df[i], NULL);
				}

				if (df[0] == 0. && df[1] == 0.) {
					/* closed gap successfully, remove the added entry */
					freethisge (nge);
				}
			}
		}
	}
}

static gdouble
fclosegap (struct gentry *from, struct gentry *to, gint axis, gdouble gap, gdouble *ret)
{
	gdouble rm[2], oldpos[2], times, limit, df, dx;
	gint j, k;
	struct gentry *xge, *pge, *nge, *bge[2];

	/* remember the old points to calculate ret */
	oldpos[0] = from->prev->fpoints[axis][2];
	oldpos[1] = to->fpoints[axis][2];

	rm[0] = rm[1] = gap / 2.;
	bge[0] = from; /* this is convenient for iterations */
	bge[1] = to;

	/* first try to modify large curves but if have none then settle for small */
	for (times = (10. - 1); times > 0.1; times /= 2.) {
		if (rm[0] + rm[1] == 0.) break;

		/* iterate in both directions, backwards then forwards */
		for (j = 0; j < 2; j++) {
			if (rm[j] == 0.) /* if this direction is exhausted */
				continue;

			limit = fabs (rm[j]) * (1. + times);

			for (xge = bge[j]->cntr[j]; xge != bge[!j]; xge = xge->cntr[j]) {
				dx = xge->fpoints[axis][2] - xge->prev->fpoints[axis][2];
				df = fabs (dx) - limit;
				if (df <= FEPS ) /* curve is too small to change */
					continue;

				if (df >= fabs (rm[j])) df = rm[j];
				else df *= fsign (rm[j]); /* we may cover this part of rm */

				rm[j] -= df;
				limit = fabs (rm[j]) * (1. + times);

				if (xge->type == GE_CURVE) { /* correct internal points */
					gdouble scale = ((dx + df) / dx) - 1.;
					gdouble base;

					if (j) base = xge->fpoints[axis][2];
					else base = xge->prev->fpoints[axis][2];

					for (k = 0; k < 2; k++) {
						xge->fpoints[axis][k] += scale * (xge->fpoints[axis][k] - base);
					}
				}

				/* move all the intermediate lines */
				if (j) {
					df = -df; /* absolute direction */
					pge = bge[1]->bkwd;
					nge = xge->bkwd;
				} else {
					xge->fpoints[axis][2] += df;
					pge = bge[0];
					nge = xge->frwd;
				}
				while (nge != pge) {
					if (nge->type == GE_CURVE) {
						nge->fpoints[axis][0] += df;
						nge->fpoints[axis][1] += df;
					}
					nge->fpoints[axis][2] += df;
					if (nge->next != nge->frwd) { /* last entry of contour */
						nge->frwd->prev->fpoints[axis][2] += df;
					}
					nge = nge->cntr[!j];
				}
				if (rm[j] == 0.) break;
			}
		}
	}
	/* find the difference */
	oldpos[0] -= from->prev->fpoints[axis][2];
	oldpos[1] -= to->fpoints[axis][2];

	if (ret) {
		ret[0] = oldpos[0] - from->prev->fpoints[axis][2];
		ret[1] = oldpos[1] - to->fpoints[axis][2];
	}

	return rm[0] + rm[1];
}

static struct gentry *
freethisge (struct gentry *ge)
{
	struct gentry *xge;

	if (ge->bkwd != ge->prev) {
		/* at beginning of the contour */
		xge = ge->bkwd;
		if (xge == ge) {
			/* was the only line in contour */
			/* remove the contour completely */
			/* prev is GE_MOVE, next is GE_PATH, remove them all */
			/* may be the first contour, then ->bkwd points to ge->entries */
			if (ge->prev->prev == 0) *(struct gentry **)(ge->prev->bkwd) = ge->next->next;
			else ge->prev->prev->next = ge->next->next;

			if (ge->next->next) {
				ge->next->next->prev = ge->prev->prev;
				ge->next->next->bkwd = ge->prev->bkwd;
			}

			xge = ge->next->next;
			g_free (ge->prev);
			g_free (ge->next);
			g_free (ge);

			return xge;
		}

		/* move the start point of the contour */
		if (ge->flags & GEF_FLOAT) {
			ge->prev->fx3 = xge->fx3;
			ge->prev->fy3 = xge->fy3;
		} else {
			ge->prev->ix3 = xge->ix3;
			ge->prev->iy3 = xge->iy3;
		}
	} else if (ge->frwd != ge->next) {
		/* at end of the contour */
		xge = ge->frwd->prev;
		/* move the start point of the contour */
		if (ge->flags & GEF_FLOAT) {
			xge->fx3 = ge->bkwd->fx3;
			xge->fy3 = ge->bkwd->fy3;
		} else {
			xge->ix3 = ge->bkwd->ix3;
			xge->iy3 = ge->bkwd->iy3;
		}
	}

	ge->prev->next = ge->next;
	ge->next->prev = ge->prev;
	ge->bkwd->frwd = ge->frwd;
	ge->frwd->bkwd = ge->bkwd;

	xge = ge->next;
	g_free (ge);

	return xge;
}

void
ffixquadrants (struct glyph *g)
{
	struct gentry *ge, *nge;
	gint i, j, np, oldnp;
	gdouble sp[5]; /* split points, last one empty */
	gchar dir[5]; /* for debugging, direction by which split happened */
	gdouble a, b, *pts; /* points of a curve */

	for (ge = g->entries; ge != 0; ge = ge->next) {
		if (ge->type != GE_CURVE) continue;
		
	  doagain:
		np = 0; /* no split points yet */
		for (i = 0; i < 2; i++) { /* first for x then for y */
			/* find the cooridnates of control points */
			a = ge->prev->fpoints[i][2];
			pts = &ge->fpoints[i][0];

			oldnp = np;
			np += fsqequation(3.0 * (-a + 3.0 * pts[0] - 3.0 * pts[1] + pts[2]),
							  6.0 * (a  - 2.0 * pts[0] +       pts[1]),
							  3.0 * (-a +       pts[0]),
							  &sp[np],
							  0.0,
							  1.0); /* XXX range is [0;1] */

			if (np == oldnp) continue;

			/* remove points that are too close to the ends 
			 * because hor/vert ends are permitted, also
			 * if the split point is VERY close to the ends
			 * but not exactly then just flatten it and check again.
			 */
			for (j = oldnp; j < np; j++) {
				dir[j] = i;
				if (sp[j] < 0.03) { /* front end of curve */
					if (ge->fpoints[i][0] != ge->prev->fpoints[i][2]) {
						ge->fpoints[i][0] = ge->prev->fpoints[i][2];
						goto doagain;
					}
					if (ge->fpoints[i][1] != ge->fpoints[i][0] && fsign (ge->fpoints[i][1] - ge->fpoints[i][0])) {
						ge->fpoints[i][1] = ge->fpoints[i][0];
						goto doagain;
					}
					sp[j] = sp[j + 1];
					np--;
					j--;
				} else if (sp[j] > 0.97) { /* rear end of curve */
					if (ge->fpoints[i][1] != ge->fpoints[i][2]) {
						ge->fpoints[i][1] = ge->fpoints[i][2];
						goto doagain;
					}
					if (ge->fpoints[i][0] != ge->fpoints[i][1] &&
						fsign (ge->prev->fpoints[i][2] - ge->fpoints[i][0]) != fsign (ge->fpoints[i][0] - ge->fpoints[i][1])) {
						ge->fpoints[i][0] = ge->fpoints[i][1];
						goto doagain;
					}
					sp[j] = sp[j + 1];
					np--;
					j--;
				} 
			}
		}

		/* no split points, leave it alone */
		if (np == 0) continue;

		/* sort the points ascending */
		for (i = 0; i < np; i++) {
			for (j = i + 1; j < np; j++) {
				if (sp[i] > sp[j]) {
					a = sp[i];
					sp[i] = sp[j];
					sp[j] = a;
				}
			}
		}

		/* now finally do the split on each point */
		for (j = 0; j < np; j++) {
			gdouble k1, k2, c;

			k1 = sp[j];
			k2 = 1 - k1;

			nge = newgentry (GEF_FLOAT);
			(*nge) = (*ge);

#define SPLIT(pt1, pt2)		( (pt1) + k1*((pt2)-(pt1)) ) /* order is important! */
			for (i = 0; i < 2; i++) { /* for x and y */
				a = ge->fpoints[i][0]; /* get the middle points */
				b = ge->fpoints[i][1];

				/* calculate new internal points */
				c = SPLIT(a, b);

				ge->fpoints[i][0] = SPLIT(ge->prev->fpoints[i][2], a);
				ge->fpoints[i][1] = SPLIT(ge->fpoints[i][0], c);

				nge->fpoints[i][1] = SPLIT(b, nge->fpoints[i][2]);
				nge->fpoints[i][0] = SPLIT(c, nge->fpoints[i][1]);

				ge->fpoints[i][2] = SPLIT(ge->fpoints[i][1], nge->fpoints[i][0]);
			}
#undef SPLIT

			addgeafter (ge, nge);

			/* go to the next part, adjust remaining points */
			ge = nge;
			for (i = j + 1; i < np; i++) {
				sp[i] = (sp[i] - k1) / k2;
			}
		}
	}
}

void
fsplitzigzags (struct glyph *g)
{
	struct gentry *ge, *nge;
	gdouble a, b, c, d;

	for (ge = g->entries; ge != 0; ge = ge->next) {
		if (ge->type != GE_CURVE) continue;

		/* if the curve is not a zigzag */
		if (!fiszigzag (ge)) continue;

		/* split the curve by t=0.5 */
		nge = newgentry (GEF_FLOAT);
		(*nge) = (*ge);
		nge->type = GE_CURVE;

		a = ge->prev->fx3;
		b = ge->fx1;
		c = ge->fx2;
		d = ge->fx3;
		nge->fx3 = d;
		nge->fx2 = (c + d) / 2.;
		nge->fx1 = (b + 2. * c + d) / 4.;
		ge->fx3 = (a + b * 3. + c * 3. + d) / 8.;
		ge->fx2 = (a + 2. * b + c) / 4.;
		ge->fx1 = (a + b) / 2.;

		a = ge->prev->fy3;
		b = ge->fy1;
		c = ge->fy2;
		d = ge->fy3;
		nge->fy3 = d;
		nge->fy2 = (c + d) / 2.;
		nge->fy1 = (b + 2. * c + d) / 4.;
		ge->fy3 = (a + b * 3. + c * 3. + d) / 8.;
		ge->fy2 = (a + 2. * b + c) / 4.;
		ge->fy1 = (a + b) / 2.;

		addgeafter (ge, nge);
	}
}

void
fforceconcise (struct glyph *g)
{
	struct gentry *ge, *nge, tge;
	gdouble firstlen, lastlen, sumlen, scale;
	gdouble dxw1, dyw1, dxw2, dyw2;
	gdouble dxb1, dyb1, dxe1, dye1;
	gdouble dxb2, dyb2, dxe2, dye2;
	gdouble maxsc1, maxsc2;
	gint i;

	fdelsmall (g, 0.05);
	fnormalizec (g);

	for (ge = g->entries; ge != 0; ge = ge->next) {
		if (ge->type != GE_CURVE) continue;

		/* the whole direction of curve */
		dxw1 = ge->fx3 - ge->prev->fx3;
		dyw1 = ge->fy3 - ge->prev->fy3;

		while (1) {
			/* the whole direction of curve */
			dxw1 = ge->fx3 - ge->prev->fx3;
			dyw1 = ge->fy3 - ge->prev->fy3;

			/* directions of  ends of curve */
			dxb1 = ge->fx1 - ge->prev->fx3;
			dyb1 = ge->fy1 - ge->prev->fy3;
			dxe1 = ge->fx3 - ge->fx2;
			dye1 = ge->fy3 - ge->fy2;

			nge = ge->frwd;

			if (nge->type != GE_CURVE) break;

			dxw2 = nge->fx3 - ge->fx3;
			dyw2 = nge->fy3 - ge->fy3;

			dxb2 = nge->fx1 - ge->fx3;
			dyb2 = nge->fy1 - ge->fy3;
			dxe2 = nge->fx3 - nge->fx2;
			dye2 = nge->fy3 - nge->fy2;

			/* if curve changes direction */
			if (fsign (dxw1) != fsign (dxw2) || fsign (dyw1) != fsign (dyw2))
				break;

			/* if the arch is going in other direction */
			if (fsign (fabs(dxb1 * dyw1) - fabs (dyb1 * dxw1)) * fsign (fabs (dxe2 * dyw2) - fabs (dye2 * dxw2)) > 0)
				break;

			/* get possible scale limits within which we won't cross quadrants */
			if (fcrossrays(ge, nge, &maxsc1, &maxsc2) == 0) break;

			/* would create a zigzag */
			if(maxsc1 < 1. || maxsc2 < 1.) break;

			ge->dir = fgetcvdir (ge);
			nge->dir = fgetcvdir (nge);

			/* would create a zigzag */
			if (((ge->dir & CVDIR_FRONT) - CVDIR_FEQUAL) * ((nge->dir & CVDIR_REAR) - CVDIR_REQUAL) < 0)
				break;

			firstlen = sqrt (dxe1 * dxe1 + dye1 * dye1);
			lastlen = sqrt (dxb2 * dxb2 + dyb2 * dyb2);
			sumlen = firstlen + lastlen;

			/* check the scale limits */
			if (sumlen / firstlen > maxsc1 || sumlen / lastlen > maxsc2)
				break;

			/* OK, it seems like we can attempt to join these two curves */
			tge.flags = ge->flags;
			tge.prev = ge->prev;
			tge.fx1 = ge->fx1;
			tge.fy1 = ge->fy1;
			tge.fx2 = nge->fx2;
			tge.fy2 = nge->fy2;
			tge.fx3 = nge->fx3;
			tge.fy3 = nge->fy3;

			dxb1 = tge.fx1 - tge.prev->fx3;
			dyb1 = tge.fy1 - tge.prev->fy3;
			dxe1 = tge.fx3 - tge.fx2;
			dye1 = tge.fy3 - tge.fy2;

			/* scale the first segment */
			scale = sumlen / firstlen;
			tge.fx1 = tge.prev->fx3 + scale * dxb1;
			tge.fy1 = tge.prev->fy3 + scale * dyb1;

			/* scale the last segment */
			scale = sumlen / lastlen;
			tge.fx2 = tge.fx3 - scale * dxe1;
			tge.fy2 = tge.fy3 - scale * dye1;

			/* now check if we got something sensible */

			/* check if some important points is too far from original */
			scale = firstlen / sumlen;
			{
				gdouble pts[4] = { 0./*will be replaced*/, 0.5, 0.25, 0.75 };
				gint i, bad;

				pts[0] = scale;
				bad = 0;

				for (i = 0; i < sizeof (pts) / sizeof (pts[0]); i++) {
					if (fckjoinedcv (g, pts[i], &tge, ge, nge, scale)) {
						bad = 1;
						break;
					}
				}
				if (bad) break;
			}

			for (i = 0; i < 3; i++) {
				ge->fxn[i] = tge.fxn[i];
				ge->fyn[i] = tge.fyn[i];
			}

			freethisge (nge);
		}
	}
}

void
fstraighten (struct glyph *g)
{
	struct gentry *ge, *pge, *nge, *ige;
	gdouble df, iln, oln;
	gint dir, i, o;

	for (ige = g->entries; ige != 0; ige = ige->next) {
		if (ige->type != GE_CURVE) continue;

		ge = ige;
		pge = ge->bkwd;
		nge = ge->frwd;

		df = 0.;

		/* look for vertical then horizontal */
		for (i = 0; i < 2; i++) {
			o = !i; /* other axis */

			iln = fabs (ge->fpoints[i][2] - pge->fpoints[i][2]);
			oln = fabs (ge->fpoints[o][2] - pge->fpoints[o][2]);
			/*
			 * if current curve is almost a vertical line, and it
			 * doesn't begin or end horizontally (and the prev/next
			 * line doesn't join smoothly ?)
			 */
			if (oln < 1. ||
				ge->fpoints[o][2] == ge->fpoints[o][1] ||
				ge->fpoints[o][0] == pge->fpoints[o][2] ||
				iln > 2. ||
				(iln > 1.  && iln / oln > 0.1))
				continue;

			df = ge->fpoints[i][2] - pge->fpoints[i][2];
			dir = fsign (ge->fpoints[o][2] - pge->fpoints[o][2]);
			ge->type = GE_LINE;

			/*
			 * suck in all the sequence of such almost lines
			 * going in the same direction but not deviating
			 * too far from vertical
			 */
			iln = fabs (nge->fpoints[i][2] - ge->fpoints[i][2]);
			oln = nge->fpoints[o][2] - ge->fpoints[o][2];

			/* that also gives oln != 0 */
			while (fabs (df) <= 5 && nge->type == GE_CURVE &&
				   dir == fsign (oln) &&
				   iln <= 2. &&
				   (iln <= 1.  || iln / fabs (oln) <= 0.1)) {
				ge->fx3 = nge->fx3;
				ge->fy3 = nge->fy3;

				freethisge (nge);
				fixendpath (ge);
				pge = ge->bkwd;
				nge = ge->frwd;

				df = ge->fpoints[i][2] - pge->fpoints[i][2];

				iln = fabs (nge->fpoints[i][2] - ge->fpoints[i][2]);
				oln = nge->fpoints[o][2] - ge->fpoints[o][2];
			}

			/* now check what do we have as previous/next line */

			if (ge != pge) { 
				if (pge->type == GE_LINE &&
					pge->fpoints[i][2] == pge->prev->fpoints[i][2] &&
					fabs (pge->fpoints[o][2] != pge->prev->fpoints[o][2])) {
					/* join the previous line with current */
					pge->fx3 = ge->fx3;
					pge->fy3 = ge->fy3;

					ige = freethisge (ge)->prev; /* keep the iterator valid */
					ge = pge;
					fixendpath (ge);
					pge = ge->bkwd;
				}
			}

			if (ge != nge) { 
				if (nge->type == GE_LINE &&
					nge->fpoints[i][2] == ge->fpoints[i][2] &&
					fabs (nge->fpoints[o][2] != ge->fpoints[o][2])) {
					/* join the next line with current */
					ge->fx3 = nge->fx3;
					ge->fy3 = nge->fy3;

					freethisge (nge);
					fixendpath (ge);
					pge = ge->bkwd;
					nge = ge->frwd;
				}
			}

			if (ge != pge) { 
				/* try to align the lines if neccessary */
				if (df != 0.) fclosegap (ge, ge, i, df, NULL);
			} else {
				/* contour consists of only one line, get rid of it */
				ige = freethisge (ge)->prev; /* keep the iterator valid */
			}

			break; /* don't bother looking at the other axis */
		}
	}
}

void
pathtoint (struct glyph *g)
{
	struct gentry *ge;
	gint x[3], y[3];
	gint i;

	fdelsmall (g, 1.0); /* get rid of sub-pixel contours */

	/* 1st pass, collect the directions of the curves: have
	 * to do that in advance, while everyting is float
	 */
	for (ge = g->entries; ge != 0; ge = ge->next) {
		if (!(ge->flags & GEF_FLOAT)) {
			g_warning ("glyphs has int entry, found in conversion to int -- %d\n", g->char_code);
			return;
		}
		if (ge->type == GE_CURVE) ge->dir = fgetcvdir (ge);
	}

	/* now do the conversion */
	for (ge = g->entries; ge != 0; ge = ge->next) {
		switch (ge->type) {
			case GE_MOVE:
			case GE_LINE:
				x[0] = iround (ge->fx3);
				y[0] = iround (ge->fy3);
				for (i = 0; i < 3; i++) { /* put some valid values everywhere, for convenience */
					ge->ixn[i] = x[0];
					ge->iyn[i] = y[0];
				}
				break;
			case GE_CURVE:
				for (i = 0; i < 3; i++) {
					x[i] = iround (ge->fxn[i]);
					y[i] = iround (ge->fyn[i]);
				}

				for (i = 0; i < 3; i++) {
					ge->ixn[i] = x[i];
					ge->iyn[i] = y[i];
				}
				ge->flags &= ~GEF_FLOAT; /* for fixcvdir */
				fixcvdir (ge, ge->dir);

				break;
		}
		ge->flags &= ~GEF_FLOAT;
	}
	g->flags &= ~GF_FLOAT;
}

void
flattencurves (struct glyph *g)
{
	struct gentry *ge;
	gint x0, y0, x1, y1, x2, y2, x3, y3;

	for (ge = g->entries; ge != 0; ge = ge->next) {
		if (ge->type != GE_CURVE) continue;

		x0 = ge->prev->ix3;
		y0 = ge->prev->iy3;
		x1 = ge->ix1;
		y1 = ge->iy1;
		x2 = ge->ix2;
		y2 = ge->iy2;
		x3 = ge->ix3;
		y3 = ge->iy3;

		if ((x1 - x0) * (y2 - y1) == (x2 - x1) * (y1 - y0) &&
			(x1 - x0) * (y3 - y2) == (x3 - x2) * (y1 - y0)) {
			ge->type = GE_LINE;
		}
	}
}

static gint
isign (gint x)
{
	if (x > 0) return 1;
	else if (x < 0) return -1;
	else return 0;
}

static gint
fsign (gdouble x)
{
	if (x > 0.0) return 1;
	else if (x < 0.0) return -1;
	else return 0;
}

static gint
fsqequation (gdouble a, gdouble b, gdouble c, gdouble *res, gdouble min, gdouble max)
{
	gdouble D;
	gint n;

	if (fabs (a) < 0.000001) { /* if a linear equation */
		n = 0;
		if (fabs (b) < 0.000001) /* not an equation at all */
			return 0;
		res[0] = -c/b;
		if (res[0] >= min && res[0] <= max) n++;
		return n;
	}

	D = b * b - 4.0 * a * c;
	if (D < 0) return 0;

	D = sqrt (D);

	n = 0;
	res[0] = (-b + D) / (2 * a);
	if (res[0] >= min && res[0] <= max) n++;

	res[n] = (-b - D) / (2 * a);
	if (res[n] >= min && res[n] <= max) n++;

	/* return 2nd solution only if it's different enough */
	if (n == 2 && fabs (res[0] - res[1]) < 0.000001) n=1;

	return n;
}

static gint
fiszigzag (struct gentry *ge)
{
	gdouble k, k1, k2;
	gdouble a, b;

	if (ge->type != GE_CURVE) return 0;

	a = fabs (ge->fy2 - ge->fy1);
	b = fabs (ge->fx2 - ge->fx1);
	k = a < FEPS ? (b <FEPS ? 1. : FBIGVAL) : b / a;
	a = fabs (ge->fy1 - ge->prev->fy3);
	b = fabs (ge->fx1 - ge->prev->fx3);
	k1 = a < FEPS ? (b < FEPS ? 1. : FBIGVAL) : b / a;
	a = fabs (ge->fy3 - ge->fy2);
	b = fabs (ge->fx3 - ge->fx2);
	k2 = a < FEPS ? (b <FEPS ? 1. : FBIGVAL) : b / a;

	/* if the curve is not a zigzag */
	if ((k1 >= k && k2 <= k) || (k1 <= k && k2 >= k)) return 0;
	else return 1;
}

static void
fdelsmall (struct glyph *g, gdouble minlen)
{
	struct gentry *ge, *nge, *pge, *xge, *next;
	gint i, k;
	gdouble dx, dy, d2, d2m;
	gdouble minlen2;
#define TIMESLARGER 10.	/* how much larger must be a curve to not change too much */

	minlen2 = minlen * minlen;

	for (ge = g->entries; ge != 0; ge = next) {
		next = ge->next;

		if (ge->type != GE_CURVE && ge->type != GE_LINE) continue;

		d2m = 0;
		for (i = (ge->type == GE_CURVE? 0: 2); i < 3; i++) {
			dx = ge->fxn[i] - ge->prev->fx3;
			dy = ge->fyn[i] - ge->prev->fy3;
			d2 = dx * dx + dy * dy;
			if(d2m < d2) d2m = d2;
		}

		if (d2m > minlen2) { /* line is not too small */
			/* XXX add more normalization here */
			continue;
		}

		/* if the line is too small */

		/* check forwards if we have a whole sequence of them */
		nge = ge;
		for (xge = ge->frwd; xge != ge; xge = xge->frwd) {
			d2m = 0;
			for (i = (xge->type == GE_CURVE? 0: 2); i < 3; i++) {
				dx = xge->fxn[i] - xge->prev->fx3;
				dy = xge->fyn[i] - xge->prev->fy3;
				d2 = dx*dx + dy*dy;
				if (d2m < d2)
					d2m = d2;
			}
			if (d2m > minlen2) /* line is not too small */
				break;
			nge = xge;
			if (next == nge) /* move the next step past this sequence */
				next = next->next;
		}

		/* check backwards if we have a whole sequence of them */
		pge = ge;
		for (xge = ge->bkwd; xge != ge; xge = xge->bkwd) {
			d2m = 0;
			for (i = (xge->type == GE_CURVE? 0: 2); i < 3; i++) {
				dx = xge->fxn[i] - xge->prev->fx3;
				dy = xge->fyn[i] - xge->prev->fy3;
				d2 = dx*dx + dy*dy;
				if (d2m < d2)
					d2m = d2;
			}
			if (d2m > minlen2) /* line is not too small */
				break;
			pge = xge;
		}

		/* now we have a sequence of small fragments in pge...nge (inclusive) */
		/* reduce whole sequence to one part and remember the middle point */
		if (pge != nge) {
			while (1) {
				xge = pge->frwd;
				if (xge == nge) {
					pge->fx1 = pge->fx2 = pge->fx3;
					pge->fx3 = nge->fx3;
					pge->fy1 = pge->fy2 = pge->fy3;
					pge->fy3 = nge->fy3;
					pge->type = GE_CURVE;
					freethisge (nge);
					break;
				}
				if (xge == nge->bkwd) {
					pge->fx1 = pge->fx2 = (pge->fx3 + xge->fx3) / 2.;
					pge->fx3 = nge->fx3;
					pge->fy1 = pge->fy2 = (pge->fy3 + xge->fy3) / 2.;
					pge->fy3 = nge->fy3;
					pge->type = GE_CURVE;
					freethisge (nge);
					freethisge (xge);
					break;
				}
				freethisge (pge);
				pge = xge;
				xge = nge->bkwd;
				freethisge (nge);
				nge = xge;
			}
		}
		ge = pge;

		/* check if the whole sequence is small */
		dx = ge->fx3 - ge->prev->fx3;
		dy = ge->fy3 - ge->prev->fy3;
		d2 = dx * dx + dy * dy;

		if (d2 > minlen2) { /* no, it is not */
			gdouble b, d;

			/* check that we did not create a monstrosity spanning quadrants */
			if (fsign (ge->fx1 - ge->prev->fx1) * fsign (ge->fx3 - ge->fx1) < 0 ||
				fsign (ge->fy1 - ge->prev->fy1) * fsign (ge->fy3 - ge->fy1) < 0 ) { 
				/* yes, we did; are both parts of this thing big enough ? */
				dx = ge->fx1 - ge->prev->fx3;
				dy = ge->fy1 - ge->prev->fy3;
				d2 = dx * dx + dy * dy;

				dx = ge->fx3 - ge->fx1;
				dy = ge->fy3 - ge->fy1;
				d2m = dx * dx + dy * dy;

				if (d2 > minlen2 && d2m > minlen2) { /* make two straights */
					nge = newgentry (GEF_FLOAT);
					*nge = *ge;
					
					for (i = 0; i < 2; i++) {
						ge->fpoints[i][2] = ge->fpoints[i][0];
						b = nge->fpoints[i][0];
						d = nge->fpoints[i][2] - b;
						nge->fpoints[i][0] = b + 0.1*d;
						nge->fpoints[i][1] = b + 0.9*d;
					}
				}
				for (i = 0; i < 2; i++) { /* make one straight or first of two straights */
					b = ge->prev->fpoints[i][2];
					d = ge->fpoints[i][2] - b;
					ge->fpoints[i][0] = b + 0.1 * d;
					ge->fpoints[i][1] = b + 0.9 * d;
				}
			}
			continue; 
		}

		if (ge->frwd == ge) { /* points to itself, just remove the path completely */
			next = freethisge (ge);
			continue;
		} 

		/* now close the gap by x and y */
		for (i = 0; i < 2; i++) {
			gdouble gap;

			gap = ge->fpoints[i][2] - ge->prev->fpoints[i][2];
			if (fclosegap (ge, ge, i, gap, NULL) != 0.0) {
				gdouble scale, base;

				/* not good, as the last resort just scale the next line */
				gap = ge->fpoints[i][2] - ge->prev->fpoints[i][2];

				nge = ge->frwd;
				base = nge->fpoints[i][2];
				dx = ge->fpoints[i][2] - base;
				if (fabs (dx) < FEPS) continue;

				scale = ((dx-gap) / dx);

				if (nge->type == GE_CURVE) {
					for (k = 0; k < 2; k++) {
						nge->fpoints[i][k] = base + scale * (nge->fpoints[i][k] - base);
					}
				}
				ge->fpoints[i][2] -= gap;
			}
		}

		/* OK, the gap is closed - remove this useless GENTRY */
		freethisge (ge);
	}
#undef TIMESLARGER
}

static void
fnormalizec (struct glyph *g)
{
	struct gentry *ge;
	gint midsame, frontsame, rearsame, i;
	gdouble d, b;

	for (ge = g->entries; ge != 0; ge = ge->next) {
		if (ge->type != GE_CURVE) continue;

		midsame = (fabs (ge->fx1 - ge->fx2) < FEPS && fabs (ge->fy1 - ge->fy2) < FEPS);
		frontsame = (fabs (ge->fx1 - ge->prev->fx3) < FEPS && fabs (ge->fy1 - ge->prev->fy3) < FEPS);
		rearsame = (fabs (ge->fx3 - ge->fx2) < FEPS && fabs (ge->fy3 - ge->fy2) < FEPS);

		if (midsame && (frontsame || rearsame)) {
			/* essentially a line */
			for (i = 0; i < 2; i++) {
				b = ge->prev->fpoints[i][2];
				d = ge->fpoints[i][2] - b;
				ge->fpoints[i][0] = b + 0.1 * d;
				ge->fpoints[i][1] = b + 0.9 * d;
			}
		} else if (frontsame) {
			for (i = 0; i < 2; i++) {
				b = ge->prev->fpoints[i][2];
				d = ge->fpoints[i][1] - b;
				ge->fpoints[i][0] = b + 0.01 * d;
			}
		} else if (rearsame) {
			for(i = 0; i < 2; i++) {
				b = ge->fpoints[i][2];
				d = ge->fpoints[i][0] - b;
				ge->fpoints[i][1] = b + 0.01 * d;
			}
		} else
			continue;
	}
}

static gint
fcrossrays (struct gentry *ge1, struct gentry *ge2, gdouble *max1, gdouble *max2)
{
	struct ray {
		gdouble x1, y1, x2, y2;
		gint isvert;
		gdouble k, b; /* lines are represented as y = k*x + b */
		gdouble *maxp;
	} ray [3];
	gdouble x, y;
	gint i;

	ray[0].x1 = ge1->prev->fx3;
	ray[0].y1 = ge1->prev->fy3;
	ray[0].x2 = ge1->fx1;
	ray[0].y2 = ge1->fy1;
	ray[0].maxp = max1;

	ray[1].x1 = ge2->fx3;
	ray[1].y1 = ge2->fy3;
	ray[1].x2 = ge2->fx2;
	ray[1].y2 = ge2->fy2;
	ray[1].maxp = max2;

	for (i = 0; i < 2; i++) {
		if(ray[i].x1 == ray[i].x2) {
			ray[i].isvert = 1;
		} else {
			ray[i].isvert = 0;
			ray[i].k = (ray[i].y2 - ray[i].y1) / (ray[i].x2 - ray[i].x1);
			ray[i].b = ray[i].y2 - ray[i].k * ray[i].x2;
		}
	}

	if (ray[0].isvert && ray[1].isvert) return 0; /* both vertical, don't cross */

	if (ray[1].isvert) {
		ray[2] = ray[0]; /* exchange them */
		ray[0] = ray[1];
		ray[1] = ray[2];
	}

	if (ray[0].isvert) {
		x = ray[0].x1;
	} else {
		if (fabs (ray[0].k - ray[1].k) < FEPS) return 0; /* parallel lines */
		x = (ray[1].b - ray[0].b) / (ray[0].k - ray[1].k) ;
	}
	y = ray[1].k * x + ray[1].b;

	for (i = 0; i < 2; i++) {
		if (ray[i].isvert)
			*ray[i].maxp = (y - ray[i].y1) / (ray[i].y2 - ray[i].y1);
		else
			*ray[i].maxp = (x - ray[i].x1) / (ray[i].x2 - ray[i].x1);
		/* check if wrong sides of rays cross */
		if (*ray[i].maxp < 0) return 0;
	}
	return 1;
}

static gint
fgetcvdir (struct gentry *ge)
{
	gdouble a, b;
	gdouble k, k1, k2;
	gint dir = 0;

	if (!(ge->flags & GEF_FLOAT)) {
		g_warning ("fgetcvdir(%p) on int entry\n", ge);
		return 0;
	}

	a = ge->fy3 - ge->prev->fy3;
	b = ge->fx3 - ge->prev->fx3;
	k = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ( b / a));
	a = ge->fy1 - ge->prev->fy3;
	b = ge->fx1 - ge->prev->fx3;
	k1 = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ( b / a));
	a = ge->fy3 - ge->fy2;
	b = ge->fx3 - ge->fx2;
	k2 = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ( b / a));

	if (k1 < k) dir |= CVDIR_FUP;
	else if (k1 > k) dir |= CVDIR_FDOWN;
	else dir |= CVDIR_FEQUAL;

	if (k2 > k) dir |= CVDIR_RUP;
	else if (k2 < k) dir |= CVDIR_RDOWN;
	else dir |= CVDIR_REQUAL;

	return dir;
}

static gint
fckjoinedcv (struct glyph *g, gdouble t, struct gentry *nge, struct gentry *old1, struct gentry *old2, gdouble k)
{
	struct gentry *oge;
	gdouble ot;
	gdouble off;
	gdouble lim;
	gint i;

	if (old2 == 0) {
		oge = old1;
		ot = t;
	} else if (t <= k && k!=0.) {
		oge = old1;
		ot = t / k;
	} else {
		oge = old2;
		ot = (t - k) / (1. - k);
	}

	for (i = 0; i < 2; i++) {
		/* permitted tolerance is 5% */
		lim = fabs (nge->fpoints[i][2] - nge->prev->fpoints[i][2]) * 0.05;

		if (lim < 3.) lim = 3.; /* for small curves the tolerance is higher */
		if (lim > 10.) lim = 10.; /* for big curves the tolerance is limited anyway */

		off = fabs (fcvval (nge, i, t) - fcvval (oge, i, ot));

		if (off > lim) {
			return 1;
		}
	}
	return 0;
}

static void
fixendpath (struct gentry *ge)
{
	struct gentry *mge;

	mge = ge->frwd->prev;
	if (mge->type == GE_MOVE) {
		if (ge->flags & GEF_FLOAT) {
			mge->fx3 = ge->fx3;
			mge->fy3 = ge->fy3;
		} else {
			mge->ix3 = ge->ix3;
			mge->iy3 = ge->iy3;
		}
	}
}

static gint
iround (gdouble val)
{
	return (gint) (val > 0 ? val + 0.5 : val - 0.5);
}

static void
fixcvdir (struct gentry *ge, gint dir)
{
	gint a, b, c, d;
	gdouble kk, kk1, kk2;
	gint changed;
	gint fdir, rdir;

	if (ge->flags & GEF_FLOAT) {
		g_warning ("fixcvdir (%p) on floating entry\n", ge);
		return;
	}

	fdir = (dir & CVDIR_FRONT) - CVDIR_FEQUAL;
	if ((dir & CVDIR_REAR) == CVDIR_RSAME) rdir = fdir; /* we need only isign, exact value doesn't matter */
	else rdir = (dir & CVDIR_REAR) - CVDIR_REQUAL;

	fixcvends (ge);

	c = isign (ge->ix3 - ge->prev->ix3);	/* note the direction of curve */
	d = isign (ge->iy3 - ge->prev->iy3);

	a = ge->iy3 - ge->prev->iy3;
	b = ge->ix3 - ge->prev->ix3;
	kk = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ((gdouble) b / (gdouble) a));
	a = ge->iy1 - ge->prev->iy3;
	b = ge->ix1 - ge->prev->ix3;
	kk1 = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ((gdouble) b / (gdouble) a));
	a = ge->iy3 - ge->iy2;
	b = ge->ix3 - ge->ix2;
	kk2 = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ((gdouble) b / (gdouble) a));

	changed = 1;
	while (changed) {
		changed = 0;

		if (fdir > 0) {
			if (kk1 > kk) {	/* the front end has problems */
				if (c * (ge->ix1 - ge->prev->ix3) > 0) {
					ge->ix1 -= c;
					changed = 1;
				} if (d * (ge->iy2 - ge->iy1) > 0) {
					ge->iy1 += d;
					changed = 1;
				}
				/* recalculate the coefficients */
				a = ge->iy3 - ge->prev->iy3;
				b = ge->ix3 - ge->prev->ix3;
				kk = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ((gdouble) b / (gdouble) a));
				a = ge->iy1 - ge->prev->iy3;
				b = ge->ix1 - ge->prev->ix3;
				kk1 = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ((gdouble) b / (gdouble) a));
			}
		} else if (fdir < 0) {
			if (kk1 < kk) {	/* the front end has problems */
				if (c * (ge->ix2 - ge->ix1) > 0) {
					ge->ix1 += c;
					changed = 1;
				} if (d * (ge->iy1 - ge->prev->iy3) > 0) {
					ge->iy1 -= d;
					changed = 1;
				}
				/* recalculate the coefficients */
				a = ge->iy1 - ge->prev->iy3;
				b = ge->ix1 - ge->prev->ix3;
				kk1 = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ((gdouble) b / (gdouble) a));
				a = ge->iy3 - ge->prev->iy3;
				b = ge->ix3 - ge->prev->ix3;
				kk = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ((gdouble) b / (gdouble) a));
			}
		}
		if (rdir > 0) {
			if (kk2 < kk) {	/* the rear end has problems */
				if (c * (ge->ix2 - ge->ix1) > 0) {
					ge->ix2 -= c;
					changed = 1;
				} if (d * (ge->iy3 - ge->iy2) > 0) {
					ge->iy2 += d;
					changed = 1;
				}
				/* recalculate the coefficients */
				a = ge->iy3 - ge->prev->iy3;
				b = ge->ix3 - ge->prev->ix3;
				kk = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ((gdouble) b / (gdouble) a));
				a = ge->iy3 - ge->iy2;
				b = ge->ix3 - ge->ix2;
				kk2 = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ((gdouble) b / (gdouble) a));
			}
		} else if (rdir < 0) {
			if (kk2 > kk) {	/* the rear end has problems */
				if (c * (ge->ix3 - ge->ix2) > 0) {
					ge->ix2 += c;
					changed = 1;
				} if (d * (ge->iy2 - ge->iy1) > 0) {
					ge->iy2 -= d;
					changed = 1;
				}
				/* recalculate the coefficients */
				a = ge->iy3 - ge->prev->iy3;
				b = ge->ix3 - ge->prev->ix3;
				kk = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ((gdouble) b / (gdouble) a));
				a = ge->iy3 - ge->iy2;
				b = ge->ix3 - ge->ix2;
				kk2 = fabs (a == 0 ? (b == 0 ? 1. : 100000.) : ((gdouble) b / (gdouble) a));
			}
		}
	}
	fixcvends (ge);
}

static void
fixcvends (struct gentry *ge)
{
	gint dx, dy;
	gint x0, y0, x1, y1, x2, y2, x3, y3;

	if (ge->type != GE_CURVE) return;

	if (ge->flags & GEF_FLOAT) {
		g_warning ("fixcvends (%p) on floating entry\n", ge);
		return;
	}

	x0 = ge->prev->ix3;
	y0 = ge->prev->iy3;
	x1 = ge->ix1;
	y1 = ge->iy1;
	x2 = ge->ix2;
	y2 = ge->iy2;
	x3 = ge->ix3;
	y3 = ge->iy3;


	/* look at the start of the curve */
	if (x1 == x0 && y1 == y0) {
		dx = x2 - x1;
		dy = y2 - y1;

		if ((dx == 0 && dy == 0) || (x2 == x3 && y2 == y3)) {
			/* Oops, we actually have a straight line */
			/*
			 * if it's small, we hope that it will get optimized
			 * later
			 */
			if (abs (x3 - x0) <= 2 || abs (y3 - y0) <= 2) {
				ge->ix1 = x3;
				ge->iy1 = y3;
				ge->ix2 = x0;
				ge->iy2 = y0;
			} else {/* just make it a line */
				ge->type = GE_LINE;
			}
		} else {
			if (abs (dx) < 4 && abs (dy) < 4) {	/* consider it very small */
				ge->ix1 = x2;
				ge->iy1 = y2;
			} else if (abs (dx) < 8 && abs (dy) < 8) {	/* consider it small */
				ge->ix1 += dx / 2;
				ge->iy1 += dy / 2;
			} else {
				ge->ix1 += dx / 4;
				ge->iy1 += dy / 4;
			}
			/* make sure that it's still on the same side */
			if (abs (x3 - x0) * abs (dy) < abs (y3 - y0) * abs (dx)) {
				if (abs (x3 - x0) * abs (ge->iy1 - y0) > abs (y3 - y0) * abs (ge->ix1 - x0))
					ge->ix1 += isign (dx);
			} else {
				if (abs (x3 - x0) * abs (ge->iy1 - y0) < abs (y3 - y0) * abs (ge->ix1 - x0))
					ge->iy1 += isign (dy);
			}

			ge->ix2 += (x3 - x2) / 8;
			ge->iy2 += (y3 - y2) / 8;
			/* make sure that it's still on the same side */
			if (abs (x3 - x0) * abs (y3 - y2) < abs (y3 - y0) * abs (x3 - x2)) {
				if (abs (x3 - x0) * abs (y3 - ge->iy2) > abs (y3 - y0) * abs (x3 - ge->ix2))
					ge->iy1 -= isign (y3 - y2);
			} else {
				if (abs (x3 - x0) * abs (y3 - ge->iy2) < abs (y3 - y0) * abs (x3 - ge->ix2))
					ge->ix1 -= isign (x3 - x2);
			}

		}
	} else if (x2 == x3 && y2 == y3) {
		dx = x1 - x2;
		dy = y1 - y2;

		if (dx == 0 && dy == 0) {
			/* Oops, we actually have a straight line */
			/*
			 * if it's small, we hope that it will get optimized
			 * later
			 */
			if (abs (x3 - x0) <= 2 || abs (y3 - y0) <= 2) {
				ge->ix1 = x3;
				ge->iy1 = y3;
				ge->ix2 = x0;
				ge->iy2 = y0;
			} else {/* just make it a line */
				ge->type = GE_LINE;
			}
		} else {
			if (abs (dx) < 4 && abs (dy) < 4) {	/* consider it very small */
				ge->ix2 = x1;
				ge->iy2 = y1;
			} else if (abs (dx) < 8 && abs (dy) < 8) {	/* consider it small */
				ge->ix2 += dx / 2;
				ge->iy2 += dy / 2;
			} else {
				ge->ix2 += dx / 4;
				ge->iy2 += dy / 4;
			}
			/* make sure that it's still on the same side */
			if (abs (x3 - x0) * abs (dy) < abs (y3 - y0) * abs (dx)) {
				if (abs (x3 - x0) * abs (ge->iy2 - y3) > abs (y3 - y0) * abs (ge->ix2 - x3))
					ge->ix2 += isign (dx);
			} else {
				if (abs (x3 - x0) * abs (ge->iy2 - y3) < abs (y3 - y0) * abs (ge->ix2 - x3))
					ge->iy2 += isign (dy);
			}

			ge->ix1 += (x0 - x1) / 8;
			ge->iy1 += (y0 - y1) / 8;
			/* make sure that it's still on the same side */
			if (abs (x3 - x0) * abs (y0 - y1) < abs (y3 - y0) * abs (x0 - x1)) {
				if (abs (x3 - x0) * abs (y0 - ge->iy1) > abs(y3 - y0) * abs (x0 - ge->ix1))
					ge->iy1 -= isign (y0 - y1);
			} else {
				if (abs (x3 - x0) * abs (y0 - ge->iy1) < abs (y3 - y0) * abs (x0 - ge->ix1))
					ge->ix1 -= isign (x0 - x1);
			}
		}
	}
}

static gdouble
fcvval (struct gentry *ge, gint axis, gdouble t)
{
	gdouble t2, mt, mt2;

	/* val = A*(1-t)^3 + 3*B*(1-t)^2*t + 3*C*(1-t)*t^2 + D*t^3 */
	t2 = t * t;
	mt = 1 - t;
	mt2 = mt * mt;
	
	return ge->prev->fpoints[axis][2] * mt2 * mt + 3 * (ge->fpoints[axis][0] * mt2 * t + ge->fpoints[axis][1] * mt * t2)	+ ge->fpoints[axis][2] * t * t2;
}

void
findblues (struct glyph_face *gf)
{
	/* hystograms for upper and lower zones */
	gshort hystl[MAXHYST];
	gshort hystu[MAXHYST];
	gshort zuhyst[MAXHYST];
	gshort zlhyst[MAXHYST];
	gint nchars;
	gint i, j, k, w, max;
	struct gentry *ge;
	struct glyph *g;
	gdouble ang;

	/* find the lowest and highest points of glyphs */
	/* and by the way build the values for FontBBox */
	/* and build the hystogram for the ItalicAngle */

	/* re-use hystl for the hystogram of italic angle */

	bbox[0] = bbox[1] = 5000;
	bbox[2] = bbox[3] = -5000;

	for (i = 0; i < MAXHYST; i++) hystl[i] = 0;

	nchars = 0;

	for (g = gf->glyph; g != NULL; g = g->next) {
		nchars++;

		g->rymin = 5000;
		g->rymax = -5000;
		for (ge = g->entries; ge != 0; ge = ge->next) {
			if (ge->type == GE_LINE) {
				j = ge->iy3 - ge->prev->iy3;
				k = ge->ix3 - ge->prev->ix3;
				if (j > 0) ang = atan2 (-k, j) * 180.0 / M_PI;
				else ang = atan2 (k, -j) * 180.0 / M_PI;

				k /= 100;
				j /= 100;
				if (ang > -45.0 && ang < 45.0) {
					/*
					 * be careful to not overflow
					 * the counter
					 */
					hystl[HYSTBASE + (int) (ang * 10.0)] += (k * k + j * j) / 4;
				}
				if (ge->iy3 == ge->prev->iy3) {
					if (ge->iy3 <= g->rymin) {
						g->rymin = ge->iy3;
						g->flatymin = 1;
					}
					if (ge->iy3 >= g->rymax) {
						g->rymax = ge->iy3;
						g->flatymax = 1;
					}
				} else {
					if (ge->iy3 < g->rymin) {
						g->rymin = ge->iy3;
						g->flatymin = 0;
					}
					if (ge->iy3 > g->rymax) {
						g->rymax = ge->iy3;
						g->flatymax = 0;
					}
				}
			} else if (ge->type == GE_CURVE) {
				if (ge->iy3 < g->rymin) {
					g->rymin = ge->iy3;
					g->flatymin = 0;
				}
				if (ge->iy3 > g->rymax) {
					g->rymax = ge->iy3;
					g->flatymax = 0;
				}
			}
			if (ge->type == GE_LINE || ge->type == GE_CURVE) {
				if (ge->ix3 < bbox[0])
					bbox[0] = ge->ix3;
				if (ge->ix3 > bbox[2])
					bbox[2] = ge->ix3;
				if (ge->iy3 < bbox[1])
					bbox[1] = ge->iy3;
				if (ge->iy3 > bbox[3])
					bbox[3] = ge->iy3;
			}
		}
	}

	/* get the most popular angle */
	max = 0;
	w = 0;
	for (i = 0; i < MAXHYST; i++) {
		if (hystl[i] > w) {
			w = hystl[i];
			max = i;
		}
	}
	ang = (double) (max - HYSTBASE) / 10.0;
	if (italic_angle == 0.0) italic_angle = ang;

	/* build the hystogram of the lower points */
	for (i = 0; i < MAXHYST; i++) hystl[i] = 0;

	for (g = gf->glyph; g != NULL; g = g->next) {
		if (g->rymin + HYSTBASE >= 0 && g->rymin < MAXHYST - HYSTBASE) {
			hystl[g->rymin + HYSTBASE]++;
		}
	}

	/* build the hystogram of the upper points */
	for (i = 0; i < MAXHYST; i++) hystu[i] = 0;

	for (g = gf->glyph; g != NULL; g = g->next) {
		if (g->rymax + HYSTBASE >= 0 && g->rymax < MAXHYST - HYSTBASE) {
			hystu[g->rymax + HYSTBASE]++;
		}
	}

	/* build the hystogram of all the possible lower zones with max width */
	for (i = 0; i < MAXHYST; i++) zlhyst[i] = 0;

	for (i = 0; i <= MAXHYST - MAXBLUEWIDTH; i++) {
		for (j = 0; j < MAXBLUEWIDTH; j++)
			zlhyst[i] += hystl[i + j];
	}

	/* build the hystogram of all the possible upper zones with max width */
	for (i = 0; i < MAXHYST; i++)
		zuhyst[i] = 0;

	for (i = 0; i <= MAXHYST - MAXBLUEWIDTH; i++) {
		for (j = 0; j < MAXBLUEWIDTH; j++)
			zuhyst[i] += hystu[i + j];
	}

	/* find the baseline */
	w = bestblue(zlhyst, hystl, zuhyst, &bluevalues[0]);

	if (w == 0)		/* no baseline, something weird */
		return;

	/* find the upper zones */
	for (nblues = 2; nblues < 14; nblues += 2) {
		w = bestblue(zuhyst, hystu, zlhyst, &bluevalues[nblues]);

		if (w * 20 < nchars)
			break;	/* don't save this zone */
	}

	/* find the lower zones */
	for (notherb = 0; notherb < 10; notherb += 2) {
		w = bestblue(zlhyst, hystl, zuhyst, &otherblues[notherb]);

		if (w * 20 < nchars)
			break;	/* don't save this zone */
	}
}

static int
bestblue (gshort *zhyst, gshort *physt, gshort *ozhyst, gint *bluetab)
{
	gint i, j, w, max, ind, first, last;

	/* find the highest point in the zones hystogram */
	/* if we have a plateau, take its center */
	/* if we have multiple peaks, take the first one */

	max = -1;
	first = last = -10;
	for (i = 0; i <= MAXHYST - MAXBLUEWIDTH; i++) {
		w = zhyst[i];
		if (w > max) {
			first = last = i;
			max = w;
		} else if (w == max) {
			if (last == i - 1)
				last = i;
		}
	}
	ind = (first + last) / 2;

	if (max == 0)		/* no zones left */
		return 0;

	/* now we reuse `first' and `last' as inclusive borders of the zone */
	first = ind;
	last = ind + (MAXBLUEWIDTH - 1);

	/* our maximal width is far too big, so we try to make it narrower */
	w = max;
	j = (w & 1);		/* a pseudo-random bit */
	while (1) {
		while (physt[first] == 0) first++;
		while (physt[last] == 0) last--;
		if (last - first < (MAXBLUEWIDTH * 2 / 3) || (max - w) * 10 > max) break;

		if (physt[first] < physt[last] || (physt[first] == physt[last] && j)) {
			if (physt[first] * 20 > w)	/* if weight is >5%, stop */
				break;
			w -= physt[first];
			first++;
			j = 0;
		} else {
			if (physt[last] * 20 > w)	/* if weight is >5%, stop */
				break;
			w -= physt[last];
			last--;
			j = 1;
		}
	}

	/* save our zone */
	bluetab[0] = first - HYSTBASE;
	bluetab[1] = last - HYSTBASE;

	/* invalidate all the zones overlapping with this one */
	/* the constant of 2 is determined by the default value of BlueFuzz */
	for (i = first - (MAXBLUEWIDTH - 1) - 2; i <= last + 2; i++) {
		if (i >= 0 && i < MAXHYST) {
			zhyst[i] = 0;
			ozhyst[i] = 0;
		}
	}

	return w;
}

void
buildstems (struct glyph *g)
{
	struct stem hs[MAX_STEMS], vs[MAX_STEMS];			/* temporary working storage */
	struct stem *sp;
	struct gentry *ge, *nge, *pge;
	gshort hs_pairs[MAX_STEMS], vs_pairs[MAX_STEMS];	/* best pairs for these stems */
	gint nx, ny;
	gint ovalue;
	gint totals, grp, lastgrp;

	g->nhs = g->nvs = 0;
	memset (hs, 0, sizeof (hs));
	memset (vs, 0, sizeof (vs));

	/* first search the whole character for possible stem points */
	for (ge = g->entries; ge != 0; ge = ge->next) {
		if (ge->type == GE_CURVE) {
			/*
			 * SURPRISE! 
			 * We consider the stems bound by the
			 * H/V ends of the curves as flat ones.
			 *
			 * But we don't include the point on the
			 * other end into the range.
			 */

			/* first check the beginning of curve */
			/* if it is horizontal, add a hstem */
			if (ge->iy1 == ge->prev->iy3) {
				hs[g->nhs].value = ge->iy1;

				if (ge->ix1 < ge->prev->ix3) hs[g->nhs].flags = ST_FLAT | ST_UP;
				else hs[g->nhs].flags = ST_FLAT;

				hs[g->nhs].origin = ge->prev->ix3;
				hs[g->nhs].ge = ge;

				if (ge->ix1 < ge->prev->ix3) {
					hs[g->nhs].from = ge->ix1 + 1;
					hs[g->nhs].to = ge->prev->ix3;
					if (hs[g->nhs].from > hs[g->nhs].to) hs[g->nhs].from--;
				} else {
					hs[g->nhs].from = ge->prev->ix3;
					hs[g->nhs].to = ge->ix1 - 1;
					if (hs[g->nhs].from > hs[g->nhs].to) hs[g->nhs].to++;
				}
				if (ge->ix1 != ge->prev->ix3) g->nhs++;
			}
			/* if it is vertical, add a vstem */
			else if (ge->ix1 == ge->prev->ix3) {
				vs[g->nvs].value = ge->ix1;

				if (ge->iy1 > ge->prev->iy3) vs[g->nvs].flags = ST_FLAT | ST_UP;
				else vs[g->nvs].flags = ST_FLAT;

				vs[g->nvs].origin = ge->prev->iy3;
				vs[g->nvs].ge = ge;

				if (ge->iy1 < ge->prev->iy3) {
					vs[g->nvs].from = ge->iy1 + 1;
					vs[g->nvs].to = ge->prev->iy3;
					if (vs[g->nvs].from > vs[g->nvs].to) vs[g->nvs].from--;
				} else {
					vs[g->nvs].from = ge->prev->iy3;
					vs[g->nvs].to = ge->iy1 - 1;
					if (vs[g->nvs].from > vs[g->nvs].to) vs[g->nvs].to++;
				}

				if (ge->iy1 != ge->prev->iy3) g->nvs++;
			}
			/* then check the end of curve */
			/* if it is horizontal, add a hstem */
			if (ge->iy3 == ge->iy2) {
				hs[g->nhs].value = ge->iy3;

				if (ge->ix3 < ge->ix2) hs[g->nhs].flags = ST_FLAT | ST_UP;
				else hs[g->nhs].flags = ST_FLAT;

				hs[g->nhs].origin = ge->ix3;
				hs[g->nhs].ge = ge->frwd;

				if (ge->ix3 < ge->ix2) {
					hs[g->nhs].from = ge->ix3;
					hs[g->nhs].to = ge->ix2 - 1;
					if (hs[g->nhs].from > hs[g->nhs].to) hs[g->nhs].to++;
				} else {
					hs[g->nhs].from = ge->ix2 + 1;
					hs[g->nhs].to = ge->ix3;
					if (hs[g->nhs].from > hs[g->nhs].to) hs[g->nhs].from--;
				}

				if (ge->ix3 != ge->ix2) g->nhs++;
			}
			/* if it is vertical, add a vstem */
			else if (ge->ix3 == ge->ix2) {
				vs[g->nvs].value = ge->ix3;

				if (ge->iy3 > ge->iy2) vs[g->nvs].flags = ST_FLAT | ST_UP;
				else vs[g->nvs].flags = ST_FLAT;

				vs[g->nvs].origin = ge->iy3;
				vs[g->nvs].ge = ge->frwd;

				if (ge->iy3 < ge->iy2) {
					vs[g->nvs].from = ge->iy3;
					vs[g->nvs].to = ge->iy2 - 1;
					if (vs[g->nvs].from > vs[g->nvs].to) vs[g->nvs].to++;
				} else {
					vs[g->nvs].from = ge->iy2 + 1;
					vs[g->nvs].to = ge->iy3;
					if (vs[g->nvs].from > vs[g->nvs].to) vs[g->nvs].from--;
				}

				if (ge->iy3 != ge->iy2) g->nvs++;
			} else {
				/*
				 * check the end of curve for a not smooth
				 * local extremum
				 */
				nge = ge->frwd;

				if (nge == 0)
					continue;
				else if (nge->type == GE_LINE) {
					nx = nge->ix3;
					ny = nge->iy3;
				} else if (nge->type == GE_CURVE) {
					nx = nge->ix1;
					ny = nge->iy1;
				} else
					continue;

				/* check for vertical extremums */
				if ((ge->iy3 > ge->iy2 && ge->iy3 > ny) || (ge->iy3 < ge->iy2 && ge->iy3 < ny)) {
					hs[g->nhs].value = ge->iy3;
					hs[g->nhs].from = hs[g->nhs].to = hs[g->nhs].origin = ge->ix3;
					hs[g->nhs].ge = ge->frwd;

					if (ge->ix3 < ge->ix2 || nx < ge->ix3) hs[g->nhs].flags = ST_UP;
					else hs[g->nhs].flags = 0;

					if (ge->ix3 != ge->ix2 || nx != ge->ix3) g->nhs++;
				}
				/*
				 * the same point may be both horizontal and
				 * vertical extremum
				 */
				/* check for horizontal extremums */
				if ((ge->ix3 > ge->ix2 && ge->ix3 > nx) || (ge->ix3 < ge->ix2 && ge->ix3 < nx)) {
					vs[g->nvs].value = ge->ix3;
					vs[g->nvs].from = vs[g->nvs].to = vs[g->nvs].origin = ge->iy3;
					vs[g->nvs].ge = ge->frwd;

					if (ge->iy3 > ge->iy2 || ny > ge->iy3) vs[g->nvs].flags = ST_UP;
					else vs[g->nvs].flags = 0;

					if (ge->iy3 != ge->iy2 || ny != ge->iy3) g->nvs++;
				}
			}

		} else if (ge->type == GE_LINE) {
			nge = ge->frwd;

			/* if it is horizontal, add a hstem */
			/* and the ends as vstems if they brace the line */
			if (ge->iy3 == ge->prev->iy3 && ge->ix3 != ge->prev->ix3) {
				hs[g->nhs].value = ge->iy3;
				if (ge->ix3 < ge->prev->ix3) {
					hs[g->nhs].flags = ST_FLAT | ST_UP;
					hs[g->nhs].from = ge->ix3;
					hs[g->nhs].to = ge->prev->ix3;
				} else {
					hs[g->nhs].flags = ST_FLAT;
					hs[g->nhs].from = ge->prev->ix3;
					hs[g->nhs].to = ge->ix3;
				}
				hs[g->nhs].origin = ge->ix3;
				hs[g->nhs].ge = ge->frwd;

				pge = ge->bkwd;

				/* add beginning as vstem */
				vs[g->nvs].value = pge->ix3;
				vs[g->nvs].origin = vs[g->nvs].from = vs[g->nvs].to = pge->iy3;
				vs[g->nvs].ge = ge;

				if (pge->type==GE_CURVE) ovalue=pge->iy2;
				else ovalue=pge->prev->iy3;

				if (pge->iy3 > ovalue) vs[g->nvs].flags = ST_UP | ST_END;
				else if (pge->iy3 < ovalue) vs[g->nvs].flags = ST_END;
				else vs[g->nvs].flags = 0;

				if (vs[g->nvs].flags != 0) g->nvs++;

				/* add end as vstem */
				vs[g->nvs].value = ge->ix3;
				vs[g->nvs].origin = vs[g->nvs].from = vs[g->nvs].to = ge->iy3;
				vs[g->nvs].ge = ge->frwd;

				if (nge->type==GE_CURVE) ovalue=nge->iy1;
				else ovalue=nge->iy3;

				if (ovalue > ge->iy3) vs[g->nvs].flags = ST_UP | ST_END;
				else if (ovalue < ge->iy3) vs[g->nvs].flags = ST_END;
				else vs[g->nvs].flags = 0;

				if (vs[g->nvs].flags != 0) g->nvs++;

				g->nhs++;
			}
			/* if it is vertical, add a vstem */
			/* and the ends as hstems if they brace the line  */
			else if (ge->ix3 == ge->prev->ix3 && ge->iy3 != ge->prev->iy3) {
				vs[g->nvs].value = ge->ix3;
				if (ge->iy3 > ge->prev->iy3) {
					vs[g->nvs].flags = ST_FLAT | ST_UP;
					vs[g->nvs].from = ge->prev->iy3;
					vs[g->nvs].to = ge->iy3;
				} else {
					vs[g->nvs].flags = ST_FLAT;
					vs[g->nvs].from = ge->iy3;
					vs[g->nvs].to = ge->prev->iy3;
				}
				vs[g->nvs].origin = ge->iy3;
				vs[g->nvs].ge = ge->frwd;

				pge = ge->bkwd;

				/* add beginning as hstem */
				hs[g->nhs].value = pge->iy3;
				hs[g->nhs].origin = hs[g->nhs].from = hs[g->nhs].to = pge->ix3;
				hs[g->nhs].ge = ge;

				if (pge->type==GE_CURVE) ovalue=pge->ix2;
				else ovalue=pge->prev->ix3;

				if (pge->ix3 < ovalue) hs[g->nhs].flags = ST_UP | ST_END;
				else if (pge->ix3 > ovalue) hs[g->nhs].flags = ST_END;
				else hs[g->nhs].flags = 0;

				if (hs[g->nhs].flags != 0) g->nhs++;

				/* add end as hstem */
				hs[g->nhs].value = ge->iy3;
				hs[g->nhs].origin = hs[g->nhs].from = hs[g->nhs].to = ge->ix3;
				hs[g->nhs].ge = ge->frwd;

				if (nge->type==GE_CURVE) ovalue=nge->ix1;
				else ovalue=nge->ix3;

				if (ovalue < ge->ix3) hs[g->nhs].flags = ST_UP | ST_END;
				else if (ovalue > ge->ix3) hs[g->nhs].flags = ST_END;
				else hs[g->nhs].flags = 0;

				if (hs[g->nhs].flags != 0) g->nhs++;

				g->nvs++;
			}
			/*
			 * check the end of line for a not smooth local
			 * extremum
			 */
			nge = ge->frwd;

			if (nge == 0)
				continue;
			else if (nge->type == GE_LINE) {
				nx = nge->ix3;
				ny = nge->iy3;
			} else if (nge->type == GE_CURVE) {
				nx = nge->ix1;
				ny = nge->iy1;
			} else
				continue;

			/* check for vertical extremums */
			if ((ge->iy3 > ge->prev->iy3 && ge->iy3 > ny) || (ge->iy3 < ge->prev->iy3 && ge->iy3 < ny)) {
				hs[g->nhs].value = ge->iy3;
				hs[g->nhs].from = hs[g->nhs].to = hs[g->nhs].origin = ge->ix3;
				hs[g->nhs].ge = ge->frwd;

				if (ge->ix3 < ge->prev->ix3 || nx < ge->ix3) hs[g->nhs].flags = ST_UP;
				else hs[g->nhs].flags = 0;

				if (ge->ix3 != ge->prev->ix3 || nx != ge->ix3) g->nhs++;
			}
			/*
			 * the same point may be both horizontal and vertical
			 * extremum
			 */
			/* check for horizontal extremums */
			if ((ge->ix3 > ge->prev->ix3 && ge->ix3 > nx) || (ge->ix3 < ge->prev->ix3 && ge->ix3 < nx)) {
				vs[g->nvs].value = ge->ix3;
				vs[g->nvs].from = vs[g->nvs].to = vs[g->nvs].origin = ge->iy3;
				vs[g->nvs].ge = ge->frwd;

				if (ge->iy3 > ge->prev->iy3 || ny > ge->iy3) vs[g->nvs].flags = ST_UP;
				else vs[g->nvs].flags = 0;

				if (ge->iy3 != ge->prev->iy3 || ny != ge->iy3) g->nvs++;
			}
		}
	}

	g->nhs = addbluestems (hs, g->nhs);
	sortstems (hs, g->nhs);
	sortstems (vs, g->nvs);

	/* find the stems interacting with the Blue Zones */
	markbluestems (hs, g->nhs);

	joinsubstems (hs, hs_pairs, g->nhs, 1);
	uniformstems (hs, hs_pairs, g->nhs);

	joinsubstems (vs, vs_pairs, g->nvs, 0);

	groupsubstems (g, hs, hs_pairs, g->nhs, vs, vs_pairs, g->nvs);

	g->nhs = joinmainstems (hs, g->nhs, 1);
	g->nvs = joinmainstems (vs, g->nvs, 0);

	if(g->nhs > 0) {
		sp = g_new0 (struct stem, g->nhs);
		g->hstems = sp;
		memcpy (sp, hs, sizeof(struct stem) * g->nhs);
	} else
		g->hstems = 0;

	if(g->nvs > 0) {
		sp = g_new0 (struct stem, g->nvs);
		g->vstems = sp;
		memcpy (sp, vs, sizeof(struct stem) * g->nvs);
	} else
		g->vstems = 0;

	/* now check that the stems won't overflow the interpreter's stem stack:
	 * some interpreters (like X11) push the stems on each change into
	 * stack and pop them only after the whole glyphs is completed.
	 */

	totals = (g->nhs + g->nvs) / 2; /* we count whole stems, not halves */
	lastgrp = -1;

	for (ge = g->entries; ge != 0; ge = ge->next) {
		grp=ge->stemid;
		if (grp >= 0 && grp != lastgrp)  {
			if(grp==0) totals += g->nsbs[0];
			else totals += g->nsbs[grp] - g->nsbs[grp-1];

			lastgrp = grp;
		}
	}

	/* be on the safe side, check for >= , not > */
	if (totals >= MAX_STEMDEPTH) {  /* oops, too deep */
		if (g->nsg > 0) {
			for (ge = g->entries; ge != 0; ge = ge->next) ge->stemid = -1;
			g_free (g->sbstems);
			g->sbstems = 0;
			g_free (g->nsbs);
			g->nsbs = 0;
			g->nsg = 0;
		}
	}

	/* now check if there are too many main stems */
	totals = (g->nhs+g->nvs) / 2; /* we count whole stems, not halves */
	if (totals >= MAX_STEMDEPTH) { 
		/* even worse, too much of non-substituted stems */
		if(g->vstems) {
			g_free (g->vstems);
			g->vstems = 0;
			g->nvs = 0;
		}
		if (g->hstems) {
			g_free(g->hstems);
			g->hstems = 0;
			g->nhs = 0;
		}
	}
}

void
stemstatistics (struct glyph *glyph)
{
#define MINDIST	10 /* minimal distance between the widths */
	gint hyst[MAXHYST + MINDIST * 2];
	gint best[12];
	gint i, j, k, w;
	gint ns;
	struct stem *s;
	struct glyph *g;

	/* start with typical stem width */

	/* build the hystogram of horizontal stem widths */
	memset(hyst, 0, sizeof hyst);

	for (g = glyph; g != NULL; g = g->next) {
		s = g->hstems;
		for (j = 0; j < g->nhs; j += 2) {
			if ((s[j].flags | s[j + 1].flags) & ST_END) continue;
			w = s[j + 1].value - s[j].value+1;
			if(w==20) /* split stems should not be counted */
				continue;
			if (w > 0 && w < MAXHYST - 1) {
				/*
				 * handle some fuzz present in
				 * converted fonts
				 */
				hyst[w+MINDIST] += MINDIST-1;
				for (k = 1; k < MINDIST - 1; k++) {
					hyst[w+MINDIST + k] += MINDIST-1-k;
					hyst[w+MINDIST - k] += MINDIST-1-k;
				}
			}
		}
	}

	/* find 12 most frequent values */
	ns = besthyst (hyst + MINDIST, 0, best, 12, MINDIST, &stdhw);

	/* store data in stemsnaph */
	for (i = 0; i < ns; i++) stemsnaph[i] = best[i];
	if (ns < 12) stemsnaph[ns] = 0;

	/* build the hystogram of vertical stem widths */
	memset (hyst, 0, sizeof hyst);

	for (g = glyph; g != NULL; g = g->next) {
		s = g->vstems;
		for (j = 0; j < g->nvs; j += 2) {
			if ((s[j].flags | s[j + 1].flags) & ST_END) continue;
			w = s[j + 1].value - s[j].value+1;
			if (w > 0 && w < MAXHYST - 1) {
				/*
				 * handle some fuzz present in
				 * converted fonts
				 */
				hyst[w + MINDIST] += MINDIST - 1;
				for (k = 1; k < MINDIST - 1; k++) {
					hyst[w + MINDIST + k] += MINDIST - 1 - k;
					hyst[w + MINDIST - k] += MINDIST - 1 - k;
				}
			}
		}
	}

	/* find 12 most frequent values */
	ns = besthyst (hyst + MINDIST, 0, best, 12, MINDIST, &stdvw);

	/* store data in stemsnaph */
	for (i = 0; i < ns; i++) stemsnapv[i] = best[i];
	if (ns < 12) stemsnapv[ns] = 0;

#undef MINDIST
}

static int
addbluestems (struct stem *s, gint n)
{
	gint i;

	for (i = 0; i < nblues && i < 2; i += 2) { /* baseline */
		s[n].value = bluevalues[i];
		s[n].flags = ST_UP | ST_ZONE;
		/* don't overlap with anything */
		s[n].origin = s[n].from = s[n].to = -10000 + i;
		n++;
		s[n].value = bluevalues[i + 1];
		s[n].flags = ST_ZONE;
		/* don't overlap with anything */
		s[n].origin = s[n].from = s[n].to = -10000 + i + 1;
		n++;
	}
	for (i = 2; i < nblues; i += 2) { /* top zones */
		s[n].value = bluevalues[i];
		s[n].flags = ST_UP | ST_ZONE | ST_TOPZONE;
		/* don't overlap with anything */
		s[n].origin = s[n].from = s[n].to = -10000 + i;
		n++;
		s[n].value = bluevalues[i + 1];
		s[n].flags = ST_ZONE | ST_TOPZONE;
		/* don't overlap with anything */
		s[n].origin = s[n].from = s[n].to = -10000 + i + 1;
		n++;
	}
	for (i = 0; i < notherb; i += 2) { /* bottom zones */
		s[n].value = otherblues[i];
		s[n].flags = ST_UP | ST_ZONE;
		/* don't overlap with anything */
		s[n].origin = s[n].from = s[n].to = -10000 + i + nblues;
		n++;
		s[n].value = otherblues[i + 1];
		s[n].flags = ST_ZONE;
		/* don't overlap with anything */
		s[n].origin = s[n].from = s[n].to = -10000 + i + 1 + nblues;
		n++;
	}
	return n;
}

static void
sortstems (struct stem *s, gint n)
{
	struct stem x;
	gint i, j;

	/* a simple sorting */
	/* hm, the ordering criteria are not quite simple :-) 
	 * if the values are tied
	 * ST_UP always goes under not ST_UP
	 * ST_ZONE goes on the most outer side
	 * ST_END goes towards inner side after ST_ZONE
	 * ST_FLAT goes on the inner side
	 */

	for (i = 0; i < n; i++) {
		for (j = i + 1; j < n; j++) {
			if (s[i].value < s[j].value) continue;
			if (s[i].value == s[j].value) {
				if ((s[i].flags & ST_UP) < (s[j].flags & ST_UP)) continue;
				if ((s[i].flags & ST_UP) == (s[j].flags & ST_UP)) {
					if (s[i].flags & ST_UP) {
						if(((s[i].flags & (ST_ZONE | ST_FLAT | ST_END)) ^ ST_FLAT) > ((s[j].flags & (ST_ZONE | ST_FLAT | ST_END)) ^ ST_FLAT))
							continue;
					} else {
						if(((s[i].flags & (ST_ZONE | ST_FLAT | ST_END)) ^ ST_FLAT) < ((s[j].flags & (ST_ZONE | ST_FLAT | ST_END)) ^ ST_FLAT))
							continue;
					}
				}
			}
			x = s[j];
			s[j] = s[i];
			s[i] = x;
		}
	}
}

static void
markbluestems (struct stem *s, gint nold)
{
	gint i, j, a, b, c;

	/*
	 * traverse the list of Blue Values, mark the lowest upper
	 * stem in each bottom zone and the topmost lower stem in
	 * each top zone with ST_BLUE
	 */

	/* top zones */
	for (i = 2; i < nblues; i += 2) {
		a = bluevalues[i];
		b = bluevalues[i + 1];

		for (j = nold - 1; j >= 0; j--) {
			if (s[j].flags & (ST_ZONE | ST_UP | ST_END)) continue;
			c = s[j].value;
			if (c < a) /* too low */
				break;
			if (c <= b) { /* found the topmost stem border */
				/* mark all the stems with the same value */
				/* include ST_END values */
				while (s[j+1].value == c && (s[j + 1].flags & ST_ZONE) == 0) j++;
				s[j].flags |= ST_BLUE;
				for (j--; j >= 0 && s[j].value == c && (s[j].flags & (ST_UP | ST_ZONE)) == 0; j--)
					s[j].flags |= ST_BLUE;
				break;
			}
		}
	}
	/* baseline */
	if (nblues >= 2) {
		a = bluevalues[0];
		b = bluevalues[1];
		for (j = 0; j < nold; j++) {
			if ((s[j].flags & (ST_ZONE | ST_UP | ST_END)) != ST_UP) continue;
			c = s[j].value;
			if (c > b) /* too high */
				break;
			if (c >= a) { /* found the lowest stem border */
				/* mark all the stems with the same value */
				/* include ST_END values */
				while (s[j-1].value == c && (s[j - 1].flags & ST_ZONE) == 0) j--;
				s[j].flags |= ST_BLUE;
				for (j++; j < nold && s[j].value == c && (s[j].flags & (ST_UP | ST_ZONE)) == ST_UP; j++)
					s[j].flags |= ST_BLUE;
				break;
			}
		}
	}
	/* bottom zones: the logic is the same as for baseline */
	for (i = 0; i < notherb; i += 2) {
		a = otherblues[i];
		b = otherblues[i + 1];
		for (j = 0; j < nold; j++) {
			if ((s[j].flags & (ST_UP | ST_ZONE | ST_END)) != ST_UP) continue;
			c = s[j].value;
			if (c > b) /* too high */
				break;
			if (c >= a) { /* found the lowest stem border */
				/* mark all the stems with the same value */
				/* include ST_END values */
				while (s[j-1].value == c && (s[j - 1].flags & ST_ZONE) == 0) j--;
				s[j].flags |= ST_BLUE;
				for (j++; j < nold && s[j].value == c && (s[j].flags & (ST_UP | ST_ZONE)) == ST_UP; j++)
					s[j].flags |= ST_BLUE;
				break;
			}
		}
	}
}

static int
joinmainstems (struct stem *s, gint nold, gint useblues)
{
#define MAX_STACK	1000
	struct stem stack[MAX_STACK];
	gint nstack = 0;
	gint sbottom = 0;
	gint nnew;
	gint i, j, k;
	gint a, b, c, w1, w2, w3;
	gint fw, fd;
	/*
	 * priority of the last found stem: 
	 * 0 - nothing found yet 
	 * 1 - has ST_END in it (one or more) 
	 * 2 - has no ST_END and no ST_FLAT, can override only one stem 
	 *     with priority 1 
	 * 3 - has no ST_END and at least one ST_FLAT, can override one 
	 *     stem with priority 2 or any number of stems with priority 1
	 * 4 (handled separately) - has ST_BLUE, can override anything
	 */
	gint readystem = 0;
	gint pri;
	gint nlps = 0;	/* number of non-committed lowest-priority stems */

	for (i = 0, nnew = 0; i < nold; i++) {
		if (s[i].flags & (ST_UP|ST_ZONE)) {
			if (s[i].flags & ST_BLUE) {
				/* we just HAVE to use this value */
				if (readystem) nnew += 2;
				readystem = 0;

				/* remember the list of Blue zone stems with the same value */
				for (a = i, i++; i < nold && s[a].value == s[i].value && (s[i].flags & ST_BLUE); i++) {
				}
				b = i; /* our range is a <= i < b */
				c = -1; /* index of our best guess up to now */
				pri = 0;
				/* try to find a match, don't cross blue zones */
				for (; i < nold && (s[i].flags & ST_BLUE) == 0; i++) {
					if (s[i].flags & ST_UP) {
						if (s[i].flags & ST_TOPZONE) break;
						else continue;
					}
					for (j = a; j < b; j++) {
						if (!stemoverlap (&s[j], &s[i])) continue;
						/* consider priorities */
						if (((s[j].flags|s[i].flags) & (ST_FLAT | ST_END)) == ST_FLAT) {
							c = i;
							goto bluematch;
						}
						if (((s[j].flags|s[i].flags) & ST_END) == 0)  {
							if (pri < 2) {
								c = i;
								pri = 2;
							}
						} else {
							if (pri == 0) {
								c = i;
								pri = 1;
							}
						}
					}
				}
			bluematch:
				/* clean up the stack */
				nstack = sbottom = 0;
				readystem = 0;
				/* add this stem */
				s[nnew++] = s[a];
				if (c < 0) { /* make one-dot-wide stem */
					if (nnew >= b) { /* have no free space */
						for (j = nold; j >= b; j--) {
							/* make free space */
							s[j] = s[j - 1];
						}
						b++;
						nold++;
					}
					s[nnew] = s[a];
					s[nnew].flags &= ~(ST_UP | ST_BLUE);
					nnew++;
					i = b - 1;
				} else {
					s[nnew++] = s[c];
					i = c; /* skip up to this point */
				}
			} else {
				if (nstack >= MAX_STACK) {
					nstack = 0;
				}
				stack[nstack++] = s[i];
			}
		} else if(s[i].flags & ST_BLUE) {
			/* again, we just HAVE to use this value */
			if (readystem) nnew += 2;
			readystem = 0;

			/* remember the list of Blue zone stems with the same value */
			for (a = i, i++; i < nold && s[a].value == s[i].value && (s[i].flags & ST_BLUE); i++) {
			}
			b = i; /* our range is a <= i < b */
			c = -1; /* index of our best guess up to now */
			pri = 0;
			/* try to find a match */
			for (i = nstack - 1; i >= 0; i--) {
				if ((stack[i].flags & ST_UP) == 0) {
					if ((stack[i].flags & (ST_ZONE | ST_TOPZONE)) == ST_ZONE) break;
					else continue;
				}
				for (j = a; j < b; j++) {
					if (!stemoverlap(&s[j], &stack[i])) continue;
					/* consider priorities */
					if (((s[j].flags|stack[i].flags) & (ST_FLAT | ST_END)) == ST_FLAT) {
						c = i;
						goto bluedownmatch;
					}
					if (((s[j].flags|stack[i].flags) & ST_END) == 0)  {
						if (pri < 2) {
							c = i;
							pri = 2;
						}
					} else {
						if (pri == 0) {
							c = i;
							pri = 1;
						}
					}
				}
			}
		bluedownmatch:
			/* if found no match make a one-dot-wide stem */
			if (c < 0) {
				c = 0;
				stack[0] = s[b - 1];
				stack[0].flags |= ST_UP;
				stack[0].flags &= ~ST_BLUE;
			}
			/* remove all the stems conflicting with this one */
			readystem = 0;
			for (j = nnew - 2; j >= 0; j -= 2) {
				if (s[j + 1].value < stack[c].value) {
					/* no conflict */
					break;
				}
				if (s[j].flags & ST_BLUE) {
					/* oops, we don't want to spoil other blue zones */
					stack[c].value = s[j + 1].value + 1;
					break;
				}
				if ((s[j].flags|s[j + 1].flags) & ST_END) {
					continue; /* pri==1, silently discard it */
				}
				/* we want to discard no nore than 2 stems of pri>=2 */
				if (++readystem > 2) {
					/* change our stem to not conflict */
					stack[c].value = s[j + 1].value + 1;
					break;
				} else {
					continue;
				}
			}
			nnew = j + 2;
			/* add this stem */
			if (nnew >= b - 1) { /* have no free space */
				for (j = nold; j >= b - 1; j--) {
					/* make free space */
					s[j] = s[j - 1];
				}
				b++;
				nold++;
			}
			s[nnew++] = stack[c];
			s[nnew++] = s[b - 1];
			/* clean up the stack */
			nstack = sbottom = 0;
			readystem = 0;
			/* set the next position to search */
			i = b - 1;
		} else if (nstack > 0) {
			/*
			 * check whether our stem overlaps with anything in
			 * stack
			 */
			for (j = nstack - 1; j >= sbottom; j--) {
				if (s[i].value <= stack[j].value) break;
				if (stack[j].flags & ST_ZONE) continue;

				if ((s[i].flags & ST_END) || (stack[j].flags & ST_END)) pri = 1;
				else if ((s[i].flags & ST_FLAT) || (stack[j].flags & ST_FLAT)) pri = 3;
				else pri = 2;

				if ((pri < readystem && s[nnew + 1].value >= stack[j].value) || !stemoverlap (&stack[j], &s[i]))
					continue;

				if (readystem > 1 && s[nnew + 1].value < stack[j].value) {
					nnew += 2;
					readystem = 0;
					nlps = 0;
				}
				/*
				 * width of the previous stem (if it's
				 * present)
				 */
				w1 = s[nnew + 1].value - s[nnew].value;

				/* width of this stem */
				w2 = s[i].value - stack[j].value;

				if (readystem == 0) {
					/* nothing yet, just add a new stem */
					s[nnew] = stack[j];
					s[nnew + 1] = s[i];
					readystem = pri;
					if (pri == 1) nlps = 1;
					else if (pri == 2) sbottom = j;
					else {
						sbottom = j + 1;
						while (sbottom < nstack && stack[sbottom].value <= stack[j].value) sbottom++;
					}
				} else if (pri == 1) {
					if (stack[j].value > s[nnew + 1].value) {
						/*
						 * doesn't overlap with the
						 * previous one
						 */
						nnew += 2;
						nlps++;
						s[nnew] = stack[j];
						s[nnew + 1] = s[i];
					} else if (w2 < w1) {
						/* is narrower */
						s[nnew] = stack[j];
						s[nnew + 1] = s[i];
					}
				} else if (pri == 2) {
					if (readystem == 2) {
						/* choose the narrower stem */
						if (w1 > w2) {
							s[nnew] = stack[j];
							s[nnew + 1] = s[i];
							sbottom = j;
						}
						/* else readystem==1 */
					} else if (stack[j].value > s[nnew + 1].value) {
						/*
						 * value doesn't overlap with
						 * the previous one
						 */
						nnew += 2;
						nlps = 0;
						s[nnew] = stack[j];
						s[nnew + 1] = s[i];
						sbottom = j;
						readystem = pri;
					} else if (nlps == 1 || stack[j].value > s[nnew - 1].value) {
						/*
						 * we can replace the top
						 * stem
						 */
						nlps = 0;
						s[nnew] = stack[j];
						s[nnew + 1] = s[i];
						readystem = pri;
						sbottom = j;
					}
				} else if (readystem == 3) {	/* that means also pri==3 */
					/* choose the narrower stem */
					if (w1 > w2) {
						s[nnew] = stack[j];
						s[nnew + 1] = s[i];
						sbottom = j + 1;
						while (sbottom < nstack && stack[sbottom].value <= stack[j].value) sbottom++;
					}
				} else if (pri == 3) {
					/*
					 * we can replace as many stems as
					 * neccessary
					 */
					nnew += 2;
					while (nnew > 0 && s[nnew - 1].value >= stack[j].value) {
						nnew -= 2;
					}
					nlps = 0;
					s[nnew] = stack[j];
					s[nnew + 1] = s[i];
					readystem = pri;
					sbottom = j + 1;
					while (sbottom < nstack && stack[sbottom].value <= stack[j].value) sbottom++;
				}
			}
		}
	}
	if (readystem) nnew += 2;

	/* change the 1-pixel-wide stems to 20-pixel-wide stems if possible 
	 * the constant 20 is recommended in the Type1 manual 
	 */
	if (useblues) {
		for (i = 0; i < nnew; i += 2) {
			if (s[i].value != s[i + 1].value) continue;
			if (((s[i].flags ^ s[i + 1].flags) & ST_BLUE) == 0) continue;
			if (s[i].flags & ST_BLUE) {
				if (nnew > i + 2 && s[i + 2].value < s[i].value + 22)
					s[i + 1].value = s[i + 2].value - 2; /* compensate for fuzziness */
				else
					s[i + 1].value += 20;
			} else {
				if (i > 0 && s[i - 1].value > s[i].value - 22)
					s[i].value = s[i - 1].value + 2; /* compensate for fuzziness */
				else
					s[i].value -= 20;
			}
		}
	}
	/* make sure that no stem it stretched between
	 * a top zone and a bottom zone
	 */
	if (useblues) {
		for (i = 0; i < nnew; i += 2) {
			a = 10000; /* lowest border of top zone crosing the stem */
			b = -10000; /* highest border of bottom zone crossing the stem */

			for (j = 2; j < nblues; j++) {
				c = bluevalues[j];
				if (c >= s[i].value && c <= s[i + 1].value && c < a) a = c;
			}
			if (nblues >= 2) {
				c = bluevalues[1];
				if (c >= s[i].value && c <= s[i + 1].value && c > b) b = c;
			}
			for(j = 1; j < notherb; j++) {
				c = otherblues[j];
				if (c >= s[i].value && c <= s[i + 1].value && c > b) b = c;
			}
			if (a != 10000 && b != -10000) { /* it is stretched */
				/* split the stem into 2 ghost stems */
				for (j = nnew + 1; j > i + 1; j--) {
					/* make free space */
					s[j]=s[j - 2];
				}
				nnew += 2;

				if (s[i].value + 22 >= a) s[i + 1].value = a - 2; /* leave space for fuzziness */
				else s[i + 1].value = s[i].value + 20;

				if (s[i + 3].value - 22 <= b) s[i + 2].value = b + 2; /* leave space for fuzziness */
				else s[i + 2].value = s[i + 3].value - 20;

				i += 2;
			}
		}
	}
	/* look for triple stems */
	for (i = 0; i < nnew; i += 2) {
		if (nnew - i >= 6) {
			a = s[i].value + s[i + 1].value;
			b = s[i + 2].value + s[i + 3].value;
			c = s[i + 4].value + s[i + 5].value;

			w1 = s[i + 1].value - s[i].value;
			w2 = s[i + 3].value - s[i + 2].value;
			w3 = s[i + 5].value - s[i + 4].value;

			fw = w3 - w1;	/* fuzz in width */
			fd = ((c - b) - (b - a));	/* fuzz in distance
							 * (doubled) */

			/* we are able to handle some fuzz */
			/*
			 * it doesn't hurt if the declared stem is a bit
			 * narrower than actual unless it's an edge in
			 * a blue zone
			 */
			if (abs (abs (fd) - abs (fw)) * 5 < w2 && abs (fw) * 20 < (w1 + w3)) {	/* width dirrerence <10% */
				if (useblues) { /* check that we don't disturb any blue stems */
					j = c;
					k = a;
					if (fw > 0) {
						if (fd > 0) {
							if (s[i + 5].flags & ST_BLUE) continue;
							j -= fw;
						} else {
							if (s[i + 4].flags & ST_BLUE) continue;
							j += fw;
						}
					} else if(fw < 0) {
						if (fd > 0) {
							if (s[i + 1].flags & ST_BLUE) continue;
							k -= fw;
						} else {
							if (s[i].flags & ST_BLUE) continue;
							k += fw;
						}
					}
					pri = ((j - b) - (b - k));

					if (pri > 0) {
						if (s[i+2].flags & ST_BLUE) continue;
					} else if (pri < 0) {
						if (s[i+3].flags & ST_BLUE) continue;
					}
				}

				/*
				 * first fix up the width of 1st and 3rd
				 * stems
				 */
				if (fw > 0) {
					if (fd > 0) {
						s[i + 5].value -= fw;
						c -= fw;
					} else {
						s[i + 4].value += fw;
						c += fw;
					}
				} else {
					if (fd > 0) {
						s[i + 1].value -= fw;
						a -= fw;
					} else {
						s[i].value += fw;
						a += fw;
					}
				}
				fd = ((c - b) - (b - a));

				if (fd > 0) {
					s[i + 2].value += abs (fd) / 2;
				} else {
					s[i + 3].value -= abs (fd) / 2;
				}

				s[i].flags |= ST_3;
				i += 4;
			}
		}
	}

	return (nnew & ~1);	/* number of lines must be always even */
}

static void
joinsubstems (struct stem *s, gshort *pairs, gint nold, gint useblues)
{
	gint i, j, x = 0;
	static guchar mx[MAX_STEMS][MAX_STEMS];

	/* we do the substituted groups of stems first
	 * and it looks like it's going to be REALLY SLOW 
	 * AND PAINFUL but let's bother about it later
	 */

	/* for the substituted stems we don't bother about [hv]stem3 -
	 * anyway the X11R6 rasterizer does not bother about hstem3
	 * at all and is able to handle only one global vstem3
	 * per glyph 
	 */

	/* clean the used part of matrix */
	for (i = 0; i < nold; i++) {
		for (j = 0; j < nold; j++) mx[i][j]=0;
	}

	/* build the matrix of stem pairs */
	for (i = 0; i < nold; i++) {
		if (s[i].flags & ST_ZONE) continue;
		if (s[i].flags & ST_BLUE) mx[i][i] = 1; /* allow to pair with itself if no better pair */
		if (s[i].flags & ST_UP) { /* the down-stems are already matched */
			setbasestem (s[i].from, s[i].to);
			for (j = i + 1; j < nold; j++) {
				if (s[i].value == s[j].value || s[j].flags & ST_ZONE) continue;
				x = subfrombase (s[j].from, s[j].to);

				if (s[j].flags & ST_UP) continue; /* match only up+down pairs */

				mx[i][j] = mx[j][i] = x;

				if (isbaseempty ()) break; /* nothing else to do */
			}
		}
	}

	/* now use the matrix to find the best pair for each stem */
	for (i = 0; i < nold; i++) {
		gint pri, lastpri, v, f;

		x = -1; /* best pair: none */
		lastpri = 0;

		v = s[i].value;
		f = s[i].flags;

		if (f & ST_ZONE) {
			pairs[i] = -1;
			continue;
		}

		if (f & ST_UP) {
			for (j = i + 1; j < nold; j++) {
				if (mx[i][j] == 0) continue;

				if ((f | s[j].flags) & ST_END) pri = 1;
				else if ((f | s[j].flags) & ST_FLAT) pri = 3;
				else pri = 2;

				if (lastpri == 0 ||
					(pri > lastpri &&
					 (lastpri == 1 || s[j].value - v < 20 || (s[x].value - v) * 2 >= s[j].value - v))) {
					lastpri = pri;
					x = j;
				}
			}
		} else {
			for (j = i - 1; j >= 0; j--) {
				if (mx[i][j] == 0) continue;

				if ((f | s[j].flags) & ST_END) pri = 1;
				else if ((f | s[j].flags) & ST_FLAT) pri = 3;
				else pri = 2;

				if (lastpri == 0 ||
					(pri > lastpri &&
					 (lastpri == 1 || v - s[j].value < 20 || (v - s[x].value) * 2 >= v - s[j].value))) {
					lastpri = pri;
					x = j;
				}
			}
		}
		if (x == -1 && mx[i][i]) pairs[i] = i; /* a special case */
		else pairs[i] = x;
	}
}

static void
uniformstems (struct stem *s, gshort *pairs, gint ns)
{
	gint i, from, to, val, dir;
	gint pri, prevpri[2], wd, prevwd[2], prevbest[2];

	for (from = 0; from < ns; from = to) {
		prevpri[0] = prevpri[1] = 0;
		prevwd[0] = prevwd[1] = 0;
		prevbest[0] = prevbest[1] = -1;
		val = s[from].value;

		for (to = from; to < ns && s[to].value == val; to++) {
			dir = ((s[to].flags & ST_UP) != 0);

			i = pairs[to]; /* the other side of this stem */
			if (i < 0 || i == to) continue; /* oops, no other side */
			wd = abs (s[i].value - val);
			if (wd == 0) continue;
			pri = 1;
			if ((s[to].flags | s[i].flags) & ST_END) pri = 0;
			if (prevbest[dir] == -1 || pri > prevpri[dir] || wd<prevwd[dir]) {
				prevbest[dir] = i;
				prevpri[dir] = pri;
				prevwd[dir] = wd;
			}
		}

		for (i = from; i < to; i++) {
			dir = ((s[i].flags & ST_UP) != 0);
			if (prevbest[dir] >= 0) {
				pairs[i] = prevbest[dir];
			}
		}
	}
}

static void
groupsubstems (struct glyph *g, struct stem *hs, gshort *hpairs, gint nhs, struct stem *vs, gshort *vpairs, gint nvs)
{
	struct gentry *ge;
	gint i;
	struct stembounds s[MAX_STEMS * 2];
	gshort egp[NSTEMGRP];
	gint nextvsi, nexthsi;

	for (i = 0; i < NSTEMGRP; i++) egp[i] = 0;

	nextvsi = nexthsi = -2; /* processed no horiz/vert line */

	gssentry_lastgrp = 0; /* reset the last group for new glyph */

	for (ge = g->entries; ge != 0; ge = ge->next) {
		if (ge->type != GE_LINE && ge->type != GE_CURVE) {
			nextvsi = nexthsi = -2; /* next path is independent */
			continue;
		}

		if (gssentry (ge, hs, hpairs, nhs, vs, vpairs, nvs, s, egp, &nextvsi, &nexthsi)) {
			/* it's better to have no substituted hints at all than have only part */
			for (ge = g->entries; ge != 0; ge = ge->next) ge->stemid = -1;
			g->nsg = 0; /* just to be safe, already is 0 by initialization */
			return;
		}

		/*
		 * handle the last vert/horiz line of the path specially,
		 * correct the hint for the first entry of the path
		 */
		if (ge->frwd != ge->next && (nextvsi != -2 || nexthsi != -2)) {
			if (gssentry (ge->frwd, hs, hpairs, nhs, vs, vpairs, nvs, s, egp, &nextvsi, &nexthsi)) {
				/* it's better to have no substituted hints at all than have only part */
				for (ge = g->entries; ge != 0; ge = ge->next) ge->stemid = -1;
				g->nsg = 0; /* just to be safe, already is 0 by initialization */
				return;
			}
		}
	}

	/* find the index of the first empty group - same as the number of groups */
	if (egp[0] > 0) {
		for (i = 1; i < NSTEMGRP && egp[i] != egp[i-1]; i++) {
		}
		g->nsg = i;
	} else
		g->nsg = 0;

	if (g->nsg == 1) { /* it would be the same as the main stems */
		/* so erase it */
		for (ge = g->entries; ge != 0; ge = ge->next) ge->stemid = -1;
		g->nsg = 0;
	}

	if (g->nsg > 0) {
		g->nsbs = g_new0 (gshort, g->nsg);
		memmove (g->nsbs, egp, g->nsg * sizeof (gshort));
		g->sbstems = g_new0 (struct stembounds, egp[g->nsg - 1]);
		memmove (g->sbstems, s, egp[g->nsg - 1] * sizeof (s[0]));
	}
}

static gint
besthyst (gint *hyst, gint base, gint *best, gint nbest, gint width, gint *bestindp)
{
	guchar hused[MAXHYST / 8 + 1];
	gint i, max, j, w, last = 0;
	gint nf = 0;

	width--;

	memset (hused, 0 , sizeof (hused));

	max = 1;
	for (i = 0; i < nbest && max != 0; i++) {
		best[i] = 0;
		max = 0;
		for (j = 1; j < MAXHYST - 1; j++) {
			w = hyst[j];

			if (w > max && (hused[j >> 3] & (1 << (j & 0x07))) == 0) {
				best[i] = j;
				max = w;
			}
		}
		if (max != 0) {
			if (max < last / 2) {
				/* do not pick the too low values */
				break;
			}
			for (j = best[i] - width; j <= best[i] + width; j++) {
				if (j >= 0 && j < MAXHYST) hused[j >> 3] |= (1 << (j & 0x07));
			}
			last = max;
			best[i] -= base;
			nf = i + 1;
		}
	}

	if (bestindp) *bestindp = best[0];

	/* sort the indexes in ascending order */
	for (i = 0; i < nf; i++) {
		for (j = i + 1; j < nf; j++) {
			if (best[j] < best[i]) {
				w = best[i];
				best[i] = best[j];
				best[j] = w;
			}
		}
	}

	return nf;
}

static int
stemoverlap (struct stem *s1, struct stem *s2)
{
	gint result;

	if ((s1->from <= s2->from && s1->to >= s2->from) || (s2->from <= s1->from && s2->to >= s1->from))
		result = 1;
	else
		result = 0;

	return result;
}

static gint
subfrombase (gint from, gint to)
{
	gint a, b;
	gint i, j;

	if (isbaseempty ()) return 0;

	/* handle the simple case simply */
	if (from > xbstem[xblast] || to < xbstem[0]) return 0;

	/* the binary search may be more efficient */
	/* but for now the linear search is OK */
	for (b = 1; from > xbstem[b]; b += 2) {
		/* result: from <= xbstem[b] */
	}
	for (a = xblast - 1; to < xbstem[a]; a -= 2) {
		/* result: to >= xbstem[a] */
	}

	/* now the interesting examples are:
	 * (it was hard for me to understand, so I looked at the examples)
	 * 1
	 *     a|-----|          |-----|b   |-----|     |-----|
	 *              f|-----|t
	 * 2
	 *     a|-----|b         |-----|    |-----|     |-----|
	 *      f|--|t
	 * 3
	 *     a|-----|b         |-----|    |-----|     |-----|
	 *           f|-----|t
	 * 4
	 *      |-----|b        a|-----|    |-----|     |-----|
	 *          f|------------|t
	 * 5
	 *      |-----|          |-----|b   |-----|    a|-----|
	 *                   f|-----------------------------|t
	 * 6
	 *      |-----|b         |-----|    |-----|    a|-----|
	 *   f|--------------------------------------------------|t
	 * 7
	 *      |-----|b         |-----|   a|-----|     |-----|
	 *          f|--------------------------|t
	 */

	if (a < b - 1) return 0; /* hits a gap  - example 1 */

	/* now the subtraction itself */

	if (a == b - 1 && from > xbstem[a] && to < xbstem[b]) {
		/* overlaps with only one subrange and splits it - example 2 */
		j = xblast;
		i = (xblast += 2);
		while (j >= b) xbstem[i--] = xbstem[j--];
		xbstem[b] = from - 1;
		xbstem[b + 1] = to + 1;

		return 1;
	/* becomes
	 * 2a
	 *     a|b   ||          |-----|    |-----|     |-----|
	 *      f|--|t
	 */
	}

	if (xbstem[b - 1] < from) {
		/* cuts the back of this subrange - examples 3, 4, 7 */
		xbstem[b] = from - 1;
		b += 2;
		/* becomes
		 * 3a
		 *     a|----|           |-----|b   |-----|     |-----|
		 *           f|-----|t
		 * 4a
		 *      |---|           a|-----|b   |-----|     |-----|
		 *          f|------------|t
		 * 7a
		 *      |---|            |-----|b  a|-----|     |-----|
		 *          f|--------------------------|t
		 */
	}

	if (xbstem[a + 1] > to) {
		/* cuts the front of this subrange - examples 4a, 5, 7a */
		xbstem[a] = to + 1;
		a -= 2;
		/* becomes
		 * 4b
		 *     a|---|              |---|b   |-----|     |-----|
		 *          f|------------|t
		 * 5b
		 *      |-----|          |-----|b  a|-----|          ||
		 *                   f|-----------------------------|t
		 * 7b
		 *      |---|           a|-----|b        ||     |-----|
		 *          f|--------------------------|t
		 */
	}

	if (a < b-1) return 1;	/* now after modification it hits a gap - examples 3a, 4b */
							/* because we have removed something */

	/* now remove the subranges completely covered by the new stem */
	/* examples 5b, 6, 7b */
	i = b - 1;
	j = a + 2;
	/* positioned as:
	 * 5b                    i                           j
	 *      |-----|          |-----|b  a|-----|          ||
	 *                   f|-----------------------------|t
	 * 6    i                                             xblast  j
	 *      |-----|b         |-----|    |-----|    a|-----|
	 *   f|--------------------------------------------------|t
	 * 7b                    i               j
	 *      |---|           a|-----|b        ||     |-----|
	 *          f|--------------------------|t
	 */
	while (j <= xblast) xbstem[i++] = xbstem[j++];
	xblast = i - 1;

	return 1;
}

static int
gssentry (struct gentry *ge, struct stem *hs, gshort *hpairs, gint nhs, struct stem *vs, gshort *vpairs, gint nvs, struct stembounds *s, gshort *egp, gint *nextvsi, gint *nexthsi)
{
	enum {
		SI_VP,	/* vertical primary */
		SI_HP,	/* horizontal primary */
		SI_SIZE /* size of the array */
	};
	gint si[SI_SIZE]; /* indexes of relevant stems */
	/* the bounds of the existing relevant stems */
	struct stembounds r[sizeof (si) / sizeof (si[0]) * 2];
	gchar rexpand;
	gint nr;
	/* yet more temporary storage */
	gshort lb, hb, isvert;
	gint conflict, grp = 0;
	gint i, j, x, y;

	/* for each line or curve we try to find a horizontal and
	 * a vertical stem corresponding to its first point
	 * (corresponding to the last point of the previous
	 * glyph entry), because the directions of the lines
	 * will be eventually reversed and it will then become the last
	 * point. And the T1 rasterizer applies the hints to 
	 * the last point.
	 *
	 */

	/* start with the common part, the first point */
	x = ge->prev->ix3;
	y = ge->prev->iy3;

	if (*nextvsi == -2) si[SI_VP] = findstemat (x, y, ge, vs, vpairs, nvs, -1);
	else {
		si[SI_VP] = *nextvsi;
		*nextvsi = -2;
	}
	if (*nexthsi == -2) si[SI_HP] = findstemat (y, x, ge, hs, hpairs, nhs, -1);
	else {
		si[SI_HP] = *nexthsi;
		*nexthsi = -2;
	}

	/*
	 * For the horizontal lines we make sure that both
	 * ends of the line have the same horizontal stem,
	 * and the same thing for vertical lines and stems.
	 * In both cases we enforce the stem for the next entry.
	 * Otherwise unpleasant effects may arise.
	 */

	if (ge->type == GE_LINE) {
		if (ge->ix3 == x) { /* vertical line */
			*nextvsi = si[SI_VP] = findstemat (x, ge->iy3, ge->frwd, vs, vpairs, nvs, si[SI_VP]);
		} else if (ge->iy3 == y) { /* horizontal line */
			*nexthsi = si[SI_HP] = findstemat (y, ge->ix3, ge->frwd, hs, hpairs, nhs, si[SI_HP]);
		}
	}

	if (si[SI_VP] + si[SI_HP] == -2) return 0; /* no stems, leave it alone */

	/* build the array of relevant bounds */
	nr = 0;
	for (i = 0; i < sizeof (si) / sizeof (si[0]); i++) {
		struct stem *sp;
		gshort *pairs;
		gint step;
		gint f;
		gint nzones, firstzone = 0, binzone, einzone;
		gint btype = 0, etype = 0;

		if (si[i] < 0) continue;

		if (i < SI_HP) {
			r[nr].isvert = 1;
			sp = vs;
			pairs = vpairs;
		} else {
			r[nr].isvert = 0;
			sp = hs;
			pairs = hpairs;
		}

		r[nr].low = sp[si[i]].value;
		r[nr].high = sp[pairs[si[i]]].value;

		if (r[nr].low > r[nr].high) {
			j = r[nr].low;
			r[nr].low = r[nr].high;
			r[nr].high = j;
			step = -1;
		} else {
			step = 1;
		}

		/* handle the interaction with Blue Zones */

		if (i >= SI_HP) { /* only for horizontal stems */
			if (si[i] == pairs[si[i]]) {
				/* special case, the outermost stem in the
				 * Blue Zone without a pair, simulate it to 20-pixel
				 */
				if (sp[si[i]].flags & ST_UP) {
					r[nr].high += 20;
					for (j = si[i] + 1; j < nhs; j++) {
						if ((sp[j].flags & (ST_ZONE|ST_TOPZONE)) == (ST_ZONE | ST_TOPZONE)) {
							if (r[nr].high > sp[j].value - 2) r[nr].high = sp[j].value - 2;
							break;
						}
					}
				} else {
					r[nr].low -= 20;
					for (j = si[i] - 1; j >= 0; j--) {
						if ((sp[j].flags & (ST_ZONE | ST_TOPZONE)) == (ST_ZONE)) {
							if (r[nr].low < sp[j].value + 2) r[nr].low = sp[j].value + 2;
							break;
						}
					}
				}
			}

			/* check that the stem borders don't end up in
			 * different Blue Zones */
			f = sp[si[i]].flags;
			nzones = 0;
			einzone = binzone = 0;
			for (j = si[i]; j != pairs[si[i]]; j += step) {
				if ((sp[j].flags & ST_ZONE) == 0) continue;
				/* if see a zone border going in the same direction */
				if (((f ^ sp[j].flags) & ST_UP) == 0) {
					if (++nzones == 1) {
						firstzone = sp[j].value; /* remember the first one */
						etype = sp[j].flags & ST_TOPZONE;
					}
					einzone = 1;
				} else { /* the opposite direction */
					if (nzones == 0) { /* beginning is in a blue zone */
						binzone = 1;
						btype = sp[j].flags & ST_TOPZONE;
					}
					einzone = 0;
				}
			}

			/* beginning and end are in Blue Zones of different types */
			if (binzone && einzone && (btype ^ etype) != 0) {
				if (sp[si[i]].flags & ST_UP) {
					if (firstzone > r[nr].low + 22) r[nr].high = r[nr].low + 20;
					else r[nr].high = firstzone - 2;
				} else {
					if (firstzone < r[nr].high - 22) r[nr].low = r[nr].high - 20;
					else r[nr].low = firstzone + 2;
				}
			}
		}
		nr++;
	}

	/* now try to find a group */
	conflict = 0; /* no conflicts found yet */
	for (j = 0; j < nr; j++) r[j].already = 0;

	/* check if it fits into the last group */
	grp = gssentry_lastgrp;
	i = (grp == 0) ? 0 : egp[grp - 1];
	for (; i < egp[grp]; i++) {
		lb = s[i].low;
		hb = s[i].high;
		isvert = s[i].isvert;
		for (j = 0; j < nr; j++) {
			if (r[j].isvert==isvert && r[j].low <= hb && r[j].high >= lb) {
				if (r[j].low == lb && r[j].high == hb) r[j].already = 1;
				else conflict = 1;
			}
		}

		if (conflict) break;
	}

	if (conflict) { /* nope, check all the groups */
		for (j = 0; j < nr; j++) r[j].already = 0;

		for (i = 0, grp = 0; i < egp[NSTEMGRP - 1]; i++) {
			if (i == egp[grp]) { /* checked all stems in a group */
				if (conflict) {
					grp++;
					conflict = 0; /* check the next group */
					for (j = 0; j < nr; j++) r[j].already = 0;
				} else
					break; /* insert into this group */
			}

			lb = s[i].low;
			hb = s[i].high;
			isvert = s[i].isvert;
			for (j = 0; j < nr; j++) {
				if (r[j].isvert == isvert && r[j].low <= hb && r[j].high >= lb) {
					if (r[j].low == lb && r[j].high == hb) r[j].already = 1;
					else conflict = 1;
				}
			}

			if (conflict) i = egp[grp] - 1; /* fast forward to the next group */
		}
	}

	/* do we have any empty group ? */
	if (conflict && grp < NSTEMGRP - 1) {
		grp++;
		conflict = 0;
		for (j = 0; j < nr; j++) r[j].already = 0;
	}

	if (conflict) { /* oops, can't find any group to fit */
		return 1;
	}

	/* OK, add stems to this group */

	rexpand = nr;
	for (j = 0; j < nr; j++) rexpand -= r[j].already;

	if (rexpand > 0) {
		for (i = egp[NSTEMGRP - 1] - 1; i >= egp[grp]; i--) s[i + rexpand] = s[i];
		for (i = 0; i < nr; i++) {
			if (!r[i].already) s[egp[grp]++] = r[i];
		}
		for (i = grp + 1; i < NSTEMGRP; i++) egp[i] += rexpand;
	}

	ge->stemid = gssentry_lastgrp = grp;

	return 0;
}

static int
findstemat (gint value, gint origin, struct gentry *ge, struct stem *sp, gshort *pairs, gint ns, gint prevbest)
{
	gint i, min, max;
	gint v, si;
	gint pri, prevpri = 0; /* priority, 0 = has ST_END, 1 = no ST_END */
	gint wd, prevwd = 0; /* stem width */

	si = -1; /* nothing yet */

	/* stems are ordered by value, binary search */
	min = 0;
	max = ns; /* min <= i < max */
	while (min < max) {
		i = (min + max) / 2;
		v = sp[i].value;
		if (v < value) min = i + 1;
		else if (v > value) max = i;
		else {
			si = i; /* temporary value */
			break;
		}
	}

	if (si < 0) return prevbest; /* found nothing this time */

	/* find the priority of the prevbest */
	/* we expect that prevbest has a pair */
	if (prevbest >= 0) {
		i = pairs[prevbest];
		prevpri = 1;
		if ((sp[prevbest].flags | sp[i].flags) & ST_END) prevpri = 0; 
		prevwd = abs (sp[i].value - value);
	}

	/* stems are not ordered by origin, so now do the linear search */

	while (si > 0 && sp[si - 1].value == value) si--; /* find the first one */

	for (; si < ns && sp[si].value == value; si++) {
		if (sp[si].origin != origin) continue;
		if (sp[si].ge != ge) continue;
		i = pairs[si]; /* the other side of this stem */
		if (i < 0) continue; /* oops, no other side */
		pri = 1;
		if ((sp[si].flags | sp[i].flags) & ST_END) pri = 0;
		wd = abs (sp[i].value - value);
		if (prevbest == -1 || pri > prevpri || (pri == prevpri && prevwd == 0) || (wd != 0 && wd < prevwd)) {
			prevbest = si;
			prevpri = pri;
			prevwd = wd;
			continue;
		}
	}

	return prevbest;
}

void
docorrectwidth(struct glyph_face *gf)
{
	struct gentry *ge;
	struct glyph *g;
	gint xmin, xmax;
	gint maxwidth, minsp;
	gint correctwidth = 1;

	if (gf->is_fixed_pitch) correctwidth = 0;

	/* enforce this minimal spacing,
	 * we limit the amount of the enforced spacing to avoid
	 * spacing the bold wonts too widely
	 */
	minsp = (stdhw > 60 || stdhw < 10) ? 60 : stdhw;

	for (g = gf->glyph; g != NULL; g = g->next) {
		g->oldwidth = g->scaledwidth; /* save the old width, will need for AFM */

		if (correctwidth) {
			xmin = 5000;
			xmax = -5000;
			for (ge = g->entries; ge != 0; ge = ge->next) {
				if (ge->type != GE_LINE && ge->type != GE_CURVE) continue;

				if (ge->ix3 <= xmin) xmin = ge->ix3;
				if (ge->ix3 >= xmax) xmax = ge->ix3;
			}

			maxwidth = xmax + minsp;
			if (g->scaledwidth < maxwidth) {
				g->scaledwidth = maxwidth;
			}
		}
	}
}

gint
print_glyph_subs (gchar **retval, struct glyph *glyph, gint startid)
{
	struct glyph *g = glyph;
	gchar *buf = NULL;
	gint i, grp;

	if (g->nsg < 1) return 0;

	g->firstsubr = startid;
	for (grp = 0; grp < g->nsg; grp++) {
		buf = ttf_printf (buf, "dup %d {\n", startid++);
		for (i = (grp == 0) ? 0 : g->nsbs[grp - 1]; i < g->nsbs[grp]; i++) {
			buf = ttf_printf (buf, "\t%d %d %cstem\n", g->sbstems[i].low, g->sbstems[i].high - g->sbstems[i].low, g->sbstems[i].isvert ? 'v' : 'h');
		}
		buf = ttf_printf (buf,
						  "\treturn\n" \
						  "} NP\n");
	}
	(*retval) = ttf_printf ((*retval), "%s", buf);
	g_free (buf);

	return g->nsg;
}

gchar *
print_glyph (struct glyph *glyph)
{
	struct glyph *g = glyph;
	struct gentry *ge;
	gint x = 0, y = 0;
	gint i;
	gint grp, lastgrp = -1;
	gchar *retval, *buf;

	retval = g_strdup_printf ("/%s {\n", g->name);

	/* consider widths >MAXLEGALWIDTH as bugs */
	if( g->scaledwidth <= MAXLEGALWIDTH ) {
		retval = ttf_printf (retval, "0 %d hsbw\n", g->scaledwidth);
	} else {
		retval = ttf_printf (retval, "0 1000 hsbw\n");
	}

	if (g->hstems) {
		for (i = 0; i < g->nhs; i += 2) {
			if (g->hstems[i].flags & ST_3) {
				retval = ttf_printf (retval,
						     "%d %d %d %d %d %d hstem3\n"
						     , g->hstems[i].value
						     , g->hstems[i + 1].value - g->hstems[i].value
						     , g->hstems[i + 2].value
						     , g->hstems[i + 3].value - g->hstems[i + 2].value
						     , g->hstems[i + 4].value
						     , g->hstems[i + 5].value - g->hstems[i + 4].value);
				i += 4;
			} else {
				retval = ttf_printf (retval,
						     "%d %d hstem\n"
						     , g->hstems[i].value, g->hstems[i + 1].value
						     , g->hstems[i + 1].value - g->hstems[i].value);
			}
		}
	}

	if (g->vstems) {
		for (i = 0; i < g->nvs; i += 2) {
			if (g->vstems[i].flags & ST_3) {
				retval = ttf_printf (retval,
						     "%d %d %d %d %d %d vstem3\n"
						     , g->vstems[i].value
						     , g->vstems[i + 1].value - g->vstems[i].value
						     , g->vstems[i + 2].value
						     , g->vstems[i + 3].value - g->vstems[i + 2].value
						     , g->vstems[i + 4].value
						     , g->vstems[i + 5].value - g->vstems[i + 4].value);
				i += 4;
			} else {
				retval = ttf_printf (retval,
						     "%d %d vstem\n"
						     , g->vstems[i].value
						     , g->vstems[i + 1].value - g->vstems[i].value);
			}
		}
	}

	for (ge = g->entries; ge != 0; ge = ge->next) {
		if (g->nsg > 0) {
			grp = ge->stemid;
			if (grp >= 0 && grp != lastgrp)  {
				retval = ttf_printf (retval, "%d 4 callsubr\n", grp + g->firstsubr);
				lastgrp = grp;
			}
		}

		switch (ge->type) {
			case GE_MOVE:
				buf = rmoveto (ge->ix3 - x, ge->iy3 - y);
				retval = ttf_printf (retval, "%s", buf);
				g_free (buf);
				x = ge->ix3;
				y = ge->iy3;
				break;
			case GE_LINE:
				buf = rlineto (ge->ix3 - x, ge->iy3 - y);
				retval = ttf_printf (retval, "%s", buf);
				g_free (buf);
				x = ge->ix3;
				y = ge->iy3;
				break;
			case GE_CURVE:
				buf = rrcurveto(ge->ix1 - x, ge->iy1 - y, ge->ix2 - ge->ix1, ge->iy2 - ge->iy1, ge->ix3 - ge->ix2, ge->iy3 - ge->iy2);
				retval = ttf_printf (retval, "%s", buf);
				g_free (buf);
				x = ge->ix3;
				y = ge->iy3;
				break;
			case GE_PATH:
				retval = ttf_printf (retval, "closepath\n");
				break;
			default:
				g_warning ("Unknown entry type '%d' -- %s\n", ge->type, g->name);
				break;
		}
	}

	retval = ttf_printf (retval, "endchar } ND\n");

	return retval;
}

gchar *
rmoveto (gint dx, gint dy)
{
	gchar *retval;

	if (dx == 0) retval = g_strdup_printf ("%d vmoveto\n", dy);
	else if (dy == 0) retval = g_strdup_printf ("%d hmoveto\n", dx);
	else retval = g_strdup_printf ("%d %d rmoveto\n", dx, dy);

	return retval;
}

gchar *
rlineto (gint dx, gint dy)
{
	gchar *retval;

	if (dx == 0 && dy == 0) return NULL;
	else if (dx == 0) retval = g_strdup_printf ("%d vlineto\n", dy);
	else if (dy == 0) retval = g_strdup_printf ("%d hlineto\n", dx);
	else retval = g_strdup_printf ("%d %d rlineto\n", dx, dy);

	return retval;
}

gchar *
rrcurveto (gint dx1, gint dy1, gint dx2, gint dy2, gint dx3, gint dy3)
{
	gchar *retval;

	if (dx1 == 0 && dx2 == 0 && dx3 == 0) retval = rlineto (0, dy1 + dy2 + dy3);
	else if (dy1 == 0 && dy2 == 0 && dy3 == 0) retval = rlineto (dx1 + dx2 + dx3, 0);
	else if (dy1 == 0 && dx3 == 0) retval = g_strdup_printf ("%d %d %d %d hvcurveto\n", dx1, dx2, dy2, dy3);
	else if (dx1 == 0 && dy3 == 0) retval = g_strdup_printf ("%d %d %d %d vhcurveto\n", dy1, dx2, dy2, dx3);
	else retval = g_strdup_printf ("%d %d %d %d %d %d rrcurveto\n", dx1, dy1, dx2, dy2, dx3, dy3);

	return retval;
}

/* ttf_type1_dump */
static void
getline (gchar **src)
{
	gint c;
	gchar *p = line;
	gint comment = 0;

	start_charstring = 0;
	while (p < line + LINESIZE) {
		c = **src;
		(*src)++;
		if (c == 0) break;
		if (c == '%') comment = 1;
		if (active && !comment && c == '{') {
			start_charstring = 1;
			break;
		}
		*p++ = (gchar)c;
		if (c == '\n') break;
	}
	*p = '\0';
}

static gchar *
eexec_start (gchar *src)
{
	gchar *retval;

	retval = eexec_string (src);

	hexcol = 0;
	in_eexec = 1;
	er = 55665;
	retval = ttf_printf (retval, "%s", eexec_byte (0));
	retval = ttf_printf (retval, "%s", eexec_byte (0));
	retval = ttf_printf (retval, "%s", eexec_byte (0));
	retval = ttf_printf (retval, "%s", eexec_byte (0));

	return retval;
}

static gchar *
eexec_string (gchar *string)
{
	gchar *retval = NULL;
	const gchar *buf;
	gint i = 0;
	size_t len;

	retval = g_new0 (gchar, strlen (string) * 3);
	while (*string) {
		buf = eexec_byte ((guchar)*string++);
		len = strlen (buf);
		strncpy (retval + i, buf, len);
		i += len;
	}
	retval[i] = 0;

	return retval;
}

static const gchar *
eexec_byte (guchar c)
{
	const gchar *retval;

	if (in_eexec) retval = output_byte (eencrypt (c));
	else retval = output_byte (c);

	return retval;
}

static guchar
eencrypt (guchar plain)
{
	guchar cipher;

	cipher = (guchar) (plain ^ (er >> 8));
	er = (guint16) ((cipher + er) * c1 + c2);

	return cipher;
}

static const gchar *
output_byte (guchar c)
{
	static gchar *hexchar = "0123456789ABCDEF";
	static gchar retval[8];
	gint i = 0;

	if (in_eexec) {
		if (hexcol >= 64) {
			retval[i++] = '\n';
			hexcol = 0;
		}
		retval[i++] = hexchar[(c >> 4) & 0xf];
		retval[i++] = hexchar[c & 0xf];
		hexcol += 2;
	} else {
		retval[i++] = c;
	}
	retval[i] = 0;

	return retval;
}

static gchar *
eexec_end (void)
{
	gint i, j, cnt = 0;
	gchar retval[8 * 66 + 2];

	retval[cnt++] = '\n';
	in_eexec = 0;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 64; j++) retval[cnt++] = '0';
		retval[cnt++] = '\n';
	}
	retval[cnt] = 0;

	return g_strdup (retval);
}

static gchar *
parse_charstring (gchar **src)
{
	struct command *cp;
	gint i;

	charstring_start ();
	while (**src != 0) {
		i = 0;
		while (**src != 0) {
			if (**src == ' ' || **src == '\t') {
				if (i != 0) break;
				(*src)++;
				continue;
			}
			line[i] = **src;
			if (**src == '\r' || **src == '\n') {
				if (i != 0) break;
				(*src)++;
				continue;
			}
			i++;
			(*src)++;
		}
		if (**src == 0) break;
		line[i] = 0;
		if (line[0] == '%') {
			while (**src != '\n' && **src != 0) (*src)++;
			continue;
		}
		if (line[0] == '}') break;
		if (is_integer (line)) {
			charstring_int (atoi (line));
		} else {
			cp = (struct command *)bsearch ((void *)line, (void *)command_table, sizeof (command_table) / sizeof (struct command), sizeof (struct command), command_compare);
			if (cp) {
				charstring_byte (cp->one);
				if (cp->two >= 0) charstring_byte (cp->two);
			} else {
				g_warning ("cannot use '%s' in charstring\n", line);
				return NULL;
			}
		}
	}
	return charstring_end ();
}

static void
charstring_start (void)
{
	gint i;

	charstring_bp = charstring_buf;
	cr = 4330;
	for (i = 0; i < lenIV; i++) *charstring_bp++ = cencrypt ((guchar)0);
}

static guchar
cencrypt (guchar plain)
{
	guchar cipher;

	cipher = (guchar) (plain ^ (cr >> 8));
	cr = (guint16) ((cipher + cr) * c1 + c2);

	return cipher;
}

static gint
is_integer (gchar *string)
{
	if (isdigit (string[0]) || string[0] == '-' || string[0] == '+') {
		while (*++string && isdigit (*string));
		if (!*string) return 1;
	}

	return 0;
}

static void
charstring_int (gint num)
{
	gint x;

	if (num >= -107 && num <= 107) {
		charstring_byte (num + 139);
	} else if (num >= 108 && num <= 1131) {
		x = num - 108;
		charstring_byte (x / 256 + 247);
		charstring_byte (x % 256);
	} else if (num >= -1131 && num <= -108) {
		x = abs (num) - 108;
		charstring_byte (x / 256 + 251);
		charstring_byte (x % 256);
	} else if (num >= (-2147483647 - 1) && num <= 2147483647) {
		charstring_byte (255);
		charstring_byte (num >> 24);
		charstring_byte (num >> 16);
		charstring_byte (num >> 8);
		charstring_byte (num);
	} else {
		g_warning ("cannot format the integer %d, too large\n", num);
	}
}

static void
charstring_byte (gint v)
{
	guchar c = (guchar)(v & 0xff);

	if (charstring_bp - charstring_buf > sizeof (charstring_buf)) {
		g_warning ("charstring_buf full (%d bytes)\n", sizeof (charstring_buf));
		return;
	}
	*charstring_bp++ = cencrypt (c);
}

static gchar *
charstring_end (void)
{
	guchar *bp;
	gchar *retval, buf[256], *buf2;

	sprintf (buf, "%d ", charstring_bp - charstring_buf);
	retval = eexec_string (buf);
	sprintf (buf, "%s ", cs_start);
	buf2 = eexec_string (buf);
	for (bp = charstring_buf; bp < charstring_bp; bp++) {
		buf2 = ttf_printf (buf2, "%s", eexec_byte (*bp));
	}

	retval = ttf_printf (retval, "%s", buf2);
	g_free (buf2);

	return retval;
}

static gint
command_compare (const void *key, const void *item)
{
	return strcmp ((gchar *)key, ((struct command *)item)->name);
}

#if 0
static gint
unilist_compare (const void *key, const void *item)
{
	if (*((gint *)key) == *((gint *)item)) return 0;
	if (*((gint *)key) < *((gint *)item)) return -1;
	return 1;
}
#endif

#if 0
gboolean
ttf_get_font_information (const gchar *filename, gchar **copyright, gchar **family_name, gchar **style_name, gchar **full_name, gchar **version, gchar **ps_name)
{
	struct stat sb;
	FILE *fp;
	struct ttf_directory *dir;
	struct ttf_dir_entry *dir_entry;
	struct ttf_name *name = NULL;
	struct ttf_name_rec *name_record;
	gchar *ttf;
	gchar *string_area, nbuf[4096], *p, *np;
	static gchar *name_fields[8];
	gint i, j;
	gint lang, len;
	gboolean found = FALSE;

	if (stat (filename, &sb) == -1) return FALSE;
	if ((ttf = g_new0 (gchar, sb.st_size)) == NULL) return FALSE;
	if ((fp = fopen (filename, "rb")) == NULL) {
		g_free (ttf);

		return FALSE;
	}
	if (fread (ttf, sizeof (gchar), sb.st_size, fp) != sb.st_size) {
		fclose (fp);
		g_free (ttf);

		return FALSE;
	}
	fclose (fp);

	dir = (struct ttf_directory *)ttf;

	if (ntohl (dir->sfntVersion) != 0x00010000) {
		g_free (ttf);

		return FALSE;
	}

	dir_entry = &(dir->list);
	for (i = 0; i < ntohs (dir->numTables); i++) {
		if (memcmp (dir_entry->tag, "name", 4) == 0) {
			name = (struct ttf_name *)(ttf + ntohl (dir_entry->offset));
		}
		dir_entry++;
	}

	string_area = (gchar *)name + ntohs (name->offset);
	name_record = &(name->nameRecords);
	np = nbuf;

	for (i = 0; i < ntohs (name->numberOfNameRecords); i++) {
		if (ntohs (name_record->platformID) == 3) {
			found = TRUE;
			lang = ntohs (name_record->languageID) & 0xff;
			len = ntohs (name_record->stringLength);
			if (lang == 0 || lang == 9) {
				j = ntohs (name_record->nameID);
				if (j < 8) {
					name_fields[j] = np;

					p = string_area + ntohs (name_record->stringOffset);
					if (((np + len + 1) - nbuf) > 4096) {
						g_free (ttf);

						return FALSE;
					}
					for (j = 0; j < len; j++) {
						if (p[j] != 0) {
							if (p[j] == '(') *np = '[';
							else if (p[j] == ')') *np = ']';
							else *np = p[j];
							np++;
						}
					}
					*np = 0;
					np++;
				}
			}
		}
		name_record++;
	}

	string_area = (gchar *)name + ntohs (name->offset);
	name_record = &(name->nameRecords);

	if (!found) {
		for (i = 0; i < ntohs (name->numberOfNameRecords); i++) {
			if (ntohs (name_record->platformID) == 1) {
				found = TRUE;
				lang = ntohs (name_record->languageID) & 0xff;
				len = ntohs (name_record->stringLength);
				if (lang == 0 || lang == 9) {
					j = ntohs (name_record->nameID);
					if (j < 8) {
						name_fields[j] = np;

						p = string_area + ntohs (name_record->stringOffset);
						if (((np + len + 1) - nbuf) > 4096) {
							g_free (ttf);

							return FALSE;
						}
						for (j = 0; j < len; j++) {
							if (p[j] != 0) {
								if (p[j] == '(') *np = '[';
								else if (p[j] == ')') *np = ']';
								else *np = p[j];
								np++;
							}
						}
						*np = 0;
						np++;
					}
				}
			}
			name_record++;
		}
	}
	if (!found) {
		g_free (ttf);

		return FALSE;
	}
	if (name_fields[4][0] == 0) name_fields[4] = name_fields[1];
	if (name_fields[6][0] == 0) {
		name_fields[6] = name_fields[4];
		if (name_fields[6][0] == 0) {
			g_free (ttf);

			return FALSE;
		}
	}
	p = name_fields[6];
	if (isdigit (*p)) *p += 'A'-'0';
	while (*p != 0) {
		if (!isalnum (*p) || *p == '_') *p = '-';
		p++;
	}
	g_free (ttf);

	/* copyright */
	if (name_fields[0][0] == 0) *copyright = g_strdup ("None");
	else *copyright = g_strdup (name_fields[0]);

	/* family name */
	if (name_fields[1][0] == 0) *family_name = g_strdup ("None");
	else *family_name = g_strdup (name_fields[1]);

	/* style name */
	if (name_fields[2][0] == 0) *style_name = g_strdup ("Regular");
	else *style_name = g_strdup (name_fields[2]);

	/* full name */
	*full_name = g_strdup (name_fields[4]);

	/* version */
	if (name_fields[5][0] == 0) *version = g_strdup ("0.0");
	else *version = g_strdup (name_fields[5]);

	/* ps name */
	*ps_name = g_strdup (name_fields[6]);

	return TRUE;
}
#endif

void
ft_get_font_information (FT_Face face, gchar **copyright, gchar **family_name, gchar **style_name, gchar **full_name, gchar **version, gchar **ps_name)
{
	/* We do mostly fake thngs here to avoid using format-specific FT2 code */
	if (copyright) {
		*copyright = g_strdup ("Embeddable font image by gnome-print, "
				       "not to be distributed unless explicitly allowed by original font license");
	}
	if (family_name) {
		*family_name = (face->family_name) ? g_strdup (face->family_name) : g_strdup ("Gnome Print Embedded");
	}
	if (style_name) {
		*style_name = (face->style_name) ? g_strdup (face->style_name) : g_strdup ("Regular");
	}
	if (full_name) {
		if (face->family_name && face->style_name) {
			*full_name = g_strdup_printf ("%s %s", face->family_name, face->style_name);
		} else if (face->family_name) {
			*full_name = g_strdup (face->family_name);
		} else {
			*full_name = g_strdup ("Gnome Print Embedded");
		}
	}
	if (version) *version = g_strdup ("0.0");
	if (ps_name) {
		gint i;
		if (face->family_name && face->style_name) {
			*ps_name = g_strdup_printf ("%s %s", face->family_name, face->style_name);
		} else if (face->family_name) {
			*ps_name = g_strdup (face->family_name);
		} else {
			*ps_name = g_strdup ("Gnome Print Embedded");
		}
		for (i = 0; (*ps_name)[i]; i++) if ((*ps_name)[i] <= ' ') (*ps_name)[i] = '-';
	}
#if 0
	FT_SfntName sn;

	/* copyright */
	if ((fterr = FT_Get_Sfnt_Name (face, TT_NAME_ID_COPYRIGHT, &sn)) != FT_Err_Ok) {
		*copyright = g_strdup ("");
	} else {
		*copyright = g_strndup (sn.string, sn.string_len);
	}

	/* family name */
	*family_name = g_strdup (face->family_name);

	/* style name */
	*style_name = g_strdup (face->style_name);

	/* full name */
	if ((fterr = FT_Get_Sfnt_Name (face, TT_NAME_ID_FULL_NAME, &sn)) != FT_Err_Ok) {
		*full_name = g_strdup_printf ("%s %s", face->family_name, face->style_name);
	} else {
		*full_name = g_strndup (sn.string, sn.string_len);
	}

	/* version */
	if ((fterr = FT_Get_Sfnt_Name (face, TT_NAME_ID_VERSION_STRING, &sn)) != FT_Err_Ok) {
		*version = g_strdup ("1.0");
	} else {
		*version = g_strndup (sn.string, sn.string_len);
	}

	/* ps name */
	if ((fterr = FT_Get_Sfnt_Name (face, TT_NAME_ID_PS_NAME, &sn)) != FT_Err_Ok) {
		*ps_name = g_strdup (face->family_name);
	} else {
		*ps_name = g_strndup (sn.string, sn.string_len);
	}
#endif
}

