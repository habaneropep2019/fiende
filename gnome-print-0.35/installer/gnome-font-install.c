#define _GNOME_FONT_INSTALL_C_

/*
 * Fontmap file generator for gnome-print
 *
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Chris Lahey <clahey@ximian.com>
 *
 */

#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <popt-gnome.h>
#include <glib.h>
/* I know, that is is not nice, but that is exactly, what xml-config gives us */
#include <parser.h>
#include <xmlmemory.h>
/* End of ugly thing */
#include <freetype/freetype.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <libgnome/gnome-i18n.h>
#include "parseAFM.h"
#include "gf-fontmap.h"
#include "gf-pfb.h"
#include "gf-ttf.h"

/* Known file types */

typedef enum {
	GFI_FILE_UNKNOWN,
	GFI_FILE_PFB,
	GFI_FILE_PFA,
	GFI_FILE_AFM,
	GFI_FILE_TTF
} GFIFileType;

/* Known data about any font file */

typedef struct {
	GFIFileType type;
	GFFileEntry entry;
	gchar *name;
	gchar *familyname;
	gchar *version;
	gint face;
} GFIFileData;

typedef struct {
	guchar *name; /* Font name */
	guchar *version;
	guchar *familyname;
	guchar *speciesname;
	guchar *psname;
	guchar *alias; /* PSName of glyphs */
} GFIAliasData;

typedef struct {
	gchar *name;
	gchar *familyname;
	gchar *psname;
	GSList *afm_list;
	GSList *pfb_list;
	GSList *ttf_list;
} GFIFontData;

static void gfi_verify_fontmap (GFFontMap *map);
static void gfi_verify_font_entry (GFFontEntry *e, gboolean sameversion);
static void gfi_verify_afm_file (GFFontEntry *e, GFFileEntry *f, gboolean force);
static void gfi_verify_pfb_file (GFFontEntry *e, GFFileEntry *f, gboolean force);
static void gfi_verify_ttf_file (GFFontEntry *e, GFFileEntry *f, gboolean force);
static GFIFileData *gfi_read_afm_file_data (const guchar *name);
static GFIFileData *gfi_read_pfb_file_data (const guchar *name);
static void gfi_read_and_register_ttf_file_data (const guchar *name);
static gboolean gfi_test_file_changed (GFFileEntry * f);

static void gfi_read_aliases (const guchar *path);
static void gfi_read_alias (xmlNodePtr node);

static void gfi_scan_path (const guchar *path, gint level);
static void gfi_try_font_file (const guchar *path);

static void gfi_sort_fonts (void);
GSList *gfi_sort_file_list (GSList *list, GFIFileType type);

static void gfi_process_aliases (void);
static void gfi_process_alias (GFIAliasData *a);

static void gfi_build_fonts (void);
static void gfi_build_font (GFIFontData * fd);
static void gfi_build_ttf_font (GFIFontData *fd);

static gboolean gfi_font_is_registered (const guchar *name);
static GFFontEntry *gfi_font_entry_get_by_psname (const guchar *name);

static FILE *gfi_get_output_stream (void);
static gboolean gfi_ensure_directory (const gchar *path);

static void gfi_write_fontmap (FILE * f);
static void gfi_write_font (xmlNodePtr root, GFFontEntry *e);

static guchar * gfi_get_species_name (const guchar *fullname, const guchar *familyname);

/*
 * We use simpler arguments than original version
 *
 * --debug prints debugging information
 * --assignment creates assignment (for both pfb and afm)
 * --fontmap-path directory for .font files
 * --afm-path directory(ies) are searched for relative afm files
 * --pfb-path directory(ies) are searched for relative pfb files
 * --target specifies output file (stdout if none)
 *
 */

static gboolean gfi_debug = FALSE;
static gboolean gfi_smart = FALSE;
static gboolean gfi_recursive = FALSE;
static gboolean gfi_clean = FALSE;
static gboolean gfi_notice = FALSE;
static gboolean gfi_usermap = TRUE;
static gboolean gfi_dynamicmap = FALSE;
static gboolean gfi_staticmap = FALSE;
static gboolean gfi_query_dirs = FALSE;
static gchar *gfi_target = NULL;

static void add_path (poptContext ctx, enum poptCallbackReason reason, const struct poptOption *opt, const char *arg, void *data);

static const struct poptOption options[] = {
	{ "debug", 'd', POPT_ARG_NONE, &gfi_debug, 0,
	  N_("Print out debugging information"), NULL },
	{ "smart", 'q', POPT_ARG_NONE, &gfi_smart, 0,
	  N_("Tries to do everything automatically"), NULL },
	{ "recursive", 'r', POPT_ARG_NONE, &gfi_recursive, 0,
	  N_("Search directories recursively for font files"), NULL },
	{ "clean", 'c', POPT_ARG_NONE, &gfi_clean, 0,
	  N_("Start from zero fontmap, instead of parsing old one"), NULL },
	{ "notice", 'n', POPT_ARG_NONE, &gfi_notice, 0,
	  N_("Include font notice field (Copyright data) into fontmap"), NULL },
	{ "user", 'u', POPT_ARG_NONE, &gfi_usermap, 0,
	  N_("Create $HOME/.gnome/fonts/gnome-print.fontmap"), NULL },
	{ "dynamic", 'd', POPT_ARG_NONE, &gfi_dynamicmap, 0,
	  N_("Create $SYSCONFDIR/gnome/fonts/gnome-print.fontmap"), NULL },
	{ "static", 's', POPT_ARG_NONE, &gfi_staticmap, 0,
	  N_("Create $DATADIR/gnome/fonts/gnome-print.fontmap"), NULL },
	{ "target", 't', POPT_ARG_STRING, &gfi_target, 0,
	  N_("Write output fontmap to specified file (- for stdout)"), NULL },
	{ "dir", 0, POPT_ARG_NONE, &gfi_query_dirs, 0,
	  N_("Return the $DATADIR and $SYSCONFDIR used for this gnome-print installation)"), NULL },
	{ NULL, '\0', POPT_ARG_CALLBACK, &add_path, 0 },
	{ "aliases", 'a', POPT_ARG_STRING, NULL, 0,
	  N_("File describing known aliases"), N_("PATH") },
#if 0
	{ "default", 'D', POPT_ARG_STRING, NULL, 0,
	  N_("Specify default font(s) for any locale"), N_("[LOCALE[,LOCALE]:fontname]") },
#endif
	POPT_AUTOHELP
	{ NULL, '\0', 0, NULL, 0 }
};

/* Well-known font paths */
static const guchar *smart_paths[] = {
        /* generic */
        "/usr/share/fonts",
        "/usr/local/share/fonts",
	DATADIR "/fonts",
        /* Ghostscript */
        "/usr/share/ghostscript/fonts",
        "/usr/lib/ghostscript/fonts",
        "/usr/local/share/ghostscript/fonts",
        "/usr/local/lib/ghostscript/fonts",
        "/usr/freeware/share/ghostscript/fonts",
	DATADIR "/ghostscript/fonts",
	LIBDIR "/ghostscript/fonts",
        /* Debian */
        "/usr/lib/texmf/fonts/type1/adobe",
        "/usr/lib/texmf/fonts/type1/omega",
        "/usr/lib/texmf/fonts/afm",
        "/usr/share/texmf/fonts/type1/adobe",
        "/usr/share/texmf/fonts/type1/omega",
        "/usr/share/texmf/fonts/afm",
        "/usr/share/a2ps",
        /* Groff */
        "/usr/share/groff/font",
        "/usr/lib/groff/font",
	DATADIR "/groff/font",
	LIBDIR "/groff/font",
        /* X */
        "/usr/X11R6/lib/X11/fonts",
	/* Misc */
	"/usr/share/abisource/fonts",
	NULL
};

static const guchar *smart_aliases[] = {
	DATADIR "/gnome/fonts/adobe-urw.font",
	DATADIR "/fonts/adobe-urw.font",
	"/usr/share/gnome/fonts/adobe-urw.font",
	"/usr/share/fonts/adobe-urw.font",
	NULL
};

static GSList *aliaspath_list = NULL;
static GSList *default_list = NULL;

/* List of GFIAliasData structures */
static GSList *alias_list = NULL;

static GSList *goodafm_list = NULL;
static GHashTable *goodafm_dict = NULL;
static GSList *goodpfb_list = NULL;
static GHashTable *goodpfb_dict = NULL;
static GSList *goodttf_list = NULL;
static GHashTable *goodttf_dict = NULL;

static GSList * font_list = NULL;
static GHashTable * font_dict = NULL;
static GSList * goodfont_list = NULL;
static GHashTable * goodfont_dict = NULL;

/* Master font database */
static GFFontDB *masterdb;
/* Existing master map and target fontmap */
static GFFontMap *mastermap, *newmap;

FT_Library ft_library;

int main (int argc, char ** argv)
{
	poptContext ctx;
	FT_Error ft_result;
	GFFontDB *db;
	char ** args;
	FILE *of;
	gint i;
	int result;

	/* Parse arguments */

	ctx = poptGetContext (NULL, argc, argv, options, 0);

	result = poptGetNextOpt (ctx);
	if (result != -1) { 
                fprintf(stderr, "%s: %s: %s\n",
                        "gnome-font-install",
                        poptBadOption(ctx, POPT_BADOPTION_NOALIAS),
                        poptStrerror(result));
                return 1;
	}
	args = (char **) poptGetArgs (ctx);

	/* Initialize FreeType library */
	ft_result = FT_Init_FreeType (&ft_library);
	if (ft_result != FT_Err_Ok) {
		fprintf (stderr, "Cannot initialize FreeType library\n");
		return 1;
	}

	setlocale (LC_NUMERIC, "C");

	/* Directories */
	if (gfi_query_dirs) {
		fprintf (stderr, "$DATADIR=%s\n", DATADIR);
		fprintf (stderr, "$SYSCONFDIR=%s\n", SYSCONFDIR);
		return 0;
	}

	/* Initialize dictionaries */

	goodafm_dict = g_hash_table_new (g_str_hash, g_str_equal);
	goodpfb_dict = g_hash_table_new (g_str_hash, g_str_equal);
	goodttf_dict = g_hash_table_new (g_str_hash, g_str_equal);
	font_dict = g_hash_table_new (g_str_hash, g_str_equal);
	goodfont_dict = g_hash_table_new (g_str_hash, g_str_equal);

	/* Step 0: Smart */
	if (gfi_smart) {
		uid_t euid;
		/* Set appropriate mode */
		gfi_staticmap = gfi_dynamicmap = gfi_usermap = FALSE;
		euid = geteuid ();
		if (!euid) {
			/* root - use dynamic map */
			gfi_dynamicmap = TRUE;
		} else {
			gfi_usermap = TRUE;
		}
		gfi_staticmap = FALSE;
		/* Set recursive */
		gfi_recursive = TRUE;
		/* Not clean */
		gfi_clean = FALSE;
		/* Set target */
		gfi_target = NULL;
		/* Set aliases */
		for (i = 0; smart_aliases[i]; i++) {
			aliaspath_list = g_slist_prepend (aliaspath_list, g_strdup (smart_aliases[i]));
		}
	}

	/* Step 1: Read existing fontmap */
	if (gfi_debug) fprintf (stderr, "Reading fontmap... ");
	if (gfi_clean) {
		db = gf_font_db_new ();
	} else {
		db = gf_font_db_load ();
	}
	masterdb = db;
	if (gfi_debug) fprintf (stderr, "Done\n");

	/* Get fontmap we are going to write into */
	if (gfi_staticmap) {
		mastermap = db->staticmaps;
		newmap = gf_fontmap_new (GF_FONTMAP_STATIC, mastermap->path);
		if (!gfi_target) gfi_target = g_concat_dir_and_file (FONTMAPDIR_STATIC, "gnome-print.fontmap");
	} else if (gfi_dynamicmap) {
		mastermap = db->dynamicmaps;
		newmap = gf_fontmap_new (GF_FONTMAP_DYNAMIC, mastermap->path);
		if (!gfi_target) gfi_target = g_concat_dir_and_file (FONTMAPDIR_DYNAMIC, "gnome-print.fontmap");
	} else {
		mastermap = db->usermaps;
		newmap = gf_fontmap_new (GF_FONTMAP_USER, mastermap->path);
		if (!gfi_target) gfi_target = g_concat_dir_and_file (g_get_home_dir (), ".gnome/fonts/gnome-print.fontmap");
	}
	g_assert (mastermap != NULL);

	/* Verify fonts */
	if (gfi_debug) fprintf (stderr, "Verifying fontmap entries\n");
	gfi_verify_fontmap (db->usermaps);
	gfi_verify_fontmap (db->dynamicmaps);
	gfi_verify_fontmap (db->staticmaps);
	/* Vacuum mastermap */
	mastermap->fonts = NULL;
	if (gfi_debug) fprintf (stderr, "Done\n");

	/* Process alias files */
	if (gfi_debug) fprintf (stderr, "Scanning alias maps...\n");
	while (aliaspath_list) {
		gfi_read_aliases ((guchar *) aliaspath_list->data);
		g_free (aliaspath_list->data);
		aliaspath_list = g_slist_remove (aliaspath_list, aliaspath_list->data);
	}
	if (gfi_debug) fprintf (stderr, "Done\n");

	/* Process directories/fontmaps */
	if (gfi_debug) fprintf (stderr, "Scanning paths\n");
	for (i = 0; args && args[i]; i++) {
		if (gfi_debug) fprintf (stderr, "Scanning path %s\n", args[i]);
		gfi_scan_path (args[i], 0);
	}
	if (gfi_smart) {
		if (gfi_debug) fprintf (stderr, "Scanning smart paths\n");
		for (i = 0; smart_paths[i]; i++) {
			if (gfi_debug) fprintf (stderr, "Scanning path %s\n", smart_paths[i]);
			gfi_scan_path (smart_paths[i], 0);
		}
	}
	if (gfi_debug) fprintf (stderr, "Done\n");

	/* Free popt context */
	poptFreeContext (ctx);

	/*
	 * Now we have:
	 *
	 * alias_list, pointing to GFIAliasData entries
	 * goodafm_list, goodpfb_list pointing to new FontData
	 * goodafm_dict, goodpfb_dict using list member names
	 *
	 */

	/* Sort all files into fonts */
	if (gfi_debug) fprintf (stderr, "Sorting fonts... ");
	gfi_sort_fonts ();
	if (gfi_debug) fprintf (stderr, "Done\n");

	/*
	 * Now we are ready to process fonts
	 *
	 */

	if (gfi_debug) fprintf (stderr, "Building fonts... ");
	gfi_build_fonts ();
	if (gfi_debug) fprintf (stderr, "Done\n");

	/*
	 * Process aliases
	 *
	 */

	if (gfi_debug) fprintf (stderr, "Sorting Type1 aliases... ");
	gfi_process_aliases ();
	if (gfi_debug) fprintf (stderr, "Done\n");

	if (!goodfont_list) {
		if (gfi_debug) fprintf (stderr, "NO FONTS\n");
	} else {
		/*
		 * Write fontmap
		 *
		 */
		of = gfi_get_output_stream ();
		if (of) {
			gfi_write_fontmap (of);
			if (of != stdout) fclose (of);
		}
	}

	gf_font_db_free (db);

	return 0;
}

static void
gfi_verify_fontmap (GFFontMap *map)
{
	while (map) {
		GFFontEntry *e;
		gint ourversion;
		ourversion = (gint) floor (1000.0 * atof (VERSION) + 0.5);
		if (gfi_debug) fprintf (stderr, "Map [%g]: %s ", (gdouble) map->gpversion / 1000.0, map->path);
		for (e = map->fonts; e != NULL; e = e->next) {
			gfi_verify_font_entry (e, (map->gpversion < ourversion));
			if (gfi_debug) fprintf (stderr, ".");
		}
		if (gfi_debug) fprintf (stderr, "\n");
		map = map->next;
	}
}

/*
 * Process GFFontEntry
 *
 * Tests, whether both afm and pfb files are valid
 * (Indirect) Saves file entries to good{$filetype}list
 * If it is alias (i.e. afm.name != pfb.name), save entry to alias_list
 *
 */

static void
gfi_verify_font_entry (GFFontEntry *e, gboolean sameversion)
{
	switch (e->type) {
	case GF_FONT_ENTRY_TYPE1:
		if (e->files[1].psname && strcmp (e->files[1].psname, e->files[0].psname)) {
			GFIAliasData *a;
			/* We are aliased entry */
			a = g_new (GFIAliasData, 1);
			a->name = g_strdup (e->name);
			a->familyname = g_strdup (e->familyname);
			a->speciesname = g_strdup (e->speciesname);
			a->psname = g_strdup (e->files[1].psname);
			a->version = g_strdup (e->version);
			a->alias = g_strdup (e->files[0].psname);
			alias_list = g_slist_prepend (alias_list, a);
			/* Verify pfb as usual */
			gfi_verify_pfb_file (e, &e->files[0], sameversion);
			/* Forced verify of afm */
			if (e->files[1].path) gfi_verify_afm_file (e, &e->files[1], TRUE);
		} else {
			gfi_verify_pfb_file (e, &e->files[0], sameversion);
			if (e->files[1].path) gfi_verify_afm_file (e, &e->files[1], FALSE);
		}
		break;
	case GF_FONT_ENTRY_TRUETYPE:
		gfi_verify_ttf_file (e, &e->files[0], sameversion);
		break;
	default:
		g_assert_not_reached ();
	}
}

/*
 * Verifies afm entry
 *
 * Checks, whether file is intact
 * (Indirectly) If not intact, or no data, try to parse it
 * (Directly, Indirectly) If good, create new GFIFileData
 * and save it to goodafm_list/goodafm_dict
 *
 */

static void
gfi_verify_afm_file (GFFontEntry *e, GFFileEntry *f, gboolean forced)
{
	GFIFileData *fd;

	/* Test, whether we are already verified and registered */
	fd = g_hash_table_lookup (goodafm_dict, f->path);
	if (fd) return;

	if (!forced && !gfi_test_file_changed (f)) {
		fd = g_new0 (GFIFileData, 1);
		fd->type = GFI_FILE_AFM;
		fd->entry.path = g_strdup (f->path);
		fd->entry.size = f->size;
		fd->entry.mtime = f->mtime;
		fd->entry.psname = g_strdup (f->psname);
		fd->name = g_strdup (e->name);
		fd->familyname = g_strdup (e->familyname);
		fd->version = g_strdup (e->version);
	} else {
		fd = gfi_read_afm_file_data (f->path);
	}

	if (fd) {
		goodafm_list = g_slist_prepend (goodafm_list, fd);
		g_hash_table_insert (goodafm_dict, fd->entry.path, fd);
	} else {
#ifdef GFI_VERBOSE
		if (gfi_debug) fprintf (stderr, "Not good: %s\n", f->name);
#endif
	}
}

/* Same as previous for pfb files */

static void
gfi_verify_pfb_file (GFFontEntry *e, GFFileEntry *f, gboolean forced)
{
	GFIFileData * fd;

	/* Test, whether we are already verified and registered */
	fd = g_hash_table_lookup (goodpfb_dict, f->path);
	if (fd) return;

	if (!forced && !gfi_test_file_changed (f)) {
		fd = g_new0 (GFIFileData, 1);
		fd->type = GFI_FILE_PFB;
		fd->entry.path = g_strdup (f->path);
		fd->entry.size = f->size;
		fd->entry.mtime = f->mtime;
		fd->entry.psname = g_strdup (f->psname);
		fd->name = g_strdup (e->name);
		fd->familyname = g_strdup (e->familyname);
		fd->version = g_strdup (e->version);
	} else {
		fd = gfi_read_pfb_file_data (f->path);
	}

	if (fd) {
		goodpfb_list = g_slist_prepend (goodpfb_list, fd);
		g_hash_table_insert (goodpfb_dict, fd->entry.path, fd);
	} else {
#ifdef GFI_VERBOSE
		if (gfi_debug) fprintf (stderr, "Not good: %s\n", f->name);
#endif
	}
}

/* Same as previous for ttf files */

static void
gfi_verify_ttf_file (GFFontEntry *e, GFFileEntry *f, gboolean forced)
{
	GFIFileData *fd;
	gchar *key;

	/* Test, whether we are already verified and registered */
	key = g_strdup_printf ("%s:%d", f->path, e->face);
	fd = g_hash_table_lookup (goodttf_dict, key);
	if (fd) return;

	if (!forced && !gfi_test_file_changed (f)) {
		fd = g_new0 (GFIFileData, 1);
		fd->type = GFI_FILE_TTF;
		fd->entry.path = g_strdup (f->path);
		fd->entry.size = f->size;
		fd->entry.mtime = f->mtime;
		fd->entry.psname = g_strdup (f->psname);
		fd->name = g_strdup (e->name);
		fd->familyname = g_strdup (e->familyname);
		fd->version = g_strdup (e->version);
		fd->face = e->face;
	} else {
		gfi_read_and_register_ttf_file_data (f->path);
		return;
	}

	if (fd) {
		goodttf_list = g_slist_prepend (goodttf_list, fd);
		g_hash_table_insert (goodttf_dict, key, fd);
	} else {
		g_free (key);
#ifdef GFI_VERBOSE
		if (gfi_debug) fprintf (stderr, "TTF Not good: %s\n", f->name);
#endif
	}
}

/*
 * Tries to parse afm file, if sucessful return GFIFileData
 * Return NULL if file does not exist/cannot be parsed
 */

static GFIFileData *
gfi_read_afm_file_data (const guchar *name)
{
	GFIFileData * fd;
	FILE * f;
	int status;
	Font_Info * fi;
	struct stat s;

	fi = NULL;

	if (stat (name, &s) < 0) return NULL;
	if (!gf_afm_check (name)) return NULL;

	f = fopen (name, "r");
	if (!f) return NULL;

	status = parseFile (f, &fi, P_G);

	fclose (f);
	if (status != AFM_ok) {
		if (fi) parseFileFree (fi);
		return NULL;
	}

	/* Loading afm succeeded, so go ahead */

	fd = g_new (GFIFileData, 1);

	fd->type = GFI_FILE_AFM;
	fd->entry.path = g_strdup (name);
	fd->entry.size = s.st_size;
	fd->entry.mtime = s.st_mtime;
	fd->entry.psname = g_strdup (fi->gfi->fontName);
	fd->name = g_strdup (fi->gfi->fullName);
	fd->familyname = g_strdup (fi->gfi->familyName);
	fd->version = g_strdup (fi->gfi->version);

	parseFileFree (fi);

	return fd;
}

/*
 * Same as previous for pfb files
 */

static GFIFileData *
gfi_read_pfb_file_data (const guchar * name)
{
	GFIFileData * fd;
	GFPFB * pfb;
	struct stat s;

	if (stat (name, &s) < 0) return NULL;

	pfb = gf_pfb_open (name);
	if (!pfb) return NULL;

	/* Loading pfb succeeded, so go ahead */

	fd = g_new (GFIFileData, 1);

	fd->type = GFI_FILE_PFB;
	fd->entry.path = g_strdup (name);
	fd->entry.size = s.st_size;
	fd->entry.mtime = s.st_mtime;
	fd->entry.psname = g_strdup (pfb->gfi.fontName);
	fd->name = g_strdup (pfb->gfi.fullName);
	fd->familyname = g_strdup (pfb->gfi.familyName);
	fd->version = g_strdup (pfb->gfi.version);

	gf_pfb_close (pfb);

	return fd;
}

/*
 * NOTICE - ttf parsing is more complex due to collections
 */

static void
gfi_read_and_register_ttf_file_data (const guchar *name)
{
	GFIFileData *fd;
	struct stat s;
	GFTTF *ttf;
	const guchar *familyname, *stylename;

	if (stat (name, &s) < 0) return;
	if (!S_ISREG (s.st_mode)) return;
	if (!g_path_is_absolute (name)) return;

	/* Hack to recognize the only free Japanese TTF font */
	if (!strcmp (name + strlen (name) - 19, "/wadalab-gothic.ttf")) {
		familyname = "Wadalab Gothic";
		stylename = NULL;
	} else {
		familyname = NULL;
		stylename = NULL;
	}

	ttf = gf_ttf_open (name, 0, familyname, stylename);
	if (ttf) {
		gint num_faces, face;
		num_faces = ttf->num_faces;
		face = 0;
		while (ttf) {
			guchar *key;

			fd = g_new (GFIFileData, 1);

			fd->type = GFI_FILE_TTF;
			fd->entry.path = g_strdup (name);
			fd->entry.size = s.st_size;
			fd->entry.mtime = s.st_mtime;
			fd->entry.psname = g_strdup (ttf->gfi.fontName);
			fd->name = g_strdup (ttf->gfi.fullName);
			fd->familyname = g_strdup (ttf->gfi.familyName);
			fd->version = g_strdup (ttf->gfi.version);
			fd->face = face;

			goodttf_list = g_slist_prepend (goodttf_list, fd);
			key = g_strdup_printf ("%s:%d", name, face);
			g_hash_table_insert (goodttf_dict, key, fd);

			gf_ttf_close (ttf);

			face += 1;

			if (face < num_faces) {
				ttf = gf_ttf_open (name, face, familyname, stylename);
			} else {
				ttf = NULL;
			}
		}
	}
}

/*
 * Checks, whether file has been changed, according to its
 * size and mtime fields
 */

static gboolean
gfi_test_file_changed (GFFileEntry *f)
{
	struct stat s;

	if (stat (f->path, &s) < 0) return TRUE;

	/* If we do not have file info, expect it to be changed */

	if ((f->size == 0) || (s.st_size != f->size)) return TRUE;
	if ((f->mtime == 0) || (s.st_mtime != f->mtime)) return TRUE;

	return FALSE;
}

/*
 * Scan path
 * Path can be either directory, in which case it is read (recursively, if needed), or
 * file, in which case it is just tested.
 */

#define MAX_RECURSION_DEPTH 32

static void
gfi_scan_path (const guchar *path, gint level)
{
	static ino_t inodes[MAX_RECURSION_DEPTH + 1];
	struct stat s;

	if (level >= MAX_RECURSION_DEPTH) return;

	if (gfi_debug) fprintf (stderr, "Scanning path %s (%s)\n", path, gfi_recursive ? "recursive" : "non-recursive");

	if (!stat (path, &s)) {
		if (S_ISDIR (s.st_mode)) {
			static gboolean cracktest = FALSE;
			static gboolean crackroot = FALSE;
			static gboolean crackdev = FALSE;
			static gboolean crackproc = FALSE;
			static struct stat s_root, s_dev, s_proc;
			gboolean seemsok;
			gint i;
			/* Check for crack-smoking */
			if (!cracktest) {
				crackroot = !stat ("/", &s_root);
				crackdev = !stat ("/dev", &s_dev);
				crackproc = !stat ("/proc", &s_proc);
				cracktest = TRUE;
			}
			seemsok = TRUE;
			/* Successful stat, check if already scanned */
			for (i = 0; i < level; i++) {
				if (s.st_ino == inodes[i]) {
					seemsok = FALSE;
					break;
				}
			}
			if (crackroot && (level > 0) && (s.st_ino == s_root.st_ino)) seemsok = FALSE;
			if (crackdev && (s.st_ino == s_dev.st_ino)) seemsok = FALSE;
			if (crackproc && (s.st_ino == s_proc.st_ino)) seemsok = FALSE;
			if (seemsok) {
				DIR * dir;
				struct dirent * dent;
				/* Not yet scanned */
				inodes[level] = s.st_ino;
				dir = opendir (path);
				if (dir) {
					while ((dent = readdir (dir))) {
						gchar *fn;
						if (!strcmp (dent->d_name, ".") || !strcmp (dent->d_name, "..")) continue;
						fn = g_concat_dir_and_file (path, dent->d_name);
						if (!stat (fn, &s)) {
							if (S_ISREG (s.st_mode)) {
								gfi_try_font_file (fn);
							} else if (S_ISDIR (s.st_mode) && gfi_recursive) {
								gfi_scan_path (fn, level + 1);
							}
						}
						g_free (fn);
					}
					closedir (dir);
				}
			} else {
				if (gfi_debug) fprintf (stderr, "Circular link or weird path: %s\n", path);
			}
		} else if (S_ISREG (s.st_mode)) {
			gfi_try_font_file (path);
		} else {
			if (gfi_debug) fprintf (stderr, "Invalid path: %s\n", path);
		}
	} else {
		if (gfi_debug) fprintf (stderr, "Unsuccessful stat: %s\n", path);
	}
}

/*
 * tries to determine file type by parsing it
 */

static void
gfi_try_font_file (const guchar *fn)
{
	GFIFileData *fd;
	struct stat s;
	gchar *name;

	if (stat (fn, &s) < 0) return;
	if (!S_ISREG (s.st_mode)) return;

	if (!g_path_is_absolute (fn)) {
		gchar *cdir;
		cdir = g_get_current_dir ();
		name = g_concat_dir_and_file (cdir, fn);
	} else {
		name = g_strdup (fn);
	}

	if (g_hash_table_lookup (goodafm_dict, name) ||
	    g_hash_table_lookup (goodpfb_dict, name) ||
	    g_hash_table_lookup (goodttf_dict, name)) {
		g_free (name);
		return;
	}

	/* Not registered, so try to determine file type */

	fd = gfi_read_afm_file_data (name);
	if (fd) {
		goodafm_list = g_slist_prepend (goodafm_list, fd);
		g_hash_table_insert (goodafm_dict, fd->entry.path, fd);
		g_free (name);
		return;
	}

	fd = gfi_read_pfb_file_data (name);
	if (fd) {
		goodpfb_list = g_slist_prepend (goodpfb_list, fd);
		g_hash_table_insert (goodpfb_dict, fd->entry.path, fd);
		g_free (name);
		return;
	}

	gfi_read_and_register_ttf_file_data (name);

	g_free (name);
	/* Cannot read :( */
}

/*
 * Arranges all afm and pfb FileData into FontData structures
 * goodfont_list - list of new FontData entries
 * goodfont_dict - use FontData name strings
 * Original lists are cleaned
 *
 */

static void
gfi_sort_fonts (void)
{
	goodafm_list = gfi_sort_file_list (goodafm_list, GFI_FILE_AFM);
	goodpfb_list = gfi_sort_file_list (goodpfb_list, GFI_FILE_PFB);
	goodttf_list = gfi_sort_file_list (goodttf_list, GFI_FILE_TTF);
}

GSList *
gfi_sort_file_list (GSList *list, GFIFileType type)
{
	while (list) {
		GFIFileData *file;
		GFIFontData *font;
		file = (GFIFileData *) list->data;
		font = g_hash_table_lookup (font_dict, file->name);
		if (!font) {
			font = g_new (GFIFontData, 1);
			font->name = g_strdup (file->name);
			font->familyname = g_strdup (file->familyname);
			font->psname = g_strdup (file->entry.psname);
			font->afm_list = font->pfb_list = font->ttf_list = NULL;
			font_list = g_slist_prepend (font_list, font);
			g_hash_table_insert (font_dict, font->name, font);
		}
		switch (type) {
		case GFI_FILE_AFM:
			font->afm_list = g_slist_prepend (font->afm_list, file);
			break;
		case GFI_FILE_PFB:
			font->pfb_list = g_slist_prepend (font->pfb_list, file);
			break;
		case GFI_FILE_TTF:
			font->ttf_list = g_slist_prepend (font->ttf_list, file);
			break;
		default:
			g_warning ("Unknown file type encountered in list: %d", type);
			break;
		}
		list = g_slist_remove (list, file);
	}

	return list;
}

/*
 * Fontmap creation step
 */

static void
gfi_process_aliases (void)
{
	while (alias_list) {
		gfi_process_alias ((GFIAliasData *) alias_list->data);
		alias_list = g_slist_remove (alias_list, alias_list->data);
	}
}

static void
gfi_process_alias (GFIAliasData *a)
{
	GFFontEntry *new, *fe;

	/* Return if we are already registered */
	/* fixme: We should test versions here */
	if (g_hash_table_lookup (goodfont_dict, a->name)) return;

	/* Check, whether we have original font */
	fe = gfi_font_entry_get_by_psname (a->alias);
	if (!fe) return;

	/* Simply compose aliased entry */
	new = g_new0 (GFFontEntry, 1);
	new->next = NULL;
	new->type = GF_FONT_ENTRY_TYPE1;
	new->name = g_strdup (a->name);
	new->version = g_strdup (a->version);
	new->familyname = g_strdup (a->familyname);
	new->speciesname = g_strdup (a->speciesname);
	new->notice = NULL;
	new->weight = g_strdup (fe->weight);
	new->italicangle = fe->italicangle;
	/* AFM */
	if (fe->files[1].path) {
		new->files[1] = fe->files[1];
		new->files[1].path = g_strdup (new->files[1].path);
	}
	/* fixme: This is hackish, but gos for now */
	new->files[1].psname = g_strdup (a->psname);
	/* PFB */
	new->files[0] = fe->files[0];
	new->files[0].path = g_strdup (new->files[0].path);
	new->files[0].psname = g_strdup (new->files[0].psname);

	/* Register it */
	if (gfi_debug) fprintf (stderr, "Registered Type1 alias: %s\n", new->name);

	goodfont_list = g_slist_prepend (goodfont_list, new);
	g_hash_table_insert (goodfont_dict, new->name, new);
}

static void
gfi_build_fonts (void)
{
	while (font_list) {
		gfi_build_font ((GFIFontData *) font_list->data);
		font_list = g_slist_remove (font_list, font_list->data);
	}
}

static void
gfi_build_font (GFIFontData *fd)
{
	GFFontEntry *new;
	GFIFileData *afmdata, *pfbdata;
	gdouble afmversion, pfbversion;
	GSList *l;
	GFPFB *pfb;

	/* Return if we are already registered */
	/* Fixme: We should free structs */
	if (g_hash_table_lookup (goodfont_dict, fd->name)) return;

	/* Are we already registered in some master fontmap? */
	if (gfi_font_is_registered (fd->name)) {
		if (gfi_debug) fprintf (stderr, "Font %s is already registered\n", fd->name);
		return;
	}
	if (gfi_debug) fprintf (stderr, "Building font %s\n", fd->name);

	pfbdata = NULL;
	pfbversion = -1e18;

	/* Find pfb vith highest version */
	for (l = fd->pfb_list; l != NULL; l = l->next) {
		GFIFileData *d;
		gdouble v;
		d = (GFIFileData *) l->data;
#if 0
		if (gfi_debug) fprintf (stderr, "Trying pfb %s\n", d->entry.path);
#endif
		v = (d->version) ? atof (d->version) : 1.0;
		if (v > pfbversion) {
			pfbversion = v;
			pfbdata = d;
		}
	}

	/* If we do not have pfb file try to build ttf entry */
	if (!pfbdata) {
		gfi_build_ttf_font (fd);
		return;
	}

	afmdata = NULL;
	afmversion = -1e18;

	/* Find afm vith highest version <= pfb version */
	for (l = fd->afm_list; l != NULL; l = l->next) {
		GFIFileData *d;
		gdouble v;
		d = (GFIFileData *) l->data;
#if 0
		if (gfi_debug) fprintf (stderr, "Trying afm %s\n", d->entry.path);
#endif
		v = (d->version) ? atof (d->version) : 1.0;
		if ((v > afmversion) && (v <= pfbversion)) {
			afmversion = v;
			afmdata = d;
			if (afmversion == pfbversion) break;
		}
	}

	/* Seems, that we have everything */
	pfb = gf_pfb_open (pfbdata->entry.path);
	if (!pfb) return;

	if (gfi_debug) fprintf (stderr, "Got pfb %s\n", pfbdata->entry.path);
	if (gfi_debug) fprintf (stderr, "Got afm %s\n", (afmdata) ? afmdata->entry.path : "NONE");

	new = g_new0 (GFFontEntry, 1);
	new->next = NULL;
	new->type = GF_FONT_ENTRY_TYPE1;
	new->name = g_strdup (pfb->gfi.fullName);
	new->version = g_strdup (pfb->gfi.version);
	new->familyname = g_strdup (pfb->gfi.familyName);
	new->speciesname = gfi_get_species_name (new->name, new->familyname);
	new->notice = (pfb->gfi.notice) ? g_strdup (pfb->gfi.notice) : NULL;
	new->weight = g_strdup (pfb->gfi.weight);
	/* PFB */
	new->files[0] = pfbdata->entry;
	/* AFM */
	if (afmdata) new->files[1] = afmdata->entry;
	/* Misc */
	new->italicangle = pfb->gfi.italicAngle;

	/* Release PFB info */
	gf_pfb_close (pfb);

	/* Register it */
	if (gfi_debug) fprintf (stderr, "Registered font: %s\n", new->name);

	goodfont_list = g_slist_prepend (goodfont_list, new);
	g_hash_table_insert (goodfont_dict, new->name, new);
}

static void
gfi_build_ttf_font (GFIFontData *fd)
{
	GFFontEntry *new;
	GFIFileData *ttfdata;
	gdouble ttfversion;
	GSList * l;
	GFTTF *ttf;

	/* Return if we are already registered */
	/* Fixme: We should free structs */
	if (g_hash_table_lookup (goodfont_dict, fd->name)) return;

	/* Are we already registered in some master fontmap? */
	if (gfi_font_is_registered (fd->name)) {
		if (gfi_debug) fprintf (stderr, "Font %s is already registered\n", fd->name);
		return;
	}

	ttfdata = NULL;
	ttfversion = -1e18;

	/* Find ttf vith highest version */
	for (l = fd->ttf_list; l != NULL; l = l->next) {
		GFIFileData *d;
		gdouble v;
		d = (GFIFileData *) l->data;
		v = (d->version) ? atof (d->version) : 1.0;
		if (v > ttfversion) {
			ttfversion = v;
			ttfdata = d;
		}
	}

	/* If we do not have ttf data, return */
	if (!ttfdata) return;

	/* Read ttf again to get Weight and italic */

	ttf = gf_ttf_open (ttfdata->entry.path, ttfdata->face, fd->familyname, NULL);
	if (!ttf) return;

	/* Now we should have everything we need */

	new = g_new0 (GFFontEntry, 1);
	new->next = NULL;
	new->type = GF_FONT_ENTRY_TRUETYPE;
	new->name = g_strdup (fd->name);
	new->version = g_strdup (ttfdata->version);
	new->familyname = g_strdup (fd->familyname);
	new->speciesname = gfi_get_species_name (new->name, new->familyname);
	new->notice = (ttf->gfi.notice) ? g_strdup (ttf->gfi.notice) : NULL;
	new->weight = g_strdup (ttf->gfi.weight);
	/* TTF */
	new->files[0] = ttfdata->entry;
	new->face = ttfdata->face;
	/* Misc */
	new->italicangle = ttf->gfi.italicAngle;

	/* Release TTF info */
	gf_ttf_close (ttf);

	/* Register it */
	if (gfi_debug) fprintf (stderr, "Registered TTF font: %s\n", new->name);

	goodfont_list = g_slist_prepend (goodfont_list, new);
	g_hash_table_insert (goodfont_dict, new->name, new);
}

static gboolean
gfi_font_is_registered (const guchar *name)
{
	GFFontMap *map;
	gint ourversion;

	ourversion = (gint) floor (1000.0 * atof (VERSION) + 0.5);

	/* Step 1 - other fontmaps of same level */
	for (map = mastermap->next; map != NULL; map = map->next) {
		if (map->gpversion >= ourversion) {
			if (g_hash_table_lookup (map->fontdict, name)) return TRUE;
		}
	}
	/* Step 2 - next levels */
	if (gfi_staticmap) {
		/* Not found in static map, return */
		return FALSE;
	} else if (gfi_dynamicmap) {
		/* Search from static maps */
		for (map = masterdb->staticmaps; map != NULL; map = map->next) {
			if (map->gpversion >= ourversion) {
				if (g_hash_table_lookup (map->fontdict, name)) return TRUE;
			}
		}
		return FALSE;
	} else {
		/* Search from dynamic maps */
		for (map = masterdb->dynamicmaps; map != NULL; map = map->next) {
			if (map->gpversion >= ourversion) {
				if (g_hash_table_lookup (map->fontdict, name)) return TRUE;
			}
		}
		/* Search from static maps */
		for (map = masterdb->staticmaps; map != NULL; map = map->next) {
			if (map->gpversion >= ourversion) {
				if (g_hash_table_lookup (map->fontdict, name)) return TRUE;
			}
		}
		return FALSE;
	}
}

static GFFontEntry *
gfi_font_entry_get_by_psname (const guchar *name)
{
	GFFontMap *map;
	GFFontEntry *fe;
	GSList *l;

	/* Step 0 - scan goodfont list */
	for (l = goodfont_list; l != NULL; l = l->next) {
		fe = (GFFontEntry *) l->data;
		if (fe->type == GF_FONT_ENTRY_TYPE1) {
			if (!strcmp (fe->files[0].psname, name)) return fe;
		}
	}
	/* Step 1 - other fontmaps of same level */
	for (map = mastermap->next; map != NULL; map = map->next) {
		for (fe = map->fonts; fe != NULL; fe = fe->next) {
			if (fe->type == GF_FONT_ENTRY_TYPE1) {
				if (!strcmp (fe->files[0].psname, name)) return fe;
			}
		}
	}
	/* Step 2 - next levels */
	if (gfi_staticmap) {
		/* Not found in static map, return */
		return NULL;
	} else if (gfi_dynamicmap) {
		/* Search from static maps */
		for (map = masterdb->staticmaps; map != NULL; map = map->next) {
			for (fe = map->fonts; fe != NULL; fe = fe->next) {
				if (fe->type == GF_FONT_ENTRY_TYPE1) {
					if (!strcmp (fe->files[0].psname, name)) return fe;
				}
			}
		}
		return FALSE;
	} else {
		/* Search from dynamic maps */
		for (map = masterdb->dynamicmaps; map != NULL; map = map->next) {
			for (fe = map->fonts; fe != NULL; fe = fe->next) {
				if (fe->type == GF_FONT_ENTRY_TYPE1) {
					if (!strcmp (fe->files[0].psname, name)) return fe;
				}
			}
		}
		/* Search from static maps */
		for (map = masterdb->staticmaps; map != NULL; map = map->next) {
			for (fe = map->fonts; fe != NULL; fe = fe->next) {
				if (fe->type == GF_FONT_ENTRY_TYPE1) {
					if (!strcmp (fe->files[0].psname, name)) return fe;
				}
			}
		}
	}

	return NULL;
}

static FILE *
gfi_get_output_stream (void)
{
	FILE *ostream;

	umask (022);

	if (!strcmp (gfi_target, "-")) {
		if (gfi_debug) fprintf (stderr, "Writing fontmap to stdout\n");
		return stdout;
	}
	if (!gfi_ensure_directory (gfi_target)) {
		g_warning ("Cannot create path for %s", gfi_target);
		return NULL;
	}
	ostream = fopen (gfi_target, "w");
	if (ostream) {
		if (gfi_debug) fprintf (stderr, "Writing fontmap to %s\n", gfi_target);
		return ostream;
	}

        fprintf (stderr, "gnome-font-install: Cannot open output file %s: %s\n",
                 gfi_target, g_strerror (errno));
	return NULL;
}

static gboolean
gfi_ensure_directory (const gchar *path)
{
	struct stat s;
	gchar *t, *p;
	GSList *l;

	if (!g_path_is_absolute (path)) {
		gchar *cdir;
		cdir = g_get_current_dir ();
		t = g_concat_dir_and_file (cdir, path);
	} else {
		t = g_strdup (path);
	}
	l = NULL;
	p = t;
	while (*p) {
		while (*p && (*p == '/')) p++;
		if (*p) l = g_slist_prepend (l, p);
		while (*p && (*p != '/')) p++;
		if (*p) {
			*p = '\0';
			p++;
		}
	}
	if (!l) return FALSE;
	l = g_slist_reverse (l);
	p = g_strdup ("");
	while (l->next) {
		gchar *n;
		n = g_strdup_printf ("%s/%s", p, (gchar *) l->data);
		l = g_slist_remove (l, l->data);
		g_free (p);
		p = n;
		if (stat (p, &s)) {
			if (mkdir (p, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) {
				g_warning ("Cannot create directory %s", p);
				return FALSE;
			}
		} else if (!S_ISDIR (s.st_mode)) {
			g_warning ("Fontmap base %s is not directory", p);
			return FALSE;
		}
	}
	g_free (p);

	return TRUE;
}

static void
gfi_write_fontmap (FILE * f)
{
	xmlDocPtr doc;
	xmlNodePtr root;

	doc = xmlNewDoc ("1.0");
	root = xmlNewDocNode (doc, NULL, "fontmap", NULL);
	xmlDocSetRootElement (doc, root);
	xmlSetProp (root, "version", "2.0");
	xmlSetProp (root, "gpversion", VERSION);

	while (goodfont_list) {
		gfi_write_font (root, (GFFontEntry *) goodfont_list->data);
		goodfont_list = g_slist_remove (goodfont_list, goodfont_list->data);
	}

	xmlDocDump (f, doc);
}

static void
gfi_write_font (xmlNodePtr root, GFFontEntry *e)
{
	xmlNodePtr n, f;
	gchar c[128];

	n = xmlNewDocNode (root->doc, NULL, "font", NULL);
	xmlAddChild (root, n);

	if (e->type == GF_FONT_ENTRY_TYPE1) {
		/* Set format */
		if (e->files[0].psname && e->files[1].psname && strcmp (e->files[0].psname, e->files[1].psname)) {
			xmlSetProp (n, "format", "type1alias");
			xmlSetProp (n, "alias", e->files[0].psname);
		} else {
			xmlSetProp (n, "format", "type1");
		}

		if (e->files[1].path) {
			/* afm file */
			f = xmlNewDocNode (root->doc, NULL, "file", NULL);
			xmlAddChild (n, f);
			xmlSetProp (f, "type", "afm");
			xmlSetProp (f, "path", e->files[1].path);
			g_snprintf (c, 128, "%d", (gint) e->files[1].size);
			xmlSetProp (f, "size", c);
			g_snprintf (c, 128, "%d", (gint) e->files[1].mtime);
			xmlSetProp (f, "mtime", c);
		}

		/* pfb file */
		f = xmlNewDocNode (root->doc, NULL, "file", NULL);
		xmlAddChild (n, f);
		xmlSetProp (f, "type", "pfb");
		xmlSetProp (f, "path", e->files[0].path);
		g_snprintf (c, 128, "%d", (gint) e->files[0].size);
		xmlSetProp (f, "size", c);
		g_snprintf (c, 128, "%d", (gint) e->files[0].mtime);
		xmlSetProp (f, "mtime", c);
	} else if (e->type == GF_FONT_ENTRY_TRUETYPE) {
		/* Set format */
		xmlSetProp (n, "format", "truetype");
		g_snprintf (c, 128, "%d", e->face);
		xmlSetProp (n, "subface", c);

		/* ttf file */
		f = xmlNewDocNode (root->doc, NULL, "file", NULL);
		xmlAddChild (n, f);
		xmlSetProp (f, "type", "ttf");
		xmlSetProp (f, "path", e->files[0].path);
		g_snprintf (c, 128, "%d", (gint) e->files[0].size);
		xmlSetProp (f, "size", c);
		g_snprintf (c, 128, "%d", (gint) e->files[0].mtime);
		xmlSetProp (f, "mtime", c);
	} else {
		/* Set format */
		xmlSetProp (n, "format", "unknown");
	}

	/* Other properties */
	xmlSetProp (n, "name", e->name);
	xmlSetProp (n, "version", e->version);
	xmlSetProp (n, "familyname", e->familyname);
	xmlSetProp (n, "speciesname", e->speciesname);
	/* FYI: Warning - there can be entries that have files[1].psname, but not files[1].path */
	xmlSetProp (n, "psname", e->files[1].psname ? e->files[1].psname : e->files[0].psname);
	if (gfi_notice && e->notice) xmlSetProp (n, "notice", e->notice);
	xmlSetProp (n, "weight", e->weight);
	g_snprintf (c, 128, "%g", e->italicangle);
	xmlSetProp (n, "italicangle", c);
}

static guchar *
gfi_get_species_name (const guchar *fullname, const guchar *familyname)
{
	gchar * p;

	p = strstr (fullname, familyname);

	if (!p) return g_strdup ("Normal");

	p = p + strlen (familyname);

	while (*p && (*p < 'A')) p++;

	if (!*p) return g_strdup ("Normal");

	return g_strdup (p);
}

static void
gfi_read_aliases (const guchar *path)
{
	xmlDocPtr doc;

	if (gfi_debug) fprintf (stderr, "Trying alias file %s ... ", path);

	doc = xmlParseFile (path);

	if (doc) {
		xmlNodePtr root;
		root = xmlDocGetRootElement (doc);
		if (!strcmp (root->name, "fontfile")) {
			xmlNodePtr child;
			/* List of font entries */
			if (gfi_debug) fprintf (stderr, "seems OK\n");
			for (child = root->xmlChildrenNode; child != NULL; child = child->next) {
				if (!strcmp (child->name, "font")) {
					gfi_read_alias (child);
				}
			}
		} else {
			if (gfi_debug) fprintf (stderr, "is not font alias file\n");
		}
		xmlFreeDoc (doc);
	} else {
		if (gfi_debug) fprintf (stderr, "is not valid xml file\n");
	}
}

static void
gfi_read_alias (xmlNodePtr node)
{
/* Here we should read alias entry and create GFIAliasData */
	xmlChar *xmlfullname, *xmlfamilyname, *xmlversion, *xmlname;
	xmlChar *xmlalias;

	xmlfullname = xmlGetProp (node, "fullname");
	xmlfamilyname = xmlGetProp (node, "familyname");
	xmlversion = xmlGetProp (node, "version");
	xmlname = xmlGetProp (node, "name");
	xmlalias = xmlGetProp (node, "alias");

	if (xmlfullname && xmlfamilyname && xmlname && xmlalias) {
		GFIAliasData *a;
		a = g_new0 (GFIAliasData, 1);
		a->name = g_strdup (xmlfullname);
		a->familyname = g_strdup (xmlfamilyname);
		a->speciesname = gfi_get_species_name (xmlfullname, xmlfamilyname);
		a->version = (xmlversion) ? g_strdup (xmlversion) : "0.0";
		a->psname = g_strdup (xmlname);
		a->alias = g_strdup (xmlalias);
		alias_list = g_slist_prepend (alias_list, a);
	}

	if (xmlfullname) xmlFree (xmlfullname);
	if (xmlfamilyname) xmlFree (xmlfamilyname);
	if (xmlversion) xmlFree (xmlversion);
	if (xmlname) xmlFree (xmlname);
	if (xmlalias) xmlFree (xmlalias);
}

/* Popt callback */

static void
add_path (poptContext ctx, enum poptCallbackReason reason, const struct poptOption *opt, const char *arg, void *data)
{
	struct stat s;

	if (arg) {
		if (opt->shortName == 'a') {
			if ((stat (arg, &s) == 0) && (S_ISREG (s.st_mode)) && arg) {
				aliaspath_list = g_slist_append (aliaspath_list, g_strdup (arg));
			} else {
				if (gfi_debug) fprintf (stderr, "%s is not a regular file\n", arg);
			}
		} else if (opt->shortName == 'D') {
			default_list = g_slist_prepend (default_list, g_strdup (arg));
		}
	}
}

