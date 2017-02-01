#include <iostream>
#include <gtk--/button.h>
#include <gtk--/main.h>
#include <gtk--/window.h>

using std::cout;
using std::endl;

using SigC::slot;

class HelloWorld : public Gtk::Window
{
  Gtk::Button m_button;

public:
  HelloWorld();
  
  // this is a callback function. the data arguments are ignored in this example..
  // More on callbacks below.
  void hello();
  
  // When the window is given the "delete_event" signal (this is given
  // by the window manager, usually by the 'close' option, or on the
  // titlebar), this in turn calls the delete_event signal and the
  // delete_event_impl virtual function.  We will override the 
  // virtual function.
  virtual int delete_event_impl(GdkEventAny *event);
  
};


// This is a callback that will hand a widget being destroyed.
void destroy_handler()
{
  Gtk::Main::quit();
}


HelloWorld::HelloWorld()
  : Gtk::Window(GTK_WINDOW_TOPLEVEL), // create a new window
    m_button("Hello World")   // creates a new button with the label "Hello World".
{
  // Here we connect the "destroy" event to a signal handler.  
  // This event occurs when we call gtk_widget_destroy() on the window,
  // or if we return 'false' in the "delete_event" callback.
  destroy.connect(slot(&destroy_handler));

  // Sets the border width of the window.
  set_border_width(10);
          
  // When the button receives the "clicked" signal, it will call the
  // hello() method. The hello() method is defined below.
  m_button.clicked.connect(slot(this, &HelloWorld::hello));
  
  // This will cause the window to be destroyed by calling
  // gtk_widget_destroy(window) when "clicked".  Again, the destroy
  // signal could come from here, or the window manager.
  m_button.clicked.connect(destroy.slot());

  // This packs the button into the window (a gtk container).
  add(m_button);

  // The final step is to display this newly created widget...
  m_button.show();
  
  // and the window
  show();

  // NOTE : These last two lines can be replaced by
  //show_all();
}


void HelloWorld::hello()
{
  cout << "Hello World" << endl;
}


int HelloWorld::delete_event_impl(GdkEventAny *event)
{
  cout << "delete event occured" << endl;

  // if you return false in the "delete_event" signal handler,
  // GTK will emit the "destroy" signal.  Returning true means
  // you don't want the window to be destroyed.
  // This is useful for popping up 'are you sure you want to quit ?'
  // type dialogs.

  // Change true to false and the main window will be destroyed with
  // a "delete_event".
  return true;
}

  
int main (int argc, char *argv[])
{
  // all GTK applications must have a gtk_main(). Control ends here
  // and waits for an event to occur (like a key press or mouse event).
  Gtk::Main kit(argc, argv);

  HelloWorld helloworld;

  kit.run();
  return 0;
}
