#include <iostream>
#include <gtk--/button.h>
#include <gtk--/box.h>
#include <gtk--/main.h>
#include <gtk--/window.h>
#include <map>


class TimerExample : public Gtk::Window
{
  // the usual stuff - nothing exciting
  Gtk::HBox   m_box;
  Gtk::Button m_add_timer, m_del_timer, m_quit;
  gint m_t_nr;

  // the start value for our timer
  static const gint COUNT_VALUE;

  // the timeout value for the timers in [ms]
  static const gint TIMEOUT_VALUE;

  // we need this to store our connections
  map<gint,Gtk::Connection> m_timers;

  // this is for storing our timer values
  // each timer countsa back from COUNT_VALUE to 0 and
  // if removed when it reaches 0
  map<gint,gint> m_counters;

public:
  TimerExample();
  
  // the callback functions for add & remove button
  void add_timer_pressed();
  void del_timer_pressed();

  // the callback for the timer
  // note that is not of the type gint callback(void)
  // since we use bind() to add a data value of type gint to it
  gint timer_callback(gint timer_nr);
  
  // this will end the application when its window is closed
  gint delete_event_impl(GdkEventAny*);
};

const gint TimerExample::COUNT_VALUE = 5;
const gint TimerExample::TIMEOUT_VALUE = 1500;


TimerExample::TimerExample() :
  m_add_timer("add a new timer"),
  m_del_timer("remove timer"),
  m_quit("Quit"),
  m_box(true,10),
  m_t_nr(0)
{
  // connect the callbacks
  m_quit.pressed.connect(Gtk::Main::quit.slot());
  m_add_timer.pressed.connect(slot(this,&TimerExample::add_timer_pressed));
  m_del_timer.pressed.connect(slot(this,&TimerExample::del_timer_pressed));

  // put buttons into container
  m_box.pack_start(m_add_timer);
  m_box.pack_start(m_del_timer);
  m_box.pack_start(m_quit);

  // set border and display all
  set_border_width(10);
  add(m_box);
  show_all();
}


void TimerExample::add_timer_pressed()
{
  // creation of a new object prevents long lines and
  // shows us a little how slots work
  // we have 0 parameters and gint as return value after calling bind 
  SigC::Slot0<gint> my_slot = bind(slot(this,&TimerExample::timer_callback),m_t_nr);
  
  // now connect the slot to Gtk::Main::timeout
  Gtk::Connection conn = Gtk::Main::timeout.connect(my_slot,TIMEOUT_VALUE);

  // memorize connection
  m_timers[m_t_nr] = conn;

  // initialize timer count
  m_counters[m_t_nr] = COUNT_VALUE + 1;

  // print some information on the console
  cout << "added timeout " << m_t_nr++ << endl;
}


void TimerExample::del_timer_pressed()
{
  // are there any timers ?
  if(m_timers.empty()) {
    // nope
    cout << "sorry, there are no timers left" << endl;
  } else {
    // get the nr of the first timer
    gint timer_nr = m_timers.begin()->first;
    // give a little information to the user
    cout << "removing timer " << timer_nr << endl;
    // delete the entry in the counter values
    m_counters.erase(timer_nr);
    // destroy the connection !!!!!
    // this is important since the connection is NOT destroyed when
    // the according Connection-Object is deleted
    // The purpose of the connection object is to give you the
    // possibility to destroy a connection without having to destroy
    // either the sender or the receiver
    // Try it and comment out the following line ....
    m_timers[timer_nr].disconnect();
    // destroy the connection
    m_timers.erase(timer_nr);
  }
}


gint TimerExample::timer_callback(gint timer_nr)
{
  // print the timernr
  cout << "This is timer " << timer_nr;
  // decrement & check counter value
  if(--m_counters[timer_nr] == 0) {
    cout << " boom" << endl;
    // delete the counter entry
    m_counters.erase(timer_nr);
    // delete the connection entry
    m_timers.erase(timer_nr);
    // note that we do not need to call disconnect on the connection
    // since we Gtk::Main does this for us when we return 0
    return 0;
  }

  // print the timer value
  cout << " - " << m_counters[timer_nr] << "/" << COUNT_VALUE << endl;
  return 1;
}


// the intresting stuff ends here


gint TimerExample::delete_event_impl(GdkEventAny*)
{ 
  Gtk::Main::quit(); return 0;
}



int main (int argc, char *argv[])
{
  Gtk::Main app(argc, argv);

  TimerExample example;

  app.run();
  return 0;
}
