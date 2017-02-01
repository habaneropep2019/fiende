/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-glyph.h - A wrapper for rsvg glyphs.

   Copyright (C) 1999, 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#ifndef EEL_GLYPH_H
#define EEL_GLYPH_H

#include <libgnome/gnome-defs.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libart_lgpl/art_rect.h>
#include <eel/eel-scalable-font.h>
#include <eel/eel-art-extensions.h>
#include <eel/eel-gdk-pixbuf-extensions.h>

BEGIN_GNOME_DECLS

typedef struct EelGlyph EelGlyph;

EelGlyph *    eel_glyph_new                     (const EelScalableFont *font,
						 int                    font_size,
						 const char            *text,
						 int                    text_length);
void          eel_glyph_free                    (EelGlyph              *glyph);
int           eel_glyph_get_width               (const EelGlyph        *glyph);
int           eel_glyph_get_height              (const EelGlyph        *glyph);
EelDimensions eel_glyph_get_dimensions          (const EelGlyph        *glyph);
ArtIRect      eel_glyph_get_underline_rectangle (const EelGlyph        *glyph);
void          eel_glyph_draw_to_pixbuf          (const EelGlyph        *glyph,
						 GdkPixbuf             *pixbuf,
						 int                    destination_x,
						 int                    destination_y,
						 ArtIRect               clip_area,
						 guint32                color,
						 int                    opacity);
ArtIRect      eel_glyph_intersect               (const EelGlyph        *glyph,
						 int                    glyph_x,
						 int                    glyph_y,
						 ArtIRect        rectangle);

#if !defined (EEL_OMIT_SELF_CHECK)
gboolean      eel_glyph_compare	                (EelGlyph              *a,
						 EelGlyph              *b);
#endif /* EEL_OMIT_SELF_CHECK */

END_GNOME_DECLS

#endif /* EEL_GLYPH_H */


