/*
  GTK-- Canvas Demo 

  Copyright 1998-1999 Kasper Peeters.
  Copyright 1999 Karl Nelson

  This program and its components are free software; you can
  redistribute it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software Foundation; either
  version 2, or (at your option) any later version.
 
  This program and its components are distributed in the hope that it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
  the GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License in
  the file COPYING accompanying this program; if not, write to the
  Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// This program demonstrates how to render on the background of
// a layout widget to use it as a drawing area.  
// 
// This provides for rapid smooth scrolling like you would expect
// from netscape.  It reduces flashing by matching the pixmap and
// background.  To use LayoutArea, derive it and replace the paint
// method with rendering on the background.

#include <iostream.h>
#include <gtk--/main.h>
#include <gtk--/button.h>
#include <gtk--/layout.h>
#include <gtk--/window.h>
#include <gtk--/table.h>
#include <gtk--/scrollbar.h>

using SigC::slot;

/***********************************************************
***** Layout with double buffered drawing area
***********************************************************/
class LayoutArea : public Gtk::Layout 
  {
   protected:
      Gdk_Pixmap   pixmap_; 
      Gdk_Pixmap   bin_pixmap_;
      Gdk_Pixmap   null_pixmap_;
      Gdk_GC       gc_;
      Gdk_Color    background_color_;

      gint         bin_width,bin_height;
      bool         has_background_;

   public:
      LayoutArea(Gtk::Adjustment &hadj,Gtk::Adjustment &vadj)
         :Gtk::Layout(hadj,vadj),
          pixmap_(0),bin_pixmap_(0),null_pixmap_(0),gc_(0)
         {
          // set event mask so we are sure to get events 
          set_events(GDK_EXPOSURE_MASK); 
          has_background_=0;
         }

      // this is what is filled out by derived classes
      virtual void paint()=0;
 
      // flush part of the pixmap to the background
      void flush(int x,int y,int w,int h);

      // reallocate the background map 
      void back_alloc();

      void set_background(const Gdk_Color& bg) 
        {
          has_background_=true;
          background_color_=bg;
        }

      virtual gint expose_event_impl(GdkEventExpose* e);
      virtual void realize_impl();
  };

void LayoutArea::flush(int x,int y,int w,int h)
  {
    Gdk_Window window=gtkobj()->bin_window;;
    bin_pixmap_.draw_pixmap(gc_,pixmap_,
    gtkobj()->xoffset+x,gtkobj()->yoffset+y,x,y,w,h);

    window.set_back_pixmap(bin_pixmap_,false);
    window.clear_area(x,y,w,h);
    window.set_back_pixmap(null_pixmap_,false);
  }

void LayoutArea::back_alloc()
  {
    Gdk_Window window=get_window();
    bin_width=width();
    bin_height=height();
    bin_pixmap_.create(window,width()+20,height()+20);
    if (has_background_)
      window.set_background(background_color_);
    flush(0,0,width(),height());
  }


gint LayoutArea::expose_event_impl(GdkEventExpose* e)
  {
    if (width()>bin_width||height()>bin_height)
      {back_alloc();}
    else  
      {flush(e->area.x,e->area.y,e->area.width,e->area.height);}

    return Gtk::Layout::expose_event_impl(e);
  }

void LayoutArea::realize_impl()
  {
    // we need to do the default realize
    Gtk::Layout::realize_impl();

    // allocate our resources
    pixmap_.create(get_window(),gtkobj()->width,gtkobj()->height);
    if (!gc_) gc_.create(pixmap_);
    paint();
    back_alloc();
  }

/***********************************************************
***** In action 
***********************************************************/
class MyLayout : public LayoutArea
  {
   private:
      Gdk_Color    white_,black_,red_;
      Gdk_Font     font_;
   public:
      MyLayout(Gtk::Adjustment &hadj,Gtk::Adjustment &vadj)
        :LayoutArea(hadj,vadj),font_(0)
        {
          // must use gtk-- colormap
          Gdk_Colormap colormap_=get_default_colormap (); 

          white_=Gdk_Color("white");
          black_=Gdk_Color("black");
          red_=Gdk_Color("red");

          colormap_.alloc(white_);
          colormap_.alloc(black_);
          colormap_.alloc(red_);

          font_.load("fixed");
  
          // to reduce flashing we will match the background color
          // to that which we will most be using.
          set_background(black_);
        }

      void paint()
        {
          // always remember to clear the pixmap first!
          gc_.set_foreground(red_);
          pixmap_.draw_rectangle(gc_,true,0,0,width(),height());

          // put something on the pixmap
          gc_.set_foreground(white_);
          gc_.set_background(black_);
          pixmap_.draw_line(gc_,0,0,100,100);
          for(unsigned i=1; i<20; ++i) {
             pixmap_.draw_string(font_,gc_,10,10*i,
               "some text to show how it works!");
             }
        }
  };


/***************************************************************
***** Test setup
***************************************************************/

void print_hello()
  {
    cout << "hello"<<endl;
  }

void print_there()
  {
    cout << "there"<<endl;
  }

class MyWindow : public Gtk::Window 
  {
   private:

   public:
      MyWindow() 
        : Gtk::Window()
         {
          Gtk::Table *table_;
          Gtk::Adjustment *hadj_,*vadj_;
          Gtk::Scrollbar *vscroll_,*hscroll_;
          Gtk::Button *b,*c;
          LayoutArea *layout_;

          table_=manage(new Gtk::Table(2,2));

          hadj_=manage(new Gtk::Adjustment(0,0,20,8,8,5));
          vadj_=manage(new Gtk::Adjustment(0,0,20,8,8,5));
          layout_=manage(new MyLayout(*hadj_,*vadj_));

          vscroll_=manage(new Gtk::VScrollbar(*(layout_->get_vadjustment())));
          hscroll_=manage(new Gtk::HScrollbar(*(layout_->get_hadjustment())));
          b=manage(new Gtk::Button("hello"));
          c=manage(new Gtk::Button("there"));
          b->clicked.connect(slot(&print_hello));
          c->clicked.connect(slot(&print_there));
 
          set_title("Layout");
          layout_->put(*b,500,500);
          layout_->put(*c,100,100);
          layout_->set_size(600,600);

          table_->attach(*layout_,0,1,0,1,
            GTK_FILL|GTK_EXPAND,GTK_FILL|GTK_EXPAND);
          table_->attach(*hscroll_,0,1,1,2,GTK_FILL,GTK_FILL);
          table_->attach(*vscroll_,1,2,0,1,GTK_FILL,GTK_FILL);

          add(*table_);
          set_usize(100,100);
          show_all();
         }

    gint delete_event_impl(GdkEventAny*) 
      { 
        Gtk::Main::quit(); return 0; 
      }

  };

int main(int argc, char **argv)
   {
    Gtk::Main mymain(argc, argv);

    MyWindow mywindow;
    mywindow.show();
    mymain.run();
   }
