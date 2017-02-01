/* gnome-stones - objects/gnome-stones.c
 *
 * Time-stamp: <1998/11/09 21:47:08 carsten>
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
static GStonesObject *OBJECT_BOULDER;
static GStonesObject *OBJECT_WALL;
static GStonesObject *OBJECT_GROWING_WALL;
static GStonesObject *OBJECT_MAGIC_WALL;
static GStonesObject *OBJECT_DIRT;
static GStonesObject *OBJECT_DIAMOND;
static GStonesObject *OBJECT_BUTTERFLY;
static GStonesObject *OBJECT_FIREFLY;
static GStonesObject *OBJECT_AMOEBA;
static GStonesObject *OBJECT_EXPLOSION;
static GStonesObject *OBJECT_GNOME;
static GStonesObject *OBJECT_ENTRANCE;
static GStonesObject *OBJECT_EXIT_CLOSED;
static GStonesObject *OBJECT_EXIT_OPENED;

static GStonesSignal SIGNAL_PLAYER_EXTRALIFE;
static GStonesSignal SIGNAL_DOOR_OPEN;
static GStonesSignal SIGNAL_MAGIC_WALL_START;
static GStonesSignal SIGNAL_CAVE_PRE_SCAN;
static GStonesSignal SIGNAL_CAVE_POST_SCAN;
static GStonesSignal SIGNAL_PLAYER_DIE;
static GStonesSignal SIGNAL_PLAYER_START;
static GStonesSignal SIGNAL_OPTION_CHANGED;


gint x_diff[4]={-1,  0,  1,  0};
gint y_diff[4]={ 0,  1,  0, -1};


/*****************************************************************************/
/* Some declarations */


static void
explosion_new (GStonesCave *cave, guint x, guint y, gboolean diamond);



/*****************************************************************************/
/* some animation stuff.  */


static guint
eight_animate (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  return (cave->frame/2) % 8;
}



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
  
  if (empty->open_door_animation)
    return empty->open_door_animation;
  else if (empty->extra_life_animation)
    return 4+random () % 4;
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
  
  "empty.png",
  empty_animate,
  NULL,
  0, 0
};


/*****************************************************************************/
/* AMOEBA stuff */

typedef struct _AmoebaData AmoebaData;

struct _AmoebaData
{
  /* Static data, read from level file.  */
  guint max_size;
  guint slow_time;
  
  /* Dynamic data.  */
  guint size;
  gboolean can_grow;
  gboolean transformed;
};


static gboolean
amoeba_init_cave (GStonesCave *cave, GStonesObjContext *context)
{
  AmoebaData *amoeba;
  
  amoeba= g_new0 (AmoebaData, 1);
  if (!amoeba)
    return FALSE;

  /* Preset with some default values.  */
  amoeba->max_size = object_context_get_int_option (context, "maxSize");  
  amoeba->slow_time= 0;
  
  amoeba->slow_time= cave_time_to_frames 
    (cave, object_context_get_float_option (context, "slowTime"));
  amoeba->can_grow = TRUE;
  
  object_context_set_private_data (context, amoeba);
  

  return TRUE;
}


static void
amoeba_scanned (GStonesCave *cave, guint x, guint y, 
		GStonesObjContext *context)
{
  AmoebaData *amoeba= (AmoebaData *) object_context_private_data (context);
  guint m;
  
  /* This should not happen.  */
  if (!amoeba->size) return;

  /* This amoeba grew to big.  */
  if (amoeba->size >= amoeba->max_size)
    {
      cave_set_entry (cave, x, y, OBJECT_BOULDER, 0);

      return;
    }

  /* Amoeba transformed to diamonds.  */
  if (amoeba->transformed)
    {
      cave_set_entry (cave, x, y, OBJECT_DIAMOND, 0);
      
      return;
    }

  if (!amoeba->can_grow)
    for (m= 0; m < 4; m++)
      {
	GStonesObject *type= cave->entry[x+x_diff[m]][y+y_diff[m]].object;
	
	if ((type == OBJECT_EMPTY) || (type == OBJECT_DIRT))
	  amoeba->can_grow= TRUE;
      }
 
  
  /* Is this amoeba willing to grow?  */
  if (random () % (amoeba->slow_time ? 128 : 16) < 4)
    {
      GStonesObject *type;
      
      /* We randomly take one direction.  */
      m= random () % 4;
      
      type= cave->entry[x+x_diff[m]][y+y_diff[m]].object;
      
      if (type == OBJECT_EMPTY || type == OBJECT_DIRT)
	{
	  cave_set_entry (cave, x+x_diff[m], y+y_diff[m], OBJECT_AMOEBA, 0);
	  cave->entry[x+x_diff[m]][y+y_diff[m]].scanned= TRUE;
	}
    }
}

static void
amoeba_signals (GStonesCave *cave, GStonesSignal signal, GStonesObjContext *context)
{
  AmoebaData *amoeba= (AmoebaData *) object_context_private_data (context);

  if (signal == SIGNAL_OPTION_CHANGED)
    {
      amoeba->max_size= object_context_get_int_option (context, "maxSize");
      amoeba->slow_time= cave_time_to_frames 
	(cave, object_context_get_float_option (context, "slowTime"));
    }
  else if (signal == SIGNAL_CAVE_PRE_SCAN)
    {
      guint x;
      guint y;

      amoeba->transformed= !amoeba->can_grow;
      amoeba->size       = 0;
      amoeba->can_grow   = FALSE;
      if (amoeba->slow_time) amoeba->slow_time--;  
      
      for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
	for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
	  if (cave->entry[x][y].object == OBJECT_AMOEBA) amoeba->size++;
    }
}


static guint
amoeba_animate (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  if ((cave->frame % 6) < 4)
    return cave->frame % 6;
  else
    return 6-cave->frame % 6;
}

static GStonesObjectOption amoeba_options[]=
{
  GSTONES_OPTION_INT  ("maxSize=200", NULL),
  GSTONES_OPTION_TIME ("slowTime=0", NULL),
  GSTONES_OPTION_END
};

static GStonesObjectDesc amoeba_object=
{
  "amoeba",

  amoeba_init_cave,
  free_private_data,
 
  amoeba_scanned,  
  amoeba_signals,

  "amoeba.png",
  amoeba_animate,
  amoeba_options,  
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
  guint state= 0;
  
  if ((cave->player_x_direction != 0) || 
      (cave->player_y_direction != 0))
    {
      gboolean moved= FALSE;
      
      guint xn= x+cave->player_x_direction;
      guint yn= y+cave->player_y_direction;
      
      GStonesObject *type= cave->entry[xn][yn].object;

      if (type == OBJECT_EMPTY || type == OBJECT_DIRT)
	{
	  if (cave->player_x_direction > 0)
	    state= 3;
	  else if (cave->player_x_direction < 0)
	    state= 4;

	  moved= TRUE;
	}
      else if (type == OBJECT_DIAMOND)
	{
	  /* We only can collect diamonds, if they are not falling.  */
	  if (cave->entry[xn][yn].state == 0)
	    {
	      gboolean extra_life;
	      
	      cave->diamonds_collected++;
	      
	      if (cave->diamonds_collected <= cave->diamonds_needed)
		{
		  player_set_diamonds (cave->player, 
				       cave->diamonds_needed-
				       cave->diamonds_collected);

		  if (cave->diamonds_collected == cave->diamonds_needed)
		    cave_emit_signal (cave, SIGNAL_DOOR_OPEN);
		  
		  extra_life= player_inc_score (cave->player, 
						cave->diamond_score);
		}
	      else
		{
		  extra_life= player_inc_score (cave->player,
					       cave->extra_diamond_score);
		}

	      if (extra_life)
		cave_emit_signal (cave, SIGNAL_PLAYER_EXTRALIFE);

	      if (cave->player_x_direction > 0)
		state= 3;
	      else if (cave->player_x_direction < 0)
		state= 4;

	      moved= TRUE;
	    }
	}
      else if (type == OBJECT_EXIT_OPENED)
	{
	  /* We finished this cave!!! */
	  moved       = TRUE;
	  cave->flags|= CAVE_FINISHED;
	}
      else if (cave->player_y_direction == 0)
	{
	  if (cave->player_x_direction > 0)
	    state= 1;
	  else
	    state= 2;
	  
	  if ((type == OBJECT_BOULDER) &&
	      (cave->entry[xn][yn].state == 0) &&
	      (cave->entry[xn+cave->player_x_direction][yn].object == OBJECT_EMPTY))
	    { 
	      if (random () % 8 == 0)
		{
		  moved= TRUE;
		  
		  cave_set_entry 
		    (cave, xn+cave->player_x_direction, yn, type, 0);
		}
	    }
	}
      
      if (moved)
	{
	  if (cave->player_push)
	    {
	      cave_set_entry (cave, xn, yn, OBJECT_EMPTY, 0);
	    }
	  else
	    {
	      cave->entry[xn][yn].object = OBJECT_GNOME;
	      cave->entry[xn][yn].scanned= TRUE;
	      cave_set_entry (cave, x, y, OBJECT_EMPTY, 0);
	      
	      cave->player_x= xn;
	      cave->player_y= yn;

	      x= xn;
	      y= yn;
	    }
	}
    }
  cave->entry[x][y].state= state;
}


void
gnome_signals (GStonesCave *cave, GStonesSignal signal, GStonesObjContext *context)
{
  if (signal == SIGNAL_PLAYER_DIE)
    {
      if (cave->flags &  CAVE_PLAYER_EXISTS)
	{
	  explosion_new (cave, cave->player_x, cave->player_y, TRUE);
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
  register gint  state= cave->entry[x][y].state;
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
/* GROWING WALL stuff */


static void
growing_wall_scanned (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  if (cave->entry[x-1][y].object == OBJECT_EMPTY)
    {
      cave_set_entry (cave, x-1, y, OBJECT_GROWING_WALL, 0);
      cave->entry[x-1][y].scanned= TRUE;
    }
  
  if (cave->entry[x+1][y].object == OBJECT_EMPTY)
    {
      cave_set_entry (cave, x+1, y, OBJECT_GROWING_WALL, 0);
      cave->entry[x+1][y].scanned= TRUE;
    }
}


static GStonesObjectDesc growing_wall_object=
{
  "growing wall",

  NULL,
  NULL,

  growing_wall_scanned,  
  NULL,

  "wall.png",
  NULL,
  NULL,
  0, 0
};


/*****************************************************************************/
/* MAGIC WALL stuff */

typedef enum
{
  MAGIC_WALL_WAITING,
  MAGIC_WALL_MILLING,
  MAGIC_WALL_EXPIRED
} MagicWallStatus;


typedef struct _MagicWallData MagicWallData;

struct _MagicWallData
{
  /* Dynamic data.  */
  MagicWallStatus status;
  guint           mill_time;
};


static gboolean
magic_wall_init_cave (GStonesCave *cave, GStonesObjContext *context)
{
  MagicWallData *magic_wall;
  
  magic_wall= g_new0 (MagicWallData, 1);
  if (!magic_wall)
    return FALSE;
  
  magic_wall->status   = MAGIC_WALL_WAITING;
  magic_wall->mill_time= cave_time_to_frames 
    (cave, object_context_get_float_option (context, "millingTime"));
  
  object_context_set_private_data (context, magic_wall);

  return TRUE;
}


static void
magic_signals (GStonesCave *cave, GStonesSignal signal, 
	       GStonesObjContext *context)
{
  MagicWallData *magic_wall= (MagicWallData *) object_context_private_data 
    (context);
  
  guint x;
  guint y;
  
  if (signal == SIGNAL_OPTION_CHANGED)
    {
      magic_wall->mill_time= cave_time_to_frames 
	(cave, object_context_get_float_option (context, "millingTime"));
    }
  if (signal == SIGNAL_CAVE_PRE_SCAN)
    {
      if (magic_wall->status == MAGIC_WALL_MILLING)
	{
	  if (magic_wall->mill_time)
	    magic_wall->mill_time--;
	  else
	    {
	      magic_wall->status= MAGIC_WALL_EXPIRED;
	      for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
		for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
		  if (cave->entry[x][y].object == OBJECT_MAGIC_WALL)
		    cave->entry[x][y].state= 2;
	    }
	}
    }
  else if (signal == SIGNAL_MAGIC_WALL_START)
    {
      if (magic_wall->status == MAGIC_WALL_WAITING)
	{    
	  magic_wall->status= MAGIC_WALL_MILLING;
	  
	  for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
	    for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
	      if (cave->entry[x][y].object == OBJECT_MAGIC_WALL)
		cave->entry[x][y].state= 1;
	}
    }
}


static guint
magic_wall_animate (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  if (cave->entry[x][y].state == 1)
    return 1+cave->frame % 4;

  return 0;
}


static GStonesObjectOption magic_wall_options[]=
{
  GSTONES_OPTION_TIME ("millingTime=0", NULL),
  GSTONES_OPTION_END
};

static GStonesObjectDesc magic_wall_object=
{
  "magic wall",
  
  magic_wall_init_cave,
  free_private_data,

  NULL,
  magic_signals,

  "wall.png",
  magic_wall_animate,
  magic_wall_options,
  0, 0
};



/*****************************************************************************/
/* FIREFLY stuff */


static void
firefly_scanned (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  guint index[3]= {1, 0, 3};
  guint m;

  /* Let this firefly explode, if it hits something it doesn't like.  */
  for (m= 0; m < 4; m++)
    {
      GStonesObject *type= cave->entry[x+x_diff[m]][y+y_diff[m]].object;
      
      if (type == OBJECT_GNOME || type == OBJECT_AMOEBA)
	{
	  explosion_new (cave, x, y, FALSE);
	  return;
	}
    }
  
  for (m= 0; m < 2; m++)
    {
      guint state= (cave->entry[x][y].state+index[m]) % 4;
      guint xn= x+x_diff[state];
      guint yn= y+y_diff[state];
      
      if (cave->entry[xn][yn].object == OBJECT_EMPTY)
	{
	  cave_set_entry (cave, x, y, OBJECT_EMPTY, 0);
	  cave_set_entry (cave, xn, yn, OBJECT_FIREFLY, state);
	  cave->entry[xn][yn].scanned= TRUE;
	  return;
	}
    }
  
  cave->entry[x][y].state= (cave->entry[x][y].state+index[2]) % 4;
}


static GStonesObjectDesc firefly_object=
{
  "firefly",
  
  NULL,
  NULL, 

  firefly_scanned,
  NULL,

  "firefly.png",
  eight_animate,
  NULL,
  0, 0
};



/*****************************************************************************/
/* BUTTERFLY stuff */


static void
butterfly_scanned (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  guint index[3]= {3, 4, 5};
  guint m;

  /* Let this firefly explode, if it hits something it doesn't like.  */
  for (m= 0; m < 4; m++)
    {
      GStonesObject *type= cave->entry[x+x_diff[m]][y+y_diff[m]].object;
      
      if (type == OBJECT_GNOME || type == OBJECT_AMOEBA)
	{
	  explosion_new (cave, x, y, TRUE);
	  return;
	}
    }
  
  for (m= 0; m < 2; m++)
    {
      guint state= (cave->entry[x][y].state+index[m]) % 4;
      guint xn= x+x_diff[state];
      guint yn= y+y_diff[state];
      
      if (cave->entry[xn][yn].object == OBJECT_EMPTY)
	{
	  cave_set_entry (cave, x, y, OBJECT_EMPTY, 0);
	  cave_set_entry (cave, xn, yn, OBJECT_BUTTERFLY, state);
	  cave->entry[xn][yn].scanned= TRUE;
	  return;
	}
    }
  
  cave->entry[x][y].state= (cave->entry[x][y].state+index[2]) % 4;
}


static GStonesObjectDesc butterfly_object=
{
  "butterfly",
  
  NULL,
  NULL,
  
  butterfly_scanned,
  NULL,

  "butterfly.png",
  eight_animate,
  NULL,
  0, 0
};



/*****************************************************************************/
/* BOULDER stuff */


static void
boulder_scanned (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  if (cave->entry[x][y+1].object == OBJECT_EMPTY)
    {
      cave_set_entry (cave, x, y, OBJECT_EMPTY, 0);
      cave_set_entry (cave, x, y+1, OBJECT_BOULDER, 1);
      cave->entry[x][y+1].scanned= TRUE;
    }
  else if ((cave->entry[x][y+1].object == OBJECT_WALL) ||
	   (((cave->entry[x][y+1].object == OBJECT_BOULDER) ||
	     (cave->entry[x][y+1].object == OBJECT_DIAMOND)) &&
	    cave->entry[x][y+1].state == 0))
	      {
		if (cave->entry[x+1][y].object   == OBJECT_EMPTY &&
		    cave->entry[x+1][y+1].object == OBJECT_EMPTY)
		  {
		    cave_set_entry (cave, x, y, OBJECT_EMPTY, 0);
		    cave_set_entry (cave, x+1, y, OBJECT_BOULDER, 1);
		    cave->entry[x+1][y].scanned= TRUE;
		  }
		else if (cave->entry[x-1][y].object   == OBJECT_EMPTY &&
			 cave->entry[x-1][y+1].object == OBJECT_EMPTY)
		  {
		    cave_set_entry (cave, x, y, OBJECT_EMPTY, 0);
		    cave_set_entry (cave, x-1, y, OBJECT_BOULDER, 1);
		    cave->entry[x-1][y].scanned= TRUE;
		  }
		else
		  cave->entry[x][y].state= 0;
	      }
  else if (cave->entry[x][y].state == 1)
    {
      GStonesObject *type= cave->entry[x][y+1].object;
      
      if ((type == OBJECT_BUTTERFLY) ||
	  (type == OBJECT_FIREFLY) ||
	  (type == OBJECT_GNOME))
	{
	  explosion_new (cave, x, y+1, type != OBJECT_FIREFLY);
	}
      else if (type == OBJECT_MAGIC_WALL)
	{
	  if (cave->entry[x][y+1].state < 2)
	    {
	      cave_emit_signal (cave, SIGNAL_MAGIC_WALL_START);
	      
	      if (cave->entry[x][y+2].object == OBJECT_EMPTY)
		{
		  cave_set_entry (cave, x, y+2, OBJECT_DIAMOND, 1);
		  cave->entry[x][y+2].scanned= TRUE;
		}
	    }
	  cave_set_entry (cave, x, y, OBJECT_EMPTY, 0);
	}
      else
	cave->entry[x][y].state= 0;
    }
  else
    cave->entry[x][y].state= 0;
}


static GStonesObjectDesc boulder_object=
{
  "boulder",

  NULL,
  NULL,

  boulder_scanned,
  NULL,
  
  "boulder.png",
  NULL,
  NULL,
  0, 0
};



/*****************************************************************************/
/* DIAMOND stuff */


static void
diamond_scanned (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  if (cave->entry[x][y+1].object == OBJECT_EMPTY)
    {
      cave->entry[x][y].object   = OBJECT_EMPTY;
      cave->entry[x][y+1].object = OBJECT_DIAMOND;
      cave->entry[x][y+1].state  = 1;
      cave->entry[x][y+1].scanned= TRUE;
    }
  else if ((cave->entry[x][y+1].object == OBJECT_WALL) ||
	   (((cave->entry[x][y+1].object == OBJECT_BOULDER) ||
	     (cave->entry[x][y+1].object == OBJECT_DIAMOND)) &&
	    cave->entry[x][y+1].state == 0))
	      {
		if (cave->entry[x+1][y].object   == OBJECT_EMPTY && 
		    cave->entry[x+1][y+1].object == OBJECT_EMPTY)
		  {
		    cave->entry[x][y].object   = OBJECT_EMPTY;
		    cave->entry[x+1][y].object = OBJECT_DIAMOND;
		    cave->entry[x+1][y].state  = 1;
		    cave->entry[x+1][y].scanned= TRUE;
		  }
		else if (cave->entry[x-1][y].object   == OBJECT_EMPTY &&
			 cave->entry[x-1][y+1].object == OBJECT_EMPTY)
		  {
		    cave->entry[x][y].object   = OBJECT_EMPTY;
		    cave->entry[x-1][y].object = OBJECT_DIAMOND;
		    cave->entry[x-1][y].state  = 1;
		    cave->entry[x-1][y].scanned= TRUE;
		  }
		else
		  cave->entry[x][y].state= 0;
	      }
  else if (cave->entry[x][y].state == 1)
    {
      GStonesObject *type= cave->entry[x][y+1].object;
      
      if ((type == OBJECT_BUTTERFLY) ||
	  (type == OBJECT_FIREFLY) ||
	  (type == OBJECT_GNOME))
	{
	  explosion_new (cave, x, y+1, type != OBJECT_FIREFLY);
	}
      else if (type == OBJECT_MAGIC_WALL)
	{
	  if (cave->entry[x][y+1].state < 2)
	    {
	      cave_emit_signal (cave, SIGNAL_MAGIC_WALL_START);
	      
	      if (cave->entry[x][y+2].object == OBJECT_EMPTY)
		{
		  cave->entry[x][y+2].object = OBJECT_BOULDER;
		  cave->entry[x][y+2].state  = 1;
		  cave->entry[x][y+2].scanned= TRUE;
		}
	    }
	  cave->entry[x][y].object= OBJECT_EMPTY;
	}
      else
	cave->entry[x][y].state= 0;
    }
  else
    cave->entry[x][y].state= 0;
}


static GStonesObjectDesc diamond_object=
{
  "diamond",

  NULL, 
  NULL,

  diamond_scanned,
  NULL,

  "diamond.png",
  eight_animate,
  NULL,
  0, 0
};



/*****************************************************************************/
/* EXPLOSION stuff */

typedef struct _ExplosionState ExplosionState;

struct _ExplosionState
{
  guint state  : 2;
  guint diamond: 1;
};


static void
explosion_scanned (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  ExplosionState state= *((ExplosionState*) &cave->entry[x][y].state);
    
  if (state.state == 2)
    {
      /* The third bit indicates, wheater the explosion will explode
         to a diamond or to nothing.  */
      if (state.diamond)
	cave->entry[x][y].object= OBJECT_DIAMOND;
      else
	cave->entry[x][y].object= OBJECT_EMPTY;
      
      cave->entry[x][y].state= 0;
      cave->entry[x][y].scanned= TRUE;
    }
  else
    {
      state.state++;
      cave->entry[x][y].state= *((guint*)&state);
    }
}


static void
explosion_new (GStonesCave *cave, guint x, guint y, gboolean diamond)
{
  guint m;
  gint x_diff[9]= {-1, -1, -1,  0,  0,  0,  1,  1,  1};
  gint y_diff[9]= {-1,  0,  1, -1,  0,  1, -1,  0,  1};

  for (m= 0 ; m < 9 ; m++)
    {
      ExplosionState state;
      guint xn= x+x_diff[m];
      guint yn= y+y_diff[m];

      GStonesObject *type= cave->entry[xn][yn].object;
      
      if (type != OBJECT_FRAME)
	{
	  if (type == OBJECT_GNOME && !(cave->flags & CAVE_FINISHED))
	    {
	      /* Unfortunetely our little gnome died ;-(  */
	      cave->flags|= CAVE_FINISHED;
	      cave->flags&= ~CAVE_PLAYER_EXISTS;
	    }

	  state.state  = 0;
	  state.diamond= diamond;

	  cave->entry[xn][yn].object = OBJECT_EXPLOSION;
	  cave->entry[xn][yn].state  = *((guint*)&state);
	  cave->entry[xn][yn].scanned= TRUE;
	}
      
    }
}


static guint
explosion_animate (GStonesCave *cave, guint x, guint y, GStonesObjContext *context)
{
  ExplosionState state= *((ExplosionState*) &cave->entry[x][y].state);
    
  return state.state;
}


static GStonesObjectDesc explosion_object=
{
  "explosion",
  
  NULL,
  NULL,

  explosion_scanned,
  NULL,

  "explosion.png",
  explosion_animate,
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


void
closed_exit_signals (GStonesCave *cave, GStonesSignal signal, GStonesObjContext *context)
{
  guint x;
  guint y;

  if (signal == SIGNAL_DOOR_OPEN)
    {
      for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
	for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
	  if (cave->entry[x][y].object == OBJECT_EXIT_CLOSED)
	    cave->entry[x][y].object= OBJECT_EXIT_OPENED;
    }
}


static GStonesObjectDesc closed_exit_object=
{
  "closed exit",

  NULL,
  NULL,
  
  NULL,
  closed_exit_signals,
  
  "door.png",
  NULL,
  NULL,
  0, 0
};


static GStonesObjectDesc opened_exit_object=
{
  "opened exit",

  NULL,
  NULL,
  
  NULL,
  NULL,

  "door.png",
  entrance_animate,
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
  OBJECT_AMOEBA      = object_register (plugin, &amoeba_object);
  OBJECT_GNOME       = object_register (plugin, &gnome_object);
  OBJECT_GROWING_WALL= object_register (plugin, &growing_wall_object);
  OBJECT_MAGIC_WALL  = object_register (plugin, &magic_wall_object);
  OBJECT_FIREFLY     = object_register (plugin, &firefly_object);
  OBJECT_BUTTERFLY   = object_register (plugin, &butterfly_object);
  OBJECT_BOULDER     = object_register (plugin, &boulder_object);
  OBJECT_DIAMOND     = object_register (plugin, &diamond_object);
  OBJECT_EXPLOSION   = object_register (plugin, &explosion_object);
  OBJECT_ENTRANCE    = object_register (plugin, &entrance_object);
  OBJECT_EXIT_CLOSED = object_register (plugin, &closed_exit_object);
  OBJECT_EXIT_OPENED = object_register (plugin, &opened_exit_object);

  SIGNAL_PLAYER_EXTRALIFE= gstones_signal ("player.extralife");
  SIGNAL_DOOR_OPEN       = gstones_signal ("door.open");
  SIGNAL_MAGIC_WALL_START= gstones_signal ("magic_wall.start");
  SIGNAL_CAVE_PRE_SCAN   = gstones_signal ("cave.pre_scan");
  SIGNAL_CAVE_POST_SCAN  = gstones_signal ("cave.post_scan");
  SIGNAL_PLAYER_DIE      = gstones_signal ("player.die");
  SIGNAL_PLAYER_START    = gstones_signal ("player.start");
  SIGNAL_OPTION_CHANGED  = gstones_signal ("option.changed");

  return "gnome-stones";
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
