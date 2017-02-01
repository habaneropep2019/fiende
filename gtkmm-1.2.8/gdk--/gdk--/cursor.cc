#include "cursor.h"
#include "pixmap.h"
#include "bitmap.h"

Gdk_Cursor::Gdk_Cursor():Gdk_Handle<GdkCursor>(0) 
  {}

Gdk_Cursor::Gdk_Cursor(GdkCursor *cursor):Gdk_Handle<GdkCursor>(cursor)
  {
   ref();
  } 

void Gdk_Cursor::release()
  {
    unref();
    obj_=0;
  }

Gdk_Cursor& Gdk_Cursor::operator = (const Gdk_Cursor& h)
  {
    if (h.obj_==obj_) return *this;
    unref();
    obj_=h.obj_;
    ref();
    return *this;
  }
  
Gdk_Cursor::Gdk_Cursor(const Gdk_Cursor& font):Gdk_Handle<GdkCursor>(font)
  {
   ref();
  }

Gdk_Cursor::Gdk_Cursor(Type type):Gdk_Handle<GdkCursor>(0)
  {
   create(type);
  }

Gdk_Cursor::~Gdk_Cursor()
  {
    unref();
  }

Gdk_Cursor::Gdk_Cursor (const Gdk_Pixmap &source,
                 const Gdk_Pixmap &mask,
                 const Gdk_Color  &fg,
                 const Gdk_Color  &bg,
                 gint             x,
                 gint             y
                )
    : Gdk_Handle<GdkCursor>(0)
  {
    create(source,mask,fg,bg,x,y);
  }

void Gdk_Cursor::create(Type cursor_type)
  {
    unref();
    obj_=gdk_cursor_new(cursor_type);
    ref();
  }

void Gdk_Cursor::create(const Gdk_Pixmap &source,
                 const Gdk_Pixmap &mask,
                 const Gdk_Color  &fg,
                 const Gdk_Color  &bg,
                 gint             x,
                 gint             y)
  {
    unref();
    obj_=gdk_cursor_new_from_pixmap(gdk_const_cast(source),
                                    gdk_const_cast(mask),
                                    gdk_const_cast(fg),
                                    gdk_const_cast(bg),
                                    x,
                                    y);
    ref();
  }

void Gdk_Cursor::destroy()
  {
    gdk_cursor_destroy(*this);
  }

