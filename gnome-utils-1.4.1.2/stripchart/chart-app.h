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

#ifndef CHART_APP_H
#define CHART_APP_H

#include "config.h"

#include <gnome.h>
#include "chart.h"
#include "strip.h"

char *prog_name;

char *config_fn;

int default_w, default_h;
int geometry_w, geometry_h;
int geometry_x, geometry_y;

void on_help_menu(void);
void on_about_menu(void);

typedef struct
{
  struct _Prefs_edit *prefs;
  struct _Param_group *strip_param_group, *pen_param_group;

  char *config_fn, *params_fn;
  GtkWidget *frame, *hbox, *strip, *pen_sep, *pen;
  GtkWidget *text_window, *text_clist, *file_sel, *editor;
  GtkNotebook *notebook;
}
Chart_app;

#include "prefs.h"
#include "params.h"
#include "eval.h"
#include "utils.h"

void on_button_press(GtkWidget *win, GdkEvent *event, Chart_app *app);
void on_show_values(GtkWidget *unused, Chart_app *app);
void menu_popup(GtkWidget *widget, GdkEvent *event, Chart_app *app);
void text_refresh(Chart *chart, Chart_app *app);
void popt_arg_extractor(
  poptContext state, enum poptCallbackReason reason,
  const struct poptOption *opt, const char *arg, void *data);
Chart_app *chart_app_new(void);

#endif /* CHART_APP_H */
