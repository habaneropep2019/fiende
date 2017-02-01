/* gnome-stones - types.h
 *
 * Time-stamp: <1998/11/21 23:56:13 carsten>
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
#ifndef TYPES_H
#define TYPES_H

#include <config.h>
#include <gmodule.h>
#include <gnome.h>



#define STONE_SIZE 32


typedef struct _GStonesPlugin       GStonesPlugin;
typedef struct _GStonesGame         GStonesGame;
typedef struct _GStonesCaveEntry    GStonesCaveEntry;
typedef struct _GStonesCave         GStonesCave;
typedef struct _GStonesPlayer       GStonesPlayer;
typedef struct _GStonesObject       GStonesObject;
typedef struct _GStonesObjectDesc   GStonesObjectDesc;
typedef struct _GStonesObjectOption GStonesObjectOption;
typedef guint                       GStonesSignal;
typedef struct _GStonesObjContext   GStonesObjContext;



/*****************************************************************************/


struct _GStonesPlugin
{
  /* The plugins handle returned by 'g_module_open'.  */
  GModule     *handle;

  /* The plugins title.  */
  gchar       *title;

  /* The hash table of all objects in this module.  The object's name is the
     key of the table.  */
  GHashTable  *objects;

  /* Pointer to the plugin's init function.  The init function must
     return the plugin's title, if the initialization was successfull.  */
  gchar     *(*objects_init) (GStonesPlugin *);
};



/*****************************************************************************/
/* The following structure describes a game object.  */

typedef gboolean (*InitCaveFunction)  (GStonesCave *cave, 
				       GStonesObjContext *context);
typedef void     (*DestroyFunction)   (GStonesCave *cave, 
				       GStonesObjContext *context);
typedef void     (*ScanFunction)      (GStonesCave *cave, guint x, guint y,
				       GStonesObjContext *context);
typedef guint    (*AnimationFunction) (GStonesCave *cave, guint x, guint y, 
				       GStonesObjContext *context);
typedef void     (*SignalFunction)    (GStonesCave *cave, GStonesSignal sig,
				       GStonesObjContext *context);
typedef enum 
{
  GSTONES_TYPE_INT,
  GSTONES_TYPE_BOOL,
  GSTONES_TYPE_STRING,
  GSTONES_TYPE_TIME,
} GStonesOptionType;


struct _GStonesObjectOption
{
  GStonesOptionType  type;
  gchar             *name;
  gchar             *description;
};


struct _GStonesObjectDesc
{
  gchar               *name;

  InitCaveFunction     init_cave_function;
  DestroyFunction      destroy_function;

  ScanFunction         scan_function;
  SignalFunction       signal_function;

  gchar               *image_name;
  AnimationFunction    animation_function;

  /* The object's options.  */
  GStonesObjectOption *options;

  /* The image to shown, if no animation_function is given.  */
  guint                image_index;

  /* Which image should be shown in the editor.  (Not used yet) */
  guint                editor_index;
};

#define GSTONES_OPTION_INT(name, description) \
  { GSTONES_TYPE_INT, name, description }
#define GSTONES_OPTION_BOOL(name, description) \
  { GSTONES_TYPE_BOOL, name, description }
#define GSTONES_OPTION_STRING(name, description) \
  { GSTONES_TYPE_STRING, name, description }
#define GSTONES_OPTION_TIME(name, description) \
  { GSTONES_TYPE_TIME, name, description }
#define GSTONES_OPTION_END \
  { GSTONES_TYPE_INT, NULL, NULL }

struct _GStonesObject
{
  GStonesObjectDesc  *description;
  GStonesPlugin      *plugin;

  /* The image to shown, if no animation_function is given.  */
  guint               image_index;

  /* Which image should be shown in the editor.  (Not used yet) */
  guint               editor_index;

  guint               num_images;
  GdkPixmap         **image;
  GdkImlibImage     **imlib_image;
};



/*****************************************************************************/
/* GStonesGame related declarations */


struct _GStonesGame
{
  gchar            *title;
  
  /* Administrativ data.  */
  gchar            *filename;
  gchar            *config_prefix;  

  guint             frame_rate;
  guint             new_life_score;
  guint             lives;

  GList            *caves;
  GList            *start_caves;

  /* The list of plugins, that this game requires.  Only references to
     allready registered plugins are stored.  */
  GList            *plugins;

  /* The list of objects, that all these plugins define.  Only
     references to allready registered objects are stored.  */
  GList            *objects;

  GHashTable       *translation_table;
};



/*****************************************************************************/
/* GStonesCave related declarations */


#define CAVE_MAX_WIDTH   80
#define CAVE_MAX_HEIGHT  40


typedef enum
{
  CAVE_FINISHED      = 1 << 0,
  CAVE_PLAYER_EXISTS = 1 << 1,
  CAVE_PAUSING       = 1 << 2
} CaveFlags;


struct _GStonesCaveEntry
{
  GStonesObject *object;
  gint           state;
  gint           anim_state;
  gboolean       scanned;
};


struct _GStonesCave
{
  gchar            *name;
  gchar            *next;

  /* Some static information about this cave.  */
  GStonesGame      *game;
  guint             width;
  guint             height;
  gboolean          is_intermission;
  guint             diamond_score;
  guint             extra_diamond_score;
  guint             diamonds_needed;
  guint             level_time;
  gchar            *message;
  GStonesPlayer    *player;

  /* Some static information about the game, that this cave belongs
     to.  */
  guint             frame_rate;

  /* Some dynamic data about this cave.  */
  CaveFlags         flags;
  gint              timer;
  guint             frame;
  guint             diamonds_collected;

  /* The player's position in this cave.  */
  guint             player_x;
  guint             player_y;

  /* The direction, that the player wants to take.  */
  gint              player_x_direction;
  gint              player_y_direction;
  gboolean          player_push;

  GStonesCaveEntry  entry[CAVE_MAX_WIDTH+2][CAVE_MAX_HEIGHT+2];

  /* This hash table holds an object context for every object that is used in
     this cave.  */
  GHashTable       *contexts;
};


/*****************************************************************************/
/* GStonesPlayer related declarations */


struct _GStonesPlayer
{
  /* Static information.  */
  guint new_life_score;
  guint max_time;
  
  /* Dynamic information.  */
  guint score;
  guint lives;
  guint time;
};


#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
