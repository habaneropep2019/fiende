// This demonstrated how to create a singleton dialog.
#include <gtk--/main.h>
#include <gtk--/window.h>
#include <gtk--/dialog.h>
#include <gtk--/button.h>
#include <iostream>

using namespace Gtk;
using SigC::slot;

class MyDialog : public Dialog {

public:
  // create one dialog only
  static MyDialog* instance() {
    if( _instance == 0 ) {
      _instance = new MyDialog;
    }
    return _instance;
  }

protected:
  MyDialog()
  {
    set_usize(100, 100);
    set_title("Little Dialog");
 
    Gtk::Button *button=manage(new Gtk::Button("Ok"));
    get_action_area()->pack_start(*button);
    button->clicked.connect(hide.slot());
    button->show();
  }

  // don't close the window, hide it
  virtual gint delete_event_impl(GdkEventAny* p0)
  {
    hide();
    return 1;
  }

private:
  static MyDialog* _instance;
};

MyDialog* MyDialog::_instance(0);


class MyWindow : public Window {

public:
  MyWindow() {

    set_usize(200, 200);

    Box* mainBox = manage(new HBox);
    MyDialog* dialog = MyDialog::instance();
    Button* button
      = manage(new Button("Show Dialog"));
    button
      ->clicked.connect(dialog->show.slot());
    mainBox->pack_start(*button);
    add(*mainBox);
  }

protected:
  virtual gint delete_event_impl(GdkEventAny* p0)
  {
    Kit::quit();
    return 0;
  }

};

int main(int argc, char* argv[])
{
  Kit kit(argc, argv);
  MyWindow win;
  win.show_all();
  kit.run();
  return 0;
}

