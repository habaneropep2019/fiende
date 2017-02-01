// This is -*- C++ -*-

/* 
 * drawable.h
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
#ifndef _GDKMM_DRAWABLE_H_
#define _GDKMM_DRAWABLE_H_

#include <gdk/gdk.h>
#include <gdk--/types.h>

namespace Gtk
{
  GTKMM_USING_STD_STRING;  
}

//: Gdk Drawable handle.
//- A drawable is a base class for all of the objects that accept
//- drawing commands.  The available drawables include pixmaps, 
//- windows, and bitmaps.  Drawable is an abstract as
//- there is no such type on the server side.  
//- 
//- To use a drawable, create a concrete drawable of the type you
//- wish to use and a GC (graphics context) for that drawable.
//- With the GC you can draw lines, strings, arcs and such. 
//-
class Gdk_Drawable: public Gdk_Handle<GdkDrawable>
  {protected:

     virtual void ref()=0;
     virtual void unref()=0;

     Gdk_Drawable ();

   public:
     Gdk_Drawable (GdkDrawable *drawable);
     Gdk_Drawable (const Gdk_Drawable &drawable);

     // Destroy a drawable handle
     virtual ~Gdk_Drawable ();

     void release();
     Gdk_Drawable& operator = (const Gdk_Drawable&);

     /* Provided methods */
     
     //: Draw a point.
     void draw_point     (const Gdk_GC &gc, 
                          gint          x1,
                          gint          y1);

     //: Draw a line.
     void draw_line      (const Gdk_GC &gc, 
                          gint          x1,
                          gint          y1,
                          gint          x2, 
                          gint          y2);

     //: Draw a rectangle.
     //- Depending on the GC it may be filled or unfilled.
     void draw_rectangle (const Gdk_GC &gc,
                          gint          filled,
                          gint          x,
                          gint          y,
                          gint          width,
                          gint          height);
  
     //: Draw an arc.
     void draw_arc       (const Gdk_GC &gc,
                          gint          filled,
                          gint          x,
                          gint          y,
                          gint          width,
                          gint          height,
                          gint          angle1,
                          gint          angle2);

     //: Draw a polygon.
     void draw_polygon   (const Gdk_GC &gc,
                          gint          filled,
                          const Gdk_Points   &points);
    

     //: Draw a polygon.
     void draw_polygon    (const Gdk_GC &gc,
			   gint        filled,
			   GdkPoint    *points,
			   gint          npoints);
    

     //: Draw a string.
     //- Requires a valid font in addition to a GC.
     void draw_string    (const Gdk_Font &font,
                          const Gdk_GC   &gc,
                          gint          x,
                          gint          y,
                          const Gtk::string &str);
 
     //: Draw text.
     //- requires a string, font, and length in addition to a GC.
     void draw_text      (const Gdk_Font &font,
                          const Gdk_GC   &gc,
                          gint          x,
                          gint          y,
                          const char*   text,
                          gint          text_length);

     //: Draw a pixmap.
     void draw_pixmap     (const Gdk_GC       &gc,
                           const Gdk_Drawable &src,
                           gint          xsrc,
                           gint          ysrc,
                           gint          xdest,
                           gint          ydest,
                           gint          width = -1,
                           gint          height = -1);

     //: Draw a bitmap. 
     void draw_bitmap    (const Gdk_GC     &gc,
                          const Gdk_Bitmap &src,
                          gint          xsrc,
                          gint          ysrc,
                          gint          xdest,
                          gint          ydest,
                          gint          width = -1,
                          gint          height = -1);

     //: Draw an image.
     void draw_image     (const Gdk_GC    &gc,
                          const Gdk_Image &image,
                          gint          xsrc,
                          gint          ysrc,
                          gint          xdest,
                          gint          ydest,
                          gint          width = -1,
                          gint          height = -1);


     //: Draw a set of points.
     //-  Draws a set of unconnected points.
     void draw_points    (const Gdk_GC        &gc,
                          const Gdk_Points   &points);

     //: Draw a set of points. 
     //-  Draws a set of unconnected points.
     void draw_points    (const Gdk_GC        &gc,
                          GdkPoint    *points,
                          gint          npoints);

     void draw_segments  (const Gdk_GC       &gc,
                          const Gdk_Segments &segs,
                          gint          nsegs);

     void draw_segments  (const Gdk_GC       &gc,
                          GdkSegment  *segs,
                          gint               nsegs);

     //: Draw a set of lines.
     //-  Draws a set of connected lines.
     void draw_lines     (const Gdk_GC        &gc,
                          const Gdk_Points   &points);

     //: Draw a set of lines.
     //-  Draws a set of connected lines.
     void draw_lines     (const Gdk_GC        &gc,
                          GdkPoint     *points,
                          gint          npoints);

#if GDK_VERSION_GT(1,0)
     void draw_text_wc   (const Gdk_Font      &font,
                          const Gdk_GC        &gc,
                          gint          x,
                          gint          y,
                          const GdkWChar *text,
                          gint          text_length);
#endif
//***** Window methods that should be drawable
     void copy_area(const Gdk_GC &gc,
                    gint   x,
                    gint   y,
                    const Gdk_Drawable& src,
                    gint   src_x,
                    gint   src_y,
                    gint   width,
                    gint   height);

     // These functions seem to be competing for the same pointer
     void set_user_data(gpointer data);
     gpointer get_user_data();
#if GDK_VERSION_GT(1,2)
     void set_data(const Gtk::string& key,
                   gpointer data,
                   GDestroyNotify destroy_func);
#endif

     void get_size(gint &width,gint &height);
     void get_position(gint &x,gint &y);
     Gdk_Visual get_visual();
     GdkWindowType get_type();

// data member access
     gint width();
     gint height();
     gint x();
     gint y();

  };

#endif // _GDKMM_DRAWABLE_H_
