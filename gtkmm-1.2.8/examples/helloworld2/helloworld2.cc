#include <iostream>
#include <gtk--/window.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/main.h>

using std::cout;

using SigC::bind;
using SigC::slot;

class HelloWorld : public Gtk::Window
{
  Gtk::HBox m_box1;
  Gtk::Button m_button1, m_button2;

public:
  HelloWorld();
  
  // Our new improved callback. (see below)
  void callback(char* data);
  
  gint delete_event_impl(GdkEventAny*) { 
    Gtk::Main::quit(); return 0; 
  }
  
};

HelloWorld::HelloWorld() :
  // Gtk::Window(GTK_WINDOW_TOPLEVEL) : not needed.
  // GTK_WINDOW_TOPLEVEL is the constructor arg's default value
  m_box1(false, 0), // creates a box to pack widgets into
  m_button1("Button 1"),
  m_button2("Button 2")
{

  // this is a new call, this just sets the title of our new window to
  // "Hello Buttons!"
  set_title("Hello Buttons!");

  // sets the border width of the window.
  set_border_width(10);

  // put the box into the main window.
  add(m_box1);

  // Now when the button is clicked, we call the "callback" function
  // with a pointer to "button 1" as it's argument
  m_button1.clicked.connect(bind<char*>(slot(this, &HelloWorld::callback), "button 1"));

  // instead of gtk_container_add, we pack this button into the invisible
  // box, which has been packed into the window.
  // note that the pack_start default arguments are true, true, 0
  m_box1.pack_start(m_button1);
  
  // always remember this step, this tells GTK that our preparation
  // for this button is complete, and it can be displayed now.
  m_button1.show();

  // call the same callback function with a different argument,
  // passing a pointer to "button 2" instead.
  m_button2.clicked.connect(bind<char*>(slot(this, &HelloWorld::callback), "button 2"));

  m_box1.pack_start(m_button2);

  // The order in which we show the buttons is not really important,
  // but I recommend showing the window last, so it all pops up at
  // once.
  m_button2.show();
  
  m_box1.show();
  
  show();

  // NOTE : These lines can be replaced by
  // show_all();

}

// Our new improved callback.  The data passed to this method is
// printed to stdout.

// Note an important difference with the gtk+ version : you have to
// specify the correct type of the argument you intend to pass, or
// connect_to_method() won't compile because the compiler won't be
// able to instantiate the template correctly (unless you cast, of
// course)
void HelloWorld::callback(char* data)
{
  cout << "Hello World - " << (char*)data
       << " was pressed" << endl;
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
