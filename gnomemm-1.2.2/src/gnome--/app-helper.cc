/* 
 * Copyright 2000 Karl Nelson
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
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkcheckmenuitem.h>
#include "app-helper.h"
#include <libgnomeui/gnome-stock.h>
#include <libgnomeui/gnome-uidefs.h>
#include <sigc++/convert.h>


namespace Gnome {
namespace UI {
/*******************************************************************/
gchar* cpp_strdup(const Gtk::string& s)
  {
    gchar* c=new gchar[s.size()+1];
    memcpy(c,s.c_str(),s.size()+1);
    return c;
  }



Info_::Info_()
  : callback_(), subtree_()
  {
    ref_count_=1;
    label_=0;
    hint_=0;
  }

Info_::Info_(const Gtk::string& label, const Gtk::string& hint)
  : callback_(), subtree_()
  {
    ref_count_=1;
    label_=cpp_strdup(label);
    hint_=0;
    if (!hint.empty()) hint_=cpp_strdup(hint);
  }

Info_::~Info_()
  {
    delete label_;
    delete hint_;
  }

void Info_::ref()
  {
    ref_count_++;
  }

void Info_::unref()
  {
    if (!--ref_count_) delete this;
  }

extern "C" {
static void gnomemm_info_call(void* object,void* data)
  {
    Info_* d=(Info_*)data;
    d->callback_(Gtk::wrap((GtkWidget*)object));
  }
} // "C"

void Info_::connect(Info& info)
  {
    info.label=label_;
    info.hint=hint_;
    info.unused_data=this;
    if (callback_.connected())
      {
        info.user_data=this;
        info.moreinfo = (void*)&gnomemm_info_call;
      }
    if (info.type() == Info::SUBTREE)
      {
        info.moreinfo = (void*)subtree_.gtkobj();
      }
    // Dirty hack
    else if (info.type() == Info::HELP)
      {
        info.moreinfo = (void*) info.label;
        info.label = 0;
      }
  }

namespace {
/* This is a bit of a kludge to get around some problems with the
   UI::Info structure.  Many of the actions on UI::Info imply the
   availablity of the Widget something which as a rule does not
   happen in gtk--.  That is we wash the object because most often
   it isn't relevent.  UI::Info is different, since the user never
   constructed the object they don't know its address.  

   The solution is to allow both gtk+ type slots and gtk-- style
   slots with and without the object.  To do this it is wired such
   that Info_ only stores the with object style.  The following converter
   changes a with to a without.  

   All info methods now take both slot styles.  
*/

// Kludged internal taken from SigC.  
//  Will replace with SigC::hide in next version of SigC. 
struct GnomeHideWidget : public SigC::AdaptorSlot_
  {
   typedef SigC::Slot1<void,Gtk::Widget*> SlotType;
   typedef SigC::Slot0<void> InSlotType;
   typedef SigC::AdaptorNode Node;
   typedef SigC::CallDataObj2<SlotType::Func,Node> CallData;
 
   static void callback(void* d,Gtk::Widget* p1)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((InSlotType::Callback&)(node->data_))();
     }

   static SigC::SlotData* create(SigC::SlotData *s)
     {
      Node *node=new Node();
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

SigC::Slot1<void,Gtk::Widget*>  gnomemm_hide_widget(const SigC::Slot0<void> &s)
  {return GnomeHideWidget::create(s.obj()); }

}

void Info_::set_callback(const Info::Callback& callback)
  {
    if (callback.connected())
      callback_=gnomemm_hide_widget(callback);
  }

void Info_::set_callback(const Info::CallbackW& callback)
  {
    callback_=callback;
  }


void Info_::set_subtree(const Array<Info>& subtree)
  {
    subtree_=subtree;
  }

Array<Info>& Info_::get_subtree()
  {
    return subtree_; //Removes Begin and End.
  }


/*******************************************************************/
Info::~Info()
  {
    if (unused_data) info_()->unref();
  }

Info::Info()
  {
    GnomeUIInfo::type=GnomeUIInfoType(0);
    GnomeUIInfo::label=0;
    GnomeUIInfo::hint=0;
    GnomeUIInfo::moreinfo=0;
    GnomeUIInfo::user_data=0;
    GnomeUIInfo::unused_data=0;
    GnomeUIInfo::pixmap_type=GnomeUIPixmapType(0);
    GnomeUIInfo::pixmap_info=0;
    GnomeUIInfo::accelerator_key=0;
    GnomeUIInfo::ac_mods=GdkModifierType(0);
    GnomeUIInfo::widget=0;
  }

Info::Info(const Info& i)
  {
    memcpy(this,&i,sizeof(Info));
    if (unused_data)
      info_()->ref();
  }

Info& Info::operator =(const Info& i)
  {
    if (this==&i) return *this;
    if (unused_data) info_()->unref();
    memcpy(this,&i,sizeof(Info));
    if (unused_data) info_()->ref();
    return *this;
  }

Gtk::Widget* Info::get_widget() const
{
  return Gtk::wrap(GnomeUIInfo::widget);
}

void Info::set_icon(const Icon& icon_)
  {
    pixmap_type=icon_.pixmap_type;
    pixmap_info=icon_.pixmap_info;
  }

void Info::set_accel(Gtk::Menu_Helpers::AccelKey key)
  {
    accelerator_key=key.key();
    ac_mods=GdkModifierType(key.mod());
    if (accelerator_key==GDK_VoidSymbol) 
      {
        accelerator_key=0;
        ac_mods=GdkModifierType(0);
      }
  }

void Info::init(Type type_)
  {
    GnomeUIInfo::type=GnomeUIInfoType(type_);
  }

void Info::init_cb(Type type_,const Icon& icon_,
            const Gtk::string& label_,const Callback& callback_,
            const Gtk::string& hint_)
  {
    GnomeUIInfo::type=GnomeUIInfoType(type_);
    Info_ *info_=new Info_(label_,hint_);
    info_->set_callback(callback_);

    set_icon(icon_);
    set_accel();

    info_->connect(*this);
  }

void Info::init_cbw(Type type_,const Icon& icon_,
            const Gtk::string& label_,const CallbackW& callback_,
            const Gtk::string& hint_)
  {
    GnomeUIInfo::type=GnomeUIInfoType(type_);
    Info_ *info_=new Info_(label_,hint_);
    info_->set_callback(callback_);

    set_icon(icon_);
    set_accel();

    info_->connect(*this);
  }


void Info::init_sub(Type type_,const Icon& icon_,
            const Gtk::string& label_,const Array<Info>& array,
            const Gtk::string& hint_)
  {
    GnomeUIInfo::type=GnomeUIInfoType(type_);
    Info_ *info_=new Info_(label_,hint_);
    info_->set_subtree(array);

    set_icon(icon_);
    set_accel();

    info_->connect(*this);
  }



extern "C" {
static void
gnomemm_info_connect (GnomeUIInfo *uiinfo, gchar *signal_name,
                GnomeUIBuilderData *uibdata)
  {
    gtk_signal_connect (GTK_OBJECT (uiinfo->widget),
                        signal_name, GTK_SIGNAL_FUNC(uiinfo->moreinfo), 
                        (gpointer)(uiinfo->user_data));
  }
} // "C"


GnomeUIBuilderData Begin::build_data_ =
  {
     &gnomemm_info_connect, 0, 0, 0, 0
  };

/*******************************************************************/

Separator::operator Gtk::Menu_Helpers::Element()
  { 
    Gtk::MenuItem *item=manage(new Gtk::MenuItem());
    item->show();
    return *item;
  }

/*******************************************************************/

SubTree::SubTree()
  {
  }


SubTree::SubTree(const Gtk::string& label,const Array<Info>& uitree,
                 const Gtk::string& tip /* =Gtk::string() */)
  {
    init_sub(SUBTREE, Icon(), label, uitree, tip);
  }

SubTree::SubTree(const Icon& icon, const Gtk::string& label,
                 const Array<Info>& uitree, const Gtk::string& tip /* =Gtk::string() */)
  {
    init_sub(SUBTREE, icon, label, uitree, tip);
  }

SubTree::~SubTree()
  {
  }


Array<Info>& SubTree::get_uitree()
{
  Info_* pInfo_ = info_();
  return pInfo_->get_subtree();
}

extern "C" {

static void 
gnomemm_radio_info_call(GtkWidget* w,gpointer data)
  {
    Info_* d=(Info_*)data;

    if (GTK_IS_TOGGLE_BUTTON(w)&&GTK_TOGGLE_BUTTON(w)->active)
      {
        d->callback_(Gtk::wrap(w));
        return;
      }
    if (GTK_IS_CHECK_MENU_ITEM(w)&&GTK_CHECK_MENU_ITEM(w)->active)
      {
        d->callback_(Gtk::wrap(w));
        return;
      }
  }

static void
gnomemm_radio_info_connect (GnomeUIInfo *uiinfo, gchar *signal_name,
                GnomeUIBuilderData *uibdata)
  {
    gtk_signal_connect (GTK_OBJECT (uiinfo->widget),
                        signal_name, GTK_SIGNAL_FUNC(gnomemm_radio_info_call),
                        (gpointer)(uiinfo->user_data));
  }

} 


GnomeUIBuilderData RadioTree::build_data_ =
  {
     &gnomemm_radio_info_connect, 0, 0, 0, 0
  };

RadioTree::RadioTree(const Gnome::UI::Array<Info>& array)
  {
    GnomeUIInfo::type=GnomeUIInfoType(RADIOITEMS);
    Gnome::UI::Info_ *info_=new Gnome::UI::Info_();
    info_->set_subtree(array);
    info_->subtree_.gtkobj()->moreinfo=(void*)&build_data_;
    moreinfo = (void*)info_->subtree_.gtkobj();
  }

/*******************************************************************/

void ConfigureItem::init(const Callback &cv,GnomeUIInfoConfigurableTypes ct)
  {
    GnomeUIInfo::type=GnomeUIInfoType(GNOME_APP_UI_ITEM_CONFIGURABLE);
    Info_ *info_=new Info_();
    info_->set_callback(cv);

    set_icon(UI::Icon());
    accelerator_key=ct;

    info_->connect(*this);
  }

void ConfigureItem::init(const CallbackW &cv,GnomeUIInfoConfigurableTypes ct)
  {
    GnomeUIInfo::type=GnomeUIInfoType(GNOME_APP_UI_ITEM_CONFIGURABLE);
    Info_ *info_=new Info_();
    info_->set_callback(cv);

    set_icon(UI::Icon());
    accelerator_key=ct;

    info_->connect(*this);
  }


void ConfigureItem::init(const Callback &cv, GnomeUIInfoConfigurableTypes ct, const Gtk::string& strLabel, const Gtk::string& strHint)
  {
    GnomeUIInfo::type=GnomeUIInfoType(GNOME_APP_UI_ITEM_CONFIGURABLE);
    Info_ *info_=new Info_();
    info_->set_callback(cv);

    set_icon(UI::Icon());
    accelerator_key=ct;

    info_->label_ = cpp_strdup(strLabel); //e.g. "Create a new foo".
    info_->hint_ = cpp_strdup(strHint); //e.g. "_New foo".
 
    info_->connect(*this);
  }

void ConfigureItem::init(const CallbackW &cv, GnomeUIInfoConfigurableTypes ct, const Gtk::string& strLabel, const Gtk::string& strHint)
  {
    GnomeUIInfo::type=GnomeUIInfoType(GNOME_APP_UI_ITEM_CONFIGURABLE);
    Info_ *info_=new Info_();
    info_->set_callback(cv);

    set_icon(UI::Icon());
    accelerator_key=ct;

    info_->label_ = cpp_strdup(strLabel); //e.g. "Create a new foo".
    info_->hint_ = cpp_strdup(strHint); //e.g. "_New foo".

    info_->connect(*this);
  }


} /*namespace UI */


namespace Menus {
 
New::New(const UI::Array<Info>& tree)
  : UI::Menu("_New",tree)
  {
    set_icon(UI::Icon(GNOME_STOCK_MENU_NEW));
    accelerator_key=GNOME_KEY_NAME_NEW;
    ac_mods=GdkModifierType(GNOME_KEY_MOD_NEW);
  }

} /* namespace Menu */

namespace UI {

//Gtk::MenuShell* MenuShell_find_menu_pos(const Gtk::MenuShell& parent,
//                                               const Gtk::string &path,
//                                               gint& pos)
//{
  //gnome_app_find_menu_pos() actually wants a GtkMenuShell*, but the arg is a GtkWidget*.
//  GtkWidget* pGtkMenuShell = gnome_app_find_menu_pos(GTK_WIDGET(parent.gtkobj()), path.c_str(), &pos);
//  return Gtk::wrap(GTK_MENU_SHELL(pGtkMenuShell));
//}

namespace {
struct UIArrayHolder
  {
     Gnome::UI::Array<Info> info_;
     UIArrayHolder(const UI::Array<Info>& info): info_(info) {}
     static void destroy(void* d)
       {
         delete ((UIArrayHolder*)d);
       }
  };
}


void fill (Gtk::MenuShell& menu_shell,
           const UI::Array<Info>& info,
           Gtk::AccelGroup& accel_group,
           gboolean         uline_accels,
           gint             pos)
{
  menu_shell.set_data_full("gnomemm-uihold",new UIArrayHolder(info),UIArrayHolder::destroy);
  gnome_app_fill_menu(menu_shell.gtkobj(), info.gtkobj(), accel_group.gtkobj(), uline_accels, pos);                  
}

void fill (Gtk::Toolbar& toolbar, 
           const UI::Array<Info> &info,
           Gtk::AccelGroup& accel_group)
{
  toolbar.set_data_full("gnomemm-uihold",new UIArrayHolder(info),UIArrayHolder::destroy);
  gnome_app_fill_toolbar(toolbar.gtkobj(), info.gtkobj(), accel_group.gtkobj());
}

} /*namespace UI */

} /* namespace Gnome */

