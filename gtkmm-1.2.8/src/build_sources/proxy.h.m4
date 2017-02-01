dnl 
dnl Gtk Proxy Signal Templates
dnl 
dnl  Copyright 1999 Karl Nelson <kenelson@ece.ucdavis.edu>
dnl 
dnl  This library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Library General Public
dnl  License as published by the Free Software Foundation; either
dnl  version 2 of the License, or (at your option) any later version.
dnl 
dnl  This library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Library General Public License for more details.
dnl 
dnl  You should have received a copy of the GNU Library General Public
dnl  License along with this library; if not, write to the Free
dnl  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
dnl 
// -*- c++ -*-
dnl Ignore the next line
/* This is a generated file, do not edit.  Generated from __file__ */
include(template.macros.m4)
#ifndef __header__
#define __header__
#include <sigc++/signal_system.h>
#include <sigc++/bind.h>
#include <sigc++/convert.h>
#include <gtk--config.h>
#include <gtk/gtkobject.h>
#include <gtk/gtksignal.h>

namespace Gtk
{

using SigC::Connection;
using SigC::manage;

//using SigC::slot;
//using SigC::convert;
//using SigC::bind;
//using SigC::Slot0;
//using SigC::Slot1;
//using SigC::Slot2;
//using SigC::Slot3;
//using SigC::Slot4;
//using SigC::Slot5;
//using SigC::Slot6;
//using SigC::Callback0;
//using SigC::Callback1;
//using SigC::Callback2;
//using SigC::Callback3;
//using SigC::Callback4;
//using SigC::Callback5;
//using SigC::Callback6;

// Forward declarations
class Object;

struct ProxyNode : public SigC::SlotNode
  {
   static void connect(Object*,
                       const char*,
                       GtkSignalFunc,
                       SigC::SlotData*,
                       bool);
   static void notify(gpointer);

   GtkObject *obj_;
   SigC::SlotData* slot_;
   void* callback_;
   int connid_;

   ProxyNode() {}
   virtual ~ProxyNode();
  };


dnl
dnl GTK_PROXY_SIGNAL([P1, P2, ...])
dnl
define([GTK_PROXY_SIGNAL],
[/****************************************************************
***** Gtk Proxy Signal NUM($1)
****************************************************************/
LINE(]__line__[)dnl

template <LIST([class R],1,ARG_CLASS($1),[$1],[
          class Obj,class gObj,int index],1)>
class [ProxySignal]NUM($1)
  {
    public:
      typedef SigC::[Callback]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1])> Callback;
      typedef SigC::[Slot]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1])> SlotType;

#ifndef GTKMM_CXX_GAUB
    protected:
      Obj *obj;
#else
      Obj *obj;
    protected:
#endif

#ifdef SIGC_CXX_PARTIAL_SPEC
      typedef R RType;
#else
      typedef SigC::Trait<R>::type RType;
#endif

      static RType gtk_callback(LIST([GtkObject*],1,ARG_BOTH($1),[$1],[void *d],1))
        {
          ProxyNode *node=(ProxyNode*)d;
          Callback* data=(Callback*)(node->callback_);
          return data->call(ARG_NAME($1));
        }

    public:

      Connection connect(const SlotType &s)
        {
         SigC::SlotData* d=s.data();
         ProxyNode::connect(obj,
                            obj->signal_names[]BRACE(index),
			    (GtkSignalFunc)gtk_callback,
                            d,
			    false);
         return d;
        }

      Connection connect_after(const SlotType &s)
        {
         SigC::SlotData* d=s.data();
         ProxyNode::connect(obj,
                            obj->signal_names[]BRACE(index),
			    (GtkSignalFunc)gtk_callback,
			    d,
			    true);
         return d;
        }

  };

template <LIST([class R],1,ARG_CLASS($1),[$1],[
          class Obj,class gObj,int index],1,[
          ]PROT(R (*emit_func)(LIST(gObj*,1,ARG_TYPE($1),[$1]))),1)>
class [EmitProxySignal]NUM($1)
  : public [ProxySignal]NUM($1) 
    <LIST([R],1,ARG_TYPE($1),[$1],[Obj,gObj,index],1)>
  {
    public:
      typedef [EmitProxySignal]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1],[Obj,gObj,index,emit_func],1)> Self;
      typedef SigC::[Slot]NUM($1)<LIST(R,1,ARG_TYPE($1),[$1])> SlotType;
      typedef SigC::CallDataObj2<typename SlotType::Func,Self> Data;

    private:

#ifdef SIGC_CXX_PARTIAL_SPEC
      typedef R RType;
#else
      typedef SigC::Trait<R>::type RType;
#endif

      static RType callback(LIST(void* d,1,ARG_BOTH($1),[$1]))
        {
          Data* data=(Data*)d;
          return data->obj->emit(ARG_NAME($1));
        }

    public:
      SlotType slot()
        {
          SigC::SlotData* tmp=manage(new SigC::SlotData());
          Data& data=reinterpret_cast<Data&>(tmp->data_);
          data.callback=&callback;
          data.obj=this;
          SigC::ScopeNode* node=tmp->receiver();
          obj->register_data(node);
          return tmp;
        }

      RType emit(ARG_BOTH($1))
        {
          return reinterpret_cast<RType (*)(LIST(gObj*,1,ARG_TYPE($1),[$1]))>
            (emit_func) (LIST(obj->gtkobj(),1,ARG_NAME($1),[$1]));
        }

      RType operator()(ARG_BOTH($1))
        {
          return reinterpret_cast<RType (*)(LIST(gObj*,1,ARG_TYPE($1),[$1]))>
            (emit_func) (LIST(obj->gtkobj(),1,ARG_NAME($1),[$1]));
        }

  };

])dnl

GTK_PROXY_SIGNAL(ARGS(P,0))
GTK_PROXY_SIGNAL(ARGS(P,1))
GTK_PROXY_SIGNAL(ARGS(P,2))
GTK_PROXY_SIGNAL(ARGS(P,3))
GTK_PROXY_SIGNAL(ARGS(P,4))
GTK_PROXY_SIGNAL(ARGS(P,5))
GTK_PROXY_SIGNAL(ARGS(P,6))


}

#endif // __header__


