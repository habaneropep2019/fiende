/* gnome-stones - player.c
 *
 * Time-stamp: <1998/10/15 20:28:48 carsten>
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
#include "player.h"
#include "status.h"

GStonesPlayer *
player_new (GStonesGame *game)
{
  GStonesPlayer *player;

  g_return_val_if_fail (game, NULL);
  
  player= g_malloc (sizeof (GStonesPlayer));
  
  if (player)
    {
      player->new_life_score= game->new_life_score;
      player->max_time      = 1;

      player->score         = 0;
      player->lives         = game->lives;
      player->time          = 0;
    }

  /* Update the statusbar.  */
  status_set_score (player->score);
  status_set_lives (player->lives);

  return player;
}


void
player_free (GStonesPlayer *player)
{
  g_free (player);
}


gboolean
player_inc_score (GStonesPlayer *player, guint inc)
{
  gboolean new_life;

  if (!player) 
    return FALSE;  
  
  player->score+= inc;

  new_life= (player->score % player->new_life_score) < inc;

  if (new_life)
    {
      gnome_app_flash (GNOME_APP (app), _("You gained an extra life!"));
      player_inc_lives (player, 1);
    }

  /* Update the statusbar.  */
  status_set_score (player->score);

  return new_life;
}


void
player_inc_lives (GStonesPlayer *player, gint inc)
{
  if (!player)
    return;
  
  player->lives+= inc;
  status_set_lives (player->lives);
}


void
player_set_diamonds (GStonesPlayer *player, guint diamonds)
{
  if (!player)
    return;
  
  status_set_diamonds (diamonds);
}


void
player_set_time (GStonesPlayer *player, guint time, gboolean countdown)
{
  if (!player)
    return;

  if (countdown && ((player->time/1000) != (time/1000)))
    {
      if (time < 10*1000)
	{	  
	  gchar buffer[64];
	  
	  sprintf (buffer, _("Seconds left: %d"), time/1000+1);
	  gnome_appbar_set_status (GNOME_APPBAR (statusbar), buffer);
	}
    }
  player->time= time;

  status_set_time ((gfloat)player->time/(gfloat)player->max_time);
}

void
player_set_max_time (GStonesPlayer *player, guint time)
{
  if (!player)
    return;

  player->max_time= time;
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
