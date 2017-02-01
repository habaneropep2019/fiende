// This is -*- C++ -*-

/* 
 * font.h
 *
 * Copyright 1998-2000 Karl Einar Nelson
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
#ifndef _GDKMM_FONT_H_
#define _GDKMM_FONT_H_

#include <gdk--/types.h>

namespace Gtk
{
  GTKMM_USING_STD_STRING;  
}

class Gdk_Font: public Gdk_Handle<GdkFont>
  {
   protected:

     // Internal for gdk reference counters
     void ref();

     // Internal for gdk reference counters
     void unref();

/* REQUIRED Handle Constructors and Destructors */
#ifdef GDKMM_HANDLES_CONNECTED_ONLY
   protected:
#else
   public:
#endif
     //: Create a unconnected font.
     Gdk_Font();

   public:
     //: Create a font handle from a GdkFont
     Gdk_Font(GdkFont *font);
  
     Gdk_Font(const Gdk_Font& font);

     //: Create a font handle loaded with specific fontname.
     Gdk_Font(const Gtk::string &font_name);

     //: Destroy a font handle.
     ~Gdk_Font();

     void create(const Gtk::string &font_name);

     void release();
     Gdk_Font& operator = (const Gdk_Font&);

     /* Provided member functions */
     
     Gdk_Font& load(const Gtk::string &font_name);

     Gdk_Font& set_load(const Gtk::string &fontset_name);

     gint font_id() const;

     gint equal(const Gdk_Font  &font) const;

     gint ascent() const;
     gint descent() const;
     gint height() const;

     gint string_width(const Gtk::string &str) const ;

     gint text_width(const char* text,int length) const;

     gint char_width(gchar     character) const;

     gint string_measure(const Gtk::string &str) const;

     gint text_measure(const char* text,int length) const;

     gint char_measure(gchar     character) const;
#if GDK_VERSION_GT(1,0)
     gint string_height(const Gtk::string &str) const;

     gint text_height(const char* text,int length) const;

     gint char_height(gchar character) const;

     void string_extents (const Gtk::string &str,
			  gint        &lbearing,
			  gint        &rbearing,
			  gint        &width,
			  gint        &ascent,
			  gint        &descent) const;

     void text_extents(const char *text, 
                       gint length,
                       gint &lbearing,
                       gint &rbearing,
                       gint &width,
                       gint &ascent,
                       gint &descent) const;

     /* These wrappers are quit raw.  We need to review them later */
     gint text_width_wc(const GdkWChar* text,gint text_length) const;

     gint char_width_wc(GdkWChar character) const;

     void text_extents_wc( const GdkWChar *text,                              
                           gint            text_length,
                           gint           &lbearing,
                           gint           &rbearing,
                           gint           &width,
                           gint           &ascent,
                           gint           &descent) const;

#endif
  };

#endif //_GDKMM_FONT_H_
