#include <gdk--/colormap.h>

void Gdk_Colormap::ref()
  {
   if (obj_) 
     gdk_colormap_ref(obj_);
  }

void Gdk_Colormap::unref()
  {
   if (obj_) 
     gdk_colormap_unref(obj_);
   obj_=0;
  }

Gdk_Colormap::Gdk_Colormap()
  :Gdk_Handle<GdkColormap>(0) 
  {
  }

Gdk_Colormap::Gdk_Colormap(GdkColormap *colormap)
  :Gdk_Handle<GdkColormap> (colormap)
  {
   ref();
  }

Gdk_Colormap::Gdk_Colormap(const Gdk_Colormap& colormap)
  :Gdk_Handle<GdkColormap> (colormap.obj_) 
  {
   ref();
  }

Gdk_Colormap::~Gdk_Colormap()
  {
   unref();
  }

void create() 
  {} // Fix me!!

void Gdk_Colormap::release()
  {
    unref();
    obj_=0;
  }

Gdk_Colormap& Gdk_Colormap::operator = (const Gdk_Colormap& h)
  {
    if (h.obj_==obj_) return *this;
    unref();
    obj_=h.obj_;
    ref();
    return *this;
  }


Gdk_Color Gdk_Colormap::white()
  {
   Gdk_Color color;
   gdk_color_white(*this,color.gdkobj());
   return color;
  }

Gdk_Color Gdk_Colormap::black()
  {
   Gdk_Color color;
   gdk_color_black(*this,color.gdkobj());
   return color;
  }

gint Gdk_Colormap::alloc(Gdk_Color &color)
  {
   return gdk_color_alloc(*this,color.gdkobj());
  }

gint Gdk_Colormap::change(Gdk_Color &color)
  {
   return gdk_color_change(*this,color.gdkobj());
  }

void Gdk_Colormap::change(gint ncolors)
  {
   gdk_colormap_change(*this,ncolors);
  }

Gdk_Colormap Gdk_Colormap::get_system()
  {
   GdkColormap *colormap=Gdk_Colormap(gdk_colormap_get_system());
   return colormap;
  }

gint Gdk_Colormap::get_system_size()
  {
   return gdk_colormap_get_system_size();
  }
