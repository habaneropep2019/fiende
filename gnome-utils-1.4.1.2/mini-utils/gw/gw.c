/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *   gw: Graphical version of "w", shows users on the system.
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
#include <libgnomeui/gnome-window-icon.h>

#include <string.h>

#define APPNAME "gw"
#define COPYRIGHT_NOTICE _("Copyright 1998, under the GNU General Public License.")

#ifndef VERSION
#define VERSION "0.0.0"
#endif

/* gnome-config section for actions on w entries */
#define ACTIONS_SECTION "Actions"

/* Used to store pointers to entries in CList 
   in the preferences dialog */
#define NAME_ENTRY_KEY "Name_Entry"
#define COMMAND_ENTRY_KEY "Command_Entry"
#define PROPERTY_BOX_KEY "PropertyBox_Key"

/*************************
  Types
  **************************/

typedef struct _Action Action;

struct _Action {
  gchar * key;      /* displayable name and config key */
  gchar * format;   /* Format for command, with escape sequences
                       for data drawn from "w" display.
                       So far:
                       %u     user name
                       %t     tty   */
};

/****************************
  Function prototypes
  ******************************/

static void prepare_app(void);
static void popup_about(void);

static gint delete_event_cb(GtkWidget * w, gpointer data);
static void about_cb(GtkWidget * w, gpointer data);

static void preferences_cb(GtkWidget *w, gpointer data);

static void get_list_column_number (void);
static void reset_list(GtkCList * list);
static gint list_clicked_cb(GtkCList * list, GdkEventButton * e);


static void new_action_in_prefs(GtkCList * list, gchar * name, 
                                gchar * command);
static void add_action_cb(GtkButton * button, gpointer clist);
static void delete_action_cb(GtkButton * button, gpointer clist);
static void clist_selected_cb(GtkCList * clist, gint row, 
                              gint col, GdkEventButton * event,
                              gpointer add_button);
static void clist_unselected_cb(GtkCList * clist, gint row,
                                gint col, GdkEventButton * event);
static void name_changed_cb(GtkEntry * entry, GtkCList * clist);
static void command_changed_cb(GtkEntry * entry, GtkCList * clist);
static void apply_prefs_cb(GnomePropertyBox * pb, gint page, GtkCList * data);
static void help_prefs_cb(GnomePropertyBox * pb, gint page);
static void add_defaults_cb(GtkButton * button, GtkCList * clist);

static void load_actions(void); 
static void clear_actions(void);
static void save_actions(void); 
static void prepend_action(const gchar * key, const gchar * format);
static void make_actions_popup(void);
static void do_action_cb(GtkWidget * menuitem, Action * a);

/***********************************
  Globals
  ***********************************/

static gint num_columns;

static GtkWidget * app;
static GtkCList * main_clist;

static GList * actions = NULL;

static GtkMenu * actions_popup = NULL;

static gint popup_x, popup_y;
static gint tty_column, name_column;

static gchar * w_command = NULL;

/* Hmm, well, I can't think of much else. */
static Action default_actions[] = {
  {"YTalk", "ytalk %u#%t"},
  {"write", "xterm -e write %u %t"},
  {"talk", "xterm -e talk %u %t"}
};
/* {"Slay", "slay %u"} */ /* Root-only. */

static gint num_defaults = sizeof(default_actions)/sizeof(Action);

/*******************************
  Main
  *******************************/

int main ( int argc, char ** argv )
{
  /* Initialize the i18n stuff */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  gnome_init (APPNAME, VERSION, argc, argv);
  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-who.png");

  w_command = gnome_is_program_in_path("w");
  if (w_command == NULL) {
    GtkWidget * dialog;
    dialog = 
      gnome_message_box_new(_("Couldn't find the \"w\" command in your path.\n"
                              "This command is needed for the program to work."),
                            GNOME_MESSAGE_BOX_ERROR,
                            GNOME_STOCK_BUTTON_CLOSE,
                            NULL);
    gtk_signal_connect(GTK_OBJECT(dialog), "close", 
                       GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
    gtk_widget_show(dialog);
    gtk_main();
    exit(EXIT_FAILURE);
  }

  load_actions();

  prepare_app();

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
	GNOMEUIINFO_MENU_EXIT_ITEM(delete_event_cb,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo settings_menu[] = {
	GNOMEUIINFO_MENU_PREFERENCES_ITEM(preferences_cb,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo main_menu[] = {
	GNOMEUIINFO_MENU_FILE_TREE(file_menu),
	GNOMEUIINFO_MENU_SETTINGS_TREE(settings_menu),
	GNOMEUIINFO_MENU_HELP_TREE(help_menu),
	GNOMEUIINFO_END
};

static void prepare_app(void)
{
  GtkWidget * app_box;
  GtkWidget * clist;
  GtkWidget * sw;
  GtkWidget * reset_button;

  app = gnome_app_new( APPNAME, _("Active User Listing") ); 

  gnome_app_create_menus(GNOME_APP(app), main_menu);

  app_box = gtk_vbox_new ( FALSE, GNOME_PAD );
  gtk_container_border_width(GTK_CONTAINER(app_box), GNOME_PAD);

  gnome_app_set_contents ( GNOME_APP(app), app_box );

  gtk_signal_connect ( GTK_OBJECT(app), "destroy",
                       GTK_SIGNAL_FUNC(gtk_main_quit),
                       0 );

  gtk_signal_connect ( GTK_OBJECT (app), "delete_event",
                       GTK_SIGNAL_FUNC (delete_event_cb),
                       NULL );
 
  reset_button = gtk_button_new_with_label(_("Update Information"));
  gtk_box_pack_start(GTK_BOX(app_box), reset_button, 
                     FALSE, FALSE, GNOME_PAD_SMALL);

  get_list_column_number ();
  clist = gtk_clist_new(num_columns);
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (sw), clist);
  main_clist = GTK_CLIST(clist);

  gtk_widget_set_events(clist, GDK_BUTTON_PRESS_MASK);

  gtk_clist_set_shadow_type(GTK_CLIST(clist), GTK_SHADOW_IN);
  gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_BROWSE);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_clist_column_titles_show(GTK_CLIST(clist));

  gtk_box_pack_start(GTK_BOX(app_box), sw, 
                     TRUE, TRUE, GNOME_PAD);


  gtk_signal_connect_object(GTK_OBJECT(reset_button), "clicked",
                            GTK_SIGNAL_FUNC(reset_list), GTK_OBJECT(clist));

  gtk_signal_connect(GTK_OBJECT(clist), "button_press_event",
                     GTK_SIGNAL_FUNC(list_clicked_cb), NULL);

  make_actions_popup();  
  reset_list(GTK_CLIST(clist));

  gtk_widget_show_all(app);
  
  /* This is probably just a hack to get around a problem in one of */
  /* the other functions that causes the columns not to be sized by */
  /* the labels, but the data instead. */
    
  reset_list(GTK_CLIST(clist));
}

static void popup_about(void)
{
  static GtkWidget * ga = NULL;
  static const char * authors[] = { "Havoc Pennington <hp@pobox.com>",
                                    NULL };

  if (ga != NULL)
  {
  	gdk_window_show (ga->window);
  	gdk_window_raise (ga->window);
	return;
  }
  ga = gnome_about_new (APPNAME,
                        VERSION, 
                        COPYRIGHT_NOTICE,
                        authors,
                        "",
                        NULL);
  gtk_signal_connect (GTK_OBJECT (ga), "destroy",
		      GTK_SIGNAL_FUNC (gtk_widget_destroyed), &ga);
  
  gtk_widget_show(ga);
}

/**********************************
  reset_list(), runs w and puts the info in the list.
  *******************************/

#define WHITESPACE " \t\r\n"

/* Adapted from reset_list below */
static void 
get_list_column_number (void)
{
  gchar * token;
  gint col = 0;
  static const gint bufsize = 255;
  gchar buffer[256]; /* For a single line */
  FILE * f;
  gchar * returned;

  /* Start getting "w" information */
  memset(buffer, '\0', sizeof(buffer));

  f = popen(w_command, "r");

  if ( f != NULL ) {

    /* The first line on Linux and Solaris is uname garbage. Skip it.
       (How portable this is, I have no idea.) */
    returned = fgets(buffer, bufsize, f);

    /* Grab the second line, and use it to find column number. */
    returned = fgets(buffer, bufsize, f);

    if ( returned == NULL ) {
      /* FIXME error handling */
    }

    token = strtok(buffer, WHITESPACE);
    while ( token ) {
      ++col;
      token = strtok(NULL, WHITESPACE);
    }

    num_columns = col;

  }

  fclose (f);

}


static void reset_list(GtkCList * list)
{
  gchar * token;
  gint col;
  gchar ** row_text; 
  static const gint bufsize = 255;
  gchar buffer[256]; /* For a single line */
  FILE * f;
  gchar * returned;
  gint total_width;
  gint optimal_width;
   
  row_text = (gchar **) g_malloc (sizeof (gchar *) * num_columns); 

  gtk_clist_freeze(list);  

  /* Put in invalid values, so we know if they aren't found. */
  tty_column = -1; name_column = -1;

  /* Start getting "w" information */
  memset(buffer, '\0', sizeof(buffer));

  f = popen(w_command, "r");

  if ( f != NULL ) {

    /* The first line on Linux and Solaris is uname garbage. Skip it.
       (How portable this is, I have no idea.) */
    returned = fgets(buffer, bufsize, f);

    /* Grab the second line, and use it to create column titles. */
    returned = fgets(buffer, bufsize, f);

    if ( returned == NULL ) {
      /* FIXME error handling */
    }

    col = 0;
    token = strtok(buffer, WHITESPACE);
    while ( token && (col < num_columns) ) {
      gtk_clist_set_column_title(list, col, token);
      
      /* See if this is the username or tty column.
         Again, maybe not portable.*/
      if ( strcasecmp(token, "user") == 0 ) {
        name_column = col;
      }
      else if ( strcasecmp(token, "tty") == 0 ) {
        tty_column = col;
      }

      ++col;
      token = strtok(NULL, WHITESPACE);
    }

    gtk_clist_clear(list); /* No errors so far, so clear the old stuff */

    /* Now parse each line of w's output and put each line in a row. */
    returned = fgets(buffer, bufsize, f);
    while ( returned != NULL ) {

      /* Have a line, parse it. */
      col = 0;
      token = strtok(buffer, WHITESPACE);
      while ( (col < num_columns) ) {
        row_text[col] = token;

        token = strtok(NULL, WHITESPACE);
        ++col;
      }
          
      gtk_clist_append(list, (gchar **)row_text);

      returned = fgets(buffer, bufsize, f);
    } 

    fclose(f);
        
    for (col=0; col < num_columns; col++)
    {
      optimal_width = gtk_clist_optimal_column_width (list, col);
      gtk_clist_set_column_width (list, col, optimal_width);
    }                           
    
    total_width = gtk_clist_columns_autosize(list);

    /* set total widget size to the sum of all column widths, 
       with sanity check. */
    gtk_widget_set_usize(GTK_WIDGET(list), CLAMP(total_width, 0, 600), 400);
    /* Make column titles non-interactive. Seems to be needed on 
       each reset. */
    gtk_clist_column_titles_passive(list);

  }

  gtk_clist_thaw(list);

  g_free (row_text);
}


/************************************
  Callbacks 
  *********************************/

static gint list_clicked_cb(GtkCList * list, GdkEventButton * e)
{
  if (actions_popup == NULL) return FALSE;

  if (e->button == 1) {
    /* Ignore button 1 */
    return FALSE; 
  }

  /* don't change the CList selection. */
  gtk_signal_emit_stop_by_name (GTK_OBJECT (list), "button_press_event");

  popup_x = e->x; popup_y = e->y; /* gdouble to gint; ? */

  gtk_menu_popup(actions_popup, NULL, NULL, NULL,
                 NULL, e->button, time(NULL));
  return TRUE; 
}

static gint delete_event_cb(GtkWidget * w, gpointer data)
{
  gtk_main_quit();
  return TRUE; /* hmm, look up what's supposed to be here. */
}

static void about_cb(GtkWidget * w, gpointer data)
{
  popup_about();
}

static void preferences_cb(GtkWidget *w, gpointer data)
{
  static GtkWidget *pb = NULL;
  GtkWidget *sw;
  GtkWidget * frame, * frame_vbox, * page_vbox, 
    * bottom_hbox, * entry_hbox, * button_shrink_vbox;
  GtkWidget * defaults_button, * add_button, * delete_button;
  GtkWidget * name_entry;
  GtkWidget * command_entry;
  GtkWidget * list;
  GtkWidget * explanation_label, * defaults_label;
  const gchar *titles[2];
  GList * tmp; 
  Action * a;
  gchar * text[2];
  gint row;

  if (pb != NULL)
  {
  	gdk_window_show (pb->window);
	gdk_window_raise (pb->window);
	return;
  }

  /* Create everything. */
  titles[0] = _("Name");
  titles[1] = _("Command Line");

  defaults_button = gtk_button_new_with_label(_("Add some defaults"));
  add_button = gtk_button_new_with_label(_("Add"));
  delete_button = gtk_button_new_with_label(_("Delete"));

  name_entry = gtk_entry_new();
  command_entry = gtk_entry_new();

  list = gtk_clist_new_with_titles(2, (gchar **)titles);
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (sw), list);
  gtk_clist_set_shadow_type(GTK_CLIST(list), GTK_SHADOW_IN);
  gtk_clist_set_selection_mode(GTK_CLIST(list), GTK_SELECTION_BROWSE);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (sw),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_clist_set_column_width(GTK_CLIST(list), 0, 130); /* Otherwise it's 0 */
  gtk_clist_column_titles_show(GTK_CLIST(list));
  gtk_clist_column_titles_passive(GTK_CLIST(list));
  gtk_widget_set_usize(list, -1, 230);

  frame = gtk_frame_new(_("Command Editor"));
  gtk_container_border_width(GTK_CONTAINER(frame), GNOME_PAD);

  frame_vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
  page_vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
  bottom_hbox = gtk_hbox_new(FALSE, GNOME_PAD);
  entry_hbox = gtk_hbox_new(FALSE, GNOME_PAD);
  button_shrink_vbox = gtk_vbox_new(FALSE, 0);

  explanation_label = 
    gtk_label_new(_("You can configure the commands on the popup menus.\n"
                    "Just enter the name you want to appear on the \n"
                    "menu, and the command to execute.\n"
                    "In the command, you can use %u to represent the\n"
                    "currently selected username, and %t for the tty."));

  defaults_label =
    gtk_label_new(_("Click this button to add\n"
                    "some sample commands."));

  pb = gnome_property_box_new();
  
  /* Some assembly required */

  gnome_property_box_append_page(GNOME_PROPERTY_BOX(pb), page_vbox,
                                 gtk_label_new(_("Menu items")));

  gtk_box_pack_start(GTK_BOX(page_vbox), frame, TRUE, TRUE, GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(page_vbox), bottom_hbox, FALSE, FALSE, GNOME_PAD);
  gtk_box_pack_start(GTK_BOX(bottom_hbox), explanation_label, 
                     FALSE, FALSE, GNOME_PAD);

  gtk_box_pack_start(GTK_BOX(button_shrink_vbox), defaults_label, 
                     FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(button_shrink_vbox), defaults_button, 
                   FALSE, FALSE, 0);

  gtk_box_pack_end(GTK_BOX(bottom_hbox), button_shrink_vbox, 
                   FALSE, FALSE, GNOME_PAD);

  gtk_container_add(GTK_CONTAINER(frame), frame_vbox);
  gtk_container_border_width(GTK_CONTAINER(frame_vbox), GNOME_PAD);

  gtk_box_pack_start(GTK_BOX(frame_vbox), entry_hbox, TRUE, TRUE, GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(frame_vbox), sw, TRUE, TRUE, GNOME_PAD_SMALL);


  gtk_box_pack_start(GTK_BOX(entry_hbox), gtk_label_new(_("Name")),
                     FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(entry_hbox), name_entry, TRUE, TRUE, GNOME_PAD_SMALL);
  gtk_box_pack_end(GTK_BOX(entry_hbox), delete_button, FALSE, FALSE, GNOME_PAD_SMALL);  
  gtk_box_pack_end(GTK_BOX(entry_hbox), add_button, FALSE, FALSE, GNOME_PAD_SMALL);  
  gtk_box_pack_end(GTK_BOX(entry_hbox), command_entry, TRUE, TRUE, GNOME_PAD_SMALL);
  gtk_box_pack_end(GTK_BOX(entry_hbox), gtk_label_new(_("Command")),
                   FALSE, FALSE, 0);

  /* Put current actions in the list. */
  
  tmp = actions;
  while (tmp) {
    a = (Action *)tmp->data;
    text[0] = a->key; text[1] = a->format;
    gtk_clist_append(GTK_CLIST(list), text);
    tmp = tmp->next;
  }

  /* Unselect the whole CList */
  row = 0;
  while ( row < GTK_CLIST(list)->rows) {
    gtk_clist_unselect_row(GTK_CLIST(list), row, 0);
    ++row;
  }

  /* Since there's no selection, make the entries insensitive. */
  gtk_widget_set_sensitive(name_entry, FALSE);
  gtk_widget_set_sensitive(command_entry, FALSE);


  /* Connect signals to make something happen. */

  gtk_object_set_data(GTK_OBJECT(list), NAME_ENTRY_KEY, name_entry);
  gtk_object_set_data(GTK_OBJECT(list), COMMAND_ENTRY_KEY, command_entry);
  gtk_object_set_data(GTK_OBJECT(list), PROPERTY_BOX_KEY, pb);

  gtk_signal_connect(GTK_OBJECT(add_button), "clicked",
                     GTK_SIGNAL_FUNC(add_action_cb), list);

  gtk_signal_connect(GTK_OBJECT(delete_button), "clicked",
                     GTK_SIGNAL_FUNC(delete_action_cb), list);

  gtk_signal_connect(GTK_OBJECT(defaults_button), "clicked",
                     GTK_SIGNAL_FUNC(add_defaults_cb), list);
    
  gtk_signal_connect(GTK_OBJECT(list), "select_row",
                     GTK_SIGNAL_FUNC(clist_selected_cb), NULL);

  gtk_signal_connect(GTK_OBJECT(list), "unselect_row",
                     GTK_SIGNAL_FUNC(clist_unselected_cb), NULL);

  gtk_signal_connect(GTK_OBJECT(pb), "apply",
                     GTK_SIGNAL_FUNC(apply_prefs_cb), list);

  gtk_signal_connect(GTK_OBJECT(pb), "destroy",
		     GTK_SIGNAL_FUNC(gtk_widget_destroyed), &pb);

  gtk_signal_connect(GTK_OBJECT(pb), "help",
		     GTK_SIGNAL_FUNC(help_prefs_cb), &pb);

  gtk_signal_connect_object(GTK_OBJECT(name_entry), "changed",
                            GTK_SIGNAL_FUNC(gnome_property_box_changed),
                            GTK_OBJECT(pb));

  gtk_signal_connect_object(GTK_OBJECT(command_entry), "changed",
                            GTK_SIGNAL_FUNC(gnome_property_box_changed),
                            GTK_OBJECT(pb));

  /* Show */
  gtk_widget_show_all(pb);
}

/*********************************
  Preferences dialog callbacks and functions 
  ********************************/

/* I think this mess of set_data and callbacks might be cleaner
   if I just used some globals. */

static void new_action_in_prefs(GtkCList * list, gchar * name, gchar * command)
{
  const gchar * text[2];
  gint new_row;
  GnomePropertyBox * pb;

  text[0] = name;
  text[1] = command;

  new_row = gtk_clist_append(list, (gchar **)text);

  gtk_clist_select_row(list, new_row, 0);

  pb = GNOME_PROPERTY_BOX(gtk_object_get_data(GTK_OBJECT(list), 
                                              PROPERTY_BOX_KEY));
  gnome_property_box_changed(pb);
}

static void add_action_cb(GtkButton * button, gpointer clist)
{ 
  new_action_in_prefs(GTK_CLIST(clist), "", "");
}

static void add_defaults_cb(GtkButton * button, GtkCList * clist)
{
  gint i = 0;

  gtk_clist_freeze(clist);

  while ( i < num_defaults ) {
    new_action_in_prefs(clist, default_actions[i].key, 
                        default_actions[i].format);
    ++i;
  }
  gtk_clist_thaw(clist);
}

static void delete_action_cb(GtkButton * button, gpointer clist)
{
  GtkEntry * name_entry, * command_entry;
  GtkCList * c = GTK_CLIST(clist);
  GnomePropertyBox * pb;

  if ( c->selection != NULL ) {
    name_entry = gtk_object_get_data(GTK_OBJECT(clist), 
                                     NAME_ENTRY_KEY);
    command_entry = gtk_object_get_data(GTK_OBJECT(clist), 
                                        COMMAND_ENTRY_KEY);
    
    gtk_entry_set_text(name_entry, "");
    gtk_entry_set_text(command_entry, "");

    gtk_clist_remove(c, GPOINTER_TO_INT(c->selection->data));

    pb = GNOME_PROPERTY_BOX(gtk_object_get_data(GTK_OBJECT(clist), 
                                                PROPERTY_BOX_KEY));
    gnome_property_box_changed(pb);
  }
}

/* Cheat a little; this dialog is so complex! */
static guint name_connection;
static guint command_connection;

static void clist_selected_cb(GtkCList * clist, gint row, 
                              gint col, GdkEventButton * event,
                              gpointer add_button)
{
  GtkEntry * name_entry, * command_entry;
  gchar * name, * command;

  /* clist is evil */
  if (row >= clist->rows)
	  return;

  name_entry = gtk_object_get_data(GTK_OBJECT(clist), 
                                   NAME_ENTRY_KEY);
  command_entry = gtk_object_get_data(GTK_OBJECT(clist), 
                                      COMMAND_ENTRY_KEY);

  gtk_clist_get_text(clist, row, 0, &name);
  gtk_clist_get_text(clist, row, 1, &command);

  gtk_entry_set_text(name_entry, name);
  gtk_entry_set_text(command_entry, command);

  name_connection = 
    gtk_signal_connect(GTK_OBJECT(name_entry), "changed",
                       GTK_SIGNAL_FUNC(name_changed_cb),
                       clist);
  command_connection = 
    gtk_signal_connect(GTK_OBJECT(command_entry), "changed",
                       GTK_SIGNAL_FUNC(command_changed_cb),
                       clist);

  gtk_widget_set_sensitive(GTK_WIDGET(name_entry), TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(command_entry), TRUE);

  if ( ! gtk_clist_row_is_visible(clist, row) ) {
    gtk_clist_moveto(clist, row, -1, 1.0, 0.0);
  }
}

static void name_changed_cb(GtkEntry * entry, GtkCList * clist)
{
  gchar * name;
  gint row;

  /* no selection */
  if (clist->selection == NULL)
	  return;

  name = gtk_entry_get_text(entry);

  row = GPOINTER_TO_INT(clist->selection->data);

  gtk_clist_set_text(clist, row, 0, name);
}

static void command_changed_cb(GtkEntry * entry, GtkCList * clist)
{
  gchar * command;
  gint row;

  /* no selection */
  if (clist->selection == NULL)
	  return;

  command = gtk_entry_get_text(entry);
  
  row = GPOINTER_TO_INT(clist->selection->data);

  gtk_clist_set_text(clist, row, 1, command);
}

static void clist_unselected_cb(GtkCList * clist, gint row,
                                gint col, GdkEventButton * event)
{
  GtkEntry * name_entry, * command_entry;

  name_entry = gtk_object_get_data(GTK_OBJECT(clist), 
                                   NAME_ENTRY_KEY);
  command_entry = gtk_object_get_data(GTK_OBJECT(clist), 
                                      COMMAND_ENTRY_KEY);
  
  gtk_signal_disconnect(GTK_OBJECT(name_entry), name_connection);
  gtk_signal_disconnect(GTK_OBJECT(command_entry), command_connection);

  if ( GTK_CLIST(clist)->selection == NULL ) {
    gtk_widget_set_sensitive(GTK_WIDGET(name_entry), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(command_entry), FALSE);
  }
}

static void apply_prefs_cb(GnomePropertyBox * pb, gint page, GtkCList * clist)
{
  gint row = 0;
  gchar * name, * command;

  if ( page != -1 ) return; /* Only act on global apply */

  clear_actions();

  while (row < clist->rows) {
    gtk_clist_get_text(clist, row, 0, &name);
    gtk_clist_get_text(clist, row, 1, &command);
    prepend_action(name, command);
    ++row;
  }

  save_actions();
  make_actions_popup();
}

void
help_prefs_cb (GnomePropertyBox *property_box, gint page_num)
{
  GnomeHelpMenuEntry help_entry_prefs = { "gw", "prefs.html" };
  gnome_help_display ( NULL, &help_entry_prefs);
}      

/*********************************************
  Non-GUI
  ***********************/

static void load_actions(void)
{
  void * config_iterator;
  Action * a;
  gchar * name, * command;

  config_iterator = gnome_config_init_iterator("/"APPNAME"/"ACTIONS_SECTION);

  if (!config_iterator)
    return;

  while ((config_iterator = gnome_config_iterator_next(config_iterator, &name, &command))) {
    a = g_new(Action, 1);
    a->key = name; a->format = command;
    actions = g_list_prepend(actions, a);
  }
}

/* Just clears the loaded list, not gnome_config */
static void clear_actions(void)
{
  GList * tmp;
  Action * a;

  tmp = actions;

  while ( tmp ) {
    a = (Action *) tmp->data;
    g_free(a->key);
    g_free(a->format);
    g_free(a);
    tmp = tmp->next;
  }
  actions = NULL;
}

static void prepend_action(const gchar * key, const gchar * format)
{
  Action * a;

  a = g_new(Action, 1);
  a->key = g_strdup(key);
  a->format = g_strdup(format);

  actions = g_list_prepend(actions, a);
}

static void save_actions(void)
{
  Action * a;
  GList * tmp;

  gnome_config_clean_section("/"APPNAME"/"ACTIONS_SECTION);
  gnome_config_push_prefix("/"APPNAME"/"ACTIONS_SECTION"/");

  tmp = actions;

  while (tmp) {  
    a = (Action *)tmp->data;
    gnome_config_set_string(a->key, a->format);
    tmp = tmp->next;
  }

  gnome_config_sync();
  gnome_config_pop_prefix();
}

static void make_actions_popup(void)
{
  GtkWidget * mi;
  GList * tmp;
  Action * a;

  if (actions_popup) {
    gtk_widget_destroy(GTK_WIDGET(actions_popup));
    actions_popup = NULL;
  }
  
  if ( actions == NULL ) {
    return; /* Don't make a menu, no actions */
  }

  actions_popup = GTK_MENU(gtk_menu_new());

  tmp = actions;

  while (tmp) {
    a = (Action *)tmp->data;
    
    mi = gtk_menu_item_new_with_label(a->key);
    gtk_signal_connect(GTK_OBJECT(mi), "activate",
                       GTK_SIGNAL_FUNC(do_action_cb), a);
    gtk_menu_append(actions_popup, mi);
    gtk_widget_show(mi);
 
    tmp = tmp->next;
  }
}

static void do_action(gchar * format, gchar * name, gchar * tty)
{
  gchar * i = g_strdup(format);
  gchar * sn_format;
  gchar * strings[2]; /* Stuff to go in the format */
  gint current_string = 0;
  gchar * full_command;
  gint command_length;

  strings[0] = NULL; strings[1] = NULL;
  sn_format = i;

  while (*i) {
    if ( *i == '%' ) {
      if ( (*(i - 1)) == '\\' ) {
        ++i;
        continue; /* escaped, ignore it. */
      }
      ++i;
      if ( *i == 'u' ) {
        /* Username */
        *i = 's'; /* sprintf format */
        strings[current_string] = name;
        ++current_string;
      }
      else if ( *i == 't' ) {
        /* tty */
        *i = 's'; 
        strings[current_string] = tty;
        ++current_string;
      }
    } /* if *i == '%' */
    ++i;
  }

  command_length = strlen(format) + ( strings[0] ? strlen(strings[0]) : 0 )
    + ( strings[1] ? strlen(strings[1]) : 0 ) + 5;

  full_command = g_malloc(command_length);

  if ( strings[0] && strings[1] ) {
    g_snprintf(full_command, command_length, sn_format, 
               strings[0], strings[1]);
  }
  else if ( strings[0] ) {
    g_snprintf(full_command, command_length, sn_format, strings[0]);
  }
  else {
    strncpy(full_command, sn_format, command_length);
  }

  g_free(sn_format);

#ifdef GNOME_ENABLE_DEBUG
  g_print(_("Running: %s\n"),full_command);
#endif
  gnome_execute_shell(NULL, full_command);
}

static void do_action_cb(GtkWidget * menuitem, Action * a)
{
  gint row, col; /* col isn't actually used for now */
  gchar * name = NULL;
  gchar * tty = NULL;

  gtk_clist_get_selection_info(main_clist,
                               popup_x,
                               popup_y,
                               &row, &col);

  if (name_column != -1) {
    gtk_clist_get_text(main_clist, row, name_column, &name);
  }
  if (tty_column != -1) {
    gtk_clist_get_text(main_clist, row, tty_column, &tty);
  }

  do_action(a->format, name, tty);
}
