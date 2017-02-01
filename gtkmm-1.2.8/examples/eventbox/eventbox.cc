/* example-start eventbox eventbox.c */

#include <gtk--/window.h>
#include <gtk--/main.h>
#include <gtk--/eventbox.h>
#include <gtk--/label.h>
#include <gtk--/tooltips.h>

class AppWindow : public Gtk::Window
{
  Gtk::Tooltips tips;

public:
  AppWindow();
  ~AppWindow();

  virtual gint delete_event_impl (GdkEventAny*);
  gint callback(GdkEventButton*)
    { 
      Gtk::Main::quit();
      return 1;
    }
};


AppWindow::AppWindow()
  : Gtk::Window(GTK_WINDOW_TOPLEVEL)
{
  Gtk::EventBox *event_box;
  Gtk::Label *label;

  set_title ("Event Box");
  set_border_width (10);

  /* Create an EventBox and add it to our toplevel window */
    
  event_box = manage( new Gtk::EventBox () );
  add (*event_box);

  /* Create a long label */
    
  label = manage( new Gtk::Label ("Click here to quit, quit, quit, quit, quit") );
  event_box-> add (*label);

  /* Clip it short. */
  label->set_usize (110, 20);

  /* And bind an action to it */
  event_box->set_events (GDK_BUTTON_PRESS_MASK);
  event_box->button_press_event.connect(slot(*this, &AppWindow::callback));
  tips.set_tip(*event_box,"Click me!");
    
  /* Yet one more thing you need an X window for ... */
    
  event_box->realize ();
//  event_box->get_window().set_cursor (gdk_cursor_new (GDK_HAND1));

  show_all ();    
}

AppWindow::~AppWindow() {}

gint AppWindow::delete_event_impl (GdkEventAny*)
{
  Gtk::Main::quit();
  return 0;
}


int 
main (int argc, char *argv[])
{
    
    Gtk::Main main (argc, argv);
    AppWindow window;
    
    Gtk::Main::run();
    
    return(0);
}
/* example-end */
