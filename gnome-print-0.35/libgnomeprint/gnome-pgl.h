#ifndef _GNOME_PGL_H_
#define _GNOME_PGL_H_

/*
 * WARNING! EXPERIMENTAL API - USE ONLY IF YOU KNOW WHAT YOU ARE DOING!
 *
 * GnomePosGlyphList - Positioned glyphlist
 *
 * Author:
 *   Lauris Kaplinski <lauris@helixcode.com>
 *
 * Copyright (C) 2000 Helix Code, Inc.
 *
 */

typedef struct _GnomePosGlyphList GnomePosGlyphList;

#include <libgnomeprint/gnome-glyphlist.h>

BEGIN_GNOME_DECLS

/*
 * Rendering flags
 */

enum {
	GNOME_PGL_RENDER_DEFAULT = 0,
	GNOME_PGL_RENDER_EXACT = (1 << 8),
};

/*
 * Constructor
 */

GnomePosGlyphList * gnome_pgl_from_gl (const GnomeGlyphList * gl, gdouble * transform, guint flags);

void gnome_pgl_destroy (GnomePosGlyphList * pgl);

ArtDRect * gnome_pgl_bbox (const GnomePosGlyphList * pgl, ArtDRect * bbox);

END_GNOME_DECLS

#endif






