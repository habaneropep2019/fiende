// This is -*- C++ -*-

/* 
 * types.h
 *
 * Copyright 1998 Karl E. Nelson
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
#ifndef _GDKMM_TYPES_H_
#define _GDKMM_TYPES_H_

#include <gdk/gdk.h>
#include <gdk--config.h>

#include <string>

// we need memset, memcpy
#include <string.h>

/*********************************************************************
***** Version macros
*********************************************************************/
/* we need features to get the gdk version numbers. */
#include <gtk/gtkfeatures.h>

/* macro for controlling version numbers */
#ifndef _GDK_VERSION
// this one is old.  Will be replaced with more flexable below.
#define _GDK_VERSION(major,minor) ((GTK_MAJOR_VERSION==major)&&(GTK_MINOR_VERSION==minor))

#define GDK_VERSION_GT(major,minor) ((GTK_MAJOR_VERSION>major)||(GTK_MAJOR_VERSION==major)&&(GTK_MINOR_VERSION>minor))
#define GDK_VERSION_GE(major,minor) ((GTK_MAJOR_VERSION>major)||(GTK_MAJOR_VERSION==major)&&(GTK_MINOR_VERSION>=minor))
#define GDK_VERSION_EQ(major,minor) ((GTK_MAJOR_VERSION==major)&&(GTK_MINOR_VERSION==minor))
#define GDK_VERSION_NE(major,minor) ((GTK_MAJOR_VERSION!=major)||(GTK_MINOR_VERSION!=minor))
#define GDK_VERSION_LE(major,minor) ((GTK_MAJOR_VERSION<major)||(GTK_MAJOR_VERSION==major)&&(GTK_MINOR_VERSION<=minor))
#define GDK_VERSION_LT(major,minor) ((GTK_MAJOR_VERSION<major)||(GTK_MAJOR_VERSION==major)&&(GTK_MINOR_VERSION<minor))

#define GDK_VERSION_GT_MICRO(major,minor,micro) \
  ((GTK_MAJOR_VERSION>major)|| \
  (GTK_MAJOR_VERSION==major)&&(GTK_MINOR_VERSION>minor)||\
  (GTK_MAJOR_VERSION==major)&&(GTK_MINOR_VERSION==minor)&&(GTK_MICRO_VERSION>micro))

#endif

/*********************************************************************
***** Forward declarations
*********************************************************************/

/* Handles */
class Gdk_Drawable;
class Gdk_Bitmap;
class Gdk_Colormap;
class Gdk_Image;
class Gdk_Pixmap;
class Gdk_Window;
class Gdk_Font;
class Gdk_GC;
class Gdk_Visual; 
class Gdk_Cursor;


/* types */
typedef GdkAtom 		Gdk_Atom;
#if GDK_VERSION_GT(1,0)
typedef GdkWChar 		Gdk_WChar;
#endif


/* Stucts */
class Gdk_Color;
class Gdk_ColorContextDither;
class Gdk_ColorContext;
class Gdk_GCValues;
#if GDK_VERSION_GT(1,0)
class Gdk_Geometry;
class Gdk_ICAttr;
#endif
class Gdk_Image; 
class Gdk_Point;
class Gdk_Points;
class Gdk_Rectangle;
class Gdk_Segment;
class Gdk_Segments;
class Gdk_WindowAttr;

class Gdk_DeviceKey;
class Gdk_DeviceInfo;
class Gdk_TimeCoord;
class Gdk_Region;

// OBSOLETE - name consistancy
typedef Gdk_ColorContextDither 	Gdk_Color_Context_Dither;
typedef Gdk_ColorContext 	Gdk_Color_Context;
typedef Gdk_TimeCoord 		Gdk_Time_Coord;
typedef Gdk_WindowAttr 		Gdk_Window_Attr;
typedef Gdk_DeviceKey 		Gdk_Device_Key;
typedef Gdk_GCValues 		Gdk_GC_Values;
typedef Gdk_DeviceInfo 		Gdk_Device_Info;
#if GDK_VERSION_GT(1,0)
typedef GdkWChar 		Gdk_W_Char;
#endif


/* Events - We need a better (light) wrapper, so unwrapped for now */
//typedef GdkEventAny		Gdk_EventAny;
//typedef GdkEventExpose 	Gdk_EventExpose;
//typedef GdkEventNoExpose	Gdk_EventNoExpose;
//typedef GdkEventVisibility	Gdk_EventVisibility;
//typedef GdkEventMotion	Gdk_EventMotion;
//typedef GdkEventButton	Gdk_EventButton;
//typedef GdkEventKey		Gdk_EventKey;
//typedef GdkEventFocus		Gdk_EventFocus;
//typedef GdkEventCrossing	Gdk_EventCrossing;
//typedef GdkEventConfigure	Gdk_EventConfigure;
//typedef GdkEventProperty	Gdk_EventProperty;
//typedef GdkEventSelection	Gdk_EventSelection;
//typedef GdkEventProximity	Gdk_EventProximity;
//#if GDK_VERSION_LE(1,0)
//typedef GdkEventOther		Gdk_EventOther;
//typedef GdkEventDragBegin	Gdk_EventDragBegin;
//typedef GdkEventDragRequest	Gdk_EventDragRequest;
//typedef GdkEventDropEnter	Gdk_EventDropEnter;
//typedef GdkEventDropDataAvailable Gdk_EventDropDataAvailable;
//typedef GdkEventDropLeave	Gdk_EventDropLeave;
//#else
//typedef GdkEventDND           Gdk_EventDND;
//#endif
//typedef GdkEventClient	Gdk_EventClient;
//typedef GdkEvent 		Gdk_Event;


/* DND */
#if GDK_VERSION_GT(1,0)
/* enum */
//typedef GdkDragAction		Gdk_DragAction;
//typedef GdkDragProtocol 	Gdk_DragProtocol;
/* struct */
class Gdk_DragContext;
// Obsolete
typedef Gdk_DragContext         Gdk_Drag_Context;
#endif


/* Enums - We need a better encapsulation for Enums*/
//typedef GdkWindowType			Gdk_WindowType;
//typedef GdkWindowClass		Gdk_WindowClass;
//typedef GdkImageType			Gdk_ImageType;
//typedef GdkVisualType			Gdk_VisualType;
//typedef GdkFontType			Gdk_FontType;
//typedef GdkWindowAttributesType	Gdk_WindowAttributesType;
//typedef GdkWindowHints		Gdk_WindowHints;
//typedef GdkFunction			Gdk_Function;
//typedef GdkFill			Gdk_Fill;
//typedef GdkFillRule			Gdk_FillRule;
//typedef GdkLineStyle			Gdk_LineStyle;
//typedef GdkCapStyle			Gdk_CapStyle;
//typedef GdkJoinStyle			Gdk_JoinStyle;
//typedef GdkCursorType			Gdk_CursorType;
//typedef GdkFilterReturn		Gdk_FilterReturn;
//typedef GdkVisibilityState		Gdk_VisibilityState;
//typedef GdkEventType			Gdk_EventType;
//typedef GdkEventMask			Gdk_EventMask;
//typedef GdkNotifyType			Gdk_NotifyType;
//typedef GdkModifierType		Gdk_ModifierType;
//typedef GdkSubwindowMode		Gdk_SubwindowMode;
//typedef GdkInputCondition		Gdk_InputCondition;
//typedef GdkStatus			Gdk_Status;
//typedef GdkByteOrder			Gdk_ByteOrder;
//typedef GdkGCValuesMask		Gdk_GCValuesMask;
//typedef GdkSelection			Gdk_Selection;
//typedef GdkPropertyState		Gdk_PropertyState;
//typedef GdkPropMode			Gdk_PropMode;
//#if GDK_VERSION_LE(1,0)
//typedef GdkDndType			Gdk_DndType;
//#endif
//typedef GdkInputSource		Gdk_InputSource;
//typedef GdkInputMode			Gdk_InputMode;
//typedef GdkAxisUse			Gdk_AxisUse;
//typedef GdkTarget			Gdk_Target;
//typedef GdkSelectionType		Gdk_SelectionType;
//typedef GdkExtensionMode		Gdk_ExtensionMode;
//typedef GdkIMStyle			Gdk_IMStyle;
//typedef GdkWMDecoration		Gdk_WMDecoration;
//typedef GdkWMFunction			Gdk_WM_Function;
//typedef GdkColorContextMode		Gdk_ColorContextMode;
//typedef GdkOverlapType		Gdk_OverlapType;


/*********************************************************************
***** Implementations 
*********************************************************************/
/* Templates */

//: Object used as base to wrap all of the gdk structures
//- Gdk passes everything as pointers to structures.  To encapsulate
//- this and protect against bad pointers, we will wrap them to
//- a class with value semantics, but with a pointer access.
//- It can be directly changed back to the gdk equivent (a pointer).
//- It is considered a private template.
template <class GdkObj>
  class Gdk_Obj
    {
     public:
       typedef GdkObj BaseObjectType;
     protected:
       bool null_;
       GdkObj obj_;

       //: Initialize and zero memory
       Gdk_Obj():null_(0)
         {
         }

     public:
       Gdk_Obj(GdkObj *obj):null_(obj==NULL)
         {
          if (null_) 
            return;
          if (&obj_!=obj)
            memcpy(&obj_,obj,sizeof(GdkObj));
         }

       Gdk_Obj(const Gdk_Obj& obj):null_(0)
         {
          if (this!=&obj)
            memcpy(&obj_,&obj.obj_,sizeof(GdkObj));
         }

       ~Gdk_Obj() {};

       operator GdkObj* ()
         {
          if (null_) 
            return 0;
          return &obj_;
         }

       operator GdkObj* () const
         {
          if (null_) 
            return 0;
          return &obj_;
         }

       GdkObj* gdkobj()
         {return &obj_;}
       const GdkObj* gdkobj() const
         {return &obj_;}

       GdkObj* operator -> ()
         {return &obj_;}
       const GdkObj* operator -> () const
         {return &obj_;}
    
    }; 

template <class GdkObj>
  class Gdk_Handle
    {
     public:
       typedef GdkObj BaseObjectType;
     private:
       void *operator new(size_t);
     protected:
       GdkObj* obj_;

       //: Create an unconnect obj.
       Gdk_Handle():obj_(0) {}

       //: Initialize from gdk
       Gdk_Handle(GdkObj* obj):obj_(obj) {}

       //: Initialize from gdk--
       Gdk_Handle(const Gdk_Handle &obj):obj_(obj.obj_) {}

     public:
       void *operator new(size_t s,void* v) {return ::operator new(s,v);}

       ~Gdk_Handle() {}
       
       GdkObj* gdkobj() const
         {return obj_;}

       //: convert back to gdk object.
       operator GdkObj* () const
         {return obj_;}

       //: Is the handle connected?
       bool connected() const
         {return (obj_!=0);}

       //: Are these the same handle?
       bool operator == (const Gdk_Handle &obj) const
         {return (obj_==obj.obj_);}

       //: Are these different handles?
       bool operator != (const Gdk_Handle &obj) const
         {return (obj_!=obj.obj_);}

#if 0
       //: Copy a handle.
       virtual Gdk_Handle& operator= (const Gdk_Handle& obj)
         {if (*this!=obj)
            {
             unref();
             obj_=obj.obj_;
             ref();
            }
          return *this;
         }
  
      //: Free a handle. (this used to be free(), but changed 
      //: it because clash with malloc debuggers)
      virtual void release()
         {unref();}
#endif
    };

//: Remove a const and make gdk+.
//- C++ can't do a const_cast and use the converter to gdk
//- at the same time.  Therefore, we must show it the way.
template <class GdkObj>
  inline typename GdkObj::BaseObjectType* gdk_const_cast(const GdkObj& obj) 
    {return const_cast<typename GdkObj::BaseObjectType*>(obj.gdkobj());}

/* simple wrappers */

class Gdk_ColorContextDither	:public Gdk_Obj<GdkColorContextDither> {};
class Gdk_ColorContext		:public Gdk_Obj<GdkColorContext> {};
class Gdk_GCValues		:public Gdk_Obj<GdkGCValues> {};
#if GDK_VERSION_GT(1,0)
class Gdk_Geometry		:public Gdk_Obj<GdkGeometry> {};
class Gdk_ICAttr		:public Gdk_Obj<GdkICAttr> {};
class Gdk_DragContext		:public Gdk_Obj<GdkDragContext> 
{
public:
  Gdk_DragContext () {};
  Gdk_DragContext ( Gdk_DragContext &x ) : 
    Gdk_Obj<GdkDragContext> ( x ) {}
  Gdk_DragContext ( GdkDragContext *x ):
    Gdk_Obj<GdkDragContext> ( x ) {}
};
#endif
class Gdk_WindowAttr		:public Gdk_Obj<GdkWindowAttr> {};
class Gdk_DeviceKey		:public Gdk_Obj<GdkDeviceKey> {};
class Gdk_DeviceInfo		:public Gdk_Obj<GdkDeviceInfo> {};
class Gdk_TimeCoord		:public Gdk_Obj<GdkTimeCoord> {};
class Gdk_Region		:public Gdk_Obj<GdkRegion> {};

/* Color */
#include <gdk--/color.h>

class Gdk_Point: public Gdk_Obj<GdkPoint>
  {
   public:
       // for convenience
       operator GdkPoint () const {
	   return obj_;
       }

     Gdk_Point()
       :Gdk_Obj<GdkPoint>() {}
     Gdk_Point(GdkPoint *pt)
       :Gdk_Obj<GdkPoint>(pt) {}
     Gdk_Point(const Gdk_Point& pt)
       :Gdk_Obj<GdkPoint>(pt) {}
     Gdk_Point(gint x,gint y)
       {obj_.x=x;
        obj_.y=y;
       }
     ~Gdk_Point();
  };

class Gdk_Points 
  {
    protected:
      GdkPoint *points_;
      int       size_;
      bool      owned_;

      // block assignment
      Gdk_Points& operator= (const Gdk_Points&);

      void a_alloc(size_t size);
      template <class Iterator>
      void a_dup(Iterator b, Iterator e) ;

    public:
      operator GdkPoint *() const { return points_; }
      int size() const            { return size_; }

      Gdk_Points(GdkPoint * points, int size);
      Gdk_Points(Gdk_Point * points, int size);
      ~Gdk_Points();

#ifndef GTKMM_CXX_AMBIGUOUS_TEMPLATES
      template <class Container>
      Gdk_Points(const Container & c) 
        {
          a_dup(c.begin(), c.end());
        }
#else
      Gdk_Points(const vector<Gdk_Point> & c) { a_dup(c.begin(), c.end()); }
      Gdk_Points(const list<Gdk_Point> & c) { a_dup(c.begin(), c.end()); }
      Gdk_Points(const vector<GdkPoint> & c) { a_dup(c.begin(), c.end()); }
      Gdk_Points(const list<GdkPoint> & c) { a_dup(c.begin(), c.end()); }
#endif
      template <class Iterator>
      Gdk_Points(Iterator b,Iterator e) { a_dup(b, e); }
  };

template <class Iterator>
void Gdk_Points::a_dup(Iterator b,Iterator e)
  {
    Iterator iter;

    for(iter = b, size_ = 0; iter != e; ++iter, ++size_);

    a_alloc(size_);

    unsigned int i;
    for(iter = b, i = 0; iter != e; ++iter, ++i)
        points_[i] = *iter;
  }


class Gdk_Rectangle:public Gdk_Obj<GdkRectangle>
  {
   public:
     Gdk_Rectangle()
       :Gdk_Obj<GdkRectangle>() {}
     Gdk_Rectangle(GdkRectangle* rect)
       :Gdk_Obj<GdkRectangle>(rect) {}
     Gdk_Rectangle(const Gdk_Rectangle& rect)
       :Gdk_Obj<GdkRectangle>(rect) {}
     Gdk_Rectangle(gint x, gint y, gint w, gint h)
       :Gdk_Obj<GdkRectangle>()
       {obj_.x=x;
        obj_.y=y;
        obj_.width=w;
        obj_.height=h;
       }
     ~Gdk_Rectangle();

#if GDK_VERSION_GT(1,0)
     GdkRectangle join(Gdk_Rectangle &src2)
       {GdkRectangle rec;
        gdk_rectangle_union(&obj_,&(src2.obj_),&rec);
        return rec;
       }
#endif
  };

#if GDK_VERSION_GT(1,0)
inline GdkRectangle join(Gdk_Rectangle &src1,Gdk_Rectangle &src2)
   {return src1.join(src2);
   }
#endif

class Gdk_Segment: public Gdk_Obj<GdkSegment>
  {
   public:
     Gdk_Segment()
       :Gdk_Obj<GdkSegment>() {}
     Gdk_Segment(GdkSegment* seg)
       :Gdk_Obj<GdkSegment>(seg) {}
     Gdk_Segment(const Gdk_Segment& seg)
       :Gdk_Obj<GdkSegment>(seg) {}
     Gdk_Segment(gint x1, gint y1, gint x2, gint y2)
       :Gdk_Obj<GdkSegment>()
       {obj_.x1=x1;
        obj_.y1=y1;
        obj_.x2=x2;
        obj_.y2=y2;
       }
     ~Gdk_Segment();
  };   

#endif // _GDKMM_TYPES_H_

