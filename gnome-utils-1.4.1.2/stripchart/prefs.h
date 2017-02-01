/* Stripchart -- the gnome-utils stripchart plotting utility
 * Copyright (C) 2000 John Kodis <kodis@jagunet.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef PREFS_H
#define PREFS_H

#include "chart-app.h"

#include <gnome-xml/parser.h>	/* XML input routines */
#include <gnome-xml/tree.h>	/* XML output routines */

struct _Prefs_edit
{
  GtkWidget *dialog, *ticks_button, *pen_button;
  GtkObject *strip_interval, *strip_filter;
  GtkObject *minor_ticks, *major_ticks;
  GtkObject *pen_interval, *pen_filter;
};
typedef struct _Prefs_edit Prefs_edit;

void prefs_to_doc(Chart_app *app, xmlDocPtr doc);
int prefs_ingest(Chart_app *app, char *fn);
void on_prefs_edit(GtkWidget *w, Chart_app *app);

#endif /* PREFS_H */
