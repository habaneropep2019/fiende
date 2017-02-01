#ifndef __GNOME_GLYPHLIST_H__
#define __GNOME_GLYPHLIST_H__

/*
 * WARNING! EXPERIMENTAL API - USE ONLY IF YOU KNOW WHAT YOU ARE DOING!
 *
 * GnomeGlyphList - Device independent glyph list for gnome display engines
 *
 * It is GtkObject - there is probably little reason for it, but at moment
 * I cannot tell the necessary structure, so it is extensible. Maybe one day
 * we can make it little thinner :)
 *
 * Author:
 *   Lauris Kaplinski <lauris@helixcode.com>
 *
 * Copyright (C) 2000 Helix Code, Inc.
 *
 */

#define GNOME_TYPE_GLYPHLIST		(gnome_glyphlist_get_type ())
#define GNOME_GLYPHLIST(obj)		(GTK_CHECK_CAST ((obj), GNOME_TYPE_GLYPHLIST, GnomeGlyphList))
#define GNOME_GLYPHLIST_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_GLYPHLIST, GnomeGlyphListClass))
#define GNOME_IS_GLYPHLIST(obj)		(GTK_CHECK_TYPE ((obj), GNOME_TYPE_GLYPHLIST))
#define GNOME_IS_GLYPHLIST_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_GLYPHLIST))

typedef struct _GnomeGlyphList		GnomeGlyphList;
typedef struct _GnomeGlyphListClass	GnomeGlyphListClass;

#include <libgnomeprint/gnome-font.h>

BEGIN_GNOME_DECLS

/* The one and only Gtk+ type */

GtkType gnome_glyphlist_get_type (void);

/* Methods */

#define gnome_glyphlist_ref(o) gtk_object_ref (GTK_OBJECT (o))
#define gnome_glyphlist_unref(o) gtk_object_unref (GTK_OBJECT (o))

/*
 * the _dumb_ versions of glyphlist creation
 */

GnomeGlyphList * gnome_glyphlist_from_text_dumb (GnomeFont * font, guint32 color,
						 gdouble kerning, gdouble letterspace,
						 const guchar * text);

GnomeGlyphList * gnome_glyphlist_from_text_sized_dumb (GnomeFont * font, guint32 color,
						       gdouble kerning, gdouble letterspace,
						       const guchar * text, gint length);

/*
 * Well - the API is slow and dumb. Hopefully you all will be composing
 * glyphlists by hand in future...
 */

void gnome_glyphlist_glyph (GnomeGlyphList * gl, gint glyph);
void gnome_glyphlist_glyphs (GnomeGlyphList * gl, gint * glyphs, gint num_glyphs);
void gnome_glyphlist_advance (GnomeGlyphList * gl, gboolean advance);
void gnome_glyphlist_moveto (GnomeGlyphList * gl, gdouble x, gdouble y);
void gnome_glyphlist_rmoveto (GnomeGlyphList * gl, gdouble x, gdouble y);
void gnome_glyphlist_font (GnomeGlyphList * gl, GnomeFont * font);
void gnome_glyphlist_color (GnomeGlyphList * gl, guint32 color);
void gnome_glyphlist_kerning (GnomeGlyphList * gl, gdouble kerning);
void gnome_glyphlist_letterspace (GnomeGlyphList * gl, gdouble letterspace);

void gnome_glyphlist_text_dumb (GnomeGlyphList * gl, const gchar * text);
void gnome_glyphlist_text_sized_dumb (GnomeGlyphList * gl, const gchar * text, gint length);

END_GNOME_DECLS

#endif /* __GNOME_GLYPHLIST_H__ */


























