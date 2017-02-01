/* item-audio.c
 * Copyright (C) 1999  Chris Lahey <clahey@umich.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gnome.h>
#include "item-audio.h"
#include "color.h"

static void item_audio_init		(ItemAudio		 *audio);
static void item_audio_class_init	(ItemAudioClass	 *klass);
static void item_audio_update (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);
static void item_audio_draw (GnomeCanvasItem *item, GdkDrawable *drawable, int x, int y, int width, int height);
static double item_audio_point (GnomeCanvasItem *item, double x, double y, int cx, int cy, GnomeCanvasItem **actual_item);
static void item_audio_translate (GnomeCanvasItem *item, double dx, double dy);
static void item_audio_set_arg (GtkObject *o, GtkArg *arg, guint arg_id);
static void item_audio_get_arg (GtkObject *object, GtkArg *arg, guint arg_id);
static void item_audio_realize (GnomeCanvasItem *item);
static void item_audio_unrealize (GnomeCanvasItem *item);


static GnomeCanvasItemClass *parent_class = NULL;


/* The arguments we take */
enum {
	ARG_0,
	ARG_BONOBO_OBJECT_DATA,
	/*	ARG_HEIGHT*/
};

GtkType
item_audio_get_type (void)
{
  static GtkType audio_type = 0;

  if (!audio_type)
    {
      static const GtkTypeInfo audio_info =
      {
        "ItemAudio",
        sizeof (ItemAudio),
        sizeof (ItemAudioClass),
        (GtkClassInitFunc) item_audio_class_init,
        (GtkObjectInitFunc) item_audio_init,
        /* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      audio_type = gtk_type_unique (gnome_canvas_item_get_type (), &audio_info);
    }

  return audio_type;
}

static void
item_audio_class_init (ItemAudioClass *klass)
{
  GtkObjectClass *object_class;
  GnomeCanvasItemClass *item_class;

  object_class = (GtkObjectClass*) klass;
  item_class = (GnomeCanvasItemClass *) klass;

  parent_class = gtk_type_class (gnome_canvas_item_get_type ());
  
  gtk_object_add_arg_type ("ItemAudio::BonoboObjectData", GTK_TYPE_POINTER, 
			   GTK_ARG_WRITABLE, ARG_BONOBO_OBJECT_DATA); 
  /*  gtk_object_add_arg_type ("ItemAudio::height", GTK_TYPE_DOUBLE, 
      GTK_ARG_READWRITE, ARG_HEIGHT);*/
 
  object_class->set_arg = item_audio_set_arg;
  object_class->get_arg = item_audio_get_arg;
  /*  object_class->destroy = item_audio_destroy; */

  /* GnomeCanvasItem method overrides */
  item_class->update      = item_audio_update;
  item_class->realize     = item_audio_realize;
  item_class->unrealize   = item_audio_unrealize;
  item_class->draw        = item_audio_draw;
  item_class->point       = item_audio_point;
  item_class->translate   = item_audio_translate;
}

static void
item_audio_init (ItemAudio *audio)
{
  audio->bonobo_object_data = NULL;
  audio->height   = 1.0;
}

static void
item_audio_update (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
  ItemAudio *item_audio = ITEM_AUDIO (item);
  if (GNOME_CANVAS_ITEM_CLASS (parent_class)->update)
    (* GNOME_CANVAS_ITEM_CLASS (parent_class)->update) (item, affine, clip_path, flags);
  
  item->x1 = 0;
  item->y1 = 0;
  if ( item_audio->bonobo_object_data )
    {
      item->x2 = item_audio->bonobo_object_data->sound_len;
    }
  else item->x2 = 0;
  item->y2 = 256;
  
  gnome_canvas_group_child_bounds (GNOME_CANVAS_GROUP (item->parent), item);
}

static void
item_audio_draw (GnomeCanvasItem *item, GdkDrawable *drawable, int x, int y, int width, int height)
{
  ItemAudio *item_audio = ITEM_AUDIO (item);
  int i;
  if( item_audio->bonobo_object_data && ( x + width > item_audio->bonobo_object_data->sound_len ) )
    {
      width = item_audio->bonobo_object_data->sound_len - x;
    }

  gdk_draw_rectangle( drawable, item_audio->bg_gc, TRUE, 0, MAX( 0, -y ), width, MIN( height - MAX( 0, -y ), 256 ) );
  if ( item_audio->bonobo_object_data && item_audio->bonobo_object_data->sound )
    {
      for( i = x; i < x + width; i++ )
	{
	  /* This conversion of the data leaves it in a somewhat odd
             state.  This should probably be improved in later
             versions.  Eventually, the data will have to be converted
             as you send it across for speed reasons. */
	  if ( ((unsigned char *) item_audio->bonobo_object_data->sound)[i] < 128 )
	    gdk_draw_point( drawable, item_audio->gc, i - x, ((unsigned char *) item_audio->bonobo_object_data->sound)[i] - y );
	  else
	    gdk_draw_point( drawable, item_audio->gc, i - x, 384 - ((unsigned char *) item_audio->bonobo_object_data->sound)[i] - y );
	}
    }
}

static double
item_audio_point (GnomeCanvasItem *item, double x, double y, int cx, int cy,
		 GnomeCanvasItem **actual_item)
{
  *actual_item = item;
  return 0.0;
}

static void
item_audio_translate (GnomeCanvasItem *item, double dx, double dy)
{
  printf ("item_audio_translate %g, %g\n", dx, dy);
}

static void
item_audio_set_arg (GtkObject *o, GtkArg *arg, guint arg_id)
{
	GnomeCanvasItem *item;
	ItemAudio *item_audio;

	item = GNOME_CANVAS_ITEM (o);
	item_audio = ITEM_AUDIO (o);
	
	switch (arg_id){
	case ARG_BONOBO_OBJECT_DATA:
	  item_audio->bonobo_object_data = GTK_VALUE_POINTER (*arg);
	  gnome_canvas_item_request_update (item);
	  break;
	  /*	case ARG_HEIGHT:
		item_audio->height = GTK_VALUE_DOUBLE (*arg);
		break*/
	}
}

static void
item_audio_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	ItemAudio *item_audio;

	item_audio = ITEM_AUDIO (object);

	switch (arg_id) {
	  /*	case ARG_WIDTH:
		GTK_VALUE_DOUBLE (*arg) = item_audio->width;
		break;*/
	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

static void
item_audio_realize (GnomeCanvasItem *item)
{
	ItemAudio *item_audio;

	item_audio = ITEM_AUDIO (item);

	if (parent_class->realize)
	  (* parent_class->realize) (item);

	if (!item->canvas->aa)
	  {
	    item_audio->gc = gdk_gc_new (item->canvas->layout.bin_window);
	    item_audio->bg_gc = gdk_gc_new (item->canvas->layout.bin_window);
	    
	    /* Allocate the default colors */
	    item_audio->background = ia_white;
	    item_audio->foreground = ia_black;
	    
	    gdk_gc_set_foreground (item_audio->gc, &item_audio->foreground);
	    gdk_gc_set_background (item_audio->gc, &item_audio->background);
	    
	    gdk_gc_set_foreground (item_audio->bg_gc, &item_audio->background);
	    gdk_gc_set_background (item_audio->bg_gc, &item_audio->foreground);
	  }
}

static void
item_audio_unrealize (GnomeCanvasItem *item)
{
	ItemAudio *item_audio;

	item_audio = ITEM_AUDIO (item);

	if (!item->canvas->aa)
	  {
	    gdk_gc_unref (item_audio->gc);
	    gdk_gc_unref (item_audio->bg_gc);
	    item_audio->gc = 0;
	    item_audio->bg_gc = 0;
	  }

	if (parent_class->unrealize)
	  (* parent_class->unrealize) (item);
}
