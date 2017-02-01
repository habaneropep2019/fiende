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
 * UIInfo "huffs goat choad" but we will try to make the best of it.
 */

#ifndef GNOMEMM_APP_HELPER_H
#define GNOMEMM_APP_HELPER_H

#include <new>
#include <gtk--/menushell.h>
#include <gtk--/toolbar.h>
#include <gtk--/accelgroup.h>
#include <libgnomeui/gnome-app.h>
#include <libgnomeui/gnome-app-helper.h>
#include <vector>

namespace Gnome {
namespace UI {
class Icon;

/*******************************************************/
// Info is the base of the UI item tree it represents broad types
// of GUI items for construction later.

template<class T_Info>
class Array;

class Info : protected GnomeUIInfo
{
  /* Note when deriving this, you must not add any fields nor may you
     add any virtuals */

  friend class Info_;
  friend class Array<Info>;

  // must not be dynamic
  void* operator new(size_t s);
public:
  void* operator new(size_t s,void* v) {return ::operator new(s,v);}

  typedef SigC::Slot0<void> Callback;
  typedef SigC::Slot1<void,Gtk::Widget*> CallbackW;
  typedef Gtk::Menu_Helpers::AccelKey AccelKey;
  enum Type
    {
      END=GNOME_APP_UI_ENDOFINFO,
      ITEM=GNOME_APP_UI_ITEM,
      TOGGLEITEM=GNOME_APP_UI_TOGGLEITEM, 
      RADIOITEMS=GNOME_APP_UI_RADIOITEMS,
      SUBTREE=GNOME_APP_UI_SUBTREE,
      SEPARATOR=GNOME_APP_UI_SEPARATOR,
      HELP=GNOME_APP_UI_HELP, 
      BUILDER=GNOME_APP_UI_BUILDER_DATA,
      CONFIGURABLE=GNOME_APP_UI_ITEM_CONFIGURABLE,
      SUBTREE_STOCK=GNOME_APP_UI_SUBTREE_STOCK
    };

  Info();
  Info(const Info&);
  ~Info();
  Info& operator =(const Info&);

  Gtk::Widget* Info::get_widget() const;

  Type type() const
    { return Type(GnomeUIInfo::type); }
  void set_icon(const Icon& icon);
  void set_accel(AccelKey ak=AccelKey());

protected:
  void init(Type type_);
  void init_cb(Type type_,const Icon& icon,
            const Gtk::string& str,const Callback& cb,
            const Gtk::string& tooltip);
  void init_cbw(Type type_,const Icon& icon,
            const Gtk::string& str,const CallbackW& cb,
            const Gtk::string& tooltip);
  void init_sub(Type type_,const Icon& icon,
            const Gtk::string& str,const Array<Info>& sub,
            const Gtk::string& tooltip);
  Info_* info_()
   { return static_cast<Info_*>(GnomeUIInfo::unused_data); }
};

/*******************************************************/
// Icons are used to represent a standard Pixmap with various states

struct Icon
{
  enum Type
    {
      NONE=GNOME_APP_PIXMAP_NONE,
      STOCK=GNOME_APP_PIXMAP_STOCK,
      DATA=GNOME_APP_PIXMAP_DATA,
      FILENAME=GNOME_APP_PIXMAP_FILENAME
    };

  GnomeUIPixmapType pixmap_type;
  gconstpointer pixmap_info;  
  const gchar* stock_id_;

  explicit Icon(const gchar* stock_id)
   : pixmap_type(GNOME_APP_PIXMAP_STOCK), pixmap_info(stock_id) {}
  Icon()
   : pixmap_type(GNOME_APP_PIXMAP_NONE), pixmap_info(0) {}
  Icon(Type type,gconstpointer info)
   : pixmap_type(GnomeUIPixmapType(type)), pixmap_info(info) {}
  ~Icon() {}
};

struct IconXpm : public Icon
{
  IconXpm(const gchar**const xpm)
   : Icon(DATA, gconstpointer(xpm)) {}
  ~IconXpm() {}
};

struct IconFile : public Icon
{
  // this must be a static, not string::c_str()
  IconFile(const gchar* file)
   : Icon(FILENAME, gconstpointer(file)) {}
  ~IconFile() {}
};


// subtree or submenu
class SubTree : public Info
{
protected:
  SubTree();
public:
  SubTree(const Gtk::string& label,const Array<Info>& uitree,
          const Gtk::string& tip=Gtk::string());
  SubTree(const Icon& icon, const Gtk::string& label,
          const Array<Info>& uitree, const Gtk::string& tip=Gtk::string());
  ~SubTree();

  Array<Info>& get_uitree();
};
typedef SubTree Menu;




// begin marker for C structures (not really for user use)
class Begin : public Info
{
static GnomeUIBuilderData build_data_;
public:
  Begin() { init(BUILDER); GnomeUIInfo::moreinfo=&build_data_; }
};

// end marker for C structures (not really for user use)
class End : public Info
{
public:
  End() { init(END); }
};

namespace Array_Helpers
{

template<class T>
struct Traits
  {
    typedef typename T::const_iterator iterator;
    static iterator begin(const T& t) {return t.begin();}
    static iterator end(const T& t) {return t.end();}
  };

// You must place an End() to use this type
template<class T_Info>
struct Traits<T_Info*>
  {
    typedef const T_Info* iterator;
    static iterator begin(const T_Info* t) {return t;}
    static iterator end( T_Info* t) {return t+64;} //64?
  };

template<size_t N, class T_Info>
struct Traits<T_Info[N]> 
  {
    typedef const T_Info* iterator;
    static iterator begin(const T_Info* t) {return t;}
    static iterator end(const T_Info* t) {return &t[N];}
  };

} /* namespace Array_Helpers */


using std::vector;

// Array converter class
template<class T_Info>
class Array 
{
  //void* operator new (size_t s);  // not implemented
  Info* data_;
  Info* begin_;
  int size_;

  template <class I> void create(I b, I e);
public:
  typedef T_Info value_type;
  typedef T_Info& reference_type;
  typedef T_Info* iterator;
  typedef T_Info* const const_iterator;
  typedef T_Info* const pointer;
  typedef T_Info* const const_pointer;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  Array& operator=(const Array& a)
    { 
      if (this==&a) return *this;
      delete [] data_; data_=0; size_=0; 
      create(a.begin(),a.end());
      return *this;
    }
  Array()
    : data_(0),begin_(0),size_(0)
    { create((T_Info*)0,(T_Info*)0); }
  Array(Array& a)
    : data_(0),begin_(0),size_(0)
    { create(a.begin(),a.end()); }
  template <class T>
  Array(const T& t)
    : data_(0),begin_(0),size_(0)
    { create(Array_Helpers::Traits<T>::begin(t),
             Array_Helpers::Traits<T>::end(t)); } 
  template <class I>
  Array(I b, I e)
    : data_(0),begin_(0),size_(0)
    { create(b, e); }

  ~Array() { delete [] data_; data_=0; size_=0; }
  
  size_t size() const { return size_; }

  iterator begin() const
    { return static_cast<T_Info*>(begin_); }
  iterator end() const
    { return static_cast<T_Info*>(begin_ + size_); }
  reference_type operator[](size_t n) const
    { return static_cast<T_Info&>(begin_[n]); }

  GnomeUIInfo* gtkobj() const 
    { return static_cast<GnomeUIInfo*>(data_); }

  vector<Info> make_return_vector(); //Used by get_uitree().
  vector<SubTree> make_return_vector_subtrees(); //Used by Gnome::App.

};


template <class T_Info>
template <class I>
void
Array<T_Info>::create(I b, I e)
  {
    // NULL list
    if (b==e)
      {
        data_=new End[1];
        return;
      }

    // determine number of Items
    for (I b2=b;b2!=e;++b2,++size_)
      if (b2->type()==Info::END) break;

    // must have a builder data on head
    if (b->type()!=Info::BUILDER)
      {
        begin_=data_=new Info[size_+2]; //plus space for BEGIN and END.
        new (begin_) Begin(); //constructor without allocation.
        begin_++; //Hide Begin() element from outside, by keeping it before begin().
      }
    else
      begin_=data_=new Info[size_+1]; //plus space for END.

    // Copy data
    for (int i=0; b!=e; ++b,++i)
      new (&begin_[i]) T_Info(*b); //constructor without allocation.

    new (&begin_[size_]) End(); //constructor without allocation.

    //At this point _size excludes the Begin() and End() GNOME internals elements,
    //so users of begin() and end() will never see them.
  }


// This is a refcounted holder to deal with C interface badness
// users don't need to deal with these unless they are making new Info types.
class Info_
{
  int ref_count_;
  Info_(const Info_&); //Prevent use of copy constructor.

protected:
  virtual ~Info_();

public:
  Info::CallbackW callback_;
  Array<Info> subtree_;
  gchar* label_;
  gchar* hint_;

  void ref();
  void unref();

  virtual void connect(Info&);
  void set_callback(const Info::Callback&);
  void set_callback(const Info::CallbackW&);
  void set_subtree(const Array<Info>&);
  Array<Info>& get_subtree();

  Info_();
  Info_(const Gtk::string& label, const Gtk::string& hint);
};


//: Fill a menu with UI::Info items.
// 'wrapper' for gnome_app_fill_menu()
void fill (Gtk::MenuShell& menu_shell,
           const UI::Array<Info>& info,
           Gtk::AccelGroup& accel_group,
           gboolean       uline_accels = true,
           gint           pos = 0);


//: Fill a toolbar with UI::Info items.
// 'wrapper' for gnome_app_fill_toolbar()
void fill (Gtk::Toolbar& toolbar, 
           const UI::Array<Info> &info,
           Gtk::AccelGroup& accel_group);





/*******************************************************/
class Separator : public Info
{
public:
  Separator() 
    { init(SEPARATOR); }
  ~Separator() {}
  operator Gtk::Menu_Helpers::Element();
}; 


// represents menu items and push buttons
class Item : public Info
{
protected:
  Item() {}
public:
  Item(const Icon& icon, const Gtk::string& str,
       const Callback& cb, const Gtk::string& tip=Gtk::string())
    { init_cb(ITEM,icon,str,cb,tip); }
  Item(const Icon& icon, const Gtk::string& str,
       const CallbackW& cb=0, const Gtk::string& tip=Gtk::string())
    { init_cbw(ITEM,icon,str,cb,tip); }
  Item(const Gtk::string& str,const Callback& cb, const Gtk::string& tip=Gtk::string())
    { init_cb(ITEM,Icon(),str,cb,tip); }
  Item(const Gtk::string& str,const CallbackW& cb=0, const Gtk::string& tip=Gtk::string())
    { init_cbw(ITEM,Icon(),str,cb,tip); }
  ~Item() {}
};

// represents toggle items and toggle buttons
class ToggleItem : public Info
{
public:
  ToggleItem(const Icon &icon, const Gtk::string& str,
             const Callback& cb=0, const Gtk::string& tip=Gtk::string())
    { init_cb(TOGGLEITEM,icon,str,cb,tip); }
  ToggleItem(const Icon &icon, const Gtk::string& str,
             const CallbackW& cb=0, const Gtk::string& tip=Gtk::string())
    { init_cbw(TOGGLEITEM,icon,str,cb,tip); }
  ToggleItem(const Gtk::string& str, const Callback& cb=0,
             const Gtk::string& tip=Gtk::string())
    { init_cb(TOGGLEITEM,Icon(),str,cb,tip); }
  ~ToggleItem() {}
};

// Loads <prefix>/share/gnome/help/<app_name>/<locale>/topic.dat
// and creates menu items for each section:
// TODO(next stable): Move the implementation in to the .cc file
class Help : public Info
{
public:
  Help(const Gtk::string& app_name)
    {
      //init_cb(HELP,Icon(), 0, 0, Gtk::string());
      GnomeUIInfo::type=GnomeUIInfoType(HELP);
 
      //The .dat identifier:
      GnomeUIInfo::moreinfo = (void*)app_name.c_str(); //const char* Should probably live as long as the app.
    }

  ~Help() {}
};


// this tree can only hold Items.
class RadioTree : public Gnome::UI::Info
{
static GnomeUIBuilderData build_data_;
public:
  RadioTree(const Gnome::UI::Array<Info>& array);
  ~RadioTree() {}
};

/**************** specific items *****************/
// configurable item (base for stock)
class ConfigureItem: public Item
{
protected:
  ConfigureItem() {}
  void init(const Callback &cv, GnomeUIInfoConfigurableTypes ct);
  void init(const Callback &cv, GnomeUIInfoConfigurableTypes ct, const Gtk::string& strLabel, const Gtk::string& strHint);
  void init(const CallbackW &cv, GnomeUIInfoConfigurableTypes ct);
  void init(const CallbackW &cv, GnomeUIInfoConfigurableTypes ct, const Gtk::string& strLabel, const Gtk::string& strHint);
public:
  ~ConfigureItem() {}
};

} /* namespace UI */

// Stock MenuIrems  
//   These all operate both a menu element for Gtk-- and as UI::Info for
//   Gnome--.  The must be static or in a container and thus can not
//   be newed.
namespace MenuItems {

// Macro to create stock menu item:
#define STOCK_MENU_ITEM(X, Y) \
  struct X : public UI::ConfigureItem \
  { \
    X(const Callback& cb) \
    { \
     init(cb, Y); \
    } \
    X(const CallbackW& cb=0) \
    { \
     init(cb, Y); \
    } \
    ~X() {} \
  }

// To create stock menu items that need label and hint (e.g. 'New'):
#define STOCK_MENU_ITEM_EXTRA(X, Y) \
  struct X : public UI::ConfigureItem \
  { \
    X(const Gtk::string& strLabel, const Gtk::string& strHint, const Callback& cb) \
    { \
     init(cb, Y, strLabel, strHint); \
    } \
    X(const Gtk::string& strLabel, const Gtk::string& strHint, const CallbackW& cb=0) \
    { \
     init(cb, Y, strLabel, strHint); \
    } \
    ~X() {} \
  }
    
/* File Menu */
STOCK_MENU_ITEM_EXTRA(New,GNOME_APP_CONFIGURABLE_ITEM_NEW); //Needs label and tip

STOCK_MENU_ITEM(Open,GNOME_APP_CONFIGURABLE_ITEM_OPEN);
STOCK_MENU_ITEM(Save,GNOME_APP_CONFIGURABLE_ITEM_SAVE);
STOCK_MENU_ITEM(SaveAs,GNOME_APP_CONFIGURABLE_ITEM_SAVE_AS);
STOCK_MENU_ITEM(Revert,GNOME_APP_CONFIGURABLE_ITEM_REVERT);
STOCK_MENU_ITEM(Print,GNOME_APP_CONFIGURABLE_ITEM_PRINT);
STOCK_MENU_ITEM(PrintSetup,GNOME_APP_CONFIGURABLE_ITEM_PRINT_SETUP);
STOCK_MENU_ITEM(Close,GNOME_APP_CONFIGURABLE_ITEM_CLOSE);
STOCK_MENU_ITEM(Exit,GNOME_APP_CONFIGURABLE_ITEM_EXIT);

/* Edit menu */
STOCK_MENU_ITEM(Cut,GNOME_APP_CONFIGURABLE_ITEM_CUT);
STOCK_MENU_ITEM(Copy,GNOME_APP_CONFIGURABLE_ITEM_COPY);
STOCK_MENU_ITEM(Paste,GNOME_APP_CONFIGURABLE_ITEM_PASTE);
STOCK_MENU_ITEM(SelectAll,GNOME_APP_CONFIGURABLE_ITEM_SELECT_ALL);
STOCK_MENU_ITEM(Clear,GNOME_APP_CONFIGURABLE_ITEM_CLEAR);
STOCK_MENU_ITEM(Undo,GNOME_APP_CONFIGURABLE_ITEM_UNDO);
STOCK_MENU_ITEM(Redo,GNOME_APP_CONFIGURABLE_ITEM_REDO);
STOCK_MENU_ITEM(Find,GNOME_APP_CONFIGURABLE_ITEM_FIND);
STOCK_MENU_ITEM(FindAgain,GNOME_APP_CONFIGURABLE_ITEM_FIND_AGAIN);
STOCK_MENU_ITEM(Replace,GNOME_APP_CONFIGURABLE_ITEM_REPLACE);
STOCK_MENU_ITEM(Properties,GNOME_APP_CONFIGURABLE_ITEM_PROPERTIES);

/* Settings menu */
STOCK_MENU_ITEM(Preferences,GNOME_APP_CONFIGURABLE_ITEM_PREFERENCES);

/* Windows menu */
STOCK_MENU_ITEM(NewWindow,GNOME_APP_CONFIGURABLE_ITEM_NEW_WINDOW);
STOCK_MENU_ITEM(CloseWindow,GNOME_APP_CONFIGURABLE_ITEM_CLOSE_WINDOW);

/* Help menu */
STOCK_MENU_ITEM(About,GNOME_APP_CONFIGURABLE_ITEM_ABOUT);

/* Game menu */
STOCK_MENU_ITEM(NewGame,GNOME_APP_CONFIGURABLE_ITEM_NEW_GAME);
STOCK_MENU_ITEM(PauseGame,GNOME_APP_CONFIGURABLE_ITEM_PAUSE_GAME);
STOCK_MENU_ITEM(RestartGame,GNOME_APP_CONFIGURABLE_ITEM_RESTART_GAME);
STOCK_MENU_ITEM(UndoMove,GNOME_APP_CONFIGURABLE_ITEM_UNDO_MOVE);
STOCK_MENU_ITEM(RedoMove,GNOME_APP_CONFIGURABLE_ITEM_REDO_MOVE);
STOCK_MENU_ITEM(Hint,GNOME_APP_CONFIGURABLE_ITEM_HINT);
STOCK_MENU_ITEM(Scores,GNOME_APP_CONFIGURABLE_ITEM_SCORES);
STOCK_MENU_ITEM(EndGame,GNOME_APP_CONFIGURABLE_ITEM_END_GAME);
#undef STOCK_MENU_ITEM
} /*namespace MenuItem*/

namespace Menus {
/* Some standard menus */


// New: If you have more than one New type then use this tree: 
struct New : public UI::Menu
{
  New(const UI::Array<Info>& tree);
  ~New() {}
};

//Other menus:
#define STOCK_MENU(X,Y) \
struct X : public UI::Menu \
{ \
  X(const UI::Array<Info>& tree = UI::Array<Info>()) \
  : UI::Menu(Y,tree) \
  {} \
  ~X() \
  {} \
}

STOCK_MENU(File,"_File");
STOCK_MENU(Files,"Fi_les");
STOCK_MENU(Edit,"_Edit");
STOCK_MENU(View,"_View");
STOCK_MENU(Help,"_Help");
STOCK_MENU(Game,"_Game");
STOCK_MENU(Settings,"_Settings");
STOCK_MENU(Windows,"_Windows");
#undef STOCK_MENU

} /* namespace Menu */


//: Utility functions for Gtk::MenuShell.
//- These should really be member methods of Gtk::MenuShell,
//- but they're part of gnomeui, not gtk, and they didn't subclass GtkMenuShell.
//static Gtk::MenuShell* MenuShell_find_menu_pos(const Gtk::MenuShell& parent,
//                                               const Gtk::string &path,
//                                               gint& pos);

//IGNORED gnome_app_fill_menu_with_data()
//IGNORED gnome_app_fill_menu_custom()
//IGNORED gnome_app_fill_toolbar_with_data()
//IGNORED gnome_app_fill_toolbar_custom()


} /* namespace Gnome */
#endif
