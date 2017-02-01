#include <gdk--/visual.h>

void Gdk_Visual::ref()
  {
   if (obj_) 
     gdk_visual_ref(obj_);
  }

void Gdk_Visual::unref()
  {
   if (obj_) 
     gdk_visual_unref(obj_);
   obj_=0;
  }

Gdk_Visual::Gdk_Visual()
  :Gdk_Handle<GdkVisual>(0)
  {}

Gdk_Visual::Gdk_Visual(GdkVisual* visual)
  :Gdk_Handle<GdkVisual>(visual)
  {
   ref();
  } 

Gdk_Visual::Gdk_Visual(const Gdk_Visual& visual)
  : Gdk_Handle<GdkVisual>(0)
  {
   obj_=visual.obj_;
   ref();
  }
  
Gdk_Visual::~Gdk_Visual()
  {
   unref();
  }

void Gdk_Visual::release()
  {
    unref();
    obj_=0;
  }

Gdk_Visual& Gdk_Visual::operator = (const Gdk_Visual& h)
  {
    if (h.obj_==obj_) return *this;
    unref();
    obj_=h.obj_;
    ref();
    return *this;
  }


gint Gdk_Visual::get_best_depth (void)
  {
   return gdk_visual_get_best_depth ();
  }
 
GdkVisualType Gdk_Visual::get_best_type (void)
  {
   return gdk_visual_get_best_type ();
  }
 
Gdk_Visual&  Gdk_Visual::get_system (void)
  {
   obj_=gdk_visual_get_system();
   ref();
   return *this;
  }
 
Gdk_Visual&  Gdk_Visual::get_best (void)
  {
   obj_=gdk_visual_get_best();
   ref();
   return *this;
  }
 
Gdk_Visual&  Gdk_Visual::get_best (gint depth)
  {
   obj_=gdk_visual_get_best_with_depth(depth);
   ref();
   return *this;
  }
 
Gdk_Visual&  Gdk_Visual::get_best (GdkVisualType  visual_type)
  {
   obj_=gdk_visual_get_best_with_type(visual_type);
   ref();
   return *this;
  }

Gdk_Visual&  Gdk_Visual::get_best (gint depth, GdkVisualType  visual_type)
  {
   obj_=gdk_visual_get_best_with_both(depth,visual_type);
   ref();
   return *this;
  }

