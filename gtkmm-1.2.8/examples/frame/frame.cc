#include <gtk--/button.h>
#include <gtk--/frame.h>
#include <gtk--/main.h>
#include <gtk--/window.h>

class AppWindow : Gtk::Window
{
public: 
  AppWindow();
  ~AppWindow();

  /* It's a good idea to do this for all application windows. */
  gint delete_event_impl (GdkEventAny*)
  {
    Gtk::Main::quit();
    return 0;
  }

};

AppWindow::AppWindow()
  : Gtk::Window (GTK_WINDOW_TOPLEVEL)
{
  Gtk::Frame* frame;

  /* Set some window properties */
  set_title("Frame Example");
  set_usize(300, 300);

  /* Here we connect the "destroy" event to a signal handler */ 
  destroy.connect (Gtk::Main::quit.slot());

  /* Sets the border width of the window. */
  set_border_width (10);

  /* Create a Frame */
  frame = manage( new Gtk::Frame() );
  add(*frame);

  /* Set the frames label */
  frame->set_label( "GTK Frame Widget" );

  /* Align the label at the right of the frame */
  frame->set_label_align( 1.0, 0.0);

  /* Set the style of the frame */
  frame->set_shadow_type(GTK_SHADOW_ETCHED_OUT);

  /* Show the frame */
  frame->show();

  /* show this window */
  show ();

}

AppWindow::~AppWindow() {}

int main( int   argc,
          char *argv[] )
{
  /* Initialise GTK */
  Gtk::Main kit(&argc, &argv);
    
  /* Create a new window */
  AppWindow app;
    
  /* Enter the event loop */
  Gtk::Main::run ();
    
  return(0);
}
