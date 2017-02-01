#include <sigc++/signal_system.h>
#include <iostream>

#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

int foo1(int i) {cout<<"f("<<i<<");"<<endl; return 1;}
int foo2(int i) {cout<<"sig2::f("<<i<<");"<<endl; return 1;}
void foo1v(int i) {cout<<"fv("<<i<<");"<<endl;}
void foo2v(int i) {cout<<"sig2::fv("<<i<<");"<<endl;}

struct A:public Object
  {
   int foo(int i)   {cout<<"A::f("<<i<<");"<<endl;return 1;}
   void foov(int i) {cout<<"A::fv("<<i<<");"<<endl;}

   Signal1<int,int> sig1;
   Signal1<void,int> sig1v;
   A():sig1(slot(*this,&A::foo)),sig1v(slot(*this,&A::foov)) {}
  };


int main(int argc,char **argv)
  {
   A a;
   a.sig1.connect(slot(foo1));
   a.sig1v.connect(slot(foo1v));

   a.sig1.emit(1);
   a.sig1v.emit(2);

   return 0;  // compiling is passing
  }
