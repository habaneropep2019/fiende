/* gnome-stones - io.c
 *
 * Time-stamp: <1999/08/09 18:05:13 robert>
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
#include "io.h"
#include "cave.h"
#include "game.h"



/*****************************************************************************/
/* The following struct is used to convert the ascii codes objects
   into real objects.  */

typedef struct _TranslationEntry    TranslationEntry;

struct _TranslationEntry
{
  GStonesObject *object;
  guint          state;
};


/*****************************************************************************/


static GList *
game_parse_cave_section (GStonesGame *game, const gchar *name)
{
  GList *cavelist= NULL;
  char  *section;
  gint   number;
  gint   n;
  
  /* FIXME: add error handling.  */
  
  section= g_strconcat (game->config_prefix, name, "/", NULL);
  gnome_config_push_prefix (section);

  number= gnome_config_get_int ("Number=-1");
  /* FIXME: Error message, if number is "-1".  */
  
  for (n= 1; n <= number; n++)
    {
      static char buffer[20];
      char *cavename;
      
      sprintf (buffer, "Cave%03d", n);
      cavename= gnome_config_get_string (buffer);

      cavelist= g_list_append (cavelist, cavename);
    }

  gnome_config_pop_prefix ();
  g_free (section);
  
  return cavelist;
}


static gboolean
game_parse_object_section (GStonesGame *game)
{
  /* FIXME: improve error handling.  */

  void     *iter;
  char     *key;
  gint      key_size;
  gboolean  is_default;
  char     *value;

  g_return_val_if_fail (game, FALSE);
  g_return_val_if_fail (!game->translation_table, FALSE);

  /* Create a new translation table.  */
  game->translation_table= g_hash_table_new (g_str_hash, g_str_equal);
  
  gnome_config_push_prefix (game->config_prefix);

  /* The key size is not yet used.  It will be usefull, if we have
     more objects in a game, than we have printable ascii characters.
     In this case, every object will be indexed by two or more
     characters (like in the xpm file format).  */

  key_size= gnome_config_get_int_with_default ("General/Object Size",
					       &is_default);
  if (is_default)
    {
      g_warning ("Error in game file: 'Object size' not specified!");
      gnome_config_pop_prefix ();

      return FALSE;
    }
  else if (key_size != 1)
    {
      g_warning ("Error in game file: 'Object size' not equal to one!");
      gnome_config_pop_prefix ();
      
      return FALSE;
    }
  
  iter= gnome_config_init_iterator ("Objects");
  if (!iter)
    {
      g_warning ("Error in game file: No 'Objects' section defined!");
      gnome_config_pop_prefix ();

      return FALSE;
    }

  while ((iter= gnome_config_iterator_next (iter, &key, &value)) != NULL)
    {
      if (strlen (key) == 1)
	{
	  TranslationEntry *entry;
	  char             *name;
	  char             *state;

	  if (strcmp (key, "_") == 0)
	    key= g_strdup (" ");
	  else
	    key= g_strdup (key);

	  name = strtok (value, ",");
	  state= strtok (NULL, ",");
	  entry= g_new0 (TranslationEntry, 1);

	  entry->object= object_find_object_by_name (name);
	  if (state)
	    entry->state= atoi (state);

	  g_hash_table_insert (game->translation_table, key, entry);
	}
    }

  gnome_config_pop_prefix ();  
  return TRUE;
}


GStonesGame*
gstones_game_load (const gchar *name)
{
  GStonesGame *game;

  game= game_new ();
  g_return_val_if_fail (game != NULL, NULL);
  
  game->filename      = g_strdup (name);
  game->config_prefix = g_strconcat ("=", name, "=/", NULL);
  
  gnome_config_push_prefix (game->config_prefix);

  /* FIXME: Add check, if title is unset. */
  game->title         = gnome_config_get_translated_string ("General/Title");
  game->frame_rate    = gnome_config_get_float ("General/Frame rate=0.2")*1000;
  game->new_life_score= gnome_config_get_int    ("General/New life score=500");
  game->lives         = gnome_config_get_int    ("General/Lives=3");
  
  /* We now determine the plugins, that this game requires.  */
  {
    gchar *token;
    gchar *plugins= gnome_config_get_string ("General/Plugins");

    for (token= strtok (plugins, ":"); token; token= strtok (NULL, ":"))
      {
	GStonesPlugin *plugin= plugin_find_by_title (token);
	
	if (!plugin || !game_add_plugin (game, plugin))
	  break;
      }
    g_free (plugins);

    if (token)
      {
	/* FIXME: Add error handling.  */
	printf ("This game needs a plugin that is not available.\n");
	game_free (game);
	
	return NULL;
      }
  }

  gnome_config_pop_prefix ();

  if (!game_parse_object_section (game))
    {
      game_free (game);
      
      return NULL;
    }
  game->caves      = game_parse_cave_section (game, "Caves");
  game->start_caves= game_parse_cave_section (game, "Start caves");

  return game;  
}



/*****************************************************************************/

static void
cave_load_object_options (GStonesCave *cave, GStonesObject *object)
{
  gchar               *fullname;
  GStonesObjectOption *option= object->description->options;

  if (!option)
    return;

  fullname= object_get_fullname (object);

  /* Now we scan the object's options.  */
  while (option->name)
    {
      gboolean  def;
      gchar    *value;
      gchar    *optionstr;

      /* Build option string.  */
      optionstr= g_strconcat (fullname, ":", option->name, NULL);

      value    = gnome_config_get_string_with_default (optionstr, &def);
      
      if (value)
	{
	  if (!def)
	    {
	      gchar *name= g_strdup (option->name);
	      gchar *tmp= strchr (name, '=');
	      
	      if (tmp)
		{
		  /* Bingo, there was some option written in the
                     config file.  */

		  *tmp='\0';		  
		  cave_set_object_option (cave, object, name, value);
		}
	      
	      g_free (name);
	    }

	  g_free (value);
	  
	}

      g_free (optionstr);

      /* Have a look at the next option.  */
      option++;
    }

  g_free (fullname);
}


GStonesCave *
gstones_cave_load (GStonesGame *game, const gchar *cavename)
{
  gchar       *prefix= NULL;
  GStonesCave *cave= NULL;
  GList       *tmp;
  guint        x;
  guint        y;

  g_return_val_if_fail (game,  NULL);

  if (cavename == NULL) 
    return NULL;
  
  cave= cave_new ();
  g_return_val_if_fail (cave, NULL);

  cave->name         = g_strdup (cavename);
  prefix= g_strconcat (game->config_prefix, cavename, "/", NULL);

  gnome_config_push_prefix (prefix);

  /* Now we load the cave data.  */
  cave->next               = gnome_config_get_string ("Next cave=");
  if (cave->next && (strlen (cave->next) == 0))
    {
      g_free (cave->next);
      cave->next= NULL;
    }
  cave->game               = game;
  cave->width              = gnome_config_get_int ("Width");
  cave->height             = gnome_config_get_int ("Height");
  cave->is_intermission    = gnome_config_get_bool ("Is intermission=false");
  cave->diamond_score      = gnome_config_get_int ("Diamond score=0");
  cave->extra_diamond_score= gnome_config_get_int ("Extra diamond score=0");
  cave->diamonds_needed    = gnome_config_get_int ("Diamonds needed");
  cave->level_time         = gnome_config_get_int ("Time")*1000;
  cave->message            = gnome_config_get_translated_string ("Message");
  if (cave->message && (strlen (cave->message) == 0))
    {
      g_free (cave->message);
      cave->message= NULL;
    }

  cave->frame_rate         = game->frame_rate;

  cave->timer              = gnome_config_get_int ("Time")*1000;

  if ((cave->width > CAVE_MAX_WIDTH) || (cave->height > CAVE_MAX_HEIGHT))
    {
      /* This cave is to big to be played with gnome-stones.  */

      gstone_error (_("The cave you are trying to load it to big for this game."));
      
      gnome_config_pop_prefix ();
      cave_free (cave);
      return NULL;
    }

  /* Now we set the fields according to the loaded cave */
  for (y= 1 ; y <= cave->height; y++)
    {
      char *line;
      char  buffer[8];
      sprintf (buffer, "Line%.2d", y);

      line= gnome_config_get_string (buffer);
      
      for (x= 1 ; x <= MIN (strlen (line), cave->width); x++)
	{
	  TranslationEntry *entry;
	  char key[2]= { '\0', '\0'};
	  
	  key[0]= line[x-1];
	  
	  entry= g_hash_table_lookup (game->translation_table, key);

	  if (entry)
	    {
	      cave_set_entry (cave, x, y, entry->object, entry->state);
	    }
	  else
	    {
	      /* An object was requested in this cave, that was not
		 declared in the game file's Object section.  */
	      
	      gstone_error (_("The cave you are trying to load includes an object, that wasn't declared."));
	      
	      gnome_config_pop_prefix ();
	      cave_free (cave);
	      g_free (line);
	      return NULL;
	    }
	}
      g_free (line);
    }

  /* We have to populate the 'objects' hash table.  */
  tmp= game->objects;
  while (tmp)
    {
      GStonesObject *object= (GStonesObject *) tmp->data;
      
      /* Add this object to the set of required objects.  */
      cave_add_object (cave, object);

      /* Now we have to look, if this object has some options set.  */
      cave_load_object_options (cave, object);

      tmp= tmp->next;
    };  

  gnome_config_pop_prefix ();
  g_free (prefix);

  return cave;
}



/*****************************************************************************/



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
