/* -*- Mode: C -*-
 * $Id: gdiskfree_menus.c,v 1.4 2001/08/10 13:14:09 mawarkus Exp $
 *
 * GDiskFree -- A disk free space toy (df on steriods).
 * Copyright 1998,1999 Gregory McLean
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.,  59 Temple Place - Suite 330, Cambridge, MA 02139, USA.
 *
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gdiskfree_app.h"
#include "gdiskfree_menus.h"
#include "gdiskfree_options.h"

/****************************************************************************
 * Forward declarations
 **/
static void            exit_cb                  (GtkWidget    *widget,
						 gpointer     data);
static void            about_cb                 (GtkWidget    *widget,
						 gpointer     data);
static void            properties_cb            (GtkWidget    *widget,
						 GDiskFreeApp *app);
/** Menus **/
static GnomeUIInfo file_menu [] = {
  GNOMEUIINFO_MENU_EXIT_ITEM (exit_cb, NULL),
  GNOMEUIINFO_END
};
static GnomeUIInfo settings_menu [] = {
  GNOMEUIINFO_MENU_PREFERENCES_ITEM(properties_cb, NULL),
  GNOMEUIINFO_END
};
static GnomeUIInfo help_menu [] = {
  GNOMEUIINFO_HELP ("gdiskfree"),
  GNOMEUIINFO_MENU_ABOUT_ITEM (about_cb, NULL),
  GNOMEUIINFO_END
};
static GnomeUIInfo app_menu [] = {
  GNOMEUIINFO_MENU_FILE_TREE (file_menu),
  GNOMEUIINFO_MENU_SETTINGS_TREE (settings_menu),
  GNOMEUIINFO_MENU_HELP_TREE (help_menu),
  GNOMEUIINFO_END
};
/**
 * gdiskfree_install_menus_and_toolbar:
 **/
void
gdiskfree_install_menus_and_toolbar (GDiskFreeApp *app)
{
  gnome_app_create_menus_with_data (GNOME_APP (app->app), app_menu, app);
  /*  gnome_app_install_menu_hints (GNOME_APP (app->app), app_menu); */
}
/****************************************************************************
 * Callback functions
 **/
static void
exit_cb (GtkWidget *widget, gpointer data)
{
  GDiskFreeApp   *app;
  gint x, y, w, h;
  app = (GDiskFreeApp *)data;
  gdk_window_get_position (app->app->window, &x, &y);
  gdk_window_get_size (app->app->window, &w, &h);

  gnome_config_set_int ("/GDiskFree/Geometry/width", w);
  gnome_config_set_int ("/GDiskFree/Geometry/height", h);
  gnome_config_set_int ("/GDiskFree/Geometry/xpos", x);
  gnome_config_set_int ("/GDiskFree/Geometry/ypos", y);
  gnome_config_sync ();

  gtk_main_quit ();
}
static void
about_cb (GtkWidget *widget, gpointer data)
{
  static GtkWidget    *dialog = NULL;
  GDiskFreeApp        *app;
  app = (GDiskFreeApp *) data;
  if (dialog != NULL)
    {
      g_assert (GTK_WIDGET_REALIZED (dialog));
      gdk_window_show (dialog->window);
      gdk_window_raise (dialog->window);
    }
  else
    {
      const gchar *authors[] = {
	"Gregory McLean <gregm@comstar.net>",
	"And others I'm sure I forgot.",
	NULL
      };
      gchar *logo = gnome_unconditional_pixmap_file ("gdiskfree-splash.png");
      dialog = gnome_about_new (_("GDiskFree"), VERSION, 
				"Copyright 1998, 1999, 2000 Gregory McLean",
				(const gchar **)authors,
				_("GDiskFree is a GNOME implementation "
				  "of the shell utility 'df'."),
				logo);
      g_free (logo);
      gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
			  (GtkSignalFunc) gtk_widget_destroyed,
			  &dialog);
      if (app->app)
	gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (app->app));
      gtk_widget_show (dialog);
    }
}
static void
properties_cb (GtkWidget *widget, GDiskFreeApp *app)
{
  gtk_widget_show_all (gdiskfree_option_dialog (app));
}
/* EOF */

