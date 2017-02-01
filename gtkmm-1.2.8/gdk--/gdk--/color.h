// This is -*- C++ -*-
// $Id: color.h,v 1.23 2000/05/14 18:48:18 kenelson Exp $

/* 
 * color.h
 *
 * Copyright (C) 1998 EMC Capital Management, Inc.
 * Copyright 1998 Karl E. Nelson <kenelson@ece.ucdavis.edu>
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

#ifndef _GDKMM_COLOR_H
#define _GDKMM_COLOR_H

#include <gdk/gdk.h>
#include <gdk--/types.h>
#include <string>

namespace Gtk
{
  GTKMM_USING_STD_STRING;  
}

//: Gdk Color
//- This is a container for a GdkColor.  
//- It stores 3 color values and the color index.
//- (It is not a handle.)
class Gdk_Color: public GdkColor
  {
   public:
     typedef GdkColor BaseObjectType;

   private:
     GdkColor* operator -> ();
     const GdkColor* operator -> () const;  

   public:
     Gdk_Color();
     Gdk_Color(const Gtk::string &str);
     Gdk_Color(const GdkColor& c);
     explicit Gdk_Color(const GdkColor* c);
     Gdk_Color(const Gdk_Color& c);
     ~Gdk_Color();

     GdkColor* gdkobj() {return this;}
     const GdkColor* gdkobj() const {return this;}

     bool operator == (const Gdk_Color &color)
       {
        return equal(color)?true:false;
       }

     gint equal(const Gdk_Color& color) const;

     void set_grey(gushort g) 
       {
        red = green = blue = g;
       }

     void set_grey_p(gdouble g) 
       {
        red = green = blue = (gushort)(g*65535);
       }

     void set_rgb(gushort red_, gushort green_, gushort blue_) 
       {
        red = red_;
        green = green_;
        blue = blue_;
       }

     void set_rgb_p(gdouble red_, gdouble green_, gdouble blue_) 
       {
        red = (gushort)(red_*65535);
        green = (gushort)(green_*65535);
        blue = (gushort)(blue_*65535);
       }

     void set_hsv(gdouble h, gdouble s, gdouble v);
     void set_hsl(gdouble h, gdouble s, gdouble l);

     gint set(const Gtk::string &str) ;
     gint parse(const Gtk::string& spec)
       {return set(spec);}

     gushort get_red() const     { return red;   }
     gushort get_green() const   { return green; }
     gushort get_blue() const    { return blue;  }

     void set_red(gushort r)     {  red = r;   }
     void set_green(gushort g)   {  green = g; }
     void set_blue(gushort b)    {  blue = b;  }

     guint get_pixel() const     { return pixel; }

     gdouble red_p() const   { return red / 65535.;   }
     gdouble green_p() const { return green / 65535.; }
     gdouble blue_p() const  { return blue / 65535.;  }

     /* Gratuitous code-bloat begins here */
     void set_random(); /* pointless, but fun */
     gdouble distance_rgb_euclidean(const Gdk_Color& obj) const;
     gdouble distance_rgb_taxicab(const Gdk_Color& obj) const;

  };

#endif // _GDKMM_COLOR_H

// $Id: color.h,v 1.23 2000/05/14 18:48:18 kenelson Exp $
