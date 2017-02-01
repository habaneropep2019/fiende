#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/main.h>
#include <gtk--/window.h>
#include <gtk--/statusbar.h>

// Gtk-- version of the "statusbar" example from the gtk+ tutorial

class StatusBar : public Gtk::Window
{
  Gtk::VBox m_vbox;
  Gtk::Button m_bPush, m_bPop;
  Gtk::Statusbar m_status_bar;

  // Note : m_context_id has to be declared *after* m_status_bar to be
  // initialized in the constructor's init list
  unsigned int m_context_id,
    m_count;  
 
  void push_item(unsigned int context_id);
  void pop_item(unsigned int context_id) { m_status_bar.pop(context_id); }
  
 public:
  StatusBar();

  /* It's a good idea to do this for all application windows. */
  gint delete_event_impl (GdkEventAny*)
  {
    Gtk::Main::quit();
    return 0;
  }

};


StatusBar::StatusBar() :
  m_vbox(false, 1),
  m_bPush("push item"),
  m_bPop("pop last item"),
  m_context_id(m_status_bar.get_context_id("Statusbar example")),
  m_count(1)
{
  set_usize(200, 100);
  set_title("Gtk-- Statusbar Example");
  
  add(m_vbox);
  
  m_vbox.pack_start(m_status_bar);

  m_bPush.clicked.connect(bind(slot(this, &StatusBar::push_item), m_context_id));
  m_vbox.pack_start(m_bPush);
  
  m_bPop.clicked.connect(bind(slot(this, &StatusBar::pop_item), m_context_id));
  m_vbox.pack_start(m_bPop);
  
  show_all();
}

void
StatusBar::push_item(unsigned int context_id)
{
  char buff[20];

  g_snprintf(buff, 20, "Item %d", m_count++);
  m_status_bar.push(context_id, buff);
}

int main (int argc, char *argv[])
{
  Gtk::Main myapp(argc, argv);

  StatusBar statusbar;

  myapp.run();
  return 0;
}

