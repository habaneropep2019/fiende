// $Id: gtkmmproc.h,v 1.31 2001/01/11 19:58:08 kenelson Exp $

/////////////////////////////////////////////////////////////////////////////
// List of arguments
/////////////////////////////////////////////////////////////////////////////

#include <string>
#include <list>
#include <stdio.h>

namespace std {}
using namespace std;

extern "C" void yyerror(char*);
extern "C" void yywarning(char*);
extern int yylineno;

extern void fixate(string inname, 
                   string outname, 
                   int desired_section, 
                   bool debug);

//#define DEBUG


// Signal attributes
extern int sig_attribute(string *);
#define ATTR_VFUNC     (1<<0)
#define ATTR_SIG       (1<<1)
#define ATTR_IMPL      (1<<2)
#define ATTR_GTK_EMIT  (1<<4)
#define ATTR_EMIT      (1<<5)
#define ATTR_NOEMIT    (1<<6)
#define ATTR_TRANSLATE (1<<7)
#define ATTR_MARSH     (1<<8)
#define ATTR_BOTH      (ATTR_SIG|ATTR_IMPL)

// Access attributes
extern int member_attribute(string *);
#define ATTR_INLINE (1<<16)
#define ATTR_VALUE  (1<<17)
#define ATTR_REF    (1<<18)

class Argument
{
  string *type_;
  string *name_;
  string *value_;

public:
  Argument(string *type,string *name=0,string *value=0);
  Argument(const Argument&);
  ~Argument();

  string get_type() const;
  string get_name() const;
  string get_both() const;
  string get_vboth() const;

  // if argument doesn't already have a name give it p#
  void assign_name(int num);
};

class ArgList
{
public:
  typedef list<Argument*> List;

private:
  friend string cpp2c_convert(ArgList&,ArgList&,int c=0,int s=0);
  friend string c2cpp_convert(ArgList&,ArgList&,int c=0,int s=0);

  int num_;
  List args_;
public:

  ArgList();
  ArgList(const ArgList& a);

  // return a coma seperated list of the args, for instance :
  // const int, char, long*
  string get_types() const ;
  string get_ctypes() const ;

  // same, but with synthesized names :
  // const int p1, char p2, long* p3
  string get_both() const;
  string get_cboth() const;
  string get_vboth() const;

  // return the synthesized names only :
  // p1, p2, p3
  string get_names() const;
  string get_cnames() const;

  string get_first_name() const {return (*(args_.begin()))->get_name(); }

  void push_arg(Argument *arg);
  void reset() ;

  bool empty() const { return args_.empty(); }
  int size() const   { return num_; }

  void add_first_arg();
  void erase_first_arg();

  ~ArgList();
};

/////////////////////////////////////////////////////////////////////////////
// C Function definition (also used for signals)
/////////////////////////////////////////////////////////////////////////////

struct FunctionDef
{
  string *name_;
  string *rettype_;
  ArgList *args_;
  bool isconst_;


  FunctionDef(string *rettype,
	      string *name,
	      ArgList* args=0,
	      bool isconst = false);

  FunctionDef(const FunctionDef& f);

  const string& get_name() const                { return *name_; }
  const string& get_return_type() const         { return *rettype_; }

  // returns the string equivenlent of this function
  string get_function();

  bool is_const() const            { return isconst_; }
  bool returns_void() const        { return *rettype_ == "void"; }
  bool has_args() const            { return args_->empty();}
  unsigned int get_nb_args() const { return args_->size(); }

  ArgList& get_args()              { return *args_; } 
  const ArgList& get_args() const  { return *args_; }
   
  void mark_const()                { isconst_=true; }

};

/////////////////////////////////////////////////////////////////////////////
// Method definition
/////////////////////////////////////////////////////////////////////////////

class MethodDef
{
protected:
  FunctionDef *cFunc_;
  FunctionDef *cppFunc_;
public:
  MethodDef(FunctionDef *cFunc,FunctionDef *cppFunc);
  virtual ~MethodDef();

  virtual void print_decl(FILE *out);
};

class StaticMethodDef : public MethodDef
{
public:
  StaticMethodDef(FunctionDef *cFunc,FunctionDef *cppFunc);
  virtual ~StaticMethodDef();

  virtual void print_decl(FILE *out);
};

/////////////////////////////////////////////////////////////////////////////
// Signal definition
/////////////////////////////////////////////////////////////////////////////
class SignalDef: public MethodDef
{
public:
  SignalDef(int attr,string *name,
	    FunctionDef *funcdef,string *cfunc=0);
  SignalDef(int attr,string *name,
	    FunctionDef *funcdef,FunctionDef *cfuncdef);

  virtual ~SignalDef();

  virtual void print_decl(FILE *out);         // immediate decl 
  void set_id(int i) {id_=i;}

protected:
  string *name_;           // signal name (in gtk+ signal tables)
  int attr_;
  int id_;
};

/////////////////////////////////////////////////////////////////////////////
// Get member 
/////////////////////////////////////////////////////////////////////////////
struct MemberDef
{ 
  int     attrib_;
  string *cppfunc_;
  string *membername_;
  string *cpptype_;
  string *ctype_;
  MemberDef(int attrib,
    string *cppfunc,string *membername,string *cpptype,string *ctype);
  virtual ~MemberDef();

  virtual void print_decl(FILE *out);
};

///////////////////////////////////////////////////////////////////////

class WidgetDef
{
private:
  WidgetDef(const WidgetDef&);
protected:
  string *cppName_;
  string *cName_;
  string *cCast_;
  string *cCheck_;
  string *base_;
  string *cppParent_;
  string *cParent_;
  string *pCast_;
 
  list<SignalDef*> signals_;
  list<MethodDef*> methods_;
  list<MemberDef*> members_;
 
  bool cast_ctor_;
  bool default_ctor_;
  bool dtor_;

  int id_;

public:
  WidgetDef();

  WidgetDef(
	    string *cppName,
	    string *cName,
	    string *cCast,
	    string *cCheck,
	    string *base,
	    string *cppParent,
	    string *cParent,
	    string *pCast=0
	    );
  ~WidgetDef();

  void push_member(MemberDef*);
  void push_signal(SignalDef*);
  void push_method(MethodDef*);

  void print_decl_begin(FILE *);
  void print_decl_end(FILE *);

  void set_dtor(FILE *);
  void set_default_ctor(FILE *);
  void set_cast_ctor(FILE *);
};

class OutputChannel
{
public:
  OutputChannel();
  ~OutputChannel();

  FILE* out() { return out_; }

  void process();

  void section_class(string ns);
  void section_class();
  void section_impl();
  void section_private();
  void section_doc();

protected:
  FILE *out_;
  char *temp1;
  char *temp2;
  static const char *const gensigM4;
};

/////////////////////////////////////////////////////////////////////////////
// Environment (holds file names and such )
/////////////////////////////////////////////////////////////////////////////
class Environment
{
protected:
  string name_;
  string header_prefix_; //see firewall().
  string srcdir_;
  string destdir_;
  string m4_include_;
  bool local_;
  bool doc_;
  bool debug_;
  bool debug_genh_;
  bool gnome_;

public:
  Environment();
  ~Environment();

  void usage();
  void init(int argc,char **argv) ;

  string header_name()  {return (destdir()+name_+".h");}
  string private_name() {if (doc_) return ""; return (destdir()+"private/"+name_+"_p.h");}
  string impl_name()    {if (doc_) return ""; return (destdir()+name_+".cc");}
  string input_name()   {return (srcdir()+name_+".gen_h");}
  string firewall();

  string srcdir()  {return srcdir_;}
  string destdir() {return destdir_;}
 
  string m4_invoke();

  bool local() {return local_;}
  bool doc()   {return doc_;}
  bool debug() {return debug_;}
  bool debug_genh() {return debug_genh_;}
  bool gnome() {return gnome_;}

  void print_includes(FILE *);
};

extern Environment env;
