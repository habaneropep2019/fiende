/* example-start fixed fixed.c */

#include <gtk--/button.h>
#include <gtk--/fixed.h>
#include <gtk--/window.h>
#include <gtk--/main.h>

class AppWindow : public Gtk::Window
{
  gint x;
  gint y;

public:
  AppWindow ();
  ~AppWindow ();

  void move_button(Gtk::Fixed*,Gtk::Button*);

  /* Here we connect the "destroy" event to a signal handler */ 
  virtual gint delete_event_impl (GdkEventAny*);
};

AppWindow::AppWindow()
  : Gtk::Window(GTK_WINDOW_TOPLEVEL), x(50), y(50)
{
  Gtk::Fixed *fixed;
  Gtk::Button *button;
  gint i;

  /* Create a new window */
  set_title("Fixed Container");

  /* Sets the border width of the window. */
  set_border_width (10);

  /* Create a Fixed Container */
  fixed = manage( new Gtk::Fixed() );
  add(*fixed);
  
  for (i = 1 ; i <= 3 ; i++) {
    /* Creates a new button with the label "Press me" */
    button = manage( new Gtk::Button("Press me") );
  
    /* When the button receives the "clicked" signal, it will call the
     * function move_button() passing it the Fixed Containter as its
     * argument. */
    button->clicked.connect(bind(slot(this,&AppWindow::move_button),fixed,button));
  
    /* This packs the button into the fixed containers window. */
    fixed->put (*button, i*50, i*50);
  }

  show_all ();
}

AppWindow::~AppWindow() {}

gint AppWindow::delete_event_impl (GdkEventAny*)
{
  Gtk::Main::quit();
  return 0;
}


/* This callback function moves the button to a new position
 * in the Fixed container. */
void AppWindow::move_button( Gtk::Fixed *fixed,
                  Gtk::Button *button )
{
  x = (x+30)%300;
  y = (y+50)%300;
  fixed->move(*button, x, y); 
}

int main( int   argc,
          char *argv[] )
{
  /* Initialise GTK */
  Gtk::Main app(&argc, &argv);
  AppWindow window;
    
  /* Enter the event loop */
  Gtk::Main::run ();
    
  return(0);
}
/* example-end */
