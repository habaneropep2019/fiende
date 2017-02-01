// Please say "hello world",  pretty please!

#include <iostream>
#include <gdk/gdk.h>
#include <gdk--.h>

namespace std {}
using namespace std;

gint attributes_mask;

Gdk gdk;
Gdk_WindowAttr attributes;
Gdk_Window window;
Gdk_GC gc;
Gdk_Color blue,red,green;
Gdk_Colormap colormap;
Gdk_Font font;
GdkEvent *event; // <--- this needs work

void redraw();

int main(int argc, char* argv[])
{
  gdk.init(argc,argv);

  // set the colormap to the system one
  colormap=colormap.get_system();

  // this will be replaced by Gdk_Window_Attr_Auto
  attributes->title = "hello";
  attributes->window_type = GDK_WINDOW_TOPLEVEL;
  attributes->wmclass_name = "hello";
  attributes->wmclass_class = 0;
  attributes->width = 100;
  attributes->height = 100;
  attributes->wclass = GDK_INPUT_OUTPUT;
  attributes->visual = gdk_visual_get_system();  // <-- need a visual class
  attributes->colormap = colormap;
  attributes->event_mask = (GDK_EXPOSURE_MASK |
                            GDK_KEY_PRESS_MASK |
                            GDK_ENTER_NOTIFY_MASK |
                            GDK_LEAVE_NOTIFY_MASK |
                            GDK_FOCUS_CHANGE_MASK |
                            GDK_STRUCTURE_MASK);

  attributes_mask = GDK_WA_VISUAL | GDK_WA_COLORMAP;
  attributes_mask |= GDK_WA_TITLE;
 
  // allocate a window 
  window=Gdk_Window(NULL,attributes,attributes_mask);
  window.show();

  // make some colors 
  red=Gdk_Color("red");
  blue=Gdk_Color("blue");
  green=Gdk_Color("green");

  // allocate the colors (not needed with Gdk_Color_Auto)
  colormap.alloc(red);
  colormap.alloc(blue);
  colormap.alloc(green);

  // allocate a gc
  gc=Gdk_GC(window);

  // make a font
  font=Gdk_Font("fixed");

  while(1)
    {event=gdk.event_get();
     if (event) 
       {
        cout <<"Event "<<event<<endl;
        cout <<"Event type"<<int(event->type) <<endl;
        switch (event->type)
          {case GDK_MAP:
           case GDK_EXPOSE:
             redraw();
             break;
           default:
             break;
          }
        gdk.event_free(event);
       }
    }
}

void redraw()
  {cout << "Redraw"<<endl;
   gc.set_foreground(red);
   window.draw_line(gc,0,0,100,100);
   gc.set_foreground(blue);
   window.draw_line(gc,100,0,0,100);
   gc.set_foreground(green);
   window.draw_string(font,gc,10,10,"hello world");
  }

