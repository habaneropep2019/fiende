#include <iostream>
#include <gtk--/button.h>
#include <gtk--/main.h>
#include <gtk--/table.h>
#include <gtk--/window.h>

// Gtk-- version of the table packing example from the gtk+ tutorial

using SigC::bind;
using SigC::slot;
using std::cout;
using std::endl;

class MyWin : public Gtk::Window
{
public:
  MyWin();
  
  Gtk::Table m_table;
  Gtk::Button m_b1, m_b2, m_bQuit;

  void callback(char* data);

  gint delete_event_impl(GdkEventAny*) { 
    Gtk::Main::quit(); return 0; 
  }

};

MyWin::MyWin() :
  m_table(2, 2, true),
  m_b1("button 1"),
  m_b2("button 2"),
  m_bQuit("Quit")
{
  set_title("Table");
  set_border_width(20);
  
  add(m_table);

  m_table.attach(m_b1, 0, 1, 0, 1);
  m_table.attach(m_b2, 1, 2, 0, 1);
  m_table.attach(m_bQuit, 0, 2, 1, 2);

  m_b1.clicked.connect(bind<char*>(slot(this, &MyWin::callback), "button 1"));
  m_b2.clicked.connect(bind<char*>(slot(this, &MyWin::callback), "button 2"));
  m_bQuit.clicked.connect(Gtk::Main::quit.slot());
  // the cast is needed to "help" template instantiation

  show_all();
}

void
MyWin::callback(char* data)
{
  cout << "Hello again - " << data << " was pressed" << endl;
}


int main(int argc, char *argv[])
{
  Gtk::Main myapp(&argc, &argv);
  MyWin mywin;

  myapp.run();
  return 0;
}


