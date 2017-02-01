/* WARNING ____ IMMATURE API ____ liable to change */

/* gnome-procbar.h - Gnome Process Bar.

   Copyright (C) 1998 Martin Baulig

   Based on the orignal gtop/procbar.c from Radek Doulik.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.
*/

#ifndef __GTOP_PROC_BAR_H__
#define __GTOP_PROC_BAR_H__

#include <libgnome/gnome-defs.h>
#include <gtk/gtkhbox.h>

BEGIN_GNOME_DECLS

#define GTOP_TYPE_PROC_BAR            (gtop_proc_bar_get_type ())
#define GTOP_PROC_BAR(obj)            (GTK_CHECK_CAST ((obj), GTOP_TYPE_PROC_BAR, GTopProcBar))
#define GTOP_PROC_BAR__CLASS(klass)   (GTK_CHECK_CLASS_CAST ((klass), GTOP_TYPE_PROC_BAR, GTopProcBarClass))
#define GTOP_IS_PROC_BAR(obj)         (GTK_CHECK_TYPE ((obj), GTOP_TYPE_PROC_BAR))
#define GTOP_IS_PROC_BAR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTOP_TYPE_PROC_BAR))

typedef struct _GTopProcBar GTopProcBar;
typedef struct _GTopProcBarPrivate GTopProcBarPrivate;
typedef struct _GTopProcBarClass GTopProcBarClass;

struct _GTopProcBar {
    GtkHBox hbox;

    GTopProcBarPrivate *_priv;
};

struct _GTopProcBarClass {
    GtkHBoxClass parent_class;
};

guint       gtop_proc_bar_get_type        (void);
GtkWidget * gtop_proc_bar_new             (GtkWidget *label,
					   gint n, GdkColor *colors,
					   gint (*cb)());
void        gtop_proc_bar_construct       (GTopProcBar *pb,
					   GtkWidget *label,
					   gint n, GdkColor *colors,
					   gint (*cb)());
void        gtop_proc_bar_set_values      (GTopProcBar *pb, const guint val []);
void        gtop_proc_bar_set_orient      (GTopProcBar *pb, gboolean vertical);
void        gtop_proc_bar_start           (GTopProcBar *pb, gint gtime, gpointer data);
void        gtop_proc_bar_stop            (GTopProcBar *pb);
void        gtop_proc_bar_update          (GTopProcBar *pb, GdkColor *colors);

END_GNOME_DECLS

#endif
