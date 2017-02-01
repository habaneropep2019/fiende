#ifndef __GF_FONTMAP_H__
#define __GF_FONTMAP_H__

/*
 * Fontmap implementation for font installer
 *
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * TODO: Recycle font entries, if they are identical for different maps
 *
 */

#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

typedef struct _GFFontDB GFFontDB;
typedef struct _GFFontMap GFFontMap;
typedef struct _GFFileEntry GFFileEntry;
typedef struct _GFFontEntry GFFontEntry;

#include <sys/types.h>
#include <glib.h>
#include <gnome-xml/tree.h>

typedef enum {
	GF_FONT_ENTRY_UNKNOWN,
	GF_FONT_ENTRY_TYPE1,
	GF_FONT_ENTRY_TRUETYPE
} GFFontEntryType;

typedef enum {
	GF_FONTMAP_UNKNOWN,
	GF_FONTMAP_USER,
	GF_FONTMAP_DYNAMIC,
	GF_FONTMAP_STATIC
} GFFontMapType;

struct _GFFileEntry {
	gchar *path;
	size_t size;
	time_t mtime;
	gchar *psname;
};

struct _GFFontEntry {
	GFFontEntry *next;
	GFFontEntryType type;
	GFFileEntry files[2];
	gint face;
	guchar *name;
	guchar *version;
	guchar *familyname;
	guchar *speciesname;
	guchar *notice;
	guchar *weight;
	gdouble italicangle;
};

struct _GFFontMap {
	GFFontMap *next;
	GFFontMapType type;
	gint gpversion; /* Multiplied by 1000 */
	gchar *path;
	GFFontEntry *fonts;
	GFFontEntry *last;
	GHashTable *fontdict;
};

struct _GFFontDB {
	GFFontMap *usermaps;
	GFFontMap *dynamicmaps;
	GFFontMap *staticmaps;
};

/* Load global font database */
GFFontDB *gf_font_db_new (void);
GFFontDB *gf_font_db_load (void);
void gf_font_db_free (GFFontDB *db);

GFFontMap *gf_fontmap_new (GFFontMapType type, const guchar *path);
void gf_fontmap_free (GFFontMap *map);

END_GNOME_DECLS

#endif
