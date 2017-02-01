#include <gtk--/box.h>
#include <gtk--/frame.h>
#include <gtk--/label.h>
#include <gtk--/window.h>
#include <gtk--/main.h>

class AppWindow : Gtk::Window
{
public: 
  AppWindow();
  ~AppWindow();

  /* It's a good idea to do this for all application windows. */
  gint delete_event_impl (GdkEventAny*)
  {
    Gtk::Main::quit();
    return 0;
  }

};

AppWindow::AppWindow()
  : Gtk::Window (GTK_WINDOW_TOPLEVEL)
{
  Gtk::Box *hbox;
  Gtk::Box *vbox;
  Gtk::Frame* frame;
  Gtk::Label* label;

  /* Set some window properties */
  set_title("Label Example");
  set_border_width (5);

  /* Here we connect the "destroy" event to a signal handler */ 
  destroy.connect (Gtk::Main::quit.slot());

  vbox = manage( new Gtk::VBox (false, 5) );
  hbox = manage( new Gtk::HBox (false, 5) );
  add (*hbox);
  hbox->pack_start (*vbox, false, false);

  frame = manage( new Gtk::Frame ("Normal Label") );
  label = manage( new Gtk::Label ("This is a Normal label") );
  frame->add (*label);
  vbox->pack_start (*frame, false, false);
  
  frame = manage( new Gtk::Frame ("Multi-line Label") );
  label = manage( new Gtk::Label ("This is a Multi-line label.\nSecond line\n" \
			 "Third line"));
  frame->add (*label);
  vbox->pack_start (*frame, false, false);

  frame = manage( new Gtk::Frame ("Left Justified Label") );
  label = manage( new Gtk::Label ("This is a Left-Justified\n" \
			 "Multi-line label.\nThird      line"));
  label->set_justify (GTK_JUSTIFY_LEFT);
  frame->add (*label);
  vbox->pack_start (*frame, false, false);
  
  frame = manage( new Gtk::Frame ("Right Justified Label"));
  label = manage( new Gtk::Label ("This is a Right-Justified\nMulti-line label.\n" \
			 "Fourth line, (j/k)"));
  label->set_justify (GTK_JUSTIFY_RIGHT);
  frame->add (*label);
  vbox->pack_start (*frame, false, false);

  vbox = manage( new Gtk::VBox (false, 5) );
  hbox->pack_start (*vbox, false, false);

  frame = manage( new Gtk::Frame ("Line wrapped label"));
  label = manage( new Gtk::Label ("This is an example of a line-wrapped label.  It " \
			 "should not be taking up the entire             " /* big space to test spacing */\
			 "width allocated to it, but automatically " \
			 "wraps the words to fit.  " \
			 "The time has come, for all good men, to come to " \
			 "the aid of their party.  " \
			 "The sixth sheik's six sheep's sick.\n" \
			 "     It supports multiple paragraphs correctly, " \
			 "and  correctly   adds "\
			 "many          extra  spaces. "));
  label->set_line_wrap (true);
  frame->add (*label);
  vbox->pack_start (*frame, false, false);
  
  frame = manage( new Gtk::Frame("Filled, wrapped label") );
  label = manage( new Gtk::Label("This is an example of a line-wrapped, filled label.  " \
			 "It should be taking "\
			 "up the entire              width allocated to it.  " \
			 "Here is a seneance to prove "\
			 "my point.  Here is another sentence. "\
			 "Here comes the sun, do de do de do.\n"\
			 "    This is a new paragraph.\n"\
			 "    This is another newer, longer, better " \
			 "paragraph.  It is coming to an end, "\
			 "unfortunately.") );
  label->set_justify (GTK_JUSTIFY_FILL);
  label->set_line_wrap (true);
  frame->add (*label);
  vbox->pack_start (*frame, false, false);
  
  frame = manage( new Gtk::Frame("Underlined label") );
  label = manage( new Gtk::Label("This label is underlined!\n"
			 "This one is underlined in quite a funky fashion") );
  label->set_justify (GTK_JUSTIFY_LEFT);
  label->set_pattern ("_________________________ _ _________ _ ______     __ _______ ___");
  frame->add (*label);
  vbox->pack_start (*frame, false, false);
  
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
