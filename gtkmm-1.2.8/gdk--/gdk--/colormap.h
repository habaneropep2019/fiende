// This is -*- C++ -*-

/* 
 * colormap.h
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
#ifndef _GDKMM_COLORMAP_H_
#define _GDKMM_COLORMAP_H_

#include <gdk--/color.h>

// Gdk Colormap Handle
class Gdk_Colormap : public Gdk_Handle<GdkColormap>
  {
   protected:
     
     // Internal function for updating gdk counters
     void ref();
     void unref();

/* REQUIRED Handle Constructor and Destructors */
#ifdef GDKMM_HANDLES_CONNECTED_ONLY
   protected:
#else
   public:
#endif
     // Create an unconnected handle
     Gdk_Colormap();

   public:
     // Wrap an existing Gdk Colormap
     Gdk_Colormap(GdkColormap *colormap);

     // Initialize handle from existing Colormap
     Gdk_Colormap(const Gdk_Colormap& colormap);

     // Delete this handle
     ~Gdk_Colormap();

     // Allocate a remote colormap 
     void create() ;

     void release();
     Gdk_Colormap& operator = (const Gdk_Colormap&);

     /* Provided method functions */

     // allocates and returns the color white
     Gdk_Color white();

     // allocates and returns the color black 
     Gdk_Color black();

     gint alloc(Gdk_Color &color);

     gint change(Gdk_Color &color);

     // name conflict
     void change(gint ncolors);

/* //////////////////////////////////////////////////////////////////// */

     // SYSTEM  ACCESS
     // colormap=colormap.get_system();
     static Gdk_Colormap get_system();

     static gint get_system_size();
  };

#endif // _GDKMM_COLORMAP_H_
