/* gnome-stones - game.c
 *
 * Time-stamp: <1998/11/21 23:53:18 carsten>
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
#include <string.h>
#include "game.h"
#include "cave.h"
#include "object.h"

GStonesGame*
game_new (void)
{
  GStonesGame *game;
  
  game= g_new0 (GStonesGame, 1);
  if (!game) return NULL;
  
  return game;
}


void
game_free (GStonesGame *game)
{
  if (!game)
    return;

  g_free (game->title);
  
  g_free (game->filename);
  g_free (game->config_prefix);

  /* Remove list of caves.  */
  g_list_foreach (game->caves, (GFunc) g_free, NULL);
  g_list_free (game->caves);
  
  /* Remove list of start_caves.  */
  g_list_foreach (game->start_caves, (GFunc) g_free, NULL);
  g_list_free (game->start_caves);
  
  /* Remove list of plugins.  We only stored references to plugins, so
     the plugins themself don't need to get freed.  Same with the
     objects.  */
  g_list_free (game->plugins);
  g_list_free (game->objects);

  /* FIXME: Free translation_table.  */

  g_free (game);
}


/*****************************************************************************/

/* A helper function to game_add_plugin.  */

static void
game_add_object (char           *name,
		 GStonesObject  *object,
		 GList         **object_list)
{
  (*object_list)= g_list_append (*object_list, object);
}


/* Make a game require a plugin.  The plugin will be added to the
   game's plugin list.  Additionally all objects, that are exported by
   the plugin are added to the game's object list.  */

gboolean
game_add_plugin (GStonesGame *game, GStonesPlugin *plugin)
{
  g_return_val_if_fail (game, FALSE);
  g_return_val_if_fail (plugin, FALSE);
  
  /* Check if this plugin is already required by this game.  If so, we
   have nothing to do.*/
  if (g_list_find (game->plugins, plugin))
    return TRUE;

  /* Add this plugin to the list of required plugins.  */
  game->plugins= g_list_append (game->plugins, plugin);

  /* Finally we have to add all objects, that this plugin exports to the
     game's list of objects.  */
  g_hash_table_foreach (plugin->objects, (GHFunc) game_add_object, 
			&game->objects);

  return TRUE;
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
