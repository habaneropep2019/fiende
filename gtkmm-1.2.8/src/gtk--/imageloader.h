// -*- mode: C++ c-basic-offset: 4  -*-
#ifndef _GTKMM_IMAGELOADER_H
#define _GTKMM_IMAGELOADER_H

/*
 * imageloader.h
 *
 * Copyright 2000 The Gtk-- Development Team
 * Copyright 2000 Joe Yandle <lupus@lycaeum.org> 
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

#include <gdk--/bitmap.h>
#include <gdk--/colormap.h>
#include <gdk--/pixmap.h>
#include <gtk--/widget.h>

namespace Gtk {
 
class ImageLoader 
  {
    protected:
      Gdk_Pixmap pix_;
      Gdk_Bitmap bit_;
      Gdk_Colormap cmap_;
  
      ImageLoader() {}
    public:
      ImageLoader(const string xpm_file) 
        {
          cmap_ = Gtk::Widget::get_default_colormap();
          pix_.create_colormap_from_xpm(Gdk_Pixmap(), cmap_, bit_, Gdk_Color(0), xpm_file);
        }

      ImageLoader(const string xpm_file, const Gdk_Color& transparent)
        {
          cmap_ = Gtk::Widget::get_default_colormap();
          pix_.create_colormap_from_xpm(Gdk_Pixmap(), cmap_, bit_, transparent, xpm_file);
        }

      Gdk_Pixmap pix() { return pix_; }
      Gdk_Bitmap bit() { return bit_; }
      Gdk_Bitmap mask() { return bit_; }

      ~ImageLoader() {}
};

class ImageLoaderData : public ImageLoader
  {
    public:
      ImageLoaderData(const char * const* data)
        {
          cmap_ = Gtk::Widget::get_default_colormap();
          pix_.create_colormap_from_xpm_d(Gdk_Pixmap(), cmap_, bit_, Gdk_Color(0), data);
        }

      ImageLoaderData(const char * const* data, const Gdk_Color& transparent)
        {
          cmap_ = Gtk::Widget::get_default_colormap();
          pix_.create_colormap_from_xpm_d(Gdk_Pixmap(), cmap_, bit_, transparent, data);
        }

      ~ImageLoaderData() {}
  };

} /* namespace Gtk */

#endif /* _GTKMM_IMAGELOADER_H */

