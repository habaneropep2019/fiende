// wintest.cpp : Defines the entry point for the console application.
//

#include <sigc++/signal_system.h>
#include <sigc++/generator.h>
#include <sigc++/rettype.h>
#include <sigc++/bind.h>
#include <sigc++/convert.h>
#include <iostream.h>
#include <string>


#ifdef SIGC_CXX_NAMESPACES
using namespace SigC;
using namespace std;
#endif

/********************************************************/
int foo1(int i) {cout<<"f("<<i<<");"<<endl;return 1;}
int foo2(int i) {cout<<"sig2::f("<<i<<");"<<endl;return 1;}
void foo1v(int i) {cout<<"fv("<<i<<");"<<endl;}
void foo2v(int i) {cout<<"sig2::fv("<<i<<");"<<endl;}

class A:public Object  
  {
   public: 
    int foo(int i) {cout<<"A::f("<<i<<");"<<endl;return 1;}
    void foov(int i) {cout<<"A::fv("<<i<<");"<<endl;}
    A() {cout<<"A()"<<endl;}
    ~A() {cout<<"~A()"<<endl;}
  };

int my_string_to_char(Callback1<int,const char*> *d,const string &s)
  {
   return d->call(s.c_str());
  }

int foobar(const char* f)
  {
    cout << f<<endl;
	return 1;
  }

int main(int argc,char **argv)
  {
   int i;
   cout << "Test contained"<<endl;
   cout << "begin"<<endl;
     {
      Handle<A,Scopes::Extend> o=gen<A>();
      i=1;
     }
   cout << "end"<<endl;
   cout << endl;

   cout << "Test extend:"<<endl;
   cout << "begin outer"<<endl;
     {
      Handle<A,Scopes::Extend> o2;
      cout << "begin inter"<<endl;
        {
         Handle<A,Scopes::Extend> o1=gen<A>();
         o2=o1;
         i=1;
        }
      cout << "end inter"<<endl;
     }
   cout << "end outer"<<endl; 
   cout << endl;

   cout << "Test limit:"<<endl;
   cout << "begin outer"<<endl;
     {
      Handle<A,Scopes::Extend> o2=gen<A>();
      cout << "begin inter"<<endl;
        {
         Handle<A,Scopes::Limit> o1=o2;
         cout<< "In inter"<<endl;
         i=1;
        }
      cout << "end inter"<<endl;
     }
   cout << "end outer"<<endl; 
   cout << endl;

   cout << ">> Sizes "<< endl;
   cout << "sizeof (Signal1<int,int>) = "<<sizeof(Signal1<int,int,Marshal<int> >)<<endl;
   A a;
   Object *o2=new Object();

   Signal1<int,int,Marshal<int> > sig1;  // int sig(int);
   Signal1<int,int,Marshal<int> > sig2; // int sig2(int);
   Signal1<void,int,Marshal<void> > sig1v;  // void sig(int);
   Signal1<void,int,Marshal<void> > sig2v; // void sig2(int);

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

   {
   cout << ">> Connecting Slots"<<endl;
   Slot1<void,int>    slot1=rettype<void>(slot(foo1));
   Slot1<float,int>   slot2=rettype<float>(slot(foo1));
   cout << ">> Calling Slots"<<endl;
   slot1.call(1);
   slot2.call(1);
   }

   {
   cout << ">> Connecting Slots"<<endl;
   Slot0<void>      slot1=bind(slot(foo1v),1);
   Slot0<int>       slot2=bind(slot(foo1),2);
   cout << ">> Calling Slots"<<endl;
   slot1.call();
   slot2.call();
   }

   Slot1<int,const string &>  s=convert(slot(foobar),my_string_to_char);
   s("hello");   
   
   return 1;
  }
