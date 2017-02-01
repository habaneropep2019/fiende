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
#include <libgnomeui/gnome-window-icon.h>

/*
 * save_handler -- run in response to a "save-yourself" signal.
 */
static int
on_save_yourself(GnomeClient *client,
  gint phase, GnomeRestartStyle restart,
  gint shutdown, GnomeInteractStyle interact,
  gint fast, Chart_app *app)
{
  int i;
  char cwd[PATH_MAX];
  char *argv[20], *config_fn;

  getcwd(cwd, sizeof(cwd));
  gnome_client_set_current_directory(client, cwd);
  config_fn = gnome_config_get_real_path(
    gnome_client_get_config_prefix(client));
  config_fn[strlen(config_fn) - 1] = '\0';
  if (opts_to_file(app, config_fn) <= 0)
    error("can't save parameters to file \"%s\".", config_fn);

  i = 0;
  argv[i++] = program_invocation_name;
  argv[i++] = "-g";
  argv[i++] = gnome_geometry_string(app->frame->window);
  argv[i++] = "-f";
  argv[i++] = config_fn;
  argv[i] = NULL;
  gnome_client_set_restart_command(client, i, argv);

  i = 0;
  argv[i++] = "rm";
  argv[i++] = "-f";
  argv[i++] = config_fn;
  argv[i] = NULL;
  gnome_client_set_discard_command(client, i, argv);

  return TRUE;
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
  GnomeClient *client;

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

  gnome_init_with_popt_table(
    _("Stripchart"), VERSION, argc, argv, popt_options, 0, NULL);
  gnome_window_icon_set_default_from_file(
    GNOME_ICONDIR "/gnome-stripchart.png");

  app = chart_app_new();
  app->frame = gnome_app_new("stripchart", _("Stripchart"));
  gtk_window_set_default_size(GTK_WINDOW(app->frame),
    geometry_w < 0 ? default_w : geometry_w, 
    geometry_h < 0 ? default_h : geometry_h);
  gtk_widget_set_uposition(app->frame, geometry_x, geometry_y);
  gtk_widget_show(app->frame);
  gtk_signal_connect(GTK_OBJECT(app->frame), "destroy", gtk_main_quit, NULL);

  gnome_app_set_contents(GNOME_APP(app->frame), app->hbox);

  gtk_widget_add_events(app->strip, GDK_BUTTON_PRESS_MASK);
  gtk_signal_connect(GTK_OBJECT(app->frame),
    "button-press-event", on_button_press, app);

  if ((client = gnome_master_client()) != NULL)
    gtk_signal_connect(GTK_OBJECT(client),
      "save_yourself", GTK_SIGNAL_FUNC(on_save_yourself), app);

  gtk_main();
  return EXIT_SUCCESS;
}
