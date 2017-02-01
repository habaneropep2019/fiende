// This is -*- C++ -*-
// $Id: gc.cc,v 1.19 2000/07/03 17:07:05 kenelson Exp $

/* 
 * gc.cc
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

// We should ask why gdk_gc_change doesn't exist! 

#include <gdk/gdk.h>
#include <gdk/gdkprivate.h>
#include <gdk--/gc.h>
#include <gdk--/font.h>
#include <gdk--/drawable.h>
#include <gdk--/pixmap.h>
#include <gdk--/bitmap.h>

/********************************************************************/


void Gdk_GC::ref()
  {
   if (obj_) 
     gdk_gc_ref(obj_);
  }

void Gdk_GC::unref()
  {
   if (obj_) 
    gdk_gc_unref(obj_);
   obj_=0;
  }

Gdk_GC::Gdk_GC():Gdk_Handle<GdkGC>(0)
  {}

Gdk_GC::Gdk_GC(GdkGC* gc)
  :Gdk_Handle<GdkGC>(gc)
  { 
   ref();
  } 

Gdk_GC::Gdk_GC(const Gdk_GC& gc): Gdk_Handle<GdkGC>(0)
  {
   obj_=gc.obj_;
   ref();
  }
  
Gdk_GC::Gdk_GC(Gdk_Drawable &drawable):Gdk_Handle<GdkGC>(0)
  {
   create(drawable);
  }

Gdk_GC::~Gdk_GC()
  {
   unref();
  }

void Gdk_GC::create(const Gdk_Drawable &drawable)
  {
   unref();
   if (drawable.connected()) 
     obj_ = gdk_gc_new(gdk_const_cast(drawable));
  }

void Gdk_GC::release()
  {
    unref();
    obj_=0;
  }

Gdk_GC& Gdk_GC::operator = (const Gdk_GC& h)
  {
    if (h.obj_==obj_) return *this;
    unref();
    obj_=h.obj_;
    ref();
    return *this;
  }



void Gdk_GC::destroy()
  {
   gdk_gc_destroy(obj_);
   obj_=0;
  }

void Gdk_GC::copy(Gdk_GC &src)
  {
   // gdk_gc_copy doesn't check for valid gc.
   g_return_if_fail(obj_ && src.obj_);
   if (obj_&&src.obj_)
     gdk_gc_copy(obj_,src.obj_);
  }

void Gdk_GC::get_values(GdkGCValues &values)
  {
   gdk_gc_get_values(*this,&values);
  }

void Gdk_GC::set_foreground(const Gdk_Color& c) 
  {
   gdk_gc_set_foreground(*this, gdk_const_cast(c));
  }
 
void Gdk_GC::set_background(const Gdk_Color& c) 
  {
   gdk_gc_set_background(*this, gdk_const_cast(c));
  }

void Gdk_GC::set_font(const Gdk_Font& font) 
  {
   gdk_gc_set_font(*this, gdk_const_cast(font));
  }
	
Gdk_Font Gdk_GC::get_font()
  {
   GdkGCValues val;
   gdk_gc_get_values(*this, &val);
   return Gdk_Font(val.font);
  }
 
void Gdk_GC::set_function(GdkFunction func) 
  {
   gdk_gc_set_function(*this, func);
  }

void Gdk_GC::set_fill(GdkFill fill) 
  {
   gdk_gc_set_fill(*this, fill);
  }

void Gdk_GC::set_tile(Gdk_Pixmap &tile) 
  {
   gdk_gc_set_tile(*this,tile);
  }

void Gdk_GC::set_stipple(Gdk_Pixmap &stip)
  {
   gdk_gc_set_stipple(*this,stip);
  }

void Gdk_GC::set_ts_origin(gint x, gint y) 
  {
   gdk_gc_set_ts_origin(*this, x, y);
  }

void Gdk_GC::set_clip_origin(gint x, gint y) 
  {
   gdk_gc_set_clip_origin(*this,x,y);
  }

void Gdk_GC::set_clip_mask(Gdk_Bitmap &mask)
  {         
    gdk_gc_set_clip_mask(*this,mask);
  }

void Gdk_GC::set_clip_mask() 
  {
   gdk_gc_set_clip_mask(*this, 0);
  }

void Gdk_GC::set_noclip() 
  {
   gdk_gc_set_clip_mask(*this, 0);
  }

void  Gdk_GC:: set_clip_rectangle  (const Gdk_Rectangle &rect)
  {
   gdk_gc_set_clip_rectangle(*this, gdk_const_cast(rect));
  }

void Gdk_GC::set_clip_rectangle(gint x, gint y, gint w, gint h) 
  {
   set_clip_rectangle(Gdk_Rectangle(x,y,w,h));
  }

void Gdk_GC::set_clip_region(Gdk_Region &region)
  {
   gdk_gc_set_clip_region(*this,region);
  }

void Gdk_GC::set_subwindow(GdkSubwindowMode mode) 
  {
   gdk_gc_set_subwindow(*this, mode);
  }

void Gdk_GC::set_exposures(bool exp) 
  {
   gdk_gc_set_exposures(*this, exp);
  }

void Gdk_GC::set_line_attributes(gint line_width,
                         GdkLineStyle line_style,
                         GdkCapStyle cap_style,
                         GdkJoinStyle join_style) 
  {
   // g_return_if_fail(obj_ != 0);
   gdk_gc_set_line_attributes(*this, line_width, line_style, 
     cap_style, join_style);
  }


void Gdk_GC::set_line_width(gint line_width)
  {
    g_return_if_fail(obj_ != 0);
    GdkGCValues val;
    gdk_gc_get_values(*this, &val);
    set_line_attributes(line_width,val.line_style,val.cap_style,val.join_style);
  }

void Gdk_GC::set_line_style(GdkLineStyle line_style)
  {
    g_return_if_fail(obj_ != 0);
    GdkGCValues val;
    gdk_gc_get_values(*this, &val);
    set_line_attributes(val.line_width,line_style,val.cap_style,val.join_style);
  }

void Gdk_GC::set_cap_style(GdkCapStyle cap_style)
  {
    g_return_if_fail(obj_ != 0);
    GdkGCValues val;
    gdk_gc_get_values(*this, &val);
    set_line_attributes(val.line_width,val.line_style,cap_style,val.join_style);
  }

void Gdk_GC::set_join_style(GdkJoinStyle join_style)
  {
    g_return_if_fail(obj_ != 0);
    GdkGCValues val;
    gdk_gc_get_values(*this, &val);
    set_line_attributes(val.line_width,val.line_style,val.cap_style,join_style);
  }

void Gdk_GC::set_dashes(gint size)
  {
   // g_return_if_fail(obj_ != 0);
   gint8 d[2] = { size, size };
   set_dashes(0,d,2);
  }

void Gdk_GC::set_dashes(gint on, gint off)
  {
   // g_return_if_fail(obj_ != 0);
   gint8 d[2] = { on, off };
   set_dashes(0,d,2);
  }

void Gdk_GC::set_dashes(gint      dash_offset,
			gint8             dash_list[],
			gint              n)
  {
// some very bad version stuff here
#if GDK_VERSION_GT_MICRO(1,2,6)
#if GDK_VERSION_LT(1,3)
    gdk_gc_set_dashes(*this,dash_offset, dash_list,n);
#else
    gdk_gc_set_dashes(*this,dash_offset, (gchar*) dash_list,n);
#endif
#else
    gdk_gc_set_dashes(*this,dash_offset, (gchar*) dash_list,n);
#endif
  }
 
