/* -*- Mode: C -*-
 * $Id: gdiskfree_app.c,v 1.12 2001/09/25 04:00:15 sopwith Exp $
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
#include <gnome.h>
#include <glibtop.h>
#include <glibtop/fsusage.h>
#include <glibtop/mountlist.h>
#include <string.h>

#include "gdiskfree_app.h"
#include "gdiskfree_menus.h"
#include "gdiskfree_options.h"

extern GDiskFreeOptions *current_options;

/****************************************************************************
 * Forward references 
 **/
static gint           delete_event_cb              (GtkWidget      *w,
						    GdkEventAny    *e,
						    gpointer       data);
/**
 * gdiskfree_app_new:
 **/
GDiskFreeApp *
gdiskfree_app_new (const gchar *geometry)
{
  GDiskFreeApp     *app;

  app = g_new (GDiskFreeApp, 1);
  
  app->app = gnome_app_new (PACKAGE, _("GDiskFree"));
  app->drive_frame  = NULL;
  app->drives       = NULL;
  gdiskfree_install_menus_and_toolbar (app);
  gtk_signal_connect (GTK_OBJECT (app->app), "delete_event", 
		      (GtkSignalFunc) delete_event_cb,
		      app);
  /** geometry parsing **/
  if (geometry != NULL)
    {
      gint x, y, w, h;
      if (gnome_parse_geometry (geometry, &x, &y, &w, &h) )
        {
          if (x != -1)
            gtk_widget_set_uposition (app->app, x, y);
          if (w != -1)
            gtk_window_set_default_size (GTK_WINDOW (app->app), w, h);
        }
      else
        {
          g_error (_("Unable to parse the geometry string '%s'"), geometry);
        }
    }
  else
    {                              /* Lets see if the config has a geometry */
      gint x, y, w, h;
      w = gnome_config_get_int ("/GDiskFree/Geometry/width");
      h = gnome_config_get_int ("/GDiskFree/Geometry/height");
      if (w != 0 && h != 0)
        gtk_window_set_default_size (GTK_WINDOW (app->app), w, h);
      x = gnome_config_get_int ("/GDiskFree/Geometry/xpos=-1");
      y = gnome_config_get_int ("/GDiskFree/Geometry/ypos=-1");
      /* this should really do fitting to screen, not just showing a 50x50
       * corner of the window, but I'm too lazy -George */
      if (x > gdk_screen_width () - 50)
	      x = gdk_screen_width () - 50;
      if (y > gdk_screen_height () - 50)
	      y = gdk_screen_height () - 50;
      if (x != -1 && y != -1)
        gtk_widget_set_uposition (app->app, x, y);
    }
  if (current_options->orientation == GTK_ORIENTATION_VERTICAL)
    app->dial_box = gtk_vbox_new (FALSE, 8);
  else
    app->dial_box = gtk_hbox_new (FALSE, 8);
  gtk_container_set_border_width (GTK_CONTAINER (app->dial_box), 6);
  gnome_app_set_contents (GNOME_APP (app->app), app->dial_box);
  return app;
}
/**
 * gdiskfree_app_close:
 **/
void
gdiskfree_app_close (GDiskFreeApp *app)
{
  gint x, y, w, h;
  g_return_if_fail (app != NULL);

  /* Last window save the position of the window */
  gdk_window_get_position (app->app->window, &x, &y);
  gdk_window_get_size (app->app->window, &w, &h);
  
  gnome_config_set_int ("/GDiskFree/Geometry/width", w);
  gnome_config_set_int ("/GDiskFree/Geometry/height", h);
  gnome_config_set_int ("/GDiskFree/Geometry/xpos", x);
  gnome_config_set_int ("/GDiskFree/Geometry/ypos", y);
  gnome_config_sync ();
  
  gtk_widget_destroy (GTK_WIDGET (app->app));
  app->app = NULL;
  g_free (app);
  gtk_main_quit ();
}
/***
 * delete_event handling
 **/
static gint 
delete_event_cb (GtkWidget *window, GdkEventAny *e, gpointer data)
{
  GDiskFreeApp *app = (GDiskFreeApp *)data;
  gdiskfree_app_close (app);
  return TRUE;
}
/****************************************************************************
 * Support functions
 **/
/**
 * gdiskfree_app_add_disk:
 **/
void
gdiskfree_app_add_disk (GDiskFreeApp *app, const gchar *disk,
			const gchar *mount_point, const gchar *disk_size)
{
  GtkWidget      *frame;
  GtkWidget      *box;
  GtkAdjustment  *adjustment;
  GDiskFreeDisk  *gdisk;

  frame = gtk_frame_new (disk);
  gtk_box_pack_start (GTK_BOX (app->dial_box), frame, TRUE, TRUE, 0);
  app->drive_frame = g_list_append (app->drive_frame, frame);
  box = gtk_vbox_new (FALSE, 1);
  gtk_container_set_border_width (GTK_CONTAINER (box), 6);
  gtk_container_add (GTK_CONTAINER (frame), box);
  adjustment = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 100.0, 0.01, 
						   0.1, 0.0));
  gdisk = g_malloc (sizeof (GDiskFreeDisk));
  gdisk->drive = g_strdup (disk);
  gdisk->mount_point = g_strdup (mount_point);
  gdisk->dial = gtk_dial_new (adjustment);
  gtk_dial_set_view_only (GTK_DIAL (gdisk->dial), TRUE);
  gtk_box_pack_start (GTK_BOX (box), gdisk->dial, FALSE, FALSE, 0);
  app->drives = g_list_append (app->drives, gdisk);
  /* Add mount point label */
  gdisk->mount_label = gtk_label_new (mount_point);
  gtk_box_pack_start (GTK_BOX (box), gdisk->mount_label, FALSE, FALSE, 0);
  gtk_widget_show_all (frame);
  if (current_options->show_mount)
    gtk_widget_show (gdisk->mount_label);
  else
    gtk_widget_hide (gdisk->mount_label);
  /* Add disk size label */
  gdisk->size_label = gtk_label_new (disk_size);
  gtk_box_pack_start (GTK_BOX (box), gdisk->size_label, FALSE, FALSE, 0);
  if (current_options->show_size)
    gtk_widget_show (gdisk->size_label);
  else
    gtk_widget_hide (gdisk->size_label);

}
/**
 * gdiskfree_app_change_orient
 **/
void
gdiskfree_app_change_orient (GDiskFreeApp *app, GtkOrientation orientation)
{
  GList          *gl;
  GtkWidget      *frame;

  gl = app->drive_frame;
  while (gl)
    {
      frame = GTK_WIDGET (gl->data);
      gtk_widget_ref (frame);
      gtk_container_remove (GTK_CONTAINER (app->dial_box), frame);
      gl = g_list_next (gl);
    }
  gtk_widget_destroy (app->dial_box);
  if (orientation == GTK_ORIENTATION_VERTICAL)
    {
      app->dial_box = gtk_vbox_new (FALSE, 8);
    }
  else
    {
      app->dial_box = gtk_hbox_new (FALSE, 8);
    }
  gtk_container_set_border_width (GTK_CONTAINER (app->dial_box), 6);
  gl = app->drive_frame;
  while (gl)
    {
      frame = GTK_WIDGET (gl->data);
      gtk_box_pack_start (GTK_BOX (app->dial_box), frame, TRUE, TRUE, 0);
      gtk_widget_unref (GTK_WIDGET (frame));
      gl = g_list_next (gl);
    }
  gtk_window_set_policy (GTK_WINDOW (app->app), FALSE, FALSE, TRUE);
  gtk_widget_hide (app->app);
  gnome_app_set_contents (GNOME_APP (app->app), app->dial_box);
  gtk_container_resize_children (GTK_CONTAINER (app->dial_box));
  gtk_widget_show_all (app->dial_box);
  gtk_widget_show (app->app);
  gtk_window_set_policy (GTK_WINDOW (app->app), FALSE, TRUE, FALSE);
}
/**
 * gdiskfree_update
 *
 * The main update function.
 **/
gboolean
gdiskfree_update (GDiskFreeApp *app)
{
  GList              *gl;
  GDiskFreeDisk      *disk;
  gdouble            used;
  gdouble            percent;
  glibtop_fsusage    fsu;

  gl = app->drives;
  if (current_options->sync_required)
    sync ();
  while (gl)
    {
      disk = (GDiskFreeDisk *)gl->data;
      memset(&fsu, 0, sizeof(fsu));
      glibtop_get_fsusage (&fsu, disk->mount_point);
      fsu.blocks /= 2*1024;
      fsu.bfree  /= 2*1024;
      fsu.bavail /= 2*1024;
      used = fsu.blocks - fsu.bfree;
      if(used + fsu.bavail != 0)
        percent = (gdouble) (used * 100.0 / (used + fsu.bavail));
      else
        percent = 100.0;
      if (percent > 100.0)
	      percent = 100.0;
      gtk_dial_set_percentage ( (GtkDial *)disk->dial, (percent / 100.0));
      gl = g_list_next (gl);
    }
  return TRUE;
}
/* EOF */

