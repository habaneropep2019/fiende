#include <gdk--/bitmap.h>
#include <gdk--/window.h>

void Gdk_Bitmap::ref()
  {
   if (obj_) 
     gdk_bitmap_ref(obj_);
  }
void Gdk_Bitmap::unref()
  {
   if (obj_) 
     gdk_bitmap_unref(obj_);
   obj_=0;
  }

Gdk_Bitmap::Gdk_Bitmap():Gdk_Drawable(0) 
  {}

Gdk_Bitmap::Gdk_Bitmap(GdkBitmap *bitmap):Gdk_Drawable(bitmap)
  {
   ref();
  }

Gdk_Bitmap::Gdk_Bitmap(const Gdk_Bitmap& bitmap):Gdk_Drawable(0)
  {
   obj_=bitmap.obj_;
   ref();
  }
    
Gdk_Bitmap::Gdk_Bitmap(Gdk_Window  &window,
           gchar      *data,
           gint        width,
           gint        height):Gdk_Drawable(0)
  {
   create(window,data,width,height);
  }

Gdk_Bitmap::Gdk_Bitmap(const Gdk_Drawable  &window,
           gchar      *data,
           gint        width,
           gint        height):Gdk_Drawable(0)
  {
   create(window,data,width,height);
  }

Gdk_Bitmap::~Gdk_Bitmap()
  {
   unref();
  }


void Gdk_Bitmap::create(Gdk_Window  &window,
           const gchar *data,
           gint        width,
           gint        height)
  {
   unref();
   obj_=gdk_bitmap_create_from_data(window,
         const_cast<gchar*>(data),width,height);
  }

void Gdk_Bitmap::create(const Gdk_Drawable  &window,
           const gchar *data,
           gint        width,
           gint        height)
  {
   unref();
   obj_=gdk_bitmap_create_from_data(window,
         const_cast<gchar*>(data),width,height);
  }

Gdk_Bitmap& Gdk_Bitmap::operator = (const Gdk_Bitmap& h)
  {
    if (h.obj_==obj_) return *this;
    unref();
    obj_=h.obj_;
    ref();
    return *this;
  }



