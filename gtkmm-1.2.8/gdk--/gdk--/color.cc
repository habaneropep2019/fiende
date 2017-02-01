// This is -*- C++ -*-
// $Id: color.cc,v 1.9 2000/08/27 20:15:42 kenelson Exp $

/* 
 * color.cc
 *
 * Copyright (C) 1998 EMC Capital Management, Inc.
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

/* always include color.h first, this way we can be sure it works 
   without depedencies to other headers.
 */
#include <gdk--/color.h>
#include <math.h>
#include <stdlib.h>


GTKMM_USING_STD_STRING;

Gdk_Color::Gdk_Color()
  { 
    GdkColor::pixel=0;
    GdkColor::red=0;
    GdkColor::green=0;
    GdkColor::blue=0;
  }

Gdk_Color::Gdk_Color(const Gtk::string &str) 
  { 
   set(str); 
  }

Gdk_Color::Gdk_Color(const GdkColor& c) 
  {
    GdkColor::pixel=c.pixel;
    GdkColor::red=c.red;
    GdkColor::green=c.green;
    GdkColor::blue=c.blue;
  }
    
Gdk_Color::Gdk_Color(const GdkColor* c)
  {
    if (c) 
      {
        GdkColor::pixel=c->pixel;
        GdkColor::red=c->red;
        GdkColor::green=c->green;
        GdkColor::blue=c->blue;
      }
    else 
      {
        GdkColor::pixel=0;
        GdkColor::red=0;
        GdkColor::green=0;
        GdkColor::blue=0;
      }
  }

Gdk_Color::Gdk_Color(const Gdk_Color& c) 
  {
    GdkColor::pixel=c.pixel;
    GdkColor::red=c.red;
    GdkColor::green=c.green;
    GdkColor::blue=c.blue;
  }

Gdk_Color::~Gdk_Color()
  {}

gint Gdk_Color::equal(const Gdk_Color& color) const
  {
   return gdk_color_equal(const_cast<Gdk_Color*>(this),
     const_cast<Gdk_Color*>(&color));
  }

gint Gdk_Color::set(const Gtk::string &str) 
  {
   return gdk_color_parse(str.c_str(), this);
  }


/*********************************************************************/
/*
  This HSL -> RGB algorithm is adapted from a qbasic program I found at
  http://www.access.digex.net/~erniew/hsv2rgb.html.
  
  Thanks to Ernie Wright <erniew@access.digex.net> for making it
  available.
*/
  
void
Gdk_Color::set_hsv(gdouble h, gdouble s, gdouble v) 
{
  h /= 60;
  gint i = (gint)h;
  gdouble p = v*(1-s);
  gdouble q = v*(1-s*(h-i));
  gdouble t = v*(1-s*(1-h+i));
  switch (i) {
  case 0:
    set_rgb_p(v,t,p);
    break;
  case 1:
    set_rgb_p(q,v,p);
    break;
  case 2:
    set_rgb_p(p,v,t);
    break;
  case 3:
    set_rgb_p(p,q,v);
    break;
  case 4:
    set_rgb_p(t,p,v);
    break;
  default:
    set_rgb_p(v,p,q);
  }
}

/*
  This HSL -> RGB algorithm is said to be from Foley and van Dam's
  "Fundamentals of Interactive Computer Graphics".

  I found it at http://blas.cis.mcmaster.ca/~monger/hsl-rgb.html.
  Thanks to Patricia Monger <monger@mcmaster.ca> for putting it up on
  the net.
*/

void
Gdk_Color::set_hsl(gdouble h, gdouble s, gdouble l)
{
  if (s == 0.0)
    set_grey_p(l);
  else {
    gdouble t2 = l < 0.5 ? l*(1+s) : l+s-l*s;
    gdouble t1 = 2*l-t2;
    h /= 360;

    gdouble tr = h + 1.0/3.0;
    gdouble tg = h;
    gdouble tb = h - 1.0/3.0;
    if (tb < 0) tb += 1.0;

    gdouble r=0, g=0, b=0;

    if (tr < 1.0/6.0)
      r = t1 +(t2-t1)*6*tr;
    else if (tr < 1.0/2.0)
      r = t2;
    else if (tr < 2.0/3.0)
      r = t1+(t2-t1)*(2.0/3.0 - tr)*6.0;

    if (tg < 1.0/6.0)
      g = t1 +(t2-t1)*6*tg;
    else if (tg < 1.0/2.0)
      g = t2;
    else if (tg < 2.0/3.0)
      g = t1+(t2-t1)*(2.0/3.0 - tg)*6.0;

    if (tb < 1.0/6.0)
      b = t1 +(t2-t1)*6*tb;
    else if (tb < 1.0/2.0)
      b = t2;
    else if (tb < 2.0/3.0)
      b = t1+(t2-t1)*(2.0/3.0 - tb)*6.0;

    set_rgb_p(r,g,b);
  }
}

// this is dangerous as random is not the same on many systems
void
Gdk_Color::set_random()
{
  set_rgb_p(rand()/(gdouble)RAND_MAX,
	    rand()/(gdouble)RAND_MAX,
	    rand()/(gdouble)RAND_MAX);
}

gdouble
Gdk_Color::distance_rgb_euclidean(const Gdk_Color& c) const
{
    gdouble rd = red_p() - c.red_p();
    gdouble gd = green_p() - c.green_p();
    gdouble bd = blue_p() - c.blue_p();
    return sqrt(rd*rd+gd*gd+bd*bd);
}

gdouble
Gdk_Color::distance_rgb_taxicab(const Gdk_Color& c) const
{
  gdouble rd = fabs(red_p() - c.red_p());
  gdouble gd = fabs(green_p() - c.green_p());
  gdouble bd = fabs(blue_p() - c.blue_p());

  // calculate max(rd,gd,bd)
  return rd < gd ? (gd < bd ? bd : gd) : (rd < bd ? bd : rd);
}

// $Id: color.cc,v 1.9 2000/08/27 20:15:42 kenelson Exp $

