// This program demonstrates the proper way to create a modal
// dialog from scratch.
//
// Note, that you must connect to the button clicked event and 
// not any of the button events.  Connecting to the button events
// means the dialog will be launched in an event loop and Gtk::Kit::run
// won't function properly.
//
#include <gtk--.h>

using SigC::slot;

class MyDialog: public Gtk::Window
  {
      Gtk::Entry* entry;
      bool canceled;
      void okay();
      void cancel();
      gint key(GdkEventKey* key);
    public:
      MyDialog(const string &label);
      ~MyDialog();
      string run();
      gint delete_event_impl(GdkEventAny*) {return true;}
  };

MyDialog::MyDialog(const string &l)
  : canceled(false)
  {
    set_modal(true);

    entry=manage(new Gtk::Entry);
    entry->key_press_event.connect(slot(*this,&MyDialog::key));

    Gtk::Button *okay_b=manage(new Gtk::Button("Okay"));
    Gtk::Button *cancel_b=manage(new Gtk::Button("Cancel"));
    okay_b->clicked.connect(slot(*this,&MyDialog::okay));
    cancel_b->clicked.connect(slot(*this,&MyDialog::cancel));

    Gtk::Box *box2=manage(new Gtk::HBox());
    box2->pack_start(*okay_b,false,false,10);
    box2->pack_end(*cancel_b,false,false,10);

    Gtk::Box *box=manage(new Gtk::VBox());
    box->pack_start(*manage(new Gtk::Label(l)),true,true,10);
    box->pack_start(*entry,true,true,10);
    box->pack_start(*box2,true,true,10);
    add(*box);
    set_border_width(10);
    set_usize(200,150);
    show_all();
  }

MyDialog::~MyDialog() {}

string MyDialog::run()
  {
    Gtk::Kit::run();
    if (canceled) 
      return "";
    return entry->get_text(); 
  }

void MyDialog::cancel()
  {
    canceled=true;
    Gtk::Kit::quit();
  }

void MyDialog::okay()
  {
    Gtk::Kit::quit();
  }

gint MyDialog::key(GdkEventKey* ke)
  {
    if (ke&&ke->keyval==GDK_Return) Gtk::Kit::quit();
    return false;
  }

/******************************************************/

void pop_dialog()
  {
    MyDialog dialog("What is your name?");
    cout << dialog.run() <<endl;
  }

int
main(int argc,char **argv)
  {
    Gtk::Kit kit(argc,argv);
    Gtk::Window win;
    Gtk::VBox box;
    Gtk::Button b1("Pop dialog");
    Gtk::Button b2("Quit");
 
    b1.clicked.connect(SigC::slot(&pop_dialog)); 
    b2.clicked.connect(Gtk::Kit::quit.slot()); 
    box.pack_start(b1); 
    box.pack_start(b2); 
    win.add(box);
    win.show_all();
    kit.run();
    return 0;
  }
