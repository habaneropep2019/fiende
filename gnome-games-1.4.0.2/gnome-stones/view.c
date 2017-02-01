/* gnome-stones - main.c
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
#include "view.h"

#include "types.h"



/*****************************************************************************/
/* Some definitions */


#define GAME_COLS  20
#define GAME_ROWS 12
#define GAME_SCROLL_MAX 3
#define GAME_SCROLL_MIN 6

#define CURTAIN_DELAY 50

#ifdef USE_KEY_RELEASE
#define GAME_EVENTS (GDK_KEY_PRESS_MASK             |\
		     GDK_KEY_RELEASE_MASK)
;
#else
#define GAME_EVENTS (GDK_KEY_PRESS_MASK)
#endif



/*****************************************************************************/
/* Some declarations */


gint view_expose_event_cb (GtkWidget *widget, GdkEventExpose *event, 
			   gpointer data);
static gint view_expose (GtkWidget *widget, GdkEventExpose *event);

extern GStonesCave *cave;


/*****************************************************************************/
/* Stuff that is needed for the GTK* type system */


static void
view_class_init (GStonesViewClass *class)
{
  GtkWidgetClass *widget_class;
  
  widget_class= (GtkWidgetClass *) class;

#ifndef USE_GNOME_CANVAS
  widget_class->expose_event= view_expose;
#endif
}



static void
view_init (GStonesView *view)
{
  view->display_mode        = DISPLAY_IMAGE;

  /* cave display */
  view->x_offset            = 0;
  view->y_offset            = 0;
  view->x_scrolling         = TRUE;
  view->y_scrolling         = TRUE;

  /* image display */
  view->image               = NULL;
  
  /* curtain display */
  view->curtain_display_mode= CURTAIN_DISPLAY_NONE;
  view->curtain_timeout     = 0;
  view->curtain             = 0;
}


GtkType
view_get_type (void)
{
  static GtkType view_type= 0;

  if (!view_type)
    {
      GtkTypeInfo view_info=
      {
	"GStonesView",
	sizeof (GStonesView),
	sizeof (GStonesViewClass),
	(GtkClassInitFunc) view_class_init,
	(GtkObjectInitFunc) view_init,
	NULL, /* reserved 1 */
	NULL, /* reserved 2 */
	(GtkClassInitFunc) NULL
      };
      
      view_type= gtk_type_unique (gtk_drawing_area_get_type (), &view_info);
    }
  
  return view_type;
} 
  

/*****************************************************************************/




GtkWidget *
view_new (GdkImlibImage *curtain_image)
{
  GStonesView    *view;
#ifdef USE_GNOME_CANVAS
  GnomeCanvas    *canvas;
#else
  GtkDrawingArea *drawing_area;
#endif
  GtkWidget      *widget;
  GtkObject      *object;

  g_return_val_if_fail (curtain_image, NULL);

  gtk_widget_push_visual (gdk_imlib_get_visual ());
  gtk_widget_push_colormap (gdk_imlib_get_colormap ());

  view= gtk_type_new (view_get_type ());
  
  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();

#ifdef USE_GNOME_CANVAS
  canvas      = GNOME_CANVAS (view);
#else
  drawing_area= GTK_DRAWING_AREA (view);
#endif
  widget      = GTK_WIDGET (view);
  object      = GTK_OBJECT (view);

#ifndef USE_GNOME_CANVAS

  gtk_widget_set_events (widget, gtk_widget_get_events (widget) | GAME_EVENTS);
  
  gtk_drawing_area_size (drawing_area, 
			 GAME_COLS * STONE_SIZE,
			 GAME_ROWS * STONE_SIZE);

  gtk_widget_show (widget);

#else

  gtk_widget_push_visual (gdk_imlib_get_visual ());
  gtk_widget_push_colormap (gdk_imlib_get_colormap ());

  view->canvas= gtk_drawing_area_new ();
  canvas= gnome_canvas_new ();

  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();
  
  gtk_widget_set_events (view->canvas, gtk_widget_get_events (view->canvas) | GAME_EVENTS);

  gtk_drawing_area_size (GTK_DRAWING_AREA (view->canvas),
			 GAME_COLS * STONE_SIZE,
			 GAME_ROWS * STONE_SIZE);

  /* Now for some experimantal gnome canvas stuff.  */
  gtk_widget_set_usize (canvas, GAME_COLS*STONE_SIZE, GAME_ROWS*STONE_SIZE);
  gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas),
				  0, 0,
				  GAME_COLS*STONE_SIZE,
				  GAME_ROWS*STONE_SIZE);

  gtk_widget_show (canvas);

  gtk_widget_show (view->canvas);
#endif

  /* Initialize curtain stuff.  */
  gdk_imlib_render (curtain_image, 
		    curtain_image->rgb_width,
		    curtain_image->rgb_height);
  
  view->curtain_image= gdk_imlib_copy_image (curtain_image);

  view->curtain_display_mode= CURTAIN_DISPLAY_CLOSING;
  view->curtain             = 0;

  return GTK_WIDGET (view);
}


/*****************************************************************************/

static gint
view_expose (GtkWidget *widget, GdkEventExpose *event)
{
  GStonesView  *view;
  GdkRectangle *area;
  int x1, y1, x2, y2, x, y;

  view= GSTONES_VIEW (widget);

  area= &event->area;

  x1 = area->x/STONE_SIZE+view->x_offset;
  y1 = area->y/STONE_SIZE+view->y_offset;
  x2 = (area->x+area->width )/STONE_SIZE+view->x_offset;
  y2 = (area->y+area->height)/STONE_SIZE+view->y_offset;
  
  for (x = x1; x <= x2; x++)
    for (y = y1; y <= y2; y++)
      {
	GdkPixmap *image= NULL;
	
	if ((view->curtain_display_mode == CURTAIN_DISPLAY_CLOSING && 
	     (x+y >= view->curtain+view->x_offset+view->y_offset)) ||
	    (view->curtain_display_mode == CURTAIN_DISPLAY_OPENING && 
	     (x+y < view->curtain+view->x_offset+view->y_offset)))
	  {
	    image= view->curtain_image;
	  }
	else if (view->display_mode == DISPLAY_IMAGE)
	  {
	  }
	else if (cave)
	  {
	    image= cave_get_image (cave, x+1, y+1);
	  }

	if (image)
	  gdk_draw_pixmap (widget->window,
			   widget->style->black_gc, image,
			   0, 0, 
			   (x-view->x_offset)*STONE_SIZE, 
			   (y-view->y_offset)*STONE_SIZE,
			   STONE_SIZE, STONE_SIZE);
	else
	  gdk_draw_pixmap (widget->window,
			   widget->style->black_gc, view->image,
			   (x-view->x_offset)*STONE_SIZE, 
			   (y-view->y_offset)*STONE_SIZE, 
			   (x-view->x_offset)*STONE_SIZE, 
			   (y-view->y_offset)*STONE_SIZE,
			   STONE_SIZE, STONE_SIZE);
      }

  return TRUE;
}

/*****************************************************************************/


static gint
view_curtain_timeout (gpointer data)
{
  GStonesView     *view= (GStonesView *) data;

  if (view->curtain_display_mode == CURTAIN_DISPLAY_CLOSING)
    {
      if (view->curtain > 0)
	{
	  /* The curtain is not closed yet.  */
	  
	  view->curtain--;
	  gtk_widget_draw (GTK_WIDGET (view), NULL);
	  return TRUE;
	}

      if (view->curtain_func)
	view->curtain_func (VIEW_CURTAIN_CLOSED);
      
      view->curtain_display_mode= CURTAIN_DISPLAY_OPENING;
      view->curtain             = GAME_COLS+GAME_ROWS;
      
      return TRUE;
    }
  else
    {
      if (view->curtain > 0)
	{
	  /* The curtain is not closed yet.  */
	  
	  view->curtain--;
	  gtk_widget_draw (GTK_WIDGET (view), NULL);
	  return TRUE;
	}
      
      view->curtain_timeout= 0;

      if (view->curtain_func)
	view->curtain_func (VIEW_CURTAIN_OPEN);
      
      return FALSE;
    }
}


void
view_set_curtain_mode (GStonesView     *view,
		       ViewCurtainMode  mode, 
		       ViewCurtainFunc  func)
{
  g_return_if_fail (view);

  if (view->curtain_timeout)
    {
      gtk_timeout_remove (view->curtain_timeout);
      view->curtain_timeout= 0;
    }
  
  switch (mode)
    {
    case VIEW_CURTAIN_OPEN:
      view->curtain_display_mode= CURTAIN_DISPLAY_NONE;
      gtk_widget_draw (GTK_WIDGET (view), NULL);
      break;
      
    case VIEW_CURTAIN_ANIMATE:
      view->curtain_display_mode= CURTAIN_DISPLAY_CLOSING;
      view->curtain             = GAME_COLS+GAME_ROWS;
      view->curtain_func        = func;


      view->curtain_timeout= 
	gtk_timeout_add (CURTAIN_DELAY, view_curtain_timeout, (gpointer) view);
      break;

    case VIEW_CURTAIN_CLOSED:
      view->curtain_display_mode= CURTAIN_DISPLAY_CLOSING;
      view->curtain             = 0;
      gtk_widget_draw (GTK_WIDGET (view), NULL);
      break;      
    }
}

void
view_display_image (GStonesView *view, GdkPixmap *image)
{
  g_return_if_fail (view);
  g_return_if_fail (GSTONES_IS_VIEW (view));
  
  if (image)
    {
      if (view->image)
	gdk_pixmap_unref (view->image);
      
      view->image= image;
      gdk_pixmap_ref (view->image);
    }

  view->display_mode= DISPLAY_IMAGE;
  gtk_widget_draw (GTK_WIDGET (view), NULL);
}


void
view_display_cave (GStonesView *view, GStonesCave *cave)
{
  g_return_if_fail (view);
  g_return_if_fail (cave);
  g_return_if_fail (GSTONES_IS_VIEW (view));
  
  view->display_mode= DISPLAY_CAVE;
  gtk_widget_draw (GTK_WIDGET (view), NULL);
}


void
view_calculate_offset (GStonesView *view, GStonesCave *cave)
{
  gint x_rel;
  gint y_rel;
  
  x_rel= cave->player_x-view->x_offset;
  y_rel= cave->player_y-view->y_offset;
  
  view->x_scrolling= view->x_scrolling || (x_rel < 3) || (x_rel > GAME_COLS+1-3) || 
    (view->x_offset+GAME_COLS > cave->width);
  view->y_scrolling= view->y_scrolling || (y_rel < 3) || (y_rel > GAME_ROWS+1-3) || 
    (view->y_offset+GAME_ROWS > cave->height);
  
  if (view->x_scrolling)
    {
      if (((x_rel < 7) || (view->x_offset+GAME_COLS > cave->width)) && (view->x_offset > 0))
	{
	  view->x_offset--;
	}
      else if ((x_rel > GAME_COLS+1-7) && (view->x_offset+GAME_COLS < cave->width))
	{
	  view->x_offset++;
	}
      else
	view->x_scrolling= FALSE;
    }

  if (view->y_scrolling)
    {      
      if (((y_rel < 5) || (view->y_offset+GAME_ROWS > cave->height)) && (view->y_offset > 0))
	{
	  view->y_offset--;
	}
      else if ((y_rel > GAME_ROWS+1-5) && (view->y_offset+GAME_ROWS < cave->height))
	{
	  view->y_offset++;
	}
      else
	view->y_scrolling= FALSE;
    }
}

