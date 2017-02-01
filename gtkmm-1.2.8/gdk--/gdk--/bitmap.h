// This is -*- C++ -*-

/* 
 * bitmap.h
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
#ifndef _GDKMM_BITMAP_H_
#define _GDKMM_BITMAP_H_

#include <gdk--/drawable.h>

//: Gdk Bitmap Handle
//- A bitmap is a pixmap with a depth of 2.  
class Gdk_Bitmap:public Gdk_Drawable
  {
   protected:
     // internal fuctions for updating gdk counters
     virtual void ref();
     virtual void unref();

/* REQUIRED Handle Constructor and Destructors */
#ifdef GDKMM_HANDLES_CONNECTED_ONLY
   protected:
#else
   public:
#endif
     //: Create an unconnected Bitmap handle
     Gdk_Bitmap();

   public:
     //: Wrap an existing GdkBitmap.
     Gdk_Bitmap(GdkBitmap *bitmap);

     //: Initialize a Bitmap handle from a previously created one.
     Gdk_Bitmap(const Gdk_Bitmap& bitmap);
    
     // backward compate
     Gdk_Bitmap(Gdk_Window  &window,
                gchar      *data,
                gint        width,
                gint        height);
     //: Initialize a Bitmap from data.
     Gdk_Bitmap(const Gdk_Drawable  &window,
                gchar      *data,
                gint        width,
                gint        height);

     //: Delete this handle.
     ~Gdk_Bitmap();

     // backward compate
     void create(Gdk_Window  &window,
                const gchar *data,
                gint        width,
                gint        height);

     //: Allocate a remote bitmap object from data.
     void create(const Gdk_Drawable  &window,
                const gchar *data,
                gint        width,
                gint        height);

     Gdk_Bitmap& operator = (const Gdk_Bitmap&);

  };

#endif

