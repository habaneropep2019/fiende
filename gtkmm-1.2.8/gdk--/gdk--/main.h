// This is -*- C++ -*-

/* 
 * main.h
 *
 * Copyright 1998 Karl E. Nelson
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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _GDKMM_MAIN_H_
#define _GDKMM_MAIN_H_

#include <gdk--/types.h>
#include <gdk--/colormap.h>

namespace Gtk
{
  GTKMM_USING_STD_STRING;  
}

// Basically a dead wrapper for all the functions that don't fit
class Gdk 
  {
   public:
     static void init(int &argc,char** &argv);
     static void exit(int error_code);
     static Gtk::string set_locale();
     static gint events_pending();
     static GdkEvent* event_get(void);
     static void event_put(GdkEvent &event);
     static GdkEvent* event_copy(GdkEvent* event);
     static void event_free(GdkEvent* event);

#if GDK_VERSION_GT(1,0)
     // (Gdk 1.1)
     static void event_get_time(GdkEvent* event);

     // (Gdk 1.1)
     static void set_sm_client_id(const Gtk::string &sm_client_id);
#endif

  };

#endif // _MAIN_GDK_H_
