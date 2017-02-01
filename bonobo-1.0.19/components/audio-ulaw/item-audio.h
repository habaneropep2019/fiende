/* item-audio.h
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
#ifndef __ITEM_AUDIO_H__
#define __ITEM_AUDIO_H__

#include <gnome.h>
#include "bonobo-audio-ulaw.h"

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#define ITEM_TYPE_AUDIO			(item_audio_get_type ())
#define ITEM_AUDIO(obj)			(GTK_CHECK_CAST ((obj), ITEM_TYPE_AUDIO, ItemAudio))
#define ITEM_AUDIO_CLASS(klass)		(GTK_CHECK_CLASS_CAST ((klass), ITEM_TYPE_AUDIO, ItemAudioClass))
#define ITEM_IS_AUDIO(obj)			(GTK_CHECK_TYPE ((obj), ITEM_TYPE_AUDIO))
#define ITEM_IS_AUDIO_CLASS(klass)		(GTK_CHECK_CLASS_TYPE ((obj), ITEM_TYPE_AUDIO))


typedef struct _ItemAudio       ItemAudio;
typedef struct _ItemAudioClass  ItemAudioClass;

struct _ItemAudio
{
  GnomeCanvasItem parent;

  /* Put your own, widget-specific fields here */
  bonobo_object_data_t *bonobo_object_data;
  double height;
  GdkGC *gc;			/* GC for drawing points */
  GdkGC *bg_gc;
  
  GdkColor   background;
  GdkColor   foreground;
};

struct _ItemAudioClass
{
  GnomeCanvasItemClass parent_class;
};


GtkType    item_audio_get_type (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __ITEM_AUDIO_H__ */
