/*  Gnome-- MDI Example
    Copyright (C) 2000 The Gtk-- Development Team

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* This example illustrates the use of the Gnome-- Multi Document Interface
 * (MDI). MDIs enable application programmers to present multiple documents
 * to the user with an arbitrary number of "views" connected to each.
 */

#include <iostream>
#include <string>
#include <vector>

#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/label.h>
#include <gtk--/pixmap.h>
#include <gtk--/separator.h>
#include <gnome--/main.h>
#include <gnome--/mdi.h>


class MyMDILabel : public Gtk::HBox
{
public:
  MyMDILabel(const string &name);

  void set_name(const string &name) { label.set_text(name); }

protected:
  Gtk::Pixmap pixmap;
  Gtk::Label label;
};

class MyMDIView : public Gtk::VBox
{
public:
  MyMDIView(int child_number, int view_number);

  SigC::Signal0<void> add_view;

protected:
  Gtk::HBox hbox;
  Gtk::Label label;
  Gtk::Button new_child;
  Gtk::Button new_view;
};

class MyMDI : public Gnome::MDI
{
public:
  MyMDI();

  void add_child();
  
protected:
  void nothing_cb() { cout << "File/Exit selected" << endl; }

  //override:
  virtual gint delete_event_impl(GdkEventAny *event);
};

class MyMDIChild : public Gnome::MDIChild
{
public:
  MyMDIChild();

  /* This method just forwards the call to the super-classes method. We need
   * it for connecting signals to it.
   * But I don't understand why you can't just connect to the base class's create_view method. murrayc.
   */
  void create_view() { Gnome::MDIChild::create_view(); }

protected:

  /* Gnome::MDIChild is a virtual class that must be inherited by a custom
   * class, implementing at least create_view_impl(). This method returns a
   * pointer to a widget that represents the document contents.
   */
  virtual Gtk::Widget *create_view_impl();

  /* By implementing set_label_impl() it is possible to have custom labels
   * for the views in the notebook and menu. Note that you have to implement
   * both variants of this method. The former variant creates a new label
   * widget, while the latter updates an existing label.
   */
  virtual Gtk::Widget* create_title_impl();

  virtual void update_title_impl(Gtk::Widget &old_label);

  int child_number;
  int view_number;

private:
  static int child_count;
};

int MyMDIChild::child_count = 0;


MyMDI* mdi_ptr = 0;
char buffer[80];

////////////////////////////////////////////////////////////////////////////

MyMDI::MyMDI() : Gnome::MDI("GnomeMDI", "GnomeMDI Title")
{
  vector<Gnome::UI::SubTree> menus;
  vector<Gnome::UI::Info> fileMenuTree;
  fileMenuTree.push_back(Gnome::MenuItems::Exit
                        (slot(this, &MyMDI::nothing_cb)));
  menus.push_back(Gnome::Menus::File(fileMenuTree));
  set_menubar_template(menus);
}

void MyMDI::add_child()
{
  MyMDIChild* child=new MyMDIChild;
  add(*child);
  child->create_toplevel_view();
}

gint 
MyMDI::delete_event_impl(GdkEventAny* e)
{
  Gtk::Main::quit();
  
  return false;
}




MyMDIChild::MyMDIChild() :
  child_number(++child_count),
  view_number(0)
{
  snprintf(buffer, sizeof(buffer), "Child #%d", child_number);
  set_name(buffer);
}

Gtk::Widget *MyMDIChild::create_view_impl()
{
  MyMDIView* v=new MyMDIView(child_number, ++view_number);
  v->add_view.connect(slot(this, &MyMDIChild::create_view));
  return v;
}

Gtk::Widget* MyMDIChild::create_title_impl()
{
  return new MyMDILabel(get_name());
}

void MyMDIChild::update_title_impl(Gtk::Widget &old_label)
{
  ((MyMDILabel &)old_label).set_name(get_name());
}


MyMDIView::MyMDIView(int child_number, int view_number) :
  new_child("New Child"),
  new_view("New View")
{
  snprintf(buffer, sizeof(buffer), "This is view #%d of child #%d.",
          view_number, child_number);
  label.set_text(buffer);

  pack_start(label);
  pack_start(hbox);
  hbox.pack_start(new_child);
  hbox.pack_start(new_view);
  new_child.clicked.connect(slot(mdi_ptr, &MyMDI::add_child));
  new_view.clicked.connect(add_view.slot());
  show_all();
}


MyMDILabel::MyMDILabel(const string &name) :
  pixmap("button.xpm"),
  label(name)
{
  pack_start(pixmap);
  pack_start(label);
  show_all();
}

////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  Gnome::Main gui("GnomeMDI", "0.1.0", argc, argv);
  MyMDI mdi;
  mdi_ptr = &mdi;
  mdi.add_child();
  gui.run();

  return 0;
}
