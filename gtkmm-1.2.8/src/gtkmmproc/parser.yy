// $Id: parser.yy,v 1.22 2000/12/09 22:59:46 jaycox Exp $

%{
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <list>
#include "gtkmmproc.h"

namespace std {}
using namespace std;  

WidgetDef noWidget;
WidgetDef *currentWidget=&noWidget;
list<WidgetDef*> widgets;

OutputChannel *channel;

extern "C"
{
int yylex();
int yyparse();
}

extern FILE *yyout, *yyin;

%}

%union {
  int ival;
  string* sval;
  Argument* vval;
  ArgList* aval;
  FunctionDef* fval;
}

%token CLASS WRAPCLASSDECL WRAPMETHODDECL SIGNALDECL ENDCLASSDEF
%token CLASSSECTION DOCSECTION PRIVATESECTION IMPLSECTION
%token CONST VOLATILE STATIC
%token FIXMEGTKCONST
%token WRAPCTORCAST WRAPCTORDEFAULT WRAPDTOR WRAPMEMBERDECL
%token <sval> SYMNAME  
%token <sval> NUMBER  
%token <sval> TYPESPECIFIER 

%token DUMMY

%%

input: classwrap input
| classwrap  
;

classwrap: wrapdef 
| endclassdef 
| method_decl
| signal_decl
| member_decl
| section_decl
| ctor_decl
;

/*
 * widget wrapping declaration :
 *
 * WRAP_CLASS(GtkmmName,     // Name of wrapping Gtk-- class (without namespace)
 *            GTKName,       // Name of wrapped GTK+ type
 *            GTKCast,       // GTK+ "type cast" macro
 *            GTKCheck,      // GTK+ "type check" macro
 *            basename,      // basename of Gtk-- methods
 *            GtkmmParent,   // Name of parent Gtk-- class
 *            GTKParent,     // Name of parent GTK+ type
 *            GTKParentCast) // GTK+ "type cast" macro for parent
 * 
 * For instance :
 * 
 * WRAP_CLASS(Button,
 *            GtkButton,
 *            GTK_BUTTON,
 *            GTK_IS_BUTTON,
 *            button,
 *            Gtk::Bin,
 *            GtkBin,
 *            GTK_BIN);
 */
wrapdef: WRAPCLASSDECL '(' SYMNAME ',' SYMNAME ',' SYMNAME ',' SYMNAME ',' SYMNAME ',' SYMNAME ',' SYMNAME ')' 
{
#ifdef DEBUG
  cout << "WRAPCLASSDECL" << endl;
  cout << "1: " << *($3) <<endl;
  cout << "2: " << *($5) <<endl;
  cout << "3: " << *($7) <<endl;
  cout << "4: " << *($9) <<endl;
  cout << "5: " << *($11) <<endl;
  cout << "6: " << *($13) <<endl;
  cout << "7: " << *($15) <<endl;
#endif
  WidgetDef *w=new WidgetDef($<sval>3,$<sval>5,$<sval>7,$<sval>9,$<sval>11,$<sval>13,$<sval>15);
  currentWidget=w;
  widgets.insert(widgets.end(),w);
  w->print_decl_begin(yyout);
}
| WRAPCLASSDECL '(' SYMNAME ',' SYMNAME ',' SYMNAME ',' SYMNAME ',' SYMNAME ',' SYMNAME ',' SYMNAME ',' SYMNAME ')' 
{
#ifdef DEBUG
  cout << "WRAPCLASSDECL" << endl;
  cout << "1: " << *($3) <<endl;
  cout << "2: " << *($5) <<endl;
  cout << "3: " << *($7) <<endl;
  cout << "4: " << *($9) <<endl;
  cout << "5: " << *($11) <<endl;
  cout << "6: " << *($13) <<endl;
  cout << "7: " << *($15) <<endl;
  cout << "8: " << *($17) <<endl;
#endif
  WidgetDef *w=new WidgetDef($<sval>3,$<sval>5,$<sval>7,$<sval>9,$<sval>11,$<sval>13,$<sval>15,$<sval>17);
  currentWidget=w;
  widgets.insert(widgets.end(),w);
  w->print_decl_begin(yyout);
}
;

typespecifiers: TYPESPECIFIER
{
  $<sval>$ = $<sval>1;
}
| typespecifiers TYPESPECIFIER
{
  *($<sval>1)+=" ";
  *($<sval>1)+=*($<sval>2);
  delete ($<sval>2);
  $<sval>$ = $<sval>1;
}
;

cv_qualifier: CONST
{
  $<sval>$ = new string("const");
}
| VOLATILE
{
  $<sval>$ = new string("volatile");
}
| FIXMEGTKCONST
{
#ifdef GTKMM_FIXME_GTK_CONST
 $<sval>$ = new string("const");
#else
 $<sval>$ = new string("");
#endif
}
;

identifier: SYMNAME
{
  $<sval>$ = $<sval>1;
}
| template_id
{
  $<sval>$ = $<sval>1;
}
| typespecifiers
{
  $<sval>$ = $<sval>1;
}
;

typename: typename '*'
{
  *($<sval>1)+="*";
  $<sval>$ = $<sval>1;
}
| typename '&'
{
  *($<sval>1)+="&";
  $<sval>$ = $<sval>1;
}
| typename cv_qualifier
{
  if (($<sval>2)->length ()) {
    *($<sval>1)+=" ";
  }
  *($<sval>1)+=*($<sval>2);
  delete ($<sval>2);
  $<sval>$ = $<sval>1;
}
| cv_qualifier identifier
{
  if (($<sval>1)->length ()) {
    *($<sval>1)+=" ";
  }
  *($<sval>1)+=*($<sval>2);
  delete ($<sval>2);
  $<sval>$ = $<sval>1;
}
| identifier
{
  $<sval>$ = $<sval>1;
}
;


/*
 * Templates
 */
template_id: SYMNAME '<' template_argument_list '>'
{
  *($<sval>1)+="<";
  *($<sval>1)+=*($<sval>3);
  *($<sval>1)+=">";
  delete ($<sval>3);
  $<sval>$ = $<sval>1;
}
| SYMNAME '<' '>'
{
  *($<sval>1)+="<>";
  $<sval>$ = $<sval>1;
}
;
 
template_argument_list: template_argument_list ',' typename
{
  *($<sval>1)+=",";
  *($<sval>1)+=*($<sval>3);
  delete ($<sval>3);
  $<sval>$ = $<sval>1;
}
| typename
{
  $<sval>$ = $<sval>1;
}
;

value_sym: SYMNAME
{ $<sval>$=$<sval>1; }
| NUMBER
{ $<sval>$=$<sval>1; }
| '|'
{ $<sval>$=new string("|"); }
;

/*
 * A initial value for a argument
 */
value_list: value_list value_sym
{ 
  *($<sval>1)+=*($<sval>2);
  delete $<sval>2;
  $<sval>$=$<sval>1;
}
| value_sym
{
  $<sval>$=$<sval>1;
}
| '(' value_list ')'
{
  *($<sval>2)="("+*($<sval>2)+")";
  $<sval>$=$<sval>2;
}
;

cpp_argument: typename
{
  $<vval>$ = new Argument($<sval>1);
}
| typename SYMNAME
{
  $<vval>$ = new Argument($<sval>1,$<sval>2);
}
| typename SYMNAME '=' value_list
{
  $<vval>$ = new Argument($<sval>1,$<sval>2,$<sval>4);
//  cout << "value = " << *($<sval>4) <<endl;
}
;

/*
 * A list of coma-seperated C++ args 
 *
 * int, const char*, char&, unsigned long
 */
cpp_arglist: cpp_arglist ',' cpp_argument
{
  ($<aval>1)->push_arg($<vval>3);
  $<aval>$ = $<aval>1;
}
| cpp_argument
{
  ArgList* l = new ArgList();
  l->push_arg($<vval>1);
  $<aval>$ = l;
}
;

cpp_func_id: typename SYMNAME '(' ')'
{
  $<fval>$ = new FunctionDef($<sval>1,$<sval>2);
}
| typename SYMNAME '(' cpp_arglist ')'
{
  $<fval>$ = new FunctionDef($<sval>1,$<sval>2,$<aval>4);
}
| typename template_id  '(' cpp_arglist ')'
{
  $<fval>$ = new FunctionDef($<sval>1,$<sval>2,$<aval>4);
}
;

cpp_func: cpp_func_id
| cpp_func_id CONST
{
  ($<fval>1)->mark_const();
  $<fval>$ = $<fval>1;
}
;

method_decl: WRAPMETHODDECL '(' cpp_func ',' cpp_func ')'
{
#ifdef DEBUG
  cout<< "WRAP_METHOD 1:" 
      << ($<fval>3)->get_function() << "  2:"
      << ($<fval>5)->get_function() <<endl;
#endif
  MethodDef *method=new MethodDef($<fval>3,$<fval>5);
  currentWidget->push_method(method);
  method->print_decl(yyout);
}
| WRAPMETHODDECL '(' cpp_func ',' STATIC cpp_func ')'
{
#ifdef DEBUG
  cout<< "WRAP_STATIC_METHOD 1:" 
      << ($<fval>3)->get_function() << "  2:"
      << ($<fval>5)->get_function() <<endl;
#endif
  MethodDef *method=new StaticMethodDef($<fval>3,$<fval>6);
  currentWidget->push_method(method);
  method->print_decl(yyout);
}
;

member_decl: WRAPMEMBERDECL '(' member_attrib_list ',' SYMNAME ',' SYMNAME ',' typename ',' typename  ')'
{
#ifdef DEBUG
  cout << "WRAP_MEMBER" 
       << "  Attrib:  "  << *($<ival>3) 
       << "  CppName: "  << *($<sval>5) 
       << "  CName:   "  << *($<sval>7) 
       << "  CppType: "  << *($<sval>9) 
       << "  CType:   "  << *($<sval>11) 
       << endl ;
#endif
  MemberDef *member=new MemberDef($<ival>3,$<sval>5,$<sval>7,$<sval>9,$<sval>11);
  currentWidget->push_member(member);
  member->print_decl(yyout);
}
;


endclassdef: ENDCLASSDEF
{
#ifdef DEBUG
  cout << "ENDCLASS"<<endl;
#endif
  currentWidget->print_decl_end(yyout);
  currentWidget=&noWidget;
  fputc('}',yyout);
}
;

sig_attrib_list: sig_attrib_list '|' SYMNAME
{
  $<ival>$=($<ival>1) | sig_attribute($<sval>3);
  delete $<sval>3;
}
| SYMNAME
{
  $<ival>$= sig_attribute($<sval>1);
  delete $<sval>1;
}
;

member_attrib_list: member_attrib_list '|' SYMNAME
{
  $<ival>$=($<ival>1) | member_attribute($<sval>3);
  delete $<sval>3;
}
| SYMNAME
{
  $<ival>$= member_attribute($<sval>1);
  delete $<sval>1;
}
;


/*
 * Signal Declaration :
 *
 * SIGNAL_SPEC("signame", both, void signame(int, char) );
 * SIGNAL_SPEC("signame", translate,
 *             void signame(GtkWidget*),
 *             void SigName(Gtk_Widget*));
 *
 *   SIGNAL_SPEC("focus",                        //1
 *                both,                          //2
 *                gint focus(GtkDirectionType),  //3
 *                gtk_container_focus);          //4
 * 
 * 1. name of signal for gtkmm class
 * 
 * 2. Set of attributes associated with the signal.
 *      vfunc - Declare a function for emiting this signal
 *      sig   - Declare a connectable signal
 *      impl  - Declare a _impl method for the user to override
 *      both  - Both a signal and impl
 *      emit  - Modifies sig to be emitable (implied if arg 4!="")
 *      noemit - Modified sig to be not emitable
 *      marsh - Replace the gtk+ marshaller if rettype != void
 *
 * 3. function signature calling this signal, connecting
 *    the signal and _impl methods.
 * 
 * 4. (optional) gtk+ class function 
 *
 *    - If not present, signal will not be user callable.
 *    - If not present and signal is type emit, gtk-- will write
 *      is own gtkmm_foo_method() function for you.
 *    - If present and noemit not specified the signal will 
 *      be emittable and therefore callable by anyone.
 *    - If the entire function is specified the signal will have
 *      a full translation activated which will allow the gtk--
 *      and gtk+ types to vary.
 *
 *                                           
 */
signal_decl: SIGNALDECL '(' '"' SYMNAME '"' ',' sig_attrib_list ',' cpp_func ')' 
{
#ifdef DEBUG
  cout << " SIGNALDECL (\""<< *($<sval>4) <<"\", "<< ($<ival>7) <<","<< ($<fval>9)->get_function() << ")" <<endl;
#endif
  SignalDef *sigDef = new SignalDef($<ival>7,$4,$<fval>9);
  currentWidget->push_signal(sigDef);
  sigDef->print_decl(yyout);
}
| SIGNALDECL '(' '"' SYMNAME '"' ',' sig_attrib_list ',' cpp_func ',' SYMNAME ')' 
{
  $<ival>7|=ATTR_EMIT;
#ifdef DEBUG
  cout << " SIGNALDECL (\""<< *($<sval>4) <<"\", "<< ($<ival>7) <<","<< ($<fval>9)->get_function() << "=>" << *($<sval>11) << ")" <<endl;
#endif
  SignalDef *sigDef = new SignalDef($<ival>7,$4,$<fval>9,$<sval>11);
  currentWidget->push_signal(sigDef);
  sigDef->print_decl(yyout);
}
| SIGNALDECL '(' '"' SYMNAME '"' ',' sig_attrib_list ',' cpp_func ',' cpp_func ')' 
{
  $<ival>7|=ATTR_TRANSLATE|ATTR_EMIT;
#ifdef DEBUG
  cout << " SIGNALDECL (\""<< *($<sval>4) <<"\", "<< ($<ival>7) <<","<< ($<fval>9)->get_function() << "=>" << ($<fval>11)->get_function() << ")" <<endl;
#endif
  SignalDef *sigDef = new SignalDef($<ival>7,$4,$<fval>9,$<fval>11);
  currentWidget->push_signal(sigDef);
  sigDef->print_decl(yyout);
};


section_decl: CLASSSECTION '(' SYMNAME ')' 
{
  channel->section_class(*($<sval>3)); 
  delete $<sval>3;
}
| CLASSSECTION
{
  channel->section_class();
}
| IMPLSECTION
{
  channel->section_impl(); 
}
| DOCSECTION
{
  channel->section_doc(); 
}
| PRIVATESECTION
{
  channel->section_private(); 
}
;

ctor_decl: WRAPCTORCAST
{
  currentWidget->set_cast_ctor(yyout);
}
| WRAPCTORDEFAULT
{
  currentWidget->set_default_ctor(yyout);
}
| WRAPDTOR
{
  currentWidget->set_dtor(yyout);
}
;


%%

int sig_attribute(string *s)
{
  if (!s) 
    yyerror("NULL in attribute");

  if ((*s)=="vfunc")     return ATTR_VFUNC;
  if ((*s)=="sig")       return ATTR_SIG;
  if ((*s)=="impl")      return ATTR_IMPL;
  if ((*s)=="emit")      return ATTR_EMIT;
  if ((*s)=="noemit")    return ATTR_NOEMIT;
  if ((*s)=="translate") return ATTR_TRANSLATE;
  if ((*s)=="both")      return ATTR_BOTH;
  if ((*s)=="marsh")     return ATTR_MARSH;
  if ((*s)=="fake")      return ATTR_GTK_EMIT;

  cerr << "Warning "<<yylineno<<": Unknown attribute. ("<<*s<<")"<<endl;
  return 0;
}

int member_attribute(string *s)
{
  if (!s)
    yyerror("NULL in attribute");
  if ((*s)=="inline")    return ATTR_INLINE;
  if ((*s)=="value")     return ATTR_VALUE;
  if ((*s)=="ref")       return ATTR_REF;

  cerr << "Warning "<<yylineno<<": Unknown attribute. ("<<*s<<")"<<endl;
  return 0;
}


int main(int argc, char *argv[])
{
  int rc;
  env.init(argc,argv);

  yyin=fopen(env.input_name().c_str(),"r");
  if (!yyin) 
    {
     cerr << "Failed to open file: "<< env.input_name().c_str() <<endl;
     exit (-1);
    }

//  headerfile =new HeaderChannel(env.header_name());
//  privatefile=new PHeaderChannel(env.private_name());
//  implfile   =new SourceChannel(env.impl_name());
  channel=new OutputChannel();
  yyout=channel->out();
  rc=yyparse();
  channel->process();
  return rc;
}

