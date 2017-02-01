#include <gtk--/arrow.h>
#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/window.h>
#include <gtk--/main.h>

class ArrowButton : public Gtk::Button
{
public:
   ArrowButton(GtkArrowType,GtkShadowType);
   ~ArrowButton();
};

/* Create an Arrow widget with the specified parameters
 * and pack it into a button */
ArrowButton::ArrowButton(GtkArrowType arrow_type,GtkShadowType shadow_type)
  : Gtk::Button()
{
  Gtk::Arrow* arrow = manage (new Gtk::Arrow (arrow_type, shadow_type));
  add (*arrow);
}
  
ArrowButton::~ArrowButton() {};

/* We will derive our new application window from window */
class AppWindow : public Gtk::Window
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
  /* Create a new window */
  : Gtk::Window(GTK_WINDOW_TOPLEVEL)
{
  ArrowButton *button;
  Gtk::HBox    *box;

  set_title ("Arrow Buttons");

  /* Sets the border width of the window. */
  set_border_width (10);
  
  /* Create a box to hold the arrows/buttons */
  box=manage (new Gtk::HBox (false, 0));
  box->set_border_width (2);

  /* Pack and show all our widgets */
  button = manage (new ArrowButton (GTK_ARROW_LEFT, GTK_SHADOW_ETCHED_IN));
  box->pack_start (*button, false, false, 3);

  button = manage(new ArrowButton (GTK_ARROW_RIGHT, GTK_SHADOW_ETCHED_IN));
  box->pack_start(*button, false, false, 3);

  button = manage (new ArrowButton (GTK_ARROW_LEFT, GTK_SHADOW_ETCHED_OUT));
  box->pack_start (*button, false, false, 3);

  button = manage (new ArrowButton (GTK_ARROW_RIGHT, GTK_SHADOW_ETCHED_OUT));
  box->pack_start (*button, false, false, 3);
 
  add (*box);
  show_all ();
}

AppWindow::~AppWindow() {};

int main (int argc, char *argv[])
{
  Gtk::Main kit(argc, argv);
  AppWindow arrows;

  kit.run ();
  return 0;
}
