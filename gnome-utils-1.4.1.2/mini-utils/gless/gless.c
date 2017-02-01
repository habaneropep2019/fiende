/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *   gless: File pager for X.
 *    
 *   Copyright (C) 1998 Havoc Pennington <hp@pobox.com>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>
#include <gnome.h>
#include <errno.h>

#include <string.h>

#define APPNAME "gless"

#ifndef VERSION
#define VERSION "0.0.0"
#endif

#define INVALID_FD -1

/**************************
  Types 
  *************************/

typedef struct _GnomeLessApp GnomeLessApp;

struct _GnomeLessApp {
  GtkWidget * app;  /* GnomeApp widget  */
  GtkWidget * less; /* GnomeLess widget */
  GnomeAppBar * appbar; /* appbar in GnomeApp, for convenience */
  gchar * file;     /* File being displayed, or NULL */
  GList * dialogs;  /* Any associated dialogs - need destroying when
                       the window closes. */
  gchar *search_string;	/* the string that was last searched for */
  gint search_position;	/* the position in the text of the last search */
};

enum {
	TARGET_URI_LIST
};

/****************************
  Function prototypes
  ******************************/

static void gless_new_app(const gchar * filename, const gchar * geometry,
                          gint fd);
static void popup_about(void);

static void gless_exit(void);
static void gless_app_close(GnomeLessApp * app);
static gboolean
gless_app_show_file(GnomeLessApp * app, const gchar * filename);
static void gless_add_dialog(GnomeLessApp * app, GtkWidget * dialog);
static void gless_remove_dialog(GnomeLessApp * app, GtkWidget * dialog);

static gint delete_event_cb(GtkWidget * w, GdkEventAny * e, gpointer data);
static void about_cb(GtkWidget * w, gpointer data);
static void open_cb(GtkWidget * w, gpointer data);
static void close_cb(GtkWidget * w, gpointer data);
static void save_as_cb(GtkWidget * w, gpointer data);
static void exit_cb(GtkWidget * w, gpointer data);
static void new_app_cb(GtkWidget * w, gpointer data);
static void fixed_cb(GtkWidget * w, gpointer data);

static void drop_file(GtkWidget *widget, GdkDragContext *context,
                      gint x, gint y, GtkSelectionData *selectionData,
                      guint info, guint time, GnomeLessApp *app );
static void find_dialog(GtkWidget * w, gpointer data );
static void find_destroy_cb(GtkWidget * w, GdkEventAny * e, gpointer data);
static void find_cb(GtkWidget * w, gint button, gpointer data);
static void find_again_cb(GtkWidget * w, gpointer data);
static int string_search(gchar * pattern, gchar * text);

/***********************************
  Globals
  ***********************************/

GList * apps = NULL;

const char ** start_files = NULL;
GList * start_geometries = NULL;
gboolean ignore_stdin = FALSE;

/*********************************
  Arg parsing and session management
  *****************************/

enum {
  GEOMETRY_KEY = -1,
  NOSTDIN_KEY  = -2
};

static void
parse_an_arg (poptContext ctx,
              enum poptCallbackReason reason,
              const struct poptOption *opt,
              const char *arg, void *data)
{
  g_print(_("Reason %d\n"), reason);
  switch (opt->val){
    
  case GEOMETRY_KEY:
    start_geometries = g_list_append (start_geometries, (char*)arg);
    break;
    
  case NOSTDIN_KEY:
    ignore_stdin = TRUE;
    break;
  }
}

static const struct poptOption options[] = {
  {NULL, '\0', POPT_ARG_CALLBACK, &parse_an_arg, 0, NULL, NULL},
  {"geometry", '\0', POPT_ARG_STRING, NULL, GEOMETRY_KEY, N_("Where to put the window, and its size"), N_("GEOMETRY")},
  {"nostdin", '\0', POPT_ARG_NONE, NULL, NOSTDIN_KEY, N_("Ignore standard input; may be needed in some cases."), NULL},
  {NULL, '\0', 0, NULL, 0}
};

static void
session_die (void)
{
  gless_exit();
}

static gint 
session_save_state (GnomeClient *client, gint phase, 
                    GnomeRestartStyle save_style, gint shutdown,
                    GnomeInteractStyle interact_style, gint fast, 
                    gpointer client_data)
{
  gchar ** argv = NULL;
  gint argc = 0, num_apps = 0;
  GList * tmp = NULL, * free_me = NULL;

  num_apps = g_list_length(apps);

  g_assert (num_apps > 0);

  /* Each app has --geometry, the geometry string, and a filename,
     plus argv[0] plus NULL terminator plus --nostdin */
  
  argv = g_malloc ( sizeof(char *) * (num_apps * 3 + 3) );

  argv[0] = (gchar *)client_data;
  argv[1] = "--nostdin";         /* Our constructed commandline
                                    will never have a pipeline */

  tmp = apps;
  argc = 2;

  while (tmp) {
    GnomeLessApp * a = tmp->data;
    gchar * s
      = gnome_geometry_string( GTK_WIDGET(a->app)->window );
    argv[argc] = g_strconcat("--geometry=", s, NULL);
    g_free(s);
    free_me = g_list_prepend(free_me, argv[argc]);
    ++argc;

    if ( a->file ) {
      argv[argc] = a->file;
      ++argc;
    }
    tmp = g_list_next(tmp);
  }
  argv[argc] = NULL;
    
  gnome_client_set_clone_command (client, argc, argv);
  gnome_client_set_restart_command (client, argc, argv);

#ifdef GNOME_ENABLE_DEBUG
  while ( *argv ) {
    printf("%s ", *argv);
    ++argv;
  }
  printf("\n"); fflush(stdout);
#endif

  tmp = free_me;
  while (tmp) {
    g_free(tmp->data);
    tmp = g_list_next(tmp);
  }
  g_list_free(free_me);

  return TRUE;
}

/*******************************
  Main
  *******************************/

int main ( int argc, char ** argv )
{
  GnomeClient * client;
  GList * tmp2 = NULL;
  poptContext ctx;
  int i;

  /* Initialize the i18n stuff */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  gnome_init_with_popt_table(APPNAME, VERSION, argc, argv, options, 0, &ctx);

  start_files = poptGetArgs(ctx);

  client = gnome_master_client ();
  if (client){
    gtk_signal_connect (GTK_OBJECT (client), "save_yourself",
                        GTK_SIGNAL_FUNC (session_save_state), argv[0]);
    gtk_signal_connect (GTK_OBJECT (client), "die",
                        GTK_SIGNAL_FUNC (session_die), NULL);
  }

  /* Save the beginning of the lists */
  tmp2 = start_geometries;

  /* Start by putting up windows for any files on the command line. */
  i = 0;
  while (start_files && start_files[i]) {
    gchar * geometry = NULL;
    if ( start_geometries ) {
      geometry = start_geometries->data;
      start_geometries = g_list_next(start_geometries);
    }
    gless_new_app(start_files[i], geometry, INVALID_FD);
    ++i;
  }

  poptFreeContext(ctx);

  /* Next case is just a geometry on the command line - we want
     it to apply if reading from stdin. */
  if ( (start_files == NULL) && start_geometries ) {
    gless_new_app(NULL, start_geometries->data, 
                  ignore_stdin ? INVALID_FD : STDIN_FILENO);
    start_geometries = g_list_next(start_geometries);
  }

  /* Now clean up any geometries that are left over with empty windows */
  while ( start_geometries ) {
    gless_new_app(NULL, start_geometries->data, INVALID_FD);
    start_geometries = g_list_next(start_geometries);
  }

  /* There were no files or geometries on the command line; try
     stdin (new_app ignores it if it's a tty).
     FIXME: if you run just "gless" with no useful stdin (pipe or tty)
     then the program may block forever? Seems like it should but 
     sometimes it doesn't. Thus the ignore_stdin option as a hack. */
  if ( (start_files == NULL) && (tmp2 == NULL) ) {
    gless_new_app(NULL, NULL, 
                  ignore_stdin ? INVALID_FD : STDIN_FILENO);
  }

  /* I am guessing that the char * in the lists (from argp) do 
     not need to be freed. ??? */
  g_list_free(tmp2);

  gtk_main();

  exit(EXIT_SUCCESS);
}

/**************************************
  Set up the GUI
  ******************************/

static GnomeUIInfo help_menu[] = {
	GNOMEUIINFO_HELP(APPNAME),
	GNOMEUIINFO_MENU_ABOUT_ITEM(about_cb,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo file_menu[] = {
	GNOMEUIINFO_MENU_NEW_ITEM(N_("_New Window"),
                              N_("New text viewer window"), 
                              new_app_cb, NULL),
	GNOMEUIINFO_MENU_OPEN_ITEM(open_cb,NULL),
	GNOMEUIINFO_MENU_SAVE_AS_ITEM(save_as_cb,NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_CLOSE_ITEM(close_cb,NULL),
	GNOMEUIINFO_MENU_EXIT_ITEM(exit_cb,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo edit_menu[] =
{
    GNOMEUIINFO_MENU_FIND_ITEM(find_dialog, NULL),
    GNOMEUIINFO_MENU_FIND_AGAIN_ITEM(find_again_cb, NULL),
    GNOMEUIINFO_END
};

static GnomeUIInfo settings_menu[] = {
	{GNOME_APP_UI_TOGGLEITEM, N_("_Fixed Font"), 
		N_("Display text with a fixed font"),
		fixed_cb, NULL, NULL,
		0, 0, 'f', 
		GDK_CONTROL_MASK, NULL },
#if 0
	GNOMEUIINFO_MENU_PREFERENCES_ITEM(preferences_cb,NULL),
	GNOMEUIINFO_SEPARATOR,
#endif
	GNOMEUIINFO_END
};

static GnomeUIInfo main_menu[] = {
	GNOMEUIINFO_MENU_FILE_TREE(file_menu),
    GNOMEUIINFO_MENU_EDIT_TREE(edit_menu),
	GNOMEUIINFO_MENU_SETTINGS_TREE(settings_menu),
	GNOMEUIINFO_MENU_HELP_TREE(help_menu),
	GNOMEUIINFO_END
};

static const GtkTargetEntry drop_types[] = {
  { "text/uri-list", 0, TARGET_URI_LIST },
};

static void gless_new_app(const gchar * filename, const gchar * geometry,
                          gint fd)
{
  GnomeLessApp * app;
  GtkWidget * app_box;

  const gint num_drop_types = sizeof( drop_types ) / 
    sizeof( drop_types [0] );


  gint width = 480, height = 500;
  gboolean geometry_error = FALSE;

  app = g_new(GnomeLessApp, 1);

  app->app = gnome_app_new( APPNAME, _("Text File Viewer") ); 
  app->appbar = GNOME_APPBAR(gnome_appbar_new(FALSE, TRUE, 
                                              GNOME_PREFERENCES_USER));
  gnome_app_set_statusbar(GNOME_APP(app->app), GTK_WIDGET(app->appbar));

  apps = g_list_append(apps, app);

  if (geometry) {
    gint x, y, w, h;
    if ( gnome_parse_geometry( geometry, 
                               &x, &y, &w, &h ) ) {

      if (x != -1)
        gtk_widget_set_uposition (GTK_WIDGET(app->app), x, y);
      if (w != -1) {
        width = w; 
        height = h;
      }
    }
    else {
      /* We want the dialog later, after the main window */
      geometry_error = TRUE;
    }
  }

  gtk_window_set_policy(GTK_WINDOW(app->app), TRUE, TRUE, FALSE);
  gtk_widget_set_usize (GTK_WIDGET(app->app), width, height);

  gnome_app_create_menus_with_data(GNOME_APP(app->app), main_menu, app);

  gnome_app_install_menu_hints(GNOME_APP(app->app), main_menu);

  app_box = gtk_vbox_new ( FALSE, GNOME_PAD );
  gtk_container_border_width(GTK_CONTAINER(app_box), GNOME_PAD);

  gnome_app_set_contents ( GNOME_APP(app->app), app_box );

  gtk_signal_connect ( GTK_OBJECT (app->app), "delete_event",
                       GTK_SIGNAL_FUNC (delete_event_cb),
                       app );

  app->less = gnome_less_new();

  gtk_box_pack_start(GTK_BOX(app_box), app->less, TRUE, TRUE, GNOME_PAD);

  gtk_widget_show_all(app->app);

  /* setup drop support */
  gtk_drag_dest_set( app->less,
                     GTK_DEST_DEFAULT_MOTION |
                     GTK_DEST_DEFAULT_HIGHLIGHT |
                     GTK_DEST_DEFAULT_DROP,
                     drop_types, num_drop_types,
                     GDK_ACTION_COPY );
  gtk_signal_connect( GTK_OBJECT( app->less ), "drag_data_received",
                      drop_file, app );

  app->file = NULL; 

  if (filename) {
    gless_app_show_file(app, filename);
  }
  else if (fd != INVALID_FD) {
    /* If we're interactive, just leave the window empty */
    if ( ! isatty(fd) ) {  
      /* Ignore return value; nothing to be done. */
      gnome_less_show_fd(GNOME_LESS(app->less), fd);
    }
  }

  if (geometry_error) {
    gnome_app_error ( GNOME_APP(app->app),
                      _("Couldn't understand geometry (position and size)\n"
                        " specified on command line"));
  }

  app->dialogs = NULL;
  app->search_string = NULL;
  app->search_position = 0;
}

static void popup_about()
{
  static GtkWidget * ga = NULL;
  static const char * authors[] = { "Havoc Pennington <hp@pobox.com>",
                        NULL };

  if (ga != NULL)
  {
  	gdk_window_show(ga->window);
	gdk_window_raise(ga->window);
	return;
  }
  ga = gnome_about_new (APPNAME,
                        VERSION, 
                        _("Copyright 1998, under the GNU General Public License."),
                        authors,
                        0,
                        0 );
  gtk_signal_connect( GTK_OBJECT(ga), "destroy",
		      GTK_SIGNAL_FUNC(gtk_widget_destroyed), &ga );
  
  gtk_widget_show(ga);
}

static void find_dialog(GtkWidget * w, gpointer data)
{
    GtkWidget *find;
    GtkWidget *entry;

    const gchar *find_history = "find_history";

    entry = gnome_entry_new(find_history);
    gnome_entry_load_history(GNOME_ENTRY(entry));
    gtk_widget_show(entry);

    find = gnome_dialog_new(_("Find string"),
			    _("Find"),
			    GNOME_STOCK_BUTTON_CLOSE,
			    NULL);

    gtk_object_set_data(GTK_OBJECT(find), "app", data);

    gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(find)->vbox),
		       entry, TRUE, TRUE, GNOME_PAD);
    gtk_signal_connect(GTK_OBJECT(find), "delete_event",
		       find_destroy_cb, NULL);
    gtk_signal_connect(GTK_OBJECT(find), "clicked",
		       find_cb, entry);

    gtk_widget_show(find);

    gless_add_dialog((GnomeLessApp *) data, find);
}


/******************************
  Misc.
  *******************************/
static void
gless_show_file_error(GnomeLessApp * app, const gchar * error)
{
  gnome_appbar_pop(app->appbar);
  gnome_app_error(GNOME_APP(app->app), error);
}

static gboolean 
gless_app_show_file(GnomeLessApp * app, const gchar * filename)
{
  gint len;

  g_return_val_if_fail(app != NULL, FALSE);
  g_return_val_if_fail(filename != NULL, FALSE);

  gnome_appbar_push (app->appbar, _("Loading..."));

  if ( ! g_file_exists(filename) ) {
    gchar * s;
    s = g_strdup_printf(_("No such file or directory:\n"
                     "%s"),
                     filename);
    gless_show_file_error(app, s);
    g_free(s);
    return FALSE;
  }

  /* Attempt to handle compressed files. */
  len = strlen(filename);
  if ( (strcmp(&filename[len - 3], ".gz") == 0) ||
       (strcmp(&filename[len - 2], ".Z" ) == 0) ) {
    gchar * s;
    if ( (s = gnome_is_program_in_path("zcat")) ) {
      gchar * c = g_strconcat(s, " ", filename, NULL);
      g_free(s);
      if ( ! gnome_less_show_command(GNOME_LESS(app->less), c) ) {
        gchar * err = 
          g_strdup_printf(_("Failed to decompress and display the file:\n"
                         "%s\n"
                         "%s"),
                         filename, g_unix_error_string(errno));
        gless_show_file_error(app, err);
        g_free(err);
        g_free(c);
        return FALSE;
      }
      g_free(c);
    }
    else {
      gless_show_file_error(app,
                            _("Unable to display compressed file.\n"
                              "zcat not found in your path."));
      return FALSE;
    }
  }
  else {
    if ( ! gnome_less_show_file(GNOME_LESS(app->less), filename) ) {
      gchar * err = 
        g_strdup_printf(_("Error loading file:\n"
                          "%s\n"
                          "%s"),
                          filename,
                          g_unix_error_string(errno));
      gless_show_file_error(app, err);
      g_free(err);
      return FALSE;
    }
  }

  gnome_appbar_pop(app->appbar);

  gtk_window_set_title(GTK_WINDOW(app->app), g_filename_pointer(filename));
  gnome_appbar_set_default(app->appbar, g_filename_pointer(filename));

  app->file = g_strdup(filename);
  app->search_position = 0;

  return TRUE;
}

static void gless_exit(void)
{
  gtk_main_quit();
}

static void gless_app_close(GnomeLessApp * app)
{
  g_return_if_fail(app != NULL);

  /* Clean up any dialogs with references to this app */
  while (app->dialogs) {
    /* This will free each link of the dialog list, so no g_list_free */
    gless_remove_dialog(app, app->dialogs->data);
    /* There's no g_list_next because it's in remove_dialog */
  }

  gtk_widget_destroy(app->app);
  g_free(app->file);
  g_free(app);

  if (g_list_length(apps) == 1) {
    /* Last window closed */
    gless_exit();
  }
  else {
    apps = g_list_remove(apps, app);
  }
}

static gboolean gless_app_save(GnomeLessApp * app, const gchar * path)
{
  gchar * s;

  g_return_val_if_fail(app != NULL, FALSE);
  g_return_val_if_fail(path != NULL, FALSE);

  gnome_appbar_push (app->appbar, _("Saving..."));

  /* FIXME this just overwrites; need to ask whether to do so. */

  if ( gnome_less_write_file(GNOME_LESS(app->less), path) ) {
    gnome_appbar_pop (app->appbar);
    return TRUE; /* succeeded */
  }

  gnome_appbar_pop (app->appbar);

  s = g_strdup_printf(_("Failed to write file:\n"
                    "%s\n"
                    "%s"),
                    path, 
                    g_unix_error_string(errno));

  gnome_app_error(GNOME_APP(app->app), s);
  
  g_free(s);

  return FALSE;
}

static void file_selection_cancel(GtkWidget * fs)
{
  GnomeLessApp * app;

  app = gtk_object_get_user_data(GTK_OBJECT(fs));

  gless_remove_dialog(app, fs);
}

static gint 
file_selection_delete(GtkWidget * fs, GdkEventAny * e, gpointer data)
{
  file_selection_cancel(fs);
  return TRUE;
}

static void file_selection_cb_save(GtkWidget * button, GtkFileSelection * fs)
{
  gchar * fn;
  GnomeLessApp * app;

  fn = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs));

  app = gtk_object_get_user_data(GTK_OBJECT(fs));

  g_return_if_fail(app != NULL);

  /* Leave the file selection up if save failed */
  if ( gless_app_save(app, fn) ) {
    gless_remove_dialog(app, GTK_WIDGET(fs));
  }
}

static void gless_app_save_as(GnomeLessApp * app)
{
  GtkWidget * fs;

  fs = gtk_file_selection_new(_("Save Text As..."));
  
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button), "clicked",
                     GTK_SIGNAL_FUNC(file_selection_cb_save), fs);

  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button),
                            "clicked",
                            GTK_SIGNAL_FUNC(file_selection_cancel), 
                            GTK_OBJECT(fs));

  gtk_signal_connect(GTK_OBJECT(fs), "delete_event", 
                     GTK_SIGNAL_FUNC(file_selection_delete), NULL);
  
  gtk_object_set_user_data(GTK_OBJECT(fs), app); 

  gless_add_dialog(app, fs);

  gtk_widget_show(fs);
}

static void file_selection_cb_open(GtkWidget * button, GtkFileSelection * fs)
{
  gchar * fn;
  GnomeLessApp * app;

  fn = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs));

  app = gtk_object_get_user_data(GTK_OBJECT(fs));

  g_return_if_fail(app != NULL);

  if ( gless_app_show_file(app, fn) ) {
    gless_remove_dialog(app, GTK_WIDGET(fs));
  }
}

static void gless_app_open(GnomeLessApp * app)
{
  GtkWidget * fs;

  fs = gtk_file_selection_new(_("Open Text File..."));
  
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button), "clicked",
                     GTK_SIGNAL_FUNC(file_selection_cb_open), fs);

  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button),
                            "clicked",
                            GTK_SIGNAL_FUNC(file_selection_cancel), 
                            GTK_OBJECT(fs));

  gtk_signal_connect(GTK_OBJECT(fs), "delete_event", 
                     GTK_SIGNAL_FUNC(file_selection_delete), NULL);
  
  gtk_object_set_user_data(GTK_OBJECT(fs), app); 

  gless_add_dialog(app, fs);

  gtk_widget_show(fs);
}

static void gless_add_dialog(GnomeLessApp * app, GtkWidget * dialog)
{
  app->dialogs = g_list_prepend(app->dialogs, dialog);
}

static void gless_remove_dialog(GnomeLessApp * app, GtkWidget * dialog)
{
  app->dialogs = g_list_remove(app->dialogs, dialog);
  gtk_widget_destroy(dialog);
}

/************************************
  Callbacks 
  *********************************/

static gint delete_event_cb(GtkWidget * w, GdkEventAny * e, gpointer data)
{
  gless_app_close((GnomeLessApp *)data);
  return TRUE;
}

static void close_cb(GtkWidget * w, gpointer data)
{
  gless_app_close((GnomeLessApp *)data);
}

static void about_cb(GtkWidget * w, gpointer data)
{
  popup_about();
}

static void save_as_cb(GtkWidget * w, gpointer data)
{
  gless_app_save_as((GnomeLessApp *)data);
}

static void open_cb(GtkWidget * w, gpointer data)
{
  gless_app_open((GnomeLessApp *)data);
}

#ifdef NEED_UNUSED_FUNCTION
static void preferences_cb(GtkWidget *w, gpointer data)
{
  gnome_ok_dialog(_("Sorry, no preferences yet."));
}
#endif

static void exit_cb(GtkWidget * w, gpointer data)
{
  gless_exit();
}

static void new_app_cb(GtkWidget * w, gpointer data)
{
  gless_new_app(NULL, NULL, INVALID_FD);
}

static void fixed_cb(GtkWidget * w, gpointer data)
{
  GnomeLess * less = GNOME_LESS(((GnomeLessApp *)data)->less);
  gnome_less_set_fixed_font(less,
                            GTK_CHECK_MENU_ITEM(w)->active);
  gnome_less_reshow        (less);
}


static void drop_file(GtkWidget *widget, GdkDragContext *context,
                      gint x, gint y, GtkSelectionData *selectionData,
                      guint info, guint time, GnomeLessApp *app )
{
	gchar *text = NULL;

    switch( info ) {
	case TARGET_URI_LIST:
      text = selectionData->data + strlen( "file:" );
      /* get rid of the \r\n from the end */
      *(text + strlen( text ) - 2) = 0;
      gless_app_show_file(app, text);
      break;
    }
    
}


static void find_destroy_cb(GtkWidget * w, GdkEventAny * e, gpointer data)
{
    GnomeLessApp *app;

    app = (GnomeLessApp *) gtk_object_get_data(GTK_OBJECT(w), "app");

    gless_remove_dialog(app, w);
}

static void find_cb(GtkWidget * w, gint button, gpointer data)
{
    GnomeLessApp *app;
    GtkWidget *entry;

    gchar *string;

    app = (GnomeLessApp *) gtk_object_get_data(GTK_OBJECT(w), "app");
    entry = gnome_entry_gtk_entry(GNOME_ENTRY(data));

    string = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));

    find_destroy_cb(w, NULL, NULL);

    if (button == 0) {
	g_free(app->search_string);
	app->search_string = string;
	find_again_cb(NULL, app);
    } else {
	g_free(string);
    }
}

static void find_again_cb(GtkWidget * w, gpointer data)
{
    GnomeLessApp *app;
    GtkText *t;

    gchar *pattern;
    gchar *text;

    gint len;

    gint result;

    app = (GnomeLessApp *) data;

    gnome_appbar_push(app->appbar, _("Searching..."));

    t = ((GnomeLess *) app->less)->text;

    pattern = app->search_string;

    len = gtk_text_get_length(t);
    text = gtk_editable_get_chars(GTK_EDITABLE(t), 0, len);

    if(!pattern) {
	gnome_appbar_pop(app->appbar);
	gnome_app_flash(GNOME_APP(app->app), _("No search string was specified!"));
        return;
    }

    result = string_search(pattern, text + app->search_position);

    if (result != -1) {
	result += app->search_position;
	app->search_position = result + strlen(pattern);
	gtk_editable_set_position(GTK_EDITABLE(t), result);
	gtk_editable_select_region(GTK_EDITABLE(t), result, app->search_position);
    } else {
	app->search_position = 0;
    }

    gnome_appbar_pop(app->appbar);
}

static int string_search(gchar * pattern, gchar * text)
{
    gint i, j, k;
    gint M = strlen(pattern);
    gint N = strlen(text);

    for (i = 0, j = M - 1; j >= 0; j--) {
	while (pattern[j] != text[i + j]) {
	    for (k = 0; j - k >= 0 && pattern[j - k] != text[i + j]; k++) {
	    }
	    i += k;
	    j = M - 1;
	    if ((i + j) > N)
		return -1;
	}
    }
    return i;
}
