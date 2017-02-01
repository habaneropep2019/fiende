// This is -*- C++ -*-

/* 
 * gc.h
 *
 * Copyright (C) 1998 EMC Capital Management, Inc.
 * Copyright (C) 1998 Karl E. Nelson
 *
 * Developed by Jon Trowbridge <trow@emccta.com> and
 * Havoc Pennington <hp@emccta.com>.
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

#ifndef _GDKMM_INC_GC_H
#define _GDKMM_INC_GC_H

#include <gdk/gdk.h>
#include <gdk--/types.h>
#include <gdk--/font.h>

//: Gdk GC (Graphics Context) Handle 
//- The Graphics Context specifies a number of server side resources
//- that are used by several drawing commands.
//- 
//- For understanding Graphics Contexts you should read chapter 10 of
//- {\link Gtk+/Gnome Application Development 
//- http://developer.gnome.org/doc/GGAD/GGAD.tar.gz}
//- (>500K) by Havoc Pennington
//- or chapter 7 of "Xlib -- C Language X Interface" from the X Consortium.
class Gdk_GC :public Gdk_Handle<GdkGC>
  {
   protected:
 
     //: {\i (internal function)} increment gdk reference counter
     void ref();

     //: {\i (internal function)} decrement gdk reference counter 
     void unref();

/* REQUIRED Handle Constructor and Destructors */
#ifdef GDKMM_HANDLES_CONNECTED_ONLY
   protected:
#else
   public:
#endif
     //: Create an unconnect GC Handle.
     Gdk_GC();

   public:
     //: Wrap an existing GdkGC.
     Gdk_GC(GdkGC* gc);

     //: Initialize a GC reference from a existing one.
     //- The new GC refers to the same Graphics Context on the server side.
     Gdk_GC(const Gdk_GC& gc);
  
     //: Create a new GC for this drawable.
     //- Allocate a remote gc object based on this drawable.
     Gdk_GC(Gdk_Drawable &drawable);

     //: Delete this handle.
     ~Gdk_GC();

     //: Create a GC on server
     //- Allocate a remote gc object based on this drawable.
     //- 
     //- If the GC Handle is already connected to a remote Graphics Context,
     //- then this connection will be released.
     //- 
     //- If the drawable handle is not connected to a remote drawable,
     //- then the GC handle is unconnected after this call.
     void create(Gdk_Drawable const &drawable);

     void release();
     Gdk_GC& operator = (const Gdk_GC&);

   private:
   
     //: Destroy a remote gc. 
     //- This is very dangerous and should probably be removed.
     //- Nuke the GC and make this copy invalid. 
     void destroy();

   public:
   /* Provided member functions */

     //: Copy server GC.
     //- This copies all properties of one remote GC to another
     //- GC on the server.
     //- 
     //- Both, source handle and destination (this) handle,
     //- have to be connected to remote Graphics Contexts when calling
     //- this method.
     void copy(Gdk_GC &src);

     //: Get GC properites.
     //- GdkGCValues is defined as struct _GdkGCValues in gdk/gdktypes.h
     void get_values(GdkGCValues &values);

     //: Set foreground color.
     void set_foreground(const Gdk_Color& c);
 
     //: Set background color.
     void set_background(const Gdk_Color& c);

     //: Set the font associated with this GC.
     void set_font(const Gdk_Font& font);
	
     //: Return the current font. 
     //- Font will not be initialized if there is no current font.  
     Gdk_Font get_font();
 
     //: Set the drawing function.
     //- GdkFunction is an enum defined in gdk/gdktypes.h. It specifies how
     //- Pixels are set in a drawable.
     //- 
     //- Common values: {\enum GDK_COPY, GDK_INVERT, GDK_XOR. }
     void set_function(GdkFunction func);

     //: Set the fill style.
     //- GdkFill is an enum defined in gdk/gdktypes.h.
     //- 
     //- The default fill style is {\enum GDK_SOLID.}
     //- When it is set, the drawing commands do what you expect.
     //- 
     //- If you want to learn about the
     //- other fill styles ({\enum GDK_SOLID, GDK_TILED, GDK_STIPPLED,
     //- GDK_OPAQUE_STIPPLED}), you should read chapter 10 of
     //- {\link Gtk+/Gnome Application Development 
     //- http://developer.gnome.org/doc/GGAD/GGAD.tar.gz}
     //- (>500K) by Havoc Pennington
     //- or chapter 7 of "Xlib -- C Language X Interface" from the
     //- X Consortium.
     void set_fill(GdkFill fill);

     //: Set the background tile for the {\enum GDK_TILED} fill style.
     //- This has an effect only if the fill style of this GC is set to
     //- {\enum GDK_TILED} with {set_fill()}.
     void set_tile(Gdk_Pixmap &tile);

     //: Set the fill stipple.
     //- This has an effect only for the {\enum GDK_STIPPLED} and
     //- {\enum GDK_OPAQUE_STIPPLED} fill styles (set with {set_fill()}).
     //- stip must have a depth of 1, it is a bitmap.
     void set_stipple(Gdk_Pixmap &stip);

     //: Set the origin of the first tile or stipple.
     //- This has an effect for fill styles (see {set_fill()}) different
     //- from {\enum GDK_SOLID}. It sets the origin of the first tile or stiple.
     //- Tiles and stiples are copied everywhere so that they cover
     //- every Pixel.
     void set_ts_origin(gint x, gint y);

     //: Set the origin of the clipping mask.
     void set_clip_origin(gint x, gint y);

     //: Set the clipping mask.
     //- Only bits set to 1 in this mask will be drawn.
     void set_clip_mask(Gdk_Bitmap &mask);

     //: Deactive the clipping mask.
     void set_clip_mask() ;

     //: Deactive the clipping mask (non-standard) 
     void set_noclip() ;

     //: Set the clipping rectangle.
     //- This method sets the clipping mask and the clipping origin for you
     //- so that only pixels inside this rectangle are drawn.
     void   set_clip_rectangle  (const Gdk_Rectangle &rect);

     //: Set the clipping rectangle.
     //- This method calls the rectangle constructor for you before calling
     //- void {set_clip_rectangle()} (const Gdk_Rectangle &rect).
     void set_clip_rectangle(gint x, gint y, gint w, gint h);

     //: Set the clip region. (needs work)
     void set_clip_region(Gdk_Region &region);
     
     //: Set the subwindow clipping mode.
     //- GdkSubwindowMode is an enum.
     //- Subwindow modes: {\enum GDK_CLIP_BY_CHILDREN, GDK_INCLUDE_INFERIORS.}
     void set_subwindow(GdkSubwindowMode mode);

     //: Determines wether Gdk_Window::copy_area can generate expose events.
     //- exp is a boolean value.
     //- 
     //- If it is set to true, and {Gdk_Window::copy_area()} is told to copy
     //- an area from a window that is currently obscured, then the XServer
     //- sends expose events to the obscured window so that the
     //- obscured contents is redrawn by the application.
     //- This contents is then used in the copy.
     void set_exposures(bool exp);

     //: Set the line attributes
     //- See {set_line_width()}, {set_line_style()}, 
     //- {set_cap_style()}, {set_join_style()}
     //- for an explanation of  the arguments.
     void set_line_attributes(gint line_width,
                              GdkLineStyle line_style=GDK_LINE_SOLID,
                              GdkCapStyle cap_style=GDK_CAP_BUTT,
                              GdkJoinStyle join_style=GDK_JOIN_MITER);


    /* A large set of convience functions (non-standard) (need minor work)  */
    /* We should make a some code for wrapping CGchange, I don't understand */
    /* why Gdk didn't support this.  We should connect them and ask.        */

      //: Set the line width.
      //- {\var line_width} is given in Pixels.
      //- 
      //- 0 is a special value for a line width of one Pixel.
      //- Lines with line_width set to 0 are usually drawn with hardware
      //- acceleration, and the exact pixels set can be different depending
      //- on the hardware in use and the direction in that the line is drawn.
      //- 
      //- This cannot happen for lines with {\var line_width}>0.
      //- For further info, see chapter 7 of
      //- "Xlib -- C Language X Interface" from the X Consortium.
      void set_line_width(gint line_width);
 
      //: Set the line style. 
      //- Lines can be drawn in a number of diffent fashions.
      //- 
      //- GdkLineStyle is an enum.
      //- Valid values for {\var line_style} are: {\enum 
      //- GDK_LINE_SOLID, GDK_LINE_ON_OFF_DASH, GDK_LINE_DOUBLE_DASH.}
      //- 
      //- Lines with line_style {\enum GDK_LINE_SOLID} are just a plain lines.
      //- 
      //- {\enum GDK_LINE_ON_OFF_DASH} produces dashed lines.
      //- The Pixels inside the dashes are drawn in the foreground color,
      //- while the Pixels between the dashes are left untouched.
      //- The cap style applies to all ends of all dashes.
      //- 
      //- {\enum GDK_LINE_DOUBLE_DASH} produces dashed lines
      //- where the Pixels inside the dashes are drawn in the foreground color
      //- while the Pixels between the dashes are drawn in the background color.
      void set_line_style(GdkLineStyle line_style);

      //: Set the cap style.
      //- Caps are the ends of lines. GdkCapStyle is an enum.
      //- 
      //- Valid values for {\var cap_style} are: {\enum
      //- GDK_CAP_NOT_LAST, GDK_CAP_BUTT, GDK_CAP_ROUND, GDK_CAP_PROJECTING.}
      //- 
      //- {\enum GDK_CAP_NOT_LAST} is equal to {\enum GDK_CAP_BUTT},
      //- except for lines with width 0,
      //- in which case the Pixel at the end point given is not drawn.
      //- 
      //- {\enum GDK_CAP_BUTT} is for lines with suare ends.
      //- 
      //- {\enum GDK_CAP_ROUND} causes lines to have filled half circles
      //- attached to their end points.
      //- 
      //- Lines drawn with {\enum GDK_CAP_PROJECTING} continue 
      //- beyond their end points
      //- for a distance equal to their half width.
      //- Line ends are square.
      void set_cap_style(GdkCapStyle cap_style);

      //: Set the joint style. 
      //- Join styles describe how lines sharing a common end point
      //- and drawn in the same drawing request connect with each other.
      //- 
      //- GdkJoinStyle is an enum. valid values for join_style are:
      //- {\enum GDK_JOIN_MITER, GDK_JOIN_ROUND, GDK_JOIN_BEVEL.}
      //- 
      //- {\enum GDK_JOIN_MITER}: Lines which form an angle of more 
      //- than 11 degrees
      //- are continued beyond the endpoint until their outer edges meet.
      //- Only the intersection set of the two continued lines is drawn.
      //- Lines with a smaller angle are joint as with {\enum GDK_JOIN_BEVEL}.
      //- 
      //- {\enum GDK_JOIN_ROUND}: The lines join in a circle with 
      //- diameter line_width.
      //- 
      //- {\enum GDK_JOIN_BEVEL}: The lines end as with CapButt and then
      //- the triangle left on the outer edge is filled.
      void set_join_style(GdkJoinStyle join_style);


      /* Here are the two cases that you use most of the time: */
      //: Set the length of dashes.
      //- On and off will have the same size. {\var size} is given in Pixels.
      void set_dashes(gint size);
     
      //: Set the length of dashes.
      //- Use this method if you want dotted lines
      //- (choose a smaller value for an than for off).
      //- {\var on} and {\var off} are given in Pixels.
      void set_dashes(gint on, gint off);
         
 
      //: Set the length of dashes.
      //- The ons and offs of a dashed line do not have to be the same all the
      //- time. With this method you can produce dash-dot lines
      //- (or dash-dot-dot or dash-dash-dot-longdash-dash-dot or whatever you
      //- like).
      //- 
      //- The dash_list is a vector of length dash_list_length.
      //- The elements of dash_list specify the length of the on/off
      //- dashes in pixels. dash_list[0], dash_list[2] ... specify
      //- the on dashes, the other values are for the off dashes.
      //- After dash_list[dash_list_length-1],
      //- dash_list[0] is again used.
      //- 
      //- dash_offset sets the pixel number with which lines are supposed
      //- to start.
      //- Usually you will set this to 0.
      //- 
      //- Constraints: dash_list_length>0, dash_list[i]>0 for all i.
      void set_dashes(gint dash_offset,
                      gint8 dash_list[], 
                      gint dash_list_length);


  };

#endif // _GDKMM_INC_GC_H
