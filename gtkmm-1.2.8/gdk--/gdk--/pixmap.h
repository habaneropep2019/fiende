// This is -*- C++ -*-

/* 
 * pixmap.h
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
#ifndef _GDKMM_PIXMAP_H_
#define _GDKMM_PIXMAP_H_

#include <gdk--/drawable.h>

namespace Gtk
{
  GTKMM_USING_STD_STRING;  
}

//: Remote Pixmap Handle
//-  In all of the create and ctors the Drawable can be set
//-  to NULL in which case the pixmap will be based on the root
//-  window.  Also the Colormap can be set to NULL, in which
//-  case it will build the colormap off of the windows colormap.
//-  Depth can be set to -1 which will cause the pixmap to take
//-  they depth of the window. 
//-  
//-  If there are not enough sonstrains specified the pixmap
//-  will warn and return an unconnected Pixmap.
class Gdk_Pixmap:public Gdk_Drawable
{
  // internal functions for updating gdk counters
  virtual void ref();
  virtual void unref();

#ifdef GDKMM_HANDLES_CONNECTED_ONLY
protected:
#else
public:
#endif
  // Create an unconnected pixmap handle
  Gdk_Pixmap();

public:

     //: Wrap an existing Gdk pixmap.
  Gdk_Pixmap(GdkPixmap *pixmap);

  //: Construct a pixmap handle from an existing one.
  Gdk_Pixmap(Gdk_Pixmap const &pixmap);

  //: Create a Pixmap based on window.
  Gdk_Pixmap(const Gdk_Drawable  &drawable,
	     gint        width, 
	     gint        height,
	     gint        depth=-1);

  //: Create a Pixmap based on root window.
  Gdk_Pixmap(gint        width, 
	     gint        height,
	     gint        depth=-1);

  //: Create a Pixmap from an array of data.
  Gdk_Pixmap(const Gdk_Drawable  &drawable,
	     const gchar      *data,
	     gint        width,
	     gint        height,
	     gint        depth,
	     const Gdk_Color  &fg,
	     const Gdk_Color  &bg);

  //: Create a Pixmap from a xpm file.
  Gdk_Pixmap(const Gdk_Drawable  &drawable,
	     const Gdk_Color   &transparent_color,
	     const Gtk::string      &filename);
 
  //: Create a Pixmap from a xpm file.
  Gdk_Pixmap(const Gdk_Drawable  &drawable,
	     Gdk_Bitmap  &bitmap,
	     const Gdk_Color   &transparent_color,
	     const Gtk::string      &filename);
     
  //: Create a Pixmap from a xpm file with colormap.
  Gdk_Pixmap(const Gdk_Drawable   &drawable,
	     Gdk_Colormap &colormap,
	     Gdk_Bitmap   &bitmap,
	     const Gdk_Color    &transparent_color,
	     const Gtk::string &filename);

  //: Destroy a pixmap handle. 
  virtual ~Gdk_Pixmap();

  Gdk_Pixmap& operator = (const Gdk_Pixmap&);


  /* Provided member functions */
  //: Create a Pixmap for a specified window.  
  //-  depth defaults to that of the window. 
  void create(const Gdk_Drawable  &drawable,
	      gint        width,
	      gint        height,
	      gint        depth=-1);
    
  //: Create a Pixmap based off of root window.
  //-  depth defaults to that of root.
  void create(gint       width,
	      gint       height,
	      gint       depth=-1);

  //: Create a Pixmap from an array of data.
  void create_from_data(
			const Gdk_Drawable  &drawable,
			const gchar *data,
			gint        width,
			gint        height,
			gint        depth,
			const Gdk_Color   &fg,
			const Gdk_Color   &bg);

  //: Create a Pixmap from a xpm file.
  void create_from_xpm(
		       const Gdk_Drawable  &drawable,
		       const Gdk_Color   &transparent_color,
		       const Gtk::string      &filename);

  //: Create a Pixmap from a xpm file.
  void create_from_xpm(
		       const Gdk_Drawable  &drawable,
		       Gdk_Bitmap  &mask,
		       const Gdk_Color   &transparent_color,
		       const Gtk::string      &filename);
    
  //: Create a Pixmap from a xpm file with colormap.
  void create_colormap_from_xpm(
				const Gdk_Drawable   &drawable,
				Gdk_Colormap &colormap,
				Gdk_Bitmap   &bitmap,
				const Gdk_Color    &transparent_color,
				const Gtk::string &filename);
     
  void create_from_xpm_d(const Gdk_Drawable &drawable, 
			 Gdk_Bitmap       &bitmap,
			 const Gdk_Color  &transparent_color,
			 const gchar * const *data);
     
  void create_colormap_from_xpm_d (const Gdk_Drawable   &drawable,
				   Gdk_Colormap &colormap,
				   Gdk_Bitmap  &mask,
				   const Gdk_Color    &transparent_color,
				   const gchar * const *data);

  static void warn_about_xpm_file_problem(const Gtk::string &filename);

  static const char* const* defaultPixmap;
  
};

#endif // _GDKMM_PIXMAP_H_
