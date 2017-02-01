#define _GNOME_FONT_FAMILY_C_

#include "config.h"

#include <libgnomeprint/gp-fontmap.h>
#include <libgnomeprint/gnome-font-family.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

struct _GnomeFontFamily {
	GtkObject object;

	gchar * name;
};

struct _GnomeFontFamilyClass {
	GtkObjectClass parent_class;
};

static void gnome_font_family_class_init (GnomeFontFamilyClass * klass);
static void gnome_font_family_init (GnomeFontFamily * family);

static void gnome_font_family_destroy (GtkObject * object);

static GtkObjectClass * parent_class = NULL;

GtkType
gnome_font_family_get_type (void) {
	static GtkType family_type = 0;
	if (!family_type) {
		GtkTypeInfo family_info = {
			"GnomeFontFamily",
			sizeof (GnomeFontFamily),
			sizeof (GnomeFontFamilyClass),
			(GtkClassInitFunc) gnome_font_family_class_init,
			(GtkObjectInitFunc) gnome_font_family_init,
			NULL, NULL,
			(GtkClassInitFunc) NULL,
		};
		family_type = gtk_type_unique (gtk_object_get_type (), &family_info);
	}
	return family_type;
}

static void
gnome_font_family_class_init (GnomeFontFamilyClass * klass)
{
	GtkObjectClass * object_class;

	object_class = (GtkObjectClass *) klass;

	parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->destroy = gnome_font_family_destroy;
}

static void
gnome_font_family_init (GnomeFontFamily * family)
{
	family->name = NULL;
}

static void
gnome_font_family_destroy (GtkObject * object)
{
	GnomeFontFamily * family;

	family = (GnomeFontFamily *) object;

	if (family->name) {
		g_free (family->name);
		family->name = NULL;
	}

	if (((GtkObjectClass *) parent_class)->destroy)
		(* ((GtkObjectClass *) parent_class)->destroy) (object);
}

GnomeFontFamily *
gnome_font_family_new (const gchar * name)
{
	GnomeFontFamily * family;
	GPFontMap * map;
	GPFamilyEntry * f;

	g_return_val_if_fail (name != NULL, NULL);

	family = NULL;

	map = gp_fontmap_get ();

	f = g_hash_table_lookup (map->familydict, name);

	if (f) {
		family = gtk_type_new (GNOME_TYPE_FONT_FAMILY);
		family->name = g_strdup (name);
	}

	gp_fontmap_release (map);

	return family;
}

GList *
gnome_font_family_style_list (GnomeFontFamily * family)
{
	GPFontMap * map;
	GPFamilyEntry * f;
	GList * list;

	g_return_val_if_fail (family != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FAMILY (family), NULL);

	list = NULL;

	map = gp_fontmap_get ();

	f = g_hash_table_lookup (map->familydict, family->name);

	if (f) {
		GSList * l;
		for (l = f->fonts; l != NULL; l = l->next) {
			GPFontEntry * e;
			e = (GPFontEntry *) l->data;
			list = g_list_prepend (list, g_strdup (e->speciesname));
		}
		list = g_list_reverse (list);
	}

	gp_fontmap_release (map);

	return list;
}

void
gnome_font_family_style_list_free (GList * list)
{
	while (list) {
		g_free (list->data);
		list = g_list_remove (list, list->data);
	}
}

GnomeFontFace *
gnome_font_family_get_face_by_stylename (GnomeFontFamily * family, const gchar * style)
{
	GnomeFontFace * face;
	GPFontMap * map;
	GPFamilyEntry * f;

	g_return_val_if_fail (family != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FAMILY (family), NULL);
	g_return_val_if_fail (style != NULL, NULL);

	face = NULL;

	map = gp_fontmap_get ();

	f = g_hash_table_lookup (map->familydict, family->name);

	if (f) {
		GSList * l;
		for (l = f->fonts; l != NULL; l = l->next) {
			GPFontEntry * e;
			e = (GPFontEntry *) l->data;
			if (!strcmp (style, e->speciesname)) {
				face = gnome_font_face_new (e->name);
			}
		}
	}

	gp_fontmap_release (map);

	return face;
}






