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

#include "dialog_druid.h"

#include <iostream>


Dialog_Druid::Dialog_Druid()
: m_Page2_Label("some info")
{ 
  m_bFinished = false;
  
  set_title("Gnome-- DruidDemo");
  set_default_size(400, 400);

  //Build Pages:

  //Page 1:
  m_DruidPage1.set_title("Example Druid - Page 1");
  m_DruidPage1.set_text("This is a Gnome::DruidPageStart. \n\
    It seems that it should just contain text via the set_text() method, \n\
    not extra widgets. You can also change the title, colors, and logo.");

  //Page 2:
  m_DruidPage2.set_title("Example Druid - Page 2");
  m_Page2_HBox.pack_start(m_Page2_Label);
  m_Page2_HBox.pack_start(m_Page2_Entry);
  
  //Gtk::VBox* pBox = Gtk::wrap(GTK_VBOX(m_DruidPage2.gtkobj()->vbox));
  Gtk::VBox* pBox = m_DruidPage2.get_vbox();
  if(pBox)
    pBox->pack_start(m_Page2_HBox);

  //Page 3:
  m_DruidPage3.set_title("Example Druid - Page 3");
  m_DruidPage3.set_text("This is a Gnome::DruidPageFinish. \n\
    This class is very similar to Gnome::DruidPageStart.");
  
  //Add Pages to Druid:
  m_Druid.pages().push_back(m_DruidPage1);
  m_Druid.pages().push_back(m_DruidPage2);
  m_Druid.pages().push_back(m_DruidPage3);
  
  m_Druid.show_all();
  get_vbox()->pack_start(m_Druid);


  //Connect signals:
  m_Druid.cancel.connect(slot(this, &Dialog_Druid::on_Druid_cancel));
  m_DruidPage3.finish.connect(slot(this, &Dialog_Druid::on_Druid_finish));
    
}



Dialog_Druid::~Dialog_Druid()
{
}

void Dialog_Druid::on_Druid_cancel()
{
  //When the user clicks [Cancel], just close the dialog.
  //You could do some validation here.
  m_bFinished = false;
  cause_close(); //stops run_and_close().
}

void Dialog_Druid::on_Druid_finish()
{
  //When the user clicks [Finish], just close the dialog.
  //You could do some validation here.
  m_bFinished = true;
  cause_close(); //stops run_and_close().
}

std::string Dialog_Druid::get_foo_value()
{
  return m_Page2_Entry.get_text();
}

bool Dialog_Druid::get_finished()
{
  return m_bFinished;
}


