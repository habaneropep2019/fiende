#define __GF_FONTMAP_C__

/*
 * Fontmap implementation for gnome-font-install
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * TODO: Recycle font entries, if they are identical for different maps
 *
 */

#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <gnome-xml/parser.h>
#include <gnome-xml/xmlmemory.h>
#include "gf-fontmap.h"

/*
 * We parse following locations for fontmaps:
 *
 * $FONTMAPDIR_STATIC ($prefix/share/gnome/fonts)
 * $FONTMAPDIR_DYNAMIC ($sysconfdir/gnome/fonts)
 * $HOME/.gnome/fonts
 *
 */

static void gf_fontmap_load_dir (GFFontDB *db, const guchar *dirname, GFFontMapType type);
static GFFontMap *gf_fontmap_load_file (const guchar *filename, GFFontMapType type);
static void gf_fm_load_fonts_2_0 (GFFontMap *map, xmlNodePtr root);
static void gf_fm_load_font_2_0_type1 (GFFontMap *map, xmlNodePtr node);
static void gf_fm_load_font_2_0_truetype (GFFontMap *map, xmlNodePtr node);
static void gf_fm_load_common_data (GFFontEntry *e, xmlNodePtr node);

static guchar *gf_fm_get_species_name (const guchar *fullname, const guchar *familyname);

GFFontDB *
gf_font_db_new (void)
{
	GFFontDB *db;
	GFFontMap *map;
	gchar *homedir, *path;

	db = g_new (GFFontDB, 1);

	homedir = g_get_home_dir ();
	path = g_concat_dir_and_file (homedir, ".gnome/fonts/gnome-print.fontmap");
	map = gf_fontmap_new (GF_FONTMAP_USER, path);
	db->usermaps = map;
	g_free (path);

	path = g_concat_dir_and_file (FONTMAPDIR_DYNAMIC, "gnome-print.fontmap");
	map = gf_fontmap_new (GF_FONTMAP_DYNAMIC, path);
	db->dynamicmaps = map;
	g_free (path);

	path = g_concat_dir_and_file (FONTMAPDIR_STATIC, "gnome-print.fontmap");
	map = gf_fontmap_new (GF_FONTMAP_STATIC, path);
	db->staticmaps = map;
	g_free (path);

	return db;
}

GFFontDB *
gf_font_db_load (void)
{
	GFFontDB *db;
	GFFontMap *map;
	struct stat s;
	gchar *homedir, *mapdir, *path;

	db = g_new (GFFontDB, 1);
	db->usermaps = NULL;
	db->dynamicmaps = NULL;
	db->staticmaps = NULL;

	/* User maps */
	homedir = g_get_home_dir ();
	mapdir = g_concat_dir_and_file (homedir, ".gnome/fonts");
	if (!stat (mapdir, &s) && S_ISDIR (s.st_mode)) {
		gf_fontmap_load_dir (db, mapdir, GF_FONTMAP_USER);
	}
	g_free (mapdir);
	/* Check for usermap */
	if (!db->usermaps || (db->usermaps->type != GF_FONTMAP_USER)) {
		path = g_concat_dir_and_file (homedir, ".gnome/fonts/gnome-print.fontmap");
		map = gf_fontmap_new (GF_FONTMAP_USER, path);
		map->next = db->usermaps;
		db->usermaps = map;
		g_free (path);
	}

	/* Dynamic map */
	if (!stat (FONTMAPDIR_DYNAMIC, &s) && S_ISDIR (s.st_mode)) {
		gf_fontmap_load_dir (db, FONTMAPDIR_DYNAMIC, GF_FONTMAP_DYNAMIC);
	}
	/* Check for dynamicmap */
	if (!db->dynamicmaps || (db->dynamicmaps->type != GF_FONTMAP_DYNAMIC)) {
		path = g_concat_dir_and_file (FONTMAPDIR_DYNAMIC, "gnome-print.fontmap");
		map = gf_fontmap_new (GF_FONTMAP_DYNAMIC, path);
		map->next = db->dynamicmaps;
		db->dynamicmaps = map;
		g_free (path);
	}

	/* Static map */
	if (!stat (FONTMAPDIR_STATIC, &s) && S_ISDIR (s.st_mode)) {
		gf_fontmap_load_dir (db, FONTMAPDIR_STATIC, GF_FONTMAP_STATIC);
	}
	/* Check for staticmap */
	if (!db->staticmaps || (db->staticmaps->type != GF_FONTMAP_STATIC)) {
		path = g_concat_dir_and_file (FONTMAPDIR_STATIC, "gnome-print.fontmap");
		map = gf_fontmap_new (GF_FONTMAP_STATIC, path);
		map->next = db->staticmaps;
		db->staticmaps = map;
		g_free (path);
	}

	return db;
}

void
gf_font_db_free (GFFontDB *db)
{
	g_return_if_fail (db != NULL);
	/* fixme: Implementing this just makes installes bigger (Lauris) */
}

GFFontMap *
gf_fontmap_new (GFFontMapType type, const guchar *path)
{
	GFFontMap *map;

	map = g_new (GFFontMap, 1);
	map->next = NULL;
	map->gpversion = 0.0;
	map->type = type;
	map->path = g_strdup (path);
	map->fonts = NULL;
	map->last = NULL;
	map->fontdict = g_hash_table_new (g_str_hash, g_str_equal);

	return map;
}

void
gf_fontmap_free (GFFontMap *map)
{
	/* This is useless */
}

static void
gf_font_db_append_map (GFFontDB *db, GFFontMap *map, GFFontMapType type)
{
	GFFontMap *ref;

	ref = NULL;
	switch (type) {
	case GF_FONTMAP_USER:
		if (!db->usermaps) {
			db->usermaps = map;
		} else {
			ref = db->usermaps;
		}
		break;
	case GF_FONTMAP_DYNAMIC:
		if (!db->dynamicmaps) {
			db->dynamicmaps = map;
		} else {
			ref = db->dynamicmaps;
		}
		break;
	case GF_FONTMAP_STATIC:
		if (!db->staticmaps) {
			db->staticmaps = map;
		} else {
			ref = db->staticmaps;
		}
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	if (ref) {
		while (ref->next) ref = ref->next;
		ref->next = map;
	}
}

static gint
gf_fontmap_compare_names (gconstpointer a, gconstpointer b)
{
	if (!strcmp (a, "gnome-print.fontmap")) return -1;
	if (!strcmp (b, "gnome-print.fontmap")) return 1;
	return strcmp (a, b);
}

/* Tries to load all *.fontmap files from given directory */

static void
gf_fontmap_load_dir (GFFontDB *db, const guchar *dirname, GFFontMapType type)
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
		files = g_slist_sort (files, gf_fontmap_compare_names);
		while (files) {
			struct stat s;
			gchar *filename;
			filename = g_concat_dir_and_file (dirname, (gchar *) files->data);
			if (!stat (filename, &s) && S_ISREG (s.st_mode)) {
				GFFontMap *map;
				if (!strcmp (files->data, "gnome-print.fontmap")) {
					map = gf_fontmap_load_file (filename, type);
				} else {
					map = gf_fontmap_load_file (filename, GF_FONTMAP_UNKNOWN);
				}
				if (map) gf_font_db_append_map (db, map, type);
			}
			g_free (files->data);
			g_free (filename);
			files = g_slist_remove (files, files->data);
		}
	}
}

/* Parse file, and add fonts to map, if it is valid fontmap */

static GFFontMap *
gf_fontmap_load_file (const guchar *filename, GFFontMapType type)
{
	GFFontMap *map;
	xmlDocPtr doc;

	map = NULL;
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
					xmlChar *gpversion;
					gpversion = xmlGetProp (root, "gpversion");
					/* We are even right version */
					map = gf_fontmap_new (type, filename);
					if (gpversion) {
						map->gpversion = (gint) floor (1000.0 * atof (gpversion) + 0.f);
						xmlFree (gpversion);
					}
					gf_fm_load_fonts_2_0 (map, root);
				}
				xmlFree (version);
			}
		}
		xmlFreeDoc (doc);
	}

	return map;
}

/* Parse root element and build fontmap step 1 */

static void
gf_fm_load_fonts_2_0 (GFFontMap *map, xmlNodePtr root)
{
	xmlNodePtr child;
	GSList *fonts;

	fonts = NULL;
	for (child = root->xmlChildrenNode; child != NULL; child = child->next) {
		if (!strcmp (child->name, "font")) {
			xmlChar *format;
			/* We are font */
			format = xmlGetProp (child, "format");
			if (format) {
				if (!strcmp (format, "type1") || !strcmp (format, "type1alias")) {
					/* We are type1/type1alias entry */
					gf_fm_load_font_2_0_type1 (map, child);
				} else if (!strcmp (format, "truetype")) {
					/* We are truetype entry */
					gf_fm_load_font_2_0_truetype (map, child);
				}
				xmlFree (format);
			}
		}
	}
}

static void
gf_fm_load_font_2_0_type1 (GFFontMap *map, xmlNodePtr node)
{
	GFFontEntry *e;
	xmlChar *xmlname, *xmlfamilyname, *xmlpsname;
	xmlNodePtr child;
	GFFileEntry afm, pfb;

	/* Get our unique name */
	xmlname = xmlGetProp (node, "name");
	if (!xmlname) return;
	/* Check, whether we are already registered */
	if (g_hash_table_lookup (map->fontdict, xmlname)) {
		xmlFree (xmlname);
		return;
	}

	/* Read required entries */
	xmlfamilyname = xmlGetProp (node, "familyname");
	xmlpsname = xmlGetProp (node, "psname");
	if (!xmlfamilyname || !xmlpsname) {
		xmlFree (xmlname);
		if (xmlfamilyname) xmlFree (xmlfamilyname);
		if (xmlpsname) xmlFree (xmlpsname);
		return;
	}

	/* Search for file entries */
	afm.path = NULL;
	pfb.path = NULL;
	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		/* Scan all children nodes */
		if (!strcmp (child->name, "file")) {
			xmlChar *type, *xmlpath, *xmlmtime, *xmlsize;
			/* We are <file> node */
			type = xmlGetProp (child, "type");
			if (type && !strcmp (type, "afm") && !afm.path) {
				xmlpath = xmlGetProp (child, "path");
				if (xmlpath) {
					xmlmtime = xmlGetProp (child, "mtime");
					xmlsize = xmlGetProp (child, "size");
					afm.path = g_strdup (xmlpath);
					afm.mtime = (xmlmtime) ? atoi (xmlmtime) : 0;
					afm.size = (xmlsize) ? atoi (xmlsize) : 0;
					afm.psname = g_strdup (xmlpsname);
					if (xmlmtime) xmlFree (xmlmtime);
					if (xmlsize) xmlFree (xmlsize);
					xmlFree (xmlpath);
				}
			} else if (type && !strcmp (type, "pfb") && !pfb.path) {
				xmlpath = xmlGetProp (child, "path");
				if (xmlpath) {
					xmlChar *xmlalias;
					xmlmtime = xmlGetProp (child, "mtime");
					xmlsize = xmlGetProp (child, "size");
					xmlalias = xmlGetProp (child, "alias");
					pfb.path = g_strdup (xmlpath);
					pfb.mtime = (xmlmtime) ? atoi (xmlmtime) : 0;
					pfb.size = (xmlsize) ? atoi (xmlsize) : 0;
					pfb.psname = (xmlalias) ? g_strdup (xmlalias) : g_strdup (xmlpsname);
					if (xmlalias) xmlFree (xmlalias);
					if (xmlmtime) xmlFree (xmlmtime);
					if (xmlsize) xmlFree (xmlsize);
					xmlFree (xmlpath);
				}
			}
			if (type) xmlFree (type);
		}
		if (afm.path && pfb.path) break;
	}
	if (!pfb.path) {
		if (afm.path) g_free (afm.path);
		if (afm.psname) g_free (afm.psname);
		if (pfb.path) g_free (pfb.path);
		if (pfb.psname) g_free (pfb.psname);
		xmlFree (xmlname);
		xmlFree (xmlfamilyname);
		xmlFree (xmlpsname);
		return;
	}

	/* We have enough information to build font */
	e = g_new0 (GFFontEntry, 1);
	e->type = GF_FONT_ENTRY_TYPE1;
	e->files[0] = pfb;
	if (afm.path) e->files[1] = afm;
	e->name = g_strdup (xmlname);
	xmlFree (xmlname);
	e->familyname = g_strdup (xmlfamilyname);
	xmlFree (xmlfamilyname);

	gf_fm_load_common_data (e, node);

	g_hash_table_insert (map->fontdict, e->name, e);
	if (map->last) {
		map->last->next = e;
	} else {
		map->fonts = e;
	}
	map->last = e;
}

static void
gf_fm_load_font_2_0_truetype (GFFontMap *map, xmlNodePtr node)
{
	GFFontEntry *e;
	xmlChar *xmlname, *xmlfamilyname, *xmlpsname;
	xmlNodePtr child;
	GFFileEntry ttf;
	xmlChar *t;

	/* Get our unique name */
	xmlname = xmlGetProp (node, "name");
	if (!xmlname) return;
	/* Check, whether we are already registered */
	if (g_hash_table_lookup (map->fontdict, xmlname)) {
		xmlFree (xmlname);
		return;
	}

	/* Read required entries */
	xmlfamilyname = xmlGetProp (node, "familyname");
	xmlpsname = xmlGetProp (node, "psname");
	if (!xmlfamilyname || !xmlpsname) {
		xmlFree (xmlname);
		if (xmlfamilyname) xmlFree (xmlfamilyname);
		if (xmlpsname) xmlFree (xmlpsname);
		return;
	}

	/* Search for file entries */
	ttf.path = NULL;
	for (child = node->xmlChildrenNode; child != NULL; child = child->next) {
		/* Scan all children nodes */
		if (!strcmp (child->name, "file")) {
			xmlChar *type, *xmlpath, *xmlmtime, *xmlsize;
			/* We are <file> node */
			type = xmlGetProp (child, "type");
			if (type && !strcmp (type, "ttf") && !ttf.path) {
				xmlpath = xmlGetProp (child, "path");
				if (xmlpath) {
					xmlmtime = xmlGetProp (child, "mtime");
					xmlsize = xmlGetProp (child, "size");
					ttf.path = g_strdup (xmlpath);
					ttf.mtime = (xmlmtime) ? atoi (xmlmtime) : 0;
					ttf.size = (xmlsize) ? atoi (xmlsize) : 0;
					ttf.psname = g_strdup (xmlpsname);
					if (xmlmtime) xmlFree (xmlmtime);
					if (xmlsize) xmlFree (xmlsize);
					xmlFree (xmlpath);
				}
			}
			if (type) xmlFree (type);
		}
		if (ttf.path) break;
	}
	if (!ttf.path) {
		if (ttf.path) g_free (ttf.path);
		if (ttf.psname) g_free (ttf.psname);
		xmlFree (xmlname);
		xmlFree (xmlfamilyname);
		xmlFree (xmlpsname);
		return;
	}

	/* We have enough information to build font */
	e = g_new0 (GFFontEntry, 1);
	e->type = GF_FONT_ENTRY_TRUETYPE;
	e->files[0] = ttf;
	e->name = g_strdup (xmlname);
	xmlFree (xmlname);
	e->familyname = g_strdup (xmlfamilyname);
	xmlFree (xmlfamilyname);

	gf_fm_load_common_data (e, node);

	t = xmlGetProp (node, "subface");
	e->face = (t) ? atoi (t) : 0;
	if (t) xmlFree (t);

	g_hash_table_insert (map->fontdict, e->name, e);
	if (map->last) {
		map->last->next = e;
	} else {
		map->fonts = e;
	}
	map->last = e;
}

static void
gf_fm_load_common_data (GFFontEntry *e, xmlNodePtr node)
{
	xmlChar *t;

	t = xmlGetProp (node, "version");
	e->version = (t) ? g_strdup (t) : g_strdup ("1.0");
	if (t) xmlFree (t);
	t = xmlGetProp (node, "notice");
	e->notice = (t) ? g_strdup (t) : NULL;
	if (t) xmlFree (t);
	t = xmlGetProp (node, "speciesname");
	e->speciesname = (t) ? g_strdup (t) : (gchar *) gf_fm_get_species_name (e->name, e->familyname);
	if (t) xmlFree (t);
	t = xmlGetProp (node, "weight");
	e->weight = (t) ? g_strdup (t) : g_strdup ("Book");
	if (t) xmlFree (t);
	t = xmlGetProp (node, "italicangle");
	if (t == NULL) {
		gchar *p;
		p = strstr (e->speciesname, "Italic");
		if (!p) p = strstr (e->speciesname, "Oblique");
		e->italicangle = p ? -10.0 : 0.0;
	} else {
		e->italicangle = atof (t);
		xmlFree (t);
	}
}

static guchar *
gf_fm_get_species_name (const guchar *fullname, const guchar *familyname)
{
	gchar * p;

	p = strstr (fullname, familyname);

	if (!p) return g_strdup ("Normal");

	p = p + strlen (familyname);

	while (*p && (*p < 'A')) p++;

	if (!*p) return g_strdup ("Normal");

	return g_strdup (p);
}

