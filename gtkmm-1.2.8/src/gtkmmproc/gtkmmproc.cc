// $Id: gtkmmproc.cc,v 1.59 2001/02/13 22:00:39 murrayc Exp $

#include <stdio.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include "gtkmmproc.h"

Environment env;

/*********************************************************************/


Argument::Argument(string *type,string *name,string *value)
  : type_(type), name_(name), value_(value)
{}

Argument::Argument(const Argument& a)
  : type_(0), name_(0), value_(0)
{
  if (a.type_) type_=new string(*(a.type_));
  if (a.name_) name_=new string(*(a.name_));
  if (a.value_) value_=new string(*(a.value_));
}
  
Argument::~Argument()
{ 
  delete type_;
  delete name_;
  delete value_;
}

void Argument::assign_name(int num)
{
  char buf[20];
  sprintf(buf,"p%d",num);
  if (!name_) name_=new string(buf);
}

string Argument::get_type() const
{
  if (type_) return *type_;
  cerr << "missing argument type" << endl;
  return "BAD_TYPE";
}

string Argument::get_name() const
{
  if (name_) return *name_;
  if (type_) cerr << *type_ << ":";
  cerr << "missing argument name" << endl;
  return "BAD_NAME";
}

string Argument::get_both() const
{
  string s;
  if (type_) { s+=*type_; }
  if (name_) { s+=" "; s+=*name_; }
  return s;
}

string Argument::get_vboth() const
{
  string s;
  if (type_) { s+=*type_; }
  if (name_) { s+=" "; s+=*name_; }
  if (value_) { s+="="; s+=*value_; }
  return s;
}

/*********************************************************************/

// Class for holding argument lists
ArgList::ArgList()
  :num_(0)
{}
 
ArgList::ArgList(const ArgList& a)
{
  num_=a.num_;
  List::const_iterator i;
  for (i=a.args_.begin();i!=a.args_.end();i++)
    args_.insert(args_.end(),new Argument(*(*i)));
}

// return a coma seperated list of the args, for instance :
// const int, char, long*
string ArgList::get_types() const 
{
  string s;
  List::const_iterator arg=args_.begin();
  while (arg!=args_.end())
    {
      s+=(*arg)->get_type();
      arg++;
      if (arg==args_.end()) break;
      s+=",";
    }
  return s;
}

// same as get_types(), but we skip 1st arg
string ArgList::get_ctypes() const
{
  string s;
  List::const_iterator arg=args_.begin();
  arg++;
  while (arg!=args_.end())
    {
      s+=(*arg)->get_type();
      arg++;
      if (arg==args_.end()) break;
      s+=",";
    }
  return s;
}

// same, but with synthesized names :
// const int p1, char p2, long* p3
string ArgList::get_both() const
{
  string s;
  List::const_iterator arg=args_.begin();
  while (arg!=args_.end())
    {
      s+=(*arg)->get_both();
      arg++;
      if (arg!=args_.end()) 
	s+=",";
    }
  return s;
}

// get arg types, names, and values :
// const int p1 = 0, char p2 = 'c', long* p3 = 0
string ArgList::get_vboth() const
{
  string s;
  List::const_iterator arg=args_.begin();
  while (arg!=args_.end())
    {
      s+=(*arg)->get_vboth();
      arg++;
      if (arg!=args_.end()) 
	s+=",";
    }
  return s;
}

// same as get_both(), but we skip the 1st arg
string ArgList::get_cboth() const
{
  string s;
  List::const_iterator arg=args_.begin();
  arg++;
  while (arg!=args_.end())
    {
      s+=(*arg)->get_both();
      arg++;
      if (arg!=args_.end())
        s+=",";
    }
  return s;
}


// return the synthesized names only :
// p1, p2, p3
string ArgList::get_names() const
{
  string s;
  List::const_iterator arg=args_.begin();
  while (arg!=args_.end())
    {
      s+=(*arg)->get_name();
      arg++;
      if (arg!=args_.end()) 
	s+=",";
    }
  return s;
}

string ArgList::get_cnames() const
{
  string s;
  List::const_iterator arg=args_.begin();
  arg++;
  while (arg!=args_.end())
    {
      s+=(*arg)->get_name();
      arg++;
      if (arg!=args_.end()) 
	s+=",";
    }
  return s;
}

void ArgList::push_arg(Argument *arg)
{
  if (!arg) return;
  arg->assign_name(num_);
  args_.insert(args_.end(),arg);
  num_++;
}

void ArgList::reset() 
{ 
  List::iterator i;
  for (i=args_.begin();i!=args_.end();i++)
    delete *i;
  args_.clear(); 
}

void ArgList::add_first_arg() 
{ 
  args_.insert(args_.begin(),
    new Argument(new string("__CNAME__*"),new string("o_")));
  num_++;
}

void ArgList::erase_first_arg() 
{ 
  if(!args_.empty()) 
    {
      delete *(args_.begin());
      args_.erase(args_.begin()); 
    }
  num_--;
}

ArgList::~ArgList()
{
  reset();
}

string cpp2c_convert(ArgList& cppFunc,ArgList& cFunc,int coma,int is_static)
{
  string tmp;
  int numargs=cFunc.num_;

  ArgList::List &cargs=cFunc.args_;
  ArgList::List &cppargs=cppFunc.args_;

  ArgList::List::iterator carg=cargs.begin();
  ArgList::List::iterator cpparg=cppargs.begin();

  if (!is_static)
    {
     carg++;
     numargs--;
    }

  if (coma&&cpparg!=cppargs.end()) tmp+=",";
     
  if (numargs!=cppFunc.num_)
    {
      cerr <<"cpp2c_convert: Argument list mismatch "<<endl;
      return "";
    }

  while (cpparg!=cppargs.end())
    {
      if ((*cpparg)->get_type()!=(*carg)->get_type())
        tmp+="__FWD_CONVERT(`"+(*carg)->get_type()+"',`"
                             +(*cpparg)->get_type()+"',`"
                             +(*cpparg)->get_name()+"')";
      else
        tmp+=(*cpparg)->get_name();
      carg++;
      cpparg++;
      if (cpparg!=cppargs.end()) tmp+=",";
    }
  return tmp;
}


string c2cpp_convert(ArgList& cppFunc,ArgList& cFunc,int coma,int is_static)
{
  string tmp;
  int numargs=cFunc.num_;

  ArgList::List &cargs=cFunc.args_;
  ArgList::List &cppargs=cppFunc.args_;

  ArgList::List::iterator carg=cargs.begin();
  ArgList::List::iterator cpparg=cppargs.begin();

  if (!is_static)
    {
     carg++;
     numargs--;
    }

  if (coma&&cpparg!=cppargs.end()) tmp+=",";
  
  if (numargs!=cppFunc.num_)
    {
      cerr <<"c2cpp_convert: Argument list mismatch " << endl;
      return "";
    }

  while (cpparg!=cppargs.end())
    {
      if ((*cpparg)->get_type()!=(*carg)->get_type())
        tmp+="__REV_CONVERT("+(*carg)->get_type()
                             +","+(*cpparg)->get_type()
                             +","+(*carg)->get_name()+")";
      else
        tmp+=(*carg)->get_name();
      carg++;
      cpparg++;
      if (cpparg!=cppargs.end()) tmp+=",";
    }
  return tmp;
}

/***********************************************************************/

// Class for holding functions
FunctionDef::FunctionDef(string *rettype,
			 string *name,
			 ArgList* args,
			 bool isconst) 
{
  if (!name) name_=new string(); name_=name;
  if (!rettype) rettype_=new string(); rettype_=rettype;
  if (!args) args_=new ArgList(); else args_=args;
  isconst_=isconst;         
}

FunctionDef::FunctionDef(const FunctionDef& f)
{
  name_=new string(*(f.name_));
  rettype_=new string(*(f.rettype_));
  args_=new ArgList(*(f.args_));
  isconst_=f.isconst_;
}

string FunctionDef::get_function()
{
  string s=*rettype_;
  s+=" ";
  s+=*name_;
  s+="(";
  s+=args_->get_types();
  s+=")";
  if (isconst_) s+=" const";
  return s;
}

/////////////////////////////////////////////////////////////////////////////
// Signal definition
/////////////////////////////////////////////////////////////////////////////
SignalDef::SignalDef(int attr,string *name,
		     FunctionDef *cppFunc, string *cfuncname)
  : MethodDef(0,cppFunc),
    name_(name),
    attr_(attr),
    id_(0)
{
  if (attr_&ATTR_NOEMIT)
    attr_&=~ATTR_EMIT;

  // fake a name and set attribute to implement gtkfunction
  if (!cfuncname)
    {
     string tmp="gtkmm_`'__BASE__`'_";
     tmp+=(*name);
     cfuncname=new string(tmp);
     if (attr_&ATTR_EMIT) attr_|=ATTR_GTK_EMIT;  
    }

  // make a gtk function prototype
  cFunc_=new FunctionDef(new string(cppFunc->get_return_type()),
                         cfuncname,
                         new ArgList(cppFunc->get_args()));

  cFunc_->get_args().add_first_arg();

}

SignalDef::SignalDef(int attr,string *name,
		     FunctionDef *cppFunc, FunctionDef *cFunc)
  : MethodDef(cFunc,cppFunc),
    name_(name),
    attr_(attr),
    id_(0)
{
  if (attr_&ATTR_NOEMIT)
    attr_&=~ATTR_EMIT;
}

SignalDef::~SignalDef()
{
  if (name_) delete name_;
}

// Declarations
void SignalDef::print_decl(FILE *fptr)
{
  fprintf(fptr,"GTKMM_PROXY_NAME(%s)\n",name_->c_str());

  if ((attr_&(ATTR_GTK_EMIT|ATTR_EMIT))==(ATTR_GTK_EMIT|ATTR_EMIT))
    fprintf(fptr,"GTKMM_GTK_EMIT(%s,%s,`%s',`%s',%s)\n",
	    cFunc_->get_return_type().c_str(), 
	    cFunc_->get_name().c_str(), 
	    cFunc_->get_args().get_cboth().c_str(),
	    cFunc_->get_args().get_cnames().c_str(),
            name_->c_str()
	    );

    
  if ((attr_&(ATTR_VFUNC|ATTR_EMIT))==(ATTR_VFUNC|ATTR_EMIT))
    MethodDef::print_decl(fptr);

  if (attr_&ATTR_IMPL)
    {
      fprintf(fptr,"GTKMM_PROXY_CALLBACK(%s,%s,%s,%s,`%s',`%s',`%s',%s)\n",
              cppFunc_->get_name().c_str(), 
              name_->c_str(), 
              cppFunc_->get_return_type().c_str(), 
              cFunc_->get_return_type().c_str(), 
              cFunc_->get_args().get_both().c_str(),
              cFunc_->get_args().get_names().c_str(),
              c2cpp_convert(cppFunc_->get_args(),cFunc_->get_args(),0,0).c_str(),
              cFunc_->get_args().get_first_name().c_str()
              );

      fprintf(fptr,"GTKMM_PROXY_IMPL(%s,%s,%s,%s,`%s',`%s')\n",
              cppFunc_->get_name().c_str(), 
              name_->c_str(), 
              cppFunc_->get_return_type().c_str(), 
              cFunc_->get_return_type().c_str(), 
              cppFunc_->get_args().get_both().c_str(),
              cpp2c_convert(cppFunc_->get_args(),cFunc_->get_args(),1,0).c_str()
              );

     if (attr_&ATTR_MARSH)
     if (cFunc_)
       if (cFunc_->get_return_type()!="void")
         { fprintf(fptr,"GTKMM_MARSHAL(%s,%s,`%s')\n",
             name_->c_str(),
             cFunc_->get_return_type().c_str(),
             cFunc_->get_args().get_ctypes().c_str());
         }
     else
       if (cppFunc_->get_return_type()!="void")
         { fprintf(fptr,"GTKMM_MARSHAL(%s,%s,`%s')\n",
             name_->c_str(),
             cppFunc_->get_return_type().c_str(),
             cppFunc_->get_args().get_types().c_str());
         }
    }

  if (!(attr_&ATTR_SIG)) return;

  fprintf(fptr,"GTKMM_PROXY_SIGNAL(%s,%s)\n",
          name_->c_str(), 
          cppFunc_->get_name().c_str()
         );

  if (!(attr_&(ATTR_EMIT|ATTR_TRANSLATE)))
    {
      fprintf(fptr,"GTKMM_PROXY_SIGNAL_NOEMIT(%s,%s,`%s',%d)\n",
              cppFunc_->get_return_type().c_str(), 
              cppFunc_->get_name().c_str(), 
              cppFunc_->get_args().get_types().c_str(),
              id_
             );
      return;
    }
  if (!(attr_&ATTR_TRANSLATE))
    {
      fprintf(fptr,"GTKMM_PROXY_SIGNAL_EMIT(%s,%s,`%s',%d,%s)\n",
              cppFunc_->get_return_type().c_str(), 
              cppFunc_->get_name().c_str(), 
              cppFunc_->get_args().get_types().c_str(),
              id_,
              cFunc_->get_name().c_str()
             );
      return;
    }

  if (!(attr_&ATTR_EMIT))
    {
      fprintf(fptr,"GTKMM_PROXY_SIGNAL_TRANSLATE_NOEMIT_DECL(%s,%s,%s,%s,`%s',`%s',%d)\n",
              cppFunc_->get_name().c_str(), 
              cFunc_->get_name().c_str(),
              cppFunc_->get_return_type().c_str(), 
              cFunc_->get_return_type().c_str(), 
              cppFunc_->get_args().get_types().c_str(),
              cFunc_->get_args().get_ctypes().c_str(),
              id_
             );
      fprintf(fptr,"GTKMM_PROXY_SIGNAL_TRANSLATE_IMPL(%s,%s,%s,%s,`%s',`%s',`%s')\n",
              cppFunc_->get_name().c_str(), 
              cFunc_->get_name().c_str(),
              cppFunc_->get_return_type().c_str(), 
              cFunc_->get_return_type().c_str(), 
              cppFunc_->get_args().get_types().c_str(),
              cFunc_->get_args().get_cboth().c_str(),
              c2cpp_convert(cppFunc_->get_args(),cFunc_->get_args(),0,0).c_str()
             );
      return;
    }

  fprintf(fptr,"GTKMM_PROXY_SIGNAL_TRANSLATE_EMIT_DECL(%s,%s,%s,%s,`%s',`%s',%d)\n",
          cppFunc_->get_name().c_str(), 
          cFunc_->get_name().c_str(),
          cppFunc_->get_return_type().c_str(), 
          cFunc_->get_return_type().c_str(), 
          cppFunc_->get_args().get_types().c_str(),
          cFunc_->get_args().get_ctypes().c_str(),
          id_
          );
  fprintf(fptr,"GTKMM_PROXY_SIGNAL_TRANSLATE_IMPL(%s,%s,%s,%s,`%s',`%s',`%s')\n",
          cppFunc_->get_name().c_str(), 
          cFunc_->get_name().c_str(),
          cppFunc_->get_return_type().c_str(), 
          cFunc_->get_return_type().c_str(), 
          cppFunc_->get_args().get_types().c_str(),
          cFunc_->get_args().get_cboth().c_str(),
          c2cpp_convert(cppFunc_->get_args(),cFunc_->get_args(),0,0).c_str()
         );
  fprintf(fptr,"GTKMM_PROXY_SIGNAL_TRANSLATE_EMIT_IMPL(%s,%s,%s,`%s',`%s')\n",
          cppFunc_->get_name().c_str(), 
          cppFunc_->get_return_type().c_str(), 
          cFunc_->get_return_type().c_str(), 
          cppFunc_->get_args().get_both().c_str(),
          cpp2c_convert(cppFunc_->get_args(),cFunc_->get_args(),0,0).c_str()
          );

}


///////////////////////////////////////////////////////////////////////
MemberDef::MemberDef(int attrib,string *cppfunc,string *membername,string *cpptype,string *ctype)
  :attrib_(attrib),cppfunc_(cppfunc),membername_(membername),cpptype_(cpptype),ctype_(ctype)
{ 
}

MemberDef::~MemberDef() 
{
  if (cppfunc_) delete cppfunc_;
  if (membername_) delete membername_;
  if (cpptype_) delete cpptype_;
  if (ctype_) delete ctype_;
}

void MemberDef::print_decl(FILE *out)
{
  switch (attrib_)
    {
      case ATTR_REF:
        fprintf(out,"GTKMM_MEMBER_REF(%s,%s,%s,%s)\n",
	  cppfunc_->c_str(),
	  membername_->c_str(),
	  cpptype_->c_str(),
	  ctype_->c_str());
        break;
      case ATTR_VALUE:
        fprintf(out,"GTKMM_MEMBER_VALUE(%s,%s,%s,%s)\n",
	  cppfunc_->c_str(),
	  membername_->c_str(),
	  cpptype_->c_str(),
	  ctype_->c_str());
        break;
      case ATTR_INLINE|ATTR_REF:
        fprintf(out,"GTKMM_MEMBER_REF_INLINE(%s,%s,%s,%s)\n",
	  cppfunc_->c_str(),
	  membername_->c_str(),
	  cpptype_->c_str(),
	  ctype_->c_str());
        break;
      case ATTR_INLINE|ATTR_VALUE:
        fprintf(out,"GTKMM_MEMBER_VALUE_INLINE(%s,%s,%s,%s)\n",
	  cppfunc_->c_str(),
	  membername_->c_str(),
	  cpptype_->c_str(),
	  ctype_->c_str());
        break;
      default:
        cerr << cppfunc_->c_str() << " - invalid attributes." <<endl;
        break;
    }
}

///////////////////////////////////////////////////////////////////////
MethodDef::MethodDef(FunctionDef *cFunc,FunctionDef *cppFunc)
    : cFunc_(cFunc), cppFunc_(cppFunc)
{}

MethodDef::~MethodDef()
{
  if (cFunc_) delete cFunc_;
  if (cppFunc_) delete cFunc_;
}



void MethodDef::print_decl(FILE *out)
{
  fprintf(out,"GTKMM_METHOD(%s,%s,%s,%s,`%s',`%s',`%s',%d)\n",
	  cppFunc_->get_name().c_str(),
	  cFunc_->get_name().c_str(),
	  cppFunc_->get_return_type().c_str(),
	  cFunc_->get_return_type().c_str(),
	  cppFunc_->get_args().get_vboth().c_str(),
	  cppFunc_->get_args().get_both().c_str(),
	  cpp2c_convert(cppFunc_->get_args(),cFunc_->get_args(),1).c_str(),
	  cppFunc_->is_const()
	  );
}

StaticMethodDef::StaticMethodDef(FunctionDef *cFunc,FunctionDef *cppFunc)
    :MethodDef(cFunc,cppFunc)
{}

StaticMethodDef::~StaticMethodDef()
{}

void StaticMethodDef::print_decl(FILE *out)
{
  fprintf(out,"GTKMM_STATIC_METHOD(%s,%s,%s,%s,`%s',`%s')\n",
	  cppFunc_->get_name().c_str(),
	  cFunc_->get_name().c_str(),
	  cppFunc_->get_return_type().c_str(),
	  cFunc_->get_return_type().c_str(),
	  cppFunc_->get_args().get_both().c_str(),
	  cpp2c_convert(cppFunc_->get_args(),cFunc_->get_args(),0,1).c_str()
	  );
}



///////////////////////////////////////////////////////////////////////

WidgetDef::WidgetDef()
  :cppName_(new string()),
   cName_(new string()),
   cCast_(new string()),
   cCheck_(new string()),
   base_(new string()),
   cppParent_(new string()),
   cParent_(new string()),
   pCast_(0),
   cast_ctor_(0),
   default_ctor_(0),
   id_(0)
{}

WidgetDef::WidgetDef( 
		     string *cppName,
		     string *cName,
		     string *cCast,
		     string *cCheck,
		     string *base,
		     string *cppParent,
		     string *cParent,
		     string *pCast
		     )
  :cppName_(cppName),
   cName_(cName),
   cCast_(cCast),
   cCheck_(cCheck),
   base_(base),
   cppParent_(cppParent),
   cParent_(cParent),
   pCast_(pCast),
   cast_ctor_(0),
   default_ctor_(0),
   id_(0)
{}

WidgetDef::~WidgetDef()
{
  delete cppName_;
  delete cppParent_;
  delete cName_;
  delete cParent_;
  delete cCast_;
  delete cCheck_;
  delete base_;
  delete pCast_;
    
  // need to empty list
  list<SignalDef*>::iterator si; 
  list<MethodDef*>::iterator mi; 
  list<MemberDef*>::iterator ai; 
  for (mi=methods_.begin();mi!=methods_.end();mi++)
    delete (*mi);
  for (si=signals_.begin();si!=signals_.end();si++)
    delete (*si);
  for (ai=members_.begin();ai!=members_.end();ai++)
    delete (*ai);
}

void WidgetDef::push_member(MemberDef *s)
{
  members_.insert(members_.end(),s);
}

void WidgetDef::push_signal(SignalDef *s)
{
  s->set_id(id_++);
  signals_.insert(signals_.end(),s);
}

void WidgetDef::push_method(MethodDef *s)
{
  methods_.insert(methods_.end(),s);
}

void WidgetDef::print_decl_begin(FILE *fptr)
{ 
  fprintf(fptr,"GTKMM_CLASS_START(%s,%s,%s,%s,%s,%s,%s,%s)\n",
	  cppName_->c_str(),
	  cName_->c_str(),
	  cCast_->c_str(),
	  cCheck_->c_str(),
	  base_->c_str(),
	  cppParent_->c_str(),
	  cParent_->c_str(),
	  pCast_->c_str()
	  );
}

void WidgetDef::print_decl_end(FILE *fptr)
{ 
  fprintf(fptr,"GTKMM_CLASS_END()\n");
}

void WidgetDef::set_default_ctor(FILE *fptr)
{
  fprintf(fptr,"GTKMM_CTOR_DEFAULT()");
}

void WidgetDef::set_cast_ctor(FILE *fptr)
{
  fprintf(fptr,"GTKMM_CTOR_CAST()");
}

void WidgetDef::set_dtor(FILE *fptr)
{
  fprintf(fptr,"GTKMM_DTOR()");
}

///////////////////////////////////////////////////////////////////////

OutputChannel::OutputChannel()
{
  FILE *tmp_;
  if (!(temp1=tempnam(NULL,"gtkmm")))
    {  
      cerr << "tempnam() failed"<<endl;
      exit (-1);
    }

  if (!(out_=fopen(temp1,"w")))
    { 
      cerr << "Could not open "<< temp1<<" for output. "<< endl;
      exit (-1);
    }

  fprintf(out_,
	  "define(`__GPREFIX__',`%s')dnl\n"
	  "include(%s)dnl\n",
	  env.gnome() ? "gnome" : "gtk",
          env.doc() ? "doc1.m4" : "stage1.m4");

  fprintf(out_,"GTKMM_START(%s,%s)\n",
	  env.firewall().c_str(),env.input_name().c_str());

  env.print_includes(out_);
}

OutputChannel::~OutputChannel()
{}

void OutputChannel::section_class()
{
  fprintf(out_,"GTKMM_SECTION(CLASS)\n");
}

void OutputChannel::section_class(string ns)
{
  fprintf(out_,"GTKMM_NAMESPACE(%s)\nGTKMM_SECTION(CLASS)\n",ns.c_str());
}

void OutputChannel::section_private()
{
  fprintf(out_,"GTKMM_SECTION(PHEADER)\n");
}

void OutputChannel::section_impl()
{
  fprintf(out_,"GTKMM_SECTION(IMPL)\n");
}

void OutputChannel::section_doc()
{
  fprintf(out_,"GTKMM_SECTION(DOC)\n");
}

void OutputChannel::process()
{
  fprintf(out_,"GTKMM_END()\n");
  fclose(out_);
 
  // run m4 once to generate intermediate file.
  temp2=tempnam(NULL,"gtkmm");
  string sc(env.m4_invoke());
  sc+=string(temp1) + " > ";
  sc+=string(temp2);

  if(system(sc.c_str()))
    cerr << "m4 invocation failed : " << sc << endl;
  else
    if (!env.debug()) unlink(temp1);

  fixate(string(temp2),env.header_name(),0,env.debug_genh());
  if (!env.doc())
    {
      fixate(string(temp2),env.private_name(),1,env.debug_genh());
      fixate(string(temp2),env.impl_name(),2,env.debug_genh());
    }

  if (!env.debug()) 
     {unlink(temp2);} 
  else
    {
      cerr << "T1: "<<temp1<<endl;
      cerr << "T2: "<<temp2<<endl;
    }
}

///////////////////////////////////////////////////////////////////

extern int yylineno;
Environment::Environment()
  : name_("test"), header_prefix_("gtkmm"),
    srcdir_("./"), destdir_("./"),
    local_(false),doc_(false),debug_(false),debug_genh_(false),gnome_(false)
{}

Environment::~Environment()
{}

void Environment::usage()
{
  cerr << "Usage: gensig2 [options] name srcdir destdir" << endl
       << "    -h" <<endl
       << "    --help           This usage message." <<endl
       << endl
       << "    --doc            Produces a header file for documentation." <<endl
       << endl
       << "    --debug          Leave intermediate output arround for analysis." <<endl
       << endl
       << "    --debug-genh     Generate line numbers back to gen_h source file." <<endl
       << endl
       << "    --m4 dir         Specify the directory with m4 files." <<endl
       << endl
       << "    -l" <<endl
       << "    --local          Local widget (not part of gtk--)." <<endl
       << endl
       << "    --header-prefix  Override default header #ifndef name." <<endl
       << endl
       << "    -g" <<endl
       << "    --gnome          Use GNOMEMM header prefix, instead of GTKMM." << endl
       << "                     See --header_prefix." <<endl
       << endl
       << "Note: This will read srcdir/name.gen_h file and generates destdir/name.cc" <<endl;
  exit(0);
};

void Environment::init(int argc,char** argv)
{

  int i=1;
  if (argc<2)
    {
      cerr<< "Invalid number of arguments."<<endl;
      usage();
    }
   
  while (argv[i][0]=='-')
    {
      string tmp(argv[i]);
      if (tmp=="-l"||tmp=="--local") 
        {
          local_=true;
        }
      else if (tmp=="-g"||tmp=="--gnome")    
        {
          gnome_=true;
          header_prefix_="gnomemm";
        }
      else if (tmp=="--header-prefix")    
        {
          i++;
          header_prefix_ = argv[i];
        }
      else if (tmp=="-h"||tmp=="--help")
        {
          usage();
        }
      else if (tmp=="--m4")
        { 
          i++;
          m4_include_=argv[i];
        }
      else if (tmp=="--debug")
        { 
          debug_=true;
        }
      else if (tmp=="--debug-genh")
        { 
          debug_genh_=true;
        }
      else if (tmp=="--doc")
        { 
          doc_=true;
        }
      else
        {
          cerr<<"Unknown argument "<<tmp<<endl;
        }
      i++;
      if (i==argc) 
        {
          cerr<< "Invalid number of arguments."<<endl;
          usage();
        }
    }

  name_=argv[i];
  if (i+1<argc) srcdir_=argv[i+1];
  if (i+2<argc) destdir_=argv[i+2];

  if(srcdir_[srcdir_.length() - 1] != '/') srcdir_ += '/';
  if(destdir_[destdir_.length() - 1] != '/') destdir_ += '/';
  
}

void Environment::print_includes(FILE *fptr)
{
  fprintf(fptr,"GTKMM_SECTION(IMPL)\n");
  if (local_)
    {
      fprintf(fptr,"#include \"%s.h\"\n",name_.c_str());
      fprintf(fptr,"#include \"private/%s_p.h\"\n",name_.c_str());
    }
  else if (gnome_)
    {
      fprintf(fptr,"#include <gnome--/%s.h>\n",name_.c_str());
      fprintf(fptr,"#include <gnome--/private/%s_p.h>\n",name_.c_str());
    }
  else
    {
      fprintf(fptr,"#include <gtk--/%s.h>\n",name_.c_str());
      fprintf(fptr,"#include <gtk--/private/%s_p.h>\n",name_.c_str());
    }
  fprintf(fptr,"GTKMM_SECTION(HEADER)\n");
}

string Environment::m4_invoke()
{
  if (!m4_include_.length())
    return M4 " -I " MACRO_DIR " ";
  else 
    return M4 " -I " + m4_include_ + " -I " MACRO_DIR " ";
}

string Environment::firewall()
{
  //Command-line --header-prefix arg can override default header #IFDEFs 
  if(header_prefix_.size())
  {
    return header_prefix_ + "_" + name_;
  }
  else
  {
    //This shouldn't happen (header_prefix_ always has a value):
	  if(gnome())
	    return "gnomemm_" + name_;
	  else
	    return "gtkmm_" + name_;
	}
}
