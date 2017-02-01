dnl $Id: gtkconvert.m4,v 1.4 2000/08/30 18:19:45 kenelson Exp $ 

#
#  Define a hashing for names
#
define(`__HASH',`__`'translit(`$*',`ABCDEFGHIJKLMNOPQRSTUVWXYZ<>[]&*:, ',`abcdefghijklmnopqrstuvwxyzVBNMRSLC_')`'')
define(`__HASH2',`__HASH(__EQUIV(`$1'))`'__HASH(__EQUIV(`$2'))')
define(`__EQUIV',`ifdef(EV`'__HASH(`$1'),EV`'__HASH(`$1'),`$1')')

#
#  __FWD_CONVERT(ctype,cpptype,name)
#    Print the conversion from ctype to cpptype
define(`__FWD_CONVERT',`dnl
pushdef(`__H1',`__HASH(`$1')')dnl
pushdef(`__H2',`__HASH(`$2')')dnl
pushdef(`__E1',`ifdef(EV`'__H1,EV`'__H1,__H1)')dnl
pushdef(`__E2',`ifdef(EV`'__H2,EV`'__H2,__H2)')dnl
pushdef(`__COV',`CF`'__E1`'__E2')dnl
ifelse(__E1,__E2,`$3',`dnl
ifdef(__COV,`indir(__COV,`$1',`$2',`$3')',`
errprint(`No forward conversion from $1 to $2 defined ($3) 
')')`'dnl
')`'dnl
popdef(`__COV')dnl
popdef(`__E2')dnl
popdef(`__E1')dnl
popdef(`__H2')dnl
popdef(`__H1')dnl
')


#
#  __REV_CONVERT(ctype,cpptype,name)
#    Print the conversion from cpptype to ctype
define(`__REV_CONVERT',`dnl
pushdef(`__H1',`__HASH(`$1')')dnl
pushdef(`__H2',`__HASH(`$2')')dnl
pushdef(`__E1',`ifdef(EV`'__H1,EV`'__H1,__H1)')dnl
pushdef(`__E2',`ifdef(EV`'__H2,EV`'__H2,__H2)')dnl
pushdef(`__COV',`CR`'__E1`'__E2')dnl
ifelse(__E1,__E2,`$3',`dnl
ifdef(__COV,`indir(__COV,`$1',`$2',`$3')',`
errprint(`No reverse conversion from $1 to $2 defined ($3)
')')`'dnl
')`'dnl
popdef(`__COV')dnl
popdef(`__E2')dnl
popdef(`__E1')dnl
popdef(`__H2')dnl
popdef(`__H1')dnl
')


#
#  Functions for populating the tables.
#
define(`GTKMM_CONVERSION',`
ifelse(`$3',,,`define(CF`'__HASH2(`$1',`$2'),`$3')')
ifelse(`$4',,,`define(CR`'__HASH2(`$1',`$2'),`$4')')
')

define(`GTKMM_EQUAL',`define(EV`'__HASH(`$1'),__HASH(`$2'))')

/*******************************************************/


#
#  __FOREACH(list,function)
#    Preform listed action many times.
#
define(`__LOOP',`ifelse(`$*',,,`indir(`LOOP',$*)`'__LOOP(shift($*))')')
define(`__FOREACH',`dnl
pushdef(`LOOP',`$2')dnl
__LOOP(`$1')dnl
popdef(`LOOP')dnl
')


/*******************************************************************/

#
# Table of widgets
#


GTKMM_EQUAL(gboolean,int)
GTKMM_EQUAL(gint,int)
GTKMM_EQUAL(gint*,int*)
GTKMM_EQUAL(gint&,int&)
GTKMM_EQUAL(guint,unsigned int)
GTKMM_EQUAL(guint*,unsigned int*)
GTKMM_EQUAL(guint&,unsigned int&)
GTKMM_EQUAL(gchar,char)
GTKMM_EQUAL(gchar*,char*)
GTKMM_EQUAL(const gchar*,const char*)
GTKMM_EQUAL(GdkAtom,Gdk_Atom)

GTKMM_EQUAL(`const gchar*',`const char*')
GTKMM_EQUAL(`Gtk::nstring',`nstring')
GTKMM_EQUAL(`Gtk::string',`string')
GTKMM_EQUAL(`const Gtk::nstring&',`const nstring&')
GTKMM_EQUAL(`const Gtk::string&',`const string&')


#
# Basic Types
GTKMM_CONVERSION(`int',`bool',`($1)$3',`(($3)?true:false)')
GTKMM_CONVERSION(`unsigned int',`bool',`($1)$3',`$3')
GTKMM_CONVERSION(`int*',`int&',`&$3',`*$3')
GTKMM_CONVERSION(`const char*',`string',`$3.c_str()',`$3')
GTKMM_CONVERSION(`const char*',`nstring',`$3.gc_str()',`$3')
GTKMM_CONVERSION(`const char*',`const string&',`$3.c_str()',`$3')
GTKMM_CONVERSION(`const char*',`const nstring&',`$3.gc_str()',`$3')

# These are for fixmegtkconst
GTKMM_CONVERSION(`gdouble*',`const gdouble*',`const_cast<gdouble*>($3)',`$3')
GTKMM_CONVERSION(`GtkWidget*',`const Gtk::Widget&',`const_cast<GtkWidget*>($3.gtkobj())',`Gtk::wrap($3)')
GTKMM_CONVERSION(`GtkObject*',`const Gtk::Object&',`const_cast<GtkObject*>($3.gtkobj())',`Gtk::wrap($3)')
GTKMM_CONVERSION(`guchar*',`const guchar*',`const_cast<guchar*>($3)',`$3')
GTKMM_CONVERSION(`char*',`const nstring&',`const_cast<$1>($3.gc_str())')
GTKMM_CONVERSION(`char*',`const Gtk::nstring&',`const_cast<$1>($3.gc_str())')

#
# Some reverse conversions
GTKMM_CONVERSION(`char*',`nstring',`',`$3')
GTKMM_CONVERSION(`char*',`Gtk::nstring',`',`$3')
GTKMM_CONVERSION(`char*',`string',`',`$3')
GTKMM_CONVERSION(`gchar*',`string',`',`$3')
GTKMM_CONVERSION(`const gchar*',`const string',`',`$3')
GTKMM_CONVERSION(`const gchar*',`const nstring',`',`$3')

#
# ENUMS
GTKMM_CONVERSION(`gint16',`GtkShadowType',,`($2)$3')
GTKMM_CONVERSION(`unsigned int',`GtkPolicyType',,`($2)$3')
GTKMM_CONVERSION(`unsigned int',`GtkPositionType',,`($2)$3')
GTKMM_CONVERSION(`unsigned int',`GtkCornerType',,`($2)$3')
GTKMM_CONVERSION(`GtkAttachOptions',`int',`($1)$3',`$3')

dnl GTKMM_CONVERSION(`char*',`const string&',`const_cast<$1>($3.c_str())')
dnl GTKMM_CONVERSION(`char*',`const nstring&',`const_cast<$1>($3.gc_str())')


# 
# Odd types 
GTKMM_CONVERSION(`GList*',`const G_List_string&',`$3.glist()')
GTKMM_CONVERSION(`GtkStyle*',`Gtk::Style&',`$3.gtkobj()')
GTKMM_CONVERSION(`GtkStyle*',`Gtk::Style*',`($3?$3->gtkobj():0)',`Gtk::wrap($3)')
GTKMM_CONVERSION(`gchar**',`const SArray&',`const_cast<$1>((const char**)$3)')


#
# Gdk types
#

dnl Gdk_Drag_Context,Gdk_Image,Gdk_Bitmap,Gdk_Color,Gdk_Colormap,Gdk_Pixmap,Gdk_Window,dnl
dnl Gdk_Visual,Gdk_Rectangle

GTKMM_CONVERSION(GdkWindow*,Gdk_Window,`$3.gdkobj()',`$3')
GTKMM_CONVERSION(GdkVisual*,Gdk_Visual,`$3.gdkobj()',`$3')
GTKMM_CONVERSION(GdkColormap*,Gdk_Colormap,`$3.gdkobj()',`$3')
GTKMM_CONVERSION(GdkFont*,Gdk_Font,`$3.gdkobj()',`$3')

GTKMM_CONVERSION(GdkDragContext*,Gdk_Drag_Context&,`$3.gdkobj()',`$3')
GTKMM_CONVERSION(GdkWindow*,Gdk_Window&,`$3.gdkobj()',`$3')
GTKMM_CONVERSION(GdkPixmap*,Gdk_Pixmap&,`$3.gdkobj()',`$3')
GTKMM_CONVERSION(GdkBitmap*,Gdk_Bitmap&,`$3.gdkobj()',`$3')
GTKMM_CONVERSION(GdkColormap*,Gdk_Colormap&,`$3.gdkobj()',`$3')

GTKMM_CONVERSION(GdkWindow*,const Gdk_Window&,`const_cast<$1>($3.gdkobj())')
GTKMM_CONVERSION(GdkColormap*,const Gdk_Colormap&,`const_cast<$1>($3.gdkobj())')
GTKMM_CONVERSION(GdkFont*,const Gdk_Font&,`const_cast<$1>($3.gdkobj())')
GTKMM_CONVERSION(GdkVisual*,const Gdk_Visual&,`const_cast<$1>($3.gdkobj())')
GTKMM_CONVERSION(GdkBitmap*,const Gdk_Bitmap&,`const_cast<$1>($3.gdkobj())')
GTKMM_CONVERSION(GdkRectangle*,const Gdk_Rectangle&,`const_cast<$1>($3.gdkobj())')

GTKMM_CONVERSION(GdkImage*,const Gdk_Image&,`const_cast<$1>($3.gdkobj())')
GTKMM_CONVERSION(GdkPixmap*,const Gdk_Pixmap&,`const_cast<$1>($3.gdkobj())')
GTKMM_CONVERSION(const GdkPixmap*,const Gdk_Pixmap&,`$3.gdkobj()')

GTKMM_CONVERSION(GdkColor*,const Gdk_Color&,`const_cast<$1>($3.gdkobj())')
GTKMM_CONVERSION(GdkImlibImage*,const Gdk_ImLib::Image&,`const_cast<$1>($3.gdkobj())')


#
# Widgets that need gtkobj

#
# Downcast Ptr <=> Ptr (Gtk+ casts don't handle 0 use normal cast)
#
define(`__FP2PD',`($`'1)unwrap($`'3)')
dnl define(`__RP2PD',`Gtk::wrap((`translit($'`2`,`:',`'')')($`'3))')
define(`__RP2PD',`Gtk::wrap((tran`'slit($`'2,:,))($`'3))')
GTKMM_CONVERSION(GtkWidget*,Gtk::Box*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::VBox*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::HBox*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::Button*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::CheckButton*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::ToggleButton*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::RadioButton*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::Scrollbar*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::HScrollbar*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::VScrollbar*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::Tree*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::OptionMenu*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::List*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::CList*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::ColumnList*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::FontSelection*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::Window*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::Entry*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::Menu*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::ColorSelection*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::ScrolledWindow*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::Label*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::MenuItem*,__FP2PD,__RP2PD)
GTKMM_CONVERSION(GtkWidget*,Gtk::ProgressBar*,__FP2PD,__RP2PD)

#
# Ptr => Ptr
#
define(`__FP2P',`($`'1)Gtk::unwrap($`'3)')
define(`__RP2P',`Gtk::wrap($`'3)')
GTKMM_CONVERSION(GtkWidget*,Gtk::Widget*,__FP2P,__RP2P)
GTKMM_CONVERSION(GtkLabel*,Gtk::Label*,__FP2P,__RP2P)
GTKMM_CONVERSION(GtkAdjustment*,Gtk::Adjustment*,__FP2P,__RP2P)
GTKMM_CONVERSION(GtkMenu*,Gtk::Menu*,__FP2P,__RP2P)
GTKMM_CONVERSION(GtkMenuItem*,Gtk::MenuItem*,__FP2P,__RP2P)
GTKMM_CONVERSION(GtkAccelGroup*,Gtk::AccelGroup*,__FP2P,__RP2P)

#
# Pointer => Ref
#
GTKMM_CONVERSION(GtkToolbar*,Gtk::Toolbar&,`$3.gtkobj()')
GTKMM_CONVERSION(GtkItem*,Gtk::Item&,`$3.gtkobj()')
GTKMM_CONVERSION(GtkAdjustment*,Gtk::Adjustment&,`$3.gtkobj()')
GTKMM_CONVERSION(GtkAccelGroup*,Gtk::AccelGroup&,`$3.gtkobj()')
GTKMM_CONVERSION(GtkObject*,Gtk::Object&,`$3.gtkobj()')
GTKMM_CONVERSION(GtkEditable*,Gtk::Editable&,`$3.gtkobj()')
GTKMM_CONVERSION(GtkWidget*,Gtk::Widget&,`$3.gtkobj()')
GTKMM_CONVERSION(GtkWindow*,Gtk::Window&,`$3.gtkobj()')
GTKMM_CONVERSION(GtkMenuItem*,Gtk::MenuItem&,`$3.gtkobj()')
GTKMM_CONVERSION(GtkMenuShell*,Gtk::MenuShell&,`$3.gtkobj()')
GTKMM_CONVERSION(GtkWindow*,const Gtk::Window&,`const_cast<$1>($3.gtkobj())')

GTKMM_CONVERSION(const GtkWidget*,`const Gtk::Widget&',`$3.gtkobj()')

dnl You must place a GTKMM_CHECK to use this! (see list.gen_h)
GTKMM_CONVERSION(GtkWidget*,Gtk::Widget&,,`*Gtk::wrap($3)')


#
# Pointer => Ref (Downcast)
#
define(`__FP2RD',`$`'3.$1::gtkobj()')
GTKMM_CONVERSION(GtkWidget*,Gtk::Label&,__FP2RD(Widget))
GTKMM_CONVERSION(GtkWidget*,Gtk::Menu&,__FP2RD(Widget))
GTKMM_CONVERSION(GtkWidget*,Gtk::MenuShell&,__FP2RD(Widget))
GTKMM_CONVERSION(GtkWidget*,Gtk::MenuItem&,__FP2RD(Widget))
GTKMM_CONVERSION(GtkWidget*,Gtk::TreeItem&,__FP2RD(Widget))
GTKMM_CONVERSION(GtkWidget*,Gtk::Window&,__FP2RD(Widget))
GTKMM_CONVERSION(GtkWidget*,const Gtk::Window&,__FP2RD(Widget))


/*******************************************************************/

# 
# Some test cases
#
A(__FWD_CONVERT(char*,gchar*,foo))
A(__FWD_CONVERT(const gchar*,const string&,foo))
A(__FWD_CONVERT(GdkPixmap*,Gdk_Pixmap&,foo))
A(__FWD_CONVERT(GtkWidget*,Gtk::Widget&,foo))
#A(__FWD_CONVERT(GtkWidget*,`Slot<int,int>',foo))

A(__REV_CONVERT(char*,gchar*,foo))
A(__REV_CONVERT(const gchar*,const string&,foo))
A(__REV_CONVERT(GdkPixmap*,Gdk_Pixmap&,foo))
A(__REV_CONVERT(GtkWidget*,Gtk::Widget*,foo))

A(__REV_CONVERT(GtkWidget*,Gtk::Box*,foo))
A(__FWD_CONVERT(GtkWidget*,Gtk::Box*,foo))

