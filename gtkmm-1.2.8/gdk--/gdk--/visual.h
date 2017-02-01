// This is -*- C++ -*-

/* 
 * visual.h
 *
 * Copyright (C) 1998 Karl E. Nelson
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

#ifndef _GDKMM_VISUAL_H
#define _GDKMM_VISUAL_H

#include <gdk/gdk.h>
#include <gdk--/types.h>


//: Gdk Visual Handle 
//- Visual refers to the properties of a display that are
//- assocuated with a window.  Visuals can not be changed.
//- They are given by the system to be used.  
// Please read {\link Handles handles.htm}
class Gdk_Visual : public Gdk_Handle<GdkVisual>
  {
   protected:
     //: (internal function) increment gdk reference counter
     void ref();

     //: (internal function) decrement gdk reference counter 
     void unref();

/* REQUIRED Handle Constructor and Destructors */
#ifdef GDKMM_HANDLES_CONNECTED_ONLY
   protected:
#else
   public:
#endif
     //: Create an unconnect Visual Handle.
     Gdk_Visual();

   public:
     //: Wrap an existing GdkVisual.
     Gdk_Visual(GdkVisual* visual);

     //: Initialize a Visual reference from a existing one.
     Gdk_Visual(const Gdk_Visual& visual);
  
     //: Delete this handle.
     ~Gdk_Visual();

     void release();
     Gdk_Visual& operator = (const Gdk_Visual&);

   private:
       //: Create a visual. 
       //- You can't create a visual!  You must ask the system
       //- for one!  Use the get functions.
       void create();

   public:
     //: Returns the best available depth for this display
     static gint get_best_depth (void);
 
     //: Returns the best available type for this display
     static GdkVisualType get_best_type (void);
 
     Gdk_Visual&  get_system (void);
 
     Gdk_Visual&  get_best (void);
 
     Gdk_Visual&  get_best (gint depth);
 
     Gdk_Visual&  get_best (GdkVisualType  visual_type);

     Gdk_Visual&  get_best (gint depth, GdkVisualType  visual_type);
 
  };

#endif // _GDKMM_VISUAL_H
