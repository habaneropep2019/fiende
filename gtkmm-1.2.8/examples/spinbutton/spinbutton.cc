#include <stdio.h>
#include <gtk--/checkbutton.h>
#include <gtk--/window.h>
#include <gtk--/frame.h>
#include <gtk--/box.h>
#include <gtk--/main.h>
#include <gtk--/spinbutton.h>
#include <gtk--/label.h>
#include <gtk--/adjustment.h>

class ToggleSnap: public Gtk::CheckButton
{
  Gtk::SpinButton* spin_;
public:
  ToggleSnap(Gtk::SpinButton *spin,const Gtk::string& s) 
      : Gtk::CheckButton(s), spin_(spin)
    {}
  void clicked_impl()
    { 
      Gtk::CheckButton::clicked_impl();
      spin_->set_snap_to_ticks(get_active()); 
    }
};

class ToggleNumeric: public Gtk::CheckButton
{
  Gtk::SpinButton* spin_;
public:
  ToggleNumeric(Gtk::SpinButton *spin,const Gtk::string& s) 
      : Gtk::CheckButton(s), spin_(spin)
    {}
  void clicked_impl()
    { 
      Gtk::CheckButton::clicked_impl();
      spin_->set_numeric(get_active()); 
    }
};

class AppWindow : public Gtk::Window
{
  Gtk::SpinButton *spinner1;
  Gtk::Label  *val_label;
public:
  AppWindow();
  ~AppWindow();

  /* It's a good idea to do this for all application windows. */
  gint delete_event_impl (GdkEventAny*)
  {
    Gtk::Main::quit();
    return 0;
  }

  void change_digits( Gtk::SpinButton *spin );
  void get_value(int display );

};


void AppWindow::change_digits( Gtk::SpinButton *spin )
{
  spinner1->set_digits ( spin->get_value_as_int ());
}

void AppWindow::get_value(int display )
{
  gchar buf[32];

  if (display == 1)
    sprintf (buf, "%d", spinner1->get_value_as_int ());
  else
    sprintf (buf, "%0.*f", spinner1->gtkobj()->digits,
             spinner1->get_value_as_float ());
  val_label->set_text (buf);
}


AppWindow::AppWindow()
  : Gtk::Window (GTK_WINDOW_TOPLEVEL)
{
  Gtk::Frame      *frame;
  Gtk::Box        *hbox;
  Gtk::Box        *main_vbox;
  Gtk::Box        *vbox;
  Gtk::Box        *vbox2;
  Gtk::SpinButton *spinner2;
  Gtk::SpinButton *spinner;
  Gtk::Button     *button;
  Gtk::CheckButton *checkbutton;
  Gtk::Label      *label;
  Gtk::Adjustment *adj;

  set_title ("Spin Button");

  main_vbox = manage( new Gtk::VBox (false, 5) );
  main_vbox->set_border_width (10);
  add (*main_vbox);
  
  frame = manage( new Gtk::Frame ("Not accelerated") );
  main_vbox->pack_start (*frame, true, true, 0);
  
  vbox = manage( new Gtk::VBox (false, 0) );
  vbox->set_border_width (5);
  frame->add (*vbox);
  
  /* Day, month, year spinners */
  
  hbox = manage( new Gtk::HBox (false, 0) );
  vbox->pack_start (*hbox, true, true, 5);
  
  vbox2 = manage( new Gtk::VBox (false, 0) );
  hbox->pack_start (*vbox2, true, true, 5);
  
  label = manage( new Gtk::Label ("Day :", 0, 0.5) );
  vbox2->pack_start (*label, false, true, 0);
  
  adj = manage( new Gtk::Adjustment (1.0, 1.0, 31.0, 1.0, 5.0, 0.0) );
  spinner = manage( new Gtk::SpinButton (*adj, 0, 0) );
  spinner->set_wrap (true);
  spinner->set_shadow_type (GTK_SHADOW_OUT);
  vbox2->pack_start (*spinner, false, true, 0);
  
  vbox2 = manage( new Gtk::VBox (false, 0) );
  hbox->pack_start (*vbox2, true, true, 5);
  
  label = manage( new Gtk::Label ("Month :", 0, 0.5) );
  vbox2->pack_start (*label, false, true, 0);
  
  adj = manage( new Gtk::Adjustment (1.0, 1.0, 12.0, 1.0, 5.0, 0.0) );
  spinner = manage( new Gtk::SpinButton (*adj, 0, 0) );
  spinner->set_wrap (true);
  spinner->set_shadow_type (GTK_SHADOW_ETCHED_IN);
  vbox2->pack_start (*spinner, false, true, 0);
  
  vbox2 = manage( new Gtk::VBox (false, 0) );
  hbox->pack_start (*vbox2, true, true, 5);
  
  label = manage( new Gtk::Label ("Year :", 0, 0.5) );
  vbox2->pack_start (*label, false, true, 0);
  
  adj = manage( new Gtk::Adjustment (1998.0, 0.0, 2100.0, 1.0, 100.0, 0.0) );
  spinner = manage( new Gtk::SpinButton (*adj, 0, 0) );
  spinner->set_wrap (false);
  spinner->set_shadow_type (GTK_SHADOW_IN);
  spinner->set_usize (55, 0);
  vbox2->pack_start (*spinner, false, true, 0);
  
  frame = manage( new Gtk::Frame ("Accelerated") );
  main_vbox->pack_start (*frame, true, true, 0);
  
  vbox = manage( new Gtk::VBox(false, 0) );
  vbox->set_border_width (5);
  frame->add (*vbox);
  
  hbox = manage( new Gtk::HBox (false, 0) );
  vbox->pack_start (*hbox, false, true, 5);
  
  vbox2 = manage( new Gtk::VBox (false, 0) );
  hbox->pack_start (*vbox2, true, true, 5);
  
  label = manage( new Gtk::Label ("Value :", 0, 0.5) );
  vbox2->pack_start (*label, false, true, 0);
  
  adj = manage( new Gtk::Adjustment (0.0, -10000.0, 10000.0,
					      0.5, 100.0, 0.0) );
  spinner1 = manage( new Gtk::SpinButton (*adj, 1.0, 2) );
  spinner1->set_wrap (true);
  spinner1->set_usize (100, 0);
  vbox2->pack_start (*spinner1, false, true, 0);
  
  vbox2 = manage( new Gtk::VBox (false, 0) );
  hbox->pack_start (*vbox2, true, true, 5);
  
  label = manage( new Gtk::Label ("Digits :", 0, 0.5) );
  vbox2->pack_start (*label, false, true, 0);
  
  adj = manage( new Gtk::Adjustment (2, 1, 5, 1, 1, 0) );

  spinner2 = manage( new Gtk::SpinButton (*adj, 0.0, 0) );
  spinner2->set_wrap (true);
  adj->value_changed.connect(bind(slot(this, &AppWindow::change_digits),spinner2));
  vbox2->pack_start (*spinner2, false, true, 0);
  
  hbox = manage( new Gtk::HBox (false, 0) );
  vbox->pack_start (*hbox, false, true, 5);
  
  checkbutton = manage( new ToggleSnap (spinner, "Snap to 0.5-ticks"));
  vbox->pack_start (*checkbutton, true, true, 0);
  checkbutton->set_active (true);
  
  checkbutton = manage( new ToggleNumeric (spinner, "Numeric only input mode") );
  vbox->pack_start ( *checkbutton, true, true, 0);
  checkbutton->set_active (true);
  
  val_label = manage( new Gtk::Label ("") );
  
  hbox = manage( new Gtk::HBox (false, 0) );
  vbox->pack_start (*hbox, false, true, 5);

  button = manage( new Gtk::Button ("Value as Int") );
  button->clicked.connect(bind(slot(this,&AppWindow::get_value),1));
  hbox->pack_start (*button, true, true, 5);
  
  button = manage( new Gtk::Button ("Value as Float") );
  button->clicked.connect(bind(slot(this,&AppWindow::get_value),2));
  hbox->pack_start (*button, true, true, 5);
  
  vbox->pack_start (*val_label, true, true, 0);
  val_label->set_text ("0");
  
  hbox = manage( new Gtk::HBox (false, 0));
  main_vbox->pack_start (*hbox, false, true, 0);
  
  button = manage( new Gtk::Button ("Close") );
  button->clicked.connect(Gtk::Main::quit.slot());
  hbox->pack_start (*button, true, true, 5);

  show_all ();
}

AppWindow::~AppWindow() {}

int main( int   argc,
          char *argv[] )
{
  /* Initialise GTK */
  Gtk::Main kit(&argc, &argv);
    
  /* Create a new window */
  AppWindow app;
    
  /* Enter the event loop */
  Gtk::Main::run ();
    
  return(0);
} 

