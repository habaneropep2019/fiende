
/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *  Revision history:
 *    Modified for gtk-- by sac@transmeta.com 
 *    Modified for gdk-- by freyd@uni-muenster.de 
 *    Modified (Added Erase function) by k.i.beaumont@larc.nasa.gov 
 *    General cleanup by Karl Nelson.
 */


#include <gtk--/main.h>
#include <gtk--/style.h>
#include <gtk--/window.h>
#include <gtk--/button.h>
#include <gtk--/box.h>
#include <gtk--/drawingarea.h>

class ScribbleDrawingArea  : public Gtk::DrawingArea
{
  /* Backing pixmap for drawing area */

  Gdk_Pixmap pixmap;
  Gdk_GC gc;
  Gdk_GC brush_gc;
  Gdk_Window win;
  Gdk_Visual visual;

  virtual gint configure_event_impl (GdkEventConfigure *event);
  virtual gint expose_event_impl (GdkEventExpose *event);
  virtual gint button_press_event_impl (GdkEventButton *event);
  virtual gint motion_notify_event_impl (GdkEventMotion *event);
  void draw_brush (gdouble x, gdouble y);

public:
  void erase();

  ScribbleDrawingArea ();
  ~ScribbleDrawingArea ();

};

ScribbleDrawingArea::ScribbleDrawingArea()
    : Gtk::DrawingArea(), pixmap (0)
  {
    set_events (GDK_EXPOSURE_MASK
		| GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK
		| GDK_POINTER_MOTION_MASK
		| GDK_POINTER_MOTION_HINT_MASK);
  }

ScribbleDrawingArea::~ScribbleDrawingArea()
  {}

/* Create a new backing pixmap of the appropriate size */
int ScribbleDrawingArea::configure_event_impl (GdkEventConfigure * /* event */)
  {
    win = get_window();
    visual = win.get_visual();

    if (pixmap)
      pixmap.release();

    gc = get_style()->get_white_gc();
    pixmap.create(get_window(),  width(), height());

    pixmap.draw_rectangle (gc, true, 0, 0, width(), height());

    return true;
  }

/* Redraw the screen from the backing pixmap */
int ScribbleDrawingArea::expose_event_impl (GdkEventExpose *event)
  {
    
    gc = get_style()->get_fg_gc(get_state());
    win.draw_pixmap(gc ,
		    pixmap,
		    event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);

    return false;
  }

/* Draw a rectangle on the screen */
void ScribbleDrawingArea::draw_brush (gdouble x, gdouble y)
  {
    GdkRectangle update_rect;
    update_rect.x = (int)x - 5;
    update_rect.y = (int)y - 5;
    update_rect.width = 10;
    update_rect.height = 10;

    // we can't create the brush gc in the ctor because there was not window
    // so we will initialize it here once, and use it from then on.
    if (!brush_gc)
      {
        brush_gc = Gdk_GC(pixmap);
        Gdk_Color red("red");
        get_colormap().alloc(red);
        brush_gc.set_foreground(red);
      }
    pixmap.draw_rectangle(
			brush_gc,
			true,
			update_rect.x, update_rect.y,
			update_rect.width, update_rect.height);
    draw(&update_rect);
  }

gint ScribbleDrawingArea::button_press_event_impl (GdkEventButton *event)
  {
    if (event->button == 1 && pixmap)
      draw_brush (event->x, event->y);

    return true;
  }

gint ScribbleDrawingArea::motion_notify_event_impl (GdkEventMotion *event)
  {
    int x, y;
    GdkModifierType state;
    Gdk_Window window(event->window);
    if (event->is_hint)
      window.get_pointer (x, y, state);
    else
      {
	x = (int)event->x;
	y = (int)event->y;
	state = (GdkModifierType) event->state;
      }
    
    if (state & GDK_BUTTON1_MASK && pixmap)
      draw_brush (x, y);
  
    return true;
  }

void ScribbleDrawingArea::erase()
  {

    // clear pixmap area to white.
    gc = get_style()->get_white_gc();
    pixmap.draw_rectangle (gc, true, 0, 0, width(), height());

    // request a refresh of whole area.
    draw(0);
  }

/*****************************************************************/

class ScribbleWindow : public Gtk::Window
  {
  
    ScribbleDrawingArea drawing_area;
    Gtk::Button eraser;
    Gtk::Button button;
    Gtk::VBox vbox;
  public:  
    ScribbleWindow ();
    ~ScribbleWindow ();
  }; 

ScribbleWindow::ScribbleWindow ()
    :  Gtk::Window(GTK_WINDOW_TOPLEVEL),
       eraser("erase"),
       button ("quit"),
       vbox (false, 0)
  {
    add (vbox);

    /* Create the drawing area */
    drawing_area.size (400, 400);
    vbox.pack_start (drawing_area, true, true, 0);


    /* Add the buttons */
    vbox.pack_start (eraser, false, false, 0);
    vbox.pack_start (button, false, false, 0);
    eraser.clicked.connect(slot(drawing_area, &ScribbleDrawingArea::erase));
    button.clicked.connect(destroy.slot());
    destroy.connect(Gtk::Main::quit.slot());

    drawing_area.show();
    eraser.show();
    button.show();
    vbox.show();
  }

ScribbleWindow::~ScribbleWindow()
  {}

int
main (int argc, char *argv[])
{
  Gtk::Main myapp(argc, argv);
  ScribbleWindow window;

  window.show();
  myapp.run();

  return 0;
}
