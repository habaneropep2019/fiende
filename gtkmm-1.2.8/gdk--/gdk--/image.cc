#include <gdk--/image.h>
#include <gdk--/drawable.h>
#include <gdk--/visual.h>


void Gdk_Image::destroy()
  {
   if (obj_)
     gdk_image_destroy(obj_);
   obj_=0;
  }

void Gdk_Image::ref() 
  {}

void Gdk_Image::unref() 
  {destroy();}

Gdk_Image::Gdk_Image():Gdk_Handle<GdkImage>(0)
  {
  }

Gdk_Image::Gdk_Image(GdkImage* image)
  :Gdk_Handle<GdkImage>(image)
  {
  } 

Gdk_Image::Gdk_Image(GdkImageType type,
          Gdk_Visual     &visual,
          gint           width,
          gint           height)
  : Gdk_Handle<GdkImage>(0)
  {
   create(type,visual,width,height);
  } 

Gdk_Image::~Gdk_Image()
  {
   destroy();
  }

void Gdk_Image::release()
  {
   destroy();
  }

GdkImage* Gdk_Image::operator -> ()
  {
   return obj_;
  }

const GdkImage* Gdk_Image::operator -> () const
  {
   return obj_;
  }

void Gdk_Image::create(GdkImageType type,
            Gdk_Visual&    visual,
            gint           width,
            gint           height)
  {
   destroy();
   obj_=gdk_image_new(type,visual,width,height);
  }

void Gdk_Image::get(Gdk_Drawable &drawable,
         gint x,gint y,gint width,gint height)
  {
   destroy();
   obj_=gdk_image_get(drawable,x,y,width,height);
  }

void Gdk_Image::put_pixel(gint x,gint y,guint32 pixel)
  {
   gdk_image_put_pixel(*this,x,y,pixel);
  }

guint32 Gdk_Image::get_pixel(gint x,gint y) const
  {
   return gdk_image_get_pixel(gdk_const_cast(*this),x,y);
  }

void Gdk_Image::assign(GdkImage* &img)
  {  
   destroy();
   obj_=img;
   img=0;
  }

