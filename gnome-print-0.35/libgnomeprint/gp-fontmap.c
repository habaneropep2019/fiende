#define _GP_FONTMAP_C_

/*
 * Fontmap implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * TODO: Recycle font entries, if they are identical for different maps
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <gnome-xml/parser.h>
#include <gnome-xml/xmlmemory.h>
#include "gp-fontmap.h"

/*
 * We parse following locations for fontmaps:
 *
 * $FONTMAPDIR_STATIC ($prefix/share/gnome/fonts)
 * $FONTMAPDIR_DYNAMIC ($sysconfdir/gnome/fonts)
 * $HOME/.gnome/fonts
 *
 */

static GPFontMap *gp_fontmap_load (void);
static void gp_fontmap_load_dir (GPFontMap *map, const guchar *dirname);
static void gp_fontmap_load_file (GPFontMap *map, const guchar *filename);
static void gp_fm_load_font_2_0_type1 (GPFontMap *map, xmlNodePtr node);
static void gp_fm_load_font_2_0_type1alias (GPFontMap *map, xmlNodePtr node);
static void gp_fm_load_font_2_0_truetype (GPFontMap *map, xmlNodePtr node);
static void gp_font_entry_2_0_load_data (GPFontEntry *e, xmlNodePtr node);
static void gp_font_entry_2_0_type1_load_files (GPFontEntryT1 *t1, xmlNodePtr node);
static void gp_font_entry_2_0_truetype_load_files (GPFontEntryTT *tt, xmlNodePtr node);
static void gp_fm_load_fonts_2_0 (GPFontMap * map, xmlNodePtr root);
static void gp_fontmap_ensure_stdaliases (GPFontMap *map);

static void gp_fontmap_ref (GPFontMap * map);
static void gp_fontmap_unref (GPFontMap * map);
static void gp_family_entry_ref (GPFamilyEntry * entry);
static void gp_family_entry_unref (GPFamilyEntry * entry);

static gchar * gp_xmlGetPropString (xmlNodePtr node, const gchar * name);
static gint gp_fe_sortname (gconstpointer a, gconstpointer b);
static gint gp_fe_sortspecies (gconstpointer a, gconstpointer b);
static gint gp_familyentry_sortname (gconstpointer a, gconstpointer b);
static gchar * gp_fm_get_species_name (const gchar * fullname, const gchar * familyname);
static gboolean gp_fm_is_changed (GPFontMap * map);

/* Fontlist -> FontMap */
static GHashTable * fontlist2map = NULL;
/* Familylist -> FontMap */
static GHashTable * familylist2map = NULL;

GPFontMap *
gp_fontmap_get (void)
{
	static GPFontMap *map = NULL;
	static time_t lastaccess = 0;

	if (map) {
		/* If > 1 sec is passed from last query, check timestamps */
		if ((time (NULL) > lastaccess) && gp_fm_is_changed (map)) {
			/* Any directory is changed, so force rereading of map */
			gp_fontmap_release (map);
			map = NULL;
		}
	}

	if (!map) {
		map = gp_fontmap_load ();
	}

	/* Save acess time */
	lastaccess = time (NULL);

	map->refcount++;

	return map;
}

void
gp_fontmap_release (GPFontMap * map)
{
	gp_fontmap_unref (map);
}

static void
gp_fontmap_ref (GPFontMap * map)
{
	g_return_if_fail (map != NULL);

	map->refcount++;
}

static void
gp_fontmap_unref (GPFontMap * map)
{
	g_return_if_fail (map != NULL);

	if (--map->refcount < 1) {
		if (map->familydict) g_hash_table_destroy (map->familydict);
		if (map->fontdict) g_hash_table_destroy (map->fontdict);
		if (map->familylist) {
			g_hash_table_remove (familylist2map, map->familylist);
			g_list_free (map->familylist);
		}
		if (map->fontlist) {
			g_hash_table_remove (fontlist2map, map->fontlist);
			g_list_free (map->fontlist);
		}
		while (map->families) {
			gp_family_entry_unref ((GPFamilyEntry *) map->families->data);
			map->families = g_slist_remove (map->families, map->families->data);
		}
		while (map->fonts) {
			gp_font_entry_unref ((GPFontEntry *) map->fonts->data);
			map->fonts = g_slist_remove (map->fonts, map->fonts->data);
		}
		while (map->defaults) {
			GSList *l;
			l = map->defaults->data;
			map->defaults = g_slist_remove (map->defaults, l);
			while (l) {
				g_free (l->data);
				l = g_slist_remove (l, l->data);
			}
		}
		if (map->defaultsdict) g_hash_table_destroy (map->defaultsdict);
		g_free (map);
	}
}

static void
gp_family_entry_ref (GPFamilyEntry * entry)
{
	entry->refcount++;
}

static void
gp_family_entry_unref (GPFamilyEntry * entry)
{
	if (--entry->refcount < 1) {
		if (entry->name) g_free (entry->name);
		if (entry->fonts) g_slist_free (entry->fonts);
		g_free (entry);
	}
}

static GPFontMap *
gp_fontmap_load (void)
{
	GPFontMap *map;
	struct stat s;
	gchar *homedir, *mapdir;
	GSList * l;

	map = g_new (GPFontMap, 1);
	/* We always hold private ref to fontmap, this is released if directories change */
	map->refcount = 1;
	map->num_fonts = 0;
	/* Clear timestamps */
	map->mtime_static = 0;
	map->mtime_dynamic = 0;
	map->mtime_user = 0;
	map->fontdict = g_hash_table_new (g_str_hash, g_str_equal);
	map->familydict = g_hash_table_new (g_str_hash, g_str_equal);
	map->fonts = NULL;
	map->families = NULL;
	map->fontlist = NULL;
	map->familylist = NULL;
	map->defaults = NULL;
	map->defaultsdict = g_hash_table_new (g_str_hash, g_str_equal);

	/* User map */
	homedir = g_get_home_dir ();
	mapdir = g_concat_dir_and_file (homedir, ".gnome/fonts");
	if (!stat (mapdir, &s) && S_ISDIR (s.st_mode)) {
		map->mtime_user = s.st_mtime;
		gp_fontmap_load_dir (map, mapdir);
	}
	g_free (mapdir);
	/* Dynamic map */
	if (!stat (FONTMAPDIR_DYNAMIC, &s) && S_ISDIR (s.st_mode)) {
		map->mtime_dynamic = s.st_mtime;
		gp_fontmap_load_dir (map, FONTMAPDIR_DYNAMIC);
	}
	/* Static map */
	if (!stat (FONTMAPDIR_STATIC, &s) && S_ISDIR (s.st_mode)) {
		map->mtime_static = s.st_mtime;
		gp_fontmap_load_dir (map, FONTMAPDIR_STATIC);
	}

	/* Sanity check */
	if (map->num_fonts < 24) {
		/* Less than 24 fonts means, you do not have PS ones */
		if (!stat (DATADIR "/fonts/fontmap2", &s) && S_ISREG (s.st_mode)) {
			gp_fontmap_load_file (map, DATADIR "/fonts/fontmap2");
		}
	}
	/* More sanity check */
	if (map->num_fonts < 24) {
		gchar *filename;
		/* Less than 24 fonts means, you do not have PS ones */
		filename = g_concat_dir_and_file (g_get_home_dir (), ".gnome/fonts/fontmap");
		if (!stat (filename, &s) && S_ISREG (s.st_mode)) {
			gp_fontmap_load_file (map, filename);
		}
		g_free (filename);
	}

	/* Still more sanity check */
	gp_fontmap_ensure_stdaliases (map);

	/* Sort fonts alphabetically */
	map->fonts = g_slist_sort (map->fonts, gp_fe_sortname);

	/* Sort fonts into familia */
	for (l = map->fonts; l != NULL; l = l->next) {
		GPFontEntry * e;
		GPFamilyEntry * f;
		e = (GPFontEntry *) l->data;
		f = g_hash_table_lookup (map->familydict, e->familyname);
		if (!f) {
			f = g_new0 (GPFamilyEntry, 1);
			gp_family_entry_ref (f);
			f->name = g_strdup (e->familyname);
			f->fonts = g_slist_prepend (f->fonts, e);
			g_hash_table_insert (map->familydict, f->name, f);
			map->families = g_slist_prepend (map->families, f);
		} else {
			f->fonts = g_slist_prepend (f->fonts, e);
		}
	}

	/* Sort familia alphabetically */
	map->families = g_slist_sort (map->families, gp_familyentry_sortname);

	/* Sort fonts inside familia */
	for (l = map->families; l != NULL; l = l->next) {
		GPFamilyEntry * f;
		f = (GPFamilyEntry *) l->data;
		f->fonts = g_slist_sort (f->fonts, gp_fe_sortspecies);
	}

	/* Compose defaultsdict */
	map->defaults = g_slist_reverse (map->defaults);
	while (map->defaults) {
		GSList *l;
		guchar *locales, *fontname;
		GPFontEntry *entry;
		l = map->defaults->data;
		map->defaults = g_slist_remove (map->defaults, l);
		locales = l->data;
		fontname = l->next->data;
		g_slist_free (l);
		entry = g_hash_table_lookup (map->fontdict, fontname);
		if (!entry) {
			GPFamilyEntry *fe;
			/* Try family */
			fe = g_hash_table_lookup (map->familydict, fontname);
			if (fe && fe->fonts) {
				entry = (GPFontEntry *) fe->fonts->data;
				l = fe->fonts;
				while (l) {
					GPFontEntry *e;
					e = (GPFontEntry *) l->data;
					if (!strcasecmp (e->speciesname, "regular") ||
					    !strcasecmp (e->speciesname, "roman") ||
					    !strcasecmp (e->speciesname, "normal")) {
						entry = e;
						break;
					}
					l = l->next;
				}
			}
		}
		if (entry) {
			guchar *l;
			l = locales;
			while (l) {
				guchar *e;
				e = strchr (l, ',');
				if (e) {
					*e = '\0';
					e += 1;
				}
				if (!g_hash_table_lookup (map->defaultsdict, l)) {
					GQuark q;
					q = g_quark_from_string (l);
					g_hash_table_insert (map->defaultsdict, g_quark_to_string (q), entry);
				}
				l = e;
			}
		}
		g_free (locales);
		g_free (fontname);
	}

	return map;
}

static gint
gp_fontmap_compare_names (gconstpointer a, gconstpointer b)
{
	if (!strcmp (a, "gnome-print.fontmap")) return -1;
	if (!strcmp (b, "gnome-print.fontmap")) return 1;
	return strcmp (a, b);
}

/* Tries to load all *.fontmap files from given directory */

static void
gp_fontmap_load_dir (GPFontMap *map, const guchar *dirname)
{
	DIR *dir;
	struct dirent *dent;

	dir = opendir (dirname);

	if (dir) {
		GSList *files;
		files = NULL;
		while ((dent = readdir (dir))) {
			gint len;
			len = strlen (dent->d_name);
			if ((len > 8) && !strcmp (dent->d_name + len - 8, ".fontmap")) {
				/* Seems to be what we are looking for */
				files = g_slist_prepend (files, g_strdup (dent->d_name));
			}
		}
		closedir (dir);
		/* Sort names alphabetically */
		files = g_slist_sort (files, gp_fontmap_compare_names);
		while (files) {
			struct stat s;
			gchar *filename;
			filename = g_concat_dir_and_file (dirname, (gchar *) files->data);
			g_free (files->data);
			if (!stat (filename, &s) && S_ISREG (s.st_mode)) {
				gp_fontmap_load_file (map, filename);
			}
			g_free (filename);
			files = g_slist_remove (files, files->data);
		}
	}
}

/* Parse file, and add fonts to map, if it is valid fontmap */

static void
gp_fontmap_load_file (GPFontMap *map, const guchar *filename)
{
	xmlDocPtr doc;

	doc = xmlParseFile (filename);
	if (doc) {
		xmlNodePtr root;
		/* In minimum we are valid xml file */
		root = xmlDocGetRootElement (doc);
		if (root && !strcmp (root->name, "fontmap")) {
			xmlChar *version;
			/* We are really fontmap */
			version = xmlGetProp (root, "version");
			if (version) {
				if (!strcmp (version, "2.0")) {
					xmlChar *test;
					gboolean try;
					/* We are even right version */
					try = TRUE;
					test = xmlGetProp (root, "test");
					if (test) {
						struct stat s;
						if (stat (test, &s) || !S_ISREG (s.st_mode)) try = FALSE;
						xmlFree (test);
					}
					if (try) gp_fm_load_fonts_2_0 (map, root);
				}
				xmlFree (version);
			}
		}
		xmlFreeDoc (doc);
	}
}

/* Parse root element and build fontmap step 1 */

static void
gp_fm_load_fonts_2_0 (GPFontMap * map, xmlNodePtr root)
{
	xmlNodePtr child;

	for (child = root->xmlChildrenNode; child != NULL; child = child->next) {
		if (!strcmp (child->name, "font")) {
			xmlChar *format;
			/* We are font */
			format = xmlGetProp (child, "format");
			if (format) {
				if (!strcmp (format, "type1")) {
					/* We are type1/type1alias entry */
					gp_fm_load_font_2_0_type1 (map, child);
				} else if (!strcmp (format, "type1alias")) {
					/* We are type1/type1alias entry */
					gp_fm_load_font_2_0_type1alias (map, child);
				} else if (!strcmp (format, "truetype")) {
					/* We are truetype entry */
					gp_fm_load_font_2_0_truetype (map, child);
				}
				xmlFree (format);
			}
		} else if (!strcmp (child->name, "default")) {
			xmlChar *font;
			font = xmlGetProp (child, "font");
			if (font) {
				xmlChar *locales;
				guchar *loc;
				GSList *l;
				locales = xmlGetProp (child, "locales");
				loc = (locales) ? g_strdup (locales) : g_strdup ("C");
				/* fixme: This is not nice (Lauris) */
				l = g_slist_prepend (NULL, g_strdup (font));
				l = g_slist_prepend (l, loc);
				map->defaults = g_slist_prepend (map->defaults, l);
				if (locales) xmlFree (locales);
				xmlFree (font);
			}
		}
	}
}

static void
gp_fm_load_font_2_0_type1 (GPFontMap *map, xmlNodePtr node)
{
	GPFontEntryT1 *t1;
	GPFontEntry *e;
	xmlChar *xmlname, *t;

	/* Get our unique name */
	xmlname = xmlGetProp (node, "name");
	/* Check, whether we are already registered */
	if (g_hash_table_lookup (map->fontdict, xmlname)) {
		xmlFree (xmlname);
		return;
	}

	t1 = g_new0 (GPFontEntryT1, 1);
	e = (GPFontEntry *) t1;

	e->type = GP_FONT_ENTRY_TYPE1;
	e->refcount = 1;
	e->face = NULL;
	e->name = g_strdup (xmlname);
	xmlFree (xmlname);

	gp_font_entry_2_0_load_data (e, node);
	gp_font_entry_2_0_type1_load_files (t1, node);
	if (!e->familyname || !e->psname || !t1->pfb.name) {
		gp_font_entry_unref (e);
		return;
	}

	e->Weight = gp_fontmap_lookup_weight (e->weight);

	if (!e->speciesname) {
		e->speciesname = gp_fm_get_species_name (e->name, e->familyname);
	}

	t = xmlGetProp (node, "italicangle");
	if (t == NULL) {
		gchar *p;
		p = strstr (e->speciesname, "Italic");
		if (!p) p = strstr (e->speciesname, "Oblique");
		e->ItalicAngle = p ? -10.0 : 0.0;
	} else {
		e->ItalicAngle = atof (t);
		xmlFree (t);
	}

	g_hash_table_insert (map->fontdict, e->name, e);
	map->num_fonts++;
	map->fonts = g_slist_prepend (map->fonts, e);
}

static void
gp_fm_load_font_2_0_type1alias (GPFontMap *map, xmlNodePtr node)
{
	GPFontEntryT1Alias *t1a;
	GPFontEntryT1 *t1;
	GPFontEntry *e;
	xmlChar *xmlname, *xmlalias, *t;

	/* Get our unique name */
	xmlname = xmlGetProp (node, "name");
	/* Check, whether we are already registered */
	if (g_hash_table_lookup (map->fontdict, xmlname)) {
		xmlFree (xmlname);
		return;
	}
	/* Get our alternate PS name */
	xmlalias = xmlGetProp (node, "alias");
	if (!xmlalias) {
		xmlFree (xmlname);
		return;
	}

	t1a = g_new0 (GPFontEntryT1Alias, 1);
	t1 = (GPFontEntryT1 *) t1a;
	e = (GPFontEntry *) t1a;

	e->type = GP_FONT_ENTRY_TYPE1_ALIAS;
	e->refcount = 1;
	e->face = NULL;
	e->name = g_strdup (xmlname);
	xmlFree (xmlname);
	t1a->alias = g_strdup (xmlalias);
	xmlFree (xmlalias);

	gp_font_entry_2_0_load_data (e, node);
	gp_font_entry_2_0_type1_load_files (t1, node);
	if (!e->familyname || !e->psname || !t1->pfb.name) {
		gp_font_entry_unref (e);
		return;
	}

	e->Weight = gp_fontmap_lookup_weight (e->weight);

	if (!e->speciesname) {
		e->speciesname = gp_fm_get_species_name (e->name, e->familyname);
	}

	t = xmlGetProp (node, "italicangle");
	if (t == NULL) {
		gchar *p;
		p = strstr (e->speciesname, "Italic");
		if (!p) p = strstr (e->speciesname, "Oblique");
		e->ItalicAngle = p ? -10.0 : 0.0;
	} else {
		e->ItalicAngle = atof (t);
		xmlFree (t);
	}

	g_hash_table_insert (map->fontdict, e->name, e);
	map->num_fonts++;
	map->fonts = g_slist_prepend (map->fonts, e);
}

static void
gp_fm_load_font_2_0_truetype (GPFontMap *map, xmlNodePtr node)
{
	GPFontEntryTT *tt;
	GPFontEntry *e;
	xmlChar *xmlname, *t;

	/* Get our unique name */
	xmlname = xmlGetProp (node, "name");
	/* Check, whether we are already registered */
	if (g_hash_table_lookup (map->fontdict, xmlname)) {
		xmlFree (xmlname);
		return;
	}

	tt = g_new0 (GPFontEntryTT, 1);
	e = (GPFontEntry *) tt;

	e->type = GP_FONT_ENTRY_TRUETYPE;
	e->refcount = 1;
	e->face = NULL;
	e->name = g_strdup (xmlname);
	xmlFree (xmlname);

	gp_font_entry_2_0_load_data (e, node);
	gp_font_entry_2_0_truetype_load_files (tt, node);
	if (!e->familyname || !e->psname || !tt->ttf.name) {
		gp_font_entry_unref (e);
		return;
	}

	e->Weight = gp_fontmap_lookup_weight (e->weight);

	if (!e->speciesname) {
		e->speciesname = gp_fm_get_species_name (e->name, e->familyname);
	}

	t = xmlGetProp (node, "italicangle");
	if (t == NULL) {
		gchar *p;
		p = strstr (e->speciesname, "Italic");
		if (!p) p = strstr (e->speciesname, "Oblique");
		e->ItalicAngle = p ? -10.0 : 0.0;
	} else {
		e->ItalicAngle = atof (t);
		xmlFree (t);
	}

	t = xmlGetProp (node, "subface");
	tt->facenum = (t) ? atoi (t) : 0;
	if (t) xmlFree (t);

	g_hash_table_insert (map->fontdict, e->name, e);
	map->num_fonts++;
	map->fonts = g_slist_prepend (map->fonts, e);
}

/* Loads common font property data */

static void
gp_font_entry_2_0_load_data (GPFontEntry *e, xmlNodePtr node)
{
	/* fixme: We could do some checking here to save alloc/free calls */
	/* name is parsed by parent */
	e->version = gp_xmlGetPropString (node, "version");
	e->familyname = gp_xmlGetPropString (node, "familyname");
	e->speciesname = gp_xmlGetPropString (node, "speciesname");
	e->psname = gp_xmlGetPropString (node, "psname");
	/* Read Weight attribute */
	e->weight = gp_xmlGetPropString (node, "weight");
	if (!e->weight) e->weight = g_strdup ("Book");
}

/* Loads "afm" and "pfb" file nodes */

static void
gp_font_entry_2_0_type1_load_files (GPFontEntryT1 *t1, xmlNodePtr node)
{
	xmlNodePtr child;

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		/* Scan all children nodes */
		if (!strcmp (child->name, "file")) {
			xmlChar *type;
			/* We are <file> node */
			type = xmlGetProp (child, "type");
			if (type && !strcmp (type, "afm") && !t1->afm.name) {
				t1->afm.name = gp_xmlGetPropString (child, "path");
			} else if (type && !strcmp (type, "pfb") && !t1->pfb.name) {
				t1->pfb.name = gp_xmlGetPropString (child, "path");
			}
			if (type) xmlFree (type);
		}
		if (t1->afm.name && t1->pfb.name) return;
	}
}

/* Loads "afm" and "pfb" file nodes */

static void
gp_font_entry_2_0_truetype_load_files (GPFontEntryTT *tt, xmlNodePtr node)
{
	xmlNodePtr child;

	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		/* Scan all children nodes */
		if (!strcmp (child->name, "file")) {
			xmlChar *type;
			/* We are <file> node */
			type = xmlGetProp (child, "type");
			if (type && !strcmp (type, "ttf") && !tt->ttf.name) {
				tt->ttf.name = gp_xmlGetPropString (child, "path");
			}
			if (type) xmlFree (type);
		}
		if (tt->ttf.name) return;
	}
}

/* This is experimental method (not public anyways) (Lauris) */

GPFontEntry *gp_font_entry_from_files (GPFontMap *map,
				       const guchar *name, const guchar *family, const guchar *species, gboolean hidden,
				       const guchar *filename, gint face, const GSList *additional)
{
	GPFontEntrySpecial *s;
	const GSList *l;
	gchar *p;

	g_return_val_if_fail (map != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (family != NULL, NULL);
	g_return_val_if_fail (species != NULL, NULL);
	g_return_val_if_fail (filename != NULL, NULL);

	if (!hidden && g_hash_table_lookup (map->fontdict, name)) {
		g_warning ("file %s: line %d: Font with name %s already exists", __FILE__, __LINE__, name);
	}

	s = g_new0 (GPFontEntrySpecial, 1);
	s->entry.type = GP_FONT_ENTRY_SPECIAL;
	s->entry.refcount = 1;
	s->entry.face = NULL;
	s->entry.name = g_strdup (name);

	s->entry.version = g_strdup ("0.0");
	s->entry.familyname = g_strdup (family);
	s->entry.speciesname = g_strdup (species);
	s->entry.psname = g_strdup ("Unnamed");

	s->entry.weight = g_strdup ("Book");

	s->file.name = g_strdup (filename);

	for (l = additional; l != NULL; l = l->next) {
		s->additional = g_slist_prepend (s->additional, g_strdup (l->data));
	}
	s->additional = g_slist_reverse (s->additional);

	s->entry.Weight = gp_fontmap_lookup_weight (s->entry.weight);
	p = strstr (s->entry.speciesname, "Italic");
	if (!p) p = strstr (s->entry.speciesname, "Oblique");
	s->entry.ItalicAngle = p ? -10.0 : 0.0;

	s->subface = face;

	return &s->entry;
}

/*
 * This is workaround for large number of installer bugs
 *
 * We just force aliased entries for well-known substitutes of standard
 * PS fonts
 *
 */

typedef struct {
	guchar *name;
	guchar *familyname;
	guchar *speciesname;
	guchar *psname;
	guchar *ref;
} GPAliasData;

static const GPAliasData aliasdata[] = {
	{"Helvetica", "Helvetica", "Normal", "Helvetica", "Nimbus Sans L Regular"},
	{"Helvetica Bold", "Helvetica", "Bold", "Helvetica-Bold", "Nimbus Sans L Bold"},
	{"Helvetica Oblique", "Helvetica", "Oblique", "Helvetica-Oblique", "Nimbus Sans L Regular Italic"},
	{"Helvetica Bold Oblique", "Helvetica", "Bold Oblique", "Helvetica-BoldOblique", "Nimbus Sans L Bold Italic"},
	{"Times Roman", "Times", "Roman", "Times-Roman", "Nimbus Roman No9 L Regular"},
	{"Times Bold", "Times", "Bold", "Times-Bold", "Nimbus Roman No9 L Medium"},
	{"Times Italic", "Times", "Italic", "Times-Italic", "Nimbus Roman No9 L Regular Italic"},
	{"Times Bold Italic", "Times", "Bold Italic", "Times-BoldItalic", "Nimbus Roman No9 L Medium Italic"},
	{"Courier", "Courier", "Normal", "Courier", "Nimbus Mono L Regular"},
	{"Courier Bold", "Courier", "Bold", "Courier-Bold", "Nimbus Mono L Bold"},
	{"Courier Oblique", "Courier", "Oblique", "Courier-Oblique", "Nimbus Mono L Regular Oblique"},
	{"Courier Bold Oblique", "Courier", "Bold Oblique", "Courier-BoldOblique", "Nimbus Mono L Bold Oblique"},
	{NULL}
};

static void
gp_fontmap_ensure_stdaliases (GPFontMap *map)
{
	gint i;

	for (i = 0; aliasdata[i].name; i++) {
		if (!g_hash_table_lookup (map->fontdict, aliasdata[i].name) && g_hash_table_lookup (map->fontdict, aliasdata[i].ref)) {
			GPFontEntry *ref;
			GPFontEntryAlias *ea;
			/* Build entry */
			ref = g_hash_table_lookup (map->fontdict, aliasdata[i].ref);
			ea = g_new0 (GPFontEntryAlias, 1);
			ea->entry.type = GP_FONT_ENTRY_ALIAS;
			ea->entry.refcount = 1;
			ea->entry.face = NULL;
			ea->entry.name = g_strdup (aliasdata[i].name);

			ea->entry.version = g_strdup (ref->version);
			ea->entry.familyname = g_strdup (aliasdata[i].familyname);
			ea->entry.speciesname = g_strdup (aliasdata[i].speciesname);
			ea->entry.psname = g_strdup (aliasdata[i].psname);

			ea->entry.weight = g_strdup (ref->weight);
			ea->entry.ItalicAngle = ref->ItalicAngle;

			ea->ref = ref;
			gp_font_entry_ref (ref);

			g_hash_table_insert (map->fontdict, ea->entry.name, ea);
			map->num_fonts++;
			map->fonts = g_slist_prepend (map->fonts, ea);
		}
	}
}

/*
 * Font Entry stuff
 *
 * If face is created, it has to reference entry
 */

void
gp_font_entry_ref (GPFontEntry * entry)
{
	g_return_if_fail (entry != NULL);
	/* refcount can be 1 or 2 at moment */
	g_return_if_fail (entry->refcount > 0);
	g_return_if_fail (entry->refcount < 2);

	entry->refcount++;
}

void
gp_font_entry_unref (GPFontEntry * entry)
{
	g_return_if_fail (entry != NULL);
	/* refcount can be 1 or 2 at moment */
	g_return_if_fail (entry->refcount > 0);
	g_return_if_fail (entry->refcount < 3);

	if (--entry->refcount < 1) {
		GPFontEntryT1 *t1;
		GPFontEntryT1Alias *t1a;
		GPFontEntrySpecial *s;

		g_return_if_fail (entry->face == NULL);

		if (entry->name) g_free (entry->name);
		if (entry->version) g_free (entry->version);
		if (entry->familyname) g_free (entry->familyname);
		if (entry->speciesname) g_free (entry->speciesname);
		if (entry->psname) g_free (entry->psname);
		if (entry->weight) g_free (entry->weight);

		switch (entry->type) {
		case GP_FONT_ENTRY_TYPE1_ALIAS:
			t1a = (GPFontEntryT1Alias *) entry;
			if (t1a->alias) g_free (t1a->alias);
		case GP_FONT_ENTRY_TYPE1:
			t1 = (GPFontEntryT1 *) entry;
			if (t1->afm.name) g_free (t1->afm.name);
			if (t1->pfb.name) g_free (t1->pfb.name);
			break;
		case GP_FONT_ENTRY_ALIAS:
			gp_font_entry_unref (((GPFontEntryAlias *) entry)->ref);
			break;
		case GP_FONT_ENTRY_SPECIAL:
			s = (GPFontEntrySpecial *) entry;
			if (s->file.name) g_free (s->file.name);
			while (s->additional) {
				g_free (s->additional->data);
				s->additional = g_slist_remove (s->additional, s->additional->data);
			}
			break;
		default:
			g_assert_not_reached ();
			break;
		}
		g_free (entry);
	}
}

/*
 * Font list stuff
 *
 * We use Hack'O'Hacks here:
 * Getting list saves list->fontmap mapping and refs fontmap
 * Freeing list releases mapping and frees fontmap
 */

GList *
gnome_font_list ()
{
	GPFontMap * map;
	GSList * l;

	map = gp_fontmap_get ();

	if (!map->fontlist) {
		for (l = map->fonts; l != NULL; l = l->next) {
			GPFontEntry * e;
			e = (GPFontEntry *) l->data;
			map->fontlist = g_list_prepend (map->fontlist, e->name);
		}
		map->fontlist = g_list_reverse (map->fontlist);
		if (!fontlist2map) fontlist2map = g_hash_table_new (NULL, NULL);
		g_hash_table_insert (fontlist2map, map->fontlist, map);
	}

	return map->fontlist;
}

void
gnome_font_list_free (GList * fontlist)
{
	GPFontMap * map;

	g_return_if_fail (fontlist != NULL);

	map = g_hash_table_lookup (fontlist2map, fontlist);
	g_return_if_fail (map != NULL);

	gp_fontmap_unref (map);
}

GList *
gnome_font_family_list ()
{
	GPFontMap * map;
	GSList * l;

	map = gp_fontmap_get ();

	if (!map->familylist) {
		for (l = map->families; l != NULL; l = l->next) {
			GPFamilyEntry * f;
			f = (GPFamilyEntry *) l->data;
			map->familylist = g_list_prepend (map->familylist, f->name);
		}
		map->familylist = g_list_reverse (map->familylist);
		if (!familylist2map) familylist2map = g_hash_table_new (NULL, NULL);
		g_hash_table_insert (familylist2map, map->familylist, map);
	}

	gp_fontmap_ref (map);

	gp_fontmap_release (map);

	return map->familylist;
}

void
gnome_font_family_list_free (GList * fontlist)
{
	GPFontMap * map;

	g_return_if_fail (fontlist != NULL);

	map = g_hash_table_lookup (familylist2map, fontlist);
	g_return_if_fail (map != NULL);

	gp_fontmap_unref (map);
}

/*
 * Returns newly allocated string or NULL
 */

static gchar *
gp_xmlGetPropString (xmlNodePtr node, const gchar * name)
{
	xmlChar * prop;
	gchar * str;

	prop = xmlGetProp (node, name);
	if (prop) {
		str = g_strdup (prop);
		xmlFree (prop);
		return str;
	}

	return NULL;
}

static gint
gp_fe_sortname (gconstpointer a, gconstpointer b)
{
	return strcasecmp (((GPFontEntry *) a)->name, ((GPFontEntry *) b)->name);
}

static gint
gp_fe_sortspecies (gconstpointer a, gconstpointer b)
{
	return strcasecmp (((GPFontEntry *) a)->speciesname, ((GPFontEntry *) b)->speciesname);
}

static gint
gp_familyentry_sortname (gconstpointer a, gconstpointer b)
{
	return strcasecmp (((GPFamilyEntry *) a)->name, ((GPFamilyEntry *) b)->name);
}

static gchar *
gp_fm_get_species_name (const gchar * fullname, const gchar * familyname)
{
	gchar * p;

	p = strstr (fullname, familyname);

	if (!p) return g_strdup ("Normal");

	p = p + strlen (familyname);

	while (*p && (*p < 'A')) p++;

	if (!*p) return g_strdup ("Normal");

	return g_strdup (p);
}

GnomeFontWeight
gp_fontmap_lookup_weight (const gchar * weight)
{
	static GHashTable * weights = NULL;
	GnomeFontWeight wcode;

	if (!weights) {
		weights = g_hash_table_new (g_str_hash, g_str_equal);

		g_hash_table_insert (weights, "Extra Light", GINT_TO_POINTER (GNOME_FONT_EXTRA_LIGHT));
		g_hash_table_insert (weights, "Extralight", GINT_TO_POINTER (GNOME_FONT_EXTRA_LIGHT));

		g_hash_table_insert (weights, "Thin", GINT_TO_POINTER (GNOME_FONT_THIN));

		g_hash_table_insert (weights, "Light", GINT_TO_POINTER (GNOME_FONT_LIGHT));

		g_hash_table_insert (weights, "Book", GINT_TO_POINTER (GNOME_FONT_BOOK));
		g_hash_table_insert (weights, "Roman", GINT_TO_POINTER (GNOME_FONT_BOOK));
		g_hash_table_insert (weights, "Regular", GINT_TO_POINTER (GNOME_FONT_BOOK));

		g_hash_table_insert (weights, "Medium", GINT_TO_POINTER (GNOME_FONT_MEDIUM));

		g_hash_table_insert (weights, "Semi", GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Semibold", GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Demi", GINT_TO_POINTER (GNOME_FONT_SEMI));
		g_hash_table_insert (weights, "Demibold", GINT_TO_POINTER (GNOME_FONT_SEMI));

		g_hash_table_insert (weights, "Bold", GINT_TO_POINTER (GNOME_FONT_BOLD));

		g_hash_table_insert (weights, "Heavy", GINT_TO_POINTER (GNOME_FONT_HEAVY));
 
		g_hash_table_insert (weights, "Extra", GINT_TO_POINTER (GNOME_FONT_EXTRABOLD));
		g_hash_table_insert (weights, "Extra Bold", GINT_TO_POINTER (GNOME_FONT_EXTRABOLD));

		g_hash_table_insert (weights, "Black", GINT_TO_POINTER (GNOME_FONT_BLACK));

		g_hash_table_insert (weights, "Extra Black", GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
		g_hash_table_insert (weights, "Extrablack", GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
		g_hash_table_insert (weights, "Ultra Bold", GINT_TO_POINTER (GNOME_FONT_EXTRABLACK));
	};

	wcode = GPOINTER_TO_INT (g_hash_table_lookup (weights, weight));

	return wcode;
}

/*
 * This is not correct, if you only edit some file,
 * but I do not want to keep timestamps for all fontmaps
 * files. So please touch directory, after editing
 * files manually.
 */

static gboolean
gp_fm_is_changed (GPFontMap * map)
{
	struct stat s;
	gchar *homedir, *userdir;

	homedir = g_get_home_dir ();
	if (homedir) {
		userdir = g_concat_dir_and_file (homedir, ".gnome/fonts");
		if (!stat (userdir, &s) && !S_ISDIR (s.st_mode)) {
			/* User dir does not exist */
			g_free (userdir);
			if (s.st_mtime != map->mtime_user) return TRUE;
		} else {
			g_free (userdir);
		}
	}

	if (!stat (FONTMAPDIR_DYNAMIC, &s) && S_ISDIR (s.st_mode)) {
		/* Dynamic dir exists */
		if (s.st_mtime != map->mtime_dynamic) return TRUE;
	}

	if (!stat (FONTMAPDIR_STATIC, &s) && S_ISDIR (s.st_mode)) {
		/* Static dir exists */
		if (s.st_mtime != map->mtime_static) return TRUE;
	}

	return FALSE;
}
