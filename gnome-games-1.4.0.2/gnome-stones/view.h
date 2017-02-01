/* gnome-stones - view.h
 *
 * Time-stamp: <1999/01/18 18:50:24 carsten>
 *
 * Copyright (C) 1998, 1999 Carsten Schaar
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
#ifndef VIEW_H
#define VIEW_H

#include <config.h>
#include <gnome.h>
#include "cave.h"

#define GSTONES_TYPE_VIEW     (view_get_type ())
#define GSTONES_VIEW(o)       (GTK_CHECK_CAST ((o), GSTONES_TYPE_VIEW, GStonesView))
#define GSTONES_VIEW_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), GSTONES_TYPE_VIEW))
#define GSTONES_IS_VIEW(o)    (GTK_CHECK_TYPE ((o), GSTONES_TYPE_VIEW))


typedef struct _GStonesView      GStonesView;
typedef struct _GStonesViewClass GStonesViewClass;


typedef enum
{
  VIEW_CURTAIN_OPEN,
  VIEW_CURTAIN_ANIMATE,
  VIEW_CURTAIN_CLOSED,
} ViewCurtainMode;


typedef enum
{
  DISPLAY_IMAGE,
  DISPLAY_CAVE,
} ViewDisplayMode;


typedef enum
{
  CURTAIN_DISPLAY_NONE,
  CURTAIN_DISPLAY_OPENING,
  CURTAIN_DISPLAY_CLOSING
} ViewCurtainDisplayMode;



typedef void (*ViewCurtainFunc) (ViewCurtainMode);


struct _GStonesView
{
  GtkDrawingArea          canvas;

  ViewDisplayMode         display_mode;

  /* cave display */
  guint                   x_offset;
  guint                   y_offset;
  gboolean                x_scrolling;
  gboolean                y_scrolling;

  /* image display */
  GdkPixmap              *image;

  /* curtain display */
  GdkPixmap              *curtain_image;
  ViewCurtainDisplayMode  curtain_display_mode;
  ViewCurtainFunc         curtain_func;
  guint                   curtain_timeout;
  guint                   curtain;
};


struct _GStonesViewClass
{
  GtkDrawingAreaClass parent_class;
};


GtkType    view_get_type         (void);

GtkWidget *view_new              (GdkImlibImage *curtain_image);

void       view_set_curtain_mode (GStonesView     *view,
				  ViewCurtainMode  mode,
				  ViewCurtainFunc  func);

void       view_display_image    (GStonesView *view, GdkPixmap *image);
void       view_display_cave     (GStonesView *view, GStonesCave *cave);
void       view_calculate_offset (GStonesView *view, GStonesCave *cave);


#endif
