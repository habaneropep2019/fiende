/* gnome-stones - game.h
 *
 * Time-stamp: <1998/11/01 16:02:26 carsten>
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
#ifndef GAME_H
#define GAME_H

#include <config.h>
#include <gnome.h>
#include "types.h"
#include "object.h"


/*****************************************************************************/
/* Game declarations.  */


GStonesGame* game_new        (void);
void         game_free       (GStonesGame *game);

/* Add a plugin to the list of required plugins.  */
gboolean     game_add_plugin (GStonesGame *game, GStonesPlugin *plugin);


#endif


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
