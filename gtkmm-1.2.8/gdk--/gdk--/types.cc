
#include "types.h"

Gdk_Rectangle::~Gdk_Rectangle() {}
Gdk_Segment::~Gdk_Segment() {}
Gdk_Point::~Gdk_Point() {}

#if !defined(__sgi) || defined(__GNUC__)
template <> Gdk_Handle<_GdkImage>::~Gdk_Handle() {}
template <> Gdk_Handle<_GdkVisual>::~Gdk_Handle() {}
template <> Gdk_Handle<_GdkGC>::~Gdk_Handle() {}
template <> Gdk_Handle<_GdkFont>::~Gdk_Handle() {}
template <> Gdk_Handle<_GdkWindow>::~Gdk_Handle() {}
template <> Gdk_Handle<_GdkColormap>::~Gdk_Handle() {}
#endif


void Gdk_Points::a_alloc(size_t size)
  {
    points_ = new GdkPoint[size];
    owned_ = true;
  }

Gdk_Points::Gdk_Points(GdkPoint * points, int size)
  {
    a_dup((GdkPoint*)points, (GdkPoint*)(&points[size]));
  }

Gdk_Points::Gdk_Points(Gdk_Point * points, int size)
  {
    a_dup((GdkPoint*)points, (GdkPoint*)(&points[size]));
  }

Gdk_Points::~Gdk_Points ()
  {
    if(owned_ == true)
      delete points_;
  }
