/* application.cc
 *
 * Copyright (C) 1999 Havoc Pennington, The Gtk-- Development Team
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <cstdio>

#include <algorithm>

#include <gnome.h>
#include <libgnome/gnome-i18n.h>
#include <gtk--/menu.h>
#include <gtk--/menubar.h>
#include <gnome--/main.h>
#include <gnome--/about.h>
#include <gnome--/dialog.h>
#include "application.h"


AppExample::AppExample()
: Gnome::App("ExampleApp", "Example App"),
  m_Status(false, true, GNOME_PREFERENCES_NEVER),
  m_Button_Insert("Insert submenu items"),
  m_Button_Disable("Disable Save menu item"),
  m_Button_Enable("Enable Save menu item")
{
  m_pMenuItem = 0;

  init();
  show_all();
}

AppExample::~AppExample()
{
  Gtk::Main::quit();      
}


void
AppExample::init()
{ 
  set_policy(false, true, false);
  set_default_size(250, 350);
  set_wmclass("exampleapp", "ExampleApp");

 
  //Connect signals:
  m_Button_Insert.clicked.connect(slot(this, &AppExample::on_button_insert));
  m_Button_Disable.clicked.connect(slot(this, &AppExample::on_button_disable));
  m_Button_Enable.clicked.connect(slot(this, &AppExample::on_button_enable));

  m_HBox.pack_start(m_Button_Insert, false); 
  m_HBox.pack_start(m_Button_Disable, false); 
  m_HBox.pack_start(m_Button_Enable, false); 
  
  m_VBox.pack_start(m_HBox, false);  
  set_contents(m_VBox);
  m_HBox.show_all();
 

  set_statusbar(m_Status);

  install_menus();
  install_toolbars();
}

void
AppExample::install_menus()
{
  type_vecGnome_UI_SubTree menu_UI_Infos; //whole menu.

  // File menu
  type_vecGnome_UI_Info menu_file; //file menu.

  //Build menu:
  menu_file.push_back(Gnome::UI::SubTree("Sub Menu", type_vecGnome_UI_Info(), "Sub menu items will be inserted here."));
  menu_file.push_back(Gnome::MenuItems::Save(slot(this, &AppExample::on_menu_generic)));
  guint posFileSave = menu_file.size() - 1; //remember for later.

  menu_file.push_back(Gnome::UI::Separator()); 
  menu_file.push_back(Gnome::MenuItems::Exit(slot(this, &AppExample::on_menu_file_exit)));

  //Add menu:
  menu_UI_Infos.push_back(Gnome::Menus::File(menu_file));

  
  //Edit menu
  type_vecGnome_UI_Info menu_edit;

  //Build menu:
  menu_edit.push_back(Gnome::MenuItems::Cut(slot(this, &AppExample::on_menu_generic)));
  menu_edit.push_back(Gnome::MenuItems::Copy(slot(this, &AppExample::on_menu_generic)));
  menu_edit.push_back(Gnome::MenuItems::Paste(slot(this, &AppExample::on_menu_generic)));
  menu_edit.push_back(Gnome::MenuItems::Clear(slot(this, &AppExample::on_menu_generic)));

  //Add menu:
  menu_UI_Infos.push_back(Gnome::Menus::Edit(menu_edit));


  //Create menus and examine the result:
  Gnome::UI::Array<Gnome::UI::SubTree>& arrayInfo = create_menus(menu_UI_Infos);

  //Get created File info:
  Gnome::UI::SubTree& subtreeFile = arrayInfo[0];
  Gnome::UI::Array<Gnome::UI::Info>& arrayInfoFile = subtreeFile.get_uitree(); //vector of menu item Gnome::UI::Info.
  
  //Get created File/Save widget from info:
  m_pMenuItem = arrayInfoFile[posFileSave].get_widget();


  install_menu_hints();
}

void
AppExample::install_toolbars()
{
  type_vecGnome_UI_Info toolbar; 

  toolbar.push_back(Gnome::UI::Item(Gnome::UI::Icon(GNOME_STOCK_PIXMAP_SAVE),
                            N_("Save "),
                            slot(this, &AppExample::on_menu_generic),
                            N_("Save this document")));
  guint posSave = toolbar.size() - 1; //Remember for later.

  toolbar.push_back(Gnome::UI::Item(Gnome::UI::Icon(GNOME_STOCK_PIXMAP_BACK),
                           N_("Prev"),
                           slot(this, &AppExample::on_menu_generic),
                           N_("Previous hello")));
  toolbar.push_back(Gnome::UI::Item(Gnome::UI::Icon(GNOME_STOCK_PIXMAP_FORWARD),
                           N_("Next"),
                           slot(this, &AppExample::on_menu_generic),
                           N_("Next hello")));

  Gnome::UI::Array<Gnome::UI::Info>& arrayInfo = create_toolbar(toolbar);
  m_pToolbarItem = arrayInfo[posSave].get_widget();
}


void       
AppExample::close()
{
  destroy();
}

gint 
AppExample::delete_event_impl(GdkEventAny* e)
{
  return false; 
}


void
AppExample::on_menu_file_exit()
{
  close();
}

void
AppExample::on_menu_generic()
{
  std::cout << "Signal handler called." << std::endl;
}


void
AppExample::on_button_insert()
{
  type_vecGnome_UI_Info vecUI_Info;
  vecUI_Info.push_back(Gnome::UI::Item("Inserted Item", slot(this, &AppExample::on_menu_generic), "Dynamically inserted item."));
  insert_menus("File/Sub Menu/", vecUI_Info);
  
}

void
AppExample::on_button_disable()
{
  if(m_pMenuItem)
    m_pMenuItem->set_sensitive(false);

  if(m_pToolbarItem)
    m_pToolbarItem->set_sensitive(false);
  else
    std::cerr << "AppExample::on_button_disable(): m_pToolbarItem == 0" << std::endl;

}

void
AppExample::on_button_enable()
{
  if(m_pMenuItem)
    m_pMenuItem->set_sensitive(true);

  if(m_pToolbarItem)
    m_pToolbarItem->set_sensitive(true);
}
