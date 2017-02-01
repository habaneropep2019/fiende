/* -*- Mode: C -*-
 * $Id: gdiskfree.c,v 1.15 2001/08/12 16:36:06 gregm Exp $
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
#include <libgnomeui/gnome-window-icon.h>
#include "gdiskfree_app.h"
#include "gdiskfree_options.h"

/****************************************************************************
 * Global config ICKY!!
 **/
GDiskFreeOptions     *current_options = NULL;
guint                timeout_id;

/****************************************************************************
 * Local functions
 **/
static void      session_die              (GnomeClient         *client,
					   gpointer            data);
static gint      save_session             (GnomeClient         *client,
					   gint                phase,
					   GnomeSaveStyle      save_style,
					   gint                is_shutdown,
					   GnomeInteractStyle  interact_style,
					   gint                is_fast,
					   gpointer            client_data);
static gint      excluded_fstype          (const char          *fstype);
guchar *	 gdiskfree_convert_size   ( unsigned long size);
/**
 * popt table
 **/
static gchar       *geometry = NULL;
poptContext        pctx;
struct poptOption options[] = {
  { "geometry", '\0', POPT_ARG_STRING, &geometry, 0,
    N_("Specify the geometry of the window."),
    N_("GEOMETRY") },
  { NULL, '\0', 0, NULL, 0 }
};
/** Support functions **/
static gint
excluded_fstype (const char *fstype)
{
  GList   *gl;
  gchar   *type;
  if (current_options->excluded == NULL)
    return 0;
  gl = current_options->excluded;
  while (gl)
    {
      type = (gchar *)gl->data;
      if (g_strncasecmp (type, fstype, strlen (fstype)) == 0)
        {
/*          g_print ("%s == %s \n", type, fstype); */
	  return 1;
        }
      gl = g_list_next (gl);
    }
  return 0;
}

guchar *
gdiskfree_convert_size ( unsigned long size)
{
     float size_f = (float) size;

     /* The firs time we divide by 1024, after that, we divide
	by 1000 */
     if (size_f < 1024)
	  return g_strdup_printf ("%.1f kb", size_f);
     size_f = size_f / 1024;
     if (size_f < 1000)
	  return g_strdup_printf ("%.1f Mb", size_f);
     size_f = size_f / 1000;
     if (size_f < 1000)
	  return g_strdup_printf ("%.1f Gb", size_f);
     size_f = size_f / 1000;
     return g_strdup_printf ("%.1f Tb", size_f);
}

/**
 * Program entry point
 **/
gint
main (int argc, gchar *argv[])
{
  GDiskFreeApp       *app;
  GnomeClient        *client;
  glibtop_mountlist  mountlist;
  glibtop_mountentry *mount_list;
  gint               i;

  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);
  
  gnome_init_with_popt_table ("gdiskfree", VERSION, argc, argv, options, 0, &pctx);
  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-diskfree.png");
  poptFreeContext (pctx);
  /** Get the configuration (or default) **/
  gdiskfree_option_init ();
  /** Connect to session managment **/
  client = gnome_master_client ();
  gtk_signal_connect (GTK_OBJECT (client), "save_yourself",
		      (GtkSignalFunc) save_session, argv[0]);
  gtk_signal_connect (GTK_OBJECT (client), "die",
		      (GtkSignalFunc) session_die, NULL);

  app = gdiskfree_app_new (geometry);
  mount_list = glibtop_get_mountlist (&mountlist, 0);
  for (i = 0; i < mountlist.number; i++)
    {
      glibtop_fsusage    fsusage;
      unsigned long      size;

      glibtop_get_fsusage (&fsusage, mount_list [i].mountdir);
      size = fsusage.blocks;
      /*g_print ("type: %s\n", mount_list[i].type);*/
      if (!(excluded_fstype (mount_list[i].type)))
	gdiskfree_app_add_disk (app, mount_list[i].devname,
				mount_list[i].mountdir,
				gdiskfree_convert_size (size / 2));
    }
  gtk_widget_show (GTK_WIDGET (app->app));
  gdiskfree_update (app);
  /* Start the update timer */
  timeout_id = gtk_timeout_add (current_options->update_interval,
				(GtkFunction )gdiskfree_update, app);
  gtk_main ();
  return 0;
}
/** Session management **/
/** 
 * save_session:
 **/
static gint 
save_session (GnomeClient *client, gint phase, GnomeSaveStyle save_style,
              gint is_shutdown, GnomeInteractStyle interact_style,
              gint is_fast, gpointer client_data)
{
  gchar **argv;
  guint argc;
  
  argv = g_malloc0(sizeof (gchar *)*4);
  argc = 1;

  argv[0] = client_data;
  /* add any addtional state info here. */
  gnome_client_set_clone_command (client, argc, argv);
  gnome_client_set_restart_command (client, argc, argv);
  return TRUE;
}
/**
 * session_die
 **/
static void
session_die (GnomeClient *client, gpointer client_data)
{
  gtk_main_quit ();
}

/* EOF */
