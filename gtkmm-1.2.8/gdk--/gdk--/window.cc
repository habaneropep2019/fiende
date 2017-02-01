#include <gdk/gdk.h>
#include <gdk--/gc.h>
#include <gdk--/window.h>
#include <gdk--/pixmap.h>
#include <gdk--/bitmap.h>
#include <gdk--/visual.h>
#include <gdk--/colormap.h>
#include <gdk--/cursor.h>
#include <gdk/gdkprivate.h>

GTKMM_USING_STD_STRING;

void Gdk_Window::destroy()
  {
   gdk_window_destroy(obj_);
   obj_=0;
  }


void Gdk_Window::ref()
  {
   if (obj_) 
     gdk_window_ref(obj_);
  }

void Gdk_Window::unref()
{
  if (obj_) 
    {
#if GDK_VERSION_GT(1,2)
      GdkDrawablePrivate *priv = (GdkDrawablePrivate *)obj_;
#else
      GdkWindowPrivate *priv = (GdkWindowPrivate *)obj_;
#endif
      if (priv->ref_count==1)
        destroy(); 
      else
        gdk_window_unref(obj_);
    }
  obj_=0;
}

Gdk_Window::Gdk_Window():Gdk_Drawable(0) 
  {
  }

Gdk_Window::Gdk_Window(GdkWindow *win):Gdk_Drawable(win) 
  {
   ref();
  }

Gdk_Window::Gdk_Window(const Gdk_Window& win):Gdk_Drawable(win)
  {
   ref();
  }

Gdk_Window::Gdk_Window(const Gdk_Window     &parent,
		       const Gdk_WindowAttr  &attributes,
		       gint            attributes_mask):Gdk_Drawable(0)
  {
   create(parent,attributes,attributes_mask);
  }

Gdk_Window::Gdk_Window(const Gdk_WindowAttr  &attributes,
		       gint attributes_mask):Gdk_Drawable(0)
  {
   create(0,attributes,attributes_mask);
  }

Gdk_Window::~Gdk_Window()
  {
   unref();
  }

void Gdk_Window::create(const Gdk_Window     &parent,
			const Gdk_WindowAttr  &attributes,
			gint            attributes_mask)
  {
   unref();
   obj_=gdk_window_new(gdk_const_cast(parent), gdk_const_cast(attributes),
		       attributes_mask);
  }

Gdk_Window& Gdk_Window::operator = (const Gdk_Window& h)
  {
    if (h.obj_==obj_) return *this;
    unref();
    obj_=h.obj_;
    ref();
    return *this;
  }


/********************** Provided member functions *************************/

void Gdk_Window::show()
  {
   gdk_window_show(*this);
  }

void Gdk_Window::hide()
  {
   gdk_window_hide(*this);
  }

void Gdk_Window::withdraw()
  {
   gdk_window_withdraw(*this);
  }

void Gdk_Window::move(gint x, gint y)
  {
   gdk_window_move(*this,x,y);
  }

void Gdk_Window::resize(gint width,gint height)
  {
   gdk_window_resize(*this,width,height);
  }

void Gdk_Window::move_resize     (gint          x,
                      gint          y,
                      gint          width,
                      gint          height)
  {
   gdk_window_move_resize(*this,x,y,width,height);
  }

void Gdk_Window::reparent        (Gdk_Window   &new_parent,
                      gint          x,
                      gint          y)
  {
   gdk_window_reparent(*this,new_parent,x,y);
  }

void Gdk_Window::clear()
  {
   gdk_window_clear(*this);
  }

void Gdk_Window::clear_area      (gint          x,
                      gint          y,
                      gint          width,
                      gint          height)
  {
   gdk_window_clear_area(*this,x,y,width,height);
  }

void Gdk_Window::clear_area_e    (gint          x,
                      gint          y,
                      gint          width,
                      gint          height)
  {
   gdk_window_clear_area_e(*this,x,y,width,height);
  }


void Gdk_Window::raise()
  {
   gdk_window_raise(*this);
  }
  
void Gdk_Window::lower()
  {
   gdk_window_lower(*this);
  }

void Gdk_Window::set_override_redirect(bool override_redirect)
  {
   gdk_window_set_override_redirect(*this,override_redirect);
  }

void Gdk_Window::add_filter      (GdkFilterFunc  function,
                      gpointer       data)
  {
   gdk_window_add_filter(*this,function,data);
  }

void Gdk_Window::remove_filter   (GdkFilterFunc  function,
                      gpointer       data)
  {
   gdk_window_remove_filter(*this,function,data);
  }

void Gdk_Window::set_hints       (gint             x,
                      gint             y,
                      gint             min_width,
                      gint             min_height,
                      gint             max_width,
                      gint             max_height,
                      gint             flags)
  {
   gdk_window_set_hints(*this,x,y,
     min_width,min_height,max_width,max_height,flags);
  }

void Gdk_Window::set_title       (const Gtk::string &title)
  {
   gdk_window_set_title(*this,title.c_str());
  }

void Gdk_Window::set_background  (const Gdk_Color       &color)
  {
   gdk_window_set_background(*this,gdk_const_cast(color));
  }

void Gdk_Window::set_back_pixmap 
                          (Gdk_Pixmap      &pixmap,
                           gint             parent_relative)
       {gdk_window_set_back_pixmap(*this,pixmap,parent_relative);
       }


void Gdk_Window::set_cursor      (const Gdk_Cursor &cursor)
  {
    gdk_window_set_cursor(*this,cursor);
  }

void Gdk_Window::set_colormap    (const Gdk_Colormap &colormap)
  {
    gdk_window_set_colormap(*this,colormap);
  }

void Gdk_Window::get_geometry    (gint            &x,
                           gint            &y,
                           gint            &width,
                           gint            &height,
                           gint            &depth)
  {
   gdk_window_get_geometry(*this,&x,&y,&width,&height,&depth);
  }

gint Gdk_Window::get_depth       ()
  {
   int d1,d2,d3,d4,depth;
   get_geometry(d1,d2,d3,d4,depth);
   return depth;
  }


Gdk_Colormap  Gdk_Window::get_colormap()
  {
   return Gdk_Colormap(gdk_window_get_colormap(*this));
  }

gint Gdk_Window::get_origin      (gint            &x,
                      gint            &y)
  {
   return gdk_window_get_origin(*this,&x,&y);
  }
#if GDK_VERSION_GT(1,0)


gint Gdk_Window::get_deskrelative_origin      
                     (gint            &x,
                      gint            &y)
  {
   return gdk_window_get_deskrelative_origin(*this,&x,&y);
  }


void Gdk_Window::get_root_origin (gint            &x,
                      gint            &y)
  {
    gdk_window_get_root_origin(*this,&x,&y);
  }
#endif

Gdk_Window Gdk_Window::get_pointer(gint            &x,
                      gint            &y,
                      GdkModifierType &mask)
  {
   return gdk_window_get_pointer(*this,&x,&y,&mask);
  }

Gdk_Window Gdk_Window::get_parent()
  {
   return Gdk_Window(gdk_window_get_parent(*this));
  }

Gdk_Window Gdk_Window::get_toplevel()
  {
   return Gdk_Window(gdk_window_get_toplevel(*this));
  }

GdkEventMask  Gdk_Window::get_events()
  {
   return gdk_window_get_events(*this);
  }

void Gdk_Window::set_events      (GdkEventMask     event_mask)
  {
   gdk_window_set_events(*this,event_mask);
  }

void Gdk_Window::set_icon        (Gdk_Window       &icon_window,
                      Gdk_Pixmap       &pixmap,
                      Gdk_Bitmap       &mask)
  {
   gdk_window_set_icon(*this,icon_window,pixmap,mask);
  }

void Gdk_Window::set_icon_name   (const Gtk::string     &name)
  {
   gdk_window_set_icon_name(*this,const_cast<gchar*>(name.c_str()));
  }

void Gdk_Window::set_group       (Gdk_Window      &leader)
  {
   gdk_window_set_group(*this,leader);
  }

void Gdk_Window::set_decorations (GdkWMDecoration  decorations)
  {
   gdk_window_set_decorations(*this,decorations);
  }

void Gdk_Window::set_functions   (GdkWMFunction    functions)
  {gdk_window_set_functions(*this,functions);
  }

#if GDK_VERSION_GT(1,0)

void Gdk_Window::set_geometry_hints(Gdk_Geometry &geometry,GdkWindowHints flags)
  {
   gdk_window_set_geometry_hints(*this,geometry,flags);
  }

void Gdk_Window::set_role(const Gtk::string& role)
  {
   gdk_window_set_role(*this,role.c_str()); 
  }

void Gdk_Window::set_transient_for(Gdk_Window &leader)
  {
   gdk_window_set_transient_for(*this,leader);
  }

#endif

#ifdef GTKMM_CXX_HAVE_PARTIAL_SPECIALIZATION

Gdk_Window::iterator Gdk_Window::get_children_begin()
  {
   return iterator(gdk_window_get_children(*this));
  }

Gdk_Window::iterator Gdk_Window::get_children_end()
  {
   GList* list=gdk_window_get_children(*this);
   return iterator(g_list_last(list));
  }

#if GDK_VERSION_GT(1,0)
Gdk_Window::iterator Gdk_Window::get_toplevels_begin()
  {
   return iterator(gdk_window_get_toplevels());
  }

Gdk_Window::iterator Gdk_Window::get_toplevels_end()
  {
   GList* list=gdk_window_get_toplevels();
   return iterator(g_list_last(list));
  }

#endif 
#endif

/*************************************************************
***** DEPRECATED
**************************************************************
  The following interfaces have been replaced with identical
  functionality back in the tree, but so as to mantain
  the interface they are still provided. 
   
  These interfaces shall be removed on the next binary upgrade.
*************************************************************/

void Gdk_Window::copy_area       (Gdk_GC       &gc,
                      gint          x,
                      gint          y,
                      Gdk_Window   &src_window,
                      gint          src_x,
                      gint          src_y,
                      gint          width,
                      gint          height)
  {
   Gdk_Drawable::copy_area(gc,x,y,src_window,src_x,src_y,width,height);
  }

GdkWindowType Gdk_Window::get_type()
  {
   return Gdk_Drawable::get_type();
  }

void Gdk_Window::get_position    (gint            &x,
                      gint            &y)
  {
   Gdk_Drawable::get_position(x,y);
  } 

void Gdk_Window::get_size        (gint            &width,
                      gint            &height)
  {
   Gdk_Drawable::get_size(width,height);
  } 

Gdk_Visual    Gdk_Window::get_visual()
  {
   return Gdk_Drawable::get_visual();
  }

//void Gdk_Window::get_user_data   (gpointer        *data);

void Gdk_Window::set_user_data   (gpointer         user_data)
  {
   Gdk_Drawable::set_user_data(user_data);
  }

