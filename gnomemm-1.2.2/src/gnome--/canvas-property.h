#ifndef _GNOMEMM_CANVAS_PROPERTY_H_
#define _GNOMEMM_CANVAS_PROPERTY_H_

#include <gtk/gtkpacker.h> //For GtkAnchorType.

namespace Gnome {
namespace CanvasHelpers {

template <class T>
struct Property
  {
    const char* name_;
    T arg_;
    Property(const char* name,const T& arg)
      : name_(name), arg_(arg)
      {}
    const T& arg() const {return arg_; }
  };

struct Properties
{
  template <class O,class T>
  static void apply(O& object,const Property<T>& property)
    { object.set(property.name_,property.arg(),0); }
};


template <class O,class T>
O& operator << (O& object,const Property<T>& property)
  {
    Properties::apply(object,property);
    return object;
  }

/********* specializations *********/

//Colors can be specified with a string or a Gdk_Color.
//FIXME: rgb constructor?
template<>
struct Property<Gdk_Color>
  {
    const char* name_;
    Gtk::string color_;
    Gdk_Color arg_;
    Property(const char* name,const Gdk_Color& arg)
      : name_(name), color_(), arg_(arg)
      {}
    Property(const char* name,const Gtk::string& color)
      : name_(name), color_(color), arg_(0)
      {}
    const void* arg() const {return (color_.size()==0)?(void*)(arg_.gdkobj()):(void*)(color_.c_str()); }
  };

//Font can be specified with a string or a Gdk_Font.
template<>
struct Property<Gdk_Font>
  {
    const char* name_;
    Gdk_Font arg_;
    Gtk::string strfont_;

    Property(const char* name,const Gdk_Font& arg)
      : name_(name), arg_(arg)
      {}
    Property(const char* name,const Gtk::string& font)
      : name_(name), arg_(0), strfont_(font)
      {}

    const void* arg() const {return (strfont_.size()==0)?(void*)(arg_.gdkobj()):(void*)(strfont_.c_str()); }
  };

struct font : public Property<Gdk_Font>  //Used by CanvasText.
  {
    font(const Gdk_Font& v): Property<Gdk_Font>("font_gdk",v) {}
    font(const Gtk::string& v): Property<Gdk_Font>("font",v) {}
  };

template<>
struct Property<Gdk_Pixmap>
  {
    const char* name_;
    Gdk_Pixmap arg_;
    Property(const char* name,const Gdk_Pixmap& arg)
      : name_(name), arg_(arg)
      {}
    GdkPixmap* arg() const {return arg_.gdkobj(); }
  };

struct fill_color : public Property<Gdk_Color> 
  { 
    fill_color(const Gdk_Color& v): Property<Gdk_Color>("fill_color_gdk",v) {}
    fill_color(const Gtk::string& v): Property<Gdk_Color>("fill_color",v) {}
  };

struct outline_color : public Property<Gdk_Color>
  {
    outline_color(const Gdk_Color& v): Property<Gdk_Color>("outline_color_gdk",v) {}
    outline_color(const Gtk::string& v): Property<Gdk_Color>("outline_color",v) {}
  };




#define GNOMEMM_PROPERTY(N,N2,T) \
struct N : public Property<T> { N(const T& v): Property<T>(#N2,v) {}};


// CanvasLine
GNOMEMM_PROPERTY(arrow_shape_a,arrow_shape_a,gdouble)
GNOMEMM_PROPERTY(arrow_shape_b,arrow_shape_b,gdouble)
GNOMEMM_PROPERTY(arrow_shape_c,arrow_shape_c,gdouble)
GNOMEMM_PROPERTY(cap_style,cap_style,GdkCapStyle)
GNOMEMM_PROPERTY(first_arrowhead,first_arrowhead,bool)
GNOMEMM_PROPERTY(join_style,join_style,GdkJoinStyle)
GNOMEMM_PROPERTY(last_arrowhead,last_arrowhead,bool)
GNOMEMM_PROPERTY(line_stype,line_style,GdkLineStyle)
GNOMEMM_PROPERTY(smooth,smooth,bool)
GNOMEMM_PROPERTY(spline_step,spline_step,guint)

// CanvasText
GNOMEMM_PROPERTY(clip,clip,bool)
GNOMEMM_PROPERTY(clip_height,clip_height,gdouble)
GNOMEMM_PROPERTY(clip_width,clip_width,gdouble)
GNOMEMM_PROPERTY(justification,justification,GtkJustification)
GNOMEMM_PROPERTY(text_height,text_height,gdouble)
GNOMEMM_PROPERTY(text_width,text_width,gdouble)
GNOMEMM_PROPERTY(x_offset,x_offset,gdouble)
GNOMEMM_PROPERTY(y_offset,y_offset,gdouble)
//FIXME: fontset

// CanvasWidget
GNOMEMM_PROPERTY(size_pixels,size_pixels,bool)

// CanvasImage, CanvasWidget
GNOMEMM_PROPERTY(height,height,gdouble)
GNOMEMM_PROPERTY(width,width,gdouble)

// CanvasRect, CanvasEllipse
GNOMEMM_PROPERTY(x1,x1,gdouble)
GNOMEMM_PROPERTY(x2,x2,gdouble)
GNOMEMM_PROPERTY(y1,y1,gdouble)
GNOMEMM_PROPERTY(y2,y2,gdouble)

// CanvasImage, CanvasText, CanvasWidget
GNOMEMM_PROPERTY(anchor,anchor,GtkAnchorType)

// CanvasPolygon, CanvasRect, CanvasEllipse
GNOMEMM_PROPERTY(outline_stipple,outline_stipple,Gdk_Pixmap)

// CanvasLine, CanvasPolygon, CanvasRect, CanvasEllipse
GNOMEMM_PROPERTY(width_pixels,width_pixels,guint)
GNOMEMM_PROPERTY(width_units,width_units,gdouble)

// CanvasGroup, CanvasImage, CanvasText, CanvasWidget
GNOMEMM_PROPERTY(x,x,gdouble)
GNOMEMM_PROPERTY(y,y,gdouble)

// CanvasLine, CanvasPolygon, CanvasRect, CanvasEllipse, CanvasText
GNOMEMM_PROPERTY(fill_stipple,fill_stipple,Gdk_Pixmap)

} /* namespace CanvasHelpers */
} /* namespace Gnome */

#endif /* _GNOMEMM_CANVAS_PROPERTY_H_ */

