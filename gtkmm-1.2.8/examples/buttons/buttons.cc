#include <iostream>
#include <gtk--/main.h>
#include <gtk--/window.h>
#include <gtk--/button.h>

using std::cout;

using SigC::slot;
using SigC::bind;

class Buttons : public Gtk::Window
{
public:
  Gtk::Button *m_button;

  Buttons();
  ~Buttons();

  void callback(char *data) 
    {
      cout << "Hello again - " << data << " was pressed" << endl;
    }

  gint delete_event_impl(GdkEventAny*) { 
    Gtk::Main::quit(); return 0; 
  }
  
};

Buttons::Buttons() 
{
  m_button = manage(new Gtk::Button());
  m_button -> add_pixlabel("info.xpm", "cool button");
   
  set_title("Pixmap'd buttons!");
  set_border_width(10);
  
  m_button->clicked.connect(bind<char*>(slot(this, &Buttons::callback),
                                        "cool button"));
  
  add(*m_button);
  show_all();
}

Buttons::~Buttons() {};

int main (int argc, char *argv[])
{
  Gtk::Main kit(argc, argv);

  Buttons buttons;

  kit.run();
  return 0;
}
