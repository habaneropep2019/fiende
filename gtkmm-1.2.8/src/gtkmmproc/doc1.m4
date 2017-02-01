dnl $Id: doc1.m4,v 1.11 2000/06/28 21:31:41 kenelson Exp $

divert(-1)
dnl ---------------- Little useful macro -----------------------
dnl
dnl
dnl Add a coma before the arg if any, do nothing otherwise
dnl _COMA_(a) -> ,a
dnl _COMA_() -> `'
dnl
define(`_COMA_', `ifelse(eval(len(`$*') >= 1), 1, `,$*')')dnl

dnl
dnl  _UPPER_(string)
dnl    uppercase a string
define(`_UPPER_',`translit(`$*',`abcdefghijklmnopqrstuvwxyz',`ABCDEFGHIJKLMNOPQRSTUVWXYZ')')

dnl
dnl  _LOWER_(string)
dnl    lower a string
define(`_LOWER_',`translit(`$*',`ABCDEFGHIJKLMNOPQRSTUVWXYZ',`abcdefghijklmnopqrstuvwxyz')')

dnl 
dnl  _QUOTE_(macro)  
dnl    If a macro generates an output with commas we need to protect it
dnl    from being broken down and interpreted
define(`_QUOTE_',``$*'')

dnl
dnl  _NUM_(arglist)   
dnl    count number of arguments
define(`_NUM_',`ifelse(len(`$*'),0,0,`$#')')

dnl
dnl  _IF_(cond,string1,string2)
dnl    places string1 if cond is empty 
define(`_IF_',`ifelse(len(_QUOTE_(translit(`$1',` '))),0,`$3',`$2')')

dnl
dnl  _ARG_LIST_(`args') 
dnl    takes `int,char' => p0,p1
define(`_C_ARG_LIST_',`dnl
define(`XX',eval(XX+1))dnl
ifelse(`$*',,,`,$1 p`'XX`'_C_ARG_LIST_(shift($*))')dnl
')

define(`ARG_LIST',`dnl
define(`XX',1)dnl
ifelse(`$*',,,`$1 p`'XX`'_C_ARG_LIST_(shift($*))')dnl
')


dnl
dnl  _ARG_NAMES_(`args') 
dnl    takes `int,char' => int p0,char p1
define(`_C_ARG_NAMES_',`dnl
define(`XX',eval(XX+1))dnl
ifelse(`$*',,,`,p`'XX`'C_ARG_NAMES(shift($*))')dnl
')

define(`_ARG_NAMES_',`dnl
define(`XX',1)dnl
ifelse(`$*',,,`p`'XX`'C_ARG_NAMES(shift($*))')dnl
')

#
# For handling of included macro files.
#
define(`__PUSHDIV__',`pushdef(`__DIV__',divnum)divert(-1)dnl`'')
define(`__POPDIV__',`divert(__DIV__)popdef(`__DIV__')dnl`'')
define(`__BEGIN__',`__PUSHDIV__(-1)')
define(`__END__',`__POPDIV__()')

changequote([,])
define([__BQ__],[changequote(,)`changequote(`,')])
changequote(`,')

changecom

dnl
dnl ----------------------- Main Headers -------------------------
dnl


define(`__SEC_PROXY',1)
define(`__SEC_SIMPL',2)
define(`__SEC_CINIT',3)
define(`__SEC_PINIT',4)
define(`__SEC_SNAME',5)
define(`__SEC_HEADER',6)
define(`__SEC_CLASS',7)
define(`__SEC_PHEADER',8)
define(`__SEC_PRIVATE',9)
define(`__SEC_IMPL',10)
define(`__SEC_DOC',12)
define(`__SEC_SRC',13)
define(`__SEC_METHOD',14)
define(`__SEC_SIGNAL',15)
define(`__SEC_PHEADER',8)
define(`__SEC_PRIVATE',9)
define(`__SEC_IMPL',10)
define(`__SEC_DOC',12)
define(`__SEC_SRC',13)
define(`__SEC_METHOD',14)
define(`__SEC_SIGNAL',15)
define(`__SEC_MEMBER',16)
define(`__SEC_CHECK',17)
define(`__SEC_TYPES',18)
define(`__SEC_USR',20)

define(`GTKMM_SECTION',`divert(__SEC_$1)dnl')
define(`GTKMM_IMPORT',`undivert(__SEC_$1)dnl')   

define(`GTKMM_NAMESPACE',`dnl
define(`__NAMESPACE__',$1)dnl
')

define(`GLINE')        


dnl
dnl GTKMM_START(CONSTANT, original_filename)
dnl GTKMM_START(GTKMM_BUTTON, button.gen_h)
define(`GTKMM_START')

dnl This does all the work of assembling the final output
dnl
dnl GTKMM_END()
dnl
define(`GTKMM_END',`dnl
divert(0)
#S 0
GTKMM_IMPORT(HEADER)
namespace __NAMESPACE__ {
GTKMM_IMPORT(CLASS)
GTKMM_IMPORT(DOC)
}
divert(-1)
GTKMM_IMPORT(PHEADER)
GTKMM_IMPORT(PRIVATE)
GTKMM_IMPORT(IMPL)
GTKMM_IMPORT(SRC)
GTKMM_IMPORT(METHOD)
GTKMM_IMPORT(SIGNAL)
GTKMM_IMPORT(MEMBER)
')

dnl
dnl GTKMM_CLASS_START(Gtk--Type, Gtk--ParentType, GtkType, GtkParentType, 
dnl                 GTK_CAST, type)
dnl e.g. :
dnl GTKMM_CLASS_START(Gtk::Button, Gtk::Bin, GtkButton, GtkBin, 
dnl                 GTK_BUTTON, button)
dnl

define(`GTKMM_CLASS_START', `
dnl
dnl  Define the args for later macros
define(`__CPPNAME__',`$1')dnl
define(`__CNAME__',`$2')dnl
define(`__CCAST__',`$3')dnl
define(`__CCHECK__',`$4')dnl
define(`__BASE__',`$5')dnl
define(`__CPPPARENT__',`$6')dnl
define(`__CPARENT__',`$7')dnl
define(`__PCAST__',`$8')dnl
dnl
dnl
GTKMM_SECTION(CLASS)
GLINE()
public:
  typedef __CPPNAME__          CppObjectType;
  typedef __CNAME__            BaseObjectType;

public:
  //: Returns the underlaying gtk+ object.
  __CNAME__* gtkobj();
  const __CNAME__* gtkobj() const;

  //: Returns true if object is this type. 
  static bool isA(Gtk::Object *object);

  virtual ~__CPPNAME__`'();

private:
')dnl

define(`GTKMM_CONVERSION',`')
define(`GTKMM_CHECK',`')

dnl
dnl GTKMM_CLASS_END
dnl
define(`GTKMM_CLASS_END',`
dnl
dnl
GTKMM_SECTION(CLASS)
GLINE()

protected:
  // impl functions
GTKMM_IMPORT(SIMPL)
')


dnl
dnl --------------------------- Signal Decl----------------------------
dnl

define(`GTKMM_PROXY_NAME')
define(`GTKMM_PROXY_SIGNAL')
define(`GTKMM_MARSHAL')

dnl Proxy2<void,   int,int,     Gtk_Foo, GtkFoo, 1, gtk_foo_sig1> sig1;
dnl      ^            ^          ^        ^     ^       ^
dnl      |            |        Gtk--     gtk+   name   gtk+
dnl    Rettype    Arguments    type      type  index  function
dnl
dnl GTKMM_PROXY_SIGDEF(return_type,func_name,`<args>',index,gtksignal)
dnl
define(`GTKMM_PROXY_SIGNAL_EMIT',`dnl
    emitable signal $1 $2($3);
')

dnl
dnl Same as above, without emit
dnl
define(`GTKMM_PROXY_SIGNAL_NOEMIT',`dnl
    signal $1 $2($3);
')



dnl
dnl Same as above, with translation
dnl
define(`GTKMM_PROXY_SIGNAL_TRANSLATE_EMIT_DECL',`dnl
    emitable signal $3 $1($5);
')

define(`GTKMM_PROXY_SIGNAL_TRANSLATE_EMIT_IMPL',`dnl
')


dnl
dnl Same as above, with translation no emit
dnl
define(`GTKMM_PROXY_SIGNAL_TRANSLATE_NOEMIT_DECL',`
    signal $3 $1($5);
')


define(`GTKMM_PROXY_SIGNAL_TRANSLATE_IMPL',`dnl
')


dnl
dnl GTKMM_PROXY_CALLBACK(cppname,gtkname,cpprettype,crettype,
dnl                  `<cargs and names>', `<cpparg names>',firstarg)
dnl   
define(`GTKMM_PROXY_CALLBACK')


dnl
dnl GTKMM_PROXY_IMPL(signame,gtkname,rettype,crettype, 
dnl                           `<cppargs>', `<carg names>')
dnl   
define(`GTKMM_PROXY_IMPL',`dnl
GTKMM_SECTION(SIMPL)
    virtual $3 $1_impl($5);
GTKMM_SECTION(CLASS)
')


dnl
dnl
dnl
define(`GTKMM_GTK_EMIT')

#
# --------------------------- Methods ----------------------------
#

###
### method 
###               $1      $2    $3         $4       $5       $6      $7    $8
###  GTKMM_METHOD(cppname,cname,cpprettype,crettype,varglist,arglist,cargs,const)
define(`GTKMM_METHOD',`dnl
  $3 $1($5)ifelse($8,1,` const');
')

###
### static method
###                         $1       $2     $3         $4      $5     $6   
###  GTKMM_STATIC_METHOD(cppname,cname,cpprettype,crettype,arglist,cargs)
define(`GTKMM_STATIC_METHOD',`dnl
  static $3 $1($5);
')

#
# --------------------------- Accessors ----------------------------
#

define(`GTKMM_MEMBER_VALUE',`dnl
  $3 get_$1() const;
')

define(`GTKMM_MEMBER_REF',`dnl
GLINE()
  $3 get_$1();
  const $3 get_$1() const;
')

define(`GTKMM_MEMBER_VALUE_INLINE',`dnl
GLINE()
  $3 get_$1() const;
')

define(`GTKMM_MEMBER_REF_INLINE',`dnl
GLINE()
  $3 get_$1();
  const $3 get_$1() const;
')


###
### ----------------------- Constructors -------------------------
###

define(`GTKMM_CTOR_CAST')
define(`GTKMM_DTOR')
define(`GTKMM_CTOR_DEFAULT',`dnl
  __CPPNAME__`'();
')

define(`GTK_METHOD_PASS',`$2')

divert(0)dnl
