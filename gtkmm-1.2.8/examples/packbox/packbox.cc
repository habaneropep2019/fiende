#include <iostream>
#include <cstdio>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/main.h>
#include <gtk--/label.h>
#include <gtk--/separator.h>
#include <gtk--/window.h>

using std::cerr;
using std::endl;

// Gtk-- version of the "packbox" example from the gtk+ tutorial


class PackBox : public Gtk::HBox
{
public:
  PackBox(bool homogeneous, gint spacing, bool expand, bool fill, gint padding);
  ~PackBox() { delete m_button6; }

  Gtk::Button m_button1, m_button2, m_button3, m_button4, m_button5,
    *m_button6;
  char padstr[80];
};

PackBox::PackBox(bool homogeneous, gint spacing, bool expand, bool fill, gint padding) :
  Gtk::HBox(homogeneous, spacing),
  m_button1("gtk_box_pack"),
  m_button2("(box,"),
  m_button3("button,"),
  m_button4(expand ? "true," : "false,"),
  m_button5(fill ? "true," : "false,")
{
  pack_start(m_button1, expand, fill,padding);
  pack_start(m_button2, expand, fill,padding);
  pack_start(m_button3, expand, fill,padding);
  pack_start(m_button4, expand, fill,padding);
  pack_start(m_button5, expand, fill,padding);

  sprintf(padstr, "%d);", padding);
  
  m_button6 = new Gtk::Button(padstr);
  pack_start(*m_button6, expand, fill,padding);
}

  

class PackBoxDemo : public Gtk::Window
{
public:
  Gtk::Button m_button;
  Gtk::VBox m_box1;
  Gtk::HBox m_boxQuit;
  Gtk::Button m_buttonQuit;
  
  Gtk::HSeparator m_seperator1, m_seperator2, m_seperator3, m_seperator4, m_seperator5;
  
  PackBoxDemo(int which);
  ~PackBoxDemo();
  
  // You should always remember to connect the destroy signal to the
  // main window.  This is very important for proper intuitive
  // behavior 
  gint delete_event_impl(GdkEventAny*) { 
    Gtk::Main::quit(); return 0; 
  }
  
};

PackBoxDemo::PackBoxDemo(int which) :
  m_box1(false, 0),
  m_boxQuit(false, 0),
  m_buttonQuit("Quit")
{
  Gtk::Label *m_label1, *m_label2;
  PackBox *m_packbox1, *m_packbox2, *m_packbox3,
    *m_packbox4, *m_packbox5;

  switch(which) {
  case 1:
    // create a new label.
    m_label1 = manage(new Gtk::Label("hbox(false, 0);"));

    // Align the label to the left side.  We'll discuss this function and 
    // others in the section on Widget Attributes. 
    m_label1->set_alignment(0, 0);
    
    // Pack the label into the vertical box (vbox box1).  Remember that 
    // widgets added to a vbox will be packed one on top of the other in
    // order. 
    m_box1.pack_start(*m_label1, false, false, 0);

    // Create a PackBox - homogeneous = false, spacing = 0,
    // expand = false, fill = false, padding = 0 
    m_packbox1 = manage(new PackBox(false, 0, false, false, 0));
    m_box1.pack_start(*m_packbox1, false, false, 0);

    // Create a PackBox - homogeneous = false, spacing = 0,
    // expand = true, fill = false, padding = 0 
    m_packbox2 = manage(new PackBox(false, 0, false, true, 0));
    m_box1.pack_start(*m_packbox2, false, false, 0);

    // Create a PackBox - homogeneous = false, spacing = 0,
    // expand = true, fill = true, padding = 0 
    m_packbox3 = manage(new PackBox(false, 0, true, true, 0));
    m_box1.pack_start(*m_packbox3, false, false, 0);
    
    // pack the separator into the vbox.  Remember each of these
    // widgets are being packed into a vbox, so they'll be stacked
    // vertically. 
    m_box1.pack_start(m_seperator1, false, true, 5);
    
    // create another new label, and show it.
    m_label2 = manage(new Gtk::Label("hbox(true, 0);"));
    m_label2->set_alignment(0, 0);
    m_box1.pack_start(*m_label2, false, false, 0);
    
    // Args are: homogeneous, spacing, expand, fill, padding
    m_packbox4 = manage(new PackBox(true, 0, true, false, 0));
    m_box1.pack_start(*m_packbox4, false, false, 0);

    // Args are: homogeneous, spacing, expand, fill, padding
    m_packbox5 = manage(new PackBox(true, 0, true, false, 0));
    m_box1.pack_start(*m_packbox5, false, false, 0);

    m_box1.pack_start(m_seperator2, false, true, 5);
    
    break;
    
  case 2:

    m_label1 = manage(new Gtk::Label("hbox(false, 10);"));
    m_label1->set_alignment(0, 0);
    m_box1.pack_start(*m_label1, false, false, 0);

    m_packbox1 = manage(new PackBox(false, 10, true, false, 0));
    m_box1.pack_start(*m_packbox1, false, false, 0);
    
    m_packbox2 = manage(new PackBox(false, 10, true, true, 0));
    m_box1.pack_start(*m_packbox2, false, false, 0);

    m_box1.pack_start(m_seperator1, false, true, 5);


    m_label2 = manage(new Gtk::Label("hbox(false, 10);"));
    m_label2->set_alignment(0, 0);
    m_box1.pack_start(*m_label2, false, false, 0);

    m_packbox3 = manage(new PackBox(false, 0, true, false, 10));
    m_box1.pack_start(*m_packbox3, false, false, 0);
    
    m_packbox4 = manage(new PackBox(false, 0, true, true, 10));
    m_box1.pack_start(*m_packbox4, false, false, 0);

    m_box1.pack_start(m_seperator2, false, true, 5);

    break;
    
  case 3:

    // This demonstrates the ability to use Gtk::Box::pack_end() to
    // right justify widgets.  First, we create a new box as before. 
    m_packbox1 = manage(new PackBox(false, 0, false, false, 0));
    // create the label that will be put at the end. 
    m_label1 = manage(new Gtk::Label("end"));
    // pack it using pack_end(), so it is put on the right side
    // of the PackBox. 
    m_packbox1->pack_end(*m_label1, false, false, 0);
    
    m_box1.pack_start(*m_packbox1, false, false, 0);
    
    // this explicitly sets the separator to 400 pixels wide by 5 pixels
    // high.  This is so the hbox we created will also be 400 pixels wide,
    // and the "end" label will be separated from the other labels in the
    // hbox.  Otherwise, all the widgets in the hbox would be packed as
    // close together as possible. 
    m_seperator1.set_usize(400, 5);
    
    // pack the separator into ourselves 
    m_box1.pack_start(m_seperator1, false, true, 5);
  }
  
  // setup the signal to destroy the window.  Remember that this will send
  // the "destroy" signal to the window which will be caught by our signal
  // handler as defined above. 
  m_buttonQuit.clicked.connect(Gtk::Main::quit.slot());

  // pack the button into the quitbox.
  // The last 3 arguments to gtk_box_pack_start are: expand, fill, padding. 
  m_boxQuit.pack_start(m_buttonQuit, true, false, 0);
  m_box1.pack_start(m_boxQuit, false, false, 0);
  
  // pack the vbox (box1) which now contains all our widgets, into the
  // main window. 
  add(m_box1);

  show_all();
  
}

PackBoxDemo::~PackBoxDemo()
{}


int main (int argc, char *argv[])
{
          
  // all GTK applications must have a gtk_main(). Control ends here
  // and waits for an event to occur (like a key press or mouse event).
  Gtk::Main myapp(&argc, &argv);

  if (argc != 2) {
    cerr << "usage: packbox num, where num is 1, 2, or 3." << endl;
    // this just does cleanup in GTK, and exits with an exit status of 1. 
    gtk_exit (1);
  }

  PackBoxDemo packboxdemo(atoi(argv[1]));

  myapp.run();
  return 0;
}
