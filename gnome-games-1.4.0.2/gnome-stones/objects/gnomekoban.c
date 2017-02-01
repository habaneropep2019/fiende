/* gnome-stones - objects/gnomekoban.c
 *
 * Time-stamp: <1999/04/21 12:33:53 carsten>
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

/*****************************************************************************/
/* Stone definitions */


static GStonesObject *OBJECT_FRAME;
static GStonesObject *OBJECT_EMPTY;
static GStonesObject *OBJECT_WALL;
static GStonesObject *OBJECT_DIRT;
static GStonesObject *OBJECT_CRATE;
static GStonesObject *OBJECT_GNOME;
static GStonesObject *OBJECT_ENTRANCE;

static GStonesSignal SIGNAL_PLAYER_EXTRALIFE;
static GStonesSignal SIGNAL_DOOR_OPEN;
static GStonesSignal SIGNAL_CAVE_PRE_SCAN;
static GStonesSignal SIGNAL_CAVE_POST_SCAN;
static GStonesSignal SIGNAL_PLAYER_DIE;
static GStonesSignal SIGNAL_PLAYER_START;


/*****************************************************************************/

static void
free_private_data (GStonesCave *cave, GStonesObjContext *context)
{
  g_free (object_context_private_data (context));
}



/*****************************************************************************/
/* EMPTY stuff */


typedef struct _EmptyData EmptyData;

struct _EmptyData
{
  /* Dynamic data.  */
  guint open_door_animation;
  guint extra_life_animation;
};


static gboolean
empty_init_cave (GStonesCave *cave, GStonesObjContext *context)
{
  EmptyData *empty;
  
  empty= g_new0 (EmptyData, 1);
  if (!empty)
    return FALSE;
  
  object_context_set_private_data (context, empty);
  
  return TRUE;
}


static guint
empty_animate (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  EmptyData *empty= (EmptyData *) object_context_private_data (context);
  
  if (cave->entry[x][y].state > 0)
    {
      guint f= cave->frame % 16;
      
      if (f > 8)
	return 16-f;
      else
	return f;
    }
  if (empty->open_door_animation)
    return empty->open_door_animation;
  else if (empty->extra_life_animation)
    return 9+random () % 4;
  else
    return 0;
}


static void
empty_signals (GStonesCave *cave, GStonesSignal signal, 
	       GStonesObjContext *context)
{
  EmptyData *empty= (EmptyData *) object_context_private_data (context); 
  
  if (signal == SIGNAL_CAVE_PRE_SCAN)
    {
      if (empty->open_door_animation)
	empty->open_door_animation--;
      
      if (empty->extra_life_animation)
	empty->extra_life_animation--;
    }
  else if (signal == SIGNAL_DOOR_OPEN)
    {
      empty->open_door_animation= 3;
    }
  else if (signal == SIGNAL_PLAYER_EXTRALIFE)
    {
      empty->extra_life_animation= 10;
    } 
}


static GStonesObjectDesc empty_object=
{
  "empty",
  empty_init_cave,
  free_private_data,
  
  NULL,
  empty_signals,
  
  "background.png",
  empty_animate,
  NULL,
  0, 0
};



/*****************************************************************************/
/* CRATE stuff */


static void
crate_signals (GStonesCave *cave, GStonesSignal signal, 
	       GStonesObjContext *context)
{
  guint x;
  guint y;

  if (signal == SIGNAL_CAVE_POST_SCAN)
    {
      gboolean finished= TRUE;
      
      for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
	for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
	  if ((cave->entry[x][y].object == OBJECT_CRATE) &&
	      (cave->entry[x][y].state == 0))
	    finished= FALSE;
      
      if (finished)
	cave->flags|= CAVE_FINISHED;
    }
}


static GStonesObjectDesc crate_object=
{
  "crate",

  NULL,
  NULL,

  NULL,
  crate_signals,
  
  "crate.png",
  NULL,
  NULL,
  0, 0
};



/*****************************************************************************/
/* ENTRANCE stuff */


static void
entrance_scanned (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  if (cave->entry[x][y].state > 0)
    {
      if (cave->entry[x][y].state == 3)
	{
	  cave->entry[x][y].object= OBJECT_GNOME;
	  cave->entry[x][y].state = 0;
	}
      else
	cave->entry[x][y].state++;
    }
}

static void
entrance_signals (GStonesCave *cave, GStonesSignal signal, GStonesObjContext *context)
{  
  guint x;
  guint y;

  if (signal == SIGNAL_PLAYER_START)
    {
      for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
	for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
	  if (cave->entry[x][y].object == OBJECT_ENTRANCE)
	    cave->entry[x][y].state= 1;
    }
}


static guint
entrance_animate (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  if (cave->entry[x][y].state == 0)
    return (cave->frame % 4)/2;
  else
    return cave->entry[x][y].state+1;
}


static GStonesObjectDesc entrance_object=
{
  "entrance",

  NULL,
  NULL,

  entrance_scanned,
  entrance_signals,

  "door.png",
  entrance_animate,
  NULL,
  0, 0
};



/*****************************************************************************/
/* GNOME stuff */


static gboolean
gnome_init_cave (GStonesCave *cave, GStonesObjContext *context)
{
  guint x;
  guint y;

  for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
    for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
      if (cave->entry[x][y].object == OBJECT_ENTRANCE)
	{
	  cave->player_x= x;
	  cave->player_y= y;
	}

  return TRUE;
}

/* The internal gnome states:
   0: standing still an waiting.
   1: pushing right
   2: pushing left
   3: walk right
   4: walk left
*/


static void
gnome_scanned (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  gint state= cave->entry[x][y].state << 4;
  
  if ((cave->player_x_direction != 0) || 
      (cave->player_y_direction != 0))
    {
      gboolean moved= FALSE;
      
      guint xn= x+cave->player_x_direction;
      guint yn= y+cave->player_y_direction;
      
      GStonesObject *type= cave->entry[xn][yn].object;

      if (type == OBJECT_EMPTY)
	{
	  if (cave->player_x_direction > 0)
	    state= 3;
	  else if (cave->player_x_direction < 0)
	    state= 4;

	  moved= TRUE;
	}
      else
	{
	  if (cave->player_x_direction > 0)
	    state= 1;
	  else if (cave->player_x_direction < 0)
	    state= 2;
	  
	  if ((type == OBJECT_CRATE) &&
	      (cave->entry[xn+cave->player_x_direction][yn+cave->player_y_direction].object == OBJECT_EMPTY))
	    { 
	      moved= TRUE;
		  
	      cave->entry[xn+cave->player_x_direction][yn+cave->player_y_direction].object= type;
	    }
	}
      
      if (moved)
	{
	  if (cave->player_push)
	    {
	      cave->entry[xn][yn].object= OBJECT_EMPTY;
	    }
	  else
	    {
	      cave->entry[xn][yn].object = OBJECT_GNOME;
	      cave->entry[xn][yn].scanned= TRUE;
	      cave->entry[x][y].object   = OBJECT_EMPTY;
	      cave->entry[x][y].state    = cave->entry[x][y].state & 0xf;
	      
	      cave->player_x= xn;
	      cave->player_y= yn;

	      x= xn;
	      y= yn;
	    }
	}
    }
  cave->entry[x][y].state= (cave->entry[x][y].state & 0xf)+(state << 4);
}


void
gnome_signals (GStonesCave *cave, GStonesSignal signal, GStonesObjContext *context)
{
  if (signal == SIGNAL_PLAYER_DIE)
    {
      if (cave->flags &  CAVE_PLAYER_EXISTS)
	{
	  /* explosion_new (cave, cave->player_x, cave->player_y, TRUE); */
	  cave->flags&= ~CAVE_PLAYER_EXISTS;
	  cave->flags|= CAVE_FINISHED;
	}
    }
}


typedef struct _GnomeAnimState    GnomeAnimState;
typedef struct _GnomeAnimSequence GnomeAnimSequence;

struct _GnomeAnimState
{
  guint sequence:8;
  guint offset  :8;
};

struct _GnomeAnimSequence
{
  guint probability;
  guint start;
  guint length;
  /* The probability, that this sequence will be played again directly
     after finishing.  The value must in a range of [0...100].  */
  guint repeat_prob;
};

GnomeAnimSequence gnome_anim[]=
{
  { 15, 0, 4, 20 },
  { 10, 4, 4, 20 }
};

static guint
gnome_animate (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  register guint idx  = 0;
  register gint  state= cave->entry[x][y].state >> 4;
  GnomeAnimState anim = *((GnomeAnimState*) &cave->entry[x][y].anim_state);
  
  switch (state)
    {
    case 0:
      {
	if (anim.sequence == 0)
	  {
	    int rnd= random () % 100;
	    
	    for (anim.sequence = sizeof (gnome_anim)/sizeof (gnome_anim[1]);
		 anim.sequence > 0; anim.sequence--)
	      {
		if (gnome_anim[anim.sequence-1].probability > rnd)
		  break;
		else
		  rnd-= gnome_anim[anim.sequence-1].probability;
	      }
	    anim.offset= 0;
	  }

	if (anim.sequence)
	  {
	    idx= gnome_anim [anim.sequence-1].start+anim.offset;

	    anim.offset++;
	    if (anim.offset >= gnome_anim [anim.sequence-1].length)
	      {
		anim.offset= 0;

		if ((random () % 100) >=
		    gnome_anim [anim.sequence-1].repeat_prob)
		  anim.sequence= 0;
	      }
	  }
	else
	  idx= 0;
      }
      break;

    case 1:
    case 2:
      anim.sequence= 0;
      idx= 8*state+cave->frame % 8;
      break;
      
    case 3:
    case 4:
      anim.sequence= 0;
      idx= 12+4*state+cave->frame % 4;
      break;

    default:
      anim.sequence= 0;
      idx= 0;
      break;
    }
  cave->entry[x][y].anim_state= *((guint*) &anim); 
  return idx;
}


static GStonesObjectDesc gnome_object=
{
  "gnome",
  
  gnome_init_cave,
  NULL,

  gnome_scanned,
  gnome_signals,

  "gnome.png",
  gnome_animate,
  NULL,
  0, 0
};


/*****************************************************************************/
/* Additional stuff.  */

static GStonesObjectDesc dirt_object=
{
  "dirt",
  NULL,
  NULL,
  
  NULL,
  NULL,
  
  "dirt.png",
  NULL,
  NULL,
  0, 0
};

static GStonesObjectDesc wall_object=
{
  "wall",
  NULL,
  NULL,
  
  NULL,
  NULL,
  
  "wall.png",
  NULL,
  NULL,
  0, 0
};
 
/*****************************************************************************/
/* Register all objects.  */

gchar *
objects_init (GStonesPlugin *plugin)
{
  OBJECT_FRAME       = object_find_object_by_name ("default:frame");
  OBJECT_EMPTY       = object_register (plugin, &empty_object);
  OBJECT_WALL        = object_register (plugin, &wall_object);
  OBJECT_DIRT        = object_register (plugin, &dirt_object);
  OBJECT_CRATE       = object_register (plugin, &crate_object);
  OBJECT_GNOME       = object_register (plugin, &gnome_object);
  OBJECT_ENTRANCE    = object_register (plugin, &entrance_object);

  SIGNAL_PLAYER_EXTRALIFE= gstones_signal ("player.extralife");
  SIGNAL_CAVE_PRE_SCAN   = gstones_signal ("cave.pre_scan");
  SIGNAL_CAVE_POST_SCAN  = gstones_signal ("cave.post_scan");
  SIGNAL_PLAYER_DIE      = gstones_signal ("player.die");
  SIGNAL_PLAYER_START    = gstones_signal ("player.start");

  return "gnomekoban";
}




/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
