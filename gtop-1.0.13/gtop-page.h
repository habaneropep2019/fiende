/*
 * gtop-page.h
 * written by Martin Baulig <martin@home-of-linux.org>
 * based upon hex-document.h from Jaka Mocnik <jaka.mocnik@kiss.uni-lj.si>
 */

#ifndef __GTOP_PAGE_H__
#define __GTOP_PAGE_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <gnome.h>

BEGIN_GNOME_DECLS

#define GTOP_PAGE(obj)		GTK_CHECK_CAST (obj, gtop_page_get_type (), GTopPage)
#define GTOP_PAGE_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, gtop_page_get_type (), GTopPageClass)
#define IS_GTOP_PAGE(obj)	GTK_CHECK_TYPE (obj, gtop_page_get_type ())

typedef GtkWidget *		(*GTopGenericFunc)(gpointer);

typedef struct _GTopPage	GTopPage;
typedef struct _GTopPageClass	GTopPageClass;

typedef enum _GTopPageType	GTopPageType;

enum _GTopPageType
{
	GTOP_PAGE_PROCVIEW = 0,
	GTOP_PAGE_MEMUSAGE,
	GTOP_PAGE_FSUSAGE,
	GTOP_PAGE_GENERIC
};

struct _GTopPage
{
	GnomeMDIChild mdi_child;

	GTopPageType type;
	gint subtype;

	GTopGenericFunc generic_func;
	gchar *config_string, *label;
	gpointer generic_data;
};

struct _GTopPageClass
{
	GnomeMDIChildClass parent_class;
};

GTopPage *gtop_generic_page_new (GTopGenericFunc, const gchar *,
				 const gchar *, gpointer);
GTopPage *gtop_page_new (GTopPageType type, gint subtype);
GnomeMDIChild *gtop_page_create_from_config (const gchar *);

guint gtop_page_get_type (void);

END_GNOME_DECLS

#endif /* __GTOP_PAGE_H__ */

