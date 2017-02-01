// -*- C++ -*-

/* druiddemo.cc
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "druid-window.h"

int main (int argc, char *argv[])
{
    // all GTK applications must have a gtk_main(). Control ends here
   // and waits for an event to occur (like a key press or mouse event).

   Gnome::Main kit("GnomeDruid", "0.1.0", argc, argv);

   Window_DruidDemo windowDemo;
   
   kit.run();
   return 0;
}
