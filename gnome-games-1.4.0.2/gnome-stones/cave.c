/* gnome-stones - cave.c
 *
 * Time-stamp: <1998/11/09 21:54:11 carsten>
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
#include "cave.h"
#include "player.h"
#include "status.h"
#include "object.h"


extern GList *object_list;

static GStonesSignal signal_option_changed= (GStonesSignal) 0;
static GStonesSignal signal_cave_pre_scan = (GStonesSignal) 0;
static GStonesSignal signal_cave_post_scan= (GStonesSignal) 0;
static GStonesSignal signal_player_die    = (GStonesSignal) 0;
static GStonesSignal signal_player_start  = (GStonesSignal) 0;



/*****************************************************************************/
/* Gnome-Stones signal stuff.  */


GStonesSignal
gstones_signal (const gchar *sig_name)
{
  GStonesSignal         signal;
  static GHashTable    *sig_table= NULL;
  static GStonesSignal  sig_next= 0;
  
  if (!sig_table)
    {
      sig_table= g_hash_table_new (g_str_hash, g_str_equal);
      if (!sig_table)
	return (GStonesSignal) 0;
    }

  signal= GPOINTER_TO_UINT (g_hash_table_lookup (sig_table, sig_name));
  if (signal) return (GStonesSignal) signal;
  
  ++sig_next;
  g_hash_table_insert (sig_table, g_strdup (sig_name), 
		       GUINT_TO_POINTER (sig_next));
  return sig_next;
}


/*****************************************************************************/
/* Additional object management.  */


/* Makes cave require a special object.  */
gboolean
cave_add_object (GStonesCave *cave, GStonesObject *object)
{
  GStonesObjContext *context;
  
  g_return_val_if_fail (cave, FALSE);
  g_return_val_if_fail (object, FALSE);
  
  /* Check, wheather this object has already been added to this cave.
     If so, than don't add it again.  */
  if (g_hash_table_lookup (cave->contexts, object))
    return TRUE;

  /* Create a new context for this object.  */
  context= object_context_new (object);
  if (!context)
    return FALSE;

  g_hash_table_insert (cave->contexts, object, context);

  if (object->description->init_cave_function)
    {
      if (!object->description->init_cave_function (cave, context))
	{
	  /* There has been an error while initializing the object.  */
	  g_hash_table_remove (cave->contexts, object);
	  object_context_free (context);
	  return FALSE;
	}
    }

  return TRUE;
}


void
cave_remove_object (GStonesCave *cave, GStonesObject *object)
{
  GStonesObjContext *context;
  
  g_return_if_fail (cave);
  g_return_if_fail (object);

  context= g_hash_table_lookup (cave->contexts, object);

  if (context)
    {
      if (object->description->destroy_function)
	object->description->destroy_function (cave, context);
      
      g_hash_table_remove (cave->contexts, object);
      
      object_context_free (context);
    }
}


gboolean
cave_object_removable (GStonesCave *cave, GStonesObject *object)
{
  /* This feature is not implemented yet, so we always return 'FALSE'.  */

  return FALSE;
}


static GStonesObjContext *
cave_get_object_context (GStonesCave *cave, GStonesObject *object)
{
  return (GStonesObjContext *) g_hash_table_lookup (cave->contexts, object);
}
 


/*****************************************************************************/
/* Object options stuff.  */

void
cave_set_object_option (      GStonesCave   *cave,
			      GStonesObject *object,
			const gchar         *name,
	                const gchar         *value)
{
  GStonesObjContext *context;

  /* Start checking the parameter.  */
  g_return_if_fail (cave);
  g_return_if_fail (object);
  g_return_if_fail (name);

  /* 'value' must not be checked. Setting a NULL value means, we want
     to unset this option.  */

  context= cave_get_object_context (cave, object);
  g_return_if_fail (context);

  object_context_set_option (context, name, value);

  /* We now send a signal to this object, telling him, that his options
     just changed.  */
  if (object->description->signal_function)
    object->description->signal_function (cave, signal_option_changed, context);
}


gchar *
cave_get_string_object_option (      GStonesCave   *cave,
				     GStonesObject *object,
			       const gchar         *name)
{
  GStonesObjContext *context;

  /* Start checking the parameter.  */
  g_return_val_if_fail (cave, NULL);
  g_return_val_if_fail (object, NULL);
  g_return_val_if_fail (name, NULL);

  context= cave_get_object_context (cave, object);
  g_return_val_if_fail (context, NULL);

  return object_context_get_string_option (context, name);
}


gboolean
cave_get_bool_object_option   (      GStonesCave   *cave, 
			             GStonesObject *object,
			       const gchar         *name)
{
  GStonesObjContext *context;

  /* Start checking the parameter.  */
  g_return_val_if_fail (cave, FALSE);
  g_return_val_if_fail (object, FALSE);
  g_return_val_if_fail (name, FALSE);

  context= cave_get_object_context (cave, object);
  g_return_val_if_fail (context, FALSE);

  return object_context_get_bool_option (context, name);
}

gint
cave_get_int_object_option   (      GStonesCave   *cave, 
			            GStonesObject *object,
			      const gchar         *name)
{
  GStonesObjContext *context;

  /* Start checking the parameter.  */
  g_return_val_if_fail (cave, 0);
  g_return_val_if_fail (object, 0);
  g_return_val_if_fail (name, 0);

  context= cave_get_object_context (cave, object);
  g_return_val_if_fail (context, 0);

  return object_context_get_int_option (context, name);
}


/*****************************************************************************/


GStonesCave *
cave_new (void)
{
  GStonesObject *frame= NULL;
  GStonesCave   *cave = NULL;
  guint x;
  guint y;
  
  /* If there are no signals defined yet, this is the right time to do
     so.  */
  if (!signal_cave_pre_scan)
    {
      signal_option_changed= gstones_signal ("option.changed");
      signal_cave_pre_scan = gstones_signal ("cave.pre_scan");
      signal_cave_post_scan= gstones_signal ("cave.post_scan");
      signal_player_die    = gstones_signal ("player.die");
      signal_player_start  = gstones_signal ("player.start");
    }
  
  frame= object_find_object_by_name ("default:frame");
  g_return_val_if_fail (frame, NULL);

  cave= g_new0 (GStonesCave, 1);
  g_return_val_if_fail (cave, NULL);

  cave->is_intermission    = FALSE;

  cave->contexts           = g_hash_table_new (g_direct_hash, g_direct_equal);

  /* First we clear the hole cave.  */
  for (y= 0 ; y <= CAVE_MAX_HEIGHT+1; y++)
    for (x= 0 ; x <= CAVE_MAX_WIDTH+1; x++)
      {
	cave->entry[x][y].object = frame;
	cave->entry[x][y].scanned= FALSE;
      }

  return cave;
}


void
cave_free (GStonesCave *cave)
{
  if (cave)
    {
      g_free (cave->name);
      g_free (cave->next);
      
      g_free (cave->message);
      
      /* FIXME: objects must be freed.  */
      
      g_free (cave);
    }
}


/*****************************************************************************/


gboolean
cave_set_entry (GStonesCave   *cave, 
		guint          x, 
		guint          y, 
		GStonesObject *object,
		guint          state)
{
  
  register GStonesCaveEntry *entry= &cave->entry[x][y];
  
  entry->object= object;
  entry->state = state;

  return TRUE;
}



/*****************************************************************************/


void
cave_set_player (GStonesCave *cave, GStonesPlayer *player)
{
  g_return_if_fail (cave);
  
  cave->player= player;

  player_set_diamonds (cave->player, MAX (cave->diamonds_needed-
					  cave->diamonds_collected, 0));

  player_set_max_time (cave->player, cave->level_time);
  player_set_time     (cave->player, cave->timer, TRUE);
}



/*****************************************************************************/


void
cave_player_die (GStonesCave *cave)
{
  cave_emit_signal (cave, signal_player_die);
}



/*****************************************************************************/


void
cave_iterate (GStonesCave *cave,
	      gint         x_direction,
	      gint         y_direction,
	      gboolean     push)
{
  guint x;
  guint y;

  g_return_if_fail (cave != NULL);
  
  cave->frame++;

  if (cave->flags & CAVE_PAUSING)
    return;

  /* Decrement timer.  */
  if ((cave->timer > 0) && (cave->flags & CAVE_PLAYER_EXISTS))
    {
      cave->timer= cave->timer-cave->frame_rate;
      if (cave->timer <= 0)
	{
	  cave_player_die (cave);
	  cave->timer= 0;
	}

      player_set_time (cave->player, cave->timer, TRUE);
    }
  
  cave->player_x_direction= x_direction;
  cave->player_y_direction= y_direction;
  cave->player_push       = push;

  cave_emit_signal (cave, signal_cave_pre_scan);

  /* Now we iterate through the hole cave.  */
  for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
    for (x= CAVE_MAX_WIDTH ; x >= 1; x--)
      if (!cave->entry[x][y].scanned && 
	  (cave->entry[x][y].object->description->scan_function))
	cave->entry[x][y].object->description->scan_function 
	  (cave, x, y, cave_get_object_context (cave, cave->entry[x][y].object));
  
  /* Remove the scanned flag on every entry.  */
  for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
    for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
      cave->entry[x][y].scanned= FALSE;

  cave_emit_signal (cave, signal_cave_post_scan);
}

void
cave_toggle_pause_mode (GStonesCave *cave)
{
  cave->flags^= CAVE_PAUSING;
}



/* The following function starts the cave by replacing the entrance
   with our gnome.  */

void
cave_start (GStonesCave *cave)
{
  /* We should only start, if the cave it still in the intro state,
     that means: ~PLAYER_EXISTS and ~FINISHED.  */

  if (!(cave->flags & CAVE_PLAYER_EXISTS) && !(cave->flags & CAVE_FINISHED))
    {
      cave_emit_signal (cave, signal_player_start);

      cave->flags|= CAVE_PLAYER_EXISTS;
    }
}



/*****************************************************************************/
/* Sends a signal to every registered object.  */


typedef struct _EmitData EmitData;

struct _EmitData
{
  GStonesCave   *cave;
  GStonesSignal  signal;
};


static void
cave_emit_signal0 (GStonesObject     *object, 
		   GStonesObjContext *context, 
		   EmitData *emit)
{
  if (object->description->signal_function)
    object->description->signal_function (emit->cave, emit->signal, context);
}


void
cave_emit_signal (GStonesCave *cave, GStonesSignal signal)
{
  EmitData emit;
  
  g_return_if_fail (cave);

  emit.cave  = cave;
  emit.signal= signal;

  g_hash_table_foreach (cave->contexts, (GHFunc) cave_emit_signal0, &emit);  
}



/*****************************************************************************/


guint
cave_time_to_frames (GStonesCave *cave, gdouble time)
{
  return (time*1000.0)/cave->frame_rate;
}


static gint
cave_get_image_index (GStonesCave *cave, guint x, guint y)
{
  register GStonesObject *object= cave->entry[x][y].object;

  /* Determine the image index, that belongs to the objects state.  */
  if (object->description->animation_function)
    return object->description->animation_function 
      (cave, x, y, cave_get_object_context (cave, object));
  else
    return OBJECT_DEFAULT_IMAGE;
}


GdkPixmap *
cave_get_image (GStonesCave *cave, guint x, guint y)
{
  g_return_val_if_fail (cave != NULL, NULL);
  
  /* Return the right GdkPixmap belonging to index 'idx'.  */
  return object_get_image (cave->entry[x][y].object, 
			   cave_get_image_index (cave, x, y));
}


GdkImlibImage *
cave_get_imlib_image (GStonesCave *cave, guint x, guint y)
{
  g_return_val_if_fail (cave != NULL, NULL);
  
  /* Return the right GdkPixmap belonging to index 'idx'.  */
  return object_get_imlib_image (cave->entry[x][y].object, 
				 cave_get_image_index (cave, x, y));
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
