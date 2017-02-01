// -*- C++ -*-

/* dialog_druid.h
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

#ifndef HEADER_DIALOG_DRUID
#define HEADER_DIALOG_DRUID

#include <gnome--.h>
#include <gtk--.h>
#include <gtk--/window.h>
#include <iostream>
#include <string>


using SigC::bind;
using SigC::slot;

//A Dialog to contain the Gnome::Druid:
class Dialog_Druid : public Gnome::Dialog
{
public:
  Dialog_Druid();
  virtual ~Dialog_Druid();

  //Signal handlers:
  virtual void on_Druid_cancel();
  virtual void on_Druid_finish();

  //methods to get user's input:
  virtual std::string get_foo_value();
  virtual bool get_finished(); //Whether finish was clicked, or cancel.
  
protected:

  //Member widgets:
  Gnome::Druid m_Druid;
  
  Gnome::DruidPageStart m_DruidPage1;
  Gnome::DruidPageStandard m_DruidPage2;
  Gnome::DruidPageFinish m_DruidPage3;

  //Widgets in pages:
  //We should really derive new page classes to contain these:
  
  //Page 1:
  Gtk::HBox m_Page2_HBox;
  Gtk::Label m_Page2_Label;
  Gtk::Entry m_Page2_Entry;

  //Member data:
  bool m_bFinished;


};

#endif //HEADER_DIALOG_DRUID
