#include <gtk--/main.h>
#include <gtk--/window.h>
#include <gtk--/box.h>
#include <gtk--/separator.h>
#include <gtk--/button.h>
#include <gtk--/radiobutton.h>
// Gtk-- version of the radiobuttons example from the gtk+ tutorial

class RadioButtons : public Gtk::Window
{
public:

  Gtk::VBox m_box1, m_box2, m_box3;
  Gtk::RadioButton m_rb1, m_rb2, m_rb3;
  Gtk::HSeparator m_seperator;
  Gtk::Button m_closeButton;
  
  RadioButtons();
  
  gint delete_event_impl(GdkEventAny*) { 
    Gtk::Main::quit(); return 0; 
  }

  void close_application() 
    {
      delete_event_impl(0);
    }

};

RadioButtons::RadioButtons() :
  m_box1(false, 0),
  m_box2(false, 10),
  m_box3(false, 10),
  m_rb1("button1"),
  m_rb2("button2"),
  m_rb3("button3"),
  m_closeButton("close")
{
  set_title("radio buttons");
  set_border_width(0);

  m_rb2.set_group(m_rb1.group());
  m_rb3.set_group(m_rb1.group());
  
  add(m_box1);
  
  m_box2.set_border_width(10);
  m_box1.pack_start(m_box2);
  
  m_box2.pack_start(m_rb1);
  m_box2.pack_start(m_rb2);
  m_rb2.set_active(true);
  
  m_box2.pack_start(m_rb3);
  
  m_box1.pack_start(m_seperator, false, true, 0);
  
  m_box3.set_border_width(10);
  m_box1.pack_start(m_box3, false, true, 0);
  
  m_closeButton.clicked.connect(slot(this,&RadioButtons::close_application));
  
  m_box3.pack_start(m_closeButton);
  m_closeButton.set_flags(GTK_CAN_DEFAULT);
  m_closeButton.grab_default();
  
  show_all();
}

int main (int argc, char *argv[])
{
          
  // all GTK applications must have a gtk_main(). Control ends here
  // and waits for an event to occur (like a key press or mouse event).
  Gtk::Main myapp(&argc, &argv);

  RadioButtons radiobuttons;

  myapp.run();
  return 0;
}
