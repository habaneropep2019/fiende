#include <gnome--.h>

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

void foo(int i)
  {
    cout << i <<endl;
  };

void foo2(Gtk::Widget *w)
  {
    cout << w << endl;
  }

MyApp::MyApp() : Gnome::App("MyApp","MyApp")
  {
    using namespace Gnome;

    UI::Info radioitems[]=
      {
        UI::Item("Item1",bind(slot(&foo),1)),
        UI::Item("Item2",bind(slot(&foo),2)),
        UI::Item("Item3",bind(slot(&foo),3)),
        UI::Item("Item4",bind(slot(&foo),4))
      };

    UI::Info radioitems2[]=
      {
        UI::Item("Item1",slot(&foo2)),
        UI::Item("Item2",slot(&foo2))
      };


    UI::Info radio_menu[]=
      { 
        UI::Item("Not Radio"),
        UI::Separator(),
        UI::RadioTree(radioitems),
        UI::Separator(),
        UI::RadioTree(radioitems2)
      };

    UI::SubTree menus[]=
       { 
         UI::Menu("Items",radio_menu)
       };
 
    create_menus(menus);
    show();
  }

gint MyApp::delete_event_impl(GdkEventAny*)
  { 
    Gtk::Main::quit();
    return FALSE;  
  }

int main (int argc, char* argv[])
  {
    Gnome::Main kit("radio-info-demo", "0.0", argc, argv);
    MyApp app;
    kit.run();
    return 0;
  }


