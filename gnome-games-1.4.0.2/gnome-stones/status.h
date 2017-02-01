/* gnome-stones - status.h
 *
 * Time-stamp: <1998/10/15 20:32:27 carsten>
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
#ifndef STATUS_H
#define STATUS_H

#include <config.h>
#include <gnome.h>



/*****************************************************************************/


extern GtkWidget *statusbar;
extern GtkWidget *app;


/*****************************************************************************/


typedef enum
{
  STATUS_GAME_INFO,
  STATUS_CAVE_INFO
} StatusType;


/* The following function returns the status widget.  */

GtkWidget *
status_widget_get (void);


/* Set the mode of the status line.  We have three different modes:
   
   STATUS_GAME_INFO:
   STATUS_CAVE_INFO:
*/

void
status_set_mode (StatusType type);



/*****************************************************************************/
/* Setting and getting the timer.  */


void
status_set_timer (guint time);

void
status_set_maxtimer (guint time);


/*****************************************************************************/
/* Setting information for the statusbar.  */

void status_set_score    (guint score);
void status_set_lives    (guint lives);
void status_set_diamonds (guint diamonds);
void status_set_cave     (char *cave);
void status_set_time     (gfloat time);

/* Setting text line information.  */

void
status_set_message (const gchar *message);



/*****************************************************************************/

void
gstone_error (const gchar *error);


#endif /* STATUS_H */

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
