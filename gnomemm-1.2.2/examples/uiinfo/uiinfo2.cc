#include <vector>
#include <list>
#include <gnome--.h>

// Demonstration of creating menu templates with STL types

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

    // we can use vectors
    vector<UI::Info> file_menu;
    file_menu.push_back(MenuItems::Open());
    file_menu.push_back(MenuItems::Save());
    file_menu.push_back(MenuItems::SaveAs());
    file_menu.push_back(UI::Separator());
    file_menu.push_back(MenuItems::Close());
    file_menu.push_back(MenuItems::Exit());

    // or we can use lists
    list<UI::Info> edit_menu;
    edit_menu.push_back(MenuItems::Cut());
    edit_menu.push_back(MenuItems::Copy());
    edit_menu.push_back(MenuItems::Paste());
    edit_menu.push_back(MenuItems::SelectAll());
    edit_menu.push_back(MenuItems::Clear());
    edit_menu.push_back(MenuItems::Undo());
    edit_menu.push_back(MenuItems::Redo());
    edit_menu.push_back(MenuItems::Find());
    edit_menu.push_back(MenuItems::FindAgain());
    edit_menu.push_back(MenuItems::Replace());
    edit_menu.push_back(MenuItems::Properties());

    vector<UI::Info> help_menu;
    help_menu.push_back(MenuItems::About());

    vector<UI::SubTree> menus;
    menus.push_back(Menus::File(file_menu));
    menus.push_back(Menus::Edit(edit_menu));
    menus.push_back(Menus::Help(help_menu));
 
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


