/*
 *
 *  This documents the usage of Gtk::Menu_Helpers.
 *  peter
 *
 */

#include <iostream>
#include <gtk--/accelgroup.h>
#include <gtk--/box.h>
#include <gtk--/main.h>
#include <gtk--/menu.h>
#include <gtk--/menubar.h>
#include <gtk--/eventbox.h>
#include <gtk--/window.h>

//From <iostream> 
using std::cout;
using std::endl;

using SigC::slot;
using SigC::bind;

using namespace Gtk;

void void_void_cb(void)
{
  cout << "reached void_void_callback" << endl;
}

void void_int_cb(int num)
{
  cout << "reached void_int_callback, num = " << num << endl;
}

void edit_cb(Gtk::string action)
{
  cout << "reached edit_callback, requested action " << action << endl;
}
void toggle_cb(Gtk::string action)
{
  cout << "reached toggle_callback, requested action " << action << endl;
}
void destroy_cb(void)
{
  Main::quit();
}
gint popup(GdkEventButton* event,Menu* menu)
{
  menu->popup(event->button,event->time);
  return true;
}


int main(int argc, char *argv[])
{
   Main kit(argc, argv);
   Menu    menu_popup;

   // Make a huge menu bar with submenus and the works!
   MenuBar *menubar                  = manage( new MenuBar());
   // Activate the helpers for easy menu setup.
   {
     using namespace Menu_Helpers;

     // Create the file menu
     Menu    *menu_file      = manage( new Menu());
     MenuList& list_file     = menu_file->items();
     
     // Connecting non-default callback, getting the element's widget
     list_file.push_back(MenuElem("_New"));
     list_file.back()->activate.connect(bind(slot(void_int_cb), 123));
                    // ^^^^^^^^^^^^^^^^

     // Usage of widget functions on menuitems
     list_file.push_back(MenuElem("_Open", slot(void_void_cb)));
     list_file.back()->set_state(GTK_STATE_INSENSITIVE);
            // ^^^^^^^^

     // Modifying a slot 
     list_file.push_back(MenuElem("Save", bind(slot(void_int_cb), 234)));
                                     //   ^^^^^^^^^^^^^^^^^^^^^^

     list_file.push_back(MenuElem("Save As",slot(void_void_cb)));

       // setting an accelerator directly
       list_file.push_back(MenuElem("_Close", CTL|'c', slot(void_void_cb)));
                                           // ^^^^^^^

     list_file.push_back(SeparatorElem());
     
     // setting an accelerator with a string
     list_file.push_back(MenuElem("_Quit", "<control>q", slot(destroy_cb)));
                                        // ^^^^^^^^^^^^

     // Create a submenu
     Menu *menu_sub = manage( new Menu());
     MenuList& list_sub = menu_sub->items();
     list_sub.push_back(MenuElem("Sub1"));
     list_sub.push_back(MenuElem("Sub2"));
     list_sub.push_back(MenuElem("Sub3"));


     // Create the edit menu
     Menu *menu_edit                = manage( new Menu());
     MenuList& list_edit     = menu_edit->items();
     list_edit.push_back(MenuElem("Cut"));
     list_edit.push_back(MenuElem("Copy"));
     list_edit.push_back(MenuElem("Paste"));
     list_edit.push_back(MenuElem("Options",*menu_sub));


     // Create the example menu
     Menu *menu_examples = manage( new Menu());
     MenuList& list_examples = menu_examples->items();
     RadioMenuItem::Group gr;
     list_examples.push_back(RadioMenuElem(gr,"RadioItem Example 1", 
                     bind<Gtk::string>(slot(toggle_cb), "radio example1")));
     list_examples.push_back(RadioMenuElem(gr,"RadioItem Example 2", 
                     bind<Gtk::string>(slot(toggle_cb), "radio example2")));
     list_examples.push_back(RadioMenuElem(gr,"RadioItem Example 3", 
                     bind<Gtk::string>(slot(toggle_cb), "radio example3")));
     static_cast<RadioMenuItem*>(list_examples.back())->set_active();
     list_examples.push_back(SeparatorElem());
     list_examples.push_back(CheckMenuElem("CheckItem Example", 
                     bind(slot(toggle_cb), (Gtk::string)"check example")));


     // Create the help menu
     Menu    *menu_help                = manage( new Menu());
     menu_help->items().push_back(MenuElem("About", slot(void_void_cb)));


     // Create the menu bar
     //   Gtk+ does not have O(1) tail lookups so you should build menus 
     //   backwards whenever you plan to make lots of access to back().
     MenuList& list_bar = menubar->items();
     list_bar.push_front(MenuElem("_Help","<control>h",*menu_help));
     list_bar.front()->right_justify();
     menubar->items().push_front(MenuElem("E_xamples","<control>x",*menu_examples));
     menubar->items().push_front(MenuElem("_Edit","<control>e",*menu_edit));
     menubar->items().push_front(MenuElem("_File","<control>f",*menu_file));
   }

   // vbox
   VBox *vbox = manage( new VBox() );
   vbox->pack_start (*menubar, false, false);

   // eventbox 
   EventBox *ebox = manage( new EventBox() );
   ebox->button_press_event.connect( bind(slot(&popup), &menu_popup) );
   vbox->pack_end(*ebox);

   Window *window = manage( new Window() );
   window->destroy.connect(slot(destroy_cb));
   window->set_title ("MenuDemo");
   window->set_default_size (600, 400);
   window->add (*vbox);
   window->set_policy (false, true, false);
   window->show_all();

   // Make a popup window
   {
     using namespace Menu_Helpers;
     MenuList& list_popup = menu_popup.items();
     list_popup.push_back(MenuElem("Floa_t 1","<control>t",
                                   slot(&void_void_cb)));
     list_popup.push_back(MenuElem("Float 2"));
     list_popup.push_back(MenuElem("Float 3"));
   }
   // the popup menu needs to be told where to place its accelerators
   menu_popup.accelerate(*window);


   kit.run();
}
