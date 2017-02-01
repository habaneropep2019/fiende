// This is -*- C++ -*-

/* 
 * window.h
 *
 * Copyright 1998-2000 Karl Einar Nelson
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

#ifndef _GDKMM_WINDOW_H_
#define _GDKMM_WINDOW_H_

// currently missing some methods and too many dependencies on
// gdk types.  Dnd interface missing
#include <gdk--/types.h>
#include <gdk--/drawable.h>
#include <gdk--/list.h>

namespace Gtk
{
  GTKMM_USING_STD_STRING;  
}

//: Gdk Window Handle
//- This is a handle to a remote window. 
//- Please read {\link Handles handles.htm}
class Gdk_Window:public Gdk_Drawable
  {
   public:
#ifdef GTKMM_CXX_HAVE_PARTIAL_SPECIALIZATION
     typedef Gdk_List_Iterator<GdkWindow,Gdk_Window> iterator;
#endif

   protected:
     //: (internal function) destroy window 
     void destroy();

     //: (internal function) increment reference counter 
     virtual void ref();

     //: (internal function) decrement reference counter
     //- Destroys the window if the count reachs zero.  (Not like 
     //- Gdk) 
     virtual void  unref();

/* REQUIRED Handle Constructor and Destructors */
#ifdef GDKMM_HANDLES_CONNECTED_ONLY
   protected:
#else
   public:
#endif
     //: Create an unconnected window handle.
     Gdk_Window();
   public:

     //: Wrap an existing GdkWindow.
     Gdk_Window(GdkWindow *win);

     //: Construct a window handle from an existing one.
     Gdk_Window(const Gdk_Window& win);

     //: Create a window.
     Gdk_Window(const Gdk_Window     &parent,
                const Gdk_WindowAttr  &attributes,
                gint            attributes_mask);

     //: Create a window having the root win. for parent
     Gdk_Window(const Gdk_WindowAttr  &attributes,
                gint            attributes_mask);

     //: Destroy a Window handle.
     virtual ~Gdk_Window();

     //GdkWindow *   gdk_window_foreign_new (guint32        anid);

     //: Create a new window on server.
     void create(const Gdk_Window     &parent,
                const Gdk_WindowAttr  &attributes,
                gint            attributes_mask);

     Gdk_Window& operator = (const Gdk_Window&);

     /* Provided member functions */

     void show();
     void hide();
     void withdraw();
     void move(gint x, gint y);
     void resize(gint width,gint height);
     void move_resize     (gint          x,
                           gint          y,
                           gint          width,
                           gint          height);
     void reparent        (Gdk_Window   &new_parent,
                           gint          x,
                           gint          y);
     void clear();
     void clear_area      (gint          x,
                           gint          y,
                           gint          width,
                           gint          height);
     void clear_area_e    (gint          x,
                           gint          y,
                           gint          width,
                           gint          height);

     // deprecated 
     void copy_area       (Gdk_GC       &gc,
                           gint          x,
                           gint          y,
                           Gdk_Window   &source_window,
                           gint          source_x,
                           gint          source_y,
                           gint          width,
                           gint          height);

     void raise();
     void lower();

     // deprecated 
     void set_user_data   (gpointer         user_data);

     void set_override_redirect(bool override_redirect);

     /* Filters */
     void add_filter      (GdkFilterFunc  function,
                           gpointer       data);
     void remove_filter   (GdkFilterFunc  function,
                           gpointer       data);
     void set_hints       (gint             x,
                           gint             y,
                           gint             min_width,
                           gint             min_height,
                           gint             max_width,
                           gint             max_height,
                           gint             flags);
     void set_title       (const Gtk::string &title);
     void set_background  (const Gdk_Color       &color);
     void set_back_pixmap (Gdk_Pixmap      &pixmap,
                           gint             parent_relative);

     void set_cursor      (const Gdk_Cursor   &cursor);
     void set_colormap    (const Gdk_Colormap &colormap);

     //: deprecated 
     void get_user_data   (gpointer        *data);

     void get_geometry    (gint            &x,
                           gint            &y,
                           gint            &width,
                           gint            &height,
                           gint            &depth);
     gint get_depth       ();

     //: deprecated
     void get_position    (gint            &x,
                           gint            &y);
 
     //: deprecated 
     void get_size        (gint            &width,
                           gint            &height);

     //: deprecated 
     Gdk_Visual    get_visual();

     Gdk_Colormap  get_colormap();

     //: deprecated 
     GdkWindowType get_type();

     gint get_origin      (gint            &x,
                           gint            &y);
#if GDK_VERSION_GT(1,0)
     //:
     gint get_deskrelative_origin      
                          (gint            &x,
                           gint            &y);

     //: 
     void get_root_origin (gint            &x,
                           gint            &y);
#endif
     Gdk_Window get_pointer(gint            &x,
                           gint            &y,
                           GdkModifierType &mask);
     Gdk_Window get_parent();
     Gdk_Window get_toplevel();

#ifdef GTKMM_CXX_HAVE_PARTIAL_SPECIALIZATION
     iterator get_children_begin  ();
     iterator get_children_end    ();
#endif

     GdkEventMask  get_events();
     void set_events      (GdkEventMask     event_mask);
     void set_icon        (Gdk_Window       &icon_window,
                           Gdk_Pixmap       &pixmap,
                           Gdk_Bitmap       &mask);
     void set_icon_name   (const Gtk::string     &name);
     void set_group       (Gdk_Window      &leader);
     void set_decorations (GdkWMDecoration  decorations);
     void set_functions   (GdkWMFunction    functions);
#if GDK_VERSION_GT(1,0)
     // Needs work.
#ifdef GTKMM_CXX_HAVE_PARTIAL_SPECIALIZATION
     iterator get_toplevels_begin(void);
     iterator get_toplevels_end(void);
#endif

     //: Set geometry Hints
     void set_geometry_hints(Gdk_Geometry &geometry,GdkWindowHints flags);

     //: Set Role
     void set_role(const Gtk::string& role);

     //
     void set_transient_for(Gdk_Window &leader);

#endif
  };

#endif // _GDKMM_WINDOW_H_
