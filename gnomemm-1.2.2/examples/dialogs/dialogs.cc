/* dialogs.cc
 *
 * Copyright 2000 Karl Nelson
 *
 */


#include <gnome.h>
#include <gtk--/button.h>
#include <gtk--/frame.h>
#include <gtk--/table.h>
#include <gnome--/app.h>
#include <gnome--/main.h>

using SigC::slot;

class App : public Gnome::App
  {
    public:
      App();
      virtual ~App();
  
     static void make_ok();
     static void make_warning();
     static void make_error();
     static void make_ok_cancel();
     static void make_ok_cancel_m();
     static void make_question();
     static void make_question_m();
     static void make_request();
     static void make_request_h();

     static void reply(int i) { cout << "Reply "<<i <<endl;}
     static void stringcb(string i) { cout << "String "<<i <<endl;}

   protected:
     void init();

     //Override:
     gint delete_event_impl(GdkEventAny*);     
  };

App::App()
  : Gnome::App("GnomeDialogs", "Gnome Dialogs")
  {
    init();
  }

App::~App()
  {}

void App::init()
  {
    Gtk::Table *table=manage(new Gtk::Table(3,3));
    Gtk::Button *button;
    button=manage(new Gtk::Button("Ok"));
    button->clicked.connect(slot(&App::make_ok));
    table->attach(*button,0,1,0,1);

    button=manage(new Gtk::Button("Warning"));
    button->clicked.connect(slot(&App::make_warning));
    table->attach(*button,0,1,1,2);

    button=manage(new Gtk::Button("Error"));
    button->clicked.connect(slot(&App::make_error));
    table->attach(*button,0,1,2,3);

    button=manage(new Gtk::Button("Ok_Cancel"));
    button->clicked.connect(slot(&App::make_ok_cancel));
    table->attach(*button,1,2,0,1);

    button=manage(new Gtk::Button("Ok_Cancel(M)"));
    button->clicked.connect(slot(&App::make_ok_cancel_m));
    table->attach(*button,2,3,0,1);

    button=manage(new Gtk::Button("Question"));
    button->clicked.connect(slot(&App::make_question));
    table->attach(*button,1,2,1,2);

    button=manage(new Gtk::Button("Question(M)"));
    button->clicked.connect(slot(&App::make_question_m));
    table->attach(*button,2,3,1,2);

    button=manage(new Gtk::Button("Request"));
    button->clicked.connect(slot(&App::make_request));
    table->attach(*button,1,2,2,3);

    button=manage(new Gtk::Button("Request(H)"));
    button->clicked.connect(slot(&App::make_request_h));
    table->attach(*button,2,3,2,3);
    
    Gtk::Frame *frame=manage(new Gtk::Frame);
    frame->add(*table);
    frame->set_shadow_type(GTK_SHADOW_IN);

    set_contents(*frame);
    show_all();
  }

void App::make_ok() 
  { Gnome::Dialogs::ok("Everything is 5 by 5."); }
void App::make_warning() 
  { Gnome::Dialogs::warning("Danger! Danger!"); }
void App::make_error()  
  { Gnome::Dialogs::error("Ack! My heart!"); }
void App::make_ok_cancel() 
  { Gnome::Dialogs::ok_cancel("Format hardrive?",slot(&App::reply)); }
void App::make_ok_cancel_m() 
  { Gnome::Dialogs::ok_cancel_modal("Format hardrive?",slot(&App::reply)); } 
void App::make_question()
  { Gnome::Dialogs::question("Do you like green eggs and ham?",slot(&App::reply)); } 
void App::make_question_m()
  { Gnome::Dialogs::question_modal("Do you like green eggs and ham?",slot(&App::reply)); } 
void App::make_request()
  { Gnome::Dialogs::request(false, "What is your name?", "John Doe", 50, slot(&App::stringcb)); }
void App::make_request_h()
  { Gnome::Dialogs::request(true, "Password?", "", 50, slot(&App::stringcb)); }

gint App::delete_event_impl(GdkEventAny*)
{ 
  Gtk::Main::quit();
  return 0; 
}

int main(int argc, char* argv[])
{
  Gnome::Main kit("GnomeDialogs", "0.1", argc, argv );
  App myApp;
  kit.run();

  return 0;
}

