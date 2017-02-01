#include <gdk/gdkprivate.h>

#include <gdk--/drawable.h>
#include <gdk--/font.h>
#include <gdk--/gc.h>
#include <gdk--/image.h>
#include <gdk--/pixmap.h>
#include <gdk--/bitmap.h>
#include <gdk--/visual.h>


GTKMM_USING_STD_STRING;

Gdk_Drawable::Gdk_Drawable ()
  :Gdk_Handle<GdkDrawable>(0) {}

Gdk_Drawable::Gdk_Drawable (GdkDrawable *drawable)
  :Gdk_Handle<GdkDrawable>(drawable) {}

Gdk_Drawable::Gdk_Drawable (const Gdk_Drawable &drawable)
  :Gdk_Handle<GdkDrawable>(drawable) {}

Gdk_Drawable::~Gdk_Drawable () {}

void Gdk_Drawable::release()  
  { 
    unref(); 
    obj_=0;
  }


Gdk_Drawable& Gdk_Drawable::operator = (const Gdk_Drawable& h)
  {
    if (h.obj_==obj_) return *this;
    unref();
    obj_=h.obj_;
    ref();
    return *this;
  }


/****************************Methods*******************************/
void Gdk_Drawable::draw_point      (const Gdk_GC       &gc,
                     gint          x1,
                     gint          y1)
  {
   gdk_draw_point(obj_,gc,x1,y1);
  }

void Gdk_Drawable::draw_line      (const Gdk_GC       &gc,
                     gint          x1,
                     gint          y1,
                     gint          x2,
                     gint          y2)
  {
   gdk_draw_line(obj_,gc,x1,y1,x2,y2);
  }

void Gdk_Drawable::draw_rectangle (const Gdk_GC       &gc,
                     gint          filled,
                     gint          x,
                     gint          y,
                     gint          width,
                     gint          height)
  {
   gdk_draw_rectangle(obj_,gc,filled,x,y,width,height);
  }

void Gdk_Drawable::draw_arc       (const Gdk_GC       &gc,
                     gint          filled,
                     gint          x,
                     gint          y,
                     gint          width,
                     gint          height,
                     gint          angle1,
                     gint          angle2)
  {
   gdk_draw_arc(obj_,gc,filled,x,y,width,height,
                angle1,angle2);
  }

void Gdk_Drawable::draw_polygon   (const Gdk_GC       &gc,
                     gint          filled,
                     const Gdk_Points   &points)
  {
   gdk_draw_polygon(obj_,gc,filled,points,points.size());
  }

void Gdk_Drawable::draw_string    (const Gdk_Font     &font,
                     const Gdk_GC       &gc,
                     gint          x,
                     gint          y,
                     const Gtk::string &str)
  {
   gdk_draw_string (obj_,font,gc,x,y,str.c_str());
  }

void Gdk_Drawable::draw_text      (const Gdk_Font     &font,
                     const Gdk_GC       &gc,
                     gint          x,
                     gint          y,
                     const char* text,
                     gint          text_length)
  {
   gdk_draw_text (obj_,font,gc,
     x,y,text,text_length);
  }

void Gdk_Drawable::draw_pixmap     
                          (const Gdk_GC       &gc,
                           const Gdk_Drawable &src,
                           gint          xsrc,
                           gint          ysrc,
                           gint          xdest,
                           gint          ydest,
                           gint          width,
                           gint          height)
  {
   gdk_draw_pixmap (obj_,gc,src,
     xsrc,ysrc,xdest,ydest,width,height);
  }

void Gdk_Drawable::draw_bitmap    
                         (const Gdk_GC       &gc,
                          const Gdk_Bitmap   &src,
                          gint          xsrc,
                          gint          ysrc,
                          gint          xdest,
                          gint          ydest,
                          gint          width,
                          gint          height)
  {
   gdk_draw_pixmap (obj_,gc,src,
     xsrc,ysrc,xdest,ydest,width,height);
  }

void Gdk_Drawable::draw_image     
                         (const Gdk_GC       &gc,
                          const Gdk_Image    &image,
                          gint          xsrc,
                          gint          ysrc,
                          gint          xdest,
                          gint          ydest,
                          gint          width,
                          gint          height)
  {
   gdk_draw_image (obj_,gc,image,
   xsrc,ysrc,xdest,ydest,width,height);
  }

void Gdk_Drawable::draw_points
			 (const Gdk_GC 	&gc,
			  const Gdk_Points	&points)
  {
   gdk_draw_points(obj_,gc,points,points.size());
  }

void Gdk_Drawable::draw_points    
                         (const Gdk_GC        &gc,
                          GdkPoint    *points,
                          gint          npoints)
  {
   gdk_draw_points (obj_,gc,points,npoints);
  }

void Gdk_Drawable::draw_segments  
                         (const Gdk_GC       &gc,
                          GdkSegment  *segs,
                          gint          nsegs)
  {
   gdk_draw_segments (obj_,gc,segs,nsegs);
  }

void Gdk_Drawable::draw_lines
			 (const Gdk_GC	&gc,
			  const Gdk_Points	&points)
  {
   gdk_draw_lines(obj_,gc,points,points.size());
  }

void Gdk_Drawable::draw_lines     
                         (const Gdk_GC        &gc,
                          GdkPoint     *points,
                          gint          npoints)
  {
   gdk_draw_lines (obj_,gc,points,npoints);
  }

void Gdk_Drawable::draw_polygon    
                          (const Gdk_GC        &gc,
                           gint        filled,
                           GdkPoint    *points,
                           gint          npoints)
  {
   gdk_draw_polygon(obj_,gc,filled,points,npoints);
  }

#if GDK_VERSION_GT(1,0)
void Gdk_Drawable::draw_text_wc   (const Gdk_Font      &font,
                     const Gdk_GC        &gc,
                     gint          x,
                     gint          y,
                     const GdkWChar *text,
                     gint          text_length)
  {
   gdk_draw_text_wc(*this,font,gc,x,y,text,text_length);
  }    
#endif

//***** Window methods that should be drawable
void Gdk_Drawable::copy_area       (const Gdk_GC       &gc,
                      gint          x,
                      gint          y,
                      const Gdk_Drawable   &source_window,
                      gint          source_x,
                      gint          source_y,
                      gint          width,
                      gint          height)
  {
   gdk_window_copy_area(*this,gc,x,y,
     source_window,source_x,source_y,width,height);
  }

void Gdk_Drawable::set_user_data   (gpointer         user_data)
  {
   gdk_window_set_user_data(*this,user_data);
  }

gpointer Gdk_Drawable::get_user_data()
  {
   gpointer rc;
   gdk_window_get_user_data(*this,&rc);
   return rc;
  }

#if GDK_VERSION_GT(1,2)
void Gdk_Drawable::set_data(const Gtk::string& key,
                   gpointer data,
                   GDestroyNotify destroy_func)
  {
   gdk_drawable_set_data(*this,key.c_str(),data,destroy_func);
  } 
#endif

void Gdk_Drawable::get_size(gint &width,gint &height)
  {
   gdk_window_get_size(*this,&width,&height);
  }

void Gdk_Drawable::get_position    (gint            &x,
                      gint            &y)
  {
   gdk_window_get_position(*this,&x,&y);
  }

Gdk_Visual    Gdk_Drawable::get_visual()
  {
   return Gdk_Visual(gdk_window_get_visual(*this));
  }

GdkWindowType Gdk_Drawable::get_type()
  {
   return gdk_window_get_type(*this);
  }

gint Gdk_Drawable::width()
  {
#if GDK_VERSION_GT(1,2)
   GdkDrawablePrivate *p=(GdkDrawablePrivate*)gdkobj();
#else
   GdkWindowPrivate *p=(GdkWindowPrivate*)gdkobj();
#endif
   g_return_val_if_fail(p!=NULL,0);
   return p->width;
  }

gint Gdk_Drawable::height()
  {
#if GDK_VERSION_GT(1,2)
   GdkDrawablePrivate *p=(GdkDrawablePrivate*)gdkobj();
#else
   GdkWindowPrivate *p=(GdkWindowPrivate*)gdkobj();
#endif
   g_return_val_if_fail(p!=NULL,0);
   return p->height;
  }

#if GDK_VERSION_GT(1,2)
// this should be moved to Window in 1.3
#else
gint Gdk_Drawable::x()
  {
   GdkWindowPrivate *p=(GdkWindowPrivate*)gdkobj();
   g_return_val_if_fail(p!=NULL,0);
   return p->x;
  }

gint Gdk_Drawable::y()
  {
   GdkWindowPrivate *p=(GdkWindowPrivate*)gdkobj();
   g_return_val_if_fail(p!=NULL,0);
   return p->y;
  }
#endif



