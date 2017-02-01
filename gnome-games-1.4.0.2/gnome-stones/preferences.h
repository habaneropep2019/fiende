/* gnome-stones - preferences.h
 *
 * Time-stamp: <1999/03/02 18:48:54 carsten>
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
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <config.h>
#include <gnome.h>
#include "types.h"


/*****************************************************************************/
/* Global Variables */

/* The default game, that should be loaded, if no client state is to
   be restored.  If this variables value is NULL, than 'default.cave'
   will be used instead.  */

extern gchar *default_game;

/* This variable specifies the currently played game.  If 'game' is
   equal to 'NULL', than no game is loaded.  */

extern GStonesGame *game;

/* The currently played cave.  This cave must be a cave, that belongs
   to the game 'game'.  */

extern GStonesCave *cave;

/* The data about the player.  */

extern GStonesPlayer *player;

/* You may start a game in different cavs.  'start_cave' decides, in
   which cave the player will start.  */

extern guint start_cave;

/* If you use a joystick as input device, this variable holds the
   device's id.  Setting it to GDK_CORE_POINTER disables the Joystick
   support.  */

extern guint32  joystick_deviceid;
extern gfloat   joystick_switch_level;

/* The game can be in different states.  These state decides, how to
   react if some events occur.  */

typedef enum
{
  STATE_STARTUP,
  STATE_TITLE,
  STATE_CURTAIN,
  STATE_WAITING_TO_START,
  STATE_RUNNING,
  STATE_COUNTDOWN,
} GameState;

extern GameState state;



/*****************************************************************************/


/* Scans a game directory for game files and adds them to an internal
   game list.  */

void
game_directory_scan (const char *directory);



/*****************************************************************************/


/* The following opens the preferences dialog box.  */

void
preferences_dialog_show (void);


/* Save preferences.  */

void
preferences_save_global (void);


/* Restores the preferences from disc.  */

gboolean
preferences_restore (void);



/*****************************************************************************/


/* Initialize the session management stuff.  */

void
session_management_init (void);



/*****************************************************************************/


#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
