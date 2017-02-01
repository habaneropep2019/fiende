#include <sigc++/signal_system.h>
#include <iostream>

#ifdef SIGC_CXX_NAMESPACES
using namespace std;
using namespace SigC;
#endif

int j=0;

int foo1(int i) {j++;cout<<"f("<<i<<");"<<endl; return 1;}
int foo2(int i) {j++;cout<<"sig2::f("<<i<<");"<<endl; return 1;}
void foo1v(int i) {j++;cout<<"fv("<<i<<");"<<endl;}
void foo2v(int i) {j++;cout<<"sig2::fv("<<i<<");"<<endl;}
struct A:public Object
  {
   int foo(int i) {j++;cout<<"A::f("<<i<<");"<<endl; return 1;}
   void foov(int i) {j++;cout<<"A::fv("<<i<<");"<<endl;}
   A() {}
  };

struct B:public A
  {
  };


int main(int argc,char **argv)
  {
   cout << ">> Sizes "<< endl;
   cout << "sizeof (Signal1<int,int>) = "<<sizeof(Signal1<int,int>)<<endl;
   B a;
   Object *o2=new Object();

   Signal1<int,int> sig1;  // int sig(int);
   Signal1<int,int> sig2; // int sig2(int);
   Signal1<void,int> sig1v;  // void sig(int);
   Signal1<void,int> sig2v; // void sig2(int);

   cout << ">> Connect to signals "<< endl;
   sig2.connect(slot(foo2));
   sig2v.connect(slot(foo2v));

   sig1.connect(slot(foo1));
   sig1.connect(slot(a,&A::foo));
   sig1.connect(sig2.slot());

   sig1v.connect(slot(foo1v));
   sig1v.connect(slot(a,&A::foov));
   sig1v.connect(sig2v.slot());

   cout << ">> Emit signals "<< endl;
   sig1.emit(1);
   sig1v.emit(2);

   return (j!=6);
  }
