/* gnome-stones - cave.h
 *
 * Time-stamp: <1998/11/01 22:10:29 carsten>
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
#ifndef CAVE_H
#define CAVE_H

#include <config.h>
#include <gnome.h>
#include "types.h"


/*****************************************************************************/
/* Gnome-Stones signal stuff.  */


GStonesSignal
gstones_signal (const gchar *sig_name);



/*****************************************************************************/

/* The following signals are used by default:
 *
 * > cave.pre_scan
 *
 *   This signal is emitted just before scanning the hole cave.  It's
 *   emitted by the cave_iterate function.
 *
 * > cave.post_scan
 *
 *   This signal is emitted by the cave_iterate function just after
 *   scanning the hole cave.
 *
 * > player.start
 *
 *   This signal is emitted when calling 'cave_start'.
 *
 * > player.die
 *
 *   This signal is emitted when calling 'cave_player_die'.  
 *
 */


/*****************************************************************************/
/* Cave declarations.  */


GStonesCave   *cave_new               (void);
void           cave_free              (GStonesCave *cave);

/* Object management.  */

gboolean cave_add_object       (GStonesCave *cave, GStonesObject *object);
void     cave_remove_object    (GStonesCave *cave, GStonesObject *object);
gboolean cave_object_removable (GStonesCave *cave, GStonesObject *object);


void           cave_set_player        (GStonesCave *cave, 
				       GStonesPlayer *player);

/* Calculate the cave's next state.

   The next function iterates the cave's state.*/

void           cave_iterate           (GStonesCave *cave,
				       gint         x_direction,
				       gint         y_direction,
				       gboolean     push);

void           cave_toggle_pause_mode (GStonesCave *cave);

/* Let our little gnome die.  You really don't want this to happen, do
   you?  */

void           cave_player_die        (GStonesCave *cave);

/* The following function starts the cave by replacing the entrance
   with our gnome.  */

void           cave_start             (GStonesCave *cave);


void           cave_emit_signal       (GStonesCave *cave, GStonesSignal sig);



guint          cave_time_to_frames    (GStonesCave *cave, gdouble time);


/* Cave entry stuff.  */
gboolean       cave_set_entry         (GStonesCave   *cave, 
				       guint          x, 
				       guint          y, 
				       GStonesObject *object,
				       guint          state);


GdkPixmap     *cave_get_image         (GStonesCave *cave, guint x, guint y);

GdkImlibImage *cave_get_imlib_image   (GStonesCave *cave, guint x, guint y);


/* Object options stuff.  */

gchar         *cave_get_string_object_option (GStonesCave   *cave, 
					      GStonesObject *object,
					      const gchar   *name);
gboolean       cave_get_bool_object_option   (GStonesCave   *cave, 
					      GStonesObject *object,
					      const gchar   *name);
gint           cave_get_int_object_option    (GStonesCave   *cave,
					      GStonesObject *object,
					      const gchar   *name);
void           cave_set_object_option        (GStonesCave   *cave,
					      GStonesObject *object,
					      const gchar   *name,
					      const gchar   *value);

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
