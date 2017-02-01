/*  Gtk+ User Interface Builder
 *  Copyright (C) 1998-2000  Damon Chaplin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>

#include <gtk/gtkmain.h>
#include <gtk/gtkrc.h>

#include "gladeconfig.h"

#ifdef USE_GNOME
#include <gnome.h>
#  ifdef ENABLE_BONOBO
#    ifdef USING_OAF
#      include <liboaf/liboaf.h>
#    else
#      include <libgnorba/gnorba.h>
#    endif
#    include <bonobo.h>
#  endif
#endif

#include "glade.h"
#include "glade_project.h"
#include "glade_project_window.h"
#include "utils.h"

/* These are the arguments parsed from the command line. */
static gchar *arg_filename     = NULL;	/* The XML file to load on start-up. */
static int    arg_write_source = 0;	/* Set to write the source & exit. */


static void parse_command_line (int argc, char *argv[]);
static guint final_setup_from_main_loop (gpointer data);
static void write_source (void);
static void usage (void);


#ifdef USE_GNOME
static poptContext pctx;

static struct poptOption options[] = {
  {
    "write-source",
    'w',
    0,
    &arg_write_source,
    0,
    N_("Write the source code and exit"),
    NULL
  },
  {
    NULL,
    '\0',
    0,
    NULL,
    0,
    NULL,
    NULL
  }
};
#endif


int
main (int argc, char *argv[])
{
  gchar *home_dir, *rc_path;
#ifdef ENABLE_BONOBO
  #ifndef USING_OAF
    CORBA_Environment ev;
  #endif  
  CORBA_ORB         orb;
#endif

  home_dir = NULL;
  rc_path = NULL;

#ifdef USE_GNOME
#  ifdef ENABLE_BONOBO
    /* Gnome sets the locale and parses rc files automatically. */
#    ifdef USING_OAF
       gnomelib_register_popt_table (oaf_popt_options, _("Oaf options"));
       gnome_init_with_popt_table ("Glade", VERSION, argc, argv,
				   options, 0, &pctx);
       orb = oaf_init (argc, argv);
#    else
       CORBA_exception_init (&ev);
       gnome_CORBA_init_with_popt_table ("Glade", VERSION, &argc, argv,
					 options, 0, &pctx, 0, &ev);
       CORBA_exception_free (&ev);
       orb = gnome_CORBA_ORB ();
#    endif
   if (bonobo_init (orb, NULL, NULL) == FALSE)
     g_error ("Could not initialize bonobo");
#  else
    /* Gnome sets the locale and parses rc files automatically. */
   gnome_init_with_popt_table ("Glade", VERSION, argc, argv, options,
			       0, &pctx);
#  endif
#else
  gtk_set_locale ();

  /* For GTK 1.2, default GTK rc files are parsed automatically. */
  home_dir = g_get_home_dir ();
  rc_path = g_strdup_printf ("%s/.gladerc", home_dir);
  gtk_rc_add_default_file (rc_path);
  g_free (rc_path);
  gtk_init (&argc, &argv);
#endif

  /* Ignore Ctrl-C. */
  /*signal (SIGINT, SIG_IGN);*/

#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, GLADE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif

  glade_init ();

  parse_command_line (argc, argv);

  /* If the --write-source option is passed, we just write the source and exit
     without even entering the GTK+ main loop. */
  if (arg_write_source)
    write_source ();

  /* We can't make any CORBA calls unless we're in the main loop.
     So we delay loading of files until then. */
  gtk_idle_add ((GtkFunction) final_setup_from_main_loop, NULL);

#ifdef ENABLE_BONOBO
  bonobo_main ();
#else
  gtk_main ();
#endif
  return 0;
}


/* Currently the only command-line argument we handle is an XML file to load.
   For Gnome we have to use popt, even though we have no options. */
#ifdef USE_GNOME
static void
parse_command_line (int argc, char *argv[])
{
  const gchar **args;
  gint i;

  args = poptGetArgs (pctx);

  for (i = 0; args && args[i]; i++)
    {
      if (arg_filename == NULL)
	arg_filename = (gchar*) args[i];
      else
	usage ();
    }

  poptFreeContext (pctx);
}

#else
static void
parse_command_line (int argc, char *argv[])
{
  gint i;

  /* GTK parses argc & argv and sets arguments to NULL if it has used them. */
  for (i = 1; i < argc; i++)
    {
      if (!argv[i]) continue;
      if (!strcmp (argv[i], "-w") || !strcmp (argv[i], "--write-source"))
	arg_write_source = 1;
      else if (arg_filename == NULL)
	arg_filename = (gchar*) argv[i];
      else
	usage ();
    }
}
#endif


/* This creates the main GUI windows and loads any XML file specified from the
   command-line. We do this here because we can't make any Bonob calls until
   the main loop is running. */
static guint
final_setup_from_main_loop (gpointer data)
{
  GladeProjectWindow *project_window;
  gchar *directory;

  project_window = glade_project_window_new ();
  gtk_widget_show (project_window->window);

  glade_show_palette ();
  glade_show_property_editor ();

  if (arg_filename)
    {
      directory = g_get_current_dir ();
      arg_filename = glade_util_make_absolute_path (directory, arg_filename);
      glade_project_window_open_project (project_window, arg_filename);
      g_free (directory);
      g_free (arg_filename);
    }

  if (current_project == NULL)
    glade_project_new ();

  glade_project_window_set_project (project_window, current_project);

  return FALSE;
}


/* Outputs the source code for the project, for when the --write-source option
   is used. This function will not return. It exits with 0 if OK, or 1 if
   there was an error writing the source. */
static void
write_source (void)
{
  GladeProject *project;
  GladeStatusCode status;
  GladeError *error;
  GList *errors;
  gchar *directory, *filename;

  if (!arg_filename) {
    g_printerr (_("glade: The XML file must be set for the '-w' or '--write-source' option.\n"));
    exit (1);
  }

  directory = g_get_current_dir ();
  filename = glade_util_make_absolute_path (directory, arg_filename);
  g_free (directory);

  status = glade_project_open (filename, &project, &errors);
  g_free (filename);

  if (errors) {
    g_printerr (_("glade: Error loading XML file.\n"));
    /* The errors aren't freed, but it doesn't really matter. */
    exit (1);
  }

  error = glade_project_write_source (project);
  if (error) {
    g_printerr (_("glade: Error writing source.\n"));
    glade_error_free (error);
    exit (1);
  }

  exit (0);
}


/* Display the available command-line options and exit. Used when an invalid
   argument was passed in. */
static void
usage (void)
{
  fprintf (stderr, "Usage: glade [-w|--write-source] [<filename>]\n");
  exit (0);
}
