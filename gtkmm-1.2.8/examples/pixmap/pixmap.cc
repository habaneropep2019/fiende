#include <iostream>
#include <gtk--/window.h>
#include <gtk--/main.h>
#include <gtk--/pixmap.h>
#include <gtk--/button.h>

// Gtk-- version of the "pixmap" example from the gtk+ tutorial

/* XPM data of Open-File icon */
static const char * xpm_data[] = {
  "16 16 3 1",
  "       c None",
  ".      c #000000000000",
  "X      c #FFFFFFFFFFFF",
  "                ",
  "   ......       ",
  "   .XXX.X.      ",
  "   .XXX.XX.     ",
  "   .XXX.XXX.    ",
  "   .XXX.....    ",
  "   .XXXXXXX.    ",
  "   .XXXXXXX.    ",
  "   .XXXXXXX.    ",
  "   .XXXXXXX.    ",
  "   .XXXXXXX.    ",
  "   .XXXXXXX.    ",
  "   .XXXXXXX.    ",
  "   .........    ",
  "                ",
  "                "
};

class Pixmap : public Gtk::Window
{
  Gtk::Button m_button;
  Gtk::Pixmap m_pixmap;

  gint delete_event_impl(GdkEventAny*) { 
    Gtk::Main::quit(); return 0; 
  }

public:
  Pixmap();
  void button_clicked() { cout << "button clicked" << endl; }

};

Pixmap::Pixmap() :
  m_pixmap(xpm_data)
{
  add(m_button);
  m_button.add(m_pixmap);

  m_button.clicked.connect(slot(this, &Pixmap::button_clicked));
  
  show_all();
}


int main (int argc, char *argv[])
{
          
  Gtk::Main myapp(argc, argv);

  Pixmap pixmap;

  myapp.run();
  return 0;
}
