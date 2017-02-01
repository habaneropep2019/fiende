/* gnome-stones - preferences.h
 *
 * Time-stamp: <1999/03/03 18:17:52 carsten>
 *
 * Copyright (C) 1998 Carsten Schaar
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "preferences.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>


/*****************************************************************************/
/* Global Variables */

/* The default game, that should be loaded, if no client state is to
   be restored.  If this variables value is NULL, than 'default.cave'
   will be used instead.  */

gchar *default_game= NULL;

/* This variable specifies the currently played game.  If 'game' is
   equal to 'NULL', than no game is loaded.  */

GStonesGame *game= NULL;

/* The currently played cave.  This cave must be a cave, that belongs
   to the game 'game'.  */

GStonesCave *cave= NULL;

/* The data about the player.  */

GStonesPlayer *player= NULL;

/* You may start a game in different cavs.  'start_cave' decides, in
   which cave the player will start.  */

guint start_cave= 0;

/* If you use a joystick as input device, this variable holds the
   device's id.  Setting it to GDK_CORE_POINTER disables the Joystick
   support.  */

guint32  joystick_deviceid    = GDK_CORE_POINTER;
gfloat   joystick_switch_level= 0.5;

/* The game can be in different states.  These state decides, how to
   react if some events occur.  */

GameState state= STATE_TITLE;


/* The preferences dialog.  */
GtkWidget *preferences_dialog= NULL;




/****************************************************************************/
/* Some extern stuff, that should be removed soon.  */

void joystick_set_properties (guint32 deviceid, gfloat switch_level);

void load_game (const char *filename, guint cave);


/****************************************************************************/
/* Stuff for managing the list of avaiable games.  */


typedef struct _GameFile GameFile;

struct _GameFile
{
  gchar *filename;
  gchar *gametitle;
  guint  caves;
};


GList *games= NULL;


static gint
compare_game_names (const GameFile *file, const char *name)
{
  return strcmp (file->filename, name);
}


static GameFile *
add_game (const char *filename)
{
  char     *prefix= NULL;
  GList    *tmp;
  GameFile *file;
  
  /* Maybe this game is already in the games list.  */
  tmp= g_list_find_custom (games, (gpointer) filename, 
			   (GCompareFunc) compare_game_names);
  if (tmp) return (GameFile *) tmp->data;
  
  /* FIXME: add some checks, if it's realy a Gnome-Stones game.  */
  file= g_malloc (sizeof (GameFile));
  
  if (file)
    {
      file->filename = g_strdup (filename);

      prefix= g_strconcat ("=", filename, "=/", NULL);
      gnome_config_push_prefix (prefix);
      
      file->gametitle= gnome_config_get_translated_string ("General/Title");
      file->caves    = gnome_config_get_int               ("Caves/Number");

      gnome_config_pop_prefix ();
      g_free (prefix);
      
      games= g_list_append (games, file);
    }

  return file;
}


void
game_directory_scan (const char *directory)
{
  DIR *dir;
  
  dir= opendir (directory);
  if (dir)
    {
      struct dirent *entry;
      
      while ((entry = readdir (dir)) != NULL)
	{
	  char *filename= g_strconcat (directory, "/", entry->d_name, NULL);
	  struct stat sbuf;
	  
	  if ((stat (filename, &sbuf)== 0) && S_ISREG (sbuf.st_mode))
	    add_game (filename);

	  g_free (filename);
	}
      
      closedir (dir);
    }
}


static void
load_game_by_number (guint n)
{
  GList *tmp= g_list_nth (games, n);

  g_return_if_fail (tmp != NULL);
  
  load_game (((GameFile *) tmp->data)->filename, 0);
}


gboolean
load_game_by_name (const char *filename, guint cave)
{
  GameFile *file= add_game (filename);
  
  if (file)
    {
      load_game (file->filename, cave);
      
      return TRUE;
    }
  
  return FALSE;
}



/*****************************************************************************/
/* Save preferences.  */


void 
preferences_save (gboolean global)
{
  gchar    *devicename= NULL;
  GList    *devices;

  gnome_config_clean_section ("Preferences");

  for (devices= gdk_input_list_devices (); devices; devices= devices->next)
    {
      GdkDeviceInfo *info = (GdkDeviceInfo *) devices->data;
      
      if (joystick_deviceid == info->deviceid)
	{
	  devicename= info->name;
	  break;
	}
    }
  
  if (devicename)
    gnome_config_set_string ("Preferences/Joystick device",  devicename);
  gnome_config_set_float ("Preferences/Joytick switch level", 
			  joystick_switch_level);

  if (game)
    {
      gnome_config_set_string ("Preferences/Game", game->filename);
      gnome_config_set_int ("Preferences/Start cave", start_cave);
    }
  
  gnome_config_clean_section ("Windows");

  if (!global)
    {
      /* Information about open windows is only stored, if a session
         manager issued a save.  */
      gnome_config_set_bool ("Windows/Preferences", 
			     preferences_dialog != NULL);
    }
  

  gnome_config_sync ();
}


void
preferences_save_global (void)
{
  gnome_config_push_prefix 
    (gnome_client_get_global_config_prefix (gnome_master_client ()));

  preferences_save (TRUE);

  gnome_config_pop_prefix ();
  gnome_config_sync ();
}


gint
preferences_save_local (GnomeClient        *client,
			gint                phase,
			GnomeSaveStyle      save_style,
			gint                shutdown,
			GnomeInteractStyle  interact_style,   
			gpointer            client_data)
{
  gchar *prefix= gnome_client_get_config_prefix (client);
  gchar *argv[3]= {"rm", "-r", NULL };
      
  gnome_config_push_prefix (prefix);
    
  preferences_save (FALSE);

  gnome_config_pop_prefix ();
  gnome_config_sync ();

  argv[2]= gnome_config_get_real_path (prefix);
  gnome_client_set_discard_command (client, 3, argv);

  return TRUE;
}



/*****************************************************************************/
/* Restoring the preferences from disc.  */


gboolean
preferences_restore (void)
{
  GnomeClient *client= gnome_master_client ();
  gchar       *devicename;
  char        *filename;
  gboolean     def;
  guint        cave;

  gnome_config_push_prefix (gnome_client_get_config_prefix (client));

  devicename= gnome_config_get_string ("Preferences/Joystick device=");
  if (devicename)
    {
      GList *devices;
      
      for (devices= gdk_input_list_devices (); devices; devices= devices->next)
	{
	  GdkDeviceInfo *info = (GdkDeviceInfo *) devices->data;
	  
	  if (strcmp (info->name, devicename) == 0)
	    {
	      joystick_deviceid= info->deviceid;
	      break;
	    }
	}      
      g_free (devicename);
    }
  if (joystick_deviceid != GDK_CORE_POINTER)
    gdk_input_set_mode (joystick_deviceid, GDK_MODE_SCREEN);


  joystick_switch_level= 
    gnome_config_get_float ("Preferences/Joytick switch level=0.5");


  filename= gnome_config_get_string_with_default ("Preferences/Game", &def);

  cave= gnome_config_get_int ("Preferences/Start cave=0");
  if (!default_game || !load_game_by_name (default_game, 0))
    {
      if (def || !load_game_by_name (filename, cave))
	{
	  load_game_by_name (CAVESDIR"/default.caves", 0);
	}
    }

  g_free (filename);

  if (gnome_config_get_bool ("Windows/Preferences=0"))
    {
      preferences_dialog_show ();
    }

  gnome_config_pop_prefix ();

  return TRUE;
}



/****************************************************************************/
/* Preferences dialog stuff.  */

typedef struct _PreferencesData PreferencesData;

struct _PreferencesData
{
  /* General information.  */
  GnomePropertyBox *property_box;
  
  /* Page one.  */
  GtkWidget        *game_list;
  gint              selected_game;

  /* Page two. */
  GtkWidget        *level_frame;
  
  guint32           joystick_deviceid;
  gfloat            joystick_switch_level;
};


static void
preferences_apply_cb (GtkWidget *w, gint page, gpointer data)
{
  PreferencesData *prdata= (PreferencesData *) data;

  g_return_if_fail (prdata != NULL);

  switch (page)
    {
    case 0:
      /* FIXME: Add some checks and warnings here.  */
      if (prdata->selected_game > -1)
	load_game_by_number (prdata->selected_game);
      break;

    case 1:
      joystick_set_properties (prdata->joystick_deviceid,
			       prdata->joystick_switch_level);
      break;
    default:
      /* After setting all needed values, we can save the programs
         state to disc.  */
      preferences_save_global ();
      break;
    }
}


static gint
preferences_destroy_cb (GtkWidget *w, gpointer data)
{
  g_free (data);

  preferences_dialog= NULL;

  return FALSE;
}


static void
preferences_changed_cb (GtkWidget *w, gpointer data)
{
  PreferencesData *prdata= (PreferencesData *) data;
  
  g_return_if_fail (prdata != NULL);

  gnome_property_box_changed (prdata->property_box);
}


static void 
game_selector_select_row (GtkCList * clist,
			  gint row, gint column,
			  GdkEvent * event, gpointer data)
{
  PreferencesData *prdata= (PreferencesData *) data;
  
  g_return_if_fail (prdata != NULL);

  preferences_changed_cb (GTK_WIDGET (clist), data);

  prdata->selected_game= row;
}


/* The joystick callbacks.  */

static void
preferences_set_joystick_device (GtkWidget *widget, gpointer data)
{
  guint32          deviceid= GPOINTER_TO_UINT(data);
  PreferencesData *prdata  = 
    (PreferencesData *) gtk_object_get_user_data (GTK_OBJECT (widget));

  prdata->joystick_deviceid= deviceid;
  
  if (deviceid == GDK_CORE_POINTER)
    {
      gtk_widget_set_sensitive (prdata->level_frame, FALSE);
    }
  else
    {
      gtk_widget_set_sensitive (prdata->level_frame, TRUE);
    }

  gnome_property_box_changed (prdata->property_box);
}


static void
preferences_set_joystick_switch_level (GtkAdjustment *adjust, gpointer data)
{
  PreferencesData *prdata  = 
    (PreferencesData *) gtk_object_get_user_data (GTK_OBJECT (adjust));

  prdata->joystick_switch_level= adjust->value;

  gnome_property_box_changed (prdata->property_box);
}


static GtkWidget *
preferences_dialog_new (void)
{
  GtkWidget *propbox;
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *list;
  GtkWidget *scrolled;

  PreferencesData *prdata;

  prdata= g_malloc (sizeof (PreferencesData));

  propbox= gnome_property_box_new ();
  prdata->property_box= GNOME_PROPERTY_BOX (propbox);
  
  gtk_window_set_title (GTK_WINDOW(&GNOME_PROPERTY_BOX(propbox)->dialog.window),
			_("Gnome-Stones Preferences"));
  gtk_window_set_wmclass (GTK_WINDOW(&GNOME_PROPERTY_BOX(propbox)->dialog.window),
			  "gnome-stones", "preferences");
  

  /* The first page of our preferences dialog. */
  box= gtk_vbox_new (FALSE, GNOME_PAD);
  gtk_container_set_border_width (GTK_CONTAINER (box), GNOME_PAD_SMALL);

  /* The list of game names.  */
  list= gtk_clist_new (3);
  prdata->game_list= list;

  gtk_clist_set_column_title (GTK_CLIST (list), 0, _("Game title"));
  gtk_clist_set_column_width (GTK_CLIST (list), 0, 250);
  gtk_clist_set_column_title (GTK_CLIST (list), 1, _("Caves"));
  gtk_clist_set_column_width (GTK_CLIST (list), 1, 50);
  gtk_clist_set_column_justification (GTK_CLIST (list), 1, GTK_JUSTIFY_RIGHT);
  gtk_clist_set_column_title (GTK_CLIST (list), 2, _("Filename"));
  gtk_clist_column_titles_passive (GTK_CLIST (list));
  gtk_clist_column_titles_show (GTK_CLIST (list));
  gtk_widget_set_usize (list, -2, 200);
  
  gtk_clist_set_selection_mode (GTK_CLIST (list), GTK_SELECTION_SINGLE);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_ALWAYS,
                                  GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled), list);
  gtk_box_pack_start (GTK_BOX (box), scrolled, FALSE, FALSE, GNOME_PAD_SMALL);

  gtk_clist_freeze (GTK_CLIST (list));
  prdata->selected_game= -1;
  {
    GList *tmp= games;
    
    while (tmp)
      {
	char buffer[10];
	GameFile *file= (GameFile *)tmp->data;
	char     *entry[3];
	gint      row;
	
	entry[0]= file->gametitle;
	sprintf (buffer, "%d", file->caves);
	entry[1]= buffer;
	entry[2]= g_basename (file->filename);
	
	row= gtk_clist_append (GTK_CLIST (list), entry);

	if (game && strcmp (file->filename, game->filename) == 0)
	  {
	    gtk_clist_select_row (GTK_CLIST (list), row, 0);
	    prdata->selected_game= row;
	  }

	tmp= tmp->next;
      }
  }
  gtk_clist_thaw (GTK_CLIST (list));
  gtk_signal_connect (GTK_OBJECT (list), "select_row", 
		      GTK_SIGNAL_FUNC (game_selector_select_row),
		      prdata);

  gtk_widget_show (list);
  gtk_widget_show (scrolled);
  gtk_widget_show (box);

  label= gtk_label_new (_("Game"));
  gtk_notebook_append_page (GTK_NOTEBOOK 
			    (GNOME_PROPERTY_BOX (propbox)->notebook), 
			    box, label);

  /* The second page of our preferences dialog. */

  prdata->joystick_deviceid    = joystick_deviceid;
  prdata->joystick_switch_level= joystick_switch_level;

  box= gtk_vbox_new (FALSE, GNOME_PAD);
  gtk_container_set_border_width (GTK_CONTAINER (box), GNOME_PAD_SMALL);
  
  {
    guint      i;
    GtkObject *adjust;
    GtkWidget *frame;
    GtkWidget *scale;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *menuitem;
    GtkWidget *optionmenu;
    GtkWidget *device_menu;
    GList     *devices;

    frame= gtk_frame_new (_("Device"));
    gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, GNOME_PAD_SMALL);
    gtk_widget_show (frame);

    vbox= gtk_vbox_new (FALSE, GNOME_PAD);
    gtk_container_add (GTK_CONTAINER (frame), vbox);
    gtk_widget_show (vbox);

    hbox= gtk_hbox_new (FALSE, GNOME_PAD);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, GNOME_PAD_SMALL);
    gtk_widget_show (hbox);

    label= gtk_label_new (_("Joystick device:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, GNOME_PAD_SMALL);
    gtk_widget_show (label);
    
    device_menu= gtk_menu_new ();

    /* We definatly have a "disable" entry.  */
    menuitem= gtk_menu_item_new_with_label (_("disabled"));
    gtk_object_set_user_data (GTK_OBJECT (menuitem), prdata);
    gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			(GtkSignalFunc) preferences_set_joystick_device,
			GUINT_TO_POINTER (GDK_CORE_POINTER));
    gtk_menu_append (GTK_MENU (device_menu), menuitem);
    gtk_widget_show (menuitem);
    
    for (devices= gdk_input_list_devices (), i= 1; devices; 
	 devices= devices->next, i++)
      {
	GdkDeviceInfo *info = (GdkDeviceInfo *) devices->data;

	if (info->deviceid != GDK_CORE_POINTER)
	  {
	    menuitem= gtk_menu_item_new_with_label (info->name);

	    gtk_object_set_user_data (GTK_OBJECT (menuitem), prdata);
            gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
                                (GtkSignalFunc) preferences_set_joystick_device,
                                GUINT_TO_POINTER (info->deviceid));

	    gtk_menu_append (GTK_MENU (device_menu), menuitem);
	    gtk_widget_show (menuitem);
	  }

	if (info->deviceid == prdata->joystick_deviceid)
	  gtk_menu_set_active (GTK_MENU (device_menu), i);
      }
    
    optionmenu= gtk_option_menu_new ();
    gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu), device_menu);
    gtk_box_pack_start (GTK_BOX (hbox), optionmenu, FALSE, FALSE, 2);
    gtk_widget_show (optionmenu);
    
    gtk_widget_show (label);
    gtk_widget_show (hbox);
    gtk_widget_show (optionmenu);

    frame= gtk_frame_new (_("Digital joystick emulation"));
    gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, GNOME_PAD_SMALL);
    gtk_widget_show (frame);

    vbox= gtk_vbox_new (FALSE, GNOME_PAD);
    gtk_container_add (GTK_CONTAINER (frame), vbox);
    gtk_widget_show (vbox);

    hbox= gtk_hbox_new (FALSE, GNOME_PAD);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, GNOME_PAD_SMALL);
    gtk_widget_show (hbox);

    label= gtk_label_new (_("Switch level:"));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, GNOME_PAD_SMALL);
    gtk_widget_show (label);

    adjust= gtk_adjustment_new (prdata->joystick_switch_level,
				0.0, 1.0, 0.02, 0.1, 0.0);
    gtk_object_set_user_data (adjust, prdata);
    gtk_signal_connect (adjust, "value_changed",
			(GtkSignalFunc) preferences_set_joystick_switch_level,
			NULL);
    
    scale= gtk_hscale_new (GTK_ADJUSTMENT (adjust));
    gtk_scale_set_digits (GTK_SCALE (scale), 2);
    gtk_box_pack_start (GTK_BOX (hbox), scale, FALSE, FALSE, GNOME_PAD_SMALL);
    gtk_widget_show (scale);
    
    if (prdata->joystick_deviceid == GDK_CORE_POINTER)
      {
	gtk_widget_set_sensitive (GTK_WIDGET (frame), FALSE);
      }

    prdata->level_frame= frame;
  }
  
  gtk_widget_show (box);

  label= gtk_label_new (_("Joystick"));
  gtk_notebook_append_page (GTK_NOTEBOOK 
			    (GNOME_PROPERTY_BOX (propbox)->notebook), 
			    box, label);

  /* The third page of our preferences dialog. */
  box= gtk_vbox_new (FALSE, GNOME_PAD);
  gtk_container_set_border_width (GTK_CONTAINER (box), GNOME_PAD_SMALL);

  label= gtk_label_new (_("Not yet implemented!"));
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, FALSE, GNOME_PAD_SMALL);

  gtk_widget_show (label);
  gtk_widget_show (box);

  label= gtk_label_new (_("Sound"));
  gtk_notebook_append_page (GTK_NOTEBOOK 
			    (GNOME_PROPERTY_BOX (propbox)->notebook), 
			    box, label);

  gtk_signal_connect (GTK_OBJECT (propbox), "destroy",
		      GTK_SIGNAL_FUNC (preferences_destroy_cb), prdata);
  gtk_signal_connect (GTK_OBJECT (propbox), "apply",
		      GTK_SIGNAL_FUNC (preferences_apply_cb), prdata);
  return propbox;
}


void
preferences_dialog_show (void)
{
  if (!preferences_dialog)
    {
      preferences_dialog= preferences_dialog_new ();
    }
  
  gtk_widget_show (preferences_dialog);
}



/****************************************************************************/
/* Initialize the session management stuff.  */


/* FIXME: should move to main.c.  */
void
gstones_exit (GnomeClient *client, gpointer client_data)
{
  exit (0);
}


void
session_management_init (void)
{
  GnomeClient *client= gnome_master_client ();
  
  gtk_signal_connect (GTK_OBJECT (client), "save_yourself",
		      GTK_SIGNAL_FUNC (preferences_save_local), 
		      GINT_TO_POINTER (FALSE));
  gtk_signal_connect (GTK_OBJECT (client), "die",
		      GTK_SIGNAL_FUNC (gstones_exit), NULL);
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
