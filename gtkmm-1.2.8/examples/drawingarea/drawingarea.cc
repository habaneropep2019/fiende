
#include <gtk--/main.h>
#include <gtk--/window.h>
#include <gtk--/drawingarea.h>


class MyArea : public Gtk::DrawingArea
  {
      Gdk_GC       gc_;
      Gdk_Color    blue_,red_;
      Gdk_Window   window_;
    public:
      MyArea()
        {
          // in the ctor you can only allocate colors, 
          // get_window() returns 0 because we have not be realized
          Gdk_Colormap colormap_=get_default_colormap ();

          blue_=Gdk_Color("blue");
          red_=Gdk_Color("red");

          colormap_.alloc(blue_);
          colormap_.alloc(red_);
        }
    protected:

      virtual void realize_impl()
        {
          // we need to do the default realize
          Gtk::DrawingArea::realize_impl();

          // Now we can allocate any additional resources we need
          window_=get_window();
          gc_.create(window_); 
          window_.set_background(red_);
          window_.clear();
          gc_.set_foreground(blue_);
        }


      virtual gint expose_event_impl(GdkEventExpose* e)
        {
          // here is where we draw on the window
          window_.clear();
          window_.draw_line(gc_,1,1,100,100);
          return true;
        }
  };

//Quit app when user closes main window:
gint on_win_delete_event(GdkEventAny* event)
{
  Gtk::Main::quit();
  return FALSE;
}

int main(int argc, char** argv)
  {
     Gtk::Kit kit(argc,argv);
     Gtk::Window win;
     win.delete_event.connect(SigC::slot(&on_win_delete_event));
     
     MyArea area;
     win.add(area);
     win.show_all();
     
     kit.run();

     return 0;
  }
