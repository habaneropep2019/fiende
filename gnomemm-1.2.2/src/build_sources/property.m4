__PUSHDIV__
divert(-1)
dnl
dnl  Macros for creating Canvas Item properties
dnl

define(`CANVAS_PROPERTY',`dnl
void set_$1`'($3);
ifelse($5,,,`dnl
  $5 get_$1`'() const;
')dnl
__PUSHDIV__
GTKMM_SECTION(METHOD)
void __NAMESPACE__::__CPPNAME__::set_$1`'($3 o)
  { 
    set("$2",__REV_CONVERT($4,$3,o),0); 
  }

ifelse($5,,,`dnl
$5 __NAMESPACE__::__CPPNAME__::get_$1`'() const
  {
    GtkArg arg;
    arg.name="$2";
    gtk_object_getv(const_cast<GtkObject*>(this->Gtk::Object::gtkobj()),1,&arg);
    return __FWD_CONVERT($6,$5,arg.d.$7);
}

')dnl
__POPDIV__
')

GTKMM_CONVERSION(void*,Gdk_Pixmap,`Gdk_Pixmap((GdkPixmap*)$3)')
GTKMM_CONVERSION(void*,Gdk_Color,`Gdk_Color(*(GdkColor*)$3)')

GTKMM_CONVERSION(int,GdkCapStyle,`$2($3)')
GTKMM_CONVERSION(int,GdkJoinStyle,`$2($3)')
GTKMM_CONVERSION(int,GdkLineStyle,`$2($3)')
GTKMM_CONVERSION(int,GtkAnchorType,`$2($3)')

GTKMM_CONVERSION(char*,const string&,,`$3.c_str()')
GTKMM_CONVERSION(`double*',`double&',`&$3',`*$3')

GTKMM_CONVERSION(void*,Gdk_Pixmap,`Gdk_Pixmap((GdkPixmap*)$3)')
GTKMM_CONVERSION(void*,Gdk_Color,`Gdk_Color(*(GdkColor*)$3)')

GTKMM_CONVERSION(GnomeCanvasPoints*,const CanvasPoints&,,`$3.gtkobj()')
GTKMM_CONVERSION(GdkColor*,const Gdk_Color&,,`$3.gdkobj()')
GTKMM_CONVERSION(GdkWindow*,`const Gdk_Pixmap&',,`($1)($3.gdkobj())')

__POPDIV__
