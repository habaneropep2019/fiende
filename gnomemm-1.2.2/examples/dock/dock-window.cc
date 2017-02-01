// -*- C++ -*-

/* window-dockdemo.cc
 * 
 * Copyright (C) 2000 Gtk-- Development Team  
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "dock-window.h"

#include <iostream>


Window_DockDemo::Window_DockDemo()
{ 
  set_title("Gnome-- DockDemo");
  set_default_size(400, 400);

  add(m_Dock);

  init_toolbars();


  //Create Dock Items:
  {
    //0:
	  m_vecDockItems.push_back(
             Gtk::manage( new Gnome::DockItem("SomeBar",
	        (GnomeDockItemBehavior)(GNOME_DOCK_ITEM_BEH_EXCLUSIVE
	        | GNOME_DOCK_ITEM_BEH_NEVER_VERTICAL)) ));

    //1:
	  m_vecDockItems.push_back(
             Gtk::manage( new Gnome::DockItem("LockedBar",
	        GNOME_DOCK_ITEM_BEH_LOCKED) ));

	  //2 to 5:
	  for(guint i = 2; i <= 5; i++)
	  {
	    gchar* pchName = g_strdup_printf ("AnotherBar%d", i);
	    string strName(pchName);
	    g_free(pchName);
	          
	    m_vecDockItems.push_back(
              Gtk::manage( new Gnome::DockItem(strName,
	         GNOME_DOCK_ITEM_BEH_NORMAL) ));
	  }
  }

  for(guint i=0; i < m_vecDockItems.size(); i++)
  {
    Gnome::DockItem* pDockItem = m_vecDockItems[i];
    Gtk::Toolbar* pToolbar = m_vecToolbars[i]; 
    pDockItem->set_border_width(1);

    //Add Toolbar i to DockItem i:
    pDockItem->add( *pToolbar );

    //Add DockItem to DockLayout:
    if(i < 3)
      m_DockLayout.add_item(*pDockItem, GNOME_DOCK_TOP, i, 0, 0);
    else
      m_DockLayout.add_item(*pDockItem, GNOME_DOCK_BOTTOM, i - 4, 0, 0);

    pToolbar->show();
    pDockItem->show();
  }

  //TODO: I have ignore the gnome_config_* stuff.
  //The original dock_demo.cc use gnome_config-* functions to save the layout.
  //But gnome-config is part of gnome-libs/libgnome, not gnome-libs/libgnomeui, so it's not wrapped yet.
  //And I believe that gnome-config will be replaced with GConf anyway. murrayc
      
  m_DockLayout.add_to_dock(m_Dock);
  //m_DockLayout.unreference(); //dock_demo.c does this.
     
  m_Frame.set_shadow_type(GTK_SHADOW_IN);
  
  m_Dock.set_client_area(m_Frame);
  m_Frame.show();
  m_Dock.show();

  m_Dock.layout_changed.connect(slot(this, &Window_DockDemo::on_Dock_layout_changed)); 
  
  show_all();
}


void print_cb(const char* s)
  { cout << s << endl; }

SigC::Slot0<void> print(const char* s)
  {
    return SigC::bind(slot(print_cb),s);
  }
    
void Window_DockDemo::init_toolbars()
{ 

  typedef vector<Gnome::UI::Info> info_vector;
  vector<info_vector> iv;
  iv.resize(6);

  //Toolbar 0:
  using namespace Gnome::UI;
  
  iv[0].push_back( Item( Icon(GNOME_STOCK_PIXMAP_NEW),
    "New", print("New") , "Create a new file" ) );
  iv[0].push_back( Item( Icon(GNOME_STOCK_PIXMAP_OPEN),
    "Open", print("Open"), "Open an existing file" ) );
  iv[0].push_back( Item( Icon(GNOME_STOCK_PIXMAP_SAVE),
    "Save", print("Save"), "Save the current file" ) );
  iv[0].push_back( Item( Icon(GNOME_STOCK_PIXMAP_SAVE_AS),
    "Save as", print("Save As"), "Save the current file with a new name" ) );

  //Toolbar 1:
  iv[1].push_back( Item( Icon(GNOME_STOCK_PIXMAP_CLOSE),
    "Close", print("Close"), "Close the current file" ) );
  iv[1].push_back( Item( Icon(GNOME_STOCK_PIXMAP_EXIT),
    "Exit", Gtk::Kit::quit.slot(), "Exit the program" ) );

  //Toolbar 2:
  iv[2].push_back( Item( Icon(GNOME_STOCK_PIXMAP_UNDO),
    "Undo", print("Undo"), "Undo the last operation" ) );
  iv[2].push_back( Item( Icon(GNOME_STOCK_PIXMAP_REDO),
    "_Redo", print("Redo"), "Redo the last undo-ed operation" ) );

  //Toolbar 3:
  iv[3].push_back( Item( Icon(GNOME_STOCK_PIXMAP_CUT),
    "Cut",  print("Cut"), "Cut the selection to the clipboard" ) );
  iv[3].push_back( Item( Icon(GNOME_STOCK_PIXMAP_COPY),
    "Copy", print("Copy"), "Copy the selection to the clipboard" ) );                                                
  iv[3].push_back( Item( Icon(GNOME_STOCK_PIXMAP_PASTE),
    "Paste", print("Paste"), "Paste the contents of the clipboard" ) );
  
  //Toolbar 4:
  iv[4].push_back( Item( Icon(GNOME_STOCK_PIXMAP_FIRST),
    "First", print("First"), "Go to the first item" ) );
  iv[4].push_back( Item( Icon(GNOME_STOCK_PIXMAP_LAST),
    "Last", print("Last"), "Go to the last item" ) );

   //Toolbar 5:
  iv[5].push_back( Item( Icon(GNOME_STOCK_PIXMAP_FIRST),
    "First", print("First"), "Go to the first item" ) );
  iv[5].push_back( Item( Icon(GNOME_STOCK_PIXMAP_LAST),
    "Last", print("Last"), "Go to the last item" ) );


  //Create Toolbars:
  for (gint i = 0; i < 6; i++)
  {
    Gtk::Toolbar* pToolbar = Gtk::manage( new Gtk::Toolbar(GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_ICONS) );
    pToolbar->set_border_width(1);
   
    fill(*pToolbar, iv[i], *get_accel_group());

    m_vecToolbars.push_back(pToolbar);
  }
                                                  
}

Window_DockDemo::~Window_DockDemo()
{
}

gint Window_DockDemo::delete_event_impl(GdkEventAny*)
{ 
  Gtk::Main::quit();
  return 0; 
}

void Window_DockDemo::on_Dock_layout_changed()
{
   std::cout << "Layout changed" << std::endl;
}

