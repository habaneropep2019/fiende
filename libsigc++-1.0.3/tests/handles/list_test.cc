
#include <sigc++/object.h>
#include <sigc++/scope.h>
#include <sigc++/handle.h>

#include <iostream>
#include <list>

#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

int global=0;

struct A:public Object
  {
   A() {cout<<"A()"<<endl;}
   ~A() {global=1;cout<<"~A()"<<endl;}
  };

typedef list<Handle<A,Scopes::Limit> > List;

int main(int argc,char **argv)
  {
   Handle<A,Scopes::RefCount > a=manage(new A());
   List l;
   List::iterator i;

   cout <<"before add"<<endl;
   i=l.insert(l.begin(),0);
   (*i)=a;
   cout <<"after add"<<endl;
   if (global) 
     return 1;
   else
     return 0;
  } 
