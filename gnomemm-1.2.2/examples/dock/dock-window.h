// -*- C++ -*-

/* window-dockdemo.h
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

#ifndef HEADER_WINDOW_DOCKDEMO
#define HEADER_WINDOW_DOCKDEMO

#include <gnome--.h>
#include <gtk--.h>
#include <gtk--/window.h>
#include <iostream>
#include <string>


using SigC::bind;
using SigC::slot;

class Window_DockDemo : public Gtk::Window
{
public:
  Window_DockDemo();
  virtual ~Window_DockDemo();
  
protected:

  //Signal handlers:
  void On_Button_Quit_Clicked();
  
  //'virtual function' signal overrides:
  gint delete_event_impl(GdkEventAny*);
  
protected:

  virtual void init_toolbars();
  
  //Signal handlers:
  virtual void on_Dock_layout_changed();
  
  //Member widgets:
  Gnome::Dock m_Dock;
  Gnome::DockLayout m_DockLayout;

  typedef std::vector<Gnome::DockItem*> type_vecDockItems;
  type_vecDockItems m_vecDockItems; //6, managed.

  typedef std::vector<Gtk::Toolbar*> type_vecToolbars;
  type_vecToolbars m_vecToolbars; //6, managed.

  Gtk::Frame m_Frame;
};

#endif //HEADER_WINDOW_DOCKDEMO
