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

#ifndef PARAMS_H
#define PARAMS_H

#ifdef HAVE_LIBGTOP
#include <glibtop.h>
#include <glibtop/union.h>
#endif

#ifdef HAVE_LIBGTOP
typedef struct
{
  glibtop_cpu     cpu;
  glibtop_mem     mem;
  glibtop_swap    swap;
  glibtop_uptime  uptime;
  glibtop_loadavg loadavg;
  glibtop_netload netload;
}
Gtop;
#endif /* HAVE_LIBGTOP */

struct _Param_group
{
  int interval;
  int visible;
  double filter;
  double t_diff;
  struct timeval t_last, t_now;
#ifdef HAVE_LIBGTOP
  int gtop_cpu, gtop_mem, gtop_swap, gtop_uptime, gtop_load, gtop_net;
  Gtop gtop_last, gtop_now;
#endif
};
typedef struct _Param_group Param_group;

typedef struct
{
  char *name, *desc, *eqn, *fn, *pattern;
  char *top_min, *top_max, *bot_min, *bot_max;
  char *scale, *plot, *color_names;
  int colors;
}
Param_desc;

typedef struct
{
  GtkWidget *table, *name, *desc, *eqn, *fn, *pattern;
  GtkWidget *top_min, *top_max, *bot_min, *bot_max;
  GtkWidget *log, *linear, *color_hbox, *notebook;
  GtkWidget *indicator, *line, *point, *solid;

  int colors, shown, changed;
  GtkWidget **color;

  ChartDatum *strip_data, *pen_data;
}
Param_page;

Param_desc **param_desc_ingest(const char *fn);
Param_page *add_page_before(Chart_app *app, int n, Param_desc *desc);
void on_param_edit(GtkWidget *unused, GtkWidget *editor);
void create_editor(Chart_app *app);
int opts_to_file(Chart_app *app, char *fn);

#endif /* PARAMS_H */
