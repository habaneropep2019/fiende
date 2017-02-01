#include <vector>
#include <list>
#include <gtk--/label.h>
#include <gtk--/button.h>
#include <gtk--/frame.h>
#include <gtk--/main.h>
#include <gnome--/app.h>
#include <gnome--/appbar.h>
#include <gnome--/about.h>

class Hello_App : public Gnome::App
{
public:
  Hello_App(const string &message,
	    const string &geometry,
	    const vector<string> &greet);

  Hello_App(const string &message); // used by new_app_cb()

  ~Hello_App();

protected:

  void init();

  void install_menus_and_toolbar();
  void close();

  // Callbacks
  virtual int delete_event_impl(GdkEventAny *event);
  void button_click_cb(Gtk::Label *label);
  void nothing_cb();
  void new_app_cb();
  void about_cb();
  gint about_close_cb();

  void exit_cb();

  static list<Hello_App*> instances_;
  
  // widgets
  Gtk::Button button_;
  Gtk::Label label_;
  Gnome::AppBar status_;
  Gtk::Frame frame_;

  Gnome::About *about_;
};

