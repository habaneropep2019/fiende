
#include <stdio.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gtk--/main.h>
#include <gtk--/toolbar.h>
#include <gtk--/button.h>
#include <gtk--/togglebutton.h>
#include <gtk--/radiobutton.h>
#include <gtk--/box.h>
#include <gtk--/window.h>

using namespace SigC;

class MainWindowClass : public Gtk::Window {
private:
  Gtk::HBox main_hbox;
  Gtk::HBox hbox;
  Gtk::Button button;
  Gtk::Toolbar toolbar;
  
  void quit_pressed_cb(void);
  void toolbar_button_cb(char *);
  
public:

   MainWindowClass (void);
  ~MainWindowClass (void);

};

void MainWindowClass::toolbar_button_cb(char *c)
{
  printf("toolbar_button_cb : %s\n",c);
}

MainWindowClass::MainWindowClass(void) : Gtk::Window(GTK_WINDOW_TOPLEVEL),
  main_hbox(false,0),
  hbox(false,0),
  button("Quit")
{

  set_usize(400,50);
  
  add(main_hbox);

  main_hbox.add(hbox);
  
  hbox.pack_start(button,false,false,0);
  hbox.pack_start(toolbar,false,false,0);

  button.clicked.connect( slot(this,&MainWindowClass::quit_pressed_cb) );

  {
  using namespace Gtk::Toolbar_Helpers;
  toolbar.tools().push_back(ButtonElem( "Click me", 
    bind<char*>( slot(this,&MainWindowClass::toolbar_button_cb),
    "'Click me' button"),
    "toolbar btn",""));
  toolbar.tools().push_back(Space());

  toolbar.tools().push_back(ButtonElem( "Click me too", 
    bind<char*>( slot(this, &MainWindowClass::toolbar_button_cb),
    "'Click me too' button"),
    "other toolbar btn", ""));

  toolbar.tools().push_back(ToggleElem( "Toggle me", 
    bind<char*>( slot(this, &MainWindowClass::toolbar_button_cb), 
    "This is from a toggle connector"),
    "toggle duh", ""));

  Gtk::RadioButton::Group gr;
  toolbar.tools().push_back(RadioElem(gr,"R1"));
  toolbar.tools().push_back(RadioElem(gr,"R2"));
  toolbar.tools().push_back(RadioElem(gr,"R3"));
  }

  toolbar.show();
  button.show();
  hbox.show();
  main_hbox.show();
}

MainWindowClass::~MainWindowClass(void)
{}

void MainWindowClass::quit_pressed_cb(void)
{
  Gtk::Main::quit();
}

int main(gint argc, gchar **argv)
{
  Gtk::Main kit(argc,argv);
  MainWindowClass main_window;

  main_window.show();
  
  kit.run();

  return(0);
}
