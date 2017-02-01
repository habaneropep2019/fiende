// -*- C++ -*-

/* messagebox-window.cc
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

#include "messagebox-window.h"

#include <iostream>


Window_MessageBoxDemo::Window_MessageBoxDemo()
: m_VBox(false, 4),
  m_HBox(false, 4),
  m_Button("Show MessageBox"),
  m_MessageBox("This is the text in a MessageBox", GNOME_MESSAGE_BOX_INFO)
{ 
  set_title("Gnome-- MessageBoxDemo");
  set_default_size(200, 50);
  
  //Add Button to Box:
  m_HBox.pack_start(m_Button, false, 4);
  m_VBox.pack_start(m_HBox, false, 4);
  add(m_VBox);

  //Finish building MessageBox:

  //some stock buttons:
  m_MessageBox.append_button(GNOME_STOCK_BUTTON_OK);
  m_MessageBox.append_button(GNOME_STOCK_BUTTON_CANCEL);
  m_MessageBox.append_button(GNOME_STOCK_BUTTON_HELP);

  //And a custom button:
  m_MessageBox.append_button("Custom button");
  
  //Connect signals:
  m_Button.clicked.connect(slot(this, &Window_MessageBoxDemo::On_Button));

  
  show_all();
}



Window_MessageBoxDemo::~Window_MessageBoxDemo()
{
}

gint Window_MessageBoxDemo::delete_event_impl(GdkEventAny*)
{ 
  Gtk::Main::quit();
  return 0; 
}

void Window_MessageBoxDemo::On_Button()
{
  gint iButton = m_MessageBox.run_and_close();

  std::cout << "Button clicked: " << iButton << std::endl;

}


