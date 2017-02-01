/* example-start arrow arrow.c */

#include <gtk--/arrow.h>
#include <gtk--/table.h>
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
  Gtk::Table   *table;

  set_title ("Arrow Buttons");

  /* Sets the border width of the window. */
  set_border_width (10);
  
  /* Create a box to hold the arrows/buttons */
  table=manage (new Gtk::Table (3,3,true));
  table->set_border_width (2);

  /* Pack and show all our widgets */
  button = manage (new ArrowButton (GTK_ARROW_UP, GTK_SHADOW_ETCHED_IN));
  table->attach (*button, 1, 2, 0, 1);

  button = manage (new ArrowButton (GTK_ARROW_LEFT, GTK_SHADOW_ETCHED_IN));
  table->attach (*button, 0, 1, 1, 2);

  button = manage (new ArrowButton (GTK_ARROW_RIGHT, GTK_SHADOW_ETCHED_IN));
  table->attach (*button, 2, 3, 1, 2);

  button = manage (new ArrowButton (GTK_ARROW_DOWN, GTK_SHADOW_ETCHED_IN));
  table->attach (*button, 1, 2, 2, 3);

  table->set_row_spacing(0,5);
  table->set_row_spacing(1,5);

  table->set_col_spacing(0,5);
  table->set_col_spacing(1,5);
 
  add (*table);
  show_all ();
}

AppWindow::~AppWindow() {};

int main (int argc, char *argv[])
{
  Gtk::Main myapp(&argc, &argv);
  AppWindow arrows;

  myapp.run ();
  return 0;
}

/* example-end */
