#include <iostream>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/checkbutton.h>
#include <gtk--/entry.h>
#include <gtk--/main.h>
#include <gtk--/window.h>

// Gtk-- version of the "entry" example from the gtk+ tutorial

class Entry : public Gtk::Window
{
  Gtk::HBox m_hbox;
  Gtk::VBox m_vbox;
  Gtk::Entry m_entry;
  Gtk::Button m_bClose;
  Gtk::CheckButton m_cbEditable, m_cbVisible;

  void toggleEditable() { m_entry.set_editable(m_cbEditable.get_active()); }
  
  void toggleVisibility() { m_entry.set_visibility(m_cbVisible.get_active()); }
  void enterCallback();

  gint delete_event_impl(GdkEventAny*) { 
    Gtk::Main::quit(); return 0; 
  }
  
public:
  Entry();
};

Entry::Entry() :
  m_hbox(false, 0),
  m_vbox(false, 0),
  m_entry(50),
  m_bClose("Close"),
  m_cbEditable("Editable"),
  m_cbVisible("Visible")
{

  set_usize(200, 100);
  set_title("Gtk-- Entry");
  
  add(m_vbox);
  
  m_entry.activate.connect(slot(this, &Entry::enterCallback));
  m_entry.set_text("hello");
  m_entry.append_text(" world");
  m_entry.select_region(0, m_entry.get_text_length());
  m_vbox.pack_start(m_entry);
  
  // Note that add() can also be used instead of pack_xxx()
  m_vbox.add(m_hbox);

  m_hbox.pack_start(m_cbEditable);
  m_cbEditable.toggled.connect(slot(this, &Entry::toggleEditable));
  m_cbEditable.set_active(true);

  m_hbox.pack_start(m_cbVisible);
  m_cbVisible.toggled.connect(slot(this,&Entry::toggleVisibility));
  m_cbVisible.set_active(true);

  m_bClose.clicked.connect(Gtk::Main::quit.slot());
  m_vbox.pack_start(m_bClose);
  m_bClose.set_flags(GTK_CAN_DEFAULT);
  m_bClose.grab_default();
  
  show_all();
}

void
Entry::enterCallback()
{
  cout << "Entry contents : " << m_entry.get_text() << endl;
}

int main (int argc, char *argv[])
{
  Gtk::Main myapp(&argc, &argv);

  Entry entry;

  myapp.run();
  return 0;
}

