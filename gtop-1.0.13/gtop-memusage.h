/*
 * gtop-memusage.h
 * written by Martin Baulig <martin@home-of-linux.org>
 * based upon hex-document.h from Jaka Mocnik <jaka.mocnik@kiss.uni-lj.si>
 */

#ifndef __GTOP_MEMUSAGE_H__
#define __GTOP_MEMUSAGE_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <gnome.h>
#include <gtop-page.h>
#include <memusage.h>

BEGIN_GNOME_DECLS
    
#define GTOP_MEMUSAGE(obj)		GTK_CHECK_CAST (obj, gtop_memusage_get_type (), GTopMemUsage)
#define GTOP_MEMUSAGE_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, gtop_memusage_get_type (), GTopMemUsageClass)
#define IS_GTOP_MEMUSAGE(obj)		GTK_CHECK_TYPE (obj, gtop_memusage_get_type ())

typedef struct _GTopMemUsage		GTopMemUsage;
typedef struct _GTopMemUsageClass	GTopMemUsageClass;

struct _GTopMemUsage
{
	GtkScrolledWindow scrolled_window;

	GTopPage *page;

	gint run_tag;
	GTopMemUsageData data;
};

struct _GTopMemUsageClass
{
	GtkScrolledWindowClass parent_class;
};

GtkWidget	*gtop_memusage_new	(GTopPage *, gint);
gchar		*gtop_memusage_label	(GTopMemUsageType);
guint		gtop_memusage_get_type	(void);

extern GnomeMDI *mdi;

extern GnomeUIInfo gtop_memusage_radio_items [];

END_GNOME_DECLS

#endif /* __GTOP_MEMUSAGE_H__ */

