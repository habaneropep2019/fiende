#include <gtk--.h>
#include <gnome--.h>

#include "mdi-window.h"

Prefs::Prefs()
{
  Gtk::VBox *vbox = manage(new Gtk::VBox());
  Gtk::Label *label = manage(new Gtk::Label("MDI Mode"));
 
  defaultMDI = manage(new Gtk::RadioButton("Default MDI"));
  defaultMDI->clicked.connect(slot(this, &Prefs::changed));

  toplevelMDI = manage(new Gtk::RadioButton("Toplevel MDI")); 
  toplevelMDI->set_group(defaultMDI->group()); 
  toplevelMDI->clicked.connect(slot(this, &Prefs::changed));

  notebookMDI = manage(new Gtk::RadioButton("Notebook MDI")); 
  notebookMDI->set_group(defaultMDI->group()); 
  notebookMDI->clicked.connect(slot(this, &Prefs::changed));

  modalMDI = manage(new Gtk::RadioButton("Modal MDI")); 
  modalMDI->set_group(defaultMDI->group()); 
  modalMDI->clicked.connect(slot(this, &Prefs::changed));

  vbox->pack_start(*defaultMDI);
  vbox->pack_start(*toplevelMDI);
  vbox->pack_start(*notebookMDI);
  vbox->pack_start(*modalMDI);
  vbox->show_all();

  append_page(*vbox, *label);

  apply.connect(slot(this, &Prefs::apply_prefs));
}

Prefs::~Prefs()
{
  mdi_object->prefs_window_closed();
}

void
Prefs::apply_prefs(int page)
{
  if (defaultMDI->get_active())
    mdi_object->set_mode(Gnome::MDI::DEFAULT);
  else if (toplevelMDI->get_active())
    mdi_object->set_mode(Gnome::MDI::TOPLEVEL);
  else if (notebookMDI->get_active())
    mdi_object->set_mode(Gnome::MDI::NOTEBOOK);
  else if (modalMDI->get_active())
    mdi_object->set_mode(Gnome::MDI::MODAL);

}
void
Prefs::start()
{
// Note that we are using the actual mdi mode,
// the app should remember if DEFAULT_MDI was used
// and use this value instead.
// if NOTEBOOK mode was set using DEFAULT_MDI,
// we will get NOTEBOOK here, not DEFAULT.
  switch (((Gnome::MDI *)mdi_object)->gtkobj()->mode) {
    case Gnome::MDI::DEFAULT:   defaultMDI->set_active(TRUE);
                                break;
    case Gnome::MDI::TOPLEVEL:  toplevelMDI->set_active(TRUE);
                                break;
    case Gnome::MDI::NOTEBOOK:  notebookMDI->set_active(TRUE);
                                break;
    case Gnome::MDI::MODAL:     modalMDI->set_active(TRUE);
                                break;
  }

  show();
}



CustomMDILabel::CustomMDILabel(const string &name) :
  label(name)
{
  pixmap = manage(new Gnome::StockPixmap(GNOME_STOCK_MENU_EXEC));
  pack_start(*pixmap);
  pack_start(label);
  show_all();
}


Gtk::Widget* CustomMDIChild::create_title_impl()
{
  return manage(new CustomMDILabel(get_name()));
}

void CustomMDIChild::update_title_impl(Gtk::Widget &old_label)
{
  ((CustomMDILabel &)old_label).set_name(get_name());
}


CustomMDIChild::CustomMDIChild(gint number)
{
  cout << this <<endl;
  vector<Gnome::UI::SubTree> menus;
  vector<Gnome::UI::Info> childMenuTree;
  char buf[20];
  
  sprintf(buf, "Child %i", number);
  set_name(buf);
  
  using namespace Gnome::UI;
  childMenuTree.push_back(Item(Icon(GNOME_STOCK_MENU_NEW),
                               N_("New backup"),
                               slot(this, &CustomMDIChild::nothing_cb),
                               N_("New backup")));
  childMenuTree.push_back(Item(Icon(GNOME_STOCK_MENU_PROP),
                               N_("Add partition"),
                               slot(this, &CustomMDIChild::nothing_cb),
                               N_("Add partition")));

  childMenuTree.push_back(Separator());
                        
  childMenuTree.push_back(Item(Icon(GNOME_STOCK_MENU_EXEC),
                               N_("Backup all data"),
                               slot(this, &CustomMDIChild::nothing_cb),
                               N_("Backup all data")));

  menus.push_back(Menu(N_("Child Menu"), childMenuTree));

  create_menus(menus);

  child_number = number;
  view_number = 0;
}

Gtk::Widget *
CustomMDIChild::create_view_impl()
{
  CustomMDIView * view = manage(new CustomMDIView(child_number, ++view_number));
  view->add_view.connect(slot(this, &CustomMDIChild::create_view));
  return view;
}

void
CustomMDIChild::nothing_cb()
{
  cout << "nothing here" << endl;
}

CustomMDIView::CustomMDIView(gint child, gint view) 
{
  char buf[20];
  
  sprintf(buf, "Child %i View %i", child, view);
  label.set_text(buf);
  label.set_usize(100, 100);

  pack_start(label);
  show_all();
}




void
MDIWindow::nothing_cb()
{
  cout << "nothing here" << endl;
}

void
MDIWindow::prefs_window_closed()
{
  properties = 0;
}


void
MDIWindow::preferences()
{
  if (properties == 0)
  {
    properties = manage (new Prefs());
    properties->mdi_object = this;
  }

  properties->start();
}

void
MDIWindow::quit_cb()
{
  //  clear()
  //  this->destroy()
  //  and then putting "Gnome::Main::quit" in the cleanup function
  //  should also work
  clear();
  Gnome::Main::quit();
}
gint 
MDIWindow::delete_event_impl(GdkEventAny* e)
{
  quit_cb();

  /* Prevent the window's destruction, since we destroyed it 
   * ourselves with quit_cb()
   */
  return true;
}

void
MDIWindow::cleanup()
{
   quit_cb();
//  Gnome::Main::quit();
}

void
MDIWindow::new_child()
{
  CustomMDIChild *child;
  
  child = manage(new CustomMDIChild(++child_number));
  
  add(*child);

  child->create_view();
}

void
MDIWindow::new_view()
{
  if (get_active_child() != 0)
    get_active_child()->create_view();
}


void
MDIWindow::close_child()
{
  if (get_active_child() != 0)
    remove(* get_active_child());
}

void 
MDIWindow::dump_children()
{
  for (MDIList::iterator i=children().begin();
       i!=children().end();
       ++i)
    cout << *i <<endl;
}

void 
MDIWindow:: app_created_impl(Gnome::App& app)
{
   app.set_statusbar(*manage(new Gtk::Statusbar));
   app.install_menu_hints();

   app.set_default_size(400, 300);
   app.set_policy(false, true, false);

   cout << &app <<endl;
}

MDIWindow::MDIWindow()
 : Gnome::MDI("mdi-test", "The great MDI test")
{
  vector<Gnome::UI::SubTree> menus;
  vector<Gnome::UI::Info> fileMenuTree, prefMenuTree, windowsMenuTree;
  vector<Gnome::UI::Info> toolbarTree;

  {
    using namespace Gnome::UI;
    fileMenuTree.push_back(Item(Icon(GNOME_STOCK_MENU_NEW),
                                N_("New Child"),
                                slot(this, &MDIWindow::new_child),
                                N_("New Child")));

    fileMenuTree.push_back(Item(Icon(GNOME_STOCK_MENU_NEW),
                                N_("New View"),
                                slot(this, &MDIWindow::new_view),
                                N_("New View for current Child")));

    fileMenuTree.push_back(Item(N_("Dump Children"),
                                slot(this, &MDIWindow::dump_children),
                                N_("Print addresses of all children")));

    fileMenuTree.push_back(Separator());
  }
                        
  {
    using namespace Gnome::MenuItems;
    fileMenuTree.push_back(Close(slot(this, &MDIWindow::close_child)));
    fileMenuTree.push_back(Exit (slot(this, &MDIWindow::quit_cb)));
    prefMenuTree.push_back(Preferences(slot(this, &MDIWindow::preferences)));
  }

  {
    using namespace Gnome::Menus;
    menus.push_back(File(fileMenuTree));
    menus.push_back(Settings(prefMenuTree));
    menus.push_back(Windows(windowsMenuTree));
  }

  set_menubar_template(menus);

  {
    using namespace Gnome::UI;
    toolbarTree.push_back(Item(Icon(GNOME_STOCK_PIXMAP_NEW),
                                N_("New Child"),
                                slot(this, &MDIWindow::new_child),
                                N_("New Child")));

    toolbarTree.push_back(Item(Icon(GNOME_STOCK_PIXMAP_NEW),
                                N_("New View"),
                                slot(this, &MDIWindow::new_view),
                                N_("New View for current Child")));
    toolbarTree.push_back(Separator());

    toolbarTree.push_back(Item(Icon(GNOME_STOCK_PIXMAP_PREFERENCES),
                                N_("Preferences"),
                                slot(this, &MDIWindow::preferences),
                                N_("Configure Program")));

    toolbarTree.push_back(Separator());

    toolbarTree.push_back(Item(Icon(GNOME_STOCK_PIXMAP_CLOSE),
                                N_("Close"),
                                slot(this, &MDIWindow::close_child),
                                N_("Close Child")));

    toolbarTree.push_back(Item(Icon(GNOME_STOCK_PIXMAP_QUIT),
                                N_("Quit"),
                                slot(this, &MDIWindow::quit_cb),
                                N_("Quit Program")));

  }


  set_toolbar_template(toolbarTree);

  destroy.connect(slot(this, &MDIWindow::cleanup));

  // we want it insert the child menu after the File menu.
  set_child_menu_path("_File");
  set_child_list_path("_Windows/");

  child_number = 0;
  properties = 0;
}  
