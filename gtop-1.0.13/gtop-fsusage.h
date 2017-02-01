/*
 * gtop-fsusage.h
 * written by Martin Baulig <martin@home-of-linux.org>
 * based upon hex-document.h from Jaka Mocnik <jaka.mocnik@kiss.uni-lj.si>
 */

#ifndef __GTOP_FSUSAGE_H__
#define __GTOP_FSUSAGE_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <gnome.h>
#include <gtop-page.h>
#include <fsusage.h>

BEGIN_GNOME_DECLS

#define GTOP_FSUSAGE(obj)		GTK_CHECK_CAST (obj, gtop_fsusage_get_type (), GTopFsUsage)
#define GTOP_FSUSAGE_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, gtop_fsusage_get_type (), GTopFsUsageClass)
#define IS_GTOP_FSUSAGE(obj)		GTK_CHECK_TYPE (obj, gtop_fsusage_get_type ())

typedef struct _GTopFsUsage		GTopFsUsage;
typedef struct _GTopFsUsageClass	GTopFsUsageClass;

struct _GTopFsUsage
{
	GtkScrolledWindow scrolled_window;

	GTopPage *page;

	gint run_tag;
	GTopFsUsageData data;
};

struct _GTopFsUsageClass
{
	GtkScrolledWindowClass parent_class;
};

GtkWidget	*gtop_fsusage_new	(GTopPage *, gint);
gchar		*gtop_fsusage_label	(GTopFsUsageType);
guint		gtop_fsusage_get_type	(void);

extern GnomeMDI *mdi;

extern GnomeUIInfo gtop_fsusage_radio_items [];

END_GNOME_DECLS

#endif /* __GTOP_FSUSAGE_H__ */

