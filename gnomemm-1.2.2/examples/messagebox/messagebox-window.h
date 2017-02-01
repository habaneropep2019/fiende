// -*- C++ -*-

/* messagebox-window.h
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

#ifndef HEADER_WINDOW_MESSAGEBOX
#define HEADER_WINDOW_MESSAGEBOX

#include <gnome--.h>
#include <gtk--.h>
#include <gtk--/window.h>
#include <iostream>
#include <string>


using SigC::bind;
using SigC::slot;

class Window_MessageBoxDemo : public Gtk::Window
{
public:
  Window_MessageBoxDemo();
  virtual ~Window_MessageBoxDemo();
  
protected:

  //Signal handlers:
  virtual void On_Button();
  
  //'virtual function' signal overrides:
  virtual gint delete_event_impl(GdkEventAny*);
  
protected:
  Gtk::VBox m_VBox;
  Gtk::HBox m_HBox;
  Gtk::Button m_Button;

  Gnome::MessageBox m_MessageBox;
  
};

#endif //HEADER_WINDOW_MESSAGEBOX
