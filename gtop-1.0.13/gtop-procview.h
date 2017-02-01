/*
 * gtop-procview.h
 * written by Martin Baulig <martin@home-of-linux.org>
 * based upon hex-document.h from Jaka Mocnik <jaka.mocnik@kiss.uni-lj.si>
 */

#ifndef __GTOP_PROCVIEW_H__
#define __GTOP_PROCVIEW_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <gnome.h>
#include <gtop-page.h>
#include <procview.h>

BEGIN_GNOME_DECLS

#define GTOP_PROCVIEW(obj)		GTK_CHECK_CAST (obj, gtop_procview_get_type (), GTopProcView)
#define GTOP_PROCVIEW_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, gtop_procview_get_type (), GTopProcViewClass)
#define IS_GTOP_PROCVIEW(obj)		GTK_CHECK_TYPE (obj, gtop_procview_get_type ())

typedef struct _GTopProcView		GTopProcView;
typedef struct _GTopProcViewClass	GTopProcViewClass;

struct _GTopProcView
{
	GtkBin bin;

	GTopPage *page;

	gint run_tag;
	GTopProcViewData data;
};

struct _GTopProcViewClass
{
	GtkBinClass parent_class;
};

GtkWidget	*gtop_procview_new	(GTopPage *, gint);
gchar		*gtop_procview_label	(GTopProcViewType);
guint		gtop_procview_get_type	(void);

extern GnomeMDI *mdi;

extern GnomeUIInfo gtop_procview_radio_items [];

END_GNOME_DECLS

#endif /* __GTOP_PROCVIEW_H__ */

