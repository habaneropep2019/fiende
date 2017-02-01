#include <gnome--.h>

// Demonstration of how to create menu templates using arrays

using SigC::slot;
using SigC::bind;


class MyApp : public Gnome::App
  {
    public:
      MyApp();

    protected:

      //Override:
      gint delete_event_impl(GdkEventAny*);
  };

MyApp::MyApp() : Gnome::App("MyApp","MyApp")
  {
    using namespace Gnome;

    UI::Info file_menu[]=
      {
        MenuItems::Open(),
        MenuItems::Save(),
        MenuItems::SaveAs(),
        UI::Separator(),
        MenuItems::Close(),
        MenuItems::Exit()
      };

    UI::Info edit_menu[]=
      { 
        MenuItems::Cut(),
        MenuItems::Copy(),
        MenuItems::Paste(),
        MenuItems::SelectAll(),
        MenuItems::Clear(),
        MenuItems::Undo(),
        MenuItems::Redo(),
        MenuItems::Find(),
        MenuItems::FindAgain(),
        MenuItems::Replace(),
        MenuItems::Properties()
      };

    UI::Info help_menu[]=
      { 
        MenuItems::About()
      };

    UI::SubTree menus[]=
       { 
         Menus::File(file_menu), 
         Menus::Edit(edit_menu), 
         Menus::Help(help_menu)
       };
 
    create_menus(menus);

    set_statusbar(*manage(new Gtk::Statusbar));
    install_menu_hints();

    set_default_size(400, 300);
    set_policy(false, true, false);

    show();
  }

gint MyApp::delete_event_impl(GdkEventAny*)
  { 
    Gtk::Main::quit();
    return FALSE; 
  }

int main (int argc, char* argv[])
  {
    Gnome::Main kit("myapp-demo", "0.0", argc, argv);
    MyApp app;
    kit.run();
    return 0;
  }


