#define __GNOME_FONT_FACE_C__

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <freetype/ftoutln.h>
#include <locale.h>

#include <libgnomeprint/gnome-print-i18n.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-font-private.h>

#define USE_GNOME_FONT_FACE_EXPERIMENTAL_INTERFACE
#include <libgnomeprint/gnome-font-face-private.h>

#include <libgnomeprint/gp-unicode.h>
#include <libgnomeprint/gp-ps-unicode.h>
#include "gp-truetype-utils.h"
#include "gp-fontmap.h"

#include "parseTT.h"

/*
 * Standard AFM attributes
 * Notice, that all distances are doubles (and FontBBox is ArtDRect)
 *
 * ItalicAngle, FontBBox, CapHeight, XHeight
 *
 * Type1 file names, if face is trivial type1 font, otherwise NULL
 * Notice, that caller has to free strings
 * Notice, that these WILL NOT be supported in gnome-font base face class,
 * but instead in some subclass
 *
 * afm, pfb, pfbname
 *
 */

enum {ARG_0,
      /* AFM Attributes */
      ARG_ITALICANGLE, ARG_FONTBBOX, ARG_CAPHEIGHT, ARG_XHEIGHT,
      /* Type1 file names */
      /* DO NOT USE THESE OUTSIDE GNOME_PRINT */
      ARG_AFM, ARG_PFB, ARG_PFBNAME
};

static void gnome_font_face_class_init (GnomeFontFaceClass * klass);
static void gnome_font_face_init (GnomeFontFace * face);
static void gnome_font_face_destroy (GtkObject * object);
static void gnome_font_face_get_arg (GtkObject * object, GtkArg * arg, guint arg_id);

static void gff_load_metrics (GnomeFontFace *face, gint glyph);
static void gff_load_outline (GnomeFontFace *face, gint glyph);

static gboolean gff_load (GnomeFontFace *face);
#define GFF_LOADED(f) ((f)->ft_face || gff_load ((GnomeFontFace *) f))

static void gff_face_from_entry (GPFontEntry *e);

#define GFE_IS_T1_A(e) (((e)->type == GP_FONT_ENTRY_TYPE1) || ((e)->type == GP_FONT_ENTRY_TYPE1_ALIAS))
#define GFE_IS_TT(e) ((e)->type == GP_FONT_ENTRY_TRUETYPE)
#define GFE_IS_S(e) ((e)->type == GP_FONT_ENTRY_SPECIAL)

static GtkObjectClass *parent_class;

GtkType
gnome_font_face_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"GnomeFontFace",
			sizeof (GnomeFontFace),
			sizeof (GnomeFontFaceClass),
			(GtkClassInitFunc) gnome_font_face_class_init,
			(GtkObjectInitFunc) gnome_font_face_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (gtk_object_get_type (), &info);
	}
	return type;
}

static void
gnome_font_face_class_init (GnomeFontFaceClass *klass)
{
	GtkObjectClass * object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	gtk_object_add_arg_type ("GnomeFontFace::ItalicAngle", GTK_TYPE_DOUBLE, GTK_ARG_READABLE, ARG_ITALICANGLE);
	gtk_object_add_arg_type ("GnomeFontFace::FontBBox", GTK_TYPE_BOXED, GTK_ARG_READABLE, ARG_FONTBBOX);
	gtk_object_add_arg_type ("GnomeFontFace::CapHeight", GTK_TYPE_DOUBLE, GTK_ARG_READABLE, ARG_CAPHEIGHT);
	gtk_object_add_arg_type ("GnomeFontFace::XHeight", GTK_TYPE_DOUBLE, GTK_ARG_READABLE, ARG_XHEIGHT);
	gtk_object_add_arg_type ("GnomeFontFace::afm", GTK_TYPE_STRING, GTK_ARG_READABLE, ARG_AFM);
	gtk_object_add_arg_type ("GnomeFontFace::pfb", GTK_TYPE_STRING, GTK_ARG_READABLE, ARG_PFB);
	gtk_object_add_arg_type ("GnomeFontFace::pfbname", GTK_TYPE_STRING, GTK_ARG_READABLE, ARG_PFBNAME);

	object_class->destroy = gnome_font_face_destroy;
	object_class->get_arg = gnome_font_face_get_arg;
}

static void
gnome_font_face_init (GnomeFontFace * face)
{
	face->entry = NULL;
	face->num_glyphs = 0;
	face->glyphs = NULL;
	face->ft_face = NULL;
}

static void
gnome_font_face_destroy (GtkObject *object)
{
	GnomeFontFace * face;

	face = (GnomeFontFace *) object;

	if (face->entry) {
		face->entry->face = NULL;
		gp_font_entry_unref (face->entry);
		face->entry = NULL;
	}

	if (face->glyphs) {
		gint i;
		for (i = 0; i < face->num_glyphs; i++) {
			if (face->glyphs[i].bpath) g_free (face->glyphs[i].bpath);
		}
		g_free (face->glyphs);
		face->glyphs = NULL;
	}

	if (face->ft_face) {
		FT_Done_Face (face->ft_face);
		face->ft_face = NULL;
	}

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
gnome_font_face_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GnomeFontFace *face;
	const ArtDRect *fbbox;

	face = GNOME_FONT_FACE (object);

	switch (arg_id) {
	case ARG_ITALICANGLE:
		/* fixme: (Lauris) */
		GTK_VALUE_DOUBLE (*arg) = gnome_font_face_is_italic (face) ? -20.0 : 0.0;
		break;
	case ARG_FONTBBOX:
		fbbox = gnome_font_face_get_stdbbox (face);
		if (fbbox) {
			ArtDRect *bbox;
			bbox = g_new (ArtDRect, 1);
			*bbox = *fbbox;
			GTK_VALUE_BOXED (*arg) = bbox;
		} else {
			GTK_VALUE_BOXED (*arg) = NULL;
		}
		break;
	case ARG_CAPHEIGHT:
		/* fixme: (Lauris) */
		GTK_VALUE_DOUBLE (*arg) = 900.0;
		break;
	case ARG_XHEIGHT:
		/* fixme: (Lauris) */
		GTK_VALUE_DOUBLE (*arg) = 600.0;
		break;
	case ARG_AFM:
		if ((face->entry->type == GP_FONT_ENTRY_TYPE1) ||
		    (face->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS)) {
			GTK_VALUE_STRING (*arg) = g_strdup (((GPFontEntryT1 *) face->entry)->afm.name);
		} else {
			GTK_VALUE_STRING (*arg) = NULL;
		}
		break;
	case ARG_PFB:
		if ((face->entry->type == GP_FONT_ENTRY_TYPE1) ||
		    (face->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS)) {
			GTK_VALUE_STRING (*arg) = g_strdup (((GPFontEntryT1 *) face->entry)->pfb.name);
		} else {
			GTK_VALUE_STRING (*arg) = NULL;
		}
		break;
	case ARG_PFBNAME:
		if (face->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS) {
			GTK_VALUE_STRING (*arg) = g_strdup (((GPFontEntryT1Alias *) face->entry)->alias);
		} else if (face->entry->type == GP_FONT_ENTRY_TYPE1) {
			GTK_VALUE_STRING (*arg) = g_strdup (face->entry->psname);
		} else {
			GTK_VALUE_STRING (*arg) = NULL;
		}
		break;
	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

/*
 * Naming
 *
 * gnome_font_face_get_name () should return one "true" name for font, that
 * does not have to be its PostScript name.
 * In future those names can be possibly localized (except ps_name)
 */

const gchar *
gnome_font_face_get_name (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->entry->name;
}

const gchar *
gnome_font_face_get_family_name (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->entry->familyname;
}

const gchar *
gnome_font_face_get_species_name (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->entry->speciesname;
}

const gchar *
gnome_font_face_get_ps_name (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return face->entry->psname;
}

/*
 * Returns number of defined glyphs in typeface
 */

gint
gnome_font_face_get_num_glyphs (const GnomeFontFace * face)
{
	g_return_val_if_fail (face != NULL, 0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return 0;
	}

	return face->num_glyphs;
}

/*
 * Metrics
 *
 * Currently those return standard values for left to right, horizontal script
 * The prefix std means, that there (a) will hopefully be methods to extract
 * different metric values and (b) for given face one text direction can
 * be defined as "default"
 * All face metrics are given in 0.001 em units
 */

const ArtDRect *
gnome_font_face_get_stdbbox (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return NULL;
	}

	return &face->bbox;
}

ArtPoint *
gnome_font_face_get_glyph_stdadvance (const GnomeFontFace * face, gint glyph, ArtPoint * advance)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	g_return_val_if_fail (advance != NULL, NULL);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return NULL;
	}

	if ((glyph < 0) || (glyph >= face->num_glyphs)) glyph = 0;

	if (!face->glyphs[glyph].metrics) gff_load_metrics ((GnomeFontFace *) face, glyph);

	*advance = face->glyphs[glyph].advance;

	return advance;
}

ArtDRect *
gnome_font_face_get_glyph_stdbbox (const GnomeFontFace * face, gint glyph, ArtDRect * bbox)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);
	g_return_val_if_fail (bbox != NULL, NULL);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return NULL;
	}

	if ((glyph < 0) || (glyph >= face->num_glyphs)) glyph = 0;

	if (!face->glyphs[glyph].metrics) gff_load_metrics ((GnomeFontFace *) face, glyph);

	*bbox = face->glyphs[glyph].bbox;

	return bbox;
}

const ArtBpath *
gnome_font_face_get_glyph_stdoutline (const GnomeFontFace * face, gint glyph)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return NULL;
	}

	if ((glyph < 0) || (glyph >= face->num_glyphs)) glyph = 0;

	if (!face->glyphs[glyph].bpath) gff_load_outline ((GnomeFontFace *) face, glyph);

	return face->glyphs[glyph].bpath;
}

/*
 * Give (possibly localized) demonstration text for given face
 * Most usually this tells about quick fox and lazy dog...
 */

const gchar *
gnome_font_face_get_sample (const GnomeFontFace * face)
{
	return _("The quick brown fox jumps over the lazy dog.");
}

GnomeFontFace *
gnome_font_face_new (const gchar * name)
{
	GPFontMap * map;
	GPFontEntry * e;

	if (name) {
		map = gp_fontmap_get ();

		e = g_hash_table_lookup (map->fontdict, name);
		if (!e) {
			gp_fontmap_release (map);
			return NULL;
		}
		if (e->face) {
			gnome_font_face_ref (e->face);
			gp_fontmap_release (map);
			return e->face;
		}

		gff_face_from_entry (e);

		gp_fontmap_release (map);

		return (e->face);
	} else {
		return gnome_font_unsized_closest (NULL, GNOME_FONT_BOOK, FALSE);
	}
}

/* 
 * Find the closest weight matching the family name, weight, and italic
 * specs. Return the unsized font.
 */

GnomeFontUnsized *
gnome_font_unsized_closest (const char *family_name,
			    GnomeFontWeight weight,
			    gboolean italic)
{
	GPFontMap * map;
	GPFontEntry * best, * entry;
	GnomeFontFace * face;
	int best_dist, dist;
	GSList * l;

	/* This should be reimplemented to use the gnome_font_family_hash. */

	map = gp_fontmap_get ();

	best = NULL;
	face = NULL;

	if (family_name) {

		best_dist = 1000000;

		for (l = map->fonts; l != NULL; l = l->next) {
			entry = (GPFontEntry *) l->data;
			if (!strcasecmp (family_name, entry->familyname)) {
				dist = abs (weight - entry->Weight) + 100 * (italic != (entry->ItalicAngle != 0));
				/* Hack to prefer normal to narrow */
				if (strstr (entry->speciesname, "Narrow")) dist += 6;
				if (dist < best_dist) {
					best_dist = dist;
					best = entry;
				}
			}
		}
	} else {
		g_warning ("file %s: line %d: No font name specified, using default", __FILE__, __LINE__);
	}

	if (best) {
		face = gnome_font_face_new (best->name);
	} else {
		/* No match, try default */
		guchar *locale, *p;
		guchar c[128];
		GPFontEntry *e;
		locale = setlocale (LC_ALL, NULL);
		if (!locale) locale = "C";
		strncpy (c, locale, 127);
		c[127] = '\0';
		p = c;
		while (isalpha (*p) || (*p == '_')) p += 1;
		if (*p) *p = '\0';
		e = g_hash_table_lookup (map->defaultsdict, c);
		if (!e) e = g_hash_table_lookup (map->defaultsdict, "C");
		if (e) {
			if (e->face) {
				gnome_font_face_ref (e->face);
			} else {
				gff_face_from_entry (e);
			}
			face = e->face;
		}
	}
	if (!face && map->fonts) {
		/* No face, no helvetica, load whatever font is first */
		GPFontEntry *e;
		e = (GPFontEntry *) map->fonts->data;
		if (e->face) {
			gnome_font_face_ref (e->face);
		} else {
			gff_face_from_entry (e);
		}
		face = e->face;
	}

	gp_fontmap_release (map);

	g_return_val_if_fail (face != NULL, NULL);

	return face;
}

#ifdef USE_GNOME_FONT_FACE_EXPERIMENTAL_INTERFACE

GnomeFontFace *
gnome_font_face_private_from_files (const guchar *filename, gint facenum, const GSList *additional)
{
	static gint num = 0;
	GnomeFontFace *face;
	GPFontMap *map;
	GPFontEntry *e;
	guchar *n, *f, *s;

	num += 1;

	n = g_strdup_printf ("Gnome Print Internal %d", num);
	f = g_strdup_printf ("Gnome Print");
	s = g_strdup_printf ("Internal %d", num);

	map = gp_fontmap_get ();

	e = gp_font_entry_from_files (map, n, f, s, TRUE, filename, facenum, additional);

	if (e) {
		gff_face_from_entry (e);
		face = e->face;
		gp_font_entry_unref (e);
	} else {
		face = NULL;
	}

	gp_fontmap_release (map);

	return face;
}

#endif

GnomeFont *
gnome_font_face_get_font (const GnomeFontFace * face, gdouble size, gdouble xres, gdouble yres)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	return gnome_font_face_get_font_full ((GnomeFontFace *) face, size, NULL);
}

GnomeFont *
gnome_font_face_get_font_default (const GnomeFontFace * face, gdouble size)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	/* fixme: At least try to look like doing something (Lauris) */
	return gnome_font_face_get_font_full ((GnomeFontFace *) face, size, NULL);
}

/* fixme: */
/* Returns the glyph width in 0.001 unit */

gdouble
gnome_font_face_get_glyph_width (const GnomeFontFace *face, gint glyph)
{
	ArtPoint p;

	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);

	p.x = 0;
	gnome_font_face_get_glyph_stdadvance (face, glyph, &p);

	return p.x;
}

/*
 * Get the glyph number corresponding to a given unicode, or -1 if it
 * is not mapped.
 *
 * fixme: We use ugly hack to avoid segfaults everywhere
 */

gint
gnome_font_face_lookup_default (const GnomeFontFace * face, gint unicode)
{
	g_return_val_if_fail (face != NULL, -1);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -1);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return -1;
	}

	/* fixme: Nobody should ask mapping of 0 */
	if (unicode < 1) return 0;

#if 0
	g_print ("glyph %d lookup %d\n", unicode, (gint) FT_Get_Char_Index (face->ft_face, unicode));
#endif

	return FT_Get_Char_Index (face->ft_face, unicode);
}

gboolean
gff_load (GnomeFontFace *face)
{
	FT_Face ft_face;
	FT_Error ft_result;
	static FT_Library ft_library = NULL;
	GPFontEntry *entry;

	if (!ft_library) {
		ft_result = FT_Init_FreeType (&ft_library);
		g_return_val_if_fail (ft_result == FT_Err_Ok, FALSE);
	}

	entry = face->entry;

	while (entry && entry->type == GP_FONT_ENTRY_ALIAS) entry = ((GPFontEntryAlias *) face->entry)->ref;
	if (!entry) {
		g_warning ("file %s: line %d: face %s: Floating alias list detected", __FILE__, __LINE__, entry->name);
		return FALSE;
	}

	if (GFE_IS_T1_A (entry)) {
		GPFontEntryT1 *t1;
		t1 = (GPFontEntryT1 *) entry;
		ft_result = FT_New_Face (ft_library, t1->pfb.name, 0, &ft_face);
		g_return_val_if_fail (ft_result == FT_Err_Ok, FALSE);
		if (t1->afm.name) {
			ft_result = FT_Attach_File (ft_face, t1->afm.name);
			if (ft_result != FT_Err_Ok) {
				g_warning ("file %s: line %d: face %s: Cannot attach file %s", __FILE__, __LINE__, entry->name, t1->afm.name);
			}
		}
	} else if (GFE_IS_TT (entry)) {
		GPFontEntryTT *tt;
		tt = (GPFontEntryTT *) entry;
		ft_result = FT_New_Face (ft_library, tt->ttf.name, tt->facenum, &ft_face);
		g_return_val_if_fail (ft_result == FT_Err_Ok, FALSE);
	} else if (GFE_IS_S (entry)) {
		GPFontEntrySpecial *s;
		GSList *l;
		s = (GPFontEntrySpecial *) entry;
		ft_result = FT_New_Face (ft_library, s->file.name, 0, &ft_face);
		g_return_val_if_fail (ft_result == FT_Err_Ok, FALSE);
		for (l = s->additional; l != NULL; l = l->next) {
			ft_result = FT_Attach_File (ft_face, (gchar *) l->data);
			if (ft_result != FT_Err_Ok) {
				g_warning ("file %s: line %d: face %s: Cannot attach file %s",
					   __FILE__, __LINE__, entry->name, (gchar *) l->data);
			}
		}
	} else {
		g_assert_not_reached ();
	}

	/* fixme: scalability */
	/* fixme: glyph names */

	face->ft_face = ft_face;

	ft_result = FT_Select_Charmap (ft_face, ft_encoding_unicode);
	if (ft_result != FT_Err_Ok) {
		g_warning ("file %s: line %d: Face %s does not have unicode charmap", __FILE__, __LINE__, entry->name);
	}

	face->num_glyphs = ft_face->num_glyphs;
	g_return_val_if_fail (face->num_glyphs > 0, FALSE);
	face->glyphs = g_new0 (GFFGlyphInfo, face->num_glyphs);

	face->ft2ps = 1000.0 / ft_face->units_per_EM;

	face->bbox.x0 = ft_face->bbox.xMin * face->ft2ps;
	face->bbox.y0 = ft_face->bbox.yMin * face->ft2ps;
	face->bbox.x1 = ft_face->bbox.xMax * face->ft2ps;
	face->bbox.y1 = ft_face->bbox.yMax * face->ft2ps;

	return TRUE;
}

static void
gff_load_metrics (GnomeFontFace *face, gint glyph)
{
	GFFGlyphInfo * gi;

	g_assert (face->ft_face);
	g_assert (!face->glyphs[glyph].metrics);

	gi = face->glyphs + glyph;

	FT_Load_Glyph (face->ft_face, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);

        gi->bbox.x0 = -face->ft_face->glyph->metrics.horiBearingX * face->ft2ps;
	gi->bbox.y1 = face->ft_face->glyph->metrics.horiBearingY * face->ft2ps;
	gi->bbox.y0 = gi->bbox.y1 - face->ft_face->glyph->metrics.height * face->ft2ps;
	gi->bbox.x1 = gi->bbox.x0 + face->ft_face->glyph->metrics.width * face->ft2ps;
	gi->advance.x = face->ft_face->glyph->metrics.horiAdvance * face->ft2ps;
	gi->advance.y = 0.0;

	face->glyphs[glyph].metrics = TRUE;
}

static ArtBpath *gff_ol2bp (FT_Outline * ol, gdouble transform[]);

static void
gff_load_outline (GnomeFontFace *face, gint glyph)
{
	gdouble a[6];

	g_assert (face->ft_face);
	g_assert (!face->glyphs[glyph].bpath);

	FT_Load_Glyph (face->ft_face, glyph, FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);

	a[0] = a[3] = face->ft2ps;
	a[1] = a[2] = a[4] = a[5] = 0.0;

	face->glyphs[glyph].bpath = gff_ol2bp (&face->ft_face->glyph->outline, a);
}

/* Bpath methods */

typedef struct {
	ArtBpath * bp;
	gint start, end;
	gdouble * t;
} GFFT2OutlineData;

static int gfft2_move_to (FT_Vector * to, void * user)
{
	GFFT2OutlineData * od;
	ArtBpath * s;
	ArtPoint p;

	od = (GFFT2OutlineData *) user;

	s = &od->bp[od->end - 1];

	p.x = to->x * od->t[0] + to->y * od->t[2];
	p.y = to->x * od->t[1] + to->y * od->t[3];

	if ((p.x != s->x3) || (p.y != s->y3)) {
		od->bp[od->end].code = ART_MOVETO;
		od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2];
		od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3];
		od->end++;
	}

	return 0;
}

static int gfft2_line_to (FT_Vector * to, void * user)
{
	GFFT2OutlineData * od;
	ArtBpath * s;
	ArtPoint p;

	od = (GFFT2OutlineData *) user;

	s = &od->bp[od->end - 1];

	p.x = to->x * od->t[0] + to->y * od->t[2];
	p.y = to->x * od->t[1] + to->y * od->t[3];

	if ((p.x != s->x3) || (p.y != s->y3)) {
		od->bp[od->end].code = ART_LINETO;
		od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2];
		od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3];
		od->end++;
	}

	return 0;
}

static int gfft2_conic_to (FT_Vector * control, FT_Vector * to, void * user)
{
	GFFT2OutlineData * od;
	ArtBpath * s, * e;
	ArtPoint c;

	od = (GFFT2OutlineData *) user;
	g_return_val_if_fail (od->end > 0, -1);

	s = &od->bp[od->end - 1];
	e = &od->bp[od->end];

	e->code = ART_CURVETO;

	c.x = control->x * od->t[0] + control->y * od->t[2];
	c.y = control->x * od->t[1] + control->y * od->t[3];
	e->x3 = to->x * od->t[0] + to->y * od->t[2];
	e->y3 = to->x * od->t[1] + to->y * od->t[3];

	od->bp[od->end].x1 = c.x - (c.x - s->x3) / 3;
	od->bp[od->end].y1 = c.y - (c.y - s->y3) / 3;
	od->bp[od->end].x2 = c.x + (e->x3 - c.x) / 3;
	od->bp[od->end].y2 = c.y + (e->y3 - c.y) / 3;
	od->end++;

	return 0;
}

static int gfft2_cubic_to (FT_Vector * control1, FT_Vector * control2, FT_Vector * to, void * user)
{
	GFFT2OutlineData * od;

	od = (GFFT2OutlineData *) user;

	od->bp[od->end].code = ART_CURVETO;
	od->bp[od->end].x1 = control1->x * od->t[0] + control1->y * od->t[2];
	od->bp[od->end].y1 = control1->x * od->t[1] + control1->y * od->t[3];
	od->bp[od->end].x2 = control2->x * od->t[0] + control2->y * od->t[2];
	od->bp[od->end].y2 = control2->x * od->t[1] + control2->y * od->t[3];
	od->bp[od->end].x3 = to->x * od->t[0] + to->y * od->t[2];
	od->bp[od->end].y3 = to->x * od->t[1] + to->y * od->t[3];
	od->end++;

	return 0;
}

FT_Outline_Funcs gfft2_outline_funcs = {
	gfft2_move_to,
	gfft2_line_to,
	gfft2_conic_to,
	gfft2_cubic_to,
	0, 0
};

/*
 * We support only 4x4 matrix here (do you need more?)
 */

static ArtBpath *
gff_ol2bp (FT_Outline * ol, gdouble transform[])
{
	GFFT2OutlineData od;

	od.bp = g_new (ArtBpath, ol->n_points * 2 + ol->n_contours + 1);
	od.start = od.end = 0;
	od.t = transform;

	FT_Outline_Decompose (ol, &gfft2_outline_funcs, &od);

	od.bp[od.end].code = ART_END;

	/* fixme: g_renew */

	return od.bp;
}

/*
 * OLD FONT CODE BEGINS HERE
 */

/* fixme: */
/* return a pointer to the (PostScript) name of the font */
const gchar * gnome_font_unsized_get_glyph_name (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	if (face->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS) {
		return ((GPFontEntryT1Alias *) face->entry)->alias;
	} else {
		return face->entry->psname;
	}
}

gdouble
gnome_font_face_get_ascender (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, 1000.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 1000.0);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return 1000.0;
	}

	return face->ft_face->ascender * face->ft2ps;
}

gdouble
gnome_font_face_get_descender (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, 500.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 500.0);
	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return 500.0;
	}

	return -face->ft_face->descender * face->ft2ps;
}

gdouble
gnome_font_face_get_underline_position (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, -100.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), -100.0);
	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return -100.0;
	}

	return face->ft_face->underline_position * face->ft2ps;
}

gdouble
gnome_font_face_get_underline_thickness (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return 0.0;
	}

	return face->ft_face->underline_thickness * face->ft2ps;
}

GnomeFontWeight
gnome_font_face_get_weight_code (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, GNOME_FONT_BOOK);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), GNOME_FONT_BOOK);

	return face->entry->Weight;
}

gboolean
gnome_font_face_is_italic (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), FALSE);

	return (face->entry->ItalicAngle != 0.0);

	return FALSE;
}

gboolean
gnome_font_face_is_fixed_width (const GnomeFontFace *face)
{
	g_return_val_if_fail (face != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), FALSE);

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		return FALSE;
	}

	return FT_IS_FIXED_WIDTH (face->ft_face);
}



/*
 * Returns PostScript name for glyph
 */

const gchar *
gnome_font_face_get_glyph_ps_name (const GnomeFontFace *face, gint glyph)
{
	static GHashTable *sgd = NULL;
	FT_Error status;
	gchar c[256], *name;

	g_return_val_if_fail (face != NULL, ".notdef");
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), ".notdef");

	if (!GFF_LOADED (face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load metrics", __FILE__, __LINE__, face->entry->name);
		return ".notdef";
	}

	if (!sgd) sgd = g_hash_table_new (g_str_hash, g_str_equal);

	if ((glyph < 0) || (glyph >= face->num_glyphs)) glyph = 0;

	status = FT_Get_Glyph_Name (face->ft_face, glyph, c, 256);
	if (status != FT_Err_Ok) return ".notdef";

	name = g_hash_table_lookup (sgd, c);
	if (!name) {
		name = g_strdup (c);
		g_hash_table_insert (sgd, name, name);
	}

	return name;
}

/*
 * Creates new face and creates link with FontEntry
 */

static void
gff_face_from_entry (GPFontEntry *e)
{
	GnomeFontFace *face;

	g_return_if_fail (e->face == NULL);

	face = gtk_type_new (GNOME_TYPE_FONT_FACE);

	gp_font_entry_ref (e);
	face->entry = e;
	e->face = face;
}

/* PSO (PostScriptObject stuff */

static void gff_pso_ensure_buffer_t1 (GFPSObject *pso, GPFontEntryT1 *t1);
static void gff_pso_ensure_buffer_tt (GFPSObject *pso, GPFontEntryTT *tt);
static void gff_pso_ensure_buffer_empty (GFPSObject *pso);
static void gf_pso_sprintf (GFPSObject * pso, const gchar * format, ...);
static void gf_pso_ensure_space (GFPSObject *pso, gint size);

/*
 * Generates intial PSObject
 */

GFPSObject *
gnome_font_face_pso_new (GnomeFontFace *face, guchar *residentname)
{
	GFPSObject *pso;

	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	pso = g_new0 (GFPSObject, 1);

	pso->face = face;
	gtk_object_ref (GTK_OBJECT (face));
	if (residentname) pso->residentname = g_strdup (residentname);
	pso->encodedname = g_strdup_printf ("GnomeUni-%s", face->entry->psname);
	pso->bufsize = 0;
	pso->length = 0;
	pso->buf = NULL;
	if (GFF_LOADED (face)) {
		pso->encodedbytes = (face->num_glyphs < 256) ? 1 : 2;
		pso->num_glyphs = face->num_glyphs;
		pso->glyphs = g_new0 (guint32, (pso->num_glyphs + 32) >> 5);
	} else {
		g_warning ("file %s: line %d: Face: %s: Cannot load face", __FILE__, __LINE__, face->entry->name);
		pso->encodedbytes = 1;
		pso->num_glyphs = 1;
		pso->glyphs = NULL;
		gff_pso_ensure_buffer_empty (pso);
	}

	return pso;
}

#define PSO_GLYPH_MARKED(pso,g) (pso->glyphs[(g) >> 5] & (1 << ((g) & 0x1f)))

void
gnome_font_face_pso_mark_glyph (GFPSObject *pso, gint glyph)
{
	g_return_if_fail (pso != NULL);

	if (!pso->glyphs) return;

	glyph = CLAMP (glyph, 0, pso->num_glyphs);

	pso->glyphs[glyph >> 5] |= (1 << (glyph & 0x1f));
}

void
gnome_font_face_pso_free (GFPSObject *pso)
{
	g_return_if_fail (pso != NULL);

	if (pso->face) gtk_object_unref (GTK_OBJECT (pso->face));
	if (pso->residentname) g_free (pso->residentname);
	if (pso->encodedname) g_free (pso->encodedname);
	if (pso->glyphs) g_free (pso->glyphs);
	if (pso->buf) g_free (pso->buf);
	g_free (pso);
}

void
gnome_font_face_pso_ensure_buffer (GFPSObject *pso)
{
	GPFontEntry *entry;

	g_return_if_fail (pso != NULL);

	entry = pso->face->entry;

	while (entry && (entry->type == GP_FONT_ENTRY_ALIAS)) entry = ((GPFontEntryAlias *) entry)->ref;
	if (!entry) {
		g_warning ("file %s: line %d: Floating alias list detected for %s",
			   __FILE__, __LINE__, pso->face->entry->name);
		gff_pso_ensure_buffer_empty (pso);
		return;
	}

	switch (entry->type) {
	case GP_FONT_ENTRY_TYPE1:
	case GP_FONT_ENTRY_TYPE1_ALIAS:
		gff_pso_ensure_buffer_t1 (pso, (GPFontEntryT1 *) entry);
		break;
	case GP_FONT_ENTRY_TRUETYPE:
		gff_pso_ensure_buffer_tt (pso, (GPFontEntryTT *) entry);
		break;
	default:
		g_warning ("file %s: line %d: Unknown face entry type %s:%d",
			   __FILE__, __LINE__, entry->name, entry->type);
		gff_pso_ensure_buffer_empty (pso);
		break;
	}
}

#define INT32_LSB(q) ((q)[0] + ((q)[1] << 8) + ((q)[2] << 16) + ((q)[3] << 24))

static void
gff_pso_ensure_buffer_t1 (GFPSObject *pso, GPFontEntryT1 *t1)
{
	struct stat st;
	guchar * fbuf;
	gint fh;
	const gchar *embeddedname;

	if (t1->entry.type == GP_FONT_ENTRY_TYPE1) {
		embeddedname = t1->entry.psname;
	} else {
		embeddedname = ((GPFontEntryT1Alias *) t1)->alias;
	}

	if (!GFF_LOADED (pso->face)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load face", __FILE__, __LINE__, pso->face->entry->name);
		gff_pso_ensure_buffer_empty (pso);
		return;
	}

	/* Step 0: mmap pfb file */
	if (stat (t1->pfb.name, &st)) {
		g_warning ("file %s: line %d: Cannot stat font file %s", __FILE__, __LINE__, t1->pfb.name);
		gff_pso_ensure_buffer_empty (pso);
		return;
	}

	fh = open (t1->pfb.name, O_RDONLY);
	if (fh < 0) {
		g_warning ("file %s: line %d: Cannot open font file %s", __FILE__, __LINE__, t1->pfb.name);
		gff_pso_ensure_buffer_empty (pso);
		return;
	}

	fbuf = mmap (NULL, st.st_size, PROT_READ, MAP_SHARED, fh, 0);
	close (fh);
	if ((fbuf == NULL) || (fbuf == (guchar *) -1)) {
		g_warning ("file %s: line %d: Cannot open font file %s", __FILE__, __LINE__, t1->pfb.name);
		gff_pso_ensure_buffer_empty (pso);
		return;
	}

	if (*fbuf == 0x80) {
		const char hextab[16] = "0123456789abcdef";
		gint idx;
		/* this is actually the same as a pfb to pfa converter
		 * Reference: Adobe technical note 5040, "Supporting Downloadable PostScript
		 * Language Fonts", page 9
		 */
		idx = 0;
		while (idx < st.st_size) {
			gint length, i;
			if (fbuf[idx] != 0x80) {
				g_warning ("file %s: line %d: Corrupt %s", __FILE__, __LINE__, t1->pfb.name);
				gff_pso_ensure_buffer_empty (pso);
				return;
			}
			switch (fbuf[idx + 1]) {
			case 1:
				length = INT32_LSB (fbuf + idx + 2);
				gf_pso_ensure_space (pso, length);
				idx += 6;
				memcpy (pso->buf + pso->length, fbuf + idx, length);
				pso->length += length;
				idx += length;
				break;
			case 2:
				length = INT32_LSB (fbuf + idx + 2);
				gf_pso_ensure_space (pso, length * 3);
				idx += 6;
				for (i = 0; i < length; i++) {
					pso->buf[pso->length++] = hextab[fbuf[idx] >> 4];
					pso->buf[pso->length++] = hextab[fbuf[idx] & 15];
					idx += 1;
					if ((i & 31) == 31 || i == length - 1) pso->buf[pso->length++] = '\n';
				}
				break;
			case 3:
				/* Finished */
				gf_pso_ensure_space (pso, 1);
				pso->buf[pso->length++] = '\n';
				idx = st.st_size;
				break;
			default:
				g_warning ("file %s: line %d: Corrupt %s", __FILE__, __LINE__, t1->pfb.name);
				gff_pso_ensure_buffer_empty (pso);
				return;
			}
		}
	} else {
		memcpy (pso->buf, fbuf, st.st_size);
		pso->buf[st.st_size] = '\0';
		pso->length = st.st_size;
	}

	/* Unmap file */
	munmap (fbuf, st.st_size);

	if (pso->encodedbytes == 1) {
		gint glyph;
		/* 8-bit vector */
		gf_pso_sprintf (pso, "/%s findfont dup length dict begin\n", embeddedname);
		gf_pso_sprintf (pso, "{1 index /FID ne {def} {pop pop} ifelse} forall\n");
		gf_pso_sprintf (pso, "/Encoding [\n");
		for (glyph = 0; glyph < 256; glyph++) {
			guint g;
			gchar c[256];
			FT_Error status;
			g = (glyph < pso->face->num_glyphs) ? glyph : 0;
			status = FT_Get_Glyph_Name (pso->face->ft_face, glyph, c, 256);
			if (status != FT_Err_Ok) {
				g_warning ("file %s: line %d: Glyph %d has no name in %s", __FILE__, __LINE__, glyph, t1->pfb.name);
#if 0
				gff_pso_ensure_buffer_empty (pso);
				return;
#else
				g_snprintf (c, 256, ".notdef");
#endif
			}
			gf_pso_sprintf (pso, ((glyph & 0xf) == 0xf) ? "/%s\n" : "/%s ", c);
		}
		gf_pso_sprintf (pso, "] def currentdict end\n");
		gf_pso_sprintf (pso, "/%s exch definefont pop\n", pso->encodedname);
	} else {
		gint nfonts, nglyphs, i, j;
		/* 16-bit vector */
		nglyphs = pso->face->num_glyphs;
		nfonts = (nglyphs + 255) >> 8;

		gf_pso_sprintf (pso, "32 dict begin\n");
		/* Common entries */
		gf_pso_sprintf (pso, "/FontType 0 def\n");
		gf_pso_sprintf (pso, "/FontMatrix [1 0 0 1 0 0] def\n");
		gf_pso_sprintf (pso, "/FontName /%s-Glyph-Composite def\n", embeddedname);
		gf_pso_sprintf (pso, "/LanguageLevel 2 def\n");

		/* Type 0 entries */
		gf_pso_sprintf (pso, "/FMapType 2 def\n");

		/* Bitch 'o' bitches */
		gf_pso_sprintf (pso, "/FDepVector [\n");
		for (i = 0; i < nfonts; i++) {
			gf_pso_sprintf (pso, "/%s findfont dup length dict begin\n", embeddedname);
			gf_pso_sprintf (pso, "{1 index /FID ne {def} {pop pop} ifelse} forall\n");
			gf_pso_sprintf (pso, "/Encoding [\n");
			for (j = 0; j < 256; j++) {
				gint glyph;
				gchar c[256];
				FT_Error status;
				glyph = 256 * i + j;
				if (glyph >= nglyphs) glyph = 0;
				status = FT_Get_Glyph_Name (pso->face->ft_face, glyph, c, 256);
				if (status != FT_Err_Ok) {
					g_warning ("file %s: line %d: Glyph %d has no name in %s", __FILE__, __LINE__, glyph, t1->pfb.name);
#if 0
					gff_pso_ensure_buffer_empty (pso);
					return;
#else
					g_snprintf (c, 256, ".notdef");
#endif
				}
				gf_pso_sprintf (pso, ((j & 0xf) == 0xf) ? "/%s\n" : "/%s ", c);
			}
			gf_pso_sprintf (pso, "] def\n");
			gf_pso_sprintf (pso, "currentdict end /%s-Glyph-Page-%d exch definefont\n", embeddedname, i);
		}
		gf_pso_sprintf (pso, "] def\n");
		gf_pso_sprintf (pso, "/Encoding [\n");
		for (i = 0; i < 256; i++) {
			gint fn;
			fn = (i < nfonts) ? i : 0;
			gf_pso_sprintf (pso, ((i & 0xf) == 0xf) ? "%d\n" : "%d  ", fn);
		}
		gf_pso_sprintf (pso, "] def\n");
		gf_pso_sprintf (pso, "currentdict end\n");
		gf_pso_sprintf (pso, "/%s exch definefont pop\n", pso->encodedname);
	}
}

/*
 * References:
 * Adobe Inc., PostScript Language Reference, 3rd edition, Addison Wesley 1999
 * Adobe Inc., The Type42 Font Format Specification, 1998 <http://partners.adobe.com/asn/developer/PDFS/TN/5012.Type42_Spec.pdf>
 */

static void
gff_pso_ensure_buffer_tt (GFPSObject *pso, GPFontEntryTT *tt)
{
	struct stat st;
	guchar *fbuf;
	gint fh;
	GSList *strings;
	const gchar * embeddedname;
	gdouble TTVersion, MfrRevision;
	const ArtDRect *bbox;
	gint i;

	/* mmap file */
	if (stat (tt->ttf.name, &st)) {
		g_warning ("file %s: line %d: Face: %s: Cannot load face", __FILE__, __LINE__, tt->entry.name);
		gff_pso_ensure_buffer_empty (pso);
		return;
	}

	fh = open (tt->ttf.name, O_RDONLY);
	if (fh < 0) {
		g_warning ("file %s: line %d: Cannot open font file %s", __FILE__, __LINE__, tt->ttf.name);
		gff_pso_ensure_buffer_empty (pso);
		return;
	}

	fbuf = mmap (NULL, st.st_size, PROT_READ, MAP_SHARED, fh, 0);
	close (fh);
	if ((fbuf == NULL) || (fbuf == (guchar *) -1)) {
		g_warning ("file %s: line %d: Cannot open font file %s", __FILE__, __LINE__, tt->ttf.name);
		gff_pso_ensure_buffer_empty (pso);
		return;
	}

	embeddedname = tt->entry.psname;

	strings = gp_tt_split_file (fbuf, st.st_size);
	if (strings) {
		/* Relatively easy - just embed TTF into PS stream */
		/* Download master font */
		TTVersion = 1.0;
		MfrRevision = 1.0;
		gf_pso_sprintf (pso, "%%!PS-TrueTypeFont-%g-%g\n", TTVersion, MfrRevision);
		gf_pso_sprintf (pso, "11 dict begin\n");
		gf_pso_sprintf (pso, "/FontName /%s def\n", embeddedname);
		gf_pso_sprintf (pso, "/Encoding 256 array\n");
		gf_pso_sprintf (pso, "0 1 255 {1 index exch /.notdef put} for\n");
		gf_pso_sprintf (pso, "readonly def\n");
		gf_pso_sprintf (pso, "/PaintType 0 def\n");
		gf_pso_sprintf (pso, "/FontMatrix [1 0 0 1 0 0] def\n");
		/* fixme: */
		bbox = gnome_font_face_get_stdbbox (pso->face);
		gf_pso_sprintf (pso, "/FontBBox [%g %g %g %g] def\n", bbox->x0, bbox->y0, bbox->x1, bbox->y1);
		gf_pso_sprintf (pso, "/FontType 42 def\n");
		/* fixme: XUID */
		/* fixme: Be more intelligent */
		gf_pso_sprintf (pso, "/sfnts [\n");
		while (strings) {
			guint start, next, size, i;
			start = GPOINTER_TO_UINT (strings->data);
			strings = g_slist_remove (strings, strings->data);
			next = strings ? GPOINTER_TO_UINT (strings->data) : st.st_size;
			size = next - start;
			gf_pso_sprintf (pso, "<\n");
			for (i = start; i < next; i+= 32) {
				gint e, j;
				e = MIN (i + 32, next);
				for (j = i; j < e; j++) {
					gf_pso_sprintf (pso, "%.2x", *(fbuf + j));
				}
				gf_pso_sprintf (pso, "\n");
			}
			gf_pso_sprintf (pso, strings ? ">\n" : "00>\n");
		}
		gf_pso_sprintf (pso, "] def\n");
		/* fixme: Use CID or something */
		gf_pso_sprintf (pso, "/CharStrings %d dict dup begin\n", pso->face->num_glyphs);
		gf_pso_sprintf (pso, "/.notdef 0 def\n");
		for (i = 1; i < pso->face->num_glyphs; i++) {
			gf_pso_sprintf (pso, "/_%d %d def\n", i, i);
		}
		gf_pso_sprintf (pso, "end readonly def\n");
		gf_pso_sprintf (pso, "FontName currentdict end definefont pop\n");
	} else {
		guchar *afm;
		/* This is somewhat experimental - convert TTF to Type1 */
		afm = ttf2pfa (pso->face->ft_face, embeddedname, pso->glyphs);
		if (!afm) {
			munmap (fbuf, st.st_size);
			g_warning ("file %s: line %d: Cannot convert TTF %s to Type1", __FILE__, __LINE__, tt->ttf.name);
			gff_pso_ensure_buffer_empty (pso);
			return;
		}
		/* We take over ownership of afm here (be careful) */
		pso->buf = afm;
		pso->bufsize = strlen (afm);
		pso->length = pso->bufsize;
	}

	/* Unmap font file */
	munmap (fbuf, st.st_size);

	/*
	 * We have font downloaded (hopefully)
	 *
	 * With CharStrings: .nodef _1 _2 _3
	 *
	 * Now we have to build usable 2-byte encoded font from it
	 *
	 */

	/* fixme: That is crap. We should use CID */
	/* Bitch 'o' bitches (Lauris) ! */
	if (pso->face->num_glyphs < 256) {
		gint glyph;
		/* 8-bit vector */
		pso->encodedbytes = 1;
		gf_pso_sprintf (pso, "/%s findfont dup length dict begin\n", embeddedname);
		gf_pso_sprintf (pso, "{1 index /FID ne {def} {pop pop} ifelse} forall\n");
		gf_pso_sprintf (pso, "/Encoding [\n");
		for (glyph = 0; glyph < 256; glyph++) {
			guint g;
			g = (glyph < pso->face->num_glyphs) ? glyph : 0;
			if ((g == 0) || !PSO_GLYPH_MARKED (pso, glyph)) {
				gf_pso_sprintf (pso, ((glyph & 0xf) == 0xf) ? "/.notdef\n" : "/.notdef ", g);
			} else {
				gf_pso_sprintf (pso, ((glyph & 0xf) == 0xf) ? "/_%d\n" : "/_%d ", g);
			}
		}
		gf_pso_sprintf (pso, "] def currentdict end\n");
		gf_pso_sprintf (pso, "/%s exch definefont pop\n", pso->encodedname);
	} else {
		gint nfonts, nglyphs, i, j;
		/* 16-bit vector */
		pso->encodedbytes = 2;
		nglyphs = pso->face->num_glyphs;
		nfonts = (nglyphs + 255) >> 8;

		gf_pso_sprintf (pso, "32 dict begin\n");
		/* Common entries */
		gf_pso_sprintf (pso, "/FontType 0 def\n");
		gf_pso_sprintf (pso, "/FontMatrix [1 0 0 1 0 0] def\n");
		gf_pso_sprintf (pso, "/FontName /%s-Glyph-Composite def\n", embeddedname);
		gf_pso_sprintf (pso, "/LanguageLevel 2 def\n");
		/* Type 0 entries */
		gf_pso_sprintf (pso, "/FMapType 2 def\n");
		/* Bitch 'o' bitches */
		gf_pso_sprintf (pso, "/FDepVector [\n");
		for (i = 0; i < nfonts; i++) {
			gf_pso_sprintf (pso, "/%s findfont dup length dict begin\n", embeddedname);
			gf_pso_sprintf (pso, "{1 index /FID ne {def} {pop pop} ifelse} forall\n");
			gf_pso_sprintf (pso, "/Encoding [\n");
			for (j = 0; j < 256; j++) {
				gint glyph;
				glyph = 256 * i + j;
				if (glyph >= nglyphs) glyph = 0;
				if ((glyph == 0) || !PSO_GLYPH_MARKED (pso, glyph)) {
					gf_pso_sprintf (pso, ((j & 0xf) == 0xf) ? "/.notdef\n" : "/.notdef ");
				} else {
					gf_pso_sprintf (pso, ((j & 0xf) == 0xf) ? "/_%d\n" : "/_%d ", glyph);
				}
			}
			gf_pso_sprintf (pso, "] def\n");
			gf_pso_sprintf (pso, "currentdict end /%s-Glyph-Page-%d exch definefont\n", embeddedname, i);
		}
		gf_pso_sprintf (pso, "] def\n");
		gf_pso_sprintf (pso, "/Encoding [\n");
		for (i = 0; i < 256; i++) {
			gint fn;
			fn = (i < nfonts) ? i : 0;
			gf_pso_sprintf (pso, ((i & 0xf) == 0xf) ? "%d\n" : "%d  ", fn);
		}
		gf_pso_sprintf (pso, "] def\n");
		gf_pso_sprintf (pso, "currentdict end\n");
		gf_pso_sprintf (pso, "/%s exch definefont pop\n", pso->encodedname);
	}
}

static void
gff_pso_ensure_buffer_empty (GFPSObject *pso)
{
	pso->length = 0;

	gf_pso_sprintf (pso, "%%Empty font generated by gnome-print\n");
	gf_pso_sprintf (pso, "8 dict begin /FontType 3 def /FontMatrix [.001 0 0 .001 0 0] def /FontBBox [0 0 750 950] def\n");
	gf_pso_sprintf (pso, "/Encoding 256 array def 0 1 255 {Encoding exch /.notdef put} for\n");
	gf_pso_sprintf (pso, "/CharProcs 2 dict def CharProcs begin /.notdef {\n");
	gf_pso_sprintf (pso, "0 0 moveto 750 0 lineto 750 950 lineto 0 950 lineto closepath\n");
	gf_pso_sprintf (pso, "50 50 moveto 700 50 lineto 700 900 lineto 50 900 lineto closepath\n");
	gf_pso_sprintf (pso, "eofill } bind def end\n");
	gf_pso_sprintf (pso, "/BuildGlyph {1000 0 0 0 750 950 setcachedevice exch /CharProcs get exch\n");
	gf_pso_sprintf (pso, "2 copy known not {pop /.notdef} if get exec } bind def\n");
	gf_pso_sprintf (pso, "/BuildChar {1 index /Encoding get exch get 1 index /BuildGlyph get exec } bind def\n");
	if (pso->encodedbytes == 1) {
		/* 8-bit empty font */
		gf_pso_sprintf (pso, "currentdict end /%s exch definefont pop\n", pso->encodedname);
	} else {
		/* 16-bit empty font */
		gf_pso_sprintf (pso, "currentdict end /%s-Base exch definefont pop\n", pso->encodedname);
		gf_pso_sprintf (pso, "32 dict begin /FontType 0 def /FontMatrix [1 0 0 1 0 0] def\n");
		gf_pso_sprintf (pso, "/FontName /%s-Glyph-Composite def\n", pso->encodedname);
		gf_pso_sprintf (pso, "/LanguageLevel 2 def /FMapType 2 def\n");
		gf_pso_sprintf (pso, "/FDepVector [/%s-Base findfont] def", pso->encodedname);
		gf_pso_sprintf (pso, "/Encoding 256 array def 0 1 255 {Encoding exch 0 put} for\n");
		gf_pso_sprintf (pso, "currentdict end /%s exch definefont pop\n", pso->encodedname);
	}
}

static void
gf_pso_sprintf (GFPSObject * pso, const gchar * format, ...)
{
	va_list arguments;
	gchar *oldlocale;
	gchar *text;
	gint len;
	
	oldlocale = setlocale (LC_NUMERIC, NULL);
	setlocale (LC_NUMERIC, "C");
		
	va_start (arguments, format);
	text = g_strdup_vprintf (format, arguments);
	va_end (arguments);

	len = strlen (text);
	gf_pso_ensure_space (pso, len);
	memcpy (pso->buf + pso->length, text, len);
	pso->length += len;
	g_free (text);

	setlocale (LC_NUMERIC, oldlocale);
}

static void
gf_pso_ensure_space (GFPSObject *pso, gint size)
{
	if (pso->length + size > pso->bufsize) {
		if (pso->bufsize < 1) {
			pso->bufsize = MAX (size, 1024);
			pso->buf = g_new (guchar, pso->bufsize);
		} else {
			while (pso->length + size > pso->bufsize) pso->bufsize <<= 1;
			pso->buf = g_renew (guchar, pso->buf, pso->bufsize);
		}
	}
}

gchar *
gnome_font_face_get_pfa (const GnomeFontUnsized *font)
{
#if 0
	GPFontEntryT1 * t1;
	const char *pfb_fn;
	FILE *f;
	char *pfb;
	int pfb_size, pfb_size_max;
	int bytes_read;

	char *flat = NULL;

	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (font), NULL);

	t1 = (GPFontEntryT1 *) font->private->entry;
	pfb_fn = t1->pfb.name;
	f = fopen (pfb_fn, "r");
	if (f != NULL) {
		pfb_size = 0;
		pfb_size_max = 32768;
		pfb = g_new (char, pfb_size_max);
		while (1) {
			bytes_read = fread (pfb + pfb_size, 1, pfb_size_max - pfb_size, f);
			if (bytes_read == 0) break;
			pfb_size += bytes_read;
			pfb = g_realloc (pfb, pfb_size_max <<= 1);
		}

		if (pfb_size) {
			if (((unsigned char *)pfb)[0] == 128) {
				flat = pfb_to_flat (pfb, pfb_size);
			} else {
				flat = g_new (char, pfb_size + 1);
				memcpy (flat, pfb, pfb_size);
				flat[pfb_size] = 0;
			}
		} else {
			flat = NULL;
		}
		g_free (pfb);
		fclose (f);
	} else {
		gchar *privname;
		/* Encode empty font */
		g_warning (_("Couldn't generate pfa for face %s\n"), pfb_fn);
		if (font->private->entry->type == GP_FONT_ENTRY_TYPE1_ALIAS) {
			privname = ((GPFontEntryT1Alias *) font->private->entry)->alias;
		} else {
			privname = font->private->entry->psname;
		}
		flat = g_strdup_printf ("%%Empty font generated by gnome-print\n"
					"8 dict begin"
					"/FontType 3 def\n"
					"/FontMatrix [.001 0 0 .001 0 0] def\n"
					"/FontBBox [0 0 750 950] def\n"
					"/Encoding 256 array def\n"
					"0 1 255 {Encoding exch /.notdef put} for\n"
					"/CharProcs 2 dict def\n"
					"CharProcs begin\n"
					"/.notdef {\n"
					"0 0 moveto 750 0 lineto 750 950 lineto 0 950 lineto closepath\n"
					"50 50 moveto 700 50 lineto 700 900 lineto 50 900 lineto closepath\n"
					"eofill\n"
					"} bind def\n"
					"end\n"
					"/BuildGlyph {\n"
					"1000 0 0 0 750 950 setcachedevice\n"
					"exch /CharProcs get exch\n"
					"2 copy known not {pop /.notdef} if\n"
					"get exec\n"
					"} bind def\n"
					"/BuildChar {1 index /Encoding get exch get\n"
					"1 index /BuildGlyph get exec\n"
					"} bind def\n"
					"currentdict\n"
					"end\n"
					"/%s exch definefont pop\n",
					privname);
	}

	return flat;
#else
	return NULL;
#endif
}


