// This is -*- C++ -*-

/* 
 * cursor.h
 *
 * Copyright 2000 Karl Einar Nelson
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
#ifndef _GDKMM_CURSOR_H_
#define _GDKMM_CURSOR_H_

#include <gdk--/types.h>

//: Cursor
//- Cursor is a pointer used to represent the location of the mouse.
//- It is limited to 2 colors, a foreground and background, by X.  
//- It has a place known as a hot spot specified by coordinates.
//- Usually you just select them from a broad set of predefined
//- Cursors using GdkCursorType.
//-
//- Note that cursors are not reference counted, therefore if you
//- create one from a pixmap you are responsible for holding it
//- until you want to destroy it.  (this is a gtk+ limitation)
class Gdk_Cursor: public Gdk_Handle<GdkCursor>
  {
   public:
     typedef GdkCursorType Type;

   protected:

     // Internal for gdk reference counters
     void ref() {};

     // Internal for gdk reference counters
     void unref() {};

/* REQUIRED Handle Constructors and Destructors */
#ifdef GDKMM_HANDLES_CONNECTED_ONLY
   protected:
#else
   public:
#endif
     //: Create a unconnected cursor.
     Gdk_Cursor();

   public:
     //: Create a cursor handle from a GdkFont
     Gdk_Cursor(GdkCursor *cursor);
  
     Gdk_Cursor(const Gdk_Cursor& font);

     //: Create a cursor handle loaded with specific fontname.
     Gdk_Cursor(Type cursor_type);

     //: Create a cursor from pixmap
     Gdk_Cursor (const Gdk_Pixmap &source, 
		 const Gdk_Pixmap &mask, 
		 const Gdk_Color  &fg,
		 const Gdk_Color  &bg,
 	 	 gint             x,
		 gint             y
		);

     ~Gdk_Cursor();

     void create(Type cursor_type);
     void create(const Gdk_Pixmap &source, 
		 const Gdk_Pixmap &mask, 
		 const Gdk_Color  &fg,
		 const Gdk_Color  &bg,
 	 	 gint             x,
		 gint             y
		);

     void destroy();
     void release();

     Gdk_Cursor& operator = (const Gdk_Cursor&);
  };

#endif //_GDKMM_CURSOR_H_
