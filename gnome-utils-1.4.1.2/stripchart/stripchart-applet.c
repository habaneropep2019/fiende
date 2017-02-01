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

#include "chart-app.h"
#include "pen.h"
#include "strip.h"

#include <sys/stat.h>

#include <applet-widget.h>

static void
applet_resize_handler(AppletWidget *applet, int i, Chart_app *app)
{
  int o = applet_widget_get_panel_orient(applet);
  int h = applet_widget_get_panel_pixel_size(applet);
  int w = (o == ORIENT_LEFT || o == ORIENT_RIGHT) ? h : 3 * h;

  gtk_widget_set_usize(GTK_WIDGET(app->frame), w, h);
  #ifdef DEBUG
  printf("resize: %dx%d, %c(%d), %d\n", w, h, "UDLR"[o], o, i);
  #endif
}

static struct
poptOption popt_options[] =
{
  { NULL,             '\0', POPT_ARG_CALLBACK, popt_arg_extractor },
  { "geometry",        'g', POPT_ARG_STRING, NULL, 'g',
    N_("Geometry string: WxH+X+Y"), N_("GEO") },
  { "config-file",     'f', POPT_ARG_STRING, NULL, 'f',
    N_("Configuration file name"), N_("FILE") },
  { NULL,             '\0', 0, NULL, 0 }
};

int
main(int argc, char *argv[])
{
  Chart_app *app;

#ifdef ENABLE_NLS
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif

  default_w = 200, default_h = 50;
  geometry_w = -1, geometry_h = -1;
  geometry_x = -1, geometry_y = -1;

  prog_name = argv[0];
  if (strrchr(prog_name, '/'))
    prog_name = strrchr(prog_name, '/') + 1;

  applet_widget_init(
    _("Stripchart"), VERSION, argc, argv, popt_options, 0, NULL);

  app = chart_app_new();
  app->frame = applet_widget_new(prog_name);
  gtk_widget_show(app->frame);

  gtk_widget_add_events(app->strip, GDK_BUTTON_PRESS_MASK);
  gtk_signal_connect(GTK_OBJECT(app->frame),
    "button-press-event", on_button_press, app);
  gtk_signal_connect(GTK_OBJECT(app->frame),
    "change_pixel_size", applet_resize_handler, app);
  gtk_signal_connect(GTK_OBJECT(app->frame),
    "change_orient", applet_resize_handler, app);

  applet_widget_add(APPLET_WIDGET(app->frame), app->hbox);

  applet_widget_register_stock_callback(APPLET_WIDGET(app->frame),
    "help", GNOME_STOCK_MENU_BLANK, _("Help"), 
    (AppletCallbackFunc)on_help_menu, NULL);
  applet_widget_register_stock_callback(APPLET_WIDGET(app->frame),
    "about", GNOME_STOCK_MENU_ABOUT, _("About"),
    (AppletCallbackFunc)on_about_menu, NULL);
  applet_widget_register_stock_callback(APPLET_WIDGET(app->frame),
    "values", GNOME_STOCK_MENU_SCORES, _("Values"),
    (AppletCallbackFunc)on_show_values, app);
  applet_widget_register_stock_callback(APPLET_WIDGET(app->frame),
    "prefs", GNOME_STOCK_MENU_PREF, _("Prefs..."),
    (AppletCallbackFunc)on_prefs_edit, app);
  applet_widget_register_stock_callback(APPLET_WIDGET(app->frame),
    "params", GNOME_STOCK_MENU_PROP, _("Params..."),
    (AppletCallbackFunc)on_param_edit, app->editor);

  applet_widget_gtk_main();
  return EXIT_SUCCESS;
}
