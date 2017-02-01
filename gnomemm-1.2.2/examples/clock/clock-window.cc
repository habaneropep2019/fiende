// -*- C++ -*-

/* clock-window.cc
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

#include "clock-window.h"

#include <iostream>


Window_Clock::Window_Clock()
: m_Clock(Gtk::Clock::REALTIME)
{ 
  set_title("Gnome-- Clock Demo");
  set_default_size(200, 50);
  
  add(m_Clock);

  show_all();
}

Window_Clock::~Window_Clock()
{
}

gint Window_Clock::delete_event_impl(GdkEventAny*)
{ 
  Gtk::Main::quit();
  return 0; 
}



