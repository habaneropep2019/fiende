/* example-start aspectframe aspectframe.cc */

#include <gtk--/window.h>
#include <gtk--/main.h>
#include <gtk--/drawingarea.h>
#include <gtk--/aspectframe.h>

class AspectWindow: public Gtk::Window
{
    Gtk::AspectFrame frame;
    Gtk::DrawingArea area;
public:
    AspectWindow();
private:
    gint delete_event_impl(GdkEventAny*) { 
	Gtk::Main::quit(); return 0; 
    }
};

AspectWindow::AspectWindow():
    frame("2x1", /* label */
	  0.5, /* center x */
	  0.5, /* center y */
	  2, /* xsize/ysize = 2 */
	  FALSE /* ignore child's aspect */)
{
    set_title("Aspect Frame");
    set_border_width(10);

    /* Add a child widget to the aspect frame */
    /* Ask for a 200x200 window, but the AspectFrame will give us a 200x100
     * window since we are forcing a 2x1 aspect ratio */
    area.set_usize(200, 200);
    frame.add(area);
    /* Create an aspect_frame and add it to our toplevel window */
    frame.show_all();
    add(frame);    
}

int
main (int argc, char *argv[])
{
    Gtk::Main m(&argc, &argv);
    AspectWindow window;
   
    window.show();
    m.run();

    return 0;
}
/* example-end */
