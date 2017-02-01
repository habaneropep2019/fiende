/*  ----------------------------------------------------------------------

    Copyright (C) 1998  Cesar Miquel  (miquel@df.uba.ar)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    ---------------------------------------------------------------------- */

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gnome.h>
#include "logview.h"

extern ConfigData *cfg;
extern GtkWidget *app;

static GtkWidget *about_window = NULL;

/* Prototypes */

void AboutShowWindow (GtkWidget *widget, gpointer user_data);


void
AboutShowWindow (GtkWidget *widget, gpointer user_data)
{
  const char *author[] = {N_("Cesar Miquel (miquel@df.uba.ar)"), NULL};
  char *comments = N_("This  program  is  part of  the  GNOME  project. " 
"Logview comes with ABSOLUTELY NO WARRANTY. This is free " 
"software, and you are welcome to redistribute it under the conditions "
"of the GNU General Public Licence. The log icon is a courtesy of "
"Tuomas Kuosmanen (a.k.a tigert).");

  if (about_window != NULL) {
	  gtk_widget_show_now (about_window);
	  gdk_window_raise (about_window->window);
	  return;
  }

  /* go get logview.xpm in $(prefix)/share/pixmaps/logview */
  about_window = gnome_about_new (_("Logview"), VERSION,
           			  N_("Copyright (C) 1998"),
				  author, _(comments),
				  "logview/logview.xpm");
  if (app != NULL)
	  gnome_dialog_set_parent (GNOME_DIALOG (about_window),
				   GTK_WINDOW (app));
  gtk_signal_connect (GTK_OBJECT (about_window), "destroy",
		      GTK_SIGNAL_FUNC (gtk_widget_destroyed),
		      &about_window);
  gtk_widget_show (about_window);
  return;

}                           

