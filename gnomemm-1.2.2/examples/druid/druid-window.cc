// -*- C++ -*-

/* druid-window.cc
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

#include "druid-window.h"

#include <iostream>


Window_DruidDemo::Window_DruidDemo()
: m_VBox(false, 4),
  m_HBox(false, 4),
  m_Button_RunDruid("Run Druid")
{ 
  set_title("Gnome-- DruidDemo");
  set_default_size(200, 50);


  //This should be a default:
  m_Dialog.close_hides(true); //Don't delete the underlying gnome object.
  
  //Add Button to Box:
  m_HBox.pack_start(m_Button_RunDruid, false, 4);
  m_VBox.pack_start(m_HBox, false, 4);
  add(m_VBox);

  //Connect signals:
  m_Button_RunDruid.clicked.connect(slot(this, &Window_DruidDemo::On_Button_RunDruid));

    
  show_all();
}



Window_DruidDemo::~Window_DruidDemo()
{
}

gint Window_DruidDemo::delete_event_impl(GdkEventAny*)
{ 
  Gtk::Main::quit();
  return 0; 
}

void Window_DruidDemo::On_Button_RunDruid()
{
  m_Dialog.run_and_close();

  if(m_Dialog.get_finished())
  {
    std::cout << "Value from Druid: " << m_Dialog.get_foo_value().c_str() << std::endl;
  }
  else
  {
    std::cout << "User cancelled Druid" << std::endl;
  }

}


