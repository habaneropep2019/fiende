#include <iostream>
#include <gtk--/adjustment.h>
#include <gtk--/button.h>
#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gtk--/progressbar.h>
#include <gtk--/main.h>
#include <gtk--/window.h>
#include <map>


class IdleExample : public Gtk::Window
{
  // the usual stuff - nothing exciting
  Gtk::Button m_quit;
  Gtk::Adjustment  m_percentage_c;
  Gtk::ProgressBar m_progressbar_c;
  Gtk::Adjustment  m_percentage_d;
  Gtk::ProgressBar m_progressbar_d;

public:
  IdleExample();
  
  // a timer-function
  gint timer_callback();
  // a idle-function
  gint idle_callback();
  
  gint delete_event_impl(GdkEventAny*);
};


IdleExample::IdleExample() :
  m_quit("Quit"),
  m_percentage_c(0,0,100,0.5),
  m_progressbar_c(m_percentage_c),
  m_percentage_d(0,0,5000,0.5),
  m_progressbar_d(m_percentage_d)
{
  // connect the callbacks
  m_quit.pressed.connect(Gtk::Main::quit.slot());

  // put buttons into container
  Gtk::VBox *vbox = manage( new Gtk::VBox(false,5));

  // adding a few widgets
  vbox->pack_start(* manage(new Gtk::Label("Formatting windows drive C:")));
  vbox->pack_start(* manage(new Gtk::Label("100 MB")));
  vbox->pack_start(m_progressbar_c);
  m_progressbar_c.set_show_text(true);

  vbox->pack_start(* manage(new Gtk::Label("")));

  vbox->pack_start(* manage(new Gtk::Label("Formatting windows drive D:")));
  vbox->pack_start(* manage(new Gtk::Label("5000 MB")));
  vbox->pack_start(m_progressbar_d);
  m_progressbar_d.set_show_text(true);

  Gtk::HBox *hbox = manage( new Gtk::HBox(false,10));
  hbox->pack_start(m_quit, true, false);
  vbox->pack_start(*hbox);

  // set border and display all
  set_border_width(5);
  add(*vbox);
  show_all();
   
  // formattinf drive c in timeout callback ;-)
  Gtk::Main::timeout.connect(slot(this,&IdleExample::timer_callback), 50);

  // formatting drive d in idle callback ;-)
  Gtk::Main::idle.connect(slot(this,&IdleExample::idle_callback));
}


// increase the progressbar's value and remove callback when done
gint IdleExample::timer_callback()
{
  float value = m_percentage_c.get_value();
  m_percentage_c.set_value(value + 0.5);
  return value < 99.99;
}


// increase the progressbar's value and remove callback when done
// note the diffrence in speed and also the impact of system load
// try to increase system load and watch the drive d value
gint IdleExample::idle_callback()
{
  float value = m_percentage_d.get_value();
  m_percentage_d.set_value(value + 0.5);
  return value < 4999.99;
}


gint IdleExample::delete_event_impl(GdkEventAny*)
{ 
  Gtk::Main::quit(); return 0;
}


int main (int argc, char *argv[])
{
  Gtk::Main app(argc, argv);

  IdleExample example;

  app.run();
  return 0;
}
