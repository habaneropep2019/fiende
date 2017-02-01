/* gnome-stones - player.h
 *
 * Time-stamp: <1998/10/15 20:32:10 carsten>
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
#ifndef PLAYER_H
#define PLAYER_H

#include <config.h>
#include <gnome.h>
#include "types.h"


GStonesPlayer *player_new          (GStonesGame   *game);
void           player_free         (GStonesPlayer *player);


/* The player gained a new life, if 'player_inc_score' returns 'TRUE'.  */

gboolean       player_inc_score    (GStonesPlayer *player, guint inc);
void           player_inc_lives    (GStonesPlayer *player, gint inc);

void           player_set_diamonds (GStonesPlayer *player, guint diamonds);
void           player_set_time     (GStonesPlayer *player, 
				    guint          time,
				    gboolean       countdown);
void           player_set_max_time (GStonesPlayer *player, guint time);

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
