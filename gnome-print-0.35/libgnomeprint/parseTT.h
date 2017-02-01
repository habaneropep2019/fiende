#ifndef __PARSE_TT_H__
#define __PARSE_TT_H__

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

#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

#include <glib.h>
#include <freetype/freetype.h>

/*
 * Convert loaded Freetype TTF face to Type1
 *
 * ft_face has to be TrueType or TrueType collection
 * embeddedname is the name for resulting PS font
 * glyphmask is bit array of used glyphs
 *
 */

guchar *ttf2pfa (FT_Face face, const guchar *embeddedname, guint32 *glyphmask);

#if 0
#include FT_FREETYPE_H
#include <stdio.h>
#include <libgnomeprint/gnome-font-face.h>
#include <libgnomeprint/gnome-font-private.h>
#include "parseAFM.h"

extern FT_Library ftlib;
extern FT_Error fterr;
extern gchar *ttf_glyph_insert_string;

gint parseTT (const char *path, gint fn, Font_Info **fi, FLAGS flags);
void ttf_cache_init (void);
void ttf_cache_destroy (void);
gint ttf_load_metric (FT_Face face, Font_Info **fi, guint32 unicode);
gint ttf_load_glyph (FT_Face face, const gchar *filename, gint fn, guint32 unicode);
gboolean ttf_has_additional_string (void);
gint ttf_get_additional_list_length (void);
guint ttf_get_additional_list_value (guint index);
gint ttf_set_additional_string (gchar *string);
gint ttf_set_additional_list (void);
#endif

END_GNOME_DECLS

#endif /* __PARSETT_H__ */
