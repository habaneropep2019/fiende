/*   Gnome Help Window - Michael Fulbright <msf@redhat.com>
 *   A help widget based on a help widget from:
 *   
 *   GTimeTracker - a time tracker
 *   Copyright (C) 1997,98 Eckehard Berns
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __GNOME_HELPWIN_H__
#define __GNOME_HELPWIN_H__


#include <gnome.h>

#ifdef HELP_USE_GTKHTML
#include <gtkhtml/gtkhtml.h>
#else
#include <gtk-xmhtml/gtk-xmhtml.h>
#endif

BEGIN_GNOME_DECLS

/* widget related macros and structures */
#define GNOME_HELPWIN(obj)         GTK_CHECK_CAST(obj, gnome_helpwin_get_type(), GnomeHelpWin)
#define GNOME_HELPWIN_CLASS(klass) GTK_CHECK_CAST_CLASS(klass, gnome_helpwin_get_type(), GnomeHelpWinClass)
#define GNOME_HELPWIN_IS_HELP(obj) GTK_CHECK_TYPE(obj, gnome_helpwin_get_type())

typedef struct _GnomeHelpWin       GnomeHelpWin;
typedef struct _GnomeHelpWinClass  GnomeHelpWinClass;

struct _GnomeHelpWin {
#ifdef HELP_USE_GTKHTML
    GtkHTML parent;
    gboolean writing;
#else
    GtkXmHTML parent;
#endif

    gchar document_path[BUFSIZ];
    gchar *html_source;
};

struct _GnomeHelpWinClass {
#ifdef HELP_USE_GTKHTML
    GtkHTMLClass parent_class;
#else
    GtkXmHTMLClass parent_class;
#endif
};




guint       gnome_helpwin_get_type       (void);
GtkWidget  *gnome_helpwin_new            (void);
guint       gnome_helpwin_close          (GnomeHelpWin *help);

/* load file straight into the HTML widget */
void        gnome_helpwin_goto           (GnomeHelpWin *help,  const char *filename);

void   	    gnome_helpwin_jump_to_anchor (GnomeHelpWin *w, gchar *a);
void   	    gnome_helpwin_jump_to_line   (GnomeHelpWin *w, gint line);
gint   	    gnome_helpwin_get_line       (GnomeHelpWin *w);

void	    statusMsg			 (gchar *msg);
void	    statusPerc			 (gfloat num);

END_GNOME_DECLS


#endif /* __GNOME_HELPWIN_H__ */


