#ifndef GNOME_UIINFOS_H
#define GNOME_UIINFOS_H

/* implementation in gnomeg.c */

#include <gnome.h>
#include <libgnomeui/gnome-app-helper.h>
#include <gtk/gtkdata.h>

BEGIN_GNOME_DECLS

#define GTK_TYPE_GNOME_UIINFOS		(gnome_uiinfos_get_type ())
#define GNOME_UIINFOS(obj)		(GTK_CHECK_CAST ((obj), GTK_TYPE_GNOME_UIINFOS, GnomeUIInfos))
#define GNOME_UIINFOS_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GNOME_UIINFOS, GnomeUIInfosClass))
#define GNOME_IS_UIINFOS(obj)		(GTK_CHECK_TYPE ((obj), GTK_TYPE_GNOME_UIINFOS))
#define GNOME_IS_UIINFOS_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GNOME_UIINFOS))

typedef struct _GnomeUIInfos       GnomeUIInfos;
typedef struct _GnomeUIInfosClass  GnomeUIInfosClass;

struct _GnomeUIInfos
{
  GtkData data;
  GnomeUIInfo *infos;
};

struct _GnomeUIInfosClass
{
  GtkDataClass parent_class;
};


GtkType        gnome_uiinfos_get_type     (void);
GnomeUIInfos*  gnome_uiinfos_new          ();

GnomeUIInfos  *gnome_uiinfos_intern       (GnomeUIInfos *);

END_GNOME_DECLS

#endif
