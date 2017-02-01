// This is -*- C++ -*-

/* 
 * image.h
 *
 * Copyright (C) 1998 Karl E. Nelson
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

#ifndef _GDKMM_IMAGE_H
#define _GDKMM_IMAGE_H

#include <gdk--/types.h>

//: Gdk Image  
//- An Image is a client side representation of a pixel map.  
//- Because it is client side it is slow to use, but takes no
//- server resources.  See Gdk_Pixmap for the server side version.
//- Images are on the client side so they are not actually handles;
//- however, they are dynamic and uncopyable. 
//- 
//- As a client side object it is not derived from drawable, 
//- so you can not use drawable methods on it.  The only
//- provided methods are set and get pixels.  It can
//- be drawn to a drawable with Gdk_Drawable::draw_image and yanked from
//- a drawable with Gdk_Image::get.
class Gdk_Image : public Gdk_Handle<GdkImage>
  {
   private:
     void destroy();
     void ref();
     void unref();

     Gdk_Image& operator = (const Gdk_Image& image);

   public: 
     //: Create an Image.
     Gdk_Image();

     //: Wrap an existing GdkImage.
     Gdk_Image(GdkImage* image);

     //: Initialize a Image from a existing one.
     //Gdk_Image(const Gdk_Image& image);
 
     //: Create a image 
     Gdk_Image(GdkImageType type,
               Gdk_Visual     &visual,
               gint           width,
               gint           height);

     //: Delete this handle.
     ~Gdk_Image();

     //: Access Image directly.
     GdkImage* operator -> ();

     //: Access Image directly.
     const GdkImage* operator -> () const;

     //: Create a image. 
     void create(GdkImageType type,
                 Gdk_Visual&    visual,
                 gint           width,
                 gint           height);

     void release();

     //: Grab a drawable contents.
     void get(Gdk_Drawable &drawable,
              gint x,gint y,gint width,gint height);
     
     //: Store a pixel in an image.
     void put_pixel(gint x,gint y,guint32 pixel);

     //: Examine a pixel in an image. 
     guint32 get_pixel(gint x,gint y) const;

     // 
     void assign(GdkImage* &img);
     
  };

#endif // _GDKMM_IMAGE_H









