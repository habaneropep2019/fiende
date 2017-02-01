#ifndef __GNOME_FONT_FAMILY_H__
#define __GNOME_FONT_FAMILY_H__

/*
 * GnomeFontFamily
 *
 * Authors:
 *   Jody Goldberg <jody@helixcode.com>
 *   Miguel de Icaza <miguel@helixcode.com>
 *   Lauris Kaplinski <lauris@helixcode.com>
 *   Christopher James Lahey <clahey@helixcode.com>
 *   Michael Meeks <michael@helixcode.com>
 *   Morten Welinder <terra@diku.dk>
 *
 * Copyright (C) 1999-2000 Helix Code, Inc. and authors
 *
  */

#define GNOME_TYPE_FONT_FAMILY		 (gnome_font_family_get_type ())
#define GNOME_FONT_FAMILY(obj)		 (GTK_CHECK_CAST ((obj), GNOME_TYPE_FONT_FAMILY, GnomeFontFamily))
#define GNOME_FONT_FAMILY_CLASS(klass)	 (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_FONT_FAMILY, GnomeFontFamilyClass))
#define GNOME_IS_FONT_FAMILY(obj)	 (GTK_CHECK_TYPE ((obj), GNOME_TYPE_FONT_FAMILY))
#define GNOME_IS_FONT_FAMILY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_FONT_FAMILY))

typedef struct _GnomeFontFamily GnomeFontFamily;
typedef struct _GnomeFontFamilyClass GnomeFontFamilyClass;

#include <libgnomeprint/gnome-font-face.h>

BEGIN_GNOME_DECLS

/* The one and only Gtk+ type */

GtkType gnome_font_family_get_type (void);

#define gnome_font_family_ref(f) gtk_object_ref (GTK_OBJECT (f))
#define gnome_font_family_unref(f) gtk_object_unref (GTK_OBJECT (f))

/*
 * Methods
 *
 */

GnomeFontFamily * gnome_font_family_new (const gchar * name);

GList * gnome_font_family_style_list (GnomeFontFamily * family);
void gnome_font_family_style_list_free (GList * list);

GnomeFontFace * gnome_font_family_get_face_by_stylename (GnomeFontFamily * family, const gchar * style);

END_GNOME_DECLS

#endif /* __GNOME_FONT_FAMILY_H__ */






